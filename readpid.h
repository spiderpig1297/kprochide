#pragma once

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>

// register and unregister the char device.
int register_hidepid_chrdev(const char* device_name);
void unregister_hidepid_chrdev(int major_num, const char* device_name);
