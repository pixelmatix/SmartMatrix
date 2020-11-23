/*
  This example shows how multiple scrolling text layers can be used
  This example requires the Adafruit_GFX Library, which you can install through Arduino Library Manager
  The "GFX" Layers in SmartMatrix Library 4.0 depend on Adafruit_GFX and give access to using large fonts included
    with Adafruit_GFX, and the ability to add custom fonts
  Performance of the "GFX" layers has been optimized, and with the Teensy 4, you can have 10 scrolling text layers
    on a 128x64 panel, and still refresh the panel at 240Hz
  Optionally draw a pattern behind the text layers using FastLED (same code as the FastLED_Functions example) by
    removing the comments from `#define INCLUDE_FASTLED_BACKGROUND` below.  With the Teensy 4 you'll still see 
    128x64 @ 240Hz Refresh even with the pattern updating at 60 FPS!
*/

#define SUPPORT_ADAFRUIT_GFX_LIBRARY

//#define INCLUDE_FASTLED_BACKGROUND

// uncomment one line to select your MatrixHardware configuration - configuration header needs to be included before <SmartMatrix3.h>
//#include <MatrixHardware_Teensy3_ShieldV4.h>        // SmartLED Shield for Teensy 3 (V4)
//#include <MatrixHardware_Teensy4_ShieldV5.h>        // SmartLED Shield for Teensy 4 (V5)
//#include <MatrixHardware_Teensy3_ShieldV1toV3.h>    // SmartMatrix Shield for Teensy 3 V1-V3
//#include <MatrixHardware_Teensy4_ShieldV4Adapter.h> // Teensy 4 Adapter attached to SmartLED Shield for Teensy 3 (V4)
//#include <MatrixHardware_ESP32_V0.h>                // This file contains multiple ESP32 hardware configurations, edit the file to define GPIOPINOUT (or add #define GPIOPINOUT with a hardcoded number before this #include)
//#include "MatrixHardware_Custom.h"                  // Copy an existing MatrixHardware file to your Sketch directory, rename, customize, and you can include it like this
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

#define COLOR_DEPTH 24                  // Choose the color depth used for storing pixels in the layers: 24 or 48 (24 is good for most sketches - If the sketch uses type `rgb24` directly, COLOR_DEPTH must be 24)
const uint16_t kMatrixWidth = 32;       // Set to the width of your display, must be a multiple of 8
const uint16_t kMatrixHeight = 32;      // Set to the height of your display
const uint8_t kRefreshDepth = 36;       // Tradeoff of color quality vs refresh rate, max brightness, and RAM usage.  36 is typically good, drop down to 24 if you need to.  On Teensy, multiples of 3, up to 48: 3, 6, 9, 12, 15, 18, 21, 24, 27, 30, 33, 36, 39, 42, 45, 48.  On ESP32: 24, 36, 48
const uint8_t kDmaBufferRows = 4;       // known working: 2-4, use 2 to save RAM, more to keep from dropping frames and automatically lowering refresh rate.  (This isn't used on ESP32, leave as default)
const uint8_t kPanelType = SM_PANELTYPE_HUB75_32ROW_MOD16SCAN;   // Choose the configuration that matches your panels.  See more details in MatrixCommonHUB75.h and the docs: https://github.com/pixelmatix/SmartMatrix/wiki
const uint32_t kMatrixOptions = (SM_HUB75_OPTIONS_NONE);        // see docs for options: https://github.com/pixelmatix/SmartMatrix/wiki
const uint8_t kScrollingLayerOptions = (SM_GFX_MONO_OPTIONS_NONE);

SMARTMATRIX_ALLOCATE_BUFFERS(matrix, kMatrixWidth, kMatrixHeight, kRefreshDepth, kDmaBufferRows, kPanelType, kMatrixOptions);

