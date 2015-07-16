# SmartMatrix Library for Teensy 3

The SmartMatrix Library is designed to make it easy to display graphics and scrolling text on an RGB 16x32 or 32x32 display connected to a Teensy 3.1.

Documentation Here:  
[docs.pixelmatix.com/SmartMatrix](http://docs.pixelmatix.com/SmartMatrix)

To download in Arduino Library form, see [Releases](https://github.com/pixelmatix/SmartMatrix/releases) on GitHub, or just add files directly to your project directory

### Software and Teensy Setup
This documentation assumes you have a general knowledge of the Teensy 3.1, how to use the Arduino IDE, and the Teensyduino addon.  If you need an overview of any of those tools, please use these references:

* [PJRC - Teensyduino](http://www.pjrc.com/teensy/teensyduino.html)
* [Arduino - Getting Started with Arduino](http://arduino.cc/en/Guide/HomePage)
* For general Teensy 3.1 support, not related to the SmartMatrix Shield or SmartMatrix Library, post a question at the [PJRC Forum](http://forum.pjrc.com/forums/3-Technical-Support-amp-Questions)

Make sure you have a supported version of the Arduino IDE and Teensyduino add-on installed.

* [Arduino IDE](http://arduino.cc/en/main/software) - version 1.6.5 recomended
* [Teensyduino](http://www.pjrc.com/teensy/td_download.html) - use the latest version

Before continuing, use the blink example in the Arduino IDE to verify you can compile and run a sketch on your Teensy 3.1.

Download the latest version of the SmartMatrix Library:  
[SmartMatrix Releases - GitHub](https://github.com/pixelmatix/SmartMatrix/releases)

Import either the "16x32" or "32x32" library depending on what resolution display you have.  If you want to work with both resolutions, import both.  See instructions from Arduino here:  
[Arduino - Libraries](http://arduino.cc/en/Guide/Libraries)

Start with the FeatureDemo Example project, included with the library.  From the Arduino File menu, choose Examples, the appropriate SmartMatrix library for your display resolution, then FeatureDemo.  

You should already have most of the correct settings to load the FeatureDemo sketch on your Teensy, from running the blink example earlier.  Under Tools, CPU Speed, make sure either 48 MHz or 96MHz (overclock) is selected.  (Some libraries are not compatible with the 72MHz CPU)

Click the Upload button, and the sketch should compile and upload to your Teensy, and start running right away.

You can use the FeatureDemo sketch as a way to get started with your own project.  Inside loop(), find a demo section that is similar to what you want to do with your project, delete the other sections, and save it as as new sketch.

### External Libraries

Some SmartMatrix examples require external libraries to compile.  You may already have older versions of these libraries installed in Arduino that may be too old to work with SmartMatrix and the examples.

Installing Arduino libraries from GitHub has a couple pitfalls.  [This Adafruit tutorial](https://learn.adafruit.com/adafruit-all-about-arduino-libraries-install-use/) explains the basics of installing libraries and how to avoid the pitfalls.

**FastLED**  
If you're having trouble compiling Aurora and are getting errors that refer to FastLED.h, try compiling one of the simpler FastLED examples first, which will help narrow down the issue.  
`FastLED_Controller`  
`FastLED_Functions`

This error means the FastLED library isn't installed (correctly):  
`fatal error: FastLED.h: No such file or directory`

The FastLED version included with Teensyduino may lag behind the latest.  It's better to install FastLED manually using the latest version [available from GitHub](https://github.com/FastLED/FastLED/releases).  If you see any of these errors, you likely have an older version of FastLED installed:  
`no known conversion for argument 4 from 'CRGB' to 'const rgb24&'`  
`error: 'inoise8' was not declared in this scope`

This can be tricky to track down as Teensyduino installs libraries into your Arduino application directory, which might not be in your Arduino sketchbook.  For OSX you will need to navigate into the the Arduino.app package to find the libraries folder and delete the old FastLED.

Install the latest version of FastLED (3.x or higher) from the FastLED releases page:
https://github.com/FastLED/FastLED/releases

**Teensy Audio Library**  
The SpectrumAnalyzer sketch requires the [Teensy Audio Library](http://www.pjrc.com/teensy/td_libs_Audio.html), which is included in [Teensyduino 1.20](http://www.pjrc.com/teensy/td_download.html).  If you have trouble compiling, first make sure you can compile either of the FastLED examples, as FastLED 3.x is also a requirement for this sketch.  If you're missing the Audio library, the best way to install is by running the Teensyduino 1.20 installer.  Make sure the "Audio" library is checked during the install, but don't check all libraries as you will downgrade FastLED.
