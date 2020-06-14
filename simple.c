/*
 * A simple module that prints a message when it is loaded/unloaded
 * to/from the kernel.
 * Created 6/5/2020
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jason");

static int __init simple_init(void)
{
	printk(KERN_INFO "LOADED MODULE\n");
	return 0;
}

static void __exit simple_exit(void)
{
	printk(KERN_INFO "UNLOADED MODULE\n");
}

module_init(simple_init);
module_exit(simple_exit);
