#!/bin/sh
source toolchain path
make imx_v7_defconfig
make -j8
make INSTALL_MOD_PATH=$PWD/ modules_install
make uImage LOADADDR=0x10800000
make dtbs
cp arch/arm/boot/uImage $PWD
cp arch/arm/boot/dts/imx6dl-sabreauto.dtb $PWD
chmod 777 uImage
