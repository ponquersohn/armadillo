#ifndef COMMAND_IOCTL_H
#define COMMAND_IOCTL_H

#include <linux/ioctl.h>
//#include <linux/device.h>

#define ARMADILLO_CLASS "armadillo"
#define ARMADILLO_CLASS_DRIVER "armadillo_cdrv"
#define ARMADILLO_DEVICE_FILE_NAME "/dev/armadillo_cdrv"
#define ARMADILLO_IOCTL_SUCCESS 0
#define ARMADILLO_MAX_PASSWORD_LENGTH 20



long armadillo_unlocked_ioctl(struct file *filp, unsigned int ioctl_num, unsigned long ioctl_arg);
int toggle_pid_unkillable(unsigned int pid,  unsigned char new_status);

 struct armadillo_ioctl_toggle_pid_unkillable  {
    unsigned char secret [ARMADILLO_MAX_PASSWORD_LENGTH];
    unsigned int pid;
    unsigned char new_status;

};

#define ARMADILLO_IOCTL_MAGIC                       0x33
#define ARMADILLO_IOCTL_ARM                         _IO     (ARMADILLO_IOCTL_MAGIC, 0)
#define ARMADILLO_IOCTL_DISARM                      _IO     (ARMADILLO_IOCTL_MAGIC, 1)
#define ARMADILLO_IOCTL_TOGGLE_PID_UNKILLABLE       _IOW    (ARMADILLO_IOCTL_MAGIC, 2, struct armadillo_ioctl_toggle_pid_unkillable)

#endif

