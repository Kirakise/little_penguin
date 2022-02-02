#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("rcaraway");
MODULE_VERSION("0.01");


static ssize_t misc_42_read(struct file *flip, char __user *buf,
		size_t count, loff_t *f_pos)
{
	return simple_read_from_buffer(buf, count, f_pos, "rcaraway", 8);
}

static ssize_t misc_42_write(struct file *flip, const char __user *buf,
		size_t count, loff_t *f_pos)
{
	char msg[count];

	if (count != 8)
		return -EINVAL;

	if (simple_write_to_buffer(msg, count, f_pos, buf, count) < 0)
		return -EINVAL;

	return strncmp("rcaraway", msg, 8) ? -EINVAL : count;
}

const struct file_operations misc = {
	.owner = THIS_MODULE,
	.read = misc_42_read,
	.write = misc_42_write,
};

static struct miscdevice misc_42 = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "fortytwo",
	.fops = &misc,
};

static int __init ft_init(void)
{
	printk(KERN_INFO "Minor is %d\n", misc_42.minor);
	return misc_register(&misc_42);
}

static void __exit ft_exit(void)
{
	printk(KERN_INFO "Exiting module\n");
	misc_deregister(&misc_42);
}

module_init(ft_init);
module_exit(ft_exit);
