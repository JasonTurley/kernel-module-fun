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

	/* Never trust user input */
	if (count < 0 || count > MAX_SIZE) {
		printk(KERN_ALERT "echo_write: invalid buffer size: %zd\n", count);
		return -EFAULT;
	}

	ui = kmalloc(sizeof(struct user_info), GFP_KERNEL);
	if (!ui) {
		printk(KERN_ALERT "kmalloc failed\n");
		return -ENOMEM;
	}

	/* Read line of text from user input */
	if (copy_from_user(ui->buf, ubuf, count)) {
		printk(KERN_ALERT "echo_write: copy_from_user() failed\n");
		kfree(ui);
		return -EFAULT;
	}

	/* TODO should this be a function? */
	ui->pid = current->pid;
	ui->buf[count-1] = '\0';
	ui->size = count;

	list_add_tail(&(ui->list), &head);

	return count;
}

/*
 * Lists each line of text written to /proc/echo, along with the pid and message
 * size.
 */
 // TODO fix endless loop on reading empty file!
static ssize_t echo_read(struct file *filp, char __user *ubuf, size_t count, loff_t *ppos)
{
	struct user_info *ui;

	/* A non-zero data value means we've successfully read from the file.
	 * Return 0 to indicate EOF. Otherwise, processess will loop forever. */
	if ((int) *ppos > 0)
		return 0;

	// TODO need to add concurrency primitives
	/* For each entry written to /proc/echo, display the pid, message, and
	 * length. */
	list_for_each_entry(ui, &head, list) {
		char buf[200];

		sprintf(buf, "%d: %s (%zd bytes)\n", ui->pid, ui->buf, ui->size);

		if (copy_to_user(ubuf + *ppos, buf, strlen(buf) + 1)) {
			printk(KERN_ALERT "copy_to_user failed\n");
			return -EFAULT;
		}

		*ppos += sizeof(buf);
	}

	return count - *ppos;
}

static const struct file_operations echo_ops = {
	.owner		= THIS_MODULE,
	.write		= echo_write,
	.read		= echo_read,
};

static int __init echo_init(void)
{
	if (!proc_create(FILENAME, 0666, NULL, &echo_ops)) {
		printk(KERN_ALERT "proc_create: failed to open %s\n", FILENAME);
		return -ENOMEM;
	}

	return 0;
}

static void __exit echo_exit(void)
{
	struct user_info *ui, *tmp;

	remove_proc_entry(FILENAME, NULL);

	list_for_each_entry_safe(ui, tmp, &head, list) {
		list_del(&(ui->list));
		kfree(ui);
	}
}

module_init(echo_init);
module_exit(echo_exit);
