#!/bin/bash

#
# oorail-system setup script
#
#
#

echo ""
echo "oorail-system setup script"
echo ""

# set factory default hostname
echo " * Setting hostname to oorail-system"
sudo hostnamectl set-hostname oorail-system
sudo bash -c "cp /etc/hosts /etc/hosts.orig"
sudo bash -c "cat /etc/hosts.orig | sed 's/raspberrypi/oorail-system/g' > /etc/hosts"

# enable ssh
echo " * Enabling OpenSSH"
systemctl enable ssh
systemctl start ssh

# update
echo " * Updating System"
cd /tmp
sudo apt-get -y update
sudo apt-get -y dist-upgrade
sudo apt-get -y upgrade

# install docker
echo " * Installing Docker"
sudo curl -sSL https://get.docker.com | sh
sudo usermod -aG docker pi

# install dependencies and  docker-compose
echo " * Installing Docker Compose and Dependencies"
sudo apt-get install -y libffi-dev libssl-dev
sudo apt-get install -y python python-pip
sudo apt-get remove -y python-configparser
sudo pip install docker-compose

# install git
echo " * Installing git"
sudo apt-get -y install git

# install packages to support mdns
echo " * Installing mdns"
sudo apt-get -y install avahi-daemon
sudo apt-get -y install libnss-mdns

sudo bash -c 'cp /etc/nsswitch.conf /etc/nsswitch.conf.orig'
sudo bash -c "cat /etc/nsswitch.conf.orig | sed 's/ dns/ dns mdns4/g' > /etc/nsswitch.conf"

# disable swap
echo " * Disable Swap"
sudo dphys-swapfile swapoff
sudo dphys-swapfile uninstall
sudo systemctl disable dphys-swapfile.service

# uncomment the command below if you are running an older version of raspbian
# and the systemctl command fails
# sudo update-rc.d dphys-swapfile remove

# update swap config
echo " * Updating swap config"
sudo mv /etc/dphys-swapfile /etc/dphys-swapfile.original
sudo cat /etc/dphys-swapfile.original | sed 's/CONF_SWAPSIZE\=100/CONF_SWAPSIZE\=0/g' > /tmp/dphys-swapfile
sudo chown 0:0 /tmp/dphys-swapfile 
sudo mv /tmp/dphys-swapfile /etc/

# create directories for oorail system
echo " * Creating oorail directory"
sudo mkdir /opt/oorail

# cleanup
echo " * Cleanup"
sudo apt autoremove -y

# run a second pass of pip install for docker-compose
echo " * Running second pass for docker-compose install"
sudo pip install docker-compose

# logout (required for docker to take effect, run test.sh next)
logout

