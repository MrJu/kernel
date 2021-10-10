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
#define NR_DUMMY 128

struct foo {
	int a;
	int b;
};

struct dummy {
	struct foo __percpu *foo;
	int bar;
	int baz;
};

static int foo_probe(struct platform_device *pdev)
{
	int i, a, b, ret = 0;
	struct dummy *dummy;

	dummy = kzalloc(sizeof(*dummy), GFP_KERNEL);
	if (!dummy) {
		ret = -ENOMEM;
		goto err0;
	}

	dummy->foo = alloc_percpu(struct foo);
	if (!dummy->foo) {
		ret = -ENOMEM;
		goto err1;
	}

	preempt_disable();
	for_each_possible_cpu(i) {
		per_cpu_ptr(dummy->foo, i)->a = i;
		per_cpu_ptr(dummy->foo, i)->b = i;
	}
	preempt_enable();

	dummy->bar = 0x1;
	dummy->baz = 0x2;

	preempt_disable();
	for_each_possible_cpu(i) {
		a = per_cpu_ptr(dummy->foo, i)->a;
		b = per_cpu_ptr(dummy->foo, i)->b;;
		pr_info("%s: %d cpu:%d "
			"dummy->foo->a:%d "
			"dummy->foo->b:%d\n",
			__func__, __LINE__, i, a, b);
	}
	preempt_enable();

	preempt_disable();
	a = this_cpu_read(dummy->foo->a);
	b = this_cpu_read(dummy->foo->b);
	pr_info("%s: %d cpu:%d a:%d b:%d\n",
			__func__, __LINE__,
			smp_processor_id(), a, b);
	preempt_enable();

err1:
	kfree(dummy);

err0:
	return ret;
}

static int foo_remove(struct platform_device *pdev)
{
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
	int ret;
	struct platform_device *pdev;

	pdev = platform_device_register_simple(DEVICE_NAME, -1, NULL, 0);
	if (IS_ERR(pdev))
		return PTR_ERR(pdev);

	ret = platform_driver_register(&foo_drv);
	if (ret) {
		platform_device_unregister(pdev);
		return ret;
	}

	return 0;
}

static void __exit foo_exit(void)
{
	struct device *dev;

	dev = bus_find_device_by_name(foo_drv.driver.bus, NULL, DEVICE_NAME);
	if (dev)
		platform_device_unregister(to_platform_device(dev));

	platform_driver_unregister(&foo_drv);
}

module_init(foo_init);
module_exit(foo_exit);

MODULE_ALIAS("foo-driver");
MODULE_LICENSE("GPL");
MODULE_VERSION(VERSION);
MODULE_DESCRIPTION("Linux is not Unix");
MODULE_AUTHOR("andrew, mrju.email@gmail.com");
