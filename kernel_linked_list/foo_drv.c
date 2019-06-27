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
#include <linux/debugfs.h>
#include <linux/list.h>
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

#define PRINTK_TAG "FOO"
#define PRINTK(format, ...) printk(PRINTK_TAG ": " format, ##__VA_ARGS__)

#define assert(format, ...)\
			PRINTK("%s, %d, %s, " format,\
				__FILE__, __LINE__, __func__, ##__VA_ARGS__);

struct val_node {
	struct list_head list;
	int val;
};

static struct list_head val_list = LIST_HEAD_INIT(val_list);

static struct dentry *linked_list_dentry;
static struct dentry *add_head_dentry;
static struct dentry *add_tail_dentry;
static struct dentry *del_head_dentry;
static struct dentry *del_tail_dentry;
static struct dentry *show_dentry;

static struct val_node *val_node_alloc(int val)
{
	struct val_node *node;

	node = kmalloc(sizeof(*node), GFP_KERNEL);
	/* I don't think it is neccessary */
	/* INIT_LIST_HEAD(&node->list); */
	node->val = val;

	return node;
}

static void val_list_add_head(int val)
{
	struct val_node *node = val_node_alloc(val);

	list_add(&node->list, &val_list);
}

static void val_list_add_tail(int val)
{
	struct val_node *node = val_node_alloc(val);

	list_add_tail(&node->list, &val_list);
}

static void val_list_del_head(void)
{
	struct list_head *list = val_list.next;
	struct val_node *node = list_entry(list, struct val_node, list);

	list_del(list);
	kfree(node);
}

static void val_list_del_tail(void)
{
	struct list_head *list = val_list.prev;
	struct val_node *node = list_entry(list, struct val_node, list);

	list_del(list);
	kfree(node);
}

static int val_list_show(struct seq_file *m, void *v)
{
	struct val_node *node;

	list_for_each_entry(node, &val_list, list) {
		seq_printf(m, "%d ", node->val);
	}
	seq_printf(m, "\n");

	return 0;
}

static void val_list_release(void)
{
	struct val_node *node, *n;

	list_for_each_entry_safe(node, n, &val_list, list)
		kfree(node);
}

static ssize_t add_head_store(struct file *file, const char __user *buf,
		size_t size, loff_t *ppos)
{
	int n;

	if (sscanf(buf, "%d", &n) != 1)
		return -EINVAL;

	val_list_add_head(n);

	return size;
}

static ssize_t add_tail_store(struct file *file, const char __user *buf,
		size_t size, loff_t *ppos)
{
	int n;

	if (sscanf(buf, "%d", &n) != 1)
		return -EINVAL;

	val_list_add_tail(n);

	return size;
}

static ssize_t del_head_store(struct file *file, const char __user *buf,
		size_t size, loff_t *ppos)
{
	bool val;
	int ret;

	ret = strtobool(buf, &val);
	if (ret < 0)
		return ret;

	if (val == false)
		return -EINVAL;

	if (list_empty(&val_list))
		return -ENOSPC;

	val_list_del_head();

	return size;
}

static ssize_t del_tail_store(struct file *file, const char __user *buf,
		size_t size, loff_t *ppos)
{
	bool val;
	int ret;

	ret = strtobool(buf, &val);
	if (ret < 0)
		return ret;

	if (val == false)
		return -EINVAL;

	if (list_empty(&val_list))
		return -ENOSPC;

	val_list_del_tail();

	return size;
}

static const struct file_operations add_head_fops = {
	.write = add_head_store,
};

static const struct file_operations add_tail_fops = {
	.write = add_tail_store,
};

static const struct file_operations del_head_fops = {
	.write = del_head_store,
};

static const struct file_operations del_tail_fops = {
	.write = del_tail_store,
};

static int show_open(struct inode *inode, struct file *file)
{
	return single_open(file, val_list_show, inode->i_private);
}

static const struct file_operations show_fops = {
	.open = show_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static int foo_probe(struct platform_device *pdev)
{
	/* to create /sys/kernel/debug/linked_list */
	linked_list_dentry = debugfs_create_dir("linked_list", NULL);

	if (!linked_list_dentry) {
		assert("failed to create linked_list\n");
		return -ENOMEM;
	}

	add_head_dentry = debugfs_create_file("add_head", S_IWUGO,
			linked_list_dentry, NULL, &add_head_fops);

	if (!add_head_dentry) {
		assert("failed to create add_head\n");
		return -ENOMEM;
	}

	add_tail_dentry = debugfs_create_file("add_tail", S_IWUGO,
			linked_list_dentry, NULL, &add_tail_fops);

	if (!add_tail_dentry) {
		assert("failed to create add_tail\n");
		return -ENOMEM;
	}

	del_head_dentry = debugfs_create_file("del_head", S_IWUGO,
			linked_list_dentry, NULL, &del_head_fops);

	if (!del_head_dentry) {
		assert("failed to create del_head\n");
		return -ENOMEM;
	}

	del_tail_dentry = debugfs_create_file("del_tail", S_IWUGO,
			linked_list_dentry, NULL, &del_tail_fops);

	if (!del_tail_dentry) {
		assert("failed to create del_tail\n");
		return -ENOMEM;
	}

	show_dentry = debugfs_create_file("show", S_IRUGO,
			linked_list_dentry, NULL, &show_fops);

	if (!show_dentry) {
		assert("failed to create show\n");
		return -ENOMEM;
	}

	return 0;
}

static int foo_remove(struct platform_device *pdev)
{
	debugfs_remove(show_dentry);
	debugfs_remove(add_head_dentry);
	debugfs_remove(add_tail_dentry);
	debugfs_remove(del_head_dentry);
	debugfs_remove(del_tail_dentry);
	debugfs_remove(linked_list_dentry);
	val_list_release();

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
