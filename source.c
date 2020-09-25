#include "readpid.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("spiderpig");
MODULE_VERSION("1.0.0");

static const char* readpid_chrdev_name = "kprochide_readpid";
static int readpid_chrdev_major_num;

static int __init mod_init(void)
{
    printk(KERN_INFO "kprochide: loaded\n");
    readpid_chrdev_major_num = register_readpid_chrdev(readpid_chrdev_name);
    if (readpid_chrdev_major_num < 0) {
        printk(KERN_ERR "kprochide: failed to register char device (%d)\n", readpid_chrdev_major_num);
        return -EBUSY;
    }

    printk(KERN_INFO "kprochide: registered char device (%d)\n", readpid_chrdev_major_num);

    return 0;
}

static void __exit mod_exit(void)
{
    unregister_readpid_chrdev(readpid_chrdev_major_num, readpid_chrdev_name);
    printk(KERN_INFO "kprochide: unregistered char device %d\n", readpid_chrdev_major_num);
}

module_init(mod_init);
module_exit(mod_exit);
