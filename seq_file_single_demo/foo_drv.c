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
#include <linux/proc_fs.h>
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

static int foo_proc_show(struct seq_file *m, void *v) {
	seq_printf(m, "%s\n", VERSION);
	return 0;
}

static int foo_proc_open(struct inode *inode, struct  file *file) {
	return single_open(file, foo_proc_show, NULL);
}

static struct file_operations foo_proc_fops = {
	.owner = THIS_MODULE,
	.open = foo_proc_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static int foo_probe(struct platform_device *pdev)
{
	struct proc_dir_entry *foo;

	printk("%s(): %d\n", __func__, __LINE__);

	foo = proc_create(ENTRY_NAME,
			S_IRUSR | S_IRGRP | S_IROTH,
			NULL, &foo_proc_fops);
	if (!foo)
		return -ENOMEM;

	return 0;
}

static int foo_remove(struct platform_device *pdev)
{
	printk("%s(): %d\n", __func__, __LINE__);

	remove_proc_entry(ENTRY_NAME, NULL);
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
