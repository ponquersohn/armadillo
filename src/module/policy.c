#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>
#include <linux/completion.h>
#include <linux/uaccess.h>
#include <linux/string.h>
#include <linux/cred.h>
#include <linux/atomic.h>
#include <linux/fs.h>

#include "module.h"
#include "policy.h"

struct armadillo_policy_request {
    struct list_head       list;          /* pending_queue or in_flight */
    u64                    id;
    pid_t                  pid;
    pid_t                  ppid;
    uid_t                  uid;
    char                   comm[ARMADILLO_POLICY_COMM_LEN];
    char                   path[ARMADILLO_POLICY_PATH_MAX];
    enum armadillo_verdict verdict;       /* set by reply */
    bool                   abandoned;     /* set when waiter gave up */
    struct completion      done;
};

static LIST_HEAD(pending_queue);              /* waiting for daemon to pull */
static LIST_HEAD(in_flight);                  /* pulled, awaiting reply */
static DEFINE_SPINLOCK(policy_lock);
static DECLARE_WAIT_QUEUE_HEAD(daemon_wq);
static atomic64_t next_id = ATOMIC64_INIT(1);

/* At most one daemon attached at a time. filp is the handle that attached. */
static struct file *daemon_filp;
static pid_t        daemon_pid;

/* Shutdown flag — cleanup_module sets this, hook stops waiting. */
static bool policy_shutting_down;

/* Runtime config. */
static u32  policy_timeout_ms = 2000;
static bool policy_fail_closed;           /* false = fail-open */

int armadillo_policy_init(void)
{
    INIT_LIST_HEAD(&pending_queue);
    INIT_LIST_HEAD(&in_flight);
    daemon_filp = NULL;
    daemon_pid = 0;
    policy_shutting_down = false;
    policy_timeout_ms = 2000;
    policy_fail_closed = false;
    APRINTK(KERN_INFO "armadillo: policy engine initialized\n");
    return 0;
}

/* Drain both queues: wake every waiter with ALLOW so tasks can make progress. */
static void policy_drain_locked(void)
{
    struct armadillo_policy_request *req, *tmp;

    list_for_each_entry_safe(req, tmp, &in_flight, list) {
        list_del(&req->list);
        req->verdict = ARMADILLO_VERDICT_ALLOW;
        complete(&req->done);
    }
    list_for_each_entry_safe(req, tmp, &pending_queue, list) {
        list_del(&req->list);
        req->verdict = ARMADILLO_VERDICT_ALLOW;
        complete(&req->done);
    }
}

void armadillo_policy_shutdown(void)
{
    unsigned long flags;

    spin_lock_irqsave(&policy_lock, flags);
    policy_shutting_down = true;
    policy_drain_locked();
    daemon_filp = NULL;
    daemon_pid = 0;
    spin_unlock_irqrestore(&policy_lock, flags);
    wake_up_all(&daemon_wq);
    APRINTK(KERN_INFO "armadillo: policy engine shut down\n");
}

int armadillo_policy_daemon_attach(struct file *filp)
{
    unsigned long flags;
    int ret = 0;

    spin_lock_irqsave(&policy_lock, flags);
    if (policy_shutting_down) {
        ret = -ESHUTDOWN;
    } else if (daemon_filp && daemon_filp != filp) {
        ret = -EBUSY;
    } else {
        daemon_filp = filp;
        daemon_pid = task_tgid_nr(current);
    }
    spin_unlock_irqrestore(&policy_lock, flags);
    if (!ret)
        APRINTK(KERN_INFO "armadillo: policy daemon attached pid=%d\n", daemon_pid);
    return ret;
}

void armadillo_policy_daemon_detach(struct file *filp)
{
    unsigned long flags;

    spin_lock_irqsave(&policy_lock, flags);
    if (daemon_filp != filp) {
        spin_unlock_irqrestore(&policy_lock, flags);
        return;
    }
    daemon_filp = NULL;
    daemon_pid = 0;
    policy_drain_locked();
    spin_unlock_irqrestore(&policy_lock, flags);
    wake_up_all(&daemon_wq);
    APRINTK(KERN_INFO "armadillo: policy daemon detached\n");
}

