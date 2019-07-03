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
#include <linux/property.h>
#include <linux/of.h>
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

/* ****************************************************************************

/ {
	foo {
		compatible = "artech,foo";
	};

	node@0 {
		a-string-property = "A String";
		a-string-list-property = "first string", "second stting";
		a-byte-data-property = [01 23 34 56];

		child-node@0 {
			first-child-property;
			second-child-property = <1>;
			a-reference-to-something = <&node1>;
		};

		child-node@1 {
		};
	};

	node1: node@1 {
		an-empty-property;
		a-cell-property = <1 2 3 4>;

		child-node@0 {
		};
	};
};

***************************************************************************** */

int foo_probe(struct platform_device *pdev)
{
	int ret, value, i;
	int cell[4];
	char byte[4];
	const void *reference;
	const char *string;
	struct device_node *np = NULL;
	struct device_node *child = NULL;
	struct property *prop = NULL;

	np = pdev->dev.of_node;
	if (!np) {
		printk("there's no device tree node\n");
		return 0;
	}

	string = of_get_property(np, "compatible", NULL);
	printk("the property compatible is %s\n", string);

	np = of_find_node_by_path("/node@0");
	if (np) {
		printk("node name is %s\n", np->name);
		printk("node full name is %s\n", np->full_name);
	} else {
		printk("/node@0 not found\n");
	}

	prop = of_find_property(np, "a-string-property", NULL);
	if (prop) {
		printk("a-string-property name is %s\n", prop->name);
		printk("a-string-property value is %s\n", (char *)prop->value);
	} else {
		printk("a-string-property not found");
	}

	for (i = 0; i < 2; i++) {
		ret = of_property_read_string_index(np,
				"a-string-list-property", i, &string);
		if (ret) {
			printk("read %s index %d failed\n",
				"a-string-list-property", i);
		} else {
			printk("%s index %d is %s\n",
				"a-string-list-property", i, string);
		}
	}

#if 0 /* alternatively  */
	const char *byte;
	byte = of_get_property(np, "a-byte-data-property", NULL);
	printk("a-byte-data-property is [%02x %02x %02x %02x]\n",
			*(byte + 0), *(byte + 1), *(byte + 2), *(byte + 3));
#endif
	ret = of_property_read_u8_array(np, "a-byte-data-property", byte, 4);
	if (ret)
		printk("%s read error with %d\n", "a-byte-data-property", ret);
	else
		printk("a-byte-data-property is [%02x %02x %02x %02x]\n",
			byte[0], byte[1], byte[2], byte[3]);

	for_each_child_of_node(np, child) {
		printk("node full name = %s\n", child->full_name);

		if (of_property_read_bool(child, "first-child-property"))
			printk("%s exists in %s\n",
				"first-child-property", child->full_name);
		else
			printk("%s doesn't exist in %s\n",
				"first-child-property", child->full_name);

		ret = of_property_read_u32(child,
				"second-child-property", &value);
		if (ret)
			printk("%s read error with %d\n",
				"second-child-property", ret);
		else
			printk("%s is %d\n", "second-child-property", value);

		reference = of_get_property(child,
				"a-reference-to-something", NULL);
		if (reference)
			printk("a-reference-to-something is %p", reference);
		else
			printk("failed to read a-reference-to-something");
	}

	np = of_find_node_by_path("/node@1");
	if (np)
		printk("node full name is %s\n", np->full_name);
	else
		printk("/node@1 not found\n");

	if (of_property_read_bool(np, "an-empty-property"))
		printk("an-empty-property exists\n");
	else
		printk("an-empty-property doesn't exist\n");

	ret = of_property_read_u32_array(np,
			"a-cell-property", cell, ARRAY_SIZE(cell));
	if (ret)
		printk("%s read error with %d\n", "a-cell-property", ret);
	else
		printk("%s is <%d %d %d %d >\n", "a-cell-property",
			cell[0], cell[1], cell[2], cell[3]);

	np = of_find_node_by_path("/node@1/child-node@0");
	if (np)
		printk("node full name is %s\n", np->full_name);
	else
		printk("/node@1 not found\n");

	return 0;
}

int foo_remove(struct platform_device *pdev)
{
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
