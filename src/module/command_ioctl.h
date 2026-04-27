#ifndef COMMAND_IOCTL_H
#define COMMAND_IOCTL_H

#include <linux/ioctl.h>
#include <linux/types.h>
#include "defines.h"

/* guard kernel-only bits */
#ifdef __KERNEL__
#include <linux/fs.h>
long armadillo_unlocked_ioctl(struct file *filp, unsigned int ioctl_num, unsigned long ioctl_arg);
int  armadillo_device_open(struct inode *inode, struct file *filp);
int  armadillo_device_release(struct inode *inode, struct file *filp);
#endif

typedef struct armadillo_ioctl_set_pid_unkillable_struct {
    unsigned char secret [ARMADILLO_MAX_PASS_LENGTH_TERMINATED];
    unsigned int pid;
    unsigned char new_status;
} armadillo_ioctl_set_pid_unkillable;

typedef struct armadillo_ioctl_lock_struct  {
    unsigned char secret [ARMADILLO_MAX_PASS_LENGTH_TERMINATED];
} armadillo_ioctl_lock;

typedef struct armadillo_ioctl_unlock_struct  {
    unsigned char secret [ARMADILLO_MAX_PASS_LENGTH_TERMINATED];
} armadillo_ioctl_unlock;

/* Policy-mediation interface (kernel ↔ userspace daemon).
 * Layout must stay in sync with src/policyd/ioctl_defs.py.
 */
typedef struct armadillo_verdict_request_struct {
    __u64 id;
    __s32 pid;
    __s32 ppid;
    __u32 uid;
    char  comm[ARMADILLO_POLICY_COMM_LEN];
    char  path[ARMADILLO_POLICY_PATH_MAX];
} armadillo_verdict_request;

typedef struct armadillo_verdict_reply_struct {
    __u64 id;
    __u32 verdict;   /* 1 = ALLOW, 2 = DENY */
} armadillo_verdict_reply;

typedef struct armadillo_policy_config_struct {
    __u32 timeout_ms;    /* 0 = wait forever */
    __u32 fail_closed;   /* 0 = FAIL_OPEN, non-zero = FAIL_CLOSED */
} armadillo_policy_config;

#define ARMADILLO_IOCTL_MAGIC                       0x33
#define ARMADILLO_IOCTL_STATUS                      _IOR    (ARMADILLO_IOCTL_MAGIC, 0)
#define ARMADILLO_IOCTL_LOCK                        _IOW    (ARMADILLO_IOCTL_MAGIC, 1, armadillo_ioctl_lock)
#define ARMADILLO_IOCTL_UNLOCK                      _IOW    (ARMADILLO_IOCTL_MAGIC, 2, armadillo_ioctl_unlock)
#define ARMADILLO_IOCTL_SET_PID_UNKILLABLE          _IOW    (ARMADILLO_IOCTL_MAGIC, 3, unsigned int)
#define ARMADILLO_IOCTL_SET_DEBUG                   _IO     (ARMADILLO_IOCTL_MAGIC, 4)

#define ARMADILLO_IOCTL_POLICY_ATTACH               _IO     (ARMADILLO_IOCTL_MAGIC, 10)
#define ARMADILLO_IOCTL_POLICY_PULL                 _IOR    (ARMADILLO_IOCTL_MAGIC, 11, armadillo_verdict_request)
#define ARMADILLO_IOCTL_POLICY_REPLY                _IOW    (ARMADILLO_IOCTL_MAGIC, 12, armadillo_verdict_reply)
#define ARMADILLO_IOCTL_POLICY_SET_CONFIG           _IOW    (ARMADILLO_IOCTL_MAGIC, 13, armadillo_policy_config)
#define ARMADILLO_IOCTL_POLICY_GET_CONFIG           _IOR    (ARMADILLO_IOCTL_MAGIC, 14, armadillo_policy_config)

#endif

