#include <SmartMatrix_32x32.h>
#include "pixelmatix.c"
#include "colorwheel.c"

SmartMatrix matrix;

int led = 13;

void setup() {
    matrix.begin();
    matrix.setBrightness(128);

    pinMode(led, OUTPUT);     
}

void loop() {
    int i;
    rgb24 *buffer;

    buffer = matrix.backBuffer();
    if ((pixelmatixlogo.width <= matrix.getScreenWidth()) &&
      (pixelmatixlogo.height <= matrix.getScreenHeight()))
        for (i = 0; i < matrix.getScreenWidth() * matrix.getScreenHeight(); i++) {
            buffer[i].red = pixelmatixlogo.pixel_data[i * 3 + 0];
            buffer[i].green = pixelmatixlogo.pixel_data[i * 3 + 1];
            buffer[i].blue = pixelmatixlogo.pixel_data[i * 3 + 2];
        }
    
    matrix.swapBuffers(true);

    digitalWrite(led, HIGH);   // turn the LED on (HIGH is the voltage level)
    delay(1000);
    digitalWrite(led, LOW);    // turn the LED off by making the voltage LOW
    delay(1000);

    buffer = matrix.backBuffer();

    if ((colorwheel.width <= matrix.getScreenWidth()) &&
      (colorwheel.height <= matrix.getScreenHeight()))
        for (i = 0; i < matrix.getScreenWidth() * matrix.getScreenHeight(); i++) {
            buffer[i].red = colorwheel.pixel_data[i * 3 + 0];
            buffer[i].green = colorwheel.pixel_data[i * 3 + 1];
            buffer[i].blue = colorwheel.pixel_data[i * 3 + 2];
        }

    matrix.swapBuffers(true);

    digitalWrite(led, HIGH);   // turn the LED on (HIGH is the voltage level)
    delay(1000);
    digitalWrite(led, LOW);    // turn the LED off by making the voltage LOW
    delay(1000);
}
