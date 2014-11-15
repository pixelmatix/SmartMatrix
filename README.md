# SmartMatrix Library for Teensy 3

Documentation Here:  
[docs.pixelmatix.com/SmartMatrix](http://docs.pixelmatix.com/SmartMatrix)

To download in Arduino Library form, see [Releases](https://github.com/pixelmatix/SmartMatrix/releases) on GitHub, or just add files directly to your project directory



If using multiple panels, see MatrixHardware_KitV1_generic.h.

If using more than 4 panels: use a Teensy 3.1 with cpu speed set to 144 MHz.

To enable 144 MHz file 'boards.txt' for teensy has to be changed:

- Change to your arduino-software installation.
- Open hardware/teensy/boards.txt

All necessary configuration for supporting 144 Mhz and 120 Mhz is already included in boards.txt but commented out.

You simply have to uncomment these two lines:

```
teensy31.menu.speed.144.name=144 MHz (overclock)
teensy31.menu.speed.120.name=120 MHz (overclock)
```

- After restarting your arduino IDE you should be able to change cpu speed to 144 MHz.


If you don't want to set cpu speed to 144 MHz you might have to set MATRIX_REFRESH_RATE to a suitable value.
