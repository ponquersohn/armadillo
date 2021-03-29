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


#define ARMADILLO_MAX_PASS_LENGTH 41
typedef struct  {
    bool locked;
    bool debug; 
    bool obfuscated_password[ARMADILLO_MAX_PASS_LENGTH];
} armadillo_status_type;

extern armadillo_status_type armadillo_status;
extern struct mutex armadillo_status_mutex;

#define ARMADILLO_LOCK_STATUS_MUTEX mutex_lock(&armadillo_status_mutex)
#define ARMADILLO_UNLOCK_STATUS_MUTEX mutex_unlock(&armadillo_status_mutex)


int armadillo_printk(const char *fmt, ...);
int armadillo_printk_nolock(const char *fmt, ...);
#define APRINTK armadillo_printk
#define APRINTK_NOLOCK armadillo_printk_nolock


bool armadillo_is_debug(void);
bool armadillo_is_locked(void);

int toggle_pid_unkillable(unsigned int pid,  unsigned char new_status);
int armadillo_lock(char * secret);
int armadillo_unlock(char * secret);

// this spinlock will protect whole configuration
//static DEFINE_SPINLOCK(module_lock);

// runtime module configuration

//bool armadillo_armed = 0;


#endif