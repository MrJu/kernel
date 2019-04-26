/*
 * Copyright (C) 2019 Andrew <mrju.email@gail.com>
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
#include <linux/delay.h>

#define STR(x) _STR(x)
#define _STR(x) #x

#define VERSION_PREFIX Foo
#define MAJOR_VERSION 1
#define MINOR_VERSION 0
#define PATCH_VERSION 0

#define VERSION STR(VERSION_PREFIX-MAJOR_VERSION.MINOR_VERSION.PATCH_VERSION)

#define BASE_MINOR 0
#define MINOR_NUM 4
#define CLASS_NAME "foo"
#define DEVICE_NAME_PREFIX "foo"
#define DEVS_NANE "foo"

#define READ_INTERVAL_IN_MSEC 1000

struct foo_device {
	dev_t devno;
	struct cdev *cdev;
	struct class *class;
	struct device *device;
};

static struct foo_device *foo;

static int foo_open(struct inode *inode, struct file *filp)
{
	int minor;

	minor = iminor(file_inode(filp));

	printk("%s(): %s%d opened\n",
			__func__, DEVICE_NAME_PREFIX, minor);

	return 0;
}

static int foo_release(struct inode *inode, struct file *filp)
{
	int minor;

	minor = iminor(file_inode(filp));

	printk("%s(): %s%d closed\n",
			__func__, DEVICE_NAME_PREFIX, minor);

	return 0;
}

static ssize_t foo_read(struct file *filp, char __user *buf,
			size_t size, loff_t * offset)
{
	int minor;

	minor = iminor(file_inode(filp));

	printk("%s(): %s%d is saying\n",
			__func__, DEVICE_NAME_PREFIX, minor);

	msleep(READ_INTERVAL_IN_MSEC);

	return size;
}

static ssize_t foo_write(struct file *filp, const char __user * buf,
			size_t size, loff_t * offset)
{
	int minor;

	minor = iminor(file_inode(filp));

	printk("%s(): %s%d is saying\n",
			__func__, DEVICE_NAME_PREFIX, minor);

	return size;
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
	int index, ret;

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

	for (index = 0; index < MINOR_NUM; index++) {
		foo->device = device_create(foo->class, NULL,
		                MKDEV(MAJOR(foo->devno), BASE_MINOR + index),
		                NULL, DEVICE_NAME_PREFIX"%d", index);

		if (IS_ERR(foo->device)) {
			ret = PTR_ERR(foo->device);
			goto err_device_create;
		}
	}

	return 0;

err_device_create:
	for (index--; index >= 0; index--) {
		device_destroy(foo->class,
				MKDEV(MAJOR(foo->devno),
				BASE_MINOR + index));
	}

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
	int index;

	printk("%s(): %d\n", __func__, __LINE__);

	for (index = 0; index < MINOR_NUM; index++) {
		device_destroy(foo->class,
				MKDEV(MAJOR(foo->devno),
				BASE_MINOR + index));
	}

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
