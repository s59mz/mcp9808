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

struct ioctl_dev {
	unsigned int temp;	// temperature in Celsius
};

#endif
