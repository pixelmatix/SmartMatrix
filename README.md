# SmartMatrix Library for Teensy 3

## Changes for ESP32

* Changes from Teensy Platform
  * Because of more RAM available and the DMA architecture on the ESP32, two entire refresh frames are used.  Refreshing the panel can take up little to no CPU usage with some more SmartMatrix changes.
  * Layer update rate is decoupled from the panel refresh rate.  Currently as a hack to avoid calculations climbing to 100% CPU usage, the Layer refresh rate is set to half of the panel refresh rate.  e.g. if you set matrix.refreshRate() to 120, and repeatedly call backgroundLayer.swapBuffers(), it will swap at max 60 times per second.
  * Refresh rate is not set exactly, it is a minimum value.  It must be set before calling matrix.begin(), or the default 120 Hz will be used
  * The method of refreshing the panel is different from Teensy.  With the current method, there is a strong tradeoff between color depth and refresh rate vs maximum brightness.  If you set a higher color depth or refresh rate, you may find that the panel isn't refreshing as bright as it was previously.
  * Refresh Rate is calculated very granularly, in powers of 2, and the calculation is based on color depth and matrix size, and will double until it reaches a maximum value, or exceeds the minimum value you set (or the default of 120 Hz).  e.g. If the minimum refresh rate is set to 120, and the refresh rate calculated for your color depth is 115 Hz (very close to 120 Hz but still lower), the library will double it and your refresh rate will be 230Hz, sacrificing maximum brightness and using up.  Refresh Rate details are printed the serial terminal so you can tweak Sketch parameters to get the best refresh rate for your situation.
  * kDmaBufferRows is not used by the ESP32 port
  * Memory must be dynamically allocated for the ESP32, so the global buffer sizes printed by Arduino compilation are hiding a significant amount of memory that won't be available for your sketch

* Not Yet Fully Working
  * Brightness must be set before calling matrix.begin(), and cannot be changed during the sketch
  * Chaining of panels to create multiple rows is broken
  * /32 Scan panels
  * Refresh rate isn't being passed to the Layers correctly, so scrolling text may not be scrolling at the speed you expect
  * AnimatedGIFs sketch hasn't been tested
  * Fully decoupling Layer refresh rate from Panel refresh rate.
  * Only updating panel buffers when there are Layer changes, reducing CPU usage even further
  * Refresh buffer reduction in 1/2 if possible (only uint8_t size data is required in I2S buffer but uint16_t is currently used)
  * Add (optional?) printfs inside matrix.begin() to more clearly state how much memory is available and how much is used by SmartMatrix Library, now that dynamic memory is being used
  * Need to add esp32 to Arduino library preferences so the IDE doesn't group the library into "INCOMPATIBLE"

## Overview

The SmartMatrix Library is designed to make it easy to display graphics and scrolling text on multiplexed RGB LED matrix panels connected to a Teensy 3.

