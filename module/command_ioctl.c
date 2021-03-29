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
#include "module.h"

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,10,0)
#include <linux/sched/signal.h>
#else
#include <linux/sched.h>
#endif

#include "command_ioctl.h"
//#include "module.h"





int toggle_pid_unkillable(unsigned int pid,  unsigned char new_status) {
    static struct task_struct * task = NULL;

    if(!pid) {
            APRINTK(KERN_ALERT "armadillo: Please specify pid of the process you want to make unkillable.\n");
        return -1;
      }
      task = pid_task(find_get_pid(pid), PIDTYPE_PID);
    if(task) {
            if (new_status) {
                task->signal->flags = task->signal->flags | SIGNAL_UNKILLABLE;
            } else {
                task->signal->flags = task->signal->flags & ~ SIGNAL_UNKILLABLE;
            }
      } else {
            APRINTK(KERN_ALERT "armadillo: Error getting task_struct for pid %d\n", pid);
        return -1;
    }
    return 0;
}

int toggle_debug(void) {
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
            
        case ARMADILLO_IOCTL_TOGGLE_PID_UNKILLABLE: {
            struct armadillo_ioctl_toggle_pid_unkillable * armadillo_ioctl_toggle_pid_unkillable_params;
            armadillo_ioctl_toggle_pid_unkillable_params = ( struct armadillo_ioctl_toggle_pid_unkillable *) ioctl_arg;

            APRINTK(KERN_DEBUG "armadillo: armadillo_ioctl_toggle_pid_unkillable: pid: %d new_status: %d",armadillo_ioctl_toggle_pid_unkillable_params->pid, armadillo_ioctl_toggle_pid_unkillable_params->new_status);

            return toggle_pid_unkillable(armadillo_ioctl_toggle_pid_unkillable_params->pid, armadillo_ioctl_toggle_pid_unkillable_params->new_status);
        }
        
        case ARMADILLO_IOCTL_TOGGLE_DEBUG: {
            
            ret = toggle_debug();
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
