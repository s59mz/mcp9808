/*  Demo application for MCP9808 sensor
 *
 *  Copyright (C) 2023 Matjaz Zibert - S59MZ
 *
 *  Linux app for device driver
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>

#include "../mcp9808_ioctl.h"

int main(int argc, char **argv) {

	int fd;
	int ret;
	struct ioctl_dev ioctl_dev;
	float temp;

	if (argc !=1) {
		printf("Use 'mcp' command only\n");
		return 0;
	}

	fd = open("/dev/mcp9808mz18", O_RDWR);

	if (fd <= 0) {
		printf("Can't open fd = %d\n", fd);
	}

	ret = ioctl(fd, IOCTL_MCP9808_READ_TEMP, &ioctl_dev);

	if (ret < 0) {
		printf("Can't ioctl, ret = %d\n", ret);
		close(fd);
		return 0;
	}

	ret = close(fd);

	if (ret < 0) {
		printf("Can't close, ret = %d\n", ret);
	}

	temp = ((float) (ioctl_dev.temp & 0x1fff))/16;

	printf("Temp: %.2f'C,\n", temp);
	
	return 0;
}
