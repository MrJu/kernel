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
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/delay.h>
#include <linux/i2c.h>

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
	unsigned int dummy;
};

static ssize_t dummy_show(struct class *class,
				struct class_attribute *attr, char *buf)
{
	struct foo_device *foo = container_of(class, struct foo_device, class);
	return sprintf(buf, "dummy: %d\n", foo->dummy);
}
static CLASS_ATTR_RO(dummy);

static struct attribute *foo_class_attrs[] = {
	&class_attr_dummy.attr,
	NULL,
};
ATTRIBUTE_GROUPS(foo_class);

static int foo_open(struct inode *inode, struct file *filp)
{
	printk("%s(): %d\n", __func__, __LINE__);
	return 0;
}

ssize_t static foo_read(struct file *filp, char __user *buf,
			size_t size, loff_t *offset)
{
	printk("%s(): %d, size: %lu\n", __func__, __LINE__, size);
	msleep(READ_INTERVAL_IN_MSEC);
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

static int foo_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
	int ret;
	struct foo_device *foo;

	printk("%s(): %d\n", __func__, __LINE__);

	foo = devm_kzalloc(&client->dev, sizeof(*foo), GFP_KERNEL);
	if (!foo)
		return -ENOMEM;

	i2c_set_clientdata(client, foo);

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

	foo->dev = device_create(&foo->class, &client->dev,
			foo->devno, NULL, DEVICE_NAME);
	if (IS_ERR(foo->dev)) {
		ret = PTR_ERR(foo->dev);
		goto err_device_create;
	}

	return 0;

err_device_create:
	class_unregister(&foo->class);

err_cdev_add:
	unregister_chrdev_region(foo->devno, MINOR_NUM);

err_alloc_chrdev:
	return ret;
}

static int foo_remove(struct i2c_client *client)
{
	struct foo_device *foo = i2c_get_clientdata(client);

	printk("%s(): %d\n", __func__, __LINE__);

	device_destroy(&foo->class, foo->devno);
	class_unregister(&foo->class);
	unregister_chrdev_region(foo->devno, MINOR_NUM);
	devm_kfree(&client->dev, foo);

	return 0;
}

static const struct i2c_device_id foo_i2c_id[] = {
	{
		.name = DEVICE_NAME,
		.driver_data = 0,
	},
	{
		/* end */
	}
};
MODULE_DEVICE_TABLE(i2c, foo_i2c_id);

static const struct of_device_id foo_of_match[] = {
	{
		.compatible = "artech, foo",
	},
	{
		.compatible = "artech, foo-dev",
	},
	{
		/* end */
	}
};
MODULE_DEVICE_TABLE(of, foo_of_match);

static struct i2c_driver foo_driver = {
	.driver = {
		.name	= DEVICE_NAME,
		.of_match_table = of_match_ptr(foo_of_match),
	},
	.probe		= foo_probe,
	.remove		= foo_remove,
	.id_table	= foo_i2c_id,
};

static int __init foo_init(void)
{
	return i2c_add_driver(&foo_driver);
}

static void __exit foo_exit(void)
{
	i2c_del_driver(&foo_driver);
}

module_init(foo_init);
module_exit(foo_exit);

MODULE_ALIAS("foo-driver");
MODULE_LICENSE("GPL");
MODULE_VERSION(VERSION);
MODULE_DESCRIPTION("Linux is not Unix");
MODULE_AUTHOR("andrew, mrju.email@gmail.com");
