/*
    SmartMatrix Features Demo - Louis Beaudoin (Pixelmatix)
    This example code is released into the public domain
*/

//#include "SmartMatrix_16x32.h"
//#include "SmartMatrix_32x32.h"
#include "SmartMatrix.h"

SmartMatrix matrix;

const int defaultBrightness = 100*(255/100);    // full brightness
//const int defaultBrightness = 15*(255/100);    // dim: 15% brightness
const int defaultScrollOffset = 6;
const rgb24 defaultBackgroundColor = {0x40, 0, 0};

// Teensy 3.0 has the LED on pin 13
const int ledPin = 13;

// the setup() method runs once, when the sketch starts
void setup() {
    // initialize the digital pin as an output.
    pinMode(ledPin, OUTPUT);

    Serial.begin(38400);

    matrix.begin();
    matrix.setBrightness(defaultBrightness);

    matrix.setScrollOffsetFromEdge(defaultScrollOffset);

    matrix.setColorCorrection(cc24);
}

#define DEMO_INTRO              1
#define DEMO_RAW_BITMAP         1

  unsigned char framerate = 100;

// the loop() method runs over and over again,
// as long as the board has power
void loop() {
    int i, j;
    unsigned long currentMillis;

    // clear screen
    matrix.fillScreen(defaultBackgroundColor);
    matrix.swapBuffers(true);

    {
        // "Drawing Functions"
        matrix.setScrollColor({0xff, 0xff, 0xff});
        matrix.setScrollMode(wrapForward);
        matrix.setScrollSpeed(40);
        matrix.setScrollFont(font6x10);
        matrix.scrollText("Draw Characters or String", 1);

        const uint transitionTime = 5500;
        const int delayBetweenCharacters = 500;
        const int leftEdgeOffset = 2;
        const int spaceBetweenCharacters = 6;

        matrix.setRefreshRate(framerate);

        currentMillis = millis();

        matrix.fillScreen({0, 0x80, 0x80});
        matrix.swapBuffers(true);

        matrix.setFont(font5x7);
        matrix.drawChar(leftEdgeOffset + 0 * spaceBetweenCharacters, matrix.getScreenHeight() / 2, {0xff, 0, 0}, '0' + framerate/100);
        delay(delayBetweenCharacters);
        matrix.swapBuffers(true);
        matrix.drawChar(leftEdgeOffset + 1 * spaceBetweenCharacters, matrix.getScreenHeight() / 2, {0xff, 0, 0}, '0' + framerate%100/10);
        delay(delayBetweenCharacters);
        matrix.swapBuffers(true);
        matrix.drawChar(leftEdgeOffset + 2 * spaceBetweenCharacters, matrix.getScreenHeight() / 2, {0xff, 0, 0}, '0' + framerate%10);
        delay(delayBetweenCharacters);
        matrix.swapBuffers(true);
        matrix.drawChar(leftEdgeOffset + 3 * spaceBetweenCharacters, matrix.getScreenHeight() / 2, {0xff, 0, 0}, 'H');
        delay(delayBetweenCharacters);
        matrix.swapBuffers(true);
        matrix.drawChar(leftEdgeOffset + 4 * spaceBetweenCharacters, matrix.getScreenHeight() / 2, {0xff, 0, 0}, 'z');
        delay(delayBetweenCharacters);
        matrix.swapBuffers(true);

        delay(delayBetweenCharacters);

        while (millis() < currentMillis + transitionTime);

        framerate++;
    {
        // "Drawing Functions"
        matrix.setScrollColor({0xff, 0, 0});
        matrix.setScrollMode(wrapForward);
        matrix.setScrollSpeed(40);
        matrix.setScrollFont(font6x10);
        matrix.scrollText("24-bit Color, even with low brightness", 1);

        const uint transitionTime = 7000;

        matrix.setFont(font5x7);

        currentMillis = millis();

        while (millis() - currentMillis < transitionTime) {
            float fraction = ((float)millis() - currentMillis) / ((float)transitionTime / 2);

            if (fraction > 1.0)
                fraction = 2.0 - fraction;
            int brightness = fraction * 255.0;
            matrix.setBrightness(brightness);

            char value[] = "000";
            char percent[] = "100%";
            value[0] = '0' + brightness / 100;
            value[1] = '0' + (brightness % 100) / 10;
            value[2] = '0' + brightness % 10;
            percent[0] = '0' + (brightness * 100.0 / 255) / 100;
            percent[1] = '0' + (int)(brightness * 100.0 / 255) % 100 / 10;
            percent[2] = '0' + (int)(brightness * 100.0 / 255) % 10;

            rgb24 *buffer = matrix.backBuffer();

            extern const bitmap_font weathericon;

            for (i = 0; i < 32 * matrix.getScreenHeight(); i++) {
                buffer[i].red = weathericon.Bitmap[i * 3 + 0];
                buffer[i].green = weathericon.Bitmap[i * 3 + 1];
                buffer[i].blue = weathericon.Bitmap[i * 3 + 2];
            }

            matrix.drawString(12, 16, {0xff, 0, 0}, value);
            matrix.drawString(12, 24, {0xff, 0, 0}, percent);
            matrix.swapBuffers(true);
        }

        matrix.setBrightness(defaultBrightness);
    }
  }
}
