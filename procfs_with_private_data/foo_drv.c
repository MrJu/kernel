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

#define VERSION_PREFIX Foo
#define MAJOR_VERSION 1
#define MINOR_VERSION 0
#define PATCH_VERSION 0

#define VERSION STR(VERSION_PREFIX-MAJOR_VERSION.MINOR_VERSION.PATCH_VERSION)

#define DEVICE_NAME "foo"

#define ENTRY_NAME "foo"
#define PARENT_ENTRY_NAME "debug"

static struct proc_dir_entry *parent;

static int foo_proc_show(struct seq_file *m, void *v) {
	unsigned int *enable = PDE_DATA(file_inode(m->file));
	seq_printf(m, "%s\n", *enable? "enable" : "disable");
	return 0;
}

static int foo_proc_open(struct inode *inode, struct  file *file) {
	return single_open(file, foo_proc_show, NULL);
}

static ssize_t foo_proc_write(struct file *filp, const char __user *buf,
			size_t size, loff_t *offset)
{
	int err, val;
	unsigned int *enable = PDE_DATA(file_inode(filp));
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

	if (!!val)
		*enable = 1;
	else
		*enable = 0;

	kfree(temp);

	return size;
}

static struct file_operations foo_proc_fops = {
	.owner = THIS_MODULE,
	.open = foo_proc_open,
	.read = seq_read,
	.write = foo_proc_write,
	.llseek = seq_lseek,
	.release = single_release,
};

static int foo_probe(struct platform_device *pdev)
{
	struct proc_dir_entry *foo;
	unsigned int enable = 0;

	parent = proc_mkdir(PARENT_ENTRY_NAME, NULL);
	if (!parent)
		return -ENOMEM;

	foo = proc_create_data(ENTRY_NAME,
			S_IRUSR | S_IRGRP | S_IROTH,
			parent, &foo_proc_fops, &enable);
	if (!foo) {
		remove_proc_entry(PARENT_ENTRY_NAME, NULL);
		return -ENOMEM;
	}

	return 0;
}

static int foo_remove(struct platform_device *pdev)
{
	remove_proc_entry(ENTRY_NAME, parent);
	remove_proc_entry(PARENT_ENTRY_NAME, NULL);
	return 0;
}

static struct platform_driver foo_drv = {
	.probe	= foo_probe,
	.remove	= foo_remove,
	.driver	= {
		.name = DEVICE_NAME,
	}
};

static int __init foo_init(void)
{
	return platform_driver_register(&foo_drv);
}

static void __exit foo_exit(void)
{
	platform_driver_unregister(&foo_drv);
}

module_init(foo_init);
module_exit(foo_exit);

MODULE_ALIAS("foo-driver");
MODULE_LICENSE("GPL");
MODULE_VERSION(VERSION);
MODULE_DESCRIPTION("Linux is not Unix");
MODULE_AUTHOR("andrew, mrju.email@gmail.com");
