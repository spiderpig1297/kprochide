#include "readpid.h"

#include <linux/fs.h>
#include <linux/slab.h>

#define DECIMAL_BASE (10)

static int device_open_count = 0;

static int device_open(struct inode* inode, struct file* file);
static int device_release(struct inode* inode, struct file* file);
static ssize_t device_read(struct file *fs, char *buffer, size_t len, loff_t *offset);
static ssize_t device_write(struct file *fs, const char*buffer, size_t len, loff_t *offset);

static struct file_operations _file_ops = {
    .read = device_read,
    .write = device_write,
    .open = device_open,
    .release = device_release
};

int register_readpid_chrdev(const char* device_name)
{
    return register_chrdev(0, device_name, &_file_ops);
}

void unregister_readpid_chrdev(int major_num, const char* device_name) 
{
    unregister_chrdev(major_num, device_name);
}

static int device_open(struct inode* inode, struct file* file)
{
    if (device_open_count) {
        return -EBUSY;
    }

    device_open_count++;
    try_module_get(THIS_MODULE);
    return 0;
}

static int device_release(struct inode* inode, struct file* file)
{
    device_open_count--;
    module_put(THIS_MODULE);
    return 0;
}

static ssize_t device_read(struct file *fs, char *buffer, size_t len, loff_t *offset)
{
    struct list_head *pos = NULL;
    struct list_head *tmp;
    struct pid_info *info = NULL;

    list_for_each_safe(pos, tmp, &pid_list) {
        info = NULL;

        printk(KERN_INFO "kprochide: in for each\n");

        info = list_entry(pos, struct pid_info, _pid_list_head);
        if (NULL == info) {
            continue;
        }

        printk(KERN_INFO "info->pid_number=%d\n", info->pid_number);
        list_del(pos);
        kfree(info);
    }

    return len;
}

static ssize_t device_write(struct file *fs, const char*buffer, size_t len, loff_t *offset)
{
    int pid_as_int = 0;
    int conversion_result = kstrtoint(buffer, DECIMAL_BASE, &pid_as_int);
    if ((-EINVAL == conversion_result) || (-ERANGE == conversion_result)) {
        printk(KERN_DEBUG "kprochide: failed to convert buffer to pid_t.\n");
        return EIO;
    }

    pid_t pid = (pid_t)pid_as_int;
    printk(KERN_INFO "kprochide: received pid %d\n", pid);

    // add a new pid_info* to the linked-list.
    struct pid_info* info = (struct pid_info*)kmalloc(sizeof(struct pid_info), GFP_KERNEL);
    info->pid_number = pid;

    printk(KERN_INFO "kprochide: adding pid %d to list\n", pid);
    list_add_tail(&info->_pid_list_head, &pid_list);

    return len;
}
