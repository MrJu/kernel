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
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/platform_device.h>

#define STR(x) _STR(x)
#define _STR(x) #x

#define VERSION_PREFIX Foo
#define MAJOR_VERSION 1
#define MINOR_VERSION 0
#define PATCH_VERSION 0

#define VERSION STR(VERSION_PREFIX-MAJOR_VERSION.MINOR_VERSION.PATCH_VERSION)

#define DEVICE_NAME "foo"
#define KTHREAD_NAME "foo-task"
#define INTERVAL_IN_MSEC 500

struct task_info {
	char *name;
	int id;
	int (*func)(void *);
	struct task_struct *task;
};

static int task_func(void *data)
{
	struct task_info *task_info = data;

	while (!kthread_should_stop()) {
		printk("%s(): name is %s, id is %d\n", __func__,
				task_info->name, task_info->id);
		msleep(INTERVAL_IN_MSEC);
	}

	return 0;
}

static int foo_probe(struct platform_device *pdev)
{
	struct task_info *task_info;

	task_info = devm_kzalloc(&pdev->dev, sizeof(*task_info), GFP_KERNEL);
	if (!task_info)
		return -ENOMEM;

	task_info->name = KTHREAD_NAME;
	task_info->id = 0;
	task_info->func = task_func;

	task_info->task = kthread_create(task_info->func,
			(void *)task_info, task_info->name);
	if (IS_ERR(task_info->task))
		return PTR_ERR(task_info->task);

	platform_set_drvdata(pdev, task_info);

	wake_up_process(task_info->task);

	return 0;
}

static int foo_remove(struct platform_device *pdev)
{
	struct task_info *task_info = platform_get_drvdata(pdev);

	kthread_stop(task_info->task);
	devm_kfree(&pdev->dev, task_info);

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
