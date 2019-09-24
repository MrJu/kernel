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

static char *map[] = {
	"TASK_RUNNING",
	"TASK_INTERRUPTIBLE",
	"TASK_UNINTERRUPTIBLE",
	"UNKNOWN",
};

struct task_info {
	char *name;
	int id;
	int (*func)(void *);
	struct task_struct *task;
	void *priv;
};

struct foo_drv {
	struct task_info *foo_info;
	struct task_info *bar_info;
};

static int task_func(void *data)
{
	struct task_info *task_info = data;
	struct task_struct *task = task_info->task;
	struct foo_drv *foo = task_info->priv;

	while (!kthread_should_stop()) {
		if (task_info->id == 0) {
			if (task->state < (1 << 2))
				printk("%s: %s %d %s\n",
					task_info->name,
					__func__, __LINE__, map[task->state]);

			set_current_state(TASK_INTERRUPTIBLE);
			if (task->state < (1 << 2))
				printk("%s: %s %d %s\n",
					task_info->name,
					__func__, __LINE__, map[task->state]);

			schedule_timeout(500);
			if (task->state < (1 << 2))
				printk("%s: %s %d %s\n",
					task_info->name,
					__func__, __LINE__, map[task->state]);
		} else {
			if (foo->foo_info->task->state < (1 << 2))
				printk("%s: %s %d %s\n",
					task_info->name,
					__func__, __LINE__,
					map[foo->foo_info->task->state]);
			msleep(500);
		}
	}

	return 0;
}

static int foo_probe(struct platform_device *pdev)
{
	struct foo_drv *foo;
	struct task_info *foo_info;
	struct task_info *bar_info;

	foo = devm_kzalloc(&pdev->dev, sizeof(*foo), GFP_KERNEL);
	if (!foo)
		return -ENOMEM;

	foo_info = devm_kzalloc(&pdev->dev, sizeof(*foo_info), GFP_KERNEL);
	if (!foo_info)
		return -ENOMEM;


	bar_info = devm_kzalloc(&pdev->dev, sizeof(*bar_info), GFP_KERNEL);
	if (!bar_info)
		return -ENOMEM;

	foo_info->name = "foo_task";
	foo_info->id = 0;
	foo_info->func = task_func;
	foo_info->priv = foo;

	foo_info->task = kthread_create(foo_info->func,
			(void *)foo_info, foo_info->name);
	if (IS_ERR(foo_info->task))
		return PTR_ERR(foo_info->task);

	bar_info->name = "bar_task";
	bar_info->id = 1;
	bar_info->func = task_func;
	bar_info->priv = foo;

	bar_info->task = kthread_create(bar_info->func,
			(void *)bar_info, bar_info->name);
	if (IS_ERR(bar_info->task))
		return PTR_ERR(bar_info->task);

	foo->foo_info = foo_info;
	foo->bar_info = bar_info;

	platform_set_drvdata(pdev, foo);

	wake_up_process(foo->foo_info->task);
	wake_up_process(foo->bar_info->task);

	return 0;
}

static int foo_remove(struct platform_device *pdev)
{
	struct foo_drv *foo = platform_get_drvdata(pdev);

	kthread_stop(foo->foo_info->task);
	kthread_stop(foo->bar_info->task);
	devm_kfree(&pdev->dev, foo->foo_info);
	devm_kfree(&pdev->dev, foo->bar_info);
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
