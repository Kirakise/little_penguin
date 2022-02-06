#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/time.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("rcaraway");
MODULE_VERSION("0.01");

static int __init ft_init(void)
{
	struct timespec64 tv;
	ktime_get_real_ts64(&tv);
	
	printk(KERN_INFO "[%ptTd %ptTt] Hello world!\n", &tv, &tv);
	return (0);
}

static void __exit ft_exit(void)
{
	struct timespec64 tv;
	ktime_get_real_ts64(&tv);
	
	printk(KERN_INFO "[%ptTd %ptTt] Cleaning up module\n", &tv, &tv);
}

module_init(ft_init);
module_exit(ft_exit);
