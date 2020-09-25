#include "hidepid.h"

#include <linux/delay.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("spiderpig");
MODULE_VERSION("1.0.0");

static const char* hidepid_chrdev_name = "kprochide_hidepid";
static int hidepid_chrdev_major_num;

static int __init mod_init(void)
{
    printk(KERN_INFO "kprochide: loaded\n");
    hidepid_chrdev_major_num = register_hidepid_chrdev(hidepid_chrdev_name);
    if (hidepid_chrdev_major_num < 0) {
        printk(KERN_ERR "kprochide: failed to register char device (%d)\n", hidepid_chrdev_major_num);
        return -EBUSY;
    }

    printk(KERN_INFO "kprochide: registered char device (%d)\n", hidepid_chrdev_major_num);
    printk(KERN_INFO "kprochide: please run 'mknod /dev/hidepid c %d 0'\n", hidepid_chrdev_major_num);

    return 0;
}

static void __exit mod_exit(void)
{
    unregister_hidepid_chrdev(hidepid_chrdev_major_num, hidepid_chrdev_name);
    printk(KERN_INFO "kprochide: unregistered char device %d\n", hidepid_chrdev_major_num);
}

module_init(mod_init);
module_exit(mod_exit);
