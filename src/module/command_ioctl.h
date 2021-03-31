#ifndef COMMAND_IOCTL_H
#define COMMAND_IOCTL_H

#include <linux/ioctl.h>
#include <linux/fs.h>
#include "defines.h"

//if true then we are in the KBUILD process
#ifdef LINUX_VERSION_CODE
long armadillo_unlocked_ioctl(struct file *filp, unsigned int ioctl_num, unsigned long ioctl_arg);

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


#define ARMADILLO_IOCTL_MAGIC                       0x33
#define ARMADILLO_IOCTL_STATUS                      _IOR    (ARMADILLO_IOCTL_MAGIC, 0)
#define ARMADILLO_IOCTL_LOCK                        _IOW    (ARMADILLO_IOCTL_MAGIC, 1, armadillo_ioctl_lock)
#define ARMADILLO_IOCTL_UNLOCK                      _IOW    (ARMADILLO_IOCTL_MAGIC, 2, armadillo_ioctl_unlock)
#define ARMADILLO_IOCTL_SET_PID_UNKILLABLE          _IOW    (ARMADILLO_IOCTL_MAGIC, 3, unsigned int)
#define ARMADILLO_IOCTL_SET_DEBUG                   _IO     (ARMADILLO_IOCTL_MAGIC, 4)

#endif

