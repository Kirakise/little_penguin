#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/dcache.h>
#include <linux/proc_fs.h>
#include <linux/miscdevice.h>
#include <linux/path.h>
#include <linux/fs.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("rcaraway");
MODULE_VERSION("0.01");

static struct proc_dir_entry *ent;

static ssize_t misc_42_read(struct file *flip, char __user *buf,
		size_t count, loff_t *f_pos)
{
	struct path path;
	struct dentry *curdentry;
	struct dentry *thedentry;
	char *s;
	char *tmp;

	kernfs_path("/", LOOKUP_FOLLOW, &path);
	thedentry = path.dentry;
	list_for_each_entry(curdentry,
			&thedentry->d_subdirs,
			d_child) {
		if (curdentry->d_flags & DCACHE_MOUNTED)
		{
			tmp = s;
			s = strjoin(s, curdentry->d_name.name);
			kfree(tmp);
			tmp = s;
			s = strjoin(s, "\n");
			kfree(tmp);

		}
	}
	simple_read_from_buffer(buf, count, f_pos, s, strlen(s));
	kfree(s);
}

static struct file_operations ops = {
	.owner = THIS_MODULE,
	.read = misc_42_read,
}

static int __init ft_init(void)
{
	ent = proc_create("mymounts", 0660, NULL, &ops);
	return 0;
}

static void __exit ft_exit(void)
{
	proc_remove(ent);
	printk(KERN_INFO "Exiting module\n");
}

module_init(ft_init);
module_exit(ft_exit);
