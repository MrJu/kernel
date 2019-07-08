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
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include "ringbuffer.h"

#define STR(x) _STR(x)
#define _STR(x) #x

#define VERSION_PREFIX Foo
#define MAJOR_VERSION 1
#define MINOR_VERSION 0
#define PATCH_VERSION 0

#define VERSION STR(VERSION_PREFIX-MAJOR_VERSION.MINOR_VERSION.PATCH_VERSION)

#define CLASS_NAME "foo"
#define DEVICE_NAME "foo"

#define RBUF_SIZE 1024

struct foo_device {
	struct class class;
	struct ringbuffer *ringbuf;
};

static ssize_t read_show(struct class *class,
				struct class_attribute *attr, char *buf)
{
	size_t avail, bytes;
	struct foo_device *foo;
	char * temp;

	foo = container_of(class, struct foo_device, class);

	avail = ringbuffer_avail(foo->ringbuf, RBUF_RD);

	temp = kzalloc(avail + 1, GFP_KERNEL);
	if (!temp)
		return -ENOMEM;

	ringbuffer_read(foo->ringbuf, temp, avail);

	*(temp + avail) = '\0';

	bytes = sprintf(buf, "%s\n", temp);

	kfree(temp);

	return bytes;
}
static CLASS_ATTR_RO(read);

static ssize_t write_store(struct class *class,
				struct class_attribute *attr,
				const char *buf, size_t count)
{
	size_t avail;
	struct foo_device *foo;

	foo = container_of(class, struct foo_device, class);

	avail = ringbuffer_avail(foo->ringbuf, RBUF_WR);
	if (avail < count - 1)
		return -ENOBUFS;

	ringbuffer_write(foo->ringbuf, (char *)buf, count - 1);

	return count;
}
static CLASS_ATTR_WO(write);

static struct attribute *foo_class_attrs[] = {
	&class_attr_write.attr,
	&class_attr_read.attr,
	NULL,
};
ATTRIBUTE_GROUPS(foo_class);

static int foo_probe(struct platform_device *pdev)
{
	int ret;
	struct foo_device *foo;

	foo = devm_kzalloc(&pdev->dev, sizeof(*foo), GFP_KERNEL);
	if (!foo)
		return -ENOMEM;

	foo->ringbuf = ringbuffer_create(RBUF_SIZE, GFP_KERNEL);
	if (IS_ERR(foo->ringbuf))
		return PTR_ERR(foo->ringbuf);

	foo->class.owner = THIS_MODULE;
	foo->class.name = CLASS_NAME;
	foo->class.class_groups = foo_class_groups;
	ret = class_register(&foo->class);
	if (ret) {
		ringbuffer_destroy(foo->ringbuf);
		return ret;
	}

	platform_set_drvdata(pdev, foo);

	return 0;
}

static int foo_remove(struct platform_device *pdev)
{
	struct foo_device *foo = platform_get_drvdata(pdev);

	class_unregister(&foo->class);
	ringbuffer_destroy(foo->ringbuf);
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
