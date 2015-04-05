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


Changes to pixelmatix' branch
------------------------------

* Support for bigger resolutions (= chained panels)
* Removed includes of SmartMatrix_32x32.h, using SmartMatrix.h instead
* Use generic getScreenWidth() and getScreenHeight() where possible
* SpectrumAnalyzer: added different modes, now scales to bigger resolutions
* Foreground functions can be disabled to save memory  (#define DISABLE_FOREGROUND_FUNCTIONS)
* Generic MatrixHardware_KitV1_generic.h may be used instead of MatrixHardware_KitV1_32x32.h or _16x32.h


Added example
-------------

* MatrixClock3: Uses the internal RTC of Teensy 3.0 or Teensy 3.1,
  mimics 7-segment displays and shows time and date.


Bugs in this branch
-------------------

* Foreground (scrolling and drawing) functions only work for rotation = 0

