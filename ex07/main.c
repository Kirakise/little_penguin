#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/debugfs.h>
#include <linux/mutex.h>
#include <linux/jiffies.h>
#include <linux/slab.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("rcaraway");
MODULE_VERSION("0.01");


static struct dentry *Dir = 0;
static struct dentry *food = 0;
static struct dentry *idd = 0;
static struct dentry *jiffiesd = 0;

static char *foo_str;
static size_t foo_str_len;

DEFINE_MUTEX(foo_mutex);

static ssize_t id_file_read(struct file *flip, char *buffer,
		size_t len, loff_t *offset)
{
	return simple_read_from_buffer(buffer, len, offset, "rcaraway", 8); 
}


static ssize_t id_file_write(struct file *flip, const char *buffer,
		size_t len, loff_t *offset)
{
	char msg[8];

	if (len != 8)
		return -EINVAL;
	if (simple_write_to_buffer(msg, len, offset, buffer, len) < 0)
		return -EINVAL;
	return strncmp("rcaraway", msg, 8) ? -EINVAL : len;
}

static ssize_t foo_file_read(struct file *flip, char *buffer,
		size_t len, loff_t *offset)
{
	int retval;
	mutex_lock(&foo_mutex);

	if (!foo_str) {
		mutex_unlock(&foo_mutex);
		return 0;
	}
	
	retval = simple_read_from_buffer(buffer, len, offset,
			foo_str, foo_str_len);

	mutex_unlock(&foo_mutex);
	return retval;
}

static ssize_t foo_file_write(struct file *flip, const char *buffer,
		size_t len, loff_t *offset)
{
	char *new_str;

	if (len > PAGE_SIZE)
		return -EINVAL;

	new_str = kmalloc(len, GFP_KERNEL);

	if (copy_from_user(new_str, buffer, len)) {
		kfree(new_str);
		return -EINVAL;
	}

	mutex_lock(&foo_mutex);

	kfree(foo_str);
	foo_str = new_str;
	foo_str_len = len;

	mutex_unlock(&foo_mutex);

	return len;
}

static ssize_t jiffies_file_read(struct file *flip, char *buffer,
		size_t len, loff_t *offset)
{
	int size = snprintf(buffer, len, "%lu\n", jiffies);

	return size > len ? len : size;
}

static struct file_operations id_fops = {
	.read = id_file_read,
	.write = id_file_write,
};

static struct file_operations foo_fops = {
	.read = foo_file_read,
	.write = foo_file_write,
};

static struct file_operations jiffies_fops = {
	.read = jiffies_file_read,
};

static int error_print(void)
{
	printk(KERN_ERR "Failed to create some file\n");
	debugfs_remove_recursive(Dir);
	return 1;
}

static int __init ft_init(void)
{
	Dir = debugfs_create_dir("fortytwo", NULL);

	if (!Dir || (long)Dir == -ENODEV){
		printk(KERN_ERR "failed to create directory\n");
		return 1;
	}
	food = debugfs_create_file("foo", 0644, Dir, NULL, &foo_fops);

	if (!food || (long)food == -ENODEV)
		return error_print();

	jiffiesd = debugfs_create_file("jiffues", 0444, Dir, NULL,
			&jiffies_fops);

	if (!jiffiesd || (long)jiffiesd == -ENODEV)
		return error_print();

	idd = debugfs_create_file("id", 0666, Dir, NULL, &id_fops);

	if (!idd || (long)idd == -ENODEV)
		return error_print();

	printk(KERN_INFO "Everything seems fine\n");
	return 0;
}

static void __exit ft_exit(void)
{
	debugfs_remove_recursive(Dir);

	if (foo_str)
		kfree(foo_str);

	printk(KERN_INFO "Module exited\n");
}

module_init(ft_init);
module_exit(ft_exit);
