/*
 * Based on SpectrumAnalyzerBasic by Paul Stoffregen included in the Teensy Audio Library
 * Modified by Jason Coon for the SmartMatrix Library
 * Requires Teensyduino 1.20 and the Teensy Audio Library
 * Also requires FastLED 3.0 or higher
 * If you are having trouble compiling, see
 * the troubleshooting instructions here:
 * http://docs.pixelmatix.com/SmartMatrix/#external-libraries
 *
 * Requires the following libraries:
 * Teensy Audio Library: https://github.com/PaulStoffregen/Audio
 * Smartmatrix Library for Teensy 3: https://github.com/pixelmatix/smartmatrix/releases
 * FastLED v3.0 or higher: https://github.com/FastLED/FastLED/releases
 *
 * Uses line in on pin A2.  For more information, and a recommended analog input circuit, see: http://www.pjrc.com/teensy/gui/?info=AudioInputAnalog
 *
 * You can change the pin used for ADC with the ADC_INPUT_PIN definition below.
 * There are no dedicated ADC pins brought out on the SmartMatrix Shield,
 * but even if you've used all the pins on the SmartMatrix expansion header,
 * you can use solder pins directly to the Teensy to use A14/DAC, A11, or A10
 *
 * Additional analyzer modes and scaling to bigger displays by
 * Wolfgang Astleitner (mrwastl@users.sf.net)
 */

// all these libraries are required for the Teensy Audio Library
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>

#include <SmartMatrix.h>
#include <FastLED.h>

SmartMatrix matrix;

#define ADC_INPUT_PIN   A2

// undefine to disable switch button support
#define SWITCH_INPUT_PIN 19


// spectrum analyzer modes:
//  0 ... horizontal lines
//  1 ... rainbow bars
//  2 ... filled rainbow bars
//  3 ... filled 3-level bars
//  4 ... filled rainbow outlined bars
//  5 ... filled 3-level outlined bars
int analyzerMode = 1;

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

const rgb24 black = CRGB(0, 0, 0);

byte status = 0;

// width of a bar
int barWidth = 0;
int barY;

// start and end position of bar (including outline)
int barXstart;
int barXend;
// bottom line of bar
int barBottom;

// analyzer type 3 and 4
//const int barLim[2] = { (matrix.getScreenHeight() * 50) / 100, (matrix.getScreenHeight() * 25) / 100 };
const int barLim[2] = { (matrix.getScreenHeight() * 40) / 100, (matrix.getScreenHeight() * 20) / 100 };
//const rgb24 barCols[3] = { CRGB(0, 255, 0), CRGB(255, 255, 0), CRGB(255, 0, 0) };
//const rgb24 barBorderCols[3] = { CRGB(0, 127, 0), CRGB(127, 127, 0), CRGB(127, 0, 0) };
const rgb24 barCols[3] = { CRGB(0, 191, 0), CRGB(255, 255, 0), CRGB(255, 0, 0) };
const rgb24 barBorderCols[3] = { CRGB(0, 91, 0), CRGB(127, 127, 0), CRGB(127, 0, 0) };

// colour of current bar
rgb24 color;
rgb24 borderColor;

int shiftCol = 0;
int shiftSpeed = 1;
int shiftCounter = 0;
int shiftStep = 2;

#ifdef SWITCH_INPUT_PIN
int buttonDelay = 0;
#endif

