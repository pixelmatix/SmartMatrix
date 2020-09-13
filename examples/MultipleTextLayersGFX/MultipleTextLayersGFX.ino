/*
  This example shows how multiple scrolling text layers can be used
  This example requires the Adafruit_GFX Library, which you can install through Arduino Library Manager
  The "GFX" Layers in SmartMatrix Library 4.0 depend on Adafruit_GFX and give access to using large fonts included
    with Adafruit_GFX, and the ability to add custom fonts
  Performance of the "GFX" layers has been optimized, and 10 scrolling text layers can be used with a 128x64 panel
*/

#define SUPPORT_ADAFRUIT_GFX_LIBRARY

// uncomment one line to select your MatrixHardware configuration - configuration header needs to be included before <SmartMatrix3.h>
//#include <MatrixHardware_ESP32_V0.h>    // This file contains multiple ESP32 hardware configurations, edit the file to define GPIOPINOUT (or add #define GPIOPINOUT with a hardcoded number before this #include)
//#include <MatrixHardware_KitV1.h>       // SmartMatrix Shield for Teensy 3 V1-V3
//#include <MatrixHardware_KitV4.h>       // SmartLED Shield for Teensy 3 V4
//#include <MatrixHardware_KitV4T4.h>     // Teensy 4 Wired to SmartLED Shield for Teensy 3 V4
//#include <MatrixHardware_T4Adapter.h>   // Teensy 4 Adapter attached to SmartLED Shield for Teensy 3 V4
//#include "MatrixHardware_Custom.h"      // Copy an existing MatrixHardware file to your Sketch directory, rename, customize, and you can include it like this
#include <SmartMatrix3.h>

#include <Fonts/Picopixel.h>
#include <Fonts/TomThumb.h>
#include <Fonts/FreeMono12pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>
#include <Fonts/FreeMonoBoldOblique24pt7b.h>
#include <Fonts/FreeSerif24pt7b.h>
#include <Fonts/Org_01.h>
#include <Fonts/Tiny3x3a2pt7b.h>
#include <Fonts/FreeSerifItalic9pt7b.h>
#include <Fonts/FreeSansOblique24pt7b.h>

#define COLOR_DEPTH 24                  // known working: 24, 48 - If the sketch uses type `rgb24` directly, COLOR_DEPTH must be 24
const uint16_t kMatrixWidth = 32;        // known working: 32, 64, 96, 128, 256
const uint16_t kMatrixHeight = 32;       // known working: 16, 32, 48, 64, 128
const uint8_t kRefreshDepth = 36;       // known working: 24, 36, 48 (on Teensy 4.x: 3, 6, 9, 12, 15, 18, 21, 24, 27, 30, 33, 36, 39, 42, 45, 48)
const uint8_t kDmaBufferRows = 4;       // known working: 2-4, use 2 to save memory, more to keep from dropping frames and automatically lowering refresh rate
const uint8_t kPanelType = SMARTMATRIX_HUB75_32ROW_MOD16SCAN;   // use SMARTMATRIX_HUB75_16ROW_MOD8SCAN for common 16x32 panels
const uint8_t kMatrixOptions = (SMARTMATRIX_OPTIONS_NONE);      // see http://docs.pixelmatix.com/SmartMatrix for options
const uint8_t kScrollingLayerOptions = (SM_GFX_MONO_OPTIONS_NONE);

SMARTMATRIX_ALLOCATE_BUFFERS(matrix, kMatrixWidth, kMatrixHeight, kRefreshDepth, kDmaBufferRows, kPanelType, kMatrixOptions);

