# ApplicationUEFI
Application UEFI,
qui se connecte en TCP à la machine 192.168.0.1 sur le port 2333 et  
envoi le message "Hello, world"


### Prérequis

* qemu : emulation
* ovmf : UEFI firmware for 64-bit x86 virtual machine
* binutils-mingw-w64 gcc-mingw-w64 : compilateur

debian:  
sudo apt-get install qemu binutils-mingw-w64 gcc-mingw-w64 ovmf  

### Lancement

Préparer le réseau virtuel avec  
sudo ./init_network.sh

Ecouter sur le port 2333 dans un terminal  
nc 192.168.0.1 -v -n -l 2333

Lancer l'application UEFI avec  
sudo ./run.sh

Pour lancer l'application UEFI sous la forme d'une image usb  
qemu-system-x86_64 -L /usr/share/qemu -bios OVMF.fd -usb -usbdevice disk::fat.img  

### Préparation clef usb bootable UEFI

sudo fdisk /dev/sdb  
g (créer une nouvelle table vide de partitions GPT)  
n (ajouter une nouvelle partition)  
t (modifier le type d'une partition)  
1 (EFI System)  
w (écrire la table sur le disque et quitter)  

sudo mkfs.vfat /dev/sdb1 (formater en FAT32)  

Monter la clef et déposer l'application dans le dossier /EFI/BOOT  

mkdir /mnt/usb  
sudo mount /mnt/usb  
mkdir /mnt/usb/EFI  
mkdir /mnt/usb/EFI/BOOT  
cp BOOTX64.EFI /mnt/usb/EFI/BOOT/  
sudo umount /mnt/usb  

