/*
 * This example shows how to display bitmaps that are exported from GIMP and
 * compiled into the sketch and stored in the Teensy's Flash memory
 * See more details here:
 * http://docs.pixelmatix.com/SmartMatrix/library.html#smartmatrix-library-how-to-use-the-library-drawing-raw-bitmaps
 *
 * This example uses only the SmartMatrix Background layer
 */

// uncomment one line to select your MatrixHardware configuration - configuration header needs to be included before <SmartMatrix.h>
//#include <MatrixHardware_Teensy3_ShieldV4.h>        // SmartLED Shield for Teensy 3 (V4)
//#include <MatrixHardware_Teensy4_ShieldV5.h>        // SmartLED Shield for Teensy 4 (V5)
//#include <MatrixHardware_Teensy3_ShieldV1toV3.h>    // SmartMatrix Shield for Teensy 3 V1-V3
//#include <MatrixHardware_Teensy4_ShieldV4Adapter.h> // Teensy 4 Adapter attached to SmartLED Shield for Teensy 3 (V4)
//#include <MatrixHardware_ESP32_V0.h>                // This file contains multiple ESP32 hardware configurations, edit the file to define GPIOPINOUT (or add #define GPIOPINOUT with a hardcoded number before this #include)
//#include "MatrixHardware_Custom.h"                  // Copy an existing MatrixHardware file to your Sketch directory, rename, customize, and you can include it like this
#include <SmartMatrix.h>

#include "gimpbitmap.h"

// pixelmatix.c is a bitmap exported from GIMP without modification
#include "pixelmatix.c"

// colorwheel.c has been modified to use the gimp32x32bitmap struct
#include "colorwheel.c"

// chrome16 is a 16x16 pixel bitmap, exported from GIMP without modification
#include "chrome16.c"

#define COLOR_DEPTH 24                  // Choose the color depth used for storing pixels in the layers: 24 or 48 (24 is good for most sketches - If the sketch uses type `rgb24` directly, COLOR_DEPTH must be 24)
const uint16_t kMatrixWidth = 32;       // Set to the width of your display, must be a multiple of 8
const uint16_t kMatrixHeight = 32;      // Set to the height of your display
const uint8_t kRefreshDepth = 36;       // Tradeoff of color quality vs refresh rate, max brightness, and RAM usage.  36 is typically good, drop down to 24 if you need to.  On Teensy, multiples of 3, up to 48: 3, 6, 9, 12, 15, 18, 21, 24, 27, 30, 33, 36, 39, 42, 45, 48.  On ESP32: 24, 36, 48
const uint8_t kDmaBufferRows = 4;       // known working: 2-4, use 2 to save RAM, more to keep from dropping frames and automatically lowering refresh rate.  (This isn't used on ESP32, leave as default)
const uint8_t kPanelType = SM_PANELTYPE_HUB75_32ROW_MOD16SCAN;   // Choose the configuration that matches your panels.  See more details in MatrixCommonHub75.h and the docs: https://github.com/pixelmatix/SmartMatrix/wiki
const uint32_t kMatrixOptions = (SM_HUB75_OPTIONS_NONE);        // see docs for options: https://github.com/pixelmatix/SmartMatrix/wiki
const uint8_t kBackgroundLayerOptions = (SM_BACKGROUND_OPTIONS_NONE);

SMARTMATRIX_ALLOCATE_BUFFERS(matrix, kMatrixWidth, kMatrixHeight, kRefreshDepth, kDmaBufferRows, kPanelType, kMatrixOptions);
SMARTMATRIX_ALLOCATE_BACKGROUND_LAYER(backgroundLayer, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kBackgroundLayerOptions);

int led = -1;   // Set to -1 to disable LED flash, only needed for debugging purposes
//int led = 13; // builtin LED pin on the Teensy, interferes with refresh on Teensy 4

void drawBitmap(int16_t x, int16_t y, const gimp32x32bitmap* bitmap) {
  for(unsigned int i=0; i < bitmap->height; i++) {
    for(unsigned int j=0; j < bitmap->width; j++) {
      SM_RGB pixel = { bitmap->pixel_data[(i*bitmap->width + j)*3 + 0],
                      bitmap->pixel_data[(i*bitmap->width + j)*3 + 1],
                      bitmap->pixel_data[(i*bitmap->width + j)*3 + 2] };
      if(COLOR_DEPTH == 48) {
          pixel.red = pixel.red << 8;
          pixel.green = pixel.green << 8;
          pixel.blue = pixel.blue << 8;
      }

      backgroundLayer.drawPixel(x + j, y + i, pixel);
    }
  }
}

void setup() {
  matrix.addLayer(&backgroundLayer); 
  matrix.begin();

  matrix.setBrightness(128);

  if(led >= 0)  pinMode(led, OUTPUT);
}

void loop() {
  int x, y;
  backgroundLayer.fillScreen({0,0,0});
  x = (kMatrixWidth / 2) - (pixelmatixlogo.width/2);
  y = (kMatrixHeight / 2) - (pixelmatixlogo.height/2);
  // to use drawBitmap, must cast the pointer to pixelmatixlogo as (const gimp32x32bitmap*)
  drawBitmap(x,y,(const gimp32x32bitmap*)&pixelmatixlogo);
  backgroundLayer.swapBuffers();

  if(led >= 0)  digitalWrite(led, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);
  if(led >= 0)  digitalWrite(led, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);

  backgroundLayer.fillScreen({0,0,0});
  x = (kMatrixWidth / 2) - (colorwheel.width/2);
  y = (kMatrixHeight / 2) - (colorwheel.height/2);
  // can pass &colorwheel in directly as the bitmap source is already gimp32x32bitmap
  drawBitmap(x,y,&colorwheel);
  backgroundLayer.swapBuffers();

  if(led >= 0)  digitalWrite(led, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);
  if(led >= 0)  digitalWrite(led, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);

  backgroundLayer.fillScreen({0,0,0});
  x = (kMatrixWidth / 2) - (chrome16.width/2);
  y = (kMatrixHeight / 2) - (chrome16.height/2);
  drawBitmap(x, y, (const gimp32x32bitmap*)&chrome16);
  backgroundLayer.swapBuffers();

  if(led >= 0)  digitalWrite(led, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);
  if(led >= 0)  digitalWrite(led, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);
}