static enum armadillo_verdict fail_mode_verdict(void)
{
    return policy_fail_closed ? ARMADILLO_VERDICT_DENY : ARMADILLO_VERDICT_ALLOW;
}

enum armadillo_verdict armadillo_policy_ask_execve(const char *path)
{
    struct armadillo_policy_request *req;
    unsigned long flags;
    enum armadillo_verdict result;
    u32 timeout_ms;
    long wait_ret;

    /* Recursion guard: the daemon itself (and kernel threads) must always be allowed. */
    if (!current || (current->flags & PF_KTHREAD))
        return ARMADILLO_VERDICT_ALLOW;

    spin_lock_irqsave(&policy_lock, flags);
    if (policy_shutting_down || !daemon_filp) {
        result = fail_mode_verdict();
        spin_unlock_irqrestore(&policy_lock, flags);
        return result;
    }
    if (daemon_pid && task_tgid_nr(current) == daemon_pid) {
        spin_unlock_irqrestore(&policy_lock, flags);
        return ARMADILLO_VERDICT_ALLOW;
    }
    timeout_ms = policy_timeout_ms;
    spin_unlock_irqrestore(&policy_lock, flags);

    req = kzalloc(sizeof(*req), GFP_KERNEL);
    if (!req)
        return fail_mode_verdict();

    INIT_LIST_HEAD(&req->list);
    init_completion(&req->done);
    req->id      = (u64)atomic64_inc_return(&next_id);
    req->pid     = task_tgid_nr(current);
    req->ppid    = current->real_parent ? task_tgid_nr(current->real_parent) : 0;
    req->uid     = from_kuid(&init_user_ns, current_uid());
    req->verdict = ARMADILLO_VERDICT_PENDING;
    get_task_comm(req->comm, current);
    if (path)
        strscpy(req->path, path, sizeof(req->path));

    spin_lock_irqsave(&policy_lock, flags);
    if (policy_shutting_down || !daemon_filp) {
        spin_unlock_irqrestore(&policy_lock, flags);
        kfree(req);
        return fail_mode_verdict();
    }
    list_add_tail(&req->list, &pending_queue);
    spin_unlock_irqrestore(&policy_lock, flags);
    wake_up(&daemon_wq);

    if (timeout_ms == 0) {
        int rc = wait_for_completion_interruptible(&req->done);
        if (rc == -ERESTARTSYS) {
            /* Interrupted (fatal signal) — don't wedge the dying task. */
            spin_lock_irqsave(&policy_lock, flags);
            req->abandoned = true;
            list_del_init(&req->list);
            spin_unlock_irqrestore(&policy_lock, flags);
            kfree(req);
            return ARMADILLO_VERDICT_ALLOW;
        }
    } else {
        wait_ret = wait_for_completion_interruptible_timeout(
            &req->done, msecs_to_jiffies(timeout_ms));
        if (wait_ret == 0) {
            /* Timeout — mark abandoned so a late reply won't double-free. */
            spin_lock_irqsave(&policy_lock, flags);
            req->abandoned = true;
            list_del_init(&req->list);
            result = fail_mode_verdict();
            spin_unlock_irqrestore(&policy_lock, flags);
            kfree(req);
            return result;
        }
        if (wait_ret < 0) {
            /* -ERESTARTSYS */
            spin_lock_irqsave(&policy_lock, flags);
            req->abandoned = true;
            list_del_init(&req->list);
            spin_unlock_irqrestore(&policy_lock, flags);
            kfree(req);
            return ARMADILLO_VERDICT_ALLOW;
        }
    }

