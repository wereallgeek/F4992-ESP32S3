
# F4992-ESP32-S3 Smart Turntable Addon - Pseudo volume control

## Purpose

This Add-on utilize the turntable's anti-twomp muting system control volume by enabling a controlled muting system.
This is not a pure volume control and even less an amplifier. By controlling the muting system via PWM, tonality will also be affected. 

## Hardware

The original muting circuitry of the turntable is composed of one transistor per channel (left and right) which are fed by another transistor, that has GPIO signal in-between a voltage divider.
This add-on takes advantage of this configuration by sending a PWM instead of pure ON/OFF to the transistor, which in turn would send a varying voltage level to each channel's mute control transistors

This is far from an amplifier, but we can somewhat use it as an attenuator.

At no point does the ESP32 get in the audio path.

## GUI

The HA Volume Control section of the web config page has these options

Volume enabled - turn ON/OFF the feature. when disabled, the muting system behaves as it was designed in the 80s.
Vol low PWM - the PWM value (0..255) for 1% volume
Vol High PWM - the PWM value (0..255) for 99% volume

0% volume always sends PWM 0
100% volume always sends PWM 255
But in-between, not all values have audible differenciation; by tweaking the low/high PWM values we get more granularity on perceptible volume difference.
These values likely change with the specific turntable and amplifier used.