SMARTMATRIX_ALLOCATE_GFX_MONO_LAYER(scrollingLayer00, kMatrixWidth, kMatrixHeight, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kScrollingLayerOptions);
SMARTMATRIX_ALLOCATE_GFX_MONO_LAYER(scrollingLayer01, kMatrixWidth, kMatrixHeight, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kScrollingLayerOptions);
SMARTMATRIX_ALLOCATE_GFX_MONO_LAYER(scrollingLayer02, kMatrixWidth, kMatrixHeight, kMatrixWidth, kMatrixHeight*2, COLOR_DEPTH, kScrollingLayerOptions);
SMARTMATRIX_ALLOCATE_GFX_MONO_LAYER(scrollingLayer03, kMatrixWidth, kMatrixHeight, kMatrixWidth, kMatrixHeight*2, COLOR_DEPTH, kScrollingLayerOptions);
SMARTMATRIX_ALLOCATE_GFX_MONO_LAYER(scrollingLayer04, kMatrixWidth, kMatrixHeight, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kScrollingLayerOptions);
SMARTMATRIX_ALLOCATE_GFX_MONO_LAYER(scrollingLayer05, kMatrixWidth, kMatrixHeight, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kScrollingLayerOptions);
SMARTMATRIX_ALLOCATE_GFX_MONO_LAYER(scrollingLayer06, kMatrixWidth, kMatrixHeight, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kScrollingLayerOptions);
SMARTMATRIX_ALLOCATE_GFX_MONO_LAYER(scrollingLayer07, kMatrixWidth, kMatrixHeight, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kScrollingLayerOptions);
SMARTMATRIX_ALLOCATE_GFX_MONO_LAYER(scrollingLayer08, kMatrixWidth, kMatrixHeight, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kScrollingLayerOptions);
SMARTMATRIX_ALLOCATE_GFX_MONO_LAYER(scrollingLayer09, kMatrixWidth, kMatrixHeight, kMatrixWidth, kMatrixHeight*2, COLOR_DEPTH, kScrollingLayerOptions);

const int randomSpeedMax = 120;
const int randomSpeedMin = 20;

// select random offset, from range of what's visible on screen (0 - matrixHeight-layerHeight)
void chooseRandomVisibleOffset(SMLayerGFXMono<rgb24, rgb1, 0> *layer) {
  // first get the total number of pixels on the screen in the text's vertical direction
  uint16_t maxHeight = (layer->getLayerRotation() % 2) ? kMatrixWidth : kMatrixHeight;

  // subtract the height of the text
  maxHeight -= layer->getLocalHeight();

  // get random number within range 0..maxHeight
  maxHeight = random(maxHeight);

  // set offset that keeps the text visible
  layer->setOffsetFromTop(maxHeight);
}

rgb24 chooseRandomBrightColor() {
  rgb24 color;

  switch(random(3)) {
    case 0:
      color = rgb24(random(0x80, 0x100), random(0x40), random(0x40));
      break;

    case 1:
      color = rgb24(random(0x40), random(0x80, 0x100), random(0x40));
      break;

    case 2:
      color = rgb24(random(0x40), random(0x40), random(0x80, 0x100));
    default:
      break;
  }
  return color;
}

