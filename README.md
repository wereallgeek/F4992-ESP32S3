
# F4992 ESP32

## Purpose

Firmware for a ESP32 replacement F4992 microcomputer daughterboard as used in some Sansui linear tracking turntables

In addition to replacing a dead 7507C-114 microprocessor, this project aims at upgrading it with wifi capabilities.
To this end, an ESP32 microcontroller has been selected, and the device would be able to be accessed via Wi-Fi.
It would also converse in MTTQ to an MTTQ server for integration with Home automation software such as Home Assistant.

The Wifi connectivity, web interface & MQTT part of this code had been jumpstarted by the ESPUI_Serial_MQTT_Template from n3odym3
see https://github.com/n3odym3/ESPUI_Serial_MQTT_Template

## Connectivity

When the ESP is powered ON, it will :

- Load the settings stored in the flash memory (SSID, password, MQTT server, MQTT topic, custom variables)
- Try to connect to the last saved WiFi
- Create a Graphical User Interface 
- Respond to serial commands (ssid xxx, password xxx, mqtt xxx, mqtten xxx, ...)
- If the ESP **is** connected to the WiFi 
	- Print the IP of the ESP (you can access the web GUI from this IP)
	- Connect to the MQTT broker (if MQTT is enabled)
	- Run the code in the loop function
- If the ESP **is not** connected to the WiFi 
	- Create an access point (you can connect you smartphone to the acces point to have access to the GUI at **192.168.4.1**)
	- Run the code in the loop function

## Dependencies 

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

## Default GUI

The main tab allows interaction with the turntable's tonearm controller

The Wifi tab allows to setup the WiFi and MQTT (don't forget to save the settings)


## Usage


## Serial commands

|Command|Value/function|Example|
|--|--|--|
| ssid | [text] | ssid mywifi |
| password | [text] | password mysecretpassword|
| mqttserver | broker ip | mqttserver 192.168.1.10 | 
| mqttuser | mqtt username | mqttuser itsme | 
| mqttpass | mqtt password | mqttpass anotherpassword | 
| topicin | mqtt (subcribe) topic| topicin demo_in | 
| topicout | mqtt (publish) topic| topic demo_out | 
| mqtten | enable/disable MQTT| mqtten 1 or mqtten 0| 
| restart | [restart the ESP]| restart |
| info | [print all the settings] | info (will not print WiFi password) |


## OTA firmware update

When connected to wifi, browsing to device_ip:4992/ will allow the upload of a .bin file to the ESP32 for ongoing development.
