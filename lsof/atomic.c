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
#include <linux/sched/signal.h>
#include <linux/fdtable.h>
#include <linux/platform_device.h>
#include <linux/of.h>

#define STR(x) _STR(x)
#define _STR(x) #x

#define VERSION_PREFIX atomic
#define MAJOR_VERSION 1
#define MINOR_VERSION 0
#define PATCH_VERSION 0
#define VERSION STR(VERSION_PREFIX-MAJOR_VERSION.MINOR_VERSION.PATCH_VERSION)
#define DEVICE_NAME "atomic"

#define NR_TASKS 1
#define KTHREAD_NAME "atomic-task"
#define INTERVAL_IN_MSECS 8000

struct task_info {
	char *name;
	int id;
	int (*func)(void *);
	struct task_struct *task;
};

int __lsof(const void *task, struct file *file, unsigned nr)
{
	char *path, tmp[256];
	struct task_struct *t = (struct task_struct *) task;

	path = file_path(file, tmp, 256);
	if (IS_ERR(path))
		return PTR_ERR(path);

	pr_info("%d\t%s\t\t%s\n",
			task_pid_nr(t),
			t->comm,
			path);

	return 0;
}

int lsof(void)
{
	struct task_struct *task;

	rcu_read_lock();
	for_each_process(task) {
		task_lock(task);
		iterate_fd(task->files, 0, __lsof, task);
		task_unlock(task);
	}
	rcu_read_unlock();

	return 0;
}

static int task_func(void *data)
{
	while (!kthread_should_stop()) {
		lsof();
		msleep(INTERVAL_IN_MSECS);
	}

	return 0;
}

static int atomic_probe(struct platform_device *pdev)
{
	int i, ret;
	struct task_info *task_info, *info;

	task_info = devm_kzalloc(&pdev->dev,
			sizeof(*task_info) * NR_TASKS,
			GFP_KERNEL);
	if (!task_info)
		return -ENOMEM;

	platform_set_drvdata(pdev, task_info);

	for (i = 0, info = task_info;
			i < NR_TASKS; i++, info++) {
		info->name = KTHREAD_NAME;
		info->id = i;
		info->func = task_func;
		info->task = kthread_create(info->func,
			(void *)info, info->name);
		if (IS_ERR(info->task)) {
			ret = PTR_ERR(info->task);
			goto err;
		}

		kthread_bind(info->task, 1);
		wake_up_process(info->task);
	}

	return 0;

err:
	for (info--; i > 0; i--, info--)
		kthread_stop(info->task);

	return ret;
}

static int atomic_remove(struct platform_device *pdev)
{
	int i;
	struct task_info *task_info, *info;

	task_info = platform_get_drvdata(pdev);

	for (i = 0, info = task_info;
			i < NR_TASKS; i++, info++)
		kthread_stop(info->task);

	devm_kfree(&pdev->dev, task_info);

	return 0;
}

static struct platform_driver atomic_drv = {
	.probe	= atomic_probe,
	.remove	= atomic_remove,
	.driver	= {
		.owner = THIS_MODULE,
		.name = DEVICE_NAME,
	}
};

static int __init atomic_init(void)
{
	int ret;
	struct platform_device *pdev;

	pdev = platform_device_register_simple(DEVICE_NAME, -1, NULL, 0);
	if (IS_ERR(pdev))
		return PTR_ERR(pdev);

	ret = platform_driver_register(&atomic_drv);
	if (ret) {
		platform_device_unregister(pdev);
		return ret;
	}

	return 0;
}

static void __exit atomic_exit(void)
{
	struct device *dev;

	dev = bus_find_device_by_name(atomic_drv.driver.bus, NULL, DEVICE_NAME);
	if (dev)
		platform_device_unregister(to_platform_device(dev));

	platform_driver_unregister(&atomic_drv);
}

module_init(atomic_init);
module_exit(atomic_exit);

MODULE_ALIAS("atomic-driver");
MODULE_LICENSE("GPL");
MODULE_VERSION(VERSION);
MODULE_DESCRIPTION("Linux is not Unix");
MODULE_AUTHOR("andrew, mrju.email@gmail.com");
