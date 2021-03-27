#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0xf8cdd757, "module_layout" },
	{ 0x361c68dd, "cdev_del" },
	{ 0x71a3afd4, "cdev_init" },
	{ 0x9b7fe4d4, "__dynamic_pr_debug" },
	{ 0x64c97ae, "device_destroy" },
	{ 0x6091b333, "unregister_chrdev_region" },
	{ 0x27e1a049, "printk" },
	{ 0xb33ee6e2, "device_create" },
	{ 0x801de1f9, "unregister_ftrace_function" },
	{ 0x6e14fd0d, "pid_task" },
	{ 0xe007de41, "kallsyms_lookup_name" },
	{ 0x58d9cd11, "cdev_add" },
	{ 0xafeda40e, "ftrace_set_filter_ip" },
	{ 0xbdfb6dbb, "__fentry__" },
	{ 0xb981a763, "find_get_pid" },
	{ 0x91a5c02c, "register_ftrace_function" },
	{ 0x21e01071, "class_destroy" },
	{ 0xe42dbab4, "__class_create" },
	{ 0xe3ec2f2b, "alloc_chrdev_region" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "0DFC752EFAC9F172274134B");
MODULE_INFO(rhelversion, "8.2");
