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

#define PRINTK_TAG "FOO"
#define PRINTK(format, ...) printk(PRINTK_TAG ": " format, ##__VA_ARGS__)

#define assert(format, ...)\
			PRINTK("%s, %d, %s, " format,\
				__FILE__, __LINE__, __func__, ##__VA_ARGS__);

struct gpio_info {
	int gpio;
	bool value;
};

struct foo_device {
	const char *name;
	struct class class;
	struct gpio_info gpio_info;
};

static ssize_t status_show(struct class *class,
				struct class_attribute *attr, char *buf)
{
	struct foo_device *foo = container_of(class, struct foo_device, class);
	return sprintf(buf, "%s\n", foo->gpio_info.value? "off" : "on");
}
static CLASS_ATTR_RO(status);

static ssize_t enable_store(struct class *class,
				struct class_attribute *attr,
				const char *buf, size_t count)
{
	int err, val;
	struct foo_device *foo;

	foo = container_of(class, struct foo_device, class);

	err = kstrtouint(buf, 10, &val);
	if (err)
		return err;

	if (val) {
		gpio_set_value(foo->gpio_info.gpio, 0);
	} else {
		gpio_set_value(foo->gpio_info.gpio, 1);
	}

	foo->gpio_info.value = gpio_get_value(foo->gpio_info.gpio);
	if (!(!!val ^ foo->gpio_info.value))
		return -EAGAIN;

	return count;
}
static CLASS_ATTR_WO(enable);

static struct attribute *foo_class_attrs[] = {
	&class_attr_status.attr,
	&class_attr_enable.attr,
	NULL,
};
ATTRIBUTE_GROUPS(foo_class);

int foo_probe(struct platform_device *pdev)
{
	int ret, gpio;
	struct foo_device *foo;
	struct device_node *np = pdev->dev.of_node;

	if (!np)
		return -ENODEV;

	foo = devm_kzalloc(&pdev->dev, sizeof(*foo), GFP_KERNEL);
	if (!foo)
		return -ENOMEM;

	foo->name = dev_name(&pdev->dev);

	gpio = of_get_named_gpio(np, "foo-gpios", 0);
	if (!gpio_is_valid(gpio))
		return -ENODEV;

	foo->gpio_info.gpio = gpio;

	ret = devm_gpio_request_one(&pdev->dev, foo->gpio_info.gpio,
			GPIOF_DIR_OUT | GPIOF_INIT_LOW, foo->name);
	if (ret < 0)
		return ret;

	foo->gpio_info.value = gpio_get_value(foo->gpio_info.gpio);

	foo->class.owner = THIS_MODULE;
	foo->class.name = CLASS_NAME;
	foo->class.class_groups = foo_class_groups;
	ret = class_register(&foo->class);
	if (ret)
		return ret;

	platform_set_drvdata(pdev, foo);

	return 0;
}

int foo_remove(struct platform_device *pdev)
{
	struct foo_device *foo = platform_get_drvdata(pdev);

	class_unregister(&foo->class);
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
	foo-gpios = <&gpio 21 GPIO_ACTIVE_HIGH>;
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
