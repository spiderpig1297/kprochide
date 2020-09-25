#include "readpid.h"

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
    return 0;
}

static ssize_t device_write(struct file *fs, const char*buffer, size_t len, loff_t *offset)
{
    return 0;
}
