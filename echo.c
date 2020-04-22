/*
 * echo.c - proc file that displays the line of text written to it.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

#include <linux/slab.h>		// for kmalloc, kfree
#include <linux/fs.h>		// for basic filesystem
#include <linux/proc_fs.h>	// for the proc filesystem
#include <linux/uaccess.h>	// for copy_from/to_user
#include <linux/list.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jason");

#define FILENAME "echo"
#define MAX_SIZE 128

static LIST_HEAD(head);

/*
 * user_info - stores info about the currently running tasks.
 */
struct user_info {
	int pid;		// The current process id
	char buf[MAX_SIZE];	// Stores the user message
	size_t size;		// Size of the user message
	struct list_head list;	// List of processes that wrote to /proc/echo
};

/*
 * Called when a process writes to the /proc/echo file. Records the pid, line of
 * text, and text length.
 */
static ssize_t echo_write(struct file *filp, const char __user *ubuf, size_t count, loff_t *ppos)
{
	struct user_info *ui;

	ui = kmalloc(sizeof(struct user_info), GFP_KERNEL);

	if (!ui) {
		printk(KERN_ALERT "kmalloc failed\n");
		return -ENOMEM;
	}

	/* Never trust user input */
	if (count < 0 || count > MAX_SIZE) {
		printk(KERN_ALERT "echo_write: invalid buffer size: %zd\n", count);
		return -EFAULT;
	}

	/* Read line of text from user input */
	if (copy_from_user(ui->buf, ubuf, count)) {
		printk(KERN_ALERT "echo_write: copy_from_user() failed\n");
		return -EFAULT;
	}

	/* TODO should this be a function? */
	ui->pid = current->pid;
	ui->buf[count] = '\0';
	ui->size = count;

	list_add_tail(&(ui->list), &head);

	// TODO do not kfree her normally, doing this for debug
	kfree(ui);

	return count;
}

static const struct file_operations echo_ops = {
	.owner		= THIS_MODULE,
	.write		= echo_write,
};

static int __init echo_init(void)
{
	if (!proc_create(FILENAME, 0222, NULL, &echo_ops)) {
		printk(KERN_ALERT "proc_create: failed to open %s\n", FILENAME);
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
