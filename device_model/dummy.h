#ifndef __DUMMY
#define __DUMMY

#include <linux/device.h>

struct dummy_device {
	struct device device;
};

struct dummy_driver {
	struct device_driver driver;
	int (*probe)(struct dummy_device *);
};

extern struct bus_type dummy_bus_type;
extern int __dummy_driver_register(struct dummy_driver *, struct module *);
extern void dummy_driver_unregister(struct dummy_driver *);
extern int dummy_device_register(struct dummy_device *);
extern void dummy_device_unregister(struct dummy_device *);

static inline void *dummy_get_drvdata(const struct dummy_device *dummy_dev)
{
	return dev_get_drvdata(&dummy_dev->device);
}

static inline void dummy_set_drvdata(struct dummy_device *dummy_dev,
					void *data)
{
	dev_set_drvdata(&dummy_dev->device, data);
}


#define dummy_driver_register(drv) \
	__dummy_driver_register(drv, THIS_MODULE)

#define to_dummy_driver(drv) (container_of((drv), \
			struct dummy_driver, driver))

#define to_dummy_device(dev) container_of((dev), struct dummy_device, device);

#endif
