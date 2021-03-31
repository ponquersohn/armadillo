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

    if (!armadillo_is_locked()) {
        armadillo_set_debug(!armadillo_get_debug());
    } else {
        APRINTK(KERN_ALERT ARMADILLO_LOCKED_MESSAGE);
        ret = -EPERM;
    }
    return ret; 
}


long armadillo_unlocked_ioctl(struct file *filp, unsigned int ioctl_num, unsigned long ioctl_arg) {
    int ret;
    APRINTK(KERN_INFO "armadillo: Handling IOCTL");

/* Switch according to the ioctl called */ 

    switch (ioctl_num) {
       
        case ARMADILLO_IOCTL_LOCK: {
            int ret;
            armadillo_ioctl_lock armadillo_ioctl_lock_params;
            APRINTK(KERN_DEBUG "armadillo: armadillo_ioctl_lock: called.");

            ret = copy_from_user(&armadillo_ioctl_lock_params, (void __user *) ioctl_arg, sizeof(armadillo_ioctl_lock));          
            // we need to check input data first!
            if(ret) {
                APRINTK(KERN_INFO "armadillo: armadillo_ioctl_lock: error getting parameter\n");
                return -EFAULT;;
            }
            
            return armadillo_lock(armadillo_ioctl_lock_params.secret);
        }

        case ARMADILLO_IOCTL_UNLOCK: {            
            int ret;
            armadillo_ioctl_unlock armadillo_ioctl_unlock_params;
            APRINTK(KERN_DEBUG "armadillo: armadillo_ioctl_unlock: called.");

            ret = copy_from_user(&armadillo_ioctl_unlock_params, (void __user *) ioctl_arg, sizeof(armadillo_ioctl_unlock));          
            // we need to check input data first!
            if(ret) {
                APRINTK(KERN_INFO "armadillo: armadillo_ioctl_unlock: error getting parameter\n");
                return -EFAULT;;
            }

            
            return armadillo_unlock(armadillo_ioctl_unlock_params.secret);
        }        
        //case ARMADILLO_IOCTL_DISARM:
        //    return ARMADILLO_IOCTL_SUCCESS;
            
        case ARMADILLO_IOCTL_SET_PID_UNKILLABLE: {
            unsigned int pid;
            unsigned char new_status;

            // int ret;
            armadillo_ioctl_set_pid_unkillable armadillo_ioctl_set_pid_unkillable_params;
            
            // APRINTK(KERN_INFO "armadillo: set_pid_unkillable: Pre ret.\n");
            ret = copy_from_user(&armadillo_ioctl_set_pid_unkillable_params, (void __user *) ioctl_arg, sizeof(armadillo_ioctl_set_pid_unkillable));
            // APRINTK(KERN_INFO "armadillo: Post ret.\n");
            
            // we need to check input data first!
            if(ret) {
                APRINTK(KERN_INFO "armadillo: set_pid_unkillable: error getting parameter\n");
                return -EFAULT;;
            }

            pid = armadillo_ioctl_set_pid_unkillable_params.pid;
            new_status = armadillo_ioctl_set_pid_unkillable_params.new_status;
          
            APRINTK(KERN_INFO "armadillo: set_pid_unkillable: pid: %u, new_status: %c\n", pid, new_status);
            ret = armadillo_set_pid_unkillable(pid, new_status);
            return ret;
        }
        
        case ARMADILLO_IOCTL_SET_DEBUG: {
            
            ret = set_debug();
            APRINTK(KERN_INFO "armadillo: set_debug: Turning debug off.");
            if (ret == 0) {
                APRINTK(KERN_INFO "armadillo: set_debug: Turned debug on.");
            }
            return ret;
        }

        default:
            APRINTK(KERN_ALERT "armadillo: Unsupported IOCTL!!!\n");
            return -EINVAL;
    }

    return ARMADILLO_IOCTL_SUCCESS;
}
