# Track Module

This track module enables the model railway to be controlled via
a HTTP based API and an L298N module.

Supported Hardware

 * ESP32
 * L298N module

# API Reference

*API Version*: 1.0


| API URL | Function |
|---------|----------|
|/|Basic Information|
|/debug|Debug Information|
|/ping|Health Check|
|/status|Module Status|
|/api/1/tm/debug|Track Module Debug|
|/api/1/tm/hc|Track Module Health Check|
|/api/1/tm/info|Track Module Info|
|/api/1/tm/status|Track Module Status|
|/api/1/tm/profile|Track Module Profile List|
|/api/1/tm/profile/hornby|Apply Hornby Profile|
|/api/1/tm/profile/heljan|Apply Heljan Profile|
|/api/1/tm/profile/dapol|Apply Dapol Profile|
|/api/1/tm/profile/bachmann|Apply Bachmann Profile|
|/api/1/tm/profile/custom|Apply Custom Profile|
|/api/1/tm/0/speed/current|Track A Current Speed|
|/api/1/tm/1/speed/current|Track B Current Speed|
|/api/1/tm/0/speed/up|Track A Increase Speed|
|/api/1/tm/1/speed/up|Track B Increase Speed|
|/api/1/tm/0/speed/down|Track A Decrease Speed|
|/api/1/tm/1/speed/down|Track B Decrease Speed|
|/api/1/tm/0/speed/stop|Track A Slow to Stop|
|/api/1/tm/1/speed/stop|Track B Slow to Stop|
|/api/1/tm/0/stop|Track A Immediate Stop|
|/api/1/tm/1/stop|Track B Immediate Stop|
|/api/1/tm/stop|Immediate Stop - All Tracks|
|/api/1/tm/0/coast|Track A Set Speed to Normal Running Speed|
|/api/1/tm/1/coast|Track B Set Speed to Normal Running Speed|
|/api/1/tm/0/shunt|Track A Set Speed to Shunt|
|/api/1/tm/1/shunt|Track B Set Speed to Shunt|
|/api/1/tm/0/crawl|Track A Set Speed to Crawl|
|/api/1/tm/1/crawl|Track B Set Speed to Crawl|
|/api/1/tm/0/full|Track A Set Speed to Full|
|/api/1/tm/1/full|Track B Set Speed to Full|
|/api/1/tm/0/brake|Track A Push Brake|
|/api/1/tm/1/brake|Track B Push Brake|
|/api/1/tm/0/brake/on|Track A Toggle Brake On|
|/api/1/tm/1/brake/on|Track B Toggle Brake On|
|/api/1/tm/0/brake/off|Track A Taggle Brake Off|
|/api/1/tm/1/brake/off|Track B Toggle Brake Off|
|/api/1/tm/0/travel/up|Set Track A Direction of Travel to Up Line|
|/api/1/tm/1/travel/up|Set Track B Direction of Travel to Up Line|
|/api/1/tm/0/travel/down|Set Track A Direction of Travel to Down Line|
|/api/1/tm/1/travel/down|Set Track B Direction of Travel to Down Line|

## Potential Problems

### Bachmann Branchline Lighting Issue

During testing we noticed that some Bachmann Branchline (UK) would work fine but that all tail
lamps would be illuminated regardless of the direction of travel.

This problem does not occur with other manufacturers. As we do not know the exact root cause yet,
we recommend avoiding powering Bachmann Branchline locomotives with this code if you 
encounter this problem. 

### Brownout detector was triggered

Some ESP32/ESP32S boards upon loading the code with WiFi enabled, may continously 
software CPU reset (reboot) due to brownout detection. This happens when the power supply
feeding your ESP32 board (most likely USB) cannot properly power the board.

Fixes:

+ Try a powered USB hub
+ Try a different USB port on your computer
+ Try using a USB to AC adapter that has a higher current (Amp) rating
+ Use a shorter USB cable
+ Try powering the ESP32 module via the Vin / GND pins instead

Example output:



```
15:47:02.582 -> oorail-track-module, version 1.0.0

15:47:02.582 -> Copyright (c) 2020 IdeaPier LLC (oorail.co.uk)

15:47:02.582 -> 

15:47:02.582 -> For additional information visit:

15:47:02.582 ->  https://oorail.co.uk/tech/ 

15:47:02.582 -> 

15:47:02.582 -> License: GPLv3

15:47:02.582 -> 

15:47:02.582 ->  * Initializing Profile 

15:47:02.649 -> 

15:47:02.649 -> Brownout detector was triggered

15:47:02.649 -> 

15:47:02.649 -> ets Jun  8 2016 00:22:57

15:47:02.649 -> 

15:47:02.649 -> rst:0xc (SW_CPU_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)

15:47:02.649 -> configsip: 0, SPIWP:0xee

15:47:02.682 -> clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00

15:47:02.682 -> mode:DIO, clock div:1

15:47:02.682 -> load:0x3fff0018,len:4

15:47:02.682 -> load:0x3fff001c,len:1044

15:47:02.682 -> load:0x40078000,len:8896

15:47:02.682 -> load:0x40080400,len:5816

15:47:02.682 -> entry 0x400806ac

15:47:02.981 -> 
```
