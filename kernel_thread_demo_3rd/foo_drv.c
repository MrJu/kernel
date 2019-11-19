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
 *
 * Priority of a process goes from 0..MAX_PRIO-1, valid RT
 * priority is 0..MAX_RT_PRIO-1, and SCHED_NORMAL/SCHED_BATCH
 * tasks are in the range MAX_RT_PRIO..MAX_PRIO-1. Priority
 * values are inverted: lower p->prio value means higher priority.
 *
 * The MAX_USER_RT_PRIO value allows the actual maximum
 * RT priority to be separate from the value exported to
 * user-space.  This allows kernel threads to set their
 * priority to a value higher than any user task. Note:
 * MAX_RT_PRIO must not be smaller than MAX_USER_RT_PRIO.
 */

#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/signal.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>
#include <linux/sched/types.h>
#include <linux/platform_device.h>

#define STR(x) _STR(x)
#define _STR(x) #x

#define VERSION_PREFIX Foo
#define MAJOR_VERSION 1
#define MINOR_VERSION 0
#define PATCH_VERSION 0

#define VERSION STR(VERSION_PREFIX-MAJOR_VERSION.MINOR_VERSION.PATCH_VERSION)

#define DEVICE_NAME "foo"
#define INTERVAL_IN_MSEC 1000

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
	struct foo_drv *foo = task_info->priv;
	struct sched_param sched_param;

	if (task_info->id == 0)
		sched_param.sched_priority = MAX_RT_PRIO - 1;
	else
		sched_param.sched_priority = MAX_RT_PRIO / 2;

	sched_setscheduler(current, SCHED_RR, &sched_param);

	while (!kthread_should_stop()) {
		printk("%s, id %d, pid %d, cpu id %d, preempt_count %d\n",
			current->comm, task_info->id, current->pid,
			raw_smp_processor_id(), preempt_count());

		if (task_info->id == 0)
			mdelay(INTERVAL_IN_MSEC);
		else
			mdelay(INTERVAL_IN_MSEC);
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