Version 3.0 is a significant upgrade from 2.x, with a new API that is not backwards compatible.  See the [release notes on GitHub](https://github.com/pixelmatix/SmartMatrix/releases) for more details, and [MIGRATION.md](https://github.com/pixelmatix/SmartMatrix/blob/sm3.0/MIGRATION.md) for details on migrating sketches from SmartMatrix 2.x to 3.0.  You can have SmartMatrix3 installed in parallel with an existing SmartMatrix_32x32 or SmartMatrix_16x32 library without conflicts.

More documentation Here:  
[docs.pixelmatix.com/SmartMatrix](http://docs.pixelmatix.com/SmartMatrix)

To download in Arduino Library form, see [Releases](https://github.com/pixelmatix/SmartMatrix/releases) on GitHub, or just add the files in the /src directory to your Arduino project.

### Software and Teensy Setup
This documentation assumes you have a general knowledge of the Teensy 3.1/3.2, how to use the Arduino IDE, and the Teensyduino addon.  If you need an overview of any of those tools, please use these references:

* [PJRC - Teensyduino](http://www.pjrc.com/teensy/teensyduino.html)
* [Arduino - Getting Started with Arduino](http://arduino.cc/en/Guide/HomePage)
* For general Teensy 3.1/3.2 support, not related to the SmartMatrix Shield or SmartMatrix Library, post a question at the [PJRC Forum](http://forum.pjrc.com/forums/3-Technical-Support-amp-Questions)

Make sure you have a supported version of the Arduino IDE and Teensyduino add-on installed.

* [Arduino IDE](http://arduino.cc/en/main/software) - version 1.6.5 or later recommended
* [Teensyduino](http://www.pjrc.com/teensy/td_download.html) - use the latest version

Before continuing, use the blink example in the Arduino IDE to verify you can compile and run a sketch on your Teensy 3.1/3.2.

Download the latest version of the SmartMatrix Library:  
[SmartMatrix Releases - GitHub](https://github.com/pixelmatix/SmartMatrix/releases)

Import the library, see instructions from Arduino here:  
[Arduino - Libraries](http://arduino.cc/en/Guide/Libraries)

(Note if you use the Arduino Library Manager, the [AnimatedGIFs](https://github.com/pixelmatix/AnimatedGIFs) example won't be included, as that sketch is included as a Git Submodule which is not supported by Arduino Library Manager.  You will need to download the [AnimatedGIFs](https://github.com/pixelmatix/AnimatedGIFs) example separately)

Start with the FeatureDemo Example project, included with the library.  From the Arduino File menu, choose Examples, SmartMatrix3, then FeatureDemo.  

Important note for SmartLED Shield V4: This line needs to be included before (or instead of) `#include <SmartMatrix3.h>`

```
#include <SmartLEDShieldV4.h> // this line must be first
#include <SmartMatrix3.h> //optionally include this line for SmartLED Shield V4
```

You should already have most of the correct Arduino settings to load the FeatureDemo sketch on your Teensy, from running the blink example earlier.  Under Tools, CPU Speed, make sure either 48 MHz or 96MHz (overclock) is selected.  (Some libraries are not compatible with the 72MHz CPU)

The examples are configured to run on a 32x32-pixel panel.  If your resolution is different, adjust the `kMatrixWidth` and `kMatrixHeight` variables at the top of the sketch.  If you are using a 16x32-pixel panel, also change `kPanelType` to `SMARTMATRIX_HUB75_16ROW_MOD8SCAN`.

New with SmartMatrix Library 3.0, you can chain several panels together to create a wider or taller display than one panel would allow.  Set `kMatrixWidth` and `kMatrixHeight` to the overall width and height of your display.  If your display is more than one panel high, set `kMatrixOptions` to how you tiled your panels:  

* Panel Order - By default, the first panel of each row starts on the same side, so you need a long ribbon cable to go from the last panel of the previous row to the first panel of the next row.  `SMARTMATRIX_OPTIONS_C_SHAPE_STACKING` inverts the panel on each row to minimize the length of the cable going from the last panel of each row the first panel of the other row.  
* Panel Direction - By default the first panel is on the top row.  To stack panels the other way, use `SMARTMATRIX_OPTIONS_BOTTOM_TO_TOP_STACKING`.  
* To set multiple options, use the bitwise-OR operator e.g. C-shape Bottom-to-top stacking: `const uint8_t kMatrixOptions = (SMARTMATRIX_OPTIONS_C_SHAPE_STACKING | SMARTMATRIX_OPTIONS_BOTTOM_TO_TOP_STACKING);`

Click the Upload button, and the sketch should compile and upload to your Teensy, and start running right away.

You can use the FeatureDemo sketch as a way to get started with your own project.  Inside `loop()`, find a demo section that is similar to what you want to do with your project, delete the other sections, and save it as as new sketch.

### External Libraries

Some SmartMatrix examples require external libraries to compile.  You may already have older versions of these libraries installed in Arduino that may be too old to work with SmartMatrix and the examples.

Installing Arduino libraries from GitHub has a couple pitfalls.  [This Adafruit tutorial](https://learn.adafruit.com/adafruit-all-about-arduino-libraries-install-use/) explains the basics of installing libraries and how to avoid the pitfalls.

**FastLED**  
If you're having trouble compiling Aurora and are getting errors that refer to FastLED.h, try compiling the `FastLED_Functions` example first, which will help narrow down the issue.  Also make sure you are using FastLED 3.1 or later.

This error means the FastLED library isn't installed (correctly):  
`fatal error: FastLED.h: No such file or directory`

The FastLED version included with Teensyduino may lag behind the latest.  It's better to install FastLED manually using the latest version [available from GitHub](https://github.com/FastLED/FastLED/releases).  If you see any of these errors, you likely have an older version of FastLED installed:  
`no known conversion for argument 4 from 'CRGB' to 'const rgb24&'`  
`error: 'inoise8' was not declared in this scope`

This can be tricky to track down as Teensyduino installs libraries into your Arduino application directory, which might not be in your Arduino sketchbook.  For OSX you will need to navigate into the the Arduino.app package to find the libraries folder and delete the old FastLED.

Install the latest version of FastLED (3.x or higher) from the FastLED releases page:
https://github.com/FastLED/FastLED/releases

**Teensy Audio Library**  
The SpectrumAnalyzer sketch requires the [Teensy Audio Library](http://www.pjrc.com/teensy/td_libs_Audio.html), which is included in Teensyduino.  If you have trouble compiling, first make sure you can compile either the FastLED example, as FastLED 3.x is also a requirement for this sketch.  If you're missing the Audio library, the best way to install is by running the Teensyduino installer.  Make sure the "Audio" library is checked during the install, but don't check all libraries as you might downgrade FastLED.
