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
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/uaccess.h>
#include <linux/platform_device.h>

#define STR(x) _STR(x)
#define _STR(x) #x

#define VERSION_PREFIX Dummy
#define MAJOR_VERSION 1
#define MINOR_VERSION 0
#define PATCH_VERSION 0

#define VERSION STR(VERSION_PREFIX-MAJOR_VERSION.MINOR_VERSION.PATCH_VERSION)

#define BASE_MINOR 0
#define MINOR_NUM 1
#define CLASS_NAME "dummy"
#define DEVICE_NAME "dummy"
#define DEVS_NANE "dummy"

struct dummy_device {
	dev_t devno;
	struct cdev cdev;
	struct class class;
	struct device *dev;
	struct mutex mutex;
	unsigned int value;
};

static ssize_t value_show(struct class *class,
		struct class_attribute *attr, char *buf)
{
	struct dummy_device *dummy;
	dummy = container_of(class, struct dummy_device, class);
	return sprintf(buf, "%d\n", dummy->value);
}

static ssize_t value_store(struct class *class,
		struct class_attribute *attr,
		const char *buf, size_t count)
{
	int err, val;
	struct dummy_device *dummy;

	dummy = container_of(class, struct dummy_device, class);

	err = kstrtouint(buf, 10, &val);
	if (err)
		return err;

	dummy->value = val;

	return count;
}
static CLASS_ATTR_RW(value);

static struct attribute *dummy_class_attrs[] = {
	&class_attr_value.attr,
	NULL,
};
ATTRIBUTE_GROUPS(dummy_class);

static int dummy_open(struct inode *inode, struct file *filp)
{
	filp->private_data = container_of(
			inode->i_cdev,
			struct dummy_device, cdev);
	return 0;
}

ssize_t static dummy_read(struct file *filp,
		char __user *buf,
		size_t count, loff_t *offset)
{
	int ret;
	struct dummy_device *dummy = filp->private_data;

	mutex_lock(&dummy->mutex);
	ret = copy_to_user(buf, (char *)&dummy->value,
			sizeof(dummy->value));
	if (ret) {
		mutex_unlock(&dummy->mutex);
		return ret;
	}
	mutex_unlock(&dummy->mutex);

	return count;
}

static ssize_t dummy_write(struct file *filp,
		const char __user *buf,
		size_t count, loff_t *offset)
{
	int ret;
	struct dummy_device *dummy = filp->private_data;

	mutex_lock(&dummy->mutex);
	ret = copy_from_user(&dummy->value, buf, sizeof(dummy->value));
	if (ret) {
		mutex_unlock(&dummy->mutex);
		return ret;
	}
	mutex_unlock(&dummy->mutex);

	return count;
}

static int dummy_release(struct inode *inode, struct file *filp)
{
	return 0;
}

static struct file_operations dummy_fops = {
	.owner = THIS_MODULE,
	.open = dummy_open,
	.read = dummy_read,
	.write = dummy_write,
	.release = dummy_release,
};

static int dummy_probe(struct platform_device *pdev)
{
	int ret;
	struct dummy_device *dummy;

	dummy = devm_kzalloc(&pdev->dev,
			sizeof(*dummy), GFP_KERNEL);
	if (!dummy)
		return -ENOMEM;

	mutex_init(&dummy->mutex);

	ret = alloc_chrdev_region(&dummy->devno,
			BASE_MINOR, MINOR_NUM, DEVS_NANE);
	if (ret < 0)
		return ret;

	cdev_init(&dummy->cdev, &dummy_fops);
	ret = cdev_add(&dummy->cdev, dummy->devno, MINOR_NUM);
	if (ret)
		goto err_cdev_add;

	dummy->class.owner = THIS_MODULE;
	dummy->class.name = CLASS_NAME;
	dummy->class.class_groups = dummy_class_groups;

	ret = class_register(&dummy->class);
	if (ret)
		goto err_cdev_add;

	dummy->dev = device_create(&dummy->class,
			&pdev->dev, dummy->devno, NULL, DEVICE_NAME);
	if (IS_ERR(dummy->dev)) {
		ret = PTR_ERR(dummy->dev);
		goto err_device_create;
	}

	platform_set_drvdata(pdev, dummy);

	return 0;

err_device_create:
	class_unregister(&dummy->class);

err_cdev_add:
	unregister_chrdev_region(dummy->devno, MINOR_NUM);
	return ret;
}

static int dummy_remove(struct platform_device *pdev)
{
	struct dummy_device *dummy = platform_get_drvdata(pdev);

	device_destroy(&dummy->class, dummy->devno);
	class_unregister(&dummy->class);
	unregister_chrdev_region(dummy->devno, MINOR_NUM);
	devm_kfree(&pdev->dev, dummy);

	return 0;
}

static struct platform_driver dummy_drv = {
	.probe	= dummy_probe,
	.remove	= dummy_remove,
	.driver	= {
		.name = DEVICE_NAME,
	}
};

static int __init dummy_init(void)
{
	int ret;
	struct platform_device *pdev;

	pdev = platform_device_register_simple(DEVICE_NAME,
			-1, NULL, 0);
	if (IS_ERR(pdev))
		return PTR_ERR(pdev);

	ret = platform_driver_register(&dummy_drv);
	if (ret) {
		platform_device_unregister(pdev);
		return ret;
	}

	return 0;
}

static void __exit dummy_exit(void)
{
	struct device *dev;

	dev = bus_find_device_by_name(dummy_drv.driver.bus,
			NULL, DEVICE_NAME);
	if (dev)
		platform_device_unregister(to_platform_device(dev));

	platform_driver_unregister(&dummy_drv);
}

module_init(dummy_init);
module_exit(dummy_exit);

MODULE_ALIAS("dummy-driver");
MODULE_LICENSE("GPL");
MODULE_VERSION(VERSION);
MODULE_DESCRIPTION("Linux is not Unix");
MODULE_AUTHOR("andrew, mrju.email@gmail.com");
