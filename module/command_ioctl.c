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

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,0,0)
#include <linux/sched/signal.h>
#else
#include <linux/sched.h>
#endif

#include "command_ioctl.h"
#include "module.h"

long armadillo_unlocked_ioctl(struct file *filp, unsigned int ioctl_num, unsigned long ioctl_arg) {
    printk(KERN_INFO "Armadillo: Handling IOCTL");

/* Switch according to the ioctl called */ 
    switch (ioctl_num) {
        //case ARMADILLO_IOCTL_ARM:
            
        //case ARMADILLO_IOCTL_DISARM:
        //    return ARMADILLO_IOCTL_SUCCESS;
            
        case ARMADILLO_IOCTL_TOGGLE_PID_UNKILLABLE: {
            struct armadillo_ioctl_toggle_pid_unkillable * armadillo_ioctl_toggle_pid_unkillable_params;
            armadillo_ioctl_toggle_pid_unkillable_params = ( struct armadillo_ioctl_toggle_pid_unkillable *) ioctl_arg;
            printk(KERN_DEBUG "armadillo: armadillo_ioctl_toggle_pid_unkillable: pid: %d new_status: %d",armadillo_ioctl_toggle_pid_unkillable_params->pid, armadillo_ioctl_toggle_pid_unkillable_params->new_status);

            return toggle_pid_unkillable(armadillo_ioctl_toggle_pid_unkillable_params->pid, armadillo_ioctl_toggle_pid_unkillable_params->new_status);

            return ARMADILLO_IOCTL_SUCCESS;
        }
        
        default:
            printk(KERN_ALERT "armadillo: Unsupported IOCTL!!!\n");
            return -EINVAL;
    }

    return ARMADILLO_IOCTL_SUCCESS;
}

int toggle_pid_unkillable(unsigned int pid,  unsigned char new_status) {
    static struct task_struct * task = NULL;

    if(!pid) {
            printk(KERN_ALERT "armadillo: Please specify pid of the process you want to make unkillable.\n");
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
            printk(KERN_ALERT "armadillo: Error getting task_struct for pid %d\n", pid);
        return -1;
    }
    return 0;
}
