#!/bin/bash

## this script assumes you setup the SDD using the makedisk.sh script
## and that you have run the init-data.sh script

### TODO add a safety check for /oorail/images/docker exists

sudo bash -c 'mv /var/lib/docker /oorail/images/docker'
sudo bash -c 'ln -s /oorail/images/docker/docker /var/lib/docker'
sudo bash -c 'chmod 0711 /var/lib/docker'

