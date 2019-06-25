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
#include <linux/delay.h>
#include <linux/input.h>
#include <linux/platform_device.h>

#define STR(x) _STR(x)
#define _STR(x) #x

#define VERSION_PREFIX Foo
#define MAJOR_VERSION 1
#define MINOR_VERSION 0
#define PATCH_VERSION 0

#define VERSION STR(VERSION_PREFIX-MAJOR_VERSION.MINOR_VERSION.PATCH_VERSION)

#define CLASS_NAME "foo"
#define DEVICE_NAME "foo"

#define MS_TO_NS(x) (x * 1E6L)
#define DELAY_MS 1L

enum STATUS {
	RELEASED,
	PRESSED,
	STATUS_COUNT,
};

struct foo_input {
	struct input_dev *input;
	struct class class;
	struct hrtimer timer;
	ktime_t ktime;
	unsigned int trigger;
	unsigned int status;
};


static ssize_t status_show(struct class *class,
				struct class_attribute *attr, char *buf)
{
	struct foo_input *foo = container_of(class, struct foo_input, class);
	return sprintf(buf, "%s\n", foo->status? "pressed" : "released");
}
static CLASS_ATTR_RO(status);

static ssize_t trigger_show(struct class *class,
				struct class_attribute *attr, char *buf)
{
	struct foo_input *foo = container_of(class, struct foo_input, class);
	return sprintf(buf, "%d\n", foo->trigger? 1 : 0);
}

static ssize_t trigger_store(struct class *class,
				struct class_attribute *attr,
				const char *buf, size_t count)
{
	int err, val;
	struct foo_input *foo;

	foo = container_of(class, struct foo_input, class);

	err = kstrtouint(buf, 10, &val);
	if (err)
		return err;

	if (val)
		foo->trigger = PRESSED;
	else
		foo->trigger = RELEASED;

	return count;
}
static CLASS_ATTR_RW(trigger);

static struct attribute *foo_class_attrs[] = {
	&class_attr_status.attr,
	&class_attr_trigger.attr,
	NULL,
};
ATTRIBUTE_GROUPS(foo_class);

static enum hrtimer_restart hrtimer_handler(struct hrtimer *timer)
{
	struct foo_input *foo = container_of(timer, struct foo_input, timer);

	if (foo->trigger ^ foo->status) {
		if (foo->trigger) {
			input_report_key(foo->input, KEY_ENTER, foo->trigger);
			input_sync(foo->input);
			foo->status = PRESSED;
		} else {
			input_report_key(foo->input, KEY_ENTER, foo->trigger);
			input_sync(foo->input);
			foo->status = RELEASED;
		}
	}

	hrtimer_forward_now(&foo->timer, foo->ktime);

	return HRTIMER_RESTART;
}

static int foo_probe(struct platform_device *pdev)
{
	int ret;
	struct foo_input *foo;
	struct input_dev *input;

	foo = devm_kzalloc(&pdev->dev, sizeof(*foo), GFP_KERNEL);
	if (!foo)
		return -ENOMEM;

	input = input_allocate_device();
	if (!input) {
		ret = -ENOMEM;
		goto err_alloc_input;
	}

	/* try to uncomment to see the performance */
	/* set_bit(EV_REP, input->evbit); */
	set_bit(EV_KEY, input->evbit);
	set_bit(KEY_ENTER, input->keybit);

	input->name = DEVICE_NAME;
	input->phys = "artech/input0";
	input->dev.parent = &pdev->dev;

	input->id.vendor = 0x0512;
	input->id.product = 0x0304;
	input->id.version = 0x0012;

	ret = input_register_device(input);
	if (ret)
		goto err_register_input;

	foo->class.owner = THIS_MODULE;
	foo->class.name = CLASS_NAME;
	foo->class.class_groups = foo_class_groups;
	ret = class_register(&foo->class);
	if (ret)
		goto err_register_class;

	foo->input = input;
	platform_set_drvdata(pdev, foo);

	foo->ktime = ktime_set(0, MS_TO_NS(DELAY_MS));
	hrtimer_init(&foo->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	foo->timer.function = hrtimer_handler;
	hrtimer_start(&foo->timer, foo->ktime, HRTIMER_MODE_REL);

	return 0;

err_register_class:
	input_unregister_device(input);

err_register_input:
	input_free_device(input);

err_alloc_input:
	return ret;
}

static int foo_remove(struct platform_device *pdev)
{
	struct foo_input *foo = platform_get_drvdata(pdev);

	printk("%s(): %d\n", __func__, __LINE__);

	hrtimer_cancel(&foo->timer);
	class_unregister(&foo->class);
	input_unregister_device(foo->input);
	input_free_device(foo->input);
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
