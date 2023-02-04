# mcp9808
Linux device driver for MCP9808 temperature sensor

### Compiling the module for Linux (arm64)
export ARCH=arm64

export CROSS_COMPILE=aarch64-linux-gnu-

export LINUX_DIR="*path-to-linux-precompiled-headers*"

make

### Compile the device tree overlay
dtc -@ -I dts -O dtb -o mcp9808_overlay.dtbo mcp9808_overlay.dts

### Compile the demo app
cd app

make

### Install the module on target (Raspberry Pi 4):
sudo dtoverlay mcp9808_overlay.dtbo

dtoverlay -l

sudo insmod mcp9808.ko

dmesg | tail

### Run the demo app on target
sudo ./mcp

### NOTE
The floating point temperature calculation is done in the demo app (in Linux User Space).

### Remove the Module and Overlay
sudo rmmod mcp9808

sudo dtoverlay -d mcp9808_overlay
