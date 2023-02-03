/*  The simplest kernel module.
 *
 *  Copyright (C) 2023 Matjaz Zibert - S59MZ
 *
 *  Linux I2C device driver for MCP9808 Temperature sensor
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/delay.h>

#include <linux/cdev.h>
#include <linux/i2c.h>

#include "mcp9808_ioctl.h"

#define MAX_DEV 8

#define DRV_NAME   "mcp9808"
#define CLASS_NAM  "mcp"

#define mz_info(x, ...)	//pr_info(x, ##__VA_ARGS__)
#define mz_bug()		pr_info("ERROR: %s line %d\n", __FILE__, __LINE__);

struct my_dev {
	struct ioctl_dev ioctl_dev;
	struct i2c_client * client;
	struct cdev cdev;
	dev_t devt;
};

static int NumofDev = 0;
static dev_t DevNum;

static struct class * Class;

static int dev_open(struct inode * inode, struct file * fd) {

	struct my_dev * mydev;
	struct i2c_client * client;

	mydev = container_of(inode->i_cdev, struct my_dev, cdev);
	client = mydev->client;

	fd->private_data = mydev;

	mz_info("Device: %s opened, addr=0x%x\n", client->name, client->addr);

	return 0;
}

static int dev_close(struct inode * inode, struct file * fd) {

	struct my_dev * mydev;
	struct i2c_client * client;

	mydev = container_of(inode->i_cdev, struct my_dev, cdev);
	client = mydev->client;

	fd->private_data = NULL;

	mz_info("Device: %s closed, addr=0x%x\n", client->name, client->addr);

	return 0;
}

static long dev_ioctl(struct file *fd, unsigned int cmd, unsigned long arg)
{
	int ret;
	struct my_dev * mydev;
	struct i2c_client * client;
	struct ioctl_dev * ioctl;

	mydev = fd->private_data;
	client = mydev->client;
	ioctl = &mydev->ioctl_dev;

	mz_info("Device %s ioctl, addr=0x%x, cmd=%d\n", 
				client->name, client->addr, cmd);

	switch(cmd) {

		case IOCTL_MCP9808_READ_TEMP:

			ret = i2c_smbus_read_word_swapped(client, 0x05);

			if (ret < 0) {
				mz_bug();
				return -EIO;
			}

			mz_info("Ioctl Read: %2x\n", ret);

			// the read value must also be devided by 16 as float in next step
			ioctl->temp = (unsigned short) (ret & 0xffff);

			if (!arg) {
				mz_bug();
				return -EINVAL;
			}

			ret = copy_to_user((void *) arg, ioctl, sizeof(struct ioctl_dev));

			if (ret < 0) {
				mz_bug();
				return ret;
			}

			break;

		default:
			mz_bug();
			return -EINVAL;
	}

	return 0;
}

static struct file_operations Fops = {
	.owner = THIS_MODULE,

	.open = dev_open,
	.release = dev_close,
	.unlocked_ioctl = dev_ioctl,
};

static int i2c_device_probe(struct i2c_client *client) {

	struct device * device;
	struct my_dev * my_dev;

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_BYTE_DATA))  {
		mz_bug();
		return -EIO;
	}

	my_dev = kzalloc(sizeof(struct my_dev), GFP_KERNEL);

	if (!my_dev) {
		mz_bug();
		return -ENOMEM;
	}

	cdev_init(&my_dev->cdev, &Fops);

	my_dev->cdev.owner = THIS_MODULE;
	my_dev->devt = MKDEV(MAJOR(DevNum), MINOR(DevNum) + NumofDev);
	my_dev->client = client;

	cdev_add(&my_dev->cdev, my_dev->devt, 1);

	device = device_create(Class, NULL, my_dev->devt, NULL, 
					"%s%x", client->name, client->addr);

	if (IS_ERR(device)) {
		mz_bug();
		return IS_ERR(device);
	}

	i2c_set_clientdata(client, my_dev);

	NumofDev ++;

	pr_info("Device %s%x probed: addr=0x%x, flags=%d, irq=%d, devt=%d\n", 
			client->name, client->addr, client->addr, 
			client->flags, client->irq, my_dev->devt);

	return 0;
}

static int i2c_device_remove(struct i2c_client *client) {
	struct my_dev * my_dev;

	my_dev = i2c_get_clientdata(client);

	device_destroy(Class, my_dev->devt);
	cdev_del(&my_dev->cdev);

	kfree(my_dev);

	NumofDev --;

	pr_info("Device %s%x removed\n", client->name, client->addr);

	return 0;
}

static const struct of_device_id Of_device_id[] = {
	{ .compatible = "mz,mcp9808mz" },
	{},
};

MODULE_DEVICE_TABLE(of, Of_device_id);

static struct i2c_driver I2c_driver = {
	.driver = {
		.owner = THIS_MODULE,
		.name = DRV_NAME,
		.of_match_table = of_match_ptr(Of_device_id),
	},

	.probe_new = i2c_device_probe,
	.remove = i2c_device_remove,
};

static int __init i2c_init(void) {
	int ret;

	DevNum = 0;
	NumofDev = 0;

	pr_info("Driver %s init\n", I2c_driver.driver.name);

	ret = alloc_chrdev_region(&DevNum, 0, MAX_DEV, DRV_NAME);

	if (ret < 0) {
		mz_bug();
		return ret;
	}

	Class = class_create(THIS_MODULE, CLASS_NAM);

	if (IS_ERR(Class)) {
		mz_bug();
        return PTR_ERR(Class);
	}

	ret = i2c_register_driver(THIS_MODULE, &I2c_driver);

	if (ret < 0) {
		mz_bug();
		return ret;
	}

	return 0;
}

static void __exit i2c_exit(void) {

	i2c_del_driver(&I2c_driver);
	class_destroy(Class);
	unregister_chrdev_region(DevNum, MAX_DEV);

	NumofDev = 0;
	DevNum = 0;

	pr_info("Driver %s exit\n", I2c_driver.driver.name);
}

module_init(i2c_init);
module_exit(i2c_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Matjaz Zibert");
MODULE_DESCRIPTION("Linux device driver for MCP9808 temperature sensor");

