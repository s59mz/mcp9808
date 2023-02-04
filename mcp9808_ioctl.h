/*  The simplest kernel module.
 *
 *  Copyright (C) 2023 Matjaz Zibert - S59MZ
 *
 *  Linux I2C device driver for MCP9808 temperature sensor
 */

#ifndef __MCP9808_IOCTL_H_MZ__
#define __MCP9808_IOCTL_H_MZ__

#include <linux/ioctl.h>

#define IOCTL_MCP9808_READ_TEMP		_IOR('M', 1, struct ioctl_dev)

// convert tempereture register value to Celsius degrees
#define convert_temp(x)	(((x) & 0x1000) ? \
	(256 - ((float) ((x) & 0x0fff))/16) : \
	(((float) ((x) & 0x1fff))/16));

struct ioctl_dev {
	unsigned int temp;	// temperature reg data - need to be converted
};

#endif
