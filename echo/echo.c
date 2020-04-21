/*
 * echo.c - proc file that displays the line of text written to it.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

#include <linux/fs.h>		// for basic filesystem
#include <linux/proc_fs.h>	// for the proc filesystem

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jason");

#define FILENAME "echo"

/*
 * Displays the line of text and its length.
 */
static ssize_t echo_write(struct file *filp, const char __user *ubuf, size_t count, loff_t *ppos)
{
	printk(KERN_DEBUG "write handler\n");
	return -1;
}

static const struct file_operations echo_ops = {
	.owner		= THIS_MODULE,
	.write		= echo_write,
};

static int __init echo_init(void)
{
	if (!proc_create(FILENAME, 0222, NULL, &echo_ops)) {
		printk(KERN_ERR "proc_create: failed to open %s\n", FILENAME);
		return -ENOMEM;
	}

	return 0;
}

static void __exit echo_exit(void)
{
	remove_proc_entry(FILENAME, NULL);
}

module_init(echo_init);
module_exit(echo_exit);
