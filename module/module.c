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
#include "command_ioctl.h"
#include "ftrace_hooker.h"

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,10,0)
#include <linux/sched/signal.h>
#else
#include <linux/sched.h>
#endif

#define INIT_WITH_DEBUG 1
#define INSTALL_HOOKS_ON_INIT 1


armadillo_status_type armadillo_status;
struct mutex armadillo_status_mutex;

//device class
struct class *armadillo_class;
//device_name
dev_t armadillo_dev;
static struct cdev armadillo_cdev;

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .unlocked_ioctl = armadillo_unlocked_ioctl
};

int armadillo_printk_nolock(const char *fmt, ...) {
    if (armadillo_status.debug) {
        va_list args;
        int r;

        va_start(args, fmt);
        r = vprintk(fmt, args);
        va_end(args);

        return r;
    }
    return 0;
}

int armadillo_printk(const char *fmt, ...) {
    va_list args;
    int r;

    va_start(args, fmt);
    ARMADILLO_LOCK_STATUS_MUTEX;        
    r = armadillo_printk_nolock(fmt, args);
    ARMADILLO_UNLOCK_STATUS_MUTEX;    

    va_end(args);
    
    return r;
}

bool armadillo_is_locked(void) {
    bool ret = false;
    ARMADILLO_LOCK_STATUS_MUTEX;    
    ret = armadillo_status.locked;
    ARMADILLO_UNLOCK_STATUS_MUTEX; 
    return ret;   
}



int armadillo_set_debug(bool debug) {
    ARMADILLO_LOCK_STATUS_MUTEX;    
    armadillo_status.debug = debug;
    ARMADILLO_UNLOCK_STATUS_MUTEX; 
    return 0;   
}


bool armadillo_get_debug(void) {
    bool ret;
    ARMADILLO_LOCK_STATUS_MUTEX;    
    ret = armadillo_status.debug;
    ARMADILLO_UNLOCK_STATUS_MUTEX;
    return ret;
}

int armadillo_lock(char * secret) {
    int ret = 0;
    APRINTK_NOLOCK(KERN_ALERT "armadillo: secret: %s...\n", secret);
    ARMADILLO_LOCK_STATUS_MUTEX;
    if (!armadillo_status.locked) {
        APRINTK_NOLOCK(KERN_ALERT "armadillo: Locking...\n");
        // used to prevent unloading!!
        armadillo_status.locked = true;
        if(!try_module_get(THIS_MODULE)) {
            APRINTK_NOLOCK(KERN_ALERT "armadillo: Couldnt lock unloading.\n");
        }
        ret = 0;
    } else {
        APRINTK_NOLOCK(KERN_ALERT "armadillo: Unable to lock as already locked.\n");
        ret = -1;
    }
    ARMADILLO_UNLOCK_STATUS_MUTEX;
    return ret;
}

int armadillo_unlock(char * secret) {
    int ret = 0;
    ARMADILLO_LOCK_STATUS_MUTEX;
    if (armadillo_status.locked) {
        APRINTK_NOLOCK(KERN_ALERT "armadillo: Unlocking...\n");
        armadillo_status.locked = false;
        module_put(THIS_MODULE);        
        ret = 0;
    } else {
        APRINTK_NOLOCK(KERN_ALERT "armadillo: Unable to unlock as already unlocked.\n");
        ret = -1;
    }
    mutex_unlock(&armadillo_status_mutex);
    return ret;
}

int armadillo_set_pid_unkillable(unsigned int pid,  unsigned char new_status) {
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


int init_module(void) {
    int ret;
    struct device *dev_ret;

    #if INIT_WITH_DEBUG == 1
        printk(KERN_INFO "armadillo: Init called\n");
    #endif

    memset(armadillo_status.obfuscated_password,0, ARMADILLO_MAX_PASS_LENGTH * sizeof(char));
    armadillo_status.locked = false;
    armadillo_status.debug = INIT_WITH_DEBUG;
    APRINTK(KERN_INFO "armadillo: status structure initiated.\n");
    
    mutex_init(&armadillo_status_mutex);
    
    APRINTK(KERN_INFO "armadillo: mutex initiated.\n");

    
    if ((ret = alloc_chrdev_region(&armadillo_dev, 0, 1, "Armadillo")) < 0) {
        return ret;
    }
 
    if (IS_ERR(armadillo_class = class_create(THIS_MODULE, ARMADILLO_CLASS))) {
        unregister_chrdev_region(armadillo_dev, 1);
        return PTR_ERR(armadillo_class);
    } 
    
    if (IS_ERR(dev_ret = device_create(armadillo_class, NULL, armadillo_dev, NULL, ARMADILLO_CLASS_DRIVER))) {
        class_destroy(armadillo_class);
        unregister_chrdev_region(armadillo_dev, 1);
        return PTR_ERR(dev_ret);
    }
    
    cdev_init(&armadillo_cdev, &fops);
    
    if ((ret = cdev_add(&armadillo_cdev, armadillo_dev, 1)) < 0) {
        device_destroy(armadillo_class, armadillo_dev);
        class_destroy(armadillo_class);
        unregister_chrdev_region(armadillo_dev, 1);
        return ret;
    }

//    APRINTK (KERN_DEBUG "armadillo: scary part comes, lets hook the kernel!\n");

    #ifdef INSTALL_HOOKS_ON_INIT
	ret = fh_install_hooks_all();
	if (ret)
		//@ToDo: a proper way of deinit... we need to destroy the device first... right??? :)
        return ret;
    #endif

    APRINTK(KERN_INFO "armadillo: Module initialized\n");

    return 0;
}

void cleanup_module(void) {
    #ifdef INSTALL_HOOKS_ON_INIT
    fh_remove_hooks_all();
    #endif
    cdev_del(&armadillo_cdev);
    device_destroy(armadillo_class, armadillo_dev);
    class_destroy(armadillo_class);
    unregister_chrdev_region(armadillo_dev, 1);
    APRINTK(KERN_INFO "Armadillo: Module cleaned and unloaded\n");
}

