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
#include <linux/cdev.h>
#include <linux/slab.h>

#define STR(x) _STR(x)
#define _STR(x) #x

#define VERSION_PREFIX Foo
#define MAJOR_VERSION 1
#define MINOR_VERSION 0
#define PATCH_VERSION 0

#define VERSION STR(VERSION_PREFIX-MAJOR_VERSION.MINOR_VERSION.PATCH_VERSION)

#define BASE_MINOR 0
#define MINOR_NUM 1
#define CLASS_NAME "foo"
#define DEVICE_NAME "foo"
#define DEVS_NANE "foo"


struct foo_device {
	dev_t devno;
	struct cdev *cdev;
	struct class *class;
	struct device *device;
};

static struct foo_device *foo;

static int foo_open(struct inode *inode, struct file *filp)
{
	printk("%s(): %d\n", __func__, __LINE__);
	return 0;
}

ssize_t static foo_read(struct file *filp, char __user *buf,
			size_t size, loff_t *offset)
{
	printk("%s(): %d\n", __func__, __LINE__);
	return size;
}

static ssize_t foo_write(struct file *filp, const char __user *buf,
			size_t size, loff_t *offset)
{
	printk("%s(): %d\n", __func__, __LINE__);
	return size;
}

static int foo_release(struct inode *inode, struct file *filp)
{
	printk("%s(): %d\n", __func__, __LINE__);
	return 0;
}

static struct file_operations foo_fops = {
	.owner = THIS_MODULE,
	.open = foo_open,
	.read = foo_read,
	.write = foo_write,
	.release = foo_release,
};

static int __init foo_init(void)
{
	int ret;

	printk("%s(): %d\n", __func__, __LINE__);

	foo = kzalloc(sizeof(*foo), GFP_KERNEL);
	if (!foo) {
		ret = -ENOMEM;
		goto err_device_alloc;
	}

	ret = alloc_chrdev_region(&foo->devno,
			BASE_MINOR, MINOR_NUM, DEVS_NANE);
	if (ret < 0)
		goto err_alloc_chrdev;

	foo->cdev = cdev_alloc();
	if (!foo->cdev) {
		ret = -ENOMEM;
		goto err_cdev_alloc;
	}

	cdev_init(foo->cdev, &foo_fops);

	ret = cdev_add(foo->cdev, foo->devno, MINOR_NUM);
	if (ret)
		goto err_cdev_add;

	foo->class = class_create(THIS_MODULE, CLASS_NAME);
	if (IS_ERR(foo->class)) {
		ret = PTR_ERR(foo->class);
		goto err_cdev_add;
	}

	foo->device = device_create(foo->class, NULL, foo->devno, NULL, DEVICE_NAME);
	if (IS_ERR(foo->device)) {
		ret = PTR_ERR(foo->device);
		goto err_device_create;
	}

	return 0;

err_device_create:
	class_destroy(foo->class);

err_cdev_add:
	cdev_del(foo->cdev);

err_cdev_alloc:
	unregister_chrdev_region(foo->devno, MINOR_NUM);

err_alloc_chrdev:
	kfree(foo);

err_device_alloc:
	return ret;
}

static void __exit foo_exit(void)
{
	printk("%s(): %d\n", __func__, __LINE__);
	device_destroy(foo->class, foo->devno);
	class_destroy(foo->class);
	cdev_del(foo->cdev);
	unregister_chrdev_region(foo->devno, MINOR_NUM);
	kfree(foo);
}

module_init(foo_init);
module_exit(foo_exit);

MODULE_ALIAS("foo-driver");
MODULE_LICENSE("GPL");
MODULE_VERSION(VERSION);
MODULE_DESCRIPTION("Linux is not Unix");
MODULE_AUTHOR("andrew, mrju.email@gmail.com");
