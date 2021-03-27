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
#include "hooker.h"
#include "kernfunc.h"

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,10,0)
#include <linux/sched/signal.h>
#else
#include <linux/sched.h>
#endif


 
//module_param(pid, int, 0);
//
// This function toggles the flags for process to make it killable or unkillable
// Simple stuff, takes pid and a flag as killable unkillable
//




//device class
struct class *armadillo_class;
//device_name
dev_t armadillo_dev;
static struct cdev armadillo_cdev;

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .unlocked_ioctl = armadillo_unlocked_ioctl
};


int init_module(void) {
    int ret;
    struct device *dev_ret;

    printk(KERN_INFO "armadillo: Init called\n");
    //ret = kernfunc_init();
    //if (IN_ERR(ret)) {
    ///    printk(KERN_INFO "armadillo: Unable to initialize kernfunc!\n");
    //    return ret;
    //}
    
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

//    printk (KERN_DEBUG "armadillo: scary part comes, lets hook the kernel!\n");
//    hijack_syscalls();
    printk(KERN_INFO "armadillo: Module initialized\n");

    return 0;
}

void cleanup_module(void) {
//    undo_hijack_syscalls();
    cdev_del(&armadillo_cdev);
    device_destroy(armadillo_class, armadillo_dev);
    class_destroy(armadillo_class);
    unregister_chrdev_region(armadillo_dev, 1);
    printk(KERN_INFO "Armadillo: Module cleaned and unloaded\n");
}

