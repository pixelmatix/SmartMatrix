# SmartMatrix Library for Teensy 3

## Changes for ESP32

### Overview

The SmartMatrix Library ESP32 port at a low level is based on Sprite_TM's [ESP32 I2S Parallel example](https://esp32.com/viewtopic.php?f=17&t=3188).  The ESP32 can continuously shift data from RAM through the I2S peripheral in parallel to GPIO pins, without using up CPU cycles.  This wasn't obvious to me from reading through the reference manual, and this peripheral doesn't have great documentation or example code besides Sprite_TM's example, so this was an invaluable start to the project.  It was a challenge to move from the example with 21-bit color refresh to approaching the SmartMatrix Library's performance on the Teensy with up to 48-bit color and high refresh rates.  The example code didn't scale well in RAM usage or refresh rate when increasing color depth.  The architecture of the ESP32 and the Freescale processors used in the Teensy 3 family are so different a lot of the tricks I used on the Teensy 3 wouldn't port over.  There are some significant changes from the Teensy Platform, but in general, sketches that used the Teensy SmartMatrix Library should work with the ESP32 SmartMatrix Library.

* Major (ESP32 and Teensy 3 platform) new features in this branch vs the main SmartMatrix Library branch
  * Support for multi-row-mapped panels: these are typically large panels that light up multiple rows per color channel for a brighter output overall.  e.g. a P10 32x16 /4 scan panel, which would light up two rows per color channel, doubling the brightness vs a P10 32x16 /8 scan panel.  See `MultiRowRefreshMapping.ino` sketch for more details on how to find out what mapping your panel has, and how to a new one if SmartMatrix Library doesn't already have a mapping.
  * Initial support for HUB12 panels (e.g. Freetronics DMD panels).  These panels have a single color channel, and invert the data and OE signals.  Because SmartMatrix Library is designed for HUB75 with two parallel color channels, you need to double the physical matrix height in the sketch to allow SmartMatrix Library to refresh the panel with just the first color channel.
    * Usage example with 32x16 /4 scan HUB12 panel: `kMatrixWidth = 32; kMatrixHeight = 16*2; panelType = SMARTMATRIX_HUB12_16ROW_32COL_MOD4SCAN; kMatrixOptions = (SMARTMATRIX_OPTIONS_HUB12_MODE);`
    * Pinout (HUB12->HUB75): OE=OE, A = ADX0, B = ADX1, C=ADX2, CLK/S = CLK, SCK/L = LAT, R = R1, GND=GND, No connect: E/G, D
    * Only the R1 data line is inverted at this time.  It could be possible to refresh six HUB12 panels in parallel (three from each color channel, two color channels), with some minor changes to the library, but would require a lot more work in mapping to keep track of which X/Y/color set goes to which physical pixel on the panels.

* ESP32 Port Differences from Teensy
  * Because of more RAM available and the DMA architecture on the ESP32, two entire refresh frames are used.  Refreshing the panel can take up little to no CPU usage with some more SmartMatrix changes.
  * Layer update rate is decoupled from the panel refresh rate.  By default, the Layer refresh rate is set to half of the panel refresh rate.  e.g. if you set matrix.refreshRate() to 120, and repeatedly call backgroundLayer.swapBuffers(), it will swap at max 60 times per second.
    - Call matrix.setCalcRefreshRateDivider(uint8_t newDivider) to change the division from the default of 2 to something else.  Be careful with setting to 1 with a high refresh rate, as you can end up with no CPU left to run your sketch (this will look like a blank display)
  * Refresh rate is not set exactly, it is a minimum value.  It must be set before calling matrix.begin(), or the default 120 Hz will be used
  * The method of refreshing the panel is different from Teensy.  With the current method, there is a strong tradeoff between color depth and refresh rate vs maximum brightness.  If you set a higher color depth or refresh rate, you may find that the panel isn't refreshing as bright as it was previously.
  * Refresh Rate is calculated very granularly, in powers of 2, and the calculation is based on color depth and matrix size, and will double until it reaches a maximum value, or exceeds the minimum value you set (or the default of 120 Hz).  e.g. If the minimum refresh rate is set to 120, and the refresh rate calculated for your color depth is 115 Hz (very close to 120 Hz but still lower), the library will double it and your refresh rate will be 230Hz, sacrificing maximum brightness and using up more CPU to update the frames more frequently.  Refresh Rate details are printed to the serial terminal so you can tweak Sketch parameters to get the best refresh rate for your situation.
  * kDmaBufferRows is not used by the ESP32 port
  * Memory must be dynamically allocated for the ESP32, so the global buffer sizes printed by Arduino compilation are hiding a significant amount of memory that won't be available for your sketch.  matrix.begin() includes printfs() with debug information on memory usage and memory available
    - matrix.begin() can be called with an optional argument of minimum number of bytes of DMA capable memory to keep free (so other code can malloc them after matrix.begin()).  More details below.
    - Note that the values of "32-bit Memory" and "Heap Memory" in the printfs is a bit deceiving as not all of that can be allocated by the SmartMatrix Library or most other ESP32 Arduino libraries, so it's effectively unusable RAM.  The amount of 8-bit and DMA RAM are more important stats as those are required for most libraries.