void setup() {
  delay(2000);

  randomSeed(analogRead(0));

  matrix.addLayer(&scrollingLayer00); 
  matrix.addLayer(&scrollingLayer01); 
  matrix.addLayer(&scrollingLayer02); 
  matrix.addLayer(&scrollingLayer03); 
  matrix.addLayer(&scrollingLayer04); 
  matrix.addLayer(&scrollingLayer05); 
  matrix.addLayer(&scrollingLayer06); 
  matrix.addLayer(&scrollingLayer07); 
  matrix.addLayer(&scrollingLayer08); 
  matrix.addLayer(&scrollingLayer09); 

  matrix.begin();

  scrollingLayer00.setMode((ScrollMode)random(3));
  scrollingLayer01.setMode((ScrollMode)random(3));
  scrollingLayer02.setMode((ScrollMode)random(3));
  scrollingLayer03.setMode((ScrollMode)random(3));
  scrollingLayer04.setMode((ScrollMode)random(3));
  scrollingLayer05.setMode((ScrollMode)random(3));
  scrollingLayer06.setMode((ScrollMode)random(3));
  scrollingLayer07.setMode((ScrollMode)random(3));
  scrollingLayer08.setMode((ScrollMode)random(3));
  scrollingLayer09.setMode((ScrollMode)random(3));

  scrollingLayer00.setColor(chooseRandomBrightColor());
  scrollingLayer01.setColor(chooseRandomBrightColor());
  scrollingLayer02.setColor(chooseRandomBrightColor());
  scrollingLayer03.setColor(chooseRandomBrightColor());
  scrollingLayer04.setColor(chooseRandomBrightColor());
  scrollingLayer05.setColor(chooseRandomBrightColor());
  scrollingLayer06.setColor(chooseRandomBrightColor());
  scrollingLayer07.setColor(chooseRandomBrightColor());
  scrollingLayer08.setColor(chooseRandomBrightColor());
  scrollingLayer09.setColor(chooseRandomBrightColor());

  scrollingLayer00.setSpeed(random(randomSpeedMax-randomSpeedMin) + randomSpeedMin);
  scrollingLayer01.setSpeed(random(randomSpeedMax-randomSpeedMin) + randomSpeedMin);
  scrollingLayer02.setSpeed(random(randomSpeedMax-randomSpeedMin) + randomSpeedMin);
  scrollingLayer03.setSpeed(random(randomSpeedMax-randomSpeedMin) + randomSpeedMin);
  scrollingLayer04.setSpeed(random(randomSpeedMax-randomSpeedMin) + randomSpeedMin);
  scrollingLayer05.setSpeed(random(randomSpeedMax-randomSpeedMin) + randomSpeedMin);
  scrollingLayer06.setSpeed(random(randomSpeedMax-randomSpeedMin) + randomSpeedMin);
  scrollingLayer07.setSpeed(random(randomSpeedMax-randomSpeedMin) + randomSpeedMin);
  scrollingLayer08.setSpeed(random(randomSpeedMax-randomSpeedMin) + randomSpeedMin);
  scrollingLayer09.setSpeed(random(randomSpeedMax-randomSpeedMin) + randomSpeedMin);

  scrollingLayer00.setFont(&TomThumb);
  scrollingLayer01.setFont(&FreeMono12pt7b);
  scrollingLayer02.setFont(&FreeMonoBold18pt7b);
  scrollingLayer03.setFont(&FreeMonoBoldOblique24pt7b);
  scrollingLayer04.setFont(&FreeSerif24pt7b);
  scrollingLayer05.setFont(&Org_01);
  scrollingLayer06.setFont(&Picopixel);
  scrollingLayer07.setFont(&Tiny3x3a2pt7b);
  scrollingLayer08.setFont(&FreeSerifItalic9pt7b);
  scrollingLayer09.setFont(&FreeSansOblique24pt7b);

  scrollingLayer00.setRotation(random(4));
  scrollingLayer01.setRotation(random(4));
  scrollingLayer02.setRotation(random(4));
  scrollingLayer03.setRotation(random(4));
  scrollingLayer04.setRotation(random(4));
  scrollingLayer05.setRotation(random(4));
  scrollingLayer06.setRotation(random(4));
  scrollingLayer07.setRotation(random(4));
  scrollingLayer08.setRotation(random(4));
  scrollingLayer09.setRotation(random(4));

  scrollingLayer00.start("Layer 00", -1);
  scrollingLayer01.start("Layer 01", -1);
  scrollingLayer02.start("Layer 02", -1);
  scrollingLayer03.start("Layer 03", -1);
  scrollingLayer04.start("Layer 04", -1);
  scrollingLayer05.start("Layer 05", -1);
  scrollingLayer06.start("Layer 06", -1);
  scrollingLayer07.start("Layer 07", -1);
  scrollingLayer08.start("Layer 08", -1);
  scrollingLayer09.start("Layer 09", -1);

  chooseRandomVisibleOffset(&scrollingLayer00);
  chooseRandomVisibleOffset(&scrollingLayer01);
  chooseRandomVisibleOffset(&scrollingLayer02);
  chooseRandomVisibleOffset(&scrollingLayer03);
  chooseRandomVisibleOffset(&scrollingLayer04);
  chooseRandomVisibleOffset(&scrollingLayer05);
  chooseRandomVisibleOffset(&scrollingLayer06);
  chooseRandomVisibleOffset(&scrollingLayer07);
  chooseRandomVisibleOffset(&scrollingLayer08);
  chooseRandomVisibleOffset(&scrollingLayer09);
}

void loop() {
  Serial.println(matrix.getRefreshRate());
  delay(1000);
}
