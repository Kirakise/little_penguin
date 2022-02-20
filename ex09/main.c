#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/dcache.h>
#include <linux/proc_fs.h>
#include <linux/miscdevice.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/path.h>
#include <linux/fs.h>
#include <linux/nsproxy.h>
#include <linux/list.h>
#include "cpfromkernel.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("rcaraway");
MODULE_VERSION("0.01");

static struct proc_dir_entry *ent;
static char *mountsdata;

static ssize_t misc_42_read(struct file *flip, char __user *buf,
		size_t count, loff_t *f_pos)
{
	struct mnt_namespace *ns = current->nsproxy->mnt_ns;
	struct mount *mnt;
	const char *name;
	const char *path;
	char *tmp;
	
	tmp = kmalloc(sizeof(char) * PAGE_SIZE, GFP_KERNEL);
	if (!tmp)
		return -ENOMEM;

	memset(mountsdata, 0, PAGE_SIZE);

	list_for_each_entry(mnt, &ns->list, mnt_list) {
		name = "none";
		path = "none";

		if (mnt->mnt_devname)
			name = mnt->mnt_devname;

		if (mnt->mnt_mountpoint) {
			path = dentry_path_raw(mnt->mnt_mountpoint,
					tmp, PAGE_SIZE);
		}

		if (strlen(name) == 4 && strlen(path) == 1 &&
				!memcmp(name, "none", 5) &&
				!memcmp(path, "/", 2))
			name = "root";
		snprintf(mountsdata, PAGE_SIZE, "%s%-20s%s\n", mountsdata,
				name, path);
	}
	kfree(tmp);
	return simple_read_from_buffer(buf, count, f_pos, mountsdata,
			strlen(mountsdata));
}

static struct proc_ops ops = {
	.proc_read = misc_42_read
};

static int __init ft_init(void)
{
	ent = proc_create("mymounts", 0444, NULL, &ops);
	if (!ent) {
		printk(KERN_INFO "cannot create proc file\n");
		return -EFAULT;
	}
	mountsdata = kmalloc(sizeof(char) * PAGE_SIZE, GFP_KERNEL);
	if (!mountsdata)
		return -ENOMEM;
	return 0;
}

static void __exit ft_exit(void)
{
	kfree(mountsdata);
	proc_remove(ent);
	printk(KERN_INFO "Exiting module\n");
}

module_init(ft_init);
module_exit(ft_exit);
