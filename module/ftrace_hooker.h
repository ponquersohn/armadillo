#ifndef FTRACE_HOOKER_H
#define FTRACE_HOOKER_H

#include <linux/ftrace.h>
#include <linux/kallsyms.h>
#include <linux/kernel.h>
#include <linux/linkage.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/version.h>
#include <linux/kprobes.h>


#if LINUX_VERSION_CODE < KERNEL_VERSION(5,11,0)
#define FTRACE_OPS_FL_RECURSION FTRACE_OPS_FL_RECURSION_SAFE
#endif

struct ftrace_hook {
	const char *name;
	void *function;
	void *original;

	unsigned long address;
	struct ftrace_ops ops;
};

void fh_remove_hooks(struct ftrace_hook *hooks, size_t count);
void fh_remove_hooks_all(void);

int fh_install_hooks(struct ftrace_hook *hooks, size_t count);
int fh_install_hooks_all(void);


#endif