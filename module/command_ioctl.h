#ifndef COMMAND_IOCTL_H
#define COMMAND_IOCTL_H

#include <linux/ioctl.h>
#include <linux/fs.h>

#define ARMADILLO_CLASS "armadillo"
#define ARMADILLO_CLASS_DRIVER "armadillo_cdrv"
#define ARMADILLO_DEVICE_FILE_NAME "/dev/armadillo_cdrv"
#define ARMADILLO_IOCTL_SUCCESS 0
#define ARMADILLO_MAX_PASSWORD_LENGTH 20

#define ARMADILLO_LOCKED_MESSAGE "LOCKED... You shall not pass!!!"

//if true then we are in the KBUILD process
#ifdef LINUX_VERSION_CODE
long armadillo_unlocked_ioctl(struct file *filp, unsigned int ioctl_num, unsigned long ioctl_arg);

#endif

typedef struct {
    unsigned char secret [ARMADILLO_MAX_PASSWORD_LENGTH];
    unsigned int pid;
    unsigned char new_status;
} armadillo_ioctl_set_pid_unkillable;

struct armadillo_ioctl_lock  {
    unsigned char secret [ARMADILLO_MAX_PASSWORD_LENGTH];
};

struct armadillo_ioctl_unlock  {
    unsigned char secret [ARMADILLO_MAX_PASSWORD_LENGTH];
};


#define ARMADILLO_IOCTL_MAGIC                       0x33
#define ARMADILLO_IOCTL_STATUS                      _IOR    (ARMADILLO_IOCTL_MAGIC, 0)
#define ARMADILLO_IOCTL_LOCK                        _IOW    (ARMADILLO_IOCTL_MAGIC, 1, struct armadillo_ioctl_lock)
#define ARMADILLO_IOCTL_UNLOCK                      _IOW    (ARMADILLO_IOCTL_MAGIC, 2, struct armadillo_ioctl_unlock)
#define ARMADILLO_IOCTL_SET_PID_UNKILLABLE          _IOW    (ARMADILLO_IOCTL_MAGIC, 3, armadillo_ioctl_set_pid_unkillable)
#define ARMADILLO_IOCTL_SET_DEBUG                   _IO     (ARMADILLO_IOCTL_MAGIC, 4)

#endif

