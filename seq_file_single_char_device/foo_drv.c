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

struct foo_device {
	dev_t devno;
	struct cdev *cdev;
	struct class *class;
	struct device *dev;
};

static int foo_show(struct seq_file *m, void *v) {
	seq_printf(m, "%s\n", VERSION);
	return 0;
}

static int foo_open(struct inode *inode, struct  file *file) {
	return single_open(file, foo_show, NULL);
}

static const struct file_operations foo_fops = {
	.owner = THIS_MODULE,
	.open = foo_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
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

	foo->dev = device_create(foo->class, &pdev->dev, foo->devno, NULL, DEVICE_NAME);
	if (IS_ERR(foo->dev)) {
		ret = PTR_ERR(foo->dev);
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
	struct foo_device *foo;

	printk("%s(): %d\n", __func__, __LINE__);

	foo = platform_get_drvdata(pdev);
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
