# SmartMatrix Library for Teensy 3, Teensy 4, and ESP32

SmartMatrix Library is designed to refresh HUB75 LED matrix panels and APA102-compatible addressable LEDs with high quality graphics, using simple Arduino sketches.

<p align="center"><img src="https://github.com/pixelmatix/SmartMatrix/wiki/photos/examples.gif" alt="" width="50%"/></p>

<p align="center"><i>128x64 HUB75 Panel Driven with SmartLED Shield for Teensy 4</i></p>

SmartMatrix Library 4 has support for Teensy 4.1, Teensy 4.0, Teensy 3.6, Teensy 3.5, Teensy 3.2/3.1, Teensy 3.0, as well as experimental support for ESP32.

The code to refresh HUB75 panels takes advantage of platform-specific peripherals like DMA, I2S, FlexIO, and once started runs in the background using interrupts.  It takes a lot of work to port SmartMatrix Library to a new platform.  We're working on an open source solution that will allow a lot more platforms to drive HUB75 panels, [sign up for updates here](https://github.com/pixelmatix/SmartMatrix/issues/131) if you're interested.

The documentation in this README contains the basic information you may need to run your first SmartMatrix Library sketch, but there is more detailed documentation in the [SmartMatrix Wiki](https://github.com/pixelmatix/SmartMatrix/wiki).

## Hardware

SmartMatrix Library runs best on the Teensy 4.0 and 4.1 using the SmartLED Shield for Teensy 4, available from [Crowd Supply](https://www.crowdsupply.com/pixelmatix/smartled-shield-for-teensy-4) and other distributors.  If you want to use the less powerful but more mature Teensy 3, you can use the SmartLED Shield for Teensy 3, available from [Adafruit](https://www.adafruit.com/product/1902), [SparkFun](http://sparkfun.com/products/15046), [Digi-Key](https://www.digikey.com/product-detail/en/sparkfun-electronics/DEV-15046/1568-1954-ND/9739875), and other distributors around the globe.  The shield doesn't require any soldering to get started, besides putting pins on your Teensy board.

<p align="center"><img src="https://github.com/pixelmatix/SmartMatrix/wiki/photos/slsv4.jpg" alt="" width="50%" /></p>

<p align="center"><i>SmartLED Shield for Teensy 3 - Photo Courtesy Adafruit</i></p>

There's an [adapter PCB design](https://community.pixelmatix.com/t/teensy-4-0-released/498/32) to upgrade SmartLED Shield for Teensy 3 to work with the Teensy 4.

You can wire up a [bare Teensy 3.x to a HUB75 panel](http://docs.pixelmatix.com/SmartMatrix/shieldref.html#smartled-shield-formerly-smartmatrix-shield-overview-technical-details-manually-connecting-teensy-and-panel), but at a minimum it's recommended to use 5V level shifters to drive the panels with the voltage level they are expecting.  There's a recommended circuit with latch chip to use with the Teensy 3 that will reduce the amount of pins used.  The Teensy 4 requires an external latch.

The shields are Open Source Hardware, with design files posted in the `/extras/hardware/` directories.

## Teensy 4

Teensy 4 support was contributed by [Eric Eason](https://github.com/easone)

Teensy 4 APA102 support depends on FlexIO_t4 by KurtE, which is included as a submodule in `src/lib/`.  The original FlexIO_t4 library is [on GitHub](https://github.com/KurtE/FlexIO_t4)

## ESP32

The ESP32 platform is supported with SmartMatrix Library 4.0, but not all features are up to par with the Teensy 3/4 ports.  For details on the ESP32 port, see the [Wiki](https://github.com/pixelmatix/SmartMatrix/wiki/ESP32-Wiring)

## Changes from SmartMatrix Library 3.x

- Sketches written for SmartMatrix Library 3.x should work with SmartMatrix Library 4.0 with a few changes.
- See MIGRATION.md for details on how to update your SmartMatrix Library 3.x sketches for SmartMatrix Library 4.x
- A lot of files were subtly renamed, just changing the case.  If you're trying to use git to check out a commit and get an error like `The following untracked working tree files would be overwritten by checkout`, you may need to use git from the  command line and add the `-f` parameter to force checkout (throwaway local modifications), as your git client might think it's overwriting the case-changed files and losing data.

### New Features in SmartMatrix Library 4.0

- Support for Teensy 4 and ESP32
- Support for driving APA102 LEDs on Teensy platforms
- New "GFX" layers rewritten for better efficiency, and using Adafruit_GFX for drawing, fonts, including much larger fonts
- Support for panels with non-standard mapping, e.g. 16x32/4 (MOD4) panels
- See more features in the [SmartMatrix Wiki](https://github.com/pixelmatix/SmartMatrix/wiki)

## HUB75 Panels

HUB75 RGB panels are typically used for LED billboards (e.g. Times Square), making them cost-effective and readily available. They’re much cheaper per-pixel than addressable LEDs, and available in a wide range of pixel pitch (as of now, 2 mm spacing up to 10 mm spacing per LED). They do require an external controller to continually send data to the panels to refresh them line by line, and that’s where the SmartLED Shield and SmartMatrix library come in. Adafruit, Sparkfun, and other distributors carry panels that are known to be compatible with SmartLED Shield and the SmartMatrix library, but most panels on AliExpress and other sources are compatible as well.

The pixel pitch and "RGB" are good search terms on Aliexpress, e.g. "P6 RGB" for a 6 mm pitch RGB HUB75 panel.

<p align="center"><img src="https://github.com/pixelmatix/SmartMatrix/wiki/photos/hub75panels.jpg" alt="" width="50%" /></p>

<p align="center"><i>HUB75 Panels Ranging from P2 to P10 pitch</i></p>

## Getting Started

To download in Arduino Library form, see [Releases](https://github.com/pixelmatix/SmartMatrix/releases) on GitHub, or use Arduino Library Manager.

### Software and Teensy Setup

This documentation assumes you have a general knowledge of the Teensy 3 or Teensy 4, how to use the Arduino IDE, and the Teensyduino addon.  If you need an overview of any of those tools, please use these references:

* [PJRC - Teensyduino](http://www.pjrc.com/teensy/teensyduino.html)
* [Arduino - Getting Started with Arduino](http://arduino.cc/en/Guide/HomePage)
* For general Teensy support, not related to the SmartMatrix Shield or SmartMatrix Library, post a question at the [PJRC Forum](http://forum.pjrc.com/forums/3-Technical-Support-amp-Questions)

Make sure you have a supported version of the Arduino IDE and Teensyduino add-on installed.

* [Arduino IDE](http://arduino.cc/en/main/software) - version 1.6.5 or later recommended
* [Teensyduino](http://www.pjrc.com/teensy/td_download.html) - use the latest version

Before continuing, use the blink example in the Arduino IDE to verify you can compile and run a sketch on your Teensy 3.1/3.2.

Download the latest version of the SmartMatrix Library, or install it from Arduino Library Manager:  
[SmartMatrix Releases - GitHub](https://github.com/pixelmatix/SmartMatrix/releases)

Note: "SmartMatrix" Library used to be listed in Arduino Library Manager under "SmartMatrix3".  You may need to look for the library in Arduino Library Manager or your libraries folder under either "SmartMatrix" or "SmartMatrix3" as we transition to the new name.

If you're not using Arduino Library Manager, you need to import the library into Arduino, see instructions from Arduino here:  
[Arduino - Libraries](http://arduino.cc/en/Guide/Libraries)

Some of the examples depend on other libraries, which you can download separately, or install from Arduino Library Manager.  See "External Libraries" below.

Start with the FeatureDemo Example project, included with the library.  From the Arduino File menu, choose Examples, SmartMatrix3 (or SmartMatrix), then FeatureDemo.  

Find the section at the top with the note `// uncomment one line to select your MatrixHardware configuration`, and uncomment the file appropriate for your hardware.

You should already have most of the correct Arduino settings to load the FeatureDemo sketch on your Teensy, from running the blink example earlier.  For Teensy 3: under Tools, CPU Speed, make sure either 48 MHz or 96MHz (overclock) is selected.  (Some libraries are not compatible with the 72MHz CPU).  For Teensy 4, use the default CPU speed.

The examples are configured to run on a 32x32-pixel panel.  If your resolution is different, adjust the `kMatrixWidth` and `kMatrixHeight` variables at the top of the sketch.  You may also need to change `kPanelType`.  Some common kPanelType settings:

- 32-pixel high panels, e.g. 32x32, 64x32: `SM_PANELTYPE_HUB75_32ROW_MOD16SCAN`
- 16-pixel high panels, e.g. 32x16: `SMARTMATRIX_HUB75_16ROW_MOD8SCAN`
- 64-pixel high panels, e.g. 64x64, 128x64: `SM_PANELTYPE_HUB75_64ROW_MOD32SCAN`
- For other less common panels, see more details in `MatrixCommonHub75.h` and [the wiki][https://github.com/pixelmatix/SmartMatrix/wiki)

You can chain several panels together to create a wider or taller display than one panel would allow.  Set `kMatrixWidth` and `kMatrixHeight` to the overall width and height of your display.  If your display is more than one panel high, set `kMatrixOptions` to how you tiled your panels:  

* Panel Order - By default, the first panel of each row starts on the same side, so you need a long ribbon cable to go from the last panel of the previous row to the first panel of the next row.  `SM_HUB75_OPTIONS_C_SHAPE_STACKING` inverts the panel on each row to minimize the length of the cable going from the last panel of each row the first panel of the other row.  
  * Note `SM_HUB75_OPTIONS_C_SHAPE_STACKING` isn't compatible with panels that require the Multi Row Refresh Mapping feature (if your `kPanelType` value includes the column size, it likely requires Multi Row Refresh Mapping, e.g. `SM_PANELTYPE_HUB75_16ROW_32COL_MOD2SCAN`)
* Panel Direction - By default the first panel is on the top row.  To stack panels the other way, use `SM_HUB75_OPTIONS_BOTTOM_TO_TOP_STACKING`.  
* To set multiple options, use the bitwise-OR operator e.g. for C-shape Bottom-to-top stacking: `const uint8_t kMatrixOptions = (SM_HUB75_OPTIONS_C_SHAPE_STACKING | SM_HUB75_OPTIONS_BOTTOM_TO_TOP_STACKING);`

Click the Upload button, and the sketch should compile and upload to your Teensy, and start running right away.

You can use the FeatureDemo sketch (or other example sketches) as a way to get started with your own project.  Inside `loop()`, find a demo section that is similar to what you want to do with your project, delete the other sections, and save it as as new sketch.

### External Libraries

Some SmartMatrix examples require external libraries to compile.  You may already have older versions of these libraries installed in Arduino that may be too old to work with SmartMatrix and the examples.  It's usually best to use [Arduino Library Manager](https://learn.adafruit.com/adafruit-all-about-arduino-libraries-install-use) and get the latest version of the library.

Installing Arduino libraries from GitHub has a couple pitfalls.  [This Adafruit tutorial](https://learn.adafruit.com/adafruit-all-about-arduino-libraries-install-use/) explains the basics of installing libraries and how to avoid the pitfalls.

**GifDecoder and AnimatedGIF**

There are two libraries needed for the `AnimatedGifs` example.  Both can be installed from Arduino Library Manager, or you can manually install from GitHub.

[GifDecoder](https://github.com/pixelmatix/GifDecoder/releases)

[AnimatedGIF](https://github.com/bitbank2/AnimatedGIF/releases)

**Adafruit_GFX**

You can optionally use Adafruit_GFX with SmartMatrix Library.  The new "GFX" layers in SmartMatrix Library are much more efficient, and allow for using large fonts for both static display or scrolling across the screen.

Install Adafruit_GFX using Arduino Library Manager or manually [from GitHub](https://github.com/adafruit/Adafruit-GFX-Library/releases)

**FastLED**

If you're having trouble compiling sketches that use FastLED and are getting errors that refer to FastLED.h, try compiling the `FastLED_Functions` example first, which will help narrow down the issue.  Also make sure you are using FastLED 3.1 or later.

This error means the FastLED library isn't installed (correctly):  
`fatal error: FastLED.h: No such file or directory`

The FastLED version included with Teensyduino may lag behind the latest.  It's better to install FastLED manually using the latest version [available from GitHub](https://github.com/FastLED/FastLED/releases), or using Arduino Library Manager.  If you see any of these errors, you likely have an older version of FastLED installed:  
`no known conversion for argument 4 from 'CRGB' to 'const rgb24&'`  
`error: 'inoise8' was not declared in this scope`

This can be tricky to track down as Teensyduino installs libraries into your Arduino application directory, which might not be in your Arduino sketchbook.  Look at the `ResolveLibrary` messages you get when compiling to make sure that the version of library you want is being used.

You can manually install the latest version of FastLED (3.x or higher) from the FastLED releases page:
https://github.com/FastLED/FastLED/releases

**Teensy Audio Library**

The SpectrumAnalyzer sketches require the [Teensy Audio Library](http://www.pjrc.com/teensy/td_libs_Audio.html), which is included in Teensyduino.  If you have trouble compiling, first make sure you can compile the `FastLED_Functions` example, as FastLED 3.x is also a requirement for this sketch.  If you're missing the Audio library, the best way to install is by running the Teensyduino installer.  Make sure the "Audio" library is checked during the install.

## Troubleshooting

If you need help, the best place to ask for help, or look for others who may have worked through the same issue, is the [SmartMatrix Community](https://community.pixelmatix.com).  Please don't post troubleshooting requests here on GitHub.

If you've found a bug with the code, or want to suggest an improvement, feel free to submit a GitHub Issue or Pull Request.

## Supporting SmartMatrix Library Development

A lot of work went into writing SmartMatrix Library, designing the shields, and releasing them as Open Source Hardware.  There are real costs in maintaining the documentation and community forum.  If they are useful to you and you'd like to say thank you, you can make a [donation via PayPal](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=56RA5NKYXHCLJ&source=https://github.com/pixelmatix/SmartMatrix).  Thank you!
