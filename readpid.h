#pragma once

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>

struct pid_info {
    pid_t pid_number;
    struct list_head _pid_list_head;
};

static LIST_HEAD(pid_list);

// register and unregister the char device.
int register_readpid_chrdev(const char* device_name);
void unregister_readpid_chrdev(int major_num, const char* device_name);
