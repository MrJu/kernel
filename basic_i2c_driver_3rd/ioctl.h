#ifndef __IOCTL_H
#define __IOCTL_H

struct trans_msg {
	int type;
	int trans;
};

enum IOC_NR {
	IOC_NR_0,
	IOC_NR_1,
	IOC_NR_2,
	IOC_NR_3,
	IOC_NR_CNT,
};

#define IOC_MAGIC  'c'
#define IOC_CMD_NONE	_IO(IOC_MAGIC, IOC_NR_0) /* _IO(type, nr) */
#define IOC_CMD_GET		_IOR(IOC_MAGIC, IOC_NR_1, struct trans_msg) /* _IOR(type, nr, size) */
#define IOC_CMD_SET		_IOW(IOC_MAGIC, IOC_NR_2, struct trans_msg) /* _IOW(type, nr, size) */
#define IOC_CMD_TRANS	_IOWR(IOC_MAGIC, IOC_NR_3, struct trans_msg) /* _IOWR(type, nr, size) */

#endif
