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
#include <linux/platform_device.h>

#define STR(x) _STR(x)
#define _STR(x) #x

#define VERSION_PREFIX Foo
#define MAJOR_VERSION 1
#define MINOR_VERSION 0
#define PATCH_VERSION 0

#define VERSION STR(VERSION_PREFIX-MAJOR_VERSION.MINOR_VERSION.PATCH_VERSION)

#define DEVICE_NAME "foo"

static int foo;

static ssize_t attr_show(struct kobject *kobj,
		struct kobj_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", foo);
}

static ssize_t attr_store(struct kobject *kobj,
		struct kobj_attribute *attr, const char *buf, size_t count)
{
	int ret;

	ret = kstrtoint(buf, 10, &foo);
	if (ret < 0)
		return ret;

	return count;
}

static struct kobj_attribute x_attribute =
		__ATTR(x_attr, 0664, attr_show, attr_store);
static struct kobj_attribute y_attribute =
		__ATTR(y_attr, 0664, attr_show, attr_store);
static struct kobj_attribute z_attribute =
		__ATTR(z_attr, 0664, attr_show, attr_store);

static struct attribute *foo_attrs[]= {
	&x_attribute.attr,
	&y_attribute.attr,
	&z_attribute.attr,
	NULL,
};

static struct attribute_group foo_attr_group = {
	.attrs = foo_attrs,
};

static int foo_probe(struct platform_device *pdev)
{
	int ret;
	struct kobject *foo_kobj;

	foo_kobj = kobject_create_and_add("foo_kobj", NULL);
	if(!foo_kobj) {
		ret = -ENOMEM;
		goto err_create_foo_kobj;
	}

	ret = sysfs_create_group(foo_kobj, &foo_attr_group);
	if(ret)
		goto err_sysfs_create_group;

	platform_set_drvdata(pdev, foo_kobj);

	return 0;

err_sysfs_create_group:
	kobject_put(foo_kobj);

err_create_foo_kobj:
	return ret;
}

static int foo_remove(struct platform_device *pdev)
{
	struct kobject *foo_kobj = platform_get_drvdata(pdev);
	sysfs_remove_group(foo_kobj, &foo_attr_group);
	kobject_put(foo_kobj);
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
