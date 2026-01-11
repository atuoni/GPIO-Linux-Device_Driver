#include <linux/module.h>
#include <linux/export-internal.h>
#include <linux/compiler.h>

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



static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0xfa474811, "__platform_driver_register" },
	{ 0x8b970f46, "device_destroy" },
	{ 0x6775d5d3, "class_destroy" },
	{ 0x27271c6b, "cdev_del" },
	{ 0x6091b333, "unregister_chrdev_region" },
	{ 0xe2964344, "__wake_up" },
	{ 0x36a78de3, "devm_kmalloc" },
	{ 0xcb29cd6b, "gpiod_count" },
	{ 0xbb9a6196, "gpiod_direction_output" },
	{ 0xe58ce88c, "devm_gpiod_get_index" },
	{ 0xebe20c1, "of_property_read_string_helper" },
	{ 0xe2d5255a, "strcmp" },
	{ 0x2cc6ea32, "gpiod_direction_input" },
	{ 0x2bc19084, "gpiod_to_irq" },
	{ 0x3ce80115, "devm_request_threaded_irq" },
	{ 0xe3ec2f2b, "alloc_chrdev_region" },
	{ 0xa01f13a6, "cdev_init" },
	{ 0x3a6d85d3, "cdev_add" },
	{ 0x59c02473, "class_create" },
	{ 0x2c9a4c10, "device_create" },
	{ 0x3bb3b979, "_dev_info" },
	{ 0xf0fdf6cb, "__stack_chk_fail" },
	{ 0x61fd46a9, "platform_driver_unregister" },
	{ 0x12a4e128, "__arch_copy_from_user" },
	{ 0x991fb4bf, "gpiod_set_value" },
	{ 0x6cbbfc54, "__arch_copy_to_user" },
	{ 0x30708b69, "gpiod_get_value" },
	{ 0x474e54d2, "module_layout" },
};

MODULE_INFO(depends, "");

MODULE_ALIAS("of:N*T*Camauri,mygpio");
MODULE_ALIAS("of:N*T*Camauri,mygpioC*");

MODULE_INFO(srcversion, "1C50F9A0DD74F103D1205A9");
