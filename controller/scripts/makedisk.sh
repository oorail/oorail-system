#!/bin/bash

echo ""
echo "oorail-makedisk script"
echo "Copyright (c) 2020 IdeaPier LLC"
echo ""
echo "This script creates, partitions and performs setup on an optional SDD drive"
echo ""

if [ -z "$1" ]; then
 echo "oorail-makedisk.sh script: missing disk device parameter"
 echo ""
 echo "Usage: ./oorail-makedisk.sh <device>"
 echo ""
 echo "Example: ./oorail-makedisk.sh sdf"
 echo ""
 echo "You forgot to pass in a disk device parameter"
 echo ""
 exit
fi

if [ "$1" = "mmcblk0" ]; then
 echo "ERROR: The device mmcblk0 is the flash device, you probably don't want to wipe it"
 echo ""
 exit
fi

echo " * Creating GPT partition table on /dev/{$1}"
sudo parted /dev/${1} mklabel gpt

echo " * Creating Image Partition"
echo " * Default size is 50GB"
sudo parted /dev/${1} mkpart primary ext4 0G 50G

echo " * Creating Data Partition"
echo " * Default size is rest of disk"
sudo parted /dev/${1} mkpart primary ext4 50G 100%

echo " * Formatting image partition"
echo ""
sudo mkfs.ext4 -m 0 -U random -L oorail-images -E discard,stride=4096,stripe-width=4096 /dev/${1}1

echo " * Formatting data partition"
echo ""
sudo mkfs.ext4 -m 0 -U random -L oorail-data -E discard,stride=4096,stripe-width=4096 /dev/${1}2

echo ""
sudo mkdir -p /opt/oorail/{images,data}
echo "LABEL=oorail-images /opt/oorail/images ext4 discard,noatime,commit=600,errors=remount-ro 0       1" >> /tmp/fstab-add
echo "LABEL=oorail-data /opt/oorail/data ext4 discard,noatime,commit=600,errors=remount-ro 0       1" >> /tmp/fstab-add

sudo bash -c 'cat /tmp/fstab-add >> /etc/fstab'
sudo mount -a

echo ""
echo "Cleanup"

sudo rm -rf /tmp/fstab-add

