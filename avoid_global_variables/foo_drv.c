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
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/delay.h>
#include <linux/platform_device.h>

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

#define MS_TO_NS(x) (x * 1E6L)
#define DELAY_MS 1L
#define READ_INTERVAL_IN_MSEC 1000


struct foo_device {
	dev_t devno;
	struct cdev cdev;
	struct class class;
	struct device *dev;
	struct hrtimer timer;
	ktime_t ktime;
	unsigned int callback_count;
};

static ssize_t callback_count_show(struct class *class,
				struct class_attribute *attr, char *buf)
{
	struct foo_device *foo = container_of(class, struct foo_device, class);
	return sprintf(buf, "callback_count: %d\n", foo->callback_count);
}
static CLASS_ATTR_RO(callback_count);

static struct attribute *foo_class_attrs[] = {
	&class_attr_callback_count.attr,
	NULL,
};
ATTRIBUTE_GROUPS(foo_class);

static enum hrtimer_restart hrtimer_handler(struct hrtimer *timer)
{
	struct foo_device *foo = container_of(timer, struct foo_device, timer);
	foo->callback_count++;
	hrtimer_forward_now(&foo->timer, foo->ktime);

	return HRTIMER_RESTART;
}

static int foo_open(struct inode *inode, struct file *filp)
{
	filp->private_data = container_of(inode->i_cdev, struct foo_device, cdev);
	printk("%s(): %d\n", __func__, __LINE__);
	return 0;
}

ssize_t static foo_read(struct file *filp, char __user *buf,
			size_t size, loff_t *offset)
{
	int count;
	struct foo_device *foo = filp->private_data;

	printk("%s(): %d, size: %lu\n", __func__, __LINE__, size);

	count = sprintf(buf, "callback_count: %d\n", foo->callback_count);
	msleep(READ_INTERVAL_IN_MSEC);

	return count;
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

static int foo_probe(struct platform_device *pdev)
{
	int ret;
	struct foo_device *foo;

	printk("%s(): %d\n", __func__, __LINE__);

	foo = kzalloc(sizeof(*foo), GFP_KERNEL);
	if (!foo) {
		ret = -ENOMEM;
		goto err_device_alloc;
	}

	platform_set_drvdata(pdev, foo);

	ret = alloc_chrdev_region(&foo->devno,
			BASE_MINOR, MINOR_NUM, DEVS_NANE);
	if (ret < 0)
		goto err_alloc_chrdev;

	cdev_init(&foo->cdev, &foo_fops);
	ret = cdev_add(&foo->cdev, foo->devno, MINOR_NUM);
	if (ret)
		goto err_cdev_add;

	foo->class.owner = THIS_MODULE;
	foo->class.name = CLASS_NAME;
	foo->class.class_groups = foo_class_groups;
	ret = class_register(&foo->class);
	if (ret)
		goto err_cdev_add;

	foo->dev = device_create(&foo->class,
			&pdev->dev, foo->devno, NULL, DEVICE_NAME);
	if (IS_ERR(foo->dev)) {
		ret = PTR_ERR(foo->dev);
		goto err_device_create;
	}

	foo->ktime = ktime_set(0, MS_TO_NS(DELAY_MS));
	hrtimer_init(&foo->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	foo->timer.function = hrtimer_handler;
	hrtimer_start(&foo->timer, foo->ktime, HRTIMER_MODE_REL);

	return 0;

err_device_create:
	class_unregister(&foo->class);

err_cdev_add:
	unregister_chrdev_region(foo->devno, MINOR_NUM);

err_alloc_chrdev:
	kfree(foo);

err_device_alloc:
	return ret;
}

static int foo_remove(struct platform_device *pdev)
{
	struct foo_device *foo = platform_get_drvdata(pdev);

	printk("%s(): %d\n", __func__, __LINE__);

	hrtimer_cancel(&foo->timer);
	device_destroy(&foo->class, foo->devno);
	class_unregister(&foo->class);
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
