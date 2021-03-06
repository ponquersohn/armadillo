#include <linux/build-salt.h>
#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;

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

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0x25a444ff, "module_layout" },
	{ 0x78c2e801, "cdev_del" },
	{ 0xb551d60c, "cdev_init" },
	{ 0x754d539c, "strlen" },
	{ 0x5e368f62, "kallsyms_on_each_symbol" },
	{ 0xb0bb794e, "device_destroy" },
	{ 0x6091b333, "unregister_chrdev_region" },
	{ 0xe2d5255a, "strcmp" },
	{ 0xc5850110, "printk" },
	{ 0x9166fada, "strncpy" },
	{ 0x593c1bac, "__x86_indirect_thunk_rbx" },
	{ 0xd351cf4d, "device_create" },
	{ 0x1c1e6a2, "pid_task" },
	{ 0x6042c10a, "cdev_add" },
	{ 0xdecd0b29, "__stack_chk_fail" },
	{ 0x2ea2c95c, "__x86_indirect_thunk_rax" },
	{ 0xbdfb6dbb, "__fentry__" },
	{ 0x8b9200fd, "lookup_address" },
	{ 0xbec8fc4e, "find_get_pid" },
	{ 0x10688a0c, "class_destroy" },
	{ 0x5b91a631, "__class_create" },
	{ 0xe3ec2f2b, "alloc_chrdev_region" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";

