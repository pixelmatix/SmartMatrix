/*
  Note: This example currently has very low performance on large displays (~38FPS refresh rate on 64x64)
  The Scrolling Layer refresh code is optimized if you use the new "GFX" Adafruit_GFX compatible layers instead.

  (New in SmartMatrix Library 4.0) To update a SmartMatrix Library sketch to use Adafruit_GFX compatible layers:

  - Make sure you have the Adafruit_GFX Library installed in Arduino (you can use Arduino Library Manager)
  - add `#define USE_ADAFRUIT_GFX_LAYERS` at top of sketch (this is needed for any sketch to tell SmartMatrix Library that Adafruit_GFX is present, not just this sketch)
    - Add this *before* #include <SmartMatrix.h>
    - Check the documentation Wiki for more details on why you may or may not want to use these layers

  With the "GFX" layers you're no longer limited to the fonts built into the SmartMatrix Library.  You can use any of the
    fonts included in Adafruit_GFX, or add custom fonts
*/

//#define USE_ADAFRUIT_GFX_LAYERS

// uncomment one line to select your MatrixHardware configuration - configuration header needs to be included before <SmartMatrix.h>
//#include <MatrixHardware_Teensy3_ShieldV4.h>        // SmartLED Shield for Teensy 3 (V4)
//#include <MatrixHardware_Teensy4_ShieldV5.h>        // SmartLED Shield for Teensy 4 (V5)
//#include <MatrixHardware_Teensy3_ShieldV1toV3.h>    // SmartMatrix Shield for Teensy 3 V1-V3
//#include <MatrixHardware_Teensy4_ShieldV4Adapter.h> // Teensy 4 Adapter attached to SmartLED Shield for Teensy 3 (V4)
//#include <MatrixHardware_ESP32_V0.h>                // This file contains multiple ESP32 hardware configurations, edit the file to define GPIOPINOUT (or add #define GPIOPINOUT with a hardcoded number before this #include)
//#include "MatrixHardware_Custom.h"                  // Copy an existing MatrixHardware file to your Sketch directory, rename, customize, and you can include it like this
#include <SmartMatrix.h>

#define COLOR_DEPTH 24                  // Choose the color depth used for storing pixels in the layers: 24 or 48 (24 is good for most sketches - If the sketch uses type `rgb24` directly, COLOR_DEPTH must be 24)
const uint16_t kMatrixWidth = 32;       // Set to the width of your display, must be a multiple of 8
const uint16_t kMatrixHeight = 32;      // Set to the height of your display
const uint8_t kRefreshDepth = 36;       // Tradeoff of color quality vs refresh rate, max brightness, and RAM usage.  36 is typically good, drop down to 24 if you need to.  On Teensy, multiples of 3, up to 48: 3, 6, 9, 12, 15, 18, 21, 24, 27, 30, 33, 36, 39, 42, 45, 48.  On ESP32: 24, 36, 48
const uint8_t kDmaBufferRows = 4;       // known working: 2-4, use 2 to save RAM, more to keep from dropping frames and automatically lowering refresh rate.  (This isn't used on ESP32, leave as default)
const uint8_t kPanelType = SM_PANELTYPE_HUB75_32ROW_MOD16SCAN;   // Choose the configuration that matches your panels.  See more details in MatrixCommonHub75.h and the docs: https://github.com/pixelmatix/SmartMatrix/wiki
const uint32_t kMatrixOptions = (SM_HUB75_OPTIONS_NONE);        // see docs for options: https://github.com/pixelmatix/SmartMatrix/wiki

SMARTMATRIX_ALLOCATE_BUFFERS(matrix, kMatrixWidth, kMatrixHeight, kRefreshDepth, kDmaBufferRows, kPanelType, kMatrixOptions);

const uint8_t kScrollingLayerOptions = (SM_SCROLLING_OPTIONS_NONE);
SMARTMATRIX_ALLOCATE_SCROLLING_LAYER(scrollingLayer1, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kScrollingLayerOptions);
SMARTMATRIX_ALLOCATE_SCROLLING_LAYER(scrollingLayer2, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kScrollingLayerOptions);
SMARTMATRIX_ALLOCATE_SCROLLING_LAYER(scrollingLayer3, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kScrollingLayerOptions);
SMARTMATRIX_ALLOCATE_SCROLLING_LAYER(scrollingLayer4, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kScrollingLayerOptions);
SMARTMATRIX_ALLOCATE_SCROLLING_LAYER(scrollingLayer5, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kScrollingLayerOptions);

void setup() {
  Serial.begin(115200);

  matrix.addLayer(&scrollingLayer1); 
  matrix.addLayer(&scrollingLayer2); 
  matrix.addLayer(&scrollingLayer3); 
  matrix.addLayer(&scrollingLayer4); 
  matrix.addLayer(&scrollingLayer5); 
  matrix.begin();

  scrollingLayer1.setMode(wrapForward);
  scrollingLayer2.setMode(bounceForward);
  scrollingLayer3.setMode(bounceReverse);
  scrollingLayer4.setMode(wrapForward);
  scrollingLayer5.setMode(bounceForward);

  scrollingLayer1.setColor({0xff, 0xff, 0xff});
  scrollingLayer2.setColor({0xff, 0x00, 0xff});
  scrollingLayer3.setColor({0xff, 0xff, 0x00});
  scrollingLayer4.setColor({0x00, 0x00, 0xff});
  scrollingLayer5.setColor({0xff, 0x00, 0x00});

  scrollingLayer1.setSpeed(10);
  scrollingLayer2.setSpeed(20);
  scrollingLayer3.setSpeed(40);
  scrollingLayer4.setSpeed(80);
  scrollingLayer5.setSpeed(120);

  scrollingLayer1.setFont(gohufont11b);
  scrollingLayer2.setFont(gohufont11);
  scrollingLayer3.setFont(font8x13);
  scrollingLayer4.setFont(font6x10);
  scrollingLayer5.setFont(font5x7);

  scrollingLayer4.setRotation(rotation270);
  scrollingLayer5.setRotation(rotation90);

  scrollingLayer1.setOffsetFromTop((kMatrixHeight/2) - 5);
  scrollingLayer2.setOffsetFromTop((kMatrixHeight/4) - 5);
  scrollingLayer3.setOffsetFromTop((kMatrixHeight/2 + kMatrixHeight/4) - 5);
  scrollingLayer4.setOffsetFromTop((kMatrixWidth/2 + kMatrixWidth/4) - 5);
  scrollingLayer5.setOffsetFromTop((kMatrixWidth/2 + kMatrixWidth/4) - 5);

  scrollingLayer1.start("Layer 1", -1);
  scrollingLayer2.start("Layer 2", -1);
  scrollingLayer3.start("Layer 3", -1);
  scrollingLayer4.start("Layer 4", -1);
  scrollingLayer5.start("Layer 5", -1);
}

void loop() {
  Serial.print("Refresh Rate: ");
  Serial.println(matrix.getRefreshRate());
  delay(1000);
}
