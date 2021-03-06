#ifndef A_MODULE_H
#define A_MODULE_H

//I DONT CARE ABOUT KERNELS BELOW 3.0.0... BECAUSE I DONT CARE
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 0, 0)
#error "Too low kernel version" LINUX_VERSION_CODE
#endif

#include <linux/module.h>
#include <linux/moduleparam.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lech Lachowicz");

#define MODULE_NAME "armadillo"
#define PKPRE "[" MODULE_NAME "] "

// this spinlock will protect whole configuration
//static DEFINE_SPINLOCK(module_lock);

// runtime module configuration

//bool armadillo_armed = 0;


#endif