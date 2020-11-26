/*
 * SpectrumAnalyzer Analog Example
 *
 * This version uses an ADC input to receive an Audio signal
 * Tested with Teensy 3
 * Not compatible (as of publishing with the Teensy 4)
 * Not compatible with ESP32
 * 
 * Based on SpectrumAnalyzerBasic by Paul Stoffregen included in the Teensy Audio Library
 * Modified by Jason Coon for the SmartMatrix Library
 * Requires Teensyduino 1.20 or higher and the Teensy Audio Library
 * Also requires FastLED 3.1 or higher
 * If you are having trouble compiling, see
 * the troubleshooting instructions here:
 * https://github.com/pixelmatix/SmartMatrix/#external-libraries
 *
 * Requires the following libraries:
 * Teensy Audio Library: https://github.com/PaulStoffregen/Audio
 * SerialFlash Library (a dependency of the Teensy Audio Library): https://github.com/PaulStoffregen/SerialFlash
 * FastLED v3.1 or higher: https://github.com/FastLED/FastLED/releases
 *
 * Uses line in on pin A2.  For more information, and a recommended analog input circuit, see: http://www.pjrc.com/teensy/gui/?info=AudioInputAnalog
 *
 * You can change the pin used for ADC with the ADC_INPUT_PIN definition below.
 * There are no dedicated ADC pins brought out on the SmartMatrix Shield,
 * but even if you've used all the pins on the SmartMatrix expansion header,
 * you can use solder pins directly to the Teensy to use A14/DAC, A11, or A10
 * 
 * This SmartMatrix example uses just the background layer
 */

// all these libraries are required for the Teensy Audio Library
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// uncomment one line to select your MatrixHardware configuration - configuration header needs to be included before <SmartMatrix.h>
//#include <MatrixHardware_Teensy3_ShieldV4.h>        // SmartLED Shield for Teensy 3 (V4)
//#include <MatrixHardware_Teensy4_ShieldV5.h>        // SmartLED Shield for Teensy 4 (V5)
//#include <MatrixHardware_Teensy3_ShieldV1toV3.h>    // SmartMatrix Shield for Teensy 3 V1-V3
//#include <MatrixHardware_Teensy4_ShieldV4Adapter.h> // Teensy 4 Adapter attached to SmartLED Shield for Teensy 3 (V4)
//#include <MatrixHardware_ESP32_V0.h>                // This file contains multiple ESP32 hardware configurations, edit the file to define GPIOPINOUT (or add #define GPIOPINOUT with a hardcoded number before this #include)
//#include "MatrixHardware_Custom.h"                  // Copy an existing MatrixHardware file to your Sketch directory, rename, customize, and you can include it like this
#include <SmartMatrix.h>
#include <FastLED.h>

#define COLOR_DEPTH 24                  // Choose the color depth used for storing pixels in the layers: 24 or 48 (24 is good for most sketches - If the sketch uses type `rgb24` directly, COLOR_DEPTH must be 24)
const uint16_t kMatrixWidth = 32;       // Set to the width of your display, must be a multiple of 8
const uint16_t kMatrixHeight = 32;      // Set to the height of your display
const uint8_t kRefreshDepth = 36;       // Tradeoff of color quality vs refresh rate, max brightness, and RAM usage.  36 is typically good, drop down to 24 if you need to.  On Teensy, multiples of 3, up to 48: 3, 6, 9, 12, 15, 18, 21, 24, 27, 30, 33, 36, 39, 42, 45, 48.  On ESP32: 24, 36, 48
const uint8_t kDmaBufferRows = 4;       // known working: 2-4, use 2 to save RAM, more to keep from dropping frames and automatically lowering refresh rate.  (This isn't used on ESP32, leave as default)
const uint8_t kPanelType = SM_PANELTYPE_HUB75_32ROW_MOD16SCAN;  // Choose the configuration that matches your panels.  See more details in MatrixCommonHub75.h and the docs: https://github.com/pixelmatix/SmartMatrix/wiki
const uint32_t kMatrixOptions = (SM_HUB75_OPTIONS_NONE);        // see docs for options: https://github.com/pixelmatix/SmartMatrix/wiki
const uint8_t kBackgroundLayerOptions = (SM_BACKGROUND_OPTIONS_NONE);

SMARTMATRIX_ALLOCATE_BUFFERS(matrix, kMatrixWidth, kMatrixHeight, kRefreshDepth, kDmaBufferRows, kPanelType, kMatrixOptions);
SMARTMATRIX_ALLOCATE_BACKGROUND_LAYER(backgroundLayer, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kBackgroundLayerOptions);

#define ADC_INPUT_PIN   A2

AudioInputAnalog         input(ADC_INPUT_PIN);
AudioAnalyzeFFT256       fft;
AudioConnection          audioConnection(input, 0, fft, 0);

// The scale sets how much sound is needed in each frequency range to
// show all 32 bars.  Higher numbers are more sensitive.
float scale = 256.0;

// An array to hold the 16 frequency bands
float level[16];

// This array holds the on-screen levels.  When the signal drops quickly,
// these are used to lower the on-screen level 1 bar per update, which
// looks more pleasing to corresponds to human sound perception.
int shown[16];

const SM_RGB black = CRGB(0, 0, 0);

byte status = 0;

void setup()
{
    Serial.begin(115200);

    // Initialize Matrix
    matrix.addLayer(&backgroundLayer); 
    matrix.begin();

    matrix.setBrightness(255);

    // Audio requires memory to work.
    AudioMemory(12);
}

void loop()
{
    if (fft.available()) {
        // read the 128 FFT frequencies into 16 levels
        // music is heard in octaves, but the FFT data
        // is linear, so for the higher octaves, read
        // many FFT bins together.

        // I'm skipping the first two bins, as they seem to be unusable
        // they start out at zero, but then climb and don't come back down
        // even after sound input stops
        level[0] = fft.read(2);
        level[1] = fft.read(3);
        level[2] = fft.read(4);
        level[3] = fft.read(5, 6);
        level[4] = fft.read(7, 8);
        level[5] = fft.read(9, 10);
        level[6] = fft.read(11, 14);
        level[7] = fft.read(15, 19);
        level[8] = fft.read(20, 25);
        level[9] = fft.read(26, 32);
        level[10] = fft.read(33, 41);
        level[11] = fft.read(42, 52);
        level[12] = fft.read(53, 65);
        level[13] = fft.read(66, 82);
        level[14] = fft.read(83, 103);
        level[15] = fft.read(104, 127);

        backgroundLayer.fillScreen(black);

        for (int i = 0; i < 16; i++) {
            // TODO: conversion from FFT data to display bars should be
            // exponentially scaled.  But how to keep it a simple example?
            int val = level[i] * scale;

            // trim the bars vertically to fill the matrix height
            if (val >= kMatrixHeight) val = kMatrixHeight - 1;

            if (val >= shown[i]) {
                shown[i] = val;
            }
            else {
                if (shown[i] > 0) shown[i] = shown[i] - 1;
                val = shown[i];
            }

            // color hue based on band
            SM_RGB color = CRGB(CHSV(i * 15, 255, 255));

            // draw the levels on the matrix
            if (shown[i] >= 0) {
                // scale the bars horizontally to fill the matrix width
                for (int j = 0; j < kMatrixWidth / 16; j++) {
                    backgroundLayer.drawPixel(i * 2 + j, (kMatrixHeight - 1) - val, color);
                }
            }
        }

        backgroundLayer.swapBuffers();
    }
}
