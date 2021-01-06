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
#include <linux/init.h>
#include <linux/percpu.h>
#include <linux/cpumask.h>

#define STR(x) _STR(x)
#define _STR(x) #x

#define VERSION_PREFIX Foo
#define MAJOR_VERSION 1
#define MINOR_VERSION 0
#define PATCH_VERSION 0

#define VERSION STR(VERSION_PREFIX-MAJOR_VERSION.MINOR_VERSION.PATCH_VERSION)

static DEFINE_PER_CPU(int, cpuvar) = 0;
static int __percpu *cpualloc;

static int __init foo_init(void)
{
	int cpu;

	for_each_possible_cpu(cpu)
		per_cpu(cpuvar, cpu) = cpu;

	cpualloc = alloc_percpu(int);
	for_each_possible_cpu(cpu)
		*per_cpu_ptr(cpualloc, cpu) = cpu;

	printk("%s(): %d cpuvar on current cpu = %d\n",
			__func__, __LINE__,
			get_cpu_var(cpuvar));
	put_cpu_var(cpuvar);

	printk("%s(): %d cpualloc on current cpu = %d\n",
			__func__, __LINE__,
			*per_cpu_ptr(cpualloc, smp_processor_id()));

	for_each_possible_cpu(cpu)
		printk("%s(): %d cpuvar on cpu%d = %d\n",
				__func__, __LINE__,
				cpu, per_cpu(cpuvar, cpu));

	for_each_possible_cpu(cpu)
		printk("%s(): %d cpualloc on cpu%d = %d\n",
				__func__, __LINE__, cpu,
				*per_cpu_ptr(cpualloc, cpu));

	free_percpu(cpualloc);

	return 0;
}

static void __exit foo_exit(void)
{
	printk("%s(): %d\n", __func__, __LINE__);
}

module_init(foo_init);
module_exit(foo_exit);

MODULE_ALIAS("foo-driver");
MODULE_LICENSE("GPL");
MODULE_VERSION(VERSION);
MODULE_DESCRIPTION("Linux is not Unix");
MODULE_AUTHOR("andrew, mrju.email@gmail.com");
