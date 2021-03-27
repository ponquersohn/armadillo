#include "hooker.h"
#include "kernfunc.h"
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

#define OP_JMP_REL32	0xe9
#define OP_CALL_REL32	0xe8
#ifndef CONFIG_X86_64
// i dont wont to support 32bit 
#error "I dont support x86 32bit architecture... because no."
#endif
    
    

/* here start all the hooks. This is the main part!!!
 * more comment needed
*/

// lets declare the structs to hold hooking info
struct kernsym sym_security_bprm_check;
struct kernsym sym_sys_init_module;

// this struct will be used to configure and then hold all the hooks
struct symhook {
	char *name;
	struct kernsym *sym;
	unsigned long *func;
};


// hook do_execve... for newer kernels thats security_bprm_check
int armadillo_security_bprm_check(struct linux_binprm *bprm) {
    // run should contain the pointer to original syscall
	int (*run)(struct linux_binprm *) = sym_security_bprm_check.run;
	int ret = 0;
	printk (KERN_DEBUG "armadillo: inside hooked sys_init_module\n");
    //call original syscall
    return -EFAULT;
    ret = run(bprm);
    

	return ret;
}


long armadillo_sys_init_module(void __user *umod, unsigned long len, const char __user *uargs) {
    int (*run)(void __user *, unsigned long, const char __user *) = sym_sys_init_module.run;
    int ret = 0;
    printk (KERN_DEBUG "armadillo: inside hooked sys_init_module\n");
    ret = run(umod, len, uargs);
    
    return ret;
}    

struct symhook calls_to_hook[] = {
	{"do_execve",       &sym_security_bprm_check,   (unsigned long *)armadillo_security_bprm_check},
	{"init_module",     &sym_sys_init_module,       (unsigned long *)armadillo_sys_init_module},
};
    
/*


NOW THE HOOKING MAGIC COMES


*/    
    
    
    
#define CODE_ADDR_FROM_OFFSET(insn_addr, insn_len, offset) \
	(void*)((s64)(insn_addr) + (s64)(insn_len) + (s64)(s32)(offset))

#define CODE_OFFSET_FROM_ADDR(insn_addr, insn_len, dest_addr) \
	(u32)(dest_addr - (insn_addr + (u32)insn_len))

void copy_and_fixup_insn(struct insn *src_insn, void *dest,
	const struct kernsym *func) {

	u32 *to_fixup;
	unsigned long addr;
	BUG_ON(src_insn->length == 0);
	
	memcpy((void *)dest, (const void *)src_insn->kaddr, 
		src_insn->length);
	
	if (src_insn->opcode.bytes[0] == OP_CALL_REL32 ||
	    src_insn->opcode.bytes[0] == OP_JMP_REL32) {
			
		addr = (unsigned long)CODE_ADDR_FROM_OFFSET(
			src_insn->kaddr,
			src_insn->length, 
			src_insn->immediate.value);
		
		if (addr >= (unsigned long)func->addr && 
		    addr < (unsigned long)func->addr + func->size)
			return;
		
		to_fixup = (u32 *)((unsigned long)dest + 
			insn_offset_immediate(src_insn));
		*to_fixup = CODE_OFFSET_FROM_ADDR(dest, src_insn->length,
			(void *)addr);
		return;
	}

	if (!armadillo_insn_rip_relative(src_insn))
		return;
		
	addr = (unsigned long)CODE_ADDR_FROM_OFFSET(
		src_insn->kaddr,
		src_insn->length, 
		src_insn->displacement.value);
	
	if (addr >= (unsigned long)func->addr && 
	    addr < (unsigned long)func->addr + func->size)
		return;
	
	to_fixup = (u32 *)((unsigned long)dest + 
		insn_offset_displacement(src_insn));
	*to_fixup = CODE_OFFSET_FROM_ADDR(dest, src_insn->length,
		(void *)addr);
	return;
}




// functions to set/unset write at the page that represents the given address
// this previously was code that disabled the write-protect bit of cr0, but
// this is much cleaner



static inline void set_addr_rw(unsigned long addr, bool *flag) {

	unsigned int level;
	pte_t *pte;

	*flag = true;

	pte = armadillo_lookup_address(addr, &level);

	if (pte->pte & _PAGE_RW) *flag = false;
	else pte->pte |= _PAGE_RW;

}

static inline void set_addr_ro(unsigned long addr, bool flag) {

	unsigned int level;
	pte_t *pte;

	// only set back to readonly if it was readonly before
	if (flag) {
		pte = armadillo_lookup_address(addr, &level);

		pte->pte = pte->pte &~_PAGE_RW;
	}

}

static int find_symbol_callback(struct kernsym *sym, const char *name, struct module *mod,
	unsigned long addr) {

	if (sym->found) {
		sym->end_addr = (unsigned long *)addr;
		return 1;
	}

	// this symbol was found. the next callback will be the address of the next symbol
	if (name && sym->name && !strcmp(name, sym->name)) {
		sym->addr = (unsigned long *)addr;
		sym->found = true;
	}

	return 0;
}

// find this symbol

int find_symbol_address(struct kernsym *sym, const char *symbol_name) {

	int ret;

	sym->name = (char *)symbol_name;
	sym->found = 0;
    printk (KERN_DEBUG "armadillo: in find_symbol_address for symbol: %s",symbol_name); 
	ret = kallsyms_on_each_symbol((void *)find_symbol_callback, sym);
    
    if (!ret) {
        printk (KERN_DEBUG "armadillo: find_symbol_address name: %s NOT FOUND",symbol_name); 
		return -EFAULT;
    }
	sym->size = sym->end_addr - sym->addr;
	sym->new_size = sym->size;
	sym->run = sym->addr; 
	printk (KERN_DEBUG "armadillo: found symbol name: %s adress: %p", symbol_name, sym->run);

	return 0;
}

