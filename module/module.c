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

bool armadillo_is_debug(void) {
    return true;
}


int armadillo_printk(const char *fmt, ...) {
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
    //fh_remove_hooks_all();
    cdev_del(&armadillo_cdev);
    device_destroy(armadillo_class, armadillo_dev);
    class_destroy(armadillo_class);
    unregister_chrdev_region(armadillo_dev, 1);
    APRINTK(KERN_INFO "Armadillo: Module cleaned and unloaded\n");
}

