/*
 * This demo shows how to display bitmaps that are exported from GIMP and
 * compiled into the sketch and stored in the Teensy's Flash memory
 * See more details here:
 * http://docs.pixelmatix.com/SmartMatrix/library.html#drawing-raw-bitmaps-with-backbuffer
 */

#include <SmartMatrix_32x32.h>

#include "gimpbitmap.h"

// pixelmatix.c is a bitmap exported from GIMP without modification
#include "pixelmatix.c"

// colorwheel.c has been modified to use the gimp32x32bitmap struct
#include "colorwheel.c"

SmartMatrix matrix;

int led = 13;

void drawBitmap(int16_t x, int16_t y, const gimp32x32bitmap* bitmap) {
  for(int i=0; i<bitmap->height; i++) {
    for(int j=0; j<bitmap->width; j++) {
      rgb24 pixel = { bitmap->pixel_data[(i*bitmap->width + j)*3 + 0],
                      bitmap->pixel_data[(i*bitmap->width + j)*3 + 1],
                      bitmap->pixel_data[(i*bitmap->width + j)*3 + 2] };

      matrix.drawPixel(x + j, y + i, pixel);
    }
  }
}

void setup() {
  matrix.begin();
  matrix.setBrightness(128);

  pinMode(led, OUTPUT);
}

void loop() {
  int i;
  rgb24 *buffer;

  // to use drawBitmap, must cast the pointer to pixelmatixlogo as (const gimp32x32bitmap*)
  drawBitmap(0,0,(const gimp32x32bitmap*)&pixelmatixlogo);
  matrix.swapBuffers(true);

  digitalWrite(led, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);
  digitalWrite(led, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);

  buffer = matrix.backBuffer();

  // can pass &colorwheel in directly as the bitmap source is already gimp32x32bitmap
  drawBitmap(0,0,&colorwheel);
  matrix.swapBuffers(true);

  digitalWrite(led, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);
  digitalWrite(led, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);
}
