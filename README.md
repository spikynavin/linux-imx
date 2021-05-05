++++++++++++++++++++++ Linux NXP Kernel 3.14.28 +++++++++++++++++++++++

Compilation steps:

source ~/iwave-cc (Toolchain env setup)
make (board configuration)
make menuconfig
make -j4 (Use processor available core number)
make INSTALL_MOD_PATH=$PWD/ modules_install
make uImage LOADADDR=0x10800000
make dtbs

Then Flashing the compiled kernel image with MFGTool.
