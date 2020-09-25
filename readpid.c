#include "hidepid.h"

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

int register_hidepid_chrdev(const char* device_name)
{
    return register_chrdev(0, device_name, &_file_ops);
}

void unregister_hidepid_chrdev(int major_num, const char* device_name) 
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

    // hide process

    return len;
}
