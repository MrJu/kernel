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
#include <linux/of.h>
#include <linux/input.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/timer.h>
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

#define INTERVAL_IN_MSEC 20

enum STATUS {
	RELEASED,
	PRESSED,
	STATUS_COUNT,
};

struct gpio_info {
	int gpio;
	unsigned int status;
};

struct foo_device {
	const char *name;
	struct gpio_info gpio_info;
	struct input_dev *input;
	struct timer_list timer;
};

static void timer_callback(struct timer_list *t)
{
	unsigned int value;
	struct foo_device *foo = from_timer(foo, t, timer);

	value = gpio_get_value(foo->gpio_info.gpio);
	if (value) {
		if (foo->gpio_info.status == RELEASED) {
			input_report_key(foo->input, KEY_ENTER, 0);
			input_sync(foo->input);
			foo->gpio_info.status = PRESSED;
		}
	} else {
		if (foo->gpio_info.status == PRESSED) {
			input_report_key(foo->input, KEY_ENTER, 1);
			input_sync(foo->input);
			foo->gpio_info.status = RELEASED;
		}
	}

	mod_timer(&foo->timer, jiffies + msecs_to_jiffies(INTERVAL_IN_MSEC));
}

int foo_probe(struct platform_device *pdev)
{
	int ret, gpio;
	struct foo_device *foo;
	struct input_dev *input;
	struct device_node *np = pdev->dev.of_node;

	if (!np)
		return -ENODEV;

	foo = devm_kmalloc(&pdev->dev, sizeof(*foo), GFP_KERNEL);
	if (!foo)
		return -ENOMEM;

	foo->name = dev_name(&pdev->dev);

	gpio = of_get_named_gpio(np, "interrupt-gpios", 0);
	if (!gpio_is_valid(gpio))
		return -ENODEV;

	foo->gpio_info.gpio = gpio;

	ret = devm_gpio_request_one(&pdev->dev, foo->gpio_info.gpio,
			GPIOF_DIR_IN | GPIOF_INIT_LOW, foo->name);
	if (ret < 0)
		return ret;

	foo->gpio_info.status = RELEASED;

	input = devm_input_allocate_device(&pdev->dev);
	if (!input)
		return -ENOMEM;

	/*
	 * try to uncomment to see the behavior
	 * set_bit(EV_REP, input->evbit);
	 */

	set_bit(EV_KEY, input->evbit);
	set_bit(KEY_ENTER, input->keybit);

	input->name = foo->name;
	input->phys = "artech/input0";
	input->dev.parent = &pdev->dev;

	input->id.vendor = 0x0512;
	input->id.product = 0x0304;
	input->id.version = 0x0012;

	ret = input_register_device(input);
	if (ret)
		return ret;

	foo->input = input;


	timer_setup(&foo->timer, timer_callback, 0);
	add_timer(&foo->timer);

	platform_set_drvdata(pdev, foo);

	return 0;
}

int foo_remove(struct platform_device *pdev)
{
	struct foo_device *foo = platform_get_drvdata(pdev);

	del_timer_sync(&foo->timer);
	input_unregister_device(foo->input);
	devm_gpio_free(&pdev->dev, foo->gpio_info.gpio);
	devm_kfree(&pdev->dev, foo);

	return 0;
}

static const struct platform_device_id foo_platform_id[] = {
	{
		.name = DEVICE_NAME,
		.driver_data = 0,
	},
	{
		/* end */
	},

};
MODULE_DEVICE_TABLE(platform, foo_platform_id);

/*
foo {
	compatible = "artech,foo";
	label = "artech,foo";
	interrupt-gpios = <&gpio 21 GPIO_ACTIVE_HIGH>;
};
*/

static const struct of_device_id foo_of_match[] = {
	{
		.compatible = "artech,foo",
	},
	{
		/* end */
	},
};
MODULE_DEVICE_TABLE(of, foo_of_match);

static struct platform_driver foo_drv = {
	.probe	= foo_probe,
	.remove	= foo_remove,
	.id_table = foo_platform_id,
	.driver	= {
		.owner = THIS_MODULE,
		.name = DEVICE_NAME,
		.of_match_table = of_match_ptr(foo_of_match),
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