void setup()
{
    Serial.begin(9600);

    // Initialize LED Matrix
    matrix.begin();
    matrix.setBrightness(255);

#ifdef SWITCH_INPUT_PIN
    pinMode(SWITCH_INPUT_PIN, INPUT_PULLUP);
#endif

    // scale the bars horizontally to fill the matrix width
    barWidth = matrix.getScreenWidth() / 16;

    // fallback if width is too small for outline bars
    if (analyzerMode >= 4  && matrix.getScreenWidth() <= 32) {
      analyzerMode = 1;
    }
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

        matrix.fillScreen(black);

        for (int i = 0; i < 16; i++) {
            // TODO: conversion from FFT data to display bars should be
            // exponentially scaled.  But how to keep it a simple example?
            int val = level[i] * scale;

            // trim the bars vertically to fill the matrix height
            if (val >= matrix.getScreenHeight()) val = matrix.getScreenHeight() - 1;

            if (val >= shown[i]) {
                shown[i] = val;
            }
            else {
                if (shown[i] > 0) shown[i] = shown[i] - 1;
                val = shown[i];
            }

            // color hue based on band
            color = CRGB(CHSV( ( (i * 15) + shiftCol ) % 255, 255, 255));

            barXstart = barWidth * i;
            barXend   = barWidth * (i+1) - 1;
            barBottom = matrix.getScreenHeight() - 1;

            // draw the levels on the matrix
            if (shown[i] >= 0) {
                barY = (matrix.getScreenHeight() - 1) - val;
                switch (analyzerMode) {
                  case 1:
                    matrix.drawRectangle( barXstart, barY, barXend, barBottom, color);
                    break;
                  case 2:
                    matrix.fillRectangle( barXstart, barY, barXend, barBottom, color);
                    break;
                  case 4:
                    borderColor = CRGB(CHSV( ( (i * 15) + shiftCol ) % 255, 191, 191));
                    matrix.fillRectangle( barXstart, barY, barXend, barBottom, borderColor, color);
                    // beauty correction for bottom line
                    matrix.drawLine( barXstart + 1, barBottom, barXend - 1, barBottom, color);
                    break;
                  case 3:
                  case 5:
                    // if outline: shift Xstart and Xend
                    if (analyzerMode == 5) {
                      barXstart ++;
                      barXend --;
                    }
                    // draw 'normal volume' part
                    matrix.fillRectangle( barXstart, (barY < barLim[0]) ? barLim[0] : barY, barXend, barBottom, barCols[0]);
                    if (analyzerMode == 5) {
                      matrix.drawLine( barXstart - 1, (barY < barLim[0]) ? barLim[0] : barY, barXstart - 1, barBottom, barBorderCols[0]);
                      matrix.drawLine( barXend + 1, (barY < barLim[0]) ? barLim[0] : barY, barXend + 1, barBottom, barBorderCols[0]);
                    } 
                    // draw 'higher volume' part
                    if (barY < barLim[0]) {
                      matrix.fillRectangle( barXstart, (barY < barLim[1]) ? barLim[1] : barY, barXend, barLim[0] - 1, barCols[1]);
                      if (analyzerMode == 5) {
                        matrix.drawLine( barXstart - 1, (barY < barLim[1]) ? barLim[1] : barY, barXstart - 1, barLim[0] - 1, barBorderCols[1]);
                        matrix.drawLine( barXend + 1, (barY < barLim[1]) ? barLim[1] : barY, barXend + 1, barLim[0] - 1, barBorderCols[1]);
                      } 
                    }   
                    // draw 'loud volume' part
                    if (barY < barLim[1]) {
                      matrix.fillRectangle( barXstart, barY, barXend, barLim[1] - 1, barCols[2]);
                      if (analyzerMode == 5) {
                        matrix.drawLine( barXstart - 1, barY, barXstart - 1, barLim[1] - 1, barBorderCols[2]);
                        matrix.drawLine( barXend + 1, barY, barXend + 1, barLim[1] - 1, barBorderCols[2]);
                      } 
                    }
                    break;                    
                  default:
                    matrix.drawLine( barXstart, barY, barXend, barY, color);
                }
            }
        }

        if (shiftSpeed) {
          if (shiftCounter > shiftSpeed) {
            shiftCounter = 0;
            shiftCol -= shiftStep;
            if (shiftCol > 255) {
              shiftCol = 0;
            }
          } else {
            shiftCounter ++;
          }
        }

#ifdef SWITCH_INPUT_PIN
        // analyzer_mode cycle function with super stupid button press detection
        if (digitalRead(SWITCH_INPUT_PIN) == LOW) {
          if (buttonDelay == 0) {
            analyzerMode ++;
            if (analyzerMode > 5) {
              analyzerMode = 0;
            }
          }
          buttonDelay ++;
          if (buttonDelay > 20) {
            buttonDelay = 0;
          }
        } else {
          buttonDelay = 0;
        }
#endif

        matrix.swapBuffers();

        FastLED.countFPS();
    }
}