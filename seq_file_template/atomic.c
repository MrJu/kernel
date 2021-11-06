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
#include <linux/sched.h>
#include <linux/of.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/list.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/atomic.h>

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
#define INTERVAL_IN_MSECS 10000
#define ENTRY_NAME "atomic"
#define NR_ENTRIES 128
#define NODE_OPS_MAGIC "ATOMIC_PARAMS"

struct task_info {
	char *name;
	int id;
	int (*func)(void *);
	struct task_struct *task;
};

/* MUST be the first item in the container */
struct node_ops {
	char *magic;
	int (*execute)(char **args, int nr, void *data);
	struct seq_operations *seq_ops;
};

struct queue_entry {
	int index;
	struct list_head list;
};

struct queue_buffer {
	struct node_ops node_ops;
	atomic_t enable;
	struct proc_dir_entry *entry;
	struct list_head head;
	struct mutex lock;
	struct queue_entry entries[NR_ENTRIES];
};

static int task_func(void *data)
{
	struct task_info *task_info = data;
	struct queue_buffer *qbuf = (struct queue_buffer *)(task_info
					+ sizeof(*task_info) * NR_TASKS);
	while (!kthread_should_stop()) {
		pr_info("%s: %d enable:%d\n",
				__func__, __LINE__, atomic_read(&qbuf->enable));

		msleep(INTERVAL_IN_MSECS);
	}

	return 0;
}

static int atomic_node_exec(char **params, int nr, void *data)
{
	struct queue_buffer *qbuf = data;
	unsigned long int val;
	int ret;

	if (!strcmp(params[0], "enable")) {
		if (nr < 2)
			return -EINVAL;
		ret = kstrtoul(params[1], 10, &val);
		if (ret)
			return ret;

		atomic_set(&qbuf->enable, !!val);
	}

	return 0;
}

static void *atomic_start(struct seq_file *m, loff_t *pos)
{
	struct queue_buffer *qbuf = PDE_DATA(file_inode(m->file));
	mutex_lock(&qbuf->lock);
	return seq_list_start(&qbuf->head, *pos);
}

static void *atomic_next(struct seq_file *m, void *p, loff_t *pos)
{
	struct queue_buffer *qbuf = PDE_DATA(file_inode(m->file));
	return seq_list_next(p, &qbuf->head, pos);
}

static void atomic_stop(struct seq_file *m, void *p)
{
	struct queue_buffer *qbuf = PDE_DATA(file_inode(m->file));
	mutex_unlock(&qbuf->lock);
}

static int atomic_show(struct seq_file *m, void *p)
{
	struct queue_entry *entry = list_entry(p, struct queue_entry, list);
	seq_printf(m, "%s: %d index\n", __func__, __LINE__, entry->index);
	return 0;
}

static struct seq_operations atomic_seq_ops = {
	.start	= atomic_start,
	.next	= atomic_next,
	.stop	= atomic_stop,
	.show	= atomic_show
};

static int common_node_open(struct inode *inode, struct file *file)
{
	struct node_ops *ops = PDE_DATA(file_inode(file));
	return seq_open(file, ops->seq_ops);
}

static int common_node_exec(char *args, void *data)
{
	struct node_ops *ops = data;
	char *split, *params[4];
	int i, nr = 0;

	for (i = 0; args && (i < 4); args = split, i++) {
		split = strpbrk(args, " \n");
		if (!split)
			break;

		*split++ = '\0';
		args = skip_spaces(args);
		params[i] = args;
		nr++;
	}

	BUG_ON(strcmp(ops->magic, NODE_OPS_MAGIC));

	return ops->execute(params, nr, data);
}

static ssize_t common_node_write(struct file *file,
				const char __user *buf,
				size_t size, loff_t *offset)
{
	char *tmp;
	int ret;
	void *data = PDE_DATA(file_inode(file));

	if (size == 0)
		return 0;
	if (size > PAGE_SIZE - 1)
		return -E2BIG;

	tmp = memdup_user_nul(buf, size);
	if (IS_ERR(tmp))
		return PTR_ERR(tmp);

	ret = common_node_exec(tmp, data);
	kfree(tmp);
	if (ret < 0)
		return ret;

	*offset += size;
	return size;
}

static struct file_operations common_node_fops = {
	.open		= common_node_open,
	.read		= seq_read,
	.write		= common_node_write,
	.llseek		= seq_lseek,
	.release	= seq_release,
};

static int atomic_probe(struct platform_device *pdev)
{
	int i, ret;
	struct task_info *task_info, *info;
	struct queue_buffer *qbuf;

	task_info = devm_kzalloc(&pdev->dev,
			sizeof(*task_info) * NR_TASKS
				+ sizeof(struct queue_buffer),
			GFP_KERNEL);
	if (!task_info) {
		ret = -ENOMEM;
		goto err0;
	}

	qbuf = (struct queue_buffer *)(task_info
			+ sizeof(*task_info) * NR_TASKS);
	qbuf->node_ops.magic = NODE_OPS_MAGIC;
	qbuf->node_ops.execute = atomic_node_exec;
	qbuf->node_ops.seq_ops = &atomic_seq_ops;
	atomic_set(&qbuf->enable, 0);
	mutex_init(&qbuf->lock);
	INIT_LIST_HEAD(&qbuf->head);

	qbuf->entry = proc_create_data(ENTRY_NAME,
			S_IRUSR | S_IRGRP | S_IROTH,
			NULL, &common_node_fops, qbuf);
	if (!qbuf->entry) {
		ret = -ENOMEM;
		goto err1;
	}

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
			goto err2;
		}

		kthread_bind(info->task, 1 + i % 3);
		wake_up_process(info->task);
	}

	return 0;

err2:
	for (info--; i > 0; i--, info--)
		kthread_stop(info->task);

	remove_proc_entry(ENTRY_NAME, NULL);

err1:
	devm_kfree(&pdev->dev, task_info);

err0:
	return ret;
}

static int atomic_remove(struct platform_device *pdev)
{
	int i;
	struct task_info *task_info, *info;

	remove_proc_entry(ENTRY_NAME, NULL);

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
