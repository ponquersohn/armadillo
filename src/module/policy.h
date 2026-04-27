#ifndef A_POLICY_H
#define A_POLICY_H

#include <linux/types.h>
#include "command_ioctl.h"

enum armadillo_verdict {
    ARMADILLO_VERDICT_PENDING = 0,
    ARMADILLO_VERDICT_ALLOW   = 1,
    ARMADILLO_VERDICT_DENY    = 2,
};

int  armadillo_policy_init(void);
void armadillo_policy_shutdown(void);

/* Daemon lifecycle — called from device open/release fops. */
int  armadillo_policy_daemon_attach(struct file *filp);
void armadillo_policy_daemon_detach(struct file *filp);

/* Called from the execve/execveat ftrace hook.
 * Blocks the caller until a verdict arrives or timeout/fail-mode applies.
 * Returns ARMADILLO_VERDICT_ALLOW or ARMADILLO_VERDICT_DENY.
 */
enum armadillo_verdict armadillo_policy_ask_execve(const char *path);

/* ioctl glue — user-pointer interface. */
int armadillo_policy_pull(armadillo_verdict_request __user *uout);
int armadillo_policy_reply(const armadillo_verdict_reply __user *uin);
int armadillo_policy_set_config(const armadillo_policy_config __user *uin);
int armadillo_policy_get_config(armadillo_policy_config __user *uout);

#endif
