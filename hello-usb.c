/*
 * hello-usb.c - prints a string when a usb device is connected and
 * disconnected.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/usb.h>

//MODULE_LISCENSE("GPL");

/* In this example I am using a USB Mouse with Vendor ID 413c and
 * Product ID 301a
 */

#define VENDOR_ID "413c"
#define PRODUCT_ID "301a"

static int __init hello_usb_init(void)
{
	printk(KERN_INFO "Hello USB!\n");
	return 0;
}

static void __exit hello_usb_exit(void)
{
	printk(KERN_INFO "Goodbye USB!\n");
}

module_init(hello_usb_init);
module_exit(hello_usb_exit);
