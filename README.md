# linux_device_driver
Linux Device Driver

on debian 8

Environment:
apt-get install linux-source 
cp /boot/config-3.16.0-4-amd64 ~/kernel/linux-source-3.16/.config
make oldconfig && make prepare
make scripts
#make && make modules
or cp /usr/src/linux-headers-3.16.0-4-amd64/Module.symvers .



Run:
cd hello

make
insmod hello.ko
tail -f /var/log/messages

useful commands

make clean
make distclean
modinfo
modprobe 

