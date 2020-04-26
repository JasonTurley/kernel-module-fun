/*
 * echo.c - proc file that displays the line of text written to it.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

#include <linux/fs.h>		// for basic filesystem
#include <linux/list.h>
#include <linux/proc_fs.h>	// for the proc filesystem
#include <linux/spinlock.h>
#include <linux/slab.h>		// for kmalloc, kfree
#include <linux/uaccess.h>	// for copy_from/to_user

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jason");

#define FILENAME "echo"
#define BUF_SIZE 128

static LIST_HEAD(head);

/*
 * user_info - stores info about the currently running tasks.
 */
struct user_info {
	int pid;		// The current process id
	char buf[BUF_SIZE];	// Stores the user message
	size_t size;		// Size of the user message
	struct list_head list;	// List of processes that wrote to /proc/echo
};

/* Global lock for safely accessing shared list data structure */
static spinlock_t my_lock;

/*
 * Called when a process writes to the /proc/echo file. Records the pid, line of
 * text, and text length.
 */
static ssize_t echo_write(struct file *filp, const char __user *ubuf, size_t count, loff_t *ppos)
{
	struct user_info *ui;

	/* Never trust user input */
	if (count < 0 || count > BUF_SIZE) {
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

	/* Initialize structure fields */
	ui->pid = current->pid;
	ui->buf[count-1] = '\0';
	ui->size = count;

	/* Protect critical section */
	spin_lock(&my_lock);
	list_add_tail(&(ui->list), &head);
	spin_unlock(&my_lock);

	return count;
}

/*
 * Lists each line of text written to /proc/echo, along with the pid and message
 * size.
 */
 // TODO fix endless loop on reading empty file!
static ssize_t echo_read(struct file *filp, char __user *ubuf, size_t count, loff_t *ppos)
{
	struct user_info *ui, *next;
	size_t len, bytes_read = 0;

	printk(KERN_SOH "Starting at offset %lld, read %zd bytes", *ppos,
	count);

	/* A non-zero return value means we've successfully read from the file.
	 * Return zero to indicate EOF so program doesn't loop forever. */
	if (*ppos > 0)
		return 0;

	/* For each entry written to /proc/echo, display the pid, message, and
	 * length. */
	spin_lock(&my_lock);
	list_for_each_entry_safe(ui, next, &head, list) {
		char buf[BUF_SIZE + 20];

		sprintf(buf, "%d: %s (%zd bytes)\n", ui->pid, ui->buf, ui->size);

		len = strlen(buf) + 1;

		if (copy_to_user(ubuf + *ppos, buf, len)) {
			printk(KERN_ALERT "copy_to_user failed\n");
			spin_unlock(&my_lock);
			return -EFAULT;
		}

		bytes_read += len;
		*ppos += bytes_read;
	}
	spin_unlock(&my_lock);

	return bytes_read;
}

static const struct file_operations echo_ops = {
	.owner		= THIS_MODULE,
	.write		= echo_write,
	.read		= echo_read,
};

static int __init echo_init(void)
{
	spin_lock_init(&my_lock);

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
