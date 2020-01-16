/*
 * Copyright (C) 2019 Andrew <mrju.email@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 */

#include <linux/module.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/platform_device.h>

#define STR(x) _STR(x)
#define _STR(x) #x

#define VERSION_PREFIX Kmemleak
#define MAJOR_VERSION 1
#define MINOR_VERSION 0
#define PATCH_VERSION 0

#define VERSION STR(VERSION_PREFIX-MAJOR_VERSION.MINOR_VERSION.PATCH_VERSION)

#define DEVICE_NAME "test"

#define ENTRY_NAME "kmemleak"
#define PARENT_ENTRY_NAME "test"

static struct proc_dir_entry *parent;

static ssize_t kmemleak_proc_write(struct file *filp, const char __user *buf,
			size_t size, loff_t *offset)
{
	int err, val;
	char *temp = kzalloc(size, GFP_KERNEL);
	if (!temp)
		return -ENOMEM;

	if (copy_from_user(temp, buf, size)) {
		kfree(temp);
		return -EFAULT;
	}

	err = kstrtouint(temp, 10, &val);
	if (err) {
		kfree(temp);
		return err;
	}

	if (val > KMALLOC_MAX_SIZE)
		return -EINVAL;
	else
		kmalloc(val, GFP_KERNEL);

	kfree(temp);

	return size;
}

static struct file_operations kmemleak_proc_fops = {
	.owner = THIS_MODULE,
	.write = kmemleak_proc_write,
};

static int kmemleak_probe(struct platform_device *pdev)
{
	struct proc_dir_entry *kmemleak;

	parent = proc_mkdir(PARENT_ENTRY_NAME, NULL);
	if (!parent)
		return -ENOMEM;

	kmemleak = proc_create(ENTRY_NAME,
			S_IWUSR | S_IWGRP | S_IWOTH,
			parent, &kmemleak_proc_fops);
	if (!kmemleak) {
		remove_proc_entry(PARENT_ENTRY_NAME, NULL);
		return -ENOMEM;
	}

	return 0;
}

static int kmemleak_remove(struct platform_device *pdev)
{
	remove_proc_entry(ENTRY_NAME, parent);
	remove_proc_entry(PARENT_ENTRY_NAME, NULL);
	return 0;
}

static struct platform_driver kmemleak_drv = {
	.probe	= kmemleak_probe,
	.remove	= kmemleak_remove,
	.driver	= {
		.name = DEVICE_NAME,
	}
};

static int __init kmemleak_test_init(void)
{
	int ret;
	struct platform_device *pdev;

	pdev = platform_device_register_simple(DEVICE_NAME, -1, NULL, 0);
	if (IS_ERR(pdev))
		return PTR_ERR(pdev);

	ret = platform_driver_register(&kmemleak_drv);
	if (ret) {
		platform_device_unregister(pdev);
		return ret;
	}

	return 0;
}

static void __exit kmemleak_test_exit(void)
{
	struct device *dev;

	dev = bus_find_device_by_name(kmemleak_drv.driver.bus, NULL, DEVICE_NAME);
	if (dev)
		platform_device_unregister(to_platform_device(dev));

	platform_driver_unregister(&kmemleak_drv);
}

module_init(kmemleak_test_init);
module_exit(kmemleak_test_exit);

MODULE_ALIAS("kmemleak-driver");
MODULE_LICENSE("GPL");
MODULE_VERSION(VERSION);
MODULE_DESCRIPTION("Linux is not Unix");
MODULE_AUTHOR("andrew, mrju.email@gmail.com");