* Not Yet Fully Working
  * Still seeing some crashes related to memory usage early in sketch when other memory intensive objects (e.g. WiFi library) are included in the sketch?  Need to reproduce and track down
    - May see a rolling reset before the sketch can do much
      - This is hopefully fixed now.  Please post to the [SmartMatrix Community](community.pixelmatix.com) or create a GitHub Issue if you see this error: "Guru Meditation Error: Core 1 panic'ed (Cache disabled but cached memory region accessed)"
    - May see some libraries failing, e.g. in AnimatedGIFs sketch with a large panel and high color depth, calling SD.begin() after matrix.begin() results in "No SD Card" error, likely because there was not enough RAM (specifically DMA-capable RAM) available for the SD library.  Moving the matrix.begin() call later works as the SmartMatrix Library adapts its DMA descriptor memory usage to the amount of DMA RAM available.  Also using the new `dmaRamToKeepFreeBytes` parameter when calling matrix.begin() will try to keep that amount of DMA RAM free.  The SD Library requires around 28000 bytes free, so you can call `matrix.begin(28000)` before SD.begin().
    - Todo: malloc matrixUpdateFrames buffers before other smaller buffers as they are (by far) the largest buffers to allocate?
  * Need a ~10ms delay between matrix.begin() and drawing to backgroundLayer or starting scrolling text (or drawing to indexed layer?) or initial drawing will not be displayed or might be corrupted (e.g. scrolling text shown on top of previous position of text)
    - Is this because the first call to matrixCalcCallback() from refresh will be skipped by the calcRefreshRateDivider?
  * AnimatedGIFs sketch is a bit fragile because of the ESP32 SD library 
    * In general, resetting a sketch while the SD library is connected to the SD card can result communication with the SD card not working after reset - fix it with a power cycle
  * AnumatedGIFs sketch could have performance improved: there's a lot of time spent waiting for a swapBuffers() call to complete, as it needs to wait for the next frame transition, which is just wasting time if the frame rate is set relatively low to allow more sketch CPU time for GIF decoding).
  * Only updating panel buffers when there are Layer changes, reducing CPU usage even further
  * APA102 strip support (bringing to parity with the new Teensy APA102 driver that's in this branch)
  * C-shaped Chaining of panels to create multiple rows is broken (Z-shaped is working)
  * Refresh fails at higher display sizes: 128x32 is largest that has been seen working.  Not sure about cause of failure
  * Not all examples work on the ESP32 platform
    - SpectrumAnalyzer requires Teensy
    - MatrixClock hasn't been tested
    - FASTLED_Panel_Plus_APA hasn't been tested
  * Extensive testing on Teensy code in this branch
    - The Teensy portion of the library should work identical to how it did before, but the code has been extensively refactored.  There may be some errors.
    - There is a new APA102 driver using SmartMatrix Library to get pseudo-13-bit color out of the normally 8-bit per color APA102 LEDs.  See FastLED_Panel_Plus_APA example
  * Scrolling text speed is maximum one pixel shift per frame, which is quite slow with slow calculation framerates - need to allow scrolling text more than once per frame?
  * Scrolling text has been reported to be scrolling backwards
  * Use the 96k 32-bit addressible RAM for something, so it doesn't go to waste, and more of the 8-bit addressible DMA RAM is available to drive larger panels?
    - 96kB fits two frames of 128x128xRGB24, a good fit for a double-buffered background layer.  Problem is RGB24 fits in 3 bytes and is normally accessed a byte at a time to set individual colors.  It would need to be accessed 32-bit aligned, which would be a bit of overhead, reading (up to) two 32-bit words and writing (up to) two words back to change values in a RGB24 struct.  Backbuffer() wouldn't work well as you can't provide a RGB24 pointer to the user.  

### Hardware

The library has only been tested with Espressif's ESP32 Dev Kit C.

You can hook the ESP32 Dev Kit C directly up to a panel, following the circuit that's documented in code in Sprite_TM's example, and in the `MatrixHardware_ESP32_V0.h` header.  You'll need to change the definition of `GPIOPINOUT` in that header to `ESP32_FORUM_PINOUT` if you're not using the SmartLED Shield circuit.

Some panels won't work with the 3.3V levels output by the ESP32, and you'll need 5V level shifting buffers like the shields I designed use.  Additionally, the shields have some other features that make them preferable to using just an ESP32 (and optionally level shifting buffers).

- The 5x ADDX lines are output using the RGB data lines and stored using an external latch, freeing up more pins on the ESP32
- With the addition of the external latch, there are only 8 bits of data to output via I2S, and so each clock cycle's data fits into a uint8_t instead of uint16_t.  With the I2S peripheral in 8-bit mode instead of 16-bit mode, the amount of RAM used to store refresh buffers is cut in half
- There's an additional circuit that uses the MCPWM peripheral to output short OE pulses - shorter than can be output by the I2S peripheral - enabling displaying at least an extra bit's worth of color depth at high refresh rates or lower brightnesses.
- Wiring is so much easier

Schematics, Eagle PCB files, and BOMs are in the `extras/hardware` folder.  There's both a THT and SMT shield, neither have been fully tested.  I've tested the THT version excluding the APA102 LED circuit.  A contributor to the project has tested the matrix driving portion of the SMT shield.  The THT shield was used for initial library dev and won't be maintained after a V1 of the SMT shield is released.  You can find some parts for the THT shield in the SMT BOM listed under a "THT" column.  If there is sufficient interest, I can finalize SmartLED Shield for ESP32 V1, and make an assembled version for sale.

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
