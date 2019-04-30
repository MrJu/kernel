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

#define MS_TO_NS(x) (x * 1E6L)
#define DELAY_MS 1L

static unsigned int callback_count = 0;
static struct hrtimer timer;
ktime_t kt;

static ssize_t callback_count_show(struct class *class,
				struct class_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", callback_count);
}
static CLASS_ATTR_RO(callback_count);

static struct attribute *foo_class_attrs[] = {
	&class_attr_callback_count.attr,
	NULL,
};
ATTRIBUTE_GROUPS(foo_class);

static struct class foo_class = {
	.name	= CLASS_NAME,
	.owner	= THIS_MODULE,
	.class_groups = foo_class_groups,
};

static enum hrtimer_restart  hrtimer_handler(struct hrtimer *timer)
{
	callback_count++;
	hrtimer_forward_now(timer, kt);

	return HRTIMER_RESTART;
}

static int foo_probe(struct platform_device *pdev)
{
	int err;

	printk("%s(): %d\n", __func__, __LINE__);

	err = class_register(&foo_class);
	if (err)
		return err;

	kt = ktime_set(0, MS_TO_NS(DELAY_MS));
	hrtimer_init(&timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	timer.function = hrtimer_handler;
	hrtimer_start(&timer, kt, HRTIMER_MODE_REL);

	return 0;
}

static int foo_remove(struct platform_device *pdev)
{
	printk("%s(): %d\n", __func__, __LINE__);

	hrtimer_cancel(&timer);
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