static int find_address_callback(struct kernsym *sym, const char *name, struct module *mod,
	unsigned long addr) {

	if (sym->found) {
		sym->end_addr = (unsigned long *)addr;
		return 1;
	}

	// this address was found. the next callback will be the address of the next symbol
	if (addr && (unsigned long) sym->addr == addr) {
		sym->name = armadillo_malloc(strlen(name)+1);
		strncpy(sym->name, name, strlen(sym->name)+1);
		sym->name_alloc = true;
		sym->found = true;
	}

	return 0;
}

int find_address_symbol(struct kernsym *sym, unsigned long addr) {

	int ret;

	sym->found = 0;
	sym->addr = (unsigned long *)addr;

	ret = kallsyms_on_each_symbol((void *)find_address_callback, sym);

	if (!ret)
		return -EFAULT;

	sym->size = sym->end_addr - sym->addr;
	sym->new_size = sym->size;
	sym->run = sym->addr;

	return 0;
}


int symbol_hijack(struct kernsym *sym, const char *symbol_name, unsigned long *code) {

	int ret;
	unsigned long orig_addr;
	unsigned long dest_addr;
	unsigned long end_addr;
	u32 *poffset;
	struct insn insn;
	bool pte_ro;
	
    printk (KERN_DEBUG "armadillo: Hijacking symbol: %s", symbol_name);
	ret = find_symbol_address(sym, symbol_name);
    
	if (IN_ERR(ret))
		return ret;

	if (*(u8 *)sym->addr == OP_JMP_REL32) {
		printk(PKPRE "error: %s already appears to be hijacked\n", symbol_name);
		return -EFAULT;
	}

	sym->new_addr = armadillo_malloc(sym->size);

	if (sym->new_addr == NULL) {
		printk(PKPRE
			"Failed to allocate buffer of size %lu for %s\n",
			sym->size, sym->name);
		return -ENOMEM;
	}

	memset(sym->new_addr, 0, (size_t)sym->size);

	if (sym->size < OP_JMP_SIZE) {
		ret = -EFAULT;
		goto out_error;
	}
	
	orig_addr = (unsigned long)sym->addr;
	dest_addr = (unsigned long)sym->new_addr;

	printk (KERN_DEBUG "armadillo: orig: %lu dest: %lu", orig_addr, dest_addr);
	if ((orig_addr == 0 )||(dest_addr== 0 ))  {
		ret = -EFAULT;
		goto out_error;
	}

	end_addr = orig_addr + sym->size;
	while (end_addr > orig_addr && *(u8 *)(end_addr - 1) == '\0')
		--end_addr;
	
	if (orig_addr == end_addr) {
		printk(PKPRE
			"A spurious symbol \"%s\" (address: %p) seems to contain only zeros\n",
			sym->name,
			sym->addr);
		ret = -EILSEQ;
		goto out_error;
	}
	
	while (orig_addr < end_addr) {
		armadillo_insn_init(&insn, (void *)orig_addr); 
		armadillo_insn_get_length(&insn);
		if (insn.length == 0) {
			printk(PKPRE
				"Failed to decode instruction at %p (%s+0x%lx)\n",
				(const void *)orig_addr,
				sym->name,
				orig_addr - (unsigned long)sym->addr);
			ret = -EILSEQ;
			goto out_error;
		}
		
		copy_and_fixup_insn(&insn, (void *)dest_addr, sym);
		
		orig_addr += insn.length;
		dest_addr += insn.length;
	}
	
	sym->new_size = dest_addr - (unsigned long)sym->new_addr;

	sym->run = sym->new_addr;

	set_addr_rw((unsigned long) sym->addr, &pte_ro);

	memcpy(&sym->orig_start_bytes[0], sym->addr, OP_JMP_SIZE);

	*(u8 *)sym->addr = OP_JMP_REL32;
	poffset = (u32 *)((unsigned long)sym->addr + 1);
	*poffset = CODE_OFFSET_FROM_ADDR((unsigned long)sym->addr, 
		OP_JMP_SIZE, (unsigned long)code);

	set_addr_ro((unsigned long) sym->addr, pte_ro);

	sym->hijacked = true;

	return 0;

out_error:
	armadillo_malloc_free(sym->new_addr);

	return ret;
}

void symbol_restore(struct kernsym *sym) {

	bool pte_ro;

	if (sym->hijacked) {

		set_addr_rw((unsigned long) sym->addr, &pte_ro);

		memcpy(sym->addr, &sym->orig_start_bytes[0], OP_JMP_SIZE);

		set_addr_ro((unsigned long) sym->addr, pte_ro);

		sym->hijacked = false;

		armadillo_malloc_free(sym->new_addr);

	}

	if (sym->name_alloc) {
		armadillo_malloc_free(sym->name);
		sym->name_alloc = false;
	}

	return;
}
void printfail(const char *name) {
	printk(PKPRE "warning: unable to implement protections for %s\n", name);
}

void hijack_syscalls(void) {

	int ret, i;

	for (i = 0; i < ARRAY_SIZE(calls_to_hook); i++) {
		ret = symbol_hijack(calls_to_hook[i].sym, calls_to_hook[i].name, calls_to_hook[i].func);

		if (IN_ERR(ret))
			printfail(calls_to_hook[i].name);
	}

}

void undo_hijack_syscalls(void) {
	int i;

	for (i = 0; i < ARRAY_SIZE(calls_to_hook); i++)
		symbol_restore(calls_to_hook[i].sym);
}




