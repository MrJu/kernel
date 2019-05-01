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
#include <linux/slab.h>
#include <linux/platform_device.h>

#define STR(x) _STR(x)
#define _STR(x) #x

#define VERSION_PREFIX Foo
#define MAJOR_VERSION 1
#define MINOR_VERSION 0
#define PATCH_VERSION 0

#define VERSION STR(VERSION_PREFIX-MAJOR_VERSION.MINOR_VERSION.PATCH_VERSION)

#define DEVICE_NAME "foo"

struct foo_device {
	struct kobject kobj;
	struct kset *kset;
	int foo;
};
#define kobj_to_foo(kobj) container_of(kobj, struct foo_device, kobj)

static ssize_t attr_show(struct kobject *kobj,
				struct kobj_attribute *attr, char *buf)
{
	struct foo_device *foo = kobj_to_foo(kobj);
	return sprintf(buf, "%d\n", foo->foo);
}

static ssize_t attr_store(struct kobject *kobj,
				struct kobj_attribute *attr, const char *buf, size_t count)
{
	int ret;
	struct foo_device *foo = kobj_to_foo(kobj);

	ret = kstrtoint(buf, 10, &foo->foo);
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

static void dynamic_kobj_release(struct kobject *kobj)
{
	kfree(kobj);
}

static struct kobj_type kobj_type = {
	.release	= dynamic_kobj_release,
	.sysfs_ops	= &kobj_sysfs_ops,
};

static int foo_probe(struct platform_device *pdev)
{
	int ret;
	struct foo_device *foo;

	foo = kzalloc(sizeof(*foo), GFP_KERNEL);
	if (!foo) {
		ret = -ENOMEM;
		goto err_dev_kzallc;
	}

	foo->kset = kset_create_and_add("foo_kset", NULL, NULL);
	if (!foo->kset) {
		ret = -ENOMEM;
		goto err_create_foo_kset;
	}

	kobject_init(&foo->kobj, &kobj_type);
	ret = kobject_add(&foo->kobj, &foo->kset->kobj, "foo-kobj");
	if (ret)
		goto err_kobject_add;

	ret = sysfs_create_group(&foo->kobj, &foo_attr_group);
	if(ret)
		goto err_sysfs_create_group;

	platform_set_drvdata(pdev, &foo->kobj);

	return 0;

err_sysfs_create_group:
	kobject_del(&foo->kobj);

err_kobject_add:
	kset_unregister(foo->kset);

err_create_foo_kset:
	kfree(foo);

err_dev_kzallc:
	return ret;
}

static int foo_remove(struct platform_device *pdev)
{
	struct foo_device *foo = platform_get_drvdata(pdev);
	kobject_del(&foo->kobj);
	sysfs_remove_group(&foo->kobj, &foo_attr_group);
	kset_unregister(foo->kset);
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
