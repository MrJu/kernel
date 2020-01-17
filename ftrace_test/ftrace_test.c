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
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/version.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/delay.h>
#include <linux/uaccess.h>

#define STR(x) _STR(x)
#define _STR(x) #x

#define VERSION_PREFIX Ftrace-Test
#define MAJOR_VERSION 1
#define MINOR_VERSION 0
#define PATCH_VERSION 0

#define VERSION STR(VERSION_PREFIX-MAJOR_VERSION.MINOR_VERSION.PATCH_VERSION)

static unsigned int variable;
static struct proc_dir_entry *test_dir, *test_entry;

static noinline void kill_moretime(void)
{
	mdelay(2);
}

static noinline void kill_time(void)
{
	mdelay(2);
	kill_moretime();
}

static int ftrace_test_show(struct seq_file *seq, void *v)
{
	unsigned int *ptr_var = seq->private;

	kill_time();
	seq_printf(seq, "%u\n", *ptr_var);

	return 0;
}

static ssize_t ftrace_test_write(struct file *file,
		const char __user *buffer,
		size_t count, loff_t *ppos)
{
	struct seq_file *seq = file->private_data;
	unsigned int *ptr_var = seq->private;
	char *kbuffer;
	int err;

	if (!buffer || count > PAGE_SIZE - 1)
		return -EINVAL;

	kbuffer = (char *)__get_free_page(GFP_KERNEL);
	if (!kbuffer)
		return -ENOMEM;

	if (copy_from_user(kbuffer, buffer, count)) {
		err = -EFAULT;
		goto out;
	}

	kbuffer[count] = '\0';

	*ptr_var = simple_strtoul(kbuffer, NULL, 10);

	return count;

out:
	free_page((unsigned long)buffer);

	return err;
}

static int ftrace_test_open(struct inode *inode, struct file *file)
{
	return single_open(file, ftrace_test_show, PDE_DATA(inode));
}

static const struct file_operations ftrace_test_fops =
{
	.owner = THIS_MODULE,
	.open = ftrace_test_open,
	.read = seq_read,
	.write = ftrace_test_write,
	.llseek = seq_lseek,
	.release = single_release,
};

static __init int ftrace_test_init(void)
{
	test_dir = proc_mkdir("test_dir", NULL);
	if (!test_dir)
		return -ENOMEM;

	test_entry = proc_create_data("test_rw", 0666,
			test_dir, &ftrace_test_fops, &variable);
	if (!test_entry) {
		remove_proc_entry("test_dir", NULL);
		return -ENOMEM;
	}

	return 0;
}

static __exit void ftrace_test_exit(void)
{
	remove_proc_entry("test_rw", test_dir);
	remove_proc_entry("test_dir", NULL);
}

module_init(ftrace_test_init);
module_exit(ftrace_test_exit);

MODULE_ALIAS("ftrace-test-driver");
MODULE_LICENSE("GPL");
MODULE_VERSION(VERSION);
MODULE_DESCRIPTION("Linux is not Unix");
MODULE_AUTHOR("andrew, mrju.email@gmail.com");
