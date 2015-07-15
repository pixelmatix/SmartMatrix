/*
 * This demo shows how to display bitmaps that are exported from GIMP and
 * compiled into the sketch and stored in the Teensy's Flash memory
 * See more details here:
 * http://docs.pixelmatix.com/SmartMatrix/library.html#drawing-raw-bitmaps
 */

#include <SmartMatrix3.h>

#include "gimpbitmap.h"

// pixelmatix.c is a bitmap exported from GIMP without modification
#include "pixelmatix.c"

// colorwheel.c has been modified to use the gimp32x32bitmap struct
#include "colorwheel.c"

// chrome16 is a 16x16 pixel bitmap, exported from GIMP without modification
#include "chrome16.c"

const uint8_t kMatrixHeight = 32;       // known working: 16, 32
const uint8_t kMatrixWidth = 32;        // known working: 32, 64
const uint8_t kColorDepthRgb = 36;      // known working: 36, 48 (24 isn't efficient and has color correction issues)
const uint8_t kDmaBufferRows = 4;       // known working: 4
SMARTMATRIX_ALLOCATE_BUFFERS(kMatrixWidth, kMatrixHeight, kColorDepthRgb, kDmaBufferRows);

int led = 13;

void drawBitmap(int16_t x, int16_t y, const gimp32x32bitmap* bitmap) {
  for(unsigned int i=0; i < bitmap->height; i++) {
    for(unsigned int j=0; j < bitmap->width; j++) {
      rgb24 pixel = { bitmap->pixel_data[(i*bitmap->width + j)*3 + 0],
                      bitmap->pixel_data[(i*bitmap->width + j)*3 + 1],
                      bitmap->pixel_data[(i*bitmap->width + j)*3 + 2] };

      matrix.drawPixel(x + j, y + i, pixel);
    }
  }
}

void setup() {
  SMARTMATRIX_SETUP_DEFAULT_LAYERS(kMatrixWidth, kMatrixHeight);

  matrix.setBrightness(128);

  pinMode(led, OUTPUT);
}

void loop() {
  matrix.fillScreen({0,0,0});
  // to use drawBitmap, must cast the pointer to pixelmatixlogo as (const gimp32x32bitmap*)
  drawBitmap(0,0,(const gimp32x32bitmap*)&pixelmatixlogo);
  matrix.swapBuffers();

  digitalWrite(led, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);
  digitalWrite(led, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);

  matrix.fillScreen({0,0,0});
  // can pass &colorwheel in directly as the bitmap source is already gimp32x32bitmap
  drawBitmap(0,0,&colorwheel);
  matrix.swapBuffers();

  digitalWrite(led, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);
  digitalWrite(led, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);


  matrix.fillScreen({0,0,0});
  // draw this smaller bitmap centered
  int x = (kMatrixWidth / 2) - (chrome16.width/2);
  int y = (kMatrixHeight / 2) - (chrome16.height/2);
  drawBitmap(x, y, (const gimp32x32bitmap*)&chrome16);
  matrix.swapBuffers();

  digitalWrite(led, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);
  digitalWrite(led, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);
}
