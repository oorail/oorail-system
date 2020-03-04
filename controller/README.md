# oorail-system Controller

### Summary

The oorail-system controller is the *brain* of your layout. The controller is a central location which controls, manages and monitors
all of the components on your layout. The controller can be used to provision new devices or locomotives, reprogram devices or locomotives
and provides innovative features for your layout.

The controller is Linux based using Docker containers for easy management and updates. The controller continues the design concept of the
oorail-system by offering and integrating with the API.

### Recommended Hardware

The recommended hardware for the controller is a Raspberry Pi 3 (Model B+) or Raspberry Pi 4.
A high-capacity flash card (32GB or higher) is recommended or adding a SSD to the Raspberry Pi.

Note: The controller is designed to work on any Linux system with Docker, so you could easily
repurpose any x86 or x86-64 platform, or use the ARM based Raspberry Pi.

### Operating System

We recommend using Raspbian Lite (latest) that is supported for your Raspberry Pi. If you are using another
hardware platform, we would recommend the latest Debian, Ubuntu Server LTS or Linux Mint.

### Headless Operation

The initial release supports headless operation only, it is intended to be accessed via a tablet, mobile device or
computer using a recent web browser such as Chrome or Firefox. A version supporting touch screen display is under
development.

### Data Storage

Data is stored in /opt/oorail/ there are two directories:

 * images/
 * data/

The images directory contains images to be deployed on ESP32, ESP8266, Arduino or other microcontroller devices.

The data directory contains the configuration, usage and other data related to your layout.

### Docker Basic Commands

'''
docker-compose -f controller-compose.yaml up
'''



