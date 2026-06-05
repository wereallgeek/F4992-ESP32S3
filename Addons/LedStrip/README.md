
# F4992-ESP32-S3 Smart Turntable Addon - LED STRIP

## Purpose

The F4992-ESP32S3 turntable control LED STRIP add-on serves no purpose on the turntable itself and is purely cosmetic.
By adding a WS2812B LED strip under the top cover of the arm tray area, the user gains visual feedback of what's going on.

There are different feedback animations for initialization, detection, rejection and movement.



## Hardware

The LED strip needs to follow the WS2812B standard for communitation.

The idea is to fit the strip under the top cover of the arm tray area; but different LED Strips have different LED density - more or less LEDs per inch.
The number of leds is configurable in software.

<p align="center">
  <img src="https://raw.githubusercontent.com/wereallgeek/F4992-ESP32S3/main/images/LedStrip.png">
</p>

[PCB Shared Project at PCBWay](https://www.pcbway.com/project/shareproject/F4992_ESP32S3_CPU_board_replacement_586c1eb8.html)

## Connectivity

Only one ESP32 GPIO is required to control as-many LEDs as required. GPIO48 has been selected.
There is a LED connector on the daughterboard that supplies the 3 required wires for a WS2812B LED STRIP

VCC 5V
GPIO48
GROUND

Any user wanting to use a 15V led strip can tap on the turntable's 15V power circuit, but for the needs of this turntable

## GUI

In the Web interface the Led strop addon permits to enable the LED Strip.

<p align="center">
  <img src="https://raw.githubusercontent.com/wereallgeek/F4992-ESP32S3/main/images/ui-ledStrip.png">
</p>

Ledstrip enabled - turn the add-on ON/OFF
Number of leds - the number of LEDs on the STRIP
Brightness - how bright should the LEDs be
Chaser speed - this is for the motion animation speed
Breather adjustment - this is for the initialization and detection animation speed

