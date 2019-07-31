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
#include <linux/slab.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
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

struct task_info {
	char *name;
	int id;
	int (*func)(void *);
	struct task_struct *task;
	wait_queue_head_t wait_head;
	wait_queue_entry_t wait;
	int trigger;
	struct proc_dir_entry *proc_dir;
};

static int task_func(void *data)
{
	struct task_info *task_info = data;

	while (1) {
		wait_event_interruptible(task_info->wait_head,
				task_info->trigger || kthread_should_stop());
		if(kthread_should_stop())
			break;

		printk("%s(): name is %s, id is %d\n", __func__,
				task_info->name, task_info->id);

		task_info->trigger = 0;
	}

	return 0;
}

static int foo_proc_show(struct seq_file *m, void *v) {
	struct task_info *task_info = PDE_DATA(file_inode(m->file));
	seq_printf(m, "%s\n", task_info->trigger? "enable" : "disable");
	return 0;
}

static int foo_proc_open(struct inode *inode, struct  file *file) {
	return single_open(file, foo_proc_show, NULL);
}

static ssize_t foo_proc_write(struct file *filp, const char __user *buf,
			size_t size, loff_t *offset)
{
	int err, val;
	struct task_info *task_info = PDE_DATA(file_inode(filp));
	char *temp = kzalloc(size, GFP_KERNEL);
	if (!temp)
		return -ENOMEM;

	if (copy_from_user(temp, buf, size)) {
		kfree(temp);
		return -EFAULT;
	}

	err = kstrtouint(temp, 10, &val);
	if (err) {
		kfree(temp);
		return err;
	}

	if (!!val)
		task_info->trigger = 1;
	else
		task_info->trigger = 0;

	if (task_info->trigger)
		wake_up_interruptible(&task_info->wait_head);

	kfree(temp);

	return size;
}

static struct file_operations foo_proc_fops = {
	.owner = THIS_MODULE,
	.open = foo_proc_open,
	.read = seq_read,
	.write = foo_proc_write,
	.llseek = seq_lseek,
	.release = single_release,
};

static int foo_probe(struct platform_device *pdev)
{
	struct task_info *task_info;
	struct proc_dir_entry *trigger;

	task_info = devm_kzalloc(&pdev->dev, sizeof(*task_info), GFP_KERNEL);
	if (!task_info)
		return -ENOMEM;

	task_info->name = KTHREAD_NAME;
	task_info->id = 0;
	task_info->func = task_func;
	task_info->trigger = 0;

	init_waitqueue_head(&task_info->wait_head);

	task_info->task = kthread_create(task_info->func,
			(void *)task_info, task_info->name);
	if (IS_ERR(task_info->task))
		return PTR_ERR(task_info->task);

	task_info->proc_dir = proc_mkdir("waitqueue", NULL);
	if (!task_info->proc_dir)
		return -ENOMEM;

	trigger = proc_create_data("trigger",
			S_IRUSR | S_IRGRP | S_IROTH,
			task_info->proc_dir, &foo_proc_fops, task_info);
	if (!trigger) {
		remove_proc_entry("waitqueue", NULL);
		return -ENOMEM;
	}

	platform_set_drvdata(pdev, task_info);

	wake_up_process(task_info->task);

	return 0;
}

static int foo_remove(struct platform_device *pdev)
{
	struct task_info *task_info = platform_get_drvdata(pdev);

	kthread_stop(task_info->task);
	remove_proc_entry("trigger", task_info->proc_dir);
	remove_proc_entry("waitqueue", NULL);
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
