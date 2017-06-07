#!/bin/bash
qemu-system-x86_64 -L /usr/share/qemu -bios OVMF.fd -usb -usbdevice disk::fat.img -net nic,model=e1000,vlan=0 -net tap,ifname=tap0,vlan=0,script=no

