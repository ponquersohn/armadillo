#ifndef HOOKER_H
#define HOOKER_H

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/file.h>
#include <linux/mman.h>
#include <linux/binfmts.h>
#include <linux/version.h>
#include <linux/utsname.h>
#include <linux/kallsyms.h>
#include <linux/dcache.h>
#include <linux/fs.h>
#include <linux/jiffies.h>
#include <linux/sysctl.h>
#include <linux/err.h>
#include <linux/namei.h>
#include <linux/fs_struct.h>
#include <linux/mount.h>

#include <asm/uaccess.h>
#include <asm/insn.h>
#include "module.h"


// control kernel hooking
#define OP_JMP_SIZE 5
#define IN_ERR(x) (x < 0)


struct kernsym {
	void *addr; // orig addr
	void *end_addr;
	unsigned long size;
	char *name;
	bool name_alloc; // whether or not we alloc'd memory for char *name
	u8 orig_start_bytes[OP_JMP_SIZE];
	void *new_addr;
	unsigned long new_size;
	bool found;
	bool hijacked;
	void *run;
};



int find_symbol_address(struct kernsym *, const char *);
#define armadillo_lookup_address(address, level) lookup_address(address, level);

// actually useful functions here. Those should be called from init and destroy
void hijack_syscalls(void);
void undo_hijack_syscalls(void);
int symbol_hijack(struct kernsym *sym, const char *symbol_name, unsigned long *code);
void symbol_restore(struct kernsym *sym);


#endif