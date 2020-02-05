# oorail-system
Digital DC Train Controller System

[Official Digital DC Site](https://oorail.co.uk/digital-dc/)

Welcome to the Digital DC (DDC) Project. DDC is a new model railway control 
system built using off-the-shelf DC powered electronic components and modules. 
The software that enables the DDC system is available to download for FREE 
under the GPLv3 Open Source License. Unlike proprietary systems, DDC enables 
you the railway modeler to play with the code and contribute code changes back 
to the project. The goal of the DDC project is to not only provide a new state 
of the art model railway control system but to enable the system to be used as 
an educational tool for electronic and software engineering, whether you are 13
or 93!

You can find more information and tutorial videos on our website at
[https://oorail.co.uk/digital-dc](https://oorail.co.uk/digital-dc)

### Supported Locomotives
DC and DCC ready locomotives should run fine with this system. We do not
recommend using DCC chipped locomotives with this system, even if they support
DC operation. There is a risk that the L298N module could damage your DCC chips.
Supporting DCC is in our roadmap, so please check back in the future for support.

### Release Schedule
A new release is made available to everyone on the first Tuesday of the month.

To help fund this project, a private beta release build is issued to subscribers 
on the second, third and forth Tuesdays of the month. If you are interested in
supporting this project with a paid subscription please [click here](https://oorail.co.uk/digital-dc).

#### 2020
|Release Date|Release Version|Release Notes|Status|
|------------|---------------|-------------|------|
|Feb 4th 2020|1.0.0|[Draft](https://github.com/oorail/oorail-system/blob/master/docs/release-notes/2020/Release-Notes-1.0.0.md)|Released|
|Mar 3rd 2020|1.1.0|-|-|
|Apr 7th 2020|1.2.0|-|-|
|May 5th 2020|1.3.0|-|-|
|Jun 2nd 2020|1.4.0|-|-|
|Jul 7th 2020|1.5.0|-|-|
|Aug 4th 2020|1.6.0|-|-|
|Sep 1st 2020|1.7.0|-|-|
|Oct 6th 2020|1.8.0|-|-|
|Nov 3rd 2020|1.9.0|-|-|
|Dec 1st 2020|1.10.0|-|-|
|Jan 5th 2021|2.0.0|-|-|

### Components
You can find the necessary components at our Amazon Shop, if you are looking
for 3D printable components you can check out our [Trackside3D Shop](https://trackside3d.co.uk).

[Shop at oorail to Support this Project](https://oorail.co.uk/shopping-oorail/)

### Available Modules

|Module Name|Module Function|Module Version|
|-----------|---------------|--------------|
|oorail-ddc-tm|Digital Control of DC Track|1.0.0|

### Quick Start

All of the Hardware components below can be found via our [Amazon Shop](https://oorail.co.uk/shopping-oorail/)

Hardware Components:

+ 12V DC Power Supply
+ L298n motor driver module
+ ESP32 Module
+ USB cable (depends on USB connection on your computer and ESP32 module)
+ Jumper Wires (male/female or female/female + male/male)
+ Breadboard (optional but recommended)

Software Components:

+ [Arduino IDE](https://www.arduino.cc/en/main/software)
+ [oorail-system](https://github.com/oorail/oorail-system/releases)

### Step 1: Install Arduino IDE and Configure

Follow the instructions on the Arduino IDE website (linked above). Once installed follow these steps:

+ Unzip the oorail-system zip file for the release you are using
+ File -> Preferences -> Settings Tab
+ Add Additional Boards Manager URLs 

Add the following:

```
https://dl.espressif.com/dl/package_esp32_index.json,http://arduino.esp8266.com/stable/package_esp8266com_index.json
```

+ Select OK
+ Tools -> Board -> Boards Manager
+ Type ESP32 in the search box
+ Install ESP32 by Espressif Systems (note: we used version 1.0.4)
+ Select Close
+ Tools -> Board -> NodeMCU-32S (note: if your board is specifically listed, select it)
+ Plug your board into the USB port
+ Tools -> Port -> Select your USB serial port
+ Check the other settings in Tools (speed, frequency etc)

### Step 2: Load the track module

+ File -> Open -> Select modules/oorail-ddc-tm/oorail-ddc-tm.ino
+ Select the verify icon (check mark)
+ Check for errors (errors are setup related)
+ Check versions and google search any errors
+ Once it verifies clean move to step 3

### Step 3: Assemble hardware

+ Unplug ESP32 from USB (power down)
+ Plug the ESP32 module into the breadboard

*On the L298N board attach*:

+ Power Supply 12V V+ -> Vin (L298N)
+ Power Supply 12V V- -> GND (L298N)

*Track A*:

+ L298N OUT1 -> Track (+) (polarity depends on whether its UP or DOWN line)
+ L298N OUT2 -> Track (-) (opposite polarity to OUT1)
+ L298N ENA -> ESP32 Pin 19 (GPIO)
+ L298N IN1 -> ESP32 Pin 18 (GPIO)
+ L298N IN2 -> ESP32 Pin 5 (GPIO)

*Track B (optional)*:

+ L298N OUT3 -> Track (+) (polarity depends on whether its UP or DOWN line)
+ L298N OUT4 -> Track (-) (opposite polarity to OUT1)
+ L298N ENB -> ESP32 Pin 4 (GPIO)
+ L298N IN3 -> ESP32 Pin 17 (GPIO)
+ L298N IN4 -> ESP32 Pin 16 (GPIO)

Note: You should wire Track A and Track B with the *SAME* polarity. The software will manage direction.

### Step 4: Upload Code

+ Tools -> Serial Monitor in the Arduino IDE
+ Plug the ESP32 into the USB port again
+ Update the sketch with your WiFi information

```
/* WiFi credentials */
const char* ssid = "REPLACE-ME-WITH-YOUR-WIFI-SSID";
const char* password = "REPLACE-ME-WITH-YOUR-WIFI-PASSWORD";
```

Change the above two lines, replace REPLACE-ME-WITH-YOUR-SSID with the SSID (wifi network name).
Change REPLACE-ME-WITH-YOUR-WIFI-PASSWORD with your wifi password.

+ Select verify (check mark) and make sure there are no errors
+ Verify the serial monitor does not show brownout detection
+ If brownout detection error is displayed - use a powered USB hub, shorter cable, different USB port
+ Turn on the 12V power supply to power the L298N
+ Check there is no obvious problems (smoke, burning smell etc) 
+ Select Upload (arrow pointing right)
+ Watch the status screen (Below the code) to make sure its uploaded properly

You should see something like this:

note: Ignore the L283d references, this is a typo/cosmetic error, should say L298N (fixed in 1.0.1).

```
01:01:24.091 -> 

01:01:24.091 -> oorail-track-module, version 1.0.0

01:01:24.091 -> Copyright (c) 2020 IdeaPier LLC (oorail.co.uk)

01:01:24.091 -> 

01:01:24.091 -> For additional information visit:

01:01:24.091 ->  https://oorail.co.uk/tech/ 

01:01:24.091 -> 

01:01:24.091 -> License: GPLv3

01:01:24.091 -> 

01:01:24.091 ->  * Initializing Profile 

01:01:24.191 ->  * Connecting to WiFi... 

01:01:24.689 -> .

01:01:24.689 -> Connected to wifi-oorail

01:01:24.689 -> IP address: 10.20.30.40

01:01:24.689 -> MDNS responder started

01:01:24.689 ->  * HTTP server initialized 

01:01:24.689 ->  * Initializing L283d module...

01:01:24.689 ->  * Initializing Track A...

01:01:24.689 ->  * Initializing Track B...

```

### Step 5 Testing

+ Note the IP address displayed in the serial monitor
+ If there is no IP address, check your wifi settings
+ Put a loco on the track (hint: use one you can easily replace)
+ If you are using Linux or MacOS X you can use curl to test in a terminal
+ Replace 10.20.30.40 below with the IP address displayed in the serial monitor
+ Replace /api/1/tm/profile/hornby with the manufacturer of the loco (see code or docs)
+ Note: only hornby, heljan, dapol, bachmann will work in 1.0.0

If you are using curl (example uses Heljan Class 35)

|command|description|
|-------|-----------|
|curl -X GET http://10.20.30.40/|Get System Information|
|curl -X GET http://10.20.30.40/api/1/tm/profile|What profiles are loaded|
|curl -X GET http://10.20.30.40/api/1/tm/profile/heljan|Load Heljan Profile to Track A|
|curl -X GET http://10.20.30.40/api/1/tm/profile|Check profile loaded|
|curl -X GET http://10.20.30.40/api/1/tm/0/brake/off|Turn brake off|
|curl -X GET http://10.20.30.40/api/1/tm/0/coast|Set speed to normal running speed|
|curl -X GET http://10.20.30.40/api/1/tm/0/speed/current|Check current and target speed|
|curl -X GET http://10.20.30.40/api/1/tm/0/speed/down|Slow down by speed steps|
|curl -X GET http://10.20.30.40/api/1/tm/0/speed/up|Speed up by speed steps|

If you want to use a web browser, just use the URLs above.

Note: The manufacturer profiles are intended to be starting points to play around with the
system. You may need to modify the settings as some manufacturers vary loco to loco.

For example, the dapol settings work great with the Class 52 Westerns, but when used with
Dapol Class 68s they run too fast. Similarly some HST units can run fast with the Hornby
settings.

We will make per locomotive settings available in an upcoming release.





