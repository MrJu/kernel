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
#include <linux/uaccess.h>
#include <linux/platform_device.h>
#include "ioctl.h"

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
static struct trans_msg msg;

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

static long foo_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int ret = 0;

	if (_IOC_TYPE(cmd) != IOC_MAGIC)
		return -ENOTTY;

	if (_IOC_NR(cmd) >= IOC_NR_CNT)
		return -ENOTTY;

	if (_IOC_DIR(cmd) & _IOC_READ)
		ret= !access_ok(VERIFY_WRITE, (void __user *)arg, _IOC_SIZE(cmd));
	else if (_IOC_DIR(cmd) & _IOC_WRITE)
		ret= !access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd));

	if (ret)
		return -EFAULT;

	switch(cmd) {
	case IOC_CMD_NONE:
		printk("%s(): %s\n", __func__, "IOC_CMD_NONE");

		break;

	case IOC_CMD_GET:
		ret = copy_to_user((struct trans_msg __user *)arg, &msg, _IOC_SIZE(cmd));
		if (ret)
			return -EFAULT;

		printk("%s(): %s\n", __func__, "IOC_CMD_GET");

		break;

	case IOC_CMD_SET:
		ret = copy_from_user(&msg, (struct trans_msg __user *)arg, _IOC_SIZE(cmd));
		if (ret)
			return -EFAULT;

		printk("%s(): %s\n", __func__, "IOC_CMD_SET");

		break;

	case IOC_CMD_TRANS:
		ret = copy_from_user(&msg, (struct msg __user *)arg, _IOC_SIZE(cmd));
		if (ret)
			return -EFAULT;

		ret = copy_to_user((struct trans_msg __user *)arg, &msg, _IOC_SIZE(cmd));
		if (ret)
			return -EFAULT;

		printk("%s(): %s\n", __func__, "IOC_CMD_TRANS");

		break;

	default:
		printk("%s(): %s\n", __func__, "default");

		return -ENOTTY;
	}

	return 0;
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
	.unlocked_ioctl = foo_ioctl,
	.release = foo_release,
};

static int foo_probe(struct platform_device *pdev)
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

static int foo_remove(struct platform_device *pdev)
{
	printk("%s(): %d\n", __func__, __LINE__);
	device_destroy(foo->class, foo->devno);
	class_destroy(foo->class);
	cdev_del(foo->cdev);
	unregister_chrdev_region(foo->devno, MINOR_NUM);
	kfree(foo);

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
