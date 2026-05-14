
# F4992-ESP32-S3 turntable controller

## Purpose

Firmware for a ESP32 replacement F4992 microcomputer daughterboard as used in some Sansui linear tracking turntables

In addition to replacing a dead 7507C-114 microprocessor, this project aims at upgrading it with wifi capabilities.
To this end, the ESP32-S3 microcontroller devkit has been selected. 

The resulting device connects via WiFi and communicates via MQTT to integrate with home automation platforms like Home Assistant.

The Wifi connectivity, web interface & MQTT part of this code had been jumpstarted by the ESPUI_Serial_MQTT_Template from n3odym3
see https://github.com/n3odym3/ESPUI_Serial_MQTT_Template

The main Turntable handling code had been coded by Marsupial and the project is documented at [We're all geeks](https://wereallgeeks.wordpress.com/tag/linear-turntable-retrofit/).

The original F4992 microprocessor daughterboard was designed and developped by Sansui in the 1980s and used in a range of linear tracking turntable - notably the *P-L35*, *P-L45* and *P-L55*. 

I have developped this replacement F4992 microprocessor board because I had acquired a Sansui P-L45 turntable that had suffered catastrophic surcharge causing the CPU to stop functionning. This project is aimed at replacing or upgrading such turntables.


## Hardware

The F4992-ESP32-S3 turntable controller software is meant to be run as a F4992 replacement daughterboard. The target daughterboard PCB hosts the ESP32-S3 and when installed on the turntable's motherboard acts as the turntable's original PD7507C CPU. 
It takes full control of all of the turntable's peripherals and adds new functionality.

Here is the daughterboard/motherboard interface pinout and matching GPIO addresses on the ESP32-S3 as used in the turntable controller.

<p align="center">
  <img src="https://raw.githubusercontent.com/wereallgeek/F4992-ESP32S3/main/images/F4992-ESP32S3-pinout.png">
</p>



The replacement daughterboard PCB will be made available at PCBWay's shared projects - **link to follow** - PCBWay supplied the PCBs for prototypes phase 1 and 2 of this project. Thanks again.

<p align="center">
  <img src="https://raw.githubusercontent.com/wereallgeek/F4992-ESP32S3/main/images/PCBv2.png">
</p>


## Connectivity

When the F4992-ESP32-S3 turntable controller is powered ON, it will :

- Load the settings stored in the flash memory (Name, SSID, password, MQTT server, MQTT topic, stats, custom variables)
- Try to connect to the last saved WiFi
- Create a ESPUI Graphical User Interface (unless instructed not to do so - see bootup bypass below)
	- If the ESPUI interface is bypassed, create an emergency port80 webpage server instead
- Start a firmware update (OTA) server at same address, port 4992
- Enables a serial console (which is really only useful when testing the PCB out of the turntable) to answer to (debug) commands
- Run the turntable controller software (unless instructed not to do so - see bootup bypass below)
- **If** the F4992-ESP32-S3 turntable controller **is** connected to the WiFi 	
	- Connects to WiFi, receives its IP address
	- Prints IP address to the serial console to help first setup
	- Connect to the MQTT broker (if MQTT is enabled in settings - see web interface)
- If the ESP **is not** connected to the WiFi 
	- Create an access point (you can connect you smartphone to the acces point to have access to the GUI at **192.168.4.1**)

Note: The F4992-ESP32-S3 turntable controller does **not** have to be connected to wifi to function, but not being online limit its functionality. That said, the turntable is fully usable offline, and configuration can be done trough its own access point if needed.

## Default GUI

Through the Access point or WiFi, accessing port 80 (normal port) of the F4992-ESP32-S3 turntable controller via a web browser connects to an ESPUI web interface.

<p align="center">
  <img src="https://raw.githubusercontent.com/wereallgeek/F4992-ESP32S3/main/images/espui.png">
</p>

The header shows the turntable control, which can be used instead of the physical buttons on the unit.

The Status tab shows information on what the turntable is currently doing.
the WiFi tab serves to setup the unit's name, WiFi and MQTT. it is important to press ENTER on all fields and press SAVE. changes will occur on reboot.
The addons tab serves for some add-ons configuration such as software variants of turntable functions, software volume operations (experimental) and Led pixel strip addition.
The H/W tab is where to tune the turntable controller functionality.
the F/W tab shows current firmware version and offers a link to the Over-The-Air (OTA) firmware update server

## MQTT - using the Turntable as an IOT in Home Assistant.

When connected to WiFi, if the MQTT is properly setup, the turntable will connect to the MQTT broker and let Home Assistant autodiscover discovers it.
Lots of turntable data is available trough MQTT - current state, debug information, usage statistics. In addition to the main device, a *media player* can be discovered if HACS add-on mqtt_media_player is installed.

<p align="center">
  <img src="https://raw.githubusercontent.com/wereallgeek/F4992-ESP32S3/main/images/homeassistant.png">
</p>


## Bootup bypass

An aditionnal feature was added to help software developpers in case of a bad software update. The idea is to bypass most of the processes and use the OTA server to revert firmware without having to open the turntable to repair an otherwise bricked ESP32.

Whenever uploading an experimental firmware causes issues and puts the turntable in a continuous rebooting loop, it is possible to bypass the two main culprit for such panic-crash.
The main issue is ESPUI which is very picky and limitative. The other possibility is bad software initializations in the Turntable controller thread.
When booting up - holding the left or right physical buttons on the turntable would limit what is executed and permit OTA firmware update.

<p align="center">
  <img src="https://raw.githubusercontent.com/wereallgeek/F4992-ESP32S3/main/images/bootbypass.png">
</p>


Note that the software release are tested against this - the boot bypass feature is there to help software developpers.

## Usage


## Serial commands

|Command|Value/function|Example|
|--|--|--|
| ssid | [text] | ssid mywifi |
| password | [text] | password mysecretpassword|
| mqttserver | broker ip | mqttserver 192.168.1.10 | 
| mqttuser | mqtt username | mqttuser itsme | 
| mqttpass | mqtt password | mqttpass anotherpassword | 
| topicin | mqtt (subcribe) topic | topicin demo_in | 
| topicout | mqtt (publish) topic | topic demo_out | 
| mqtten | enable/disable MQTT | mqtten 1 or mqtten 0| 
| restart | [restart the ESP] | restart |
| start | [start/stop button] | start |
| stop | [start/stop button] | stop |
| pause | [arm up/down button] | pause |
| init | [bypass initialisation and move to IDLE] | init |
| info | [print settings] | info (will not print WiFi password) |


## OTA firmware update

Browsing to device_ip:4992/ allows the upload of a .bin file to the ESP32-S3 for ongoing development.

<p align="center">
  <img src="https://raw.githubusercontent.com/wereallgeek/F4992-ESP32S3/main/images/firmware-ota.png">
</p>


## Software

Main software was developped in the Arduino IDE. The ESP32-S3 devkit host has to be properly configured within the arduino IDE - here are the settings that worked for me.

<p align="center">
  <img src="https://raw.githubusercontent.com/wereallgeek/F4992-ESP32S3/main/images/ESP32-S3-Devkit-toolsSettings.png">
</p>


## Arduino software Dependencies 

These dependencies are mandatory to compile the project. Many thanks to the respective developers as their software helped make the turntable controller a reality.

 - [ArduinoJSON 6.x](https://github.com/bblanchon/ArduinoJson)
 - [AsyncTCP](https://github.com/me-no-dev/AsyncTCP) (for ESP32)
 - [ESPAsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer)
 - [ESPAsyncTCP](https://github.com/me-no-dev/ESPAsyncTCP) (for ESP8266)
 - [ESPUI](https://github.com/s00500/ESPUI)
 - [LittleFS](https://github.com/lorol/LITTLEFS)
 - [Preferences](https://github.com/vshymanskyy/Preferences) (for ESP8266)
 - [PubSubClient](https://github.com/knolleary/pubsubclient)
 - [Bounce2](https://github.com/thomasfredericks/Bounce2) hardware switch debouncer
 - [Adafruit Neopixel](https://github.com/adafruit/adafruit_neopixel) Adafruit neopixel driver