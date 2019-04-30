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

#define CLASS_NAME "foo"
#define DEVICE_NAME "foo"

static unsigned int enable = 0;

static ssize_t version_show(struct class *class,
				struct class_attribute *attr, char *buf)
{
	return sprintf(buf, "%s\n", VERSION);
}
static CLASS_ATTR_RO(version);

static ssize_t enable_show(struct class *class,
				struct class_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", enable? 1 : 0);
}

static ssize_t enable_store(struct class *class,
				struct class_attribute *attr,
				const char *buf, size_t count)
{
	int err, val;

	err = kstrtouint(buf, 10, &val);
	if (err)
		return err;

	if (val)
		enable = 1;
	else
		enable = 0;

	return count;
}
static CLASS_ATTR_RW(enable);

static struct attribute *foo_class_attrs[] = {
	&class_attr_version.attr,
	&class_attr_enable.attr,
	NULL,
};
ATTRIBUTE_GROUPS(foo_class);

static struct class foo_class = {
	.name	= CLASS_NAME,
	.owner	= THIS_MODULE,
	.class_groups = foo_class_groups,
};

static int foo_probe(struct platform_device *pdev)
{
	printk("%s(): %d\n", __func__, __LINE__);
	return class_register(&foo_class);
}

static int foo_remove(struct platform_device *pdev)
{
	printk("%s(): %d\n", __func__, __LINE__);
	class_unregister(&foo_class);
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