    /* Verdict delivered — request is already unlinked by reply/drain path. */
    result = req->verdict;
    if (result != ARMADILLO_VERDICT_ALLOW && result != ARMADILLO_VERDICT_DENY)
        result = fail_mode_verdict();
    kfree(req);
    return result;
}

int armadillo_policy_pull(armadillo_verdict_request __user *uout)
{
    struct armadillo_policy_request *req;
    armadillo_verdict_request out;
    unsigned long flags;
    int ret;

    for (;;) {
        spin_lock_irqsave(&policy_lock, flags);
        if (policy_shutting_down) {
            spin_unlock_irqrestore(&policy_lock, flags);
            return -ESHUTDOWN;
        }
        if (!list_empty(&pending_queue)) {
            req = list_first_entry(&pending_queue,
                                   struct armadillo_policy_request, list);
            list_move_tail(&req->list, &in_flight);
            spin_unlock_irqrestore(&policy_lock, flags);
            break;
        }
        spin_unlock_irqrestore(&policy_lock, flags);

        ret = wait_event_interruptible(
            daemon_wq,
            policy_shutting_down || !list_empty(&pending_queue));
        if (ret)
            return ret; /* -ERESTARTSYS */
    }

    memset(&out, 0, sizeof(out));
    out.id   = req->id;
    out.pid  = req->pid;
    out.ppid = req->ppid;
    out.uid  = req->uid;
    memcpy(out.comm, req->comm, sizeof(out.comm));
    memcpy(out.path, req->path, sizeof(out.path));

    if (copy_to_user(uout, &out, sizeof(out))) {
        /* Put it back so no request is lost. */
        spin_lock_irqsave(&policy_lock, flags);
        list_move(&req->list, &pending_queue);
        spin_unlock_irqrestore(&policy_lock, flags);
        wake_up(&daemon_wq);
        return -EFAULT;
    }
    return 0;
}

int armadillo_policy_reply(const armadillo_verdict_reply __user *uin)
{
    armadillo_verdict_reply in;
    struct armadillo_policy_request *req, *match = NULL;
    unsigned long flags;
    enum armadillo_verdict v;

    if (copy_from_user(&in, uin, sizeof(in)))
        return -EFAULT;

    v = (in.verdict == ARMADILLO_VERDICT_DENY) ? ARMADILLO_VERDICT_DENY
                                               : ARMADILLO_VERDICT_ALLOW;

    spin_lock_irqsave(&policy_lock, flags);
    list_for_each_entry(req, &in_flight, list) {
        if (req->id == in.id) {
            match = req;
            break;
        }
    }
    if (match) {
        list_del(&match->list);
        match->verdict = v;
        complete(&match->done);
    }
    spin_unlock_irqrestore(&policy_lock, flags);

    return match ? 0 : -ENOENT;
}

int armadillo_policy_set_config(const armadillo_policy_config __user *uin)
{
    armadillo_policy_config cfg;
    unsigned long flags;

    if (copy_from_user(&cfg, uin, sizeof(cfg)))
        return -EFAULT;

    spin_lock_irqsave(&policy_lock, flags);
    policy_timeout_ms = cfg.timeout_ms;
    policy_fail_closed = !!cfg.fail_closed;
    spin_unlock_irqrestore(&policy_lock, flags);

    APRINTK(KERN_INFO "armadillo: policy config: timeout_ms=%u fail_closed=%u\n",
            cfg.timeout_ms, !!cfg.fail_closed);
    return 0;
}

int armadillo_policy_get_config(armadillo_policy_config __user *uout)
{
    armadillo_policy_config cfg;
    unsigned long flags;

    spin_lock_irqsave(&policy_lock, flags);
    cfg.timeout_ms  = policy_timeout_ms;
    cfg.fail_closed = policy_fail_closed ? 1 : 0;
    spin_unlock_irqrestore(&policy_lock, flags);

    if (copy_to_user(uout, &cfg, sizeof(cfg)))
        return -EFAULT;
    return 0;
}
