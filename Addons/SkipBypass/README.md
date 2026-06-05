
# F4992-ESP32-S3 Smart Turntable Addon - Skip Bypass

## Purpose

This Add-on is purely software, no hardware required.

This feature lets the F4992-ESP32S3 "detect" when a record skips and take corrective actions by lifting the arm, moving it forward slightly, and lowering it to continue playback.



## Hardware

Understanding the linear tracking system.
When Sansui designed the P-L series turntables they made a very powerful follower-circuit for the arm. In "Servo" mode, the arm angle is detected and the tray moves forward to bring the arm back to a correct angle.
All of this is done without any software- purely in electroncis equilibrium.
There are hardware tweaks that can be done, as seen in figure 6-5 in the Sansui P-L45 & P-L55 automatic direct-drive turntable service manuals.

fVR1 adjusts how easy it is to trigger movement
fVR2 adjusts how fast this movement will be

When properly adjusted, the arm will remain in-equilibrium on its own, but slight angle towards the center of the recors and corrective movement will happen. The smaller the movement the better. The arm should move often, but not by much.


## GUI

The Auto record-skipping bypass section of the online configuratino tool gives only a few options but they need to be properly tweaked.

lenght - the duration of immobility required for the corrective action to take place. This needs to be tweaked to match fVR1 & fVR2 and how the Servo arm motion behaves.
ammount - the number of steps the tray will take when correcting. Too small and it may require more than one correction. Too big and you skip more music than required
enable - turn the add-on on/off.
