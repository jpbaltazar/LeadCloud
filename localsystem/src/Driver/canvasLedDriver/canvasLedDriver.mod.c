#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
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
__used __section("__versions") = {
	{ 0xab2e3178, "module_layout" },
	{ 0x404c211a, "param_ops_int" },
	{ 0x9f49dcc4, "__stack_chk_fail" },
	{ 0xd9af1a1d, "device_create" },
	{ 0xeb233a45, "__kmalloc" },
	{ 0x3f7ef9cd, "__alloc_pages_nodemask" },
	{ 0xe7a89283, "__class_create" },
	{ 0xe3ec2f2b, "alloc_chrdev_region" },
	{ 0x3fd78f3b, "register_chrdev_region" },
	{ 0x6091b333, "unregister_chrdev_region" },
	{ 0x570e683b, "class_destroy" },
	{ 0xd393bb1a, "device_destroy" },
	{ 0x37a0cba, "kfree" },
	{ 0xcc3bed4e, "cdev_add" },
	{ 0xcd708882, "cdev_init" },
	{ 0xc8dcc62a, "krealloc" },
	{ 0x6cbbfc54, "__arch_copy_to_user" },
	{ 0xdcb764ad, "memset" },
	{ 0xc5850110, "printk" },
	{ 0x12a4e128, "__arch_copy_from_user" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "EAA2772063D1FF3CEB99046");