#ifdef INCLUDE_FASTLED_BACKGROUND
  const uint8_t kBackgroundLayerOptions = (SM_BACKGROUND_GFX_OPTIONS_NONE);
  SMARTMATRIX_ALLOCATE_BACKGROUND_GFX_LAYER(backgroundLayer, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kBackgroundLayerOptions);
#endif

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

#ifdef INCLUDE_FASTLED_BACKGROUND
#include <FastLED.h>

// The 32bit version of our coordinates
static uint16_t x;
static uint16_t y;
static uint16_t z;

// We're using the x/y dimensions to map to the x/y pixels on the matrix.  We'll
// use the z-axis for "time".  speed determines how fast time moves forward.  Try
// 1 for a very slow moving effect, or 60 for something that ends up looking like
// water.
// uint16_t speed = 1; // almost looks like a painting, moves very slowly
uint16_t speed = 10; // a nice starting speed, mixes well with a scale of 100
// uint16_t speed = 33;
// uint16_t speed = 100; // wicked fast!

// Scale determines how far apart the pixels in our noise matrix are.  Try
// changing these values around to see how it affects the motion of the display.  The
// higher the value of scale, the more "zoomed out" the noise iwll be.  A value
// of 1 will be so zoomed in, you'll mostly see solid colors.

// uint16_t scale = 1; // mostly just solid colors
// uint16_t scale = 4011; // very zoomed out and shimmery
uint16_t scale = 31;

// This is the array that we keep our computed noise values in
uint8_t noise[kMatrixWidth][kMatrixHeight];

// Fill the x/y array of 8-bit noise values using the inoise8 function.
void fillnoise8() {
  for(int i = 0; i < kMatrixWidth; i++) {
    int ioffset = scale * i;
    for(int j = 0; j < kMatrixHeight; j++) {
      int joffset = scale * j;
      noise[i][j] = inoise8(x + ioffset,y + joffset,z);
    }
  }
  z += speed;
}

#endif

void setup() {
  delay(2000);

  randomSeed(analogRead(0));
#ifdef INCLUDE_FASTLED_BACKGROUND
  matrix.addLayer(&backgroundLayer); 
  // Initialize our coordinates to some random values
  x = random16();
  y = random16();
  z = random16();
#endif

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
#ifdef INCLUDE_FASTLED_BACKGROUND
  
  static uint8_t circlex = 0;
  static uint8_t circley = 0;

  // if sketch uses swapBuffers(false), wait to get a new backBuffer() pointer after the swap is done:
  while(backgroundLayer.isSwapPending());

  rgb24 *buffer = backgroundLayer.backBuffer();

  static uint8_t ihue=0;
  fillnoise8();
  for(int i = 0; i < kMatrixWidth; i++) {
    for(int j = 0; j < kMatrixHeight; j++) {
      // We use the value at the (i,j) coordinate in the noise
      // array for our brightness, and the flipped value from (j,i)
      // for our pixel's hue.
      buffer[kMatrixWidth*j + i] = CRGB(CHSV(noise[j][i],255,noise[i][j]));

      // You can also explore other ways to constrain the hue used, like below
      // buffer[kMatrixHeight*j + i] = CRGB(CHSV(ihue + (noise[j][i]>>2),255,noise[i][j]));
    }
  }
  ihue+=1;

  backgroundLayer.fillCircle(circlex % kMatrixWidth, circley % kMatrixHeight, 6, (rgb24)CRGB(CHSV(ihue+128,255,255)));
  circlex += random16(2);
  circley += random16(2);

  // buffer is filled completely each time, use swapBuffers without buffer copy to save CPU cycles
  backgroundLayer.swapBuffers(false);
  matrix.countFPS();      // print the loop() frames per second to Serial
  EVERY_N_SECONDS( 1 ) {   
    Serial.print("Refresh Rate: ");
    Serial.println(matrix.getRefreshRate()); 
  }

#else  

  Serial.print("Refresh Rate: ");
  Serial.println(matrix.getRefreshRate());
  delay(1000);

#endif
}
