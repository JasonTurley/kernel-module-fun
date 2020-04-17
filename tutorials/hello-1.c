/*
 * hello-1.c - The simplest kernel module.
 */
#include <linux/init.h>
#include <linux/module.h>

MODULE_LICENSE("GLP");

int init_module(void)
{
    printk(KERN_ALERT "Hello world 1\n");
    return 0;   /* Non zero return value indicates an error */
}

void cleanup_module(void)
{
    printk(KERN_ALERT "Goodbye world 1\n");
}
