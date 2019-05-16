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
#include <linux/workqueue.h>
#include <linux/delay.h>
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
#define INTERVAL_IN_MSEC 10000

struct foo_work{
	struct kobject kobj;
	struct work_struct work;
	unsigned int callback_count;
};

static void foo_work_callback(struct work_struct *work)
{
	printk("%s ,cpu id = %d,taskname = %s\n",
			__func__,raw_smp_processor_id(),current->comm);
	msleep(INTERVAL_IN_MSEC);
}

static ssize_t workqueue_show(struct kobject *kobj,
				struct kobj_attribute *attr, char *buf)
{
	struct foo_work *foo;
	foo = container_of(kobj, struct foo_work, kobj);
	return sprintf(buf, "%u\n", foo->callback_count);
}

static ssize_t workqueue_store(struct kobject *kobj,
				struct kobj_attribute *attr, const char *buf, size_t count)
{
	int ret, val;
	struct foo_work *foo;

	ret = kstrtouint(buf, 10, &val);
	if (ret < 0)
		return ret;

	foo = container_of(kobj, struct foo_work, kobj);

	if (val) {
		schedule_work(&foo->work);
		foo->callback_count++;
	}

	return count;
}

static struct kobj_attribute workqueue_attribute =
		__ATTR(workqueue, 0664, workqueue_show, workqueue_store);

static struct attribute *foo_attrs[]= {
	&workqueue_attribute.attr,
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
	struct foo_work *foo;

	foo = devm_kzalloc(&pdev->dev, sizeof(*foo), GFP_KERNEL);
	if (!foo)
		return -ENOMEM;

	kobject_init(&foo->kobj, &kobj_type);
	ret = kobject_add(&foo->kobj, NULL, "workqueue");
	if (ret)
		goto err_kobject_add;

	ret = sysfs_create_group(&foo->kobj, &foo_attr_group);
	if(ret)
		goto err_sysfs_create_group;

	INIT_WORK(&foo->work, foo_work_callback);

	platform_set_drvdata(pdev, foo);

	return 0;

err_sysfs_create_group:
	kobject_del(&foo->kobj);

err_kobject_add:
	return ret;
}

static int foo_remove(struct platform_device *pdev)
{
	struct foo_work *foo = platform_get_drvdata(pdev);
	kobject_del(&foo->kobj);
	sysfs_remove_group(&foo->kobj, &foo_attr_group);
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
