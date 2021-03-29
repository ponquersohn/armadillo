#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/stat.h>
#include <linux/pid.h>
#include <linux/version.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include "module.h"


#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,10,0)
#include <linux/sched/signal.h>
#else
#include <linux/sched.h>
#endif

#include "command_ioctl.h"



int set_debug(void) {
    int ret = 0;
    mutex_lock(&armadillo_status_mutex);
    if (!armadillo_status.locked) {
        armadillo_status.debug = !armadillo_status.debug;
    } else {
        APRINTK(KERN_ALERT ARMADILLO_LOCKED_MESSAGE);
        ret = -EPERM;
    }
    mutex_unlock(&armadillo_status_mutex);
    return ret; 

}


long armadillo_unlocked_ioctl(struct file *filp, unsigned int ioctl_num, unsigned long ioctl_arg) {
    int ret;
    APRINTK(KERN_INFO "armadillo: Handling IOCTL");

/* Switch according to the ioctl called */ 

    switch (ioctl_num) {
        case ARMADILLO_IOCTL_LOCK: {
            struct armadillo_ioctl_lock * armadillo_ioctl_lock_params;
            armadillo_ioctl_lock_params = ( struct armadillo_ioctl_lock *) ioctl_arg;
            
            APRINTK(KERN_DEBUG "armadillo: armadillo_ioctl_lock called.");
            
            return armadillo_lock(armadillo_ioctl_lock_params->secret);
        }

        case ARMADILLO_IOCTL_UNLOCK: {
            struct armadillo_ioctl_unlock * armadillo_ioctl_unlock_params;
            armadillo_ioctl_unlock_params = ( struct armadillo_ioctl_unlock *) ioctl_arg;
            
            APRINTK(KERN_DEBUG "armadillo: armadillo_ioctl_unlock called.");
            
            return armadillo_unlock(armadillo_ioctl_unlock_params->secret);
        }        
        //case ARMADILLO_IOCTL_DISARM:
        //    return ARMADILLO_IOCTL_SUCCESS;
            
        case ARMADILLO_IOCTL_SET_PID_UNKILLABLE: {
            unsigned int pid;
            unsigned char new_status;
            //int ret;

            armadillo_ioctl_set_pid_unkillable * armadillo_ioctl_set_pid_unkillable_params;
            armadillo_ioctl_set_pid_unkillable a;
            armadillo_ioctl_set_pid_unkillable_params = &a;
            //armadillo_ioctl_set_pid_unkillable_params = (armadillo_ioctl_set_pid_unkillable *) ioctl_arg;
            ret = copy_from_user(&a, (void __user *) ioctl_arg, sizeof(armadillo_ioctl_set_pid_unkillable));
            //we need to check input data first!
            pid = armadillo_ioctl_set_pid_unkillable_params->pid;
            APRINTK(KERN_INFO "armadillo: set_pid_unkillable: pid: %u\n");
            return 1;
            new_status = armadillo_ioctl_set_pid_unkillable_params->new_status;

            APRINTK(KERN_DEBUG "armadillo: set_pid_unkillable: pid: %u, new_status: %c\n", pid, new_status);
            ret = armadillo_set_pid_unkillable(pid, new_status);
            return ret;
        }
        
        case ARMADILLO_IOCTL_SET_DEBUG: {
            
            ret = set_debug();
            APRINTK(KERN_DEBUG "armadillo: Turning debug off.");
            if (ret == 0) {
                APRINTK(KERN_DEBUG "armadillo: Turned debug on.");
            }
            return ret;
        }

        default:
            APRINTK(KERN_ALERT "armadillo: Unsupported IOCTL!!!\n");
            return -EINVAL;
    }

    return ARMADILLO_IOCTL_SUCCESS;
}
