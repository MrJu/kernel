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
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/interrupt.h>
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

#define PRINTK_TAG "FOO"
#define PRINTK(format, ...) printk(PRINTK_TAG ": " format, ##__VA_ARGS__)

#define assert(format, ...)\
			PRINTK("%s, %d, %s, " format,\
				__FILE__, __LINE__, __func__, ##__VA_ARGS__);

struct irq_info {
	int gpio;
	int irq;
};

struct foo_device {
	const char *name;
	struct irq_info irq_info;
	struct timer_list timer;
};

static void timer_callback(struct timer_list *t)
{
	int value;

	struct foo_device *foo = from_timer(foo, t, timer);

	value = gpio_get_value(foo->irq_info.gpio);
	if (value) {
		assert("button down");
	} else {
		assert("button up");
	}
}

static irqreturn_t irq_handler(int irq, void *dev)
{
	struct foo_device *foo = dev;

	mod_timer(&foo->timer, jiffies + msecs_to_jiffies(INTERVAL_IN_MSEC));

	return IRQ_HANDLED;
}

int foo_probe(struct platform_device *pdev)
{
	int ret, gpio, irq;
	struct foo_device *foo;
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

	foo->irq_info.gpio = gpio;

	ret = devm_gpio_request_one(&pdev->dev, foo->irq_info.gpio,
			GPIOF_DIR_IN | GPIOF_INIT_LOW, foo->name);
	if (ret < 0)
		return ret;


	irq  = gpio_to_irq(foo->irq_info.gpio);
	if (irq < 0)
		return irq;

	foo->irq_info.irq = irq;

	ret = devm_request_any_context_irq(&pdev->dev,
			foo->irq_info.irq,
			irq_handler,
			IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
			foo->name, (void *)foo);
	if (ret < 0)
		return ret;

	timer_setup(&foo->timer, timer_callback, 0);
	add_timer(&foo->timer);

	platform_set_drvdata(pdev, foo);

	return 0;
}

int foo_remove(struct platform_device *pdev)
{
	struct foo_device *foo = platform_get_drvdata(pdev);

	devm_free_irq(&pdev->dev, foo->irq_info.irq, (void *)foo);
	devm_gpio_free(&pdev->dev, foo->irq_info.gpio);
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
