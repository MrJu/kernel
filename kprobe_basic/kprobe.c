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
#include <linux/platform_device.h>
#include <linux/kprobes.h>

#define STR(x) _STR(x)
#define _STR(x) #x

#define VERSION_PREFIX kprobe
#define MAJOR_VERSION 1
#define MINOR_VERSION 0
#define PATCH_VERSION 0
#define VERSION STR(VERSION_PREFIX-MAJOR_VERSION.MINOR_VERSION.PATCH_VERSION)
#define DEVICE_NAME "dummy"

#define MAX_SYMBOL_LEN 64

static char symbol[MAX_SYMBOL_LEN] = "_do_fork";
module_param_string(symbol, symbol, sizeof(symbol), 0644);

static struct kprobe kp = {
	.symbol_name	= symbol,
};

static int handler_pre(struct kprobe *p, struct pt_regs *regs)
{
	dump_stack();

	return 0;
}

static void handler_post(struct kprobe *p, struct pt_regs *regs,
				unsigned long flags)
{
	dump_stack();
}

static int handler_fault(struct kprobe *p, struct pt_regs *regs, int trapnr)
{
	pr_err("%s: p->addr = 0x%px, trap #%d\n",
		__func__, p->addr, trapnr);

	return 0;
}

static int kprobe_probe(struct platform_device *pdev)
{
	int ret;

	kp.pre_handler = handler_pre;
	kp.post_handler = handler_post;
	kp.fault_handler = handler_fault;

	ret = register_kprobe(&kp);
	if (ret < 0) {
		pr_err("%s: register_kprobe failed, returned %d\n", __func__, ret);
		return ret;
	}

	pr_info("%s: kprobe at 0x%px registered\n", __func__, kp.addr);

	return 0;
}

static int kprobe_remove(struct platform_device *pdev)
{
	unregister_kprobe(&kp);
	pr_info("%s: kprobe at 0x%px unregistered\n", __func__, kp.addr);

	return 0;
}

static struct platform_driver kprobe_drv = {
	.probe	= kprobe_probe,
	.remove	= kprobe_remove,
	.driver	= {
		.name = DEVICE_NAME,
	}
};

static int __init kprobe_init(void)
{
	int ret;
	struct platform_device *pdev;

	pdev = platform_device_register_simple(DEVICE_NAME, -1, NULL, 0);
	if (IS_ERR(pdev))
		return PTR_ERR(pdev);

	ret = platform_driver_register(&kprobe_drv);
	if (ret) {
		platform_device_unregister(pdev);
		return ret;
	}

	return 0;
}

static void __exit kprobe_exit(void)
{
	struct device *dev;

	dev = bus_find_device_by_name(kprobe_drv.driver.bus, NULL, DEVICE_NAME);
	if (dev)
		platform_device_unregister(to_platform_device(dev));

	platform_driver_unregister(&kprobe_drv);
}

module_init(kprobe_init);
module_exit(kprobe_exit);

MODULE_ALIAS("kprobe-driver");
MODULE_LICENSE("GPL");
MODULE_VERSION(VERSION);
MODULE_DESCRIPTION("Linux is not Unix");
MODULE_AUTHOR("andrew, mrju.email@gmail.com");
