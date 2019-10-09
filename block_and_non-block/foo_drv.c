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
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/delay.h>
#include <linux/sched/signal.h>
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

#define READ_INTERVAL_IN_MSEC 1000

struct foo_device {
	dev_t devno;
	struct cdev cdev;
	struct class *class;
	struct device *dev;
	int ref_count;
	struct mutex open_mutex;
	wait_queue_head_t open_wait;
};

static int open_file(struct file *filp)
{
	struct foo_device *foo = filp->private_data;

	if (foo->ref_count > 0)
		return -EAGAIN;

	foo->ref_count++;

	return 0;
}

static int foo_open(struct inode *inode, struct file *filp)
{
	int err;
	wait_queue_entry_t wait;
	struct foo_device *foo
		= container_of(inode->i_cdev, struct foo_device, cdev);

	filp->private_data = foo;

	init_waitqueue_entry(&wait, current);
	add_wait_queue(&foo->open_wait, &wait);
	mutex_lock(&foo->open_mutex);
	while (1) {
		err = open_file(filp);
		if (err >= 0)
			break;

		if (err == -EAGAIN) {
			if (filp->f_flags & O_NONBLOCK) {
				err = -EBUSY;
				break;
			}
		} else
			break;

		set_current_state(TASK_INTERRUPTIBLE);
		mutex_unlock(&foo->open_mutex);
		schedule();
		mutex_lock(&foo->open_mutex);

		if (signal_pending(current)) {
			err = -ERESTARTSYS;
			break;
		}
	}

	remove_wait_queue(&foo->open_wait, &wait);
	mutex_unlock(&foo->open_mutex);

	return err;
}

ssize_t static foo_read(struct file *filp, char __user *buf,
			size_t size, loff_t *offset)
{
	int count;

	count = sprintf(buf, "%s(): process: %s, pid: %d\n",
			__func__, current->comm, current->pid);
	msleep(READ_INTERVAL_IN_MSEC);

	return count;
}

static ssize_t foo_write(struct file *filp, const char __user *buf,
			size_t size, loff_t *offset)
{
	return size;
}

static int foo_release(struct inode *inode, struct file *filp)
{
	struct foo_device *foo = filp->private_data;

	mutex_lock(&foo->open_mutex);
	foo->ref_count--;
	mutex_unlock(&foo->open_mutex);
	wake_up(&foo->open_wait);

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

	foo = devm_kzalloc(&pdev->dev, sizeof(*foo), GFP_KERNEL);
	if (!foo)
		return -ENOMEM;

	foo->ref_count = 0;
	mutex_init(&foo->open_mutex);
	init_waitqueue_head(&foo->open_wait);

	ret = alloc_chrdev_region(&foo->devno,
			BASE_MINOR, MINOR_NUM, DEVS_NANE);
	if (ret < 0)
		return -ENOMEM;

	cdev_init(&foo->cdev, &foo_fops);
	ret = cdev_add(&foo->cdev, foo->devno, MINOR_NUM);
	if (ret)
		goto err_cdev_add;

	foo->class = class_create(THIS_MODULE, CLASS_NAME);
	if (IS_ERR(foo->class)) {
		ret = PTR_ERR(foo->class);
		goto err_cdev_add;
	}

	foo->dev = device_create(foo->class,
			&pdev->dev, foo->devno, NULL, DEVICE_NAME);
	if (IS_ERR(foo->dev)) {
		ret = PTR_ERR(foo->dev);
		goto err_device_create;
	}

	platform_set_drvdata(pdev, foo);

	return 0;

err_device_create:
	class_unregister(foo->class);

err_cdev_add:
	unregister_chrdev_region(foo->devno, MINOR_NUM);

	return ret;
}

static int foo_remove(struct platform_device *pdev)
{
	struct foo_device *foo = platform_get_drvdata(pdev);

	device_destroy(foo->class, foo->devno);
	class_unregister(foo->class);
	unregister_chrdev_region(foo->devno, MINOR_NUM);
	devm_kfree(&pdev->dev, foo);

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
