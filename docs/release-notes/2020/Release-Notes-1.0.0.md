# Release Notes
### Version 1.0.0
#### Released Tuesday February 4th 2020
##### License GPLv3

## Summary

This is the initial release of the oorail-system. This implements a Digital
DC (*DDC*) Model Railway Control System. For more information on the DDC 
system please visit [https://oorail.co.uk/digital-dc](https://oorail.co.uk/digital-dc)

DO *NOT* use this system with DCC fitted locomotives. DCC ready is OK.

### Known Issues

+ [Bachmann All Lights On Issue](https://github.com/oorail/oorail-system/issues/1)

### New Features

The following new features were implemented in this release:

+ Track Module (*oorail-ddc-tm*)

#### Track Module 

The Track Module provides Web (HTTP) API control of two tracks (A and B) per module.
Each Track Module consists of an ESP32/ESP32S micro controller and an L298N motor
control board. A constant 12V+ supply is provided to the L298N motor board. You can
power the ESP32 either via USB, external 5v supply or using the 5v supply pin on the
L298N board.

The Track Module provides the following features:

+ Light level independent of speed
+ Independent control of two tracks (Track A and Track B)
+ API control of the module via HTTP
+ Forward / Reverse Support
+ Left Hand Running Direction of Travel for Up/Down lines
+ 10-bit (1023 step) digital speed control
+ Configurable PWM, defaults to DC motor safe 28kHz
+ Initial Loco Profile support
+ Preset Crawl, Shunt, Full and Brake settings
+ Initial support for inertia
+ Brake Feature (apply, on / off)
+ Module Health Check Support
+ Initial Module Info / Status / Debug support
+ Profiles for Hornby, Heljan, Dapol, Bachmann and custom manufacturers
+ Accelerate / Decelerate API support
+ Emergency All Stop Support
+ Independent Track Emergency Stop Support
+ Slow to Stop support

### Bugfixes

No bugixes as this is the initial release.

### Design Changes

This is the initial release. Please visit [https://oorail.co.uk/digital-dc](https://oorail.co.uk/digital-dc)
for design information.

The track module API only triggers commands, so if you tell the API to increase speed to normal speed, it
will increment the speed X steps per inertia delay until it reaches that target speed. While it is accelerating
to that speed, if you send another command to run at a slower speed or slow down. The system will immediately
trigger the new command. So if you were accelerating to 500, the current speed is 200 and you tell it to slow down.
It will reduce speed from 200 by X steps per the profile at the inertia delay. This was done deliberately to
enable the API to be used to mimic a physical DC controller knob.

Design points:

+ PWM defaults to 28kHz
+ Track Module is designed to operate using 10-bit (1023 steps)
+ Design assumes Vin of 12V+ into L298N board
+ Design assumes OUT1/OUT2 to Track A and OUT3/OUT4 to Track B
+ Design assumes Track A and Track B are wired in the *SAME POLARITY*
+ Design uses line up / down by default. Use travel API call to adjust
+ Hornby locos require lower value speeds (usually start around 50-60) at 10-bit
+ Heljan and Dapol require higher values to make locomotives move (400+)
+ Profiles are starting points for initial release, you will need to adjust the code
+ Code is designed to work with ESP32/ESP32S - check ENA/ENB/IN1/IN2/IN3/IN4 for pin values
+ You must change the ssid/password values to match your WiFi
+ Track Module IP is displayed on serial (use Arduino IDE + Serial Monitor)
+ Serial Line is hardcoded to 115200 baud
+ Track A is assumed UP
+ Track B is assumed DOWN
+ To use the module as UP/UP replace usage of OORAIL_TRACK_REVERSE_IN1 with OORAIL_TRACK_FORWARD_IN1
+ To use the module as UP/UP replace usage of OORAIL_TRACK_REVERSE_IN2 with OORAIL_TRACK_FORWARD_IN2
+ If you change OORAIL_PWM_RES_DEFAULT to 7-bit or 15-bit, make sure you update OORAIL_DEFAULT_MAXSPEED appropriately
+ Defaults for profiles are assigned in oorail_profile_init(), you can change them in the code there
+ API based profile loading *ONLY* works on Track A, Track B will only run the custom profile

### Caveats

This is an initial release of a new project. It provides basic functionality.
It is being released as it provides improved control over a standard basic DC
controller. Don't expect everything to work and while it has been thoroughly
tested, as with anything else that is new and experimental, your mileage will
vary. Use with caution!

+ Code is provided AS-IS we are not responsible if you blow something up
+ You must attach a serial monitor via USB to get the IP address (or know the MAC of your ESP32 board)
+ API only supports non-asynchronous communication. One command at a time.
+ Brake force, Inertia and speed profiles are initial values, they work but are not optimized
+ Use the ALL stop API command and remove power if you smell something burning
+ Profile support is very basic, it was added to make the initial release easier to use
+ Profile support will be extended in the next release
+ Some Bachmann Branchline locomotives have all running lights on

### Reporting Problems

If you run into a problem please verify that it is an issue with the code and not a problem with
your setup/wiring/components. If you are not sure, use the comments on the YouTube video linked in
the video section below for help. Please make sure that you use the video that corresponds to the
release you are using. The video below is for release 1.0.0.

If you are confident the problem is with the code, please report the [issue here](https://github.com/oorail/oorail-system/issues).

### Requests

Feature requests should be added via [issue system](https://github.com/oorail/oorail-system/issues).

Please make sure you use the prefix *Request:* in the title, and select the label *enhancement*.

### Video

coming soon..

