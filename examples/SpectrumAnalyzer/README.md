SmartMatrix Basic Spectrum Analyzer 
================================

Basic spectrum analyzer on a 32x32 RGB LED matrix using SmartMatrix, FastLED, and the Teensy Audio Library.

[![video](http://img.youtube.com/vi/w4uL2426eaQ/0.jpg)](http://www.youtube.com/watch?v=w4uL2426eaQ)  

Based on SpectrumAnalyzerBasic by Paul Stoffregen: https://github.com/PaulStoffregen/Audio/blob/master/examples/Analysis/SpectrumAnalyzerBasic/SpectrumAnalyzerBasic.ino

Requires a SmartMatrix shield: http://docs.pixelmatix.com/SmartMatrix

Requires the following libraries:

* Teensy Audio Library: https://github.com/PaulStoffregen/Audio
* Smartmatrix Library for Teensy 3: https://github.com/pixelmatix/smartmatrix
* FastLED v2.1: https://github.com/FastLED/FastLED/tree/FastLED2.1 ** will not work with older versions/branches **

Uses line in on pin A2.  For more information, and a recommended analog input circuit, see: http://www.pjrc.com/teensy/gui/?info=AudioInputAnalog

[![recommended analog input circuit](http://www.pjrc.com/teensy/gui/adccircuit.png)](http://www.pjrc.com/teensy/gui/?info=AudioInputAnalog)