/*
    SmartMatrix Features Demo - Louis Beaudoin (Pixelmatix)
    This example code is released into the public domain
*/

#include <SmartMatrix3.h>
#include "colorwheel.c"
#include "gimpbitmap.h"

#define COLOR_DEPTH 24                  // known working: 24, 48 - If the sketch uses type `rgb24` directly, COLOR_DEPTH must be 24
const uint8_t kMatrixWidth = 32;        // known working: 32, 64, 96, 128
const uint8_t kMatrixHeight = 32;       // known working: 16, 32, 48, 64
const uint8_t kRefreshDepth = 36;       // known working: 24, 36, 48
const uint8_t kDmaBufferRows = 4;       // known working: 2-4, use 2 to save memory, more to keep from dropping frames and automatically lowering refresh rate
const uint8_t kPanelType = SMARTMATRIX_HUB75_32ROW_MOD16SCAN;   // use SMARTMATRIX_HUB75_16ROW_MOD8SCAN for common 16x32 panels
const uint8_t kMatrixOptions = (SMARTMATRIX_OPTIONS_NONE);      // see http://docs.pixelmatix.com/SmartMatrix for options
const uint8_t kBackgroundLayerOptions = (SM_BACKGROUND_OPTIONS_NONE);
const uint8_t kScrollingLayerOptions = (SM_SCROLLING_OPTIONS_NONE);
const uint8_t kIndexedLayerOptions = (SM_INDEXED_OPTIONS_NONE);

SMARTMATRIX_ALLOCATE_BUFFERS(matrix, kMatrixWidth, kMatrixHeight, kRefreshDepth, kDmaBufferRows, kPanelType, kMatrixOptions);
SMARTMATRIX_ALLOCATE_BACKGROUND_LAYER(backgroundLayer, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kBackgroundLayerOptions);
SMARTMATRIX_ALLOCATE_SCROLLING_LAYER(scrollingLayer, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kScrollingLayerOptions);
SMARTMATRIX_ALLOCATE_INDEXED_LAYER(indexedLayer, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kIndexedLayerOptions);

const int defaultBrightness = 100*(255/100);    // full brightness
//const int defaultBrightness = 15*(255/100);    // dim: 15% brightness
const int defaultScrollOffset = 6;
const rgb24 defaultBackgroundColor = {0x40, 0, 0};

// Teensy 3.0 has the LED on pin 13
const int ledPin = 13;

void drawBitmap(int16_t x, int16_t y, const gimp32x32bitmap* bitmap) {
  for(unsigned int i=0; i < bitmap->height; i++) {
    for(unsigned int j=0; j < bitmap->width; j++) {
      rgb24 pixel = { bitmap->pixel_data[(i*bitmap->width + j)*3 + 0],
                      bitmap->pixel_data[(i*bitmap->width + j)*3 + 1],
                      bitmap->pixel_data[(i*bitmap->width + j)*3 + 2] };

      backgroundLayer.drawPixel(x + j, y + i, pixel);
    }
  }
}

// the setup() method runs once, when the sketch starts
void setup() {
  // initialize the digital pin as an output.
  pinMode(ledPin, OUTPUT);

  Serial.begin(38400);

  matrix.addLayer(&backgroundLayer); 
  matrix.addLayer(&scrollingLayer); 
  matrix.addLayer(&indexedLayer); 
  matrix.begin();

  matrix.setBrightness(defaultBrightness);

  scrollingLayer.setOffsetFromTop(defaultScrollOffset);

  backgroundLayer.enableColorCorrection(true);
}

#define DEMO_INTRO              1
#define DEMO_DRAWING_INTRO      1
#define DEMO_DRAWING_PIXELS     1
#define DEMO_DRAWING_LINES      1
#define DEMO_DRAWING_TRIANGLES  1
#define DEMO_DRAWING_CIRCLES    1
#define DEMO_DRAWING_RECTANGLES 1
#define DEMO_DRAWING_ROUNDRECT  1
#define DEMO_DRAWING_FILLED     1
#define DEMO_FILL_SCREEN        1
#define DEMO_DRAW_CHARACTERS    1
#define DEMO_FONT_OPTIONS       1
#define DEMO_MONO_BITMAP        1
#define DEMO_SCROLL_COLOR       1
#define DEMO_SCROLL_MODES       1
#define DEMO_SCROLL_SPEED       1
#define DEMO_SCROLL_FONTS       1
#define DEMO_SCROLL_POSITION    1
#define DEMO_SCROLL_ROTATION    1
#define DEMO_BRIGHTNESS         1
#define DEMO_RAW_BITMAP         1
#define DEMO_COLOR_CORRECTION   1
#define DEMO_BACKGND_BRIGHTNESS 1
#define DEMO_INDEXED_LAYER      1
#define DEMO_REFRESH_RATE       1
#define DEMO_READ_PIXEL         1

// the loop() method runs over and over again,
// as long as the board has power
void loop() {
    int i, j;
    unsigned long currentMillis;

    // clear screen
    backgroundLayer.fillScreen(defaultBackgroundColor);
    backgroundLayer.swapBuffers();

#if (DEMO_INTRO == 1)
    // "SmartMatrix Demo"
    scrollingLayer.setColor({0xff, 0xff, 0xff});
    scrollingLayer.setMode(wrapForward);
    scrollingLayer.setSpeed(40);
    scrollingLayer.setFont(font6x10);
    scrollingLayer.start("SmartMatrix Demo", 1);

    delay(5000);
#endif

#if (DEMO_DRAWING_INTRO == 1)
    {
        // "Drawing Functions"
        scrollingLayer.setColor({0xff, 0xff, 0xff});
        scrollingLayer.setMode(wrapForward);
        scrollingLayer.setSpeed(40);
        scrollingLayer.setFont(font6x10);
        scrollingLayer.start("Drawing Functions", 1);

        const int delayBetweenShapes = 250;

        for (i = 0; i < 5000; i += delayBetweenShapes) {
            // draw for 100ms, then update frame, repeat
            currentMillis = millis();
            int x0, y0, x1, y1, x2, y2, radius, radius2;
            // x0,y0 pair is always on the screen
            x0 = random(matrix.getScreenWidth());
            y0 = random(matrix.getScreenHeight());

#if 0
            // x1,y1 pair can be off screen;
            x1 = random(-matrix.getScreenWidth(), 2 * matrix.getScreenWidth());
            y1 = random(-matrix.getScreenHeight(), 2 * matrix.getScreenHeight());
#else
            x1 = random(matrix.getScreenWidth());
            y1 = random(matrix.getScreenHeight());
#endif
            // x2,y2 pair is on screen;
            x2 = random(matrix.getScreenWidth());
            y2 = random(matrix.getScreenHeight());

            // radius is positive, up to screen width size
            radius = random(matrix.getScreenWidth());
            radius2 = random(matrix.getScreenWidth());

            rgb24 fillColor = {(uint8_t)random(192), (uint8_t)random(192), (uint8_t)random(192)};
            rgb24 outlineColor = {(uint8_t)random(192), (uint8_t)random(192), (uint8_t)random(192)};

            switch (random(15)) {
            case 0:
                backgroundLayer.drawPixel(x0, y0, outlineColor);
                break;

            case 1:
                backgroundLayer.drawLine(x0, y0, x1, y1, outlineColor);
                break;

            case 2:
                backgroundLayer.drawCircle(x0, y0, radius, outlineColor);
                break;

            case 3:
                backgroundLayer.drawTriangle(x0, y0, x1, y1, x2, y2, outlineColor);
                break;

            case 4:
                backgroundLayer.drawRectangle(x0, y0, x1, y1, outlineColor);
                break;

            case 5:
                backgroundLayer.drawRoundRectangle(x0, y0, x1, y1, radius, outlineColor);
                break;

            case 6:
                backgroundLayer.fillCircle(x0, y0, radius, fillColor);
                break;

            case 7:
                backgroundLayer.fillTriangle(x0, y0, x1, y1, x2, y2, fillColor);
                break;

            case 8:
                backgroundLayer.fillRectangle(x0, y0, x1, y1, fillColor);
                break;

            case 9:
                backgroundLayer.fillRoundRectangle(x0, y0, x1, y1, radius, fillColor);
                break;

            case 10:
                backgroundLayer.fillCircle(x0, y0, radius, outlineColor, fillColor);
                break;

            case 11:
                backgroundLayer.fillTriangle(x0, y0, x1, y1, x2, y2, outlineColor, fillColor);
                break;

            case 12:
                backgroundLayer.fillRectangle(x0, y0, x1, y1, outlineColor, fillColor);
                break;

            case 13:
                backgroundLayer.fillRoundRectangle(x0, y0, x1, y1, radius, outlineColor, fillColor);
                break;

            case 14:
                backgroundLayer.drawEllipse(x0, y0, radius, radius2, outlineColor);

            default:
                break;
            }
            backgroundLayer.swapBuffers();
            //backgroundLayer.fillScreen({0,0,0});
            while (millis() < currentMillis + delayBetweenShapes);
        }
    }
#endif

    // "Drawing Pixels"
#if (DEMO_DRAWING_PIXELS == 1)
    {
        // "Drawing Functions"
        scrollingLayer.setColor({0xff, 0xff, 0xff});
        scrollingLayer.setMode(wrapForward);
        scrollingLayer.setSpeed(40);
        scrollingLayer.setFont(font6x10);
        scrollingLayer.start("Pixels", 1);

        const uint transitionTime = 3000;

        backgroundLayer.fillScreen({0, 0, 0});
        backgroundLayer.swapBuffers();

        currentMillis = millis();

        while (millis() - currentMillis < transitionTime) {
            int x0, y0;

            rgb24 color;
            float fraction = ((float)millis() - currentMillis) / ((float)transitionTime / 2);

            if (millis() - currentMillis < transitionTime / 2) {
                color.red = 255 - 255.0 * fraction;
                color.green = 255.0 * fraction;
                color.blue = 0;
            }
            else {
                color.red = 0;
                color.green = 255 - 255.0 / 2 * (fraction - 1.0);
                color.blue = 255.0 * (fraction - 1.0);
            }

            for (i = 0; i < 20; i++) {
                x0 = random(matrix.getScreenWidth());
                y0 = random(matrix.getScreenHeight());

                backgroundLayer.drawPixel(x0, y0, color);
            }
            backgroundLayer.swapBuffers();
        }
    }
#endif
    // "Drawing Lines"
#if (DEMO_DRAWING_LINES == 1)
    {
        scrollingLayer.setColor({0xff, 0xff, 0xff});
        scrollingLayer.setMode(wrapForward);
        scrollingLayer.setSpeed(40);
        scrollingLayer.setFont(font6x10);
        scrollingLayer.start("Horizontal and Vertical Lines", 1);

        int transitionTime = 6000;

        //backgroundLayer.fillScreen({0, 0, 0});
        //backgroundLayer.swapBuffers();

        currentMillis = millis();
        unsigned long delayCounter = currentMillis;

        for (i = 0; i < matrix.getScreenWidth(); i++) {
            rgb24 color;
            float fraction = ((float)millis() - currentMillis) / ((float)transitionTime / 2);

            color.red = 255 - 255.0 * fraction;
            color.green = 255.0 * fraction;
            color.blue = 0;

            backgroundLayer.drawFastVLine(i, 0, matrix.getScreenHeight(), color);
            backgroundLayer.swapBuffers();
            delayCounter += transitionTime / 2 / matrix.getScreenWidth();
            while (millis() < delayCounter);
        }

        for (i = 0; i < matrix.getScreenHeight(); i++) {
            rgb24 color;
            float fraction = ((float)millis() - currentMillis) / ((float)transitionTime / 2);
            fraction -= 1.0;
            if (fraction < 0) fraction = 0.0;

            color.red = 0;
            color.green = 255 - 255.0 / 2 * (fraction);
            color.blue = 255.0 * (fraction);

            backgroundLayer.drawFastHLine(0, matrix.getScreenWidth(), i, color);
            backgroundLayer.swapBuffers();
            delayCounter += transitionTime / 2 / matrix.getScreenHeight();
            while (millis() < delayCounter);
        }

        scrollingLayer.start("Diagonal Lines", 1);

        transitionTime = 4000;
        currentMillis = millis();
        delayCounter = currentMillis;

        for (i = 0; i < matrix.getScreenWidth() * 2; i++) {
            rgb24 color;
            float fraction = ((float)millis() - currentMillis) / ((float)transitionTime / 2);

            color.red = 255 - 255.0 * fraction;
            color.green = 255.0 * fraction;
            color.blue = 0;

            backgroundLayer.drawLine(i, 0, 0, i, color);
            backgroundLayer.swapBuffers();
            delayCounter += transitionTime / 2 / (matrix.getScreenWidth() * 2);
            while (millis() < delayCounter);
        }

        for (i = 0; i < matrix.getScreenWidth() * 2; i++) {
            rgb24 color;
            float fraction = ((float)millis() - currentMillis) / ((float)transitionTime / 2);
            fraction -= 1.0;
            if (fraction < 0) fraction = 0.0;

            color.red = 0;
            color.green = 255 - 255.0 / 2 * (fraction);
            color.blue = 255.0 * (fraction);

            backgroundLayer.drawLine(0, matrix.getScreenHeight() - i, matrix.getScreenWidth(), i, color);
            backgroundLayer.swapBuffers();
            delayCounter += transitionTime / 2 / (matrix.getScreenWidth() * 2);
            while (millis() < delayCounter);
        }
    }
#endif
    // "Drawing Triangles"
#if (DEMO_DRAWING_TRIANGLES == 1)
    {
        scrollingLayer.setColor({0xff, 0xff, 0xff});
        scrollingLayer.setMode(wrapForward);
        scrollingLayer.setSpeed(40);
        scrollingLayer.setFont(font6x10);
        scrollingLayer.start("Triangles", 1);

        int transitionTime = 3000;

        //backgroundLayer.fillScreen({0, 0, 0});
        //backgroundLayer.swapBuffers();

        currentMillis = millis();
        unsigned long delayCounter = currentMillis;

        for (i = 0; i < matrix.getScreenWidth(); i++) {
            rgb24 color;
            float fraction = ((float)millis() - currentMillis) / ((float)transitionTime / 2);

            color.red = 255 - 255.0 * fraction;
            color.green = 255.0 * fraction;
            color.blue = 0;

            backgroundLayer.drawTriangle(i, 0, matrix.getScreenWidth(), i, matrix.getScreenWidth() - i, matrix.getScreenHeight(), color);
            backgroundLayer.swapBuffers();
            delayCounter += transitionTime / 2 / matrix.getScreenWidth();
            while (millis() < delayCounter);
        }

        for (i = 0; i < matrix.getScreenHeight(); i++) {
            rgb24 color;
            float fraction = ((float)millis() - currentMillis) / ((float)transitionTime / 2);
            fraction -= 1.0;
            if (fraction < 0) fraction = 0.0;

            color.red = 0;
            color.green = 255 - 255.0 / 2 * (fraction);
            color.blue = 255.0 * (fraction);

            backgroundLayer.drawTriangle(matrix.getScreenWidth() - i, matrix.getScreenHeight(), 0, matrix.getScreenHeight() - i, matrix.getScreenWidth(), i, color);
            backgroundLayer.swapBuffers();
            delayCounter += transitionTime / 2 / matrix.getScreenHeight();
            while (millis() < delayCounter);
        }
    }
#endif
    // "Drawing Circles"
#if (DEMO_DRAWING_CIRCLES == 1)
    {
        scrollingLayer.setColor({0xff, 0xff, 0xff});
        scrollingLayer.setMode(wrapForward);
        scrollingLayer.setSpeed(40);
        scrollingLayer.setFont(font6x10);
        scrollingLayer.start("Circles", 1);

        int transitionTime = 6000;

        //backgroundLayer.fillScreen({0, 0, 0});
        //backgroundLayer.swapBuffers();

        currentMillis = millis();
        unsigned long delayCounter = currentMillis;

        for (i = 0; i < matrix.getScreenWidth() * 2; i++) {
            rgb24 color;
            float fraction = ((float)millis() - currentMillis) / ((float)transitionTime / 2);

            color.red = 255 - 255.0 * fraction;
            color.green = 255.0 * fraction;
            color.blue = 0;

            int radius = i / 2;
            if (radius > matrix.getScreenWidth())
                radius = matrix.getScreenWidth();

            backgroundLayer.drawCircle(i, matrix.getScreenHeight() / 2, radius, color);
            backgroundLayer.swapBuffers();
            delayCounter += transitionTime / 2 / (matrix.getScreenWidth() * 2);
            while (millis() < delayCounter);
        }

        for (i = matrix.getScreenWidth() * 2 / 3; i >= 0; i--) {
            rgb24 color;
            float fraction = ((float)millis() - currentMillis) / ((float)transitionTime / 2);
            fraction -= 1.0;
            if (fraction < 0) fraction = 0.0;

            color.red = 0;
            color.green = 255 - 255.0 / 2 * (fraction);
            color.blue = 255.0 * (fraction);

            backgroundLayer.drawCircle(matrix.getScreenWidth() / 2, matrix.getScreenHeight() / 2, i, color);
            backgroundLayer.swapBuffers();
            delayCounter += transitionTime / 2 / (matrix.getScreenWidth() * 2 / 3);
            while (millis() < delayCounter);
        }
    }
#endif

    // "Drawing Rectangles"
#if (DEMO_DRAWING_RECTANGLES == 1)
    {
        scrollingLayer.setColor({0xff, 0xff, 0xff});
        scrollingLayer.setMode(wrapForward);
        scrollingLayer.setSpeed(40);
        scrollingLayer.setFont(font6x10);
        scrollingLayer.start("Rectangles", 1);

        int transitionTime = 3000;

        //backgroundLayer.fillScreen({0, 0, 0});
        //backgroundLayer.swapBuffers();

        currentMillis = millis();
        unsigned long delayCounter = currentMillis;

        for (i = 0; i < matrix.getScreenWidth() / 2; i++) {
            rgb24 color;
            float fraction = ((float)millis() - currentMillis) / ((float)transitionTime / 2);

            color.red = 255 - 255.0 * fraction;
            color.green = 255.0 * fraction;
            color.blue = 0;

            backgroundLayer.drawRectangle(matrix.getScreenWidth() / 2 - i - 1, matrix.getScreenHeight() / 2 - i - 1, matrix.getScreenWidth() / 2 + i, matrix.getScreenHeight() / 2 + i, color);
            backgroundLayer.swapBuffers();
            delayCounter += transitionTime / 2 / (matrix.getScreenWidth() / 2);
            while (millis() < delayCounter);
        }
        for (i = 0; i < matrix.getScreenWidth(); i++) {
            rgb24 color;
            float fraction = ((float)millis() - currentMillis) / ((float)transitionTime / 2);
            fraction -= 1.0;
            if (fraction < 0) fraction = 0.0;

            color.red = 0;
            color.green = 255 - 255.0 / 2 * (fraction);
            color.blue = 255.0 * (fraction);

            backgroundLayer.drawRectangle(i / 4, i / 2, i, i, color);
            backgroundLayer.swapBuffers();
            delayCounter += transitionTime / 2 / matrix.getScreenHeight();
            while (millis() < delayCounter);
        }
    }
#endif
    // "Drawing Round Rectangles"
#if (DEMO_DRAWING_ROUNDRECT == 1)
    {
        scrollingLayer.setColor({0xff, 0xff, 0xff});
        scrollingLayer.setMode(wrapForward);
        scrollingLayer.setSpeed(40);
        scrollingLayer.setFont(font6x10);
        scrollingLayer.start("Round Rectangles", 1);

        int transitionTime = 4000;

        //backgroundLayer.fillScreen({0, 0, 0});
        //backgroundLayer.swapBuffers();

        currentMillis = millis();
        unsigned long delayCounter = currentMillis;

        for (i = 0; i < matrix.getScreenWidth() / 2; i++) {
            rgb24 color;
            float fraction = ((float)millis() - currentMillis) / ((float)transitionTime / 2);

            color.red = 255 - 255.0 * fraction;
            color.green = 255.0 * fraction;
            color.blue = 0;

            backgroundLayer.drawRoundRectangle(matrix.getScreenWidth() / 2 - i - 1, matrix.getScreenHeight() / 2 - i - 1, matrix.getScreenWidth() / 2 + i, matrix.getScreenHeight() / 2 + i, i / 2, color);
            backgroundLayer.swapBuffers();
            delayCounter += transitionTime / 2 / (matrix.getScreenWidth() / 2);
            while (millis() < delayCounter);
        }
        for (i = 0; i < matrix.getScreenWidth(); i++) {
            rgb24 color;
            float fraction = ((float)millis() - currentMillis) / ((float)transitionTime / 2);
            fraction -= 1.0;
            if (fraction < 0) fraction = 0.0;

            color.red = 0;
            color.green = 255 - 255.0 / 2 * (fraction);
            color.blue = 255.0 * (fraction);

            backgroundLayer.drawRoundRectangle(i / 4, i / 2, i, i, i / 2, color);
            backgroundLayer.swapBuffers();
            delayCounter += transitionTime / 2 / matrix.getScreenHeight();
            while (millis() < delayCounter);
        }
    }
#endif
    // Filled shapes
    // Filled shapes with outline
#if (DEMO_DRAWING_FILLED == 1)
    {
        scrollingLayer.setColor({0xff, 0xff, 0xff});
        scrollingLayer.setMode(wrapForward);
        scrollingLayer.setSpeed(40);
        scrollingLayer.setFont(font6x10);
        scrollingLayer.start("Draw Outline, Filled, or Filled with Outline", 1);

        uint transitionTime = 8500;
        int delayBetweenShapes = 200;

        //backgroundLayer.fillScreen({0, 0, 0});
        //backgroundLayer.swapBuffers();

        currentMillis = millis();
        unsigned long delayCounter = currentMillis;

        while (millis() - currentMillis < transitionTime) {
            rgb24 color;
            float fraction = ((float)millis() - currentMillis) / ((float)transitionTime);

            color.red = random(256);
            color.green = random(256);
            color.blue = 0;

            if (fraction < 1.0 / 3) {
                backgroundLayer.drawRectangle(random(matrix.getScreenWidth() / 4), random(matrix.getScreenHeight() / 4),
                                     random(matrix.getScreenWidth() / 4) + matrix.getScreenWidth() / 4, random(matrix.getScreenHeight() / 4) + matrix.getScreenHeight() / 4,
                                     color);
                backgroundLayer.drawRoundRectangle(random(matrix.getScreenWidth() / 4) + matrix.getScreenWidth() / 2, random(matrix.getScreenHeight() / 4),
                                          random(matrix.getScreenWidth() / 4) + matrix.getScreenWidth() * 3 / 4, random(matrix.getScreenHeight() / 4) + matrix.getScreenHeight() / 4,
                                          random(matrix.getScreenWidth() / 4), color);
                backgroundLayer.drawCircle(random(matrix.getScreenWidth() / 4) + matrix.getScreenWidth() / 8, random(matrix.getScreenHeight() / 4) + matrix.getScreenHeight() * 5 / 8,
                                  random(matrix.getScreenHeight() / 8), color);
                backgroundLayer.drawTriangle(random(matrix.getScreenWidth() / 2) + matrix.getScreenWidth() / 2, random(matrix.getScreenHeight() / 2) + matrix.getScreenHeight() / 2,
                                    random(matrix.getScreenWidth() / 2) + matrix.getScreenWidth() / 2, random(matrix.getScreenHeight() / 2) + matrix.getScreenHeight() / 2,
                                    random(matrix.getScreenWidth() / 2) + matrix.getScreenWidth() / 2, random(matrix.getScreenHeight() / 2) + matrix.getScreenHeight() / 2,
                                    color);
            } else if (fraction < 2.0 / 3) {
                backgroundLayer.fillRectangle(random(matrix.getScreenWidth() / 4), random(matrix.getScreenHeight() / 4),
                                     random(matrix.getScreenWidth() / 4) + matrix.getScreenWidth() / 4, random(matrix.getScreenHeight() / 4) + matrix.getScreenHeight() / 4,
                                     color);
                backgroundLayer.fillRoundRectangle(random(matrix.getScreenWidth() / 4) + matrix.getScreenWidth() / 2, random(matrix.getScreenHeight() / 4),
                                          random(matrix.getScreenWidth() / 4) + matrix.getScreenWidth() * 3 / 4, random(matrix.getScreenHeight() / 4) + matrix.getScreenHeight() / 4,
                                          random(matrix.getScreenWidth() / 4), color);
                backgroundLayer.fillCircle(random(matrix.getScreenWidth() / 4) + matrix.getScreenWidth() / 8, random(matrix.getScreenHeight() / 4) + matrix.getScreenHeight() * 5 / 8,
                                  random(matrix.getScreenHeight() / 8), color);
                backgroundLayer.fillTriangle(random(matrix.getScreenWidth() / 2) + matrix.getScreenWidth() / 2, random(matrix.getScreenHeight() / 2) + matrix.getScreenHeight() / 2,
                                    random(matrix.getScreenWidth() / 2) + matrix.getScreenWidth() / 2, random(matrix.getScreenHeight() / 2) + matrix.getScreenHeight() / 2,
                                    random(matrix.getScreenWidth() / 2) + matrix.getScreenWidth() / 2, random(matrix.getScreenHeight() / 2) + matrix.getScreenHeight() / 2,
                                    color);
            } else {
                backgroundLayer.fillRectangle(random(matrix.getScreenWidth() / 4), random(matrix.getScreenHeight() / 4),
                                     random(matrix.getScreenWidth() / 4) + matrix.getScreenWidth() / 4, random(matrix.getScreenHeight() / 4) + matrix.getScreenHeight() / 4,
                {0, 0, 0xff}, color);
                backgroundLayer.fillRoundRectangle(random(matrix.getScreenWidth() / 4) + matrix.getScreenWidth() / 2, random(matrix.getScreenHeight() / 4),
                                          random(matrix.getScreenWidth() / 4) + matrix.getScreenWidth() * 3 / 4, random(matrix.getScreenHeight() / 4) + matrix.getScreenHeight() / 4,
                                          random(matrix.getScreenWidth() / 4), {0, 0, 0xff}, color);
                backgroundLayer.fillCircle(random(matrix.getScreenWidth() / 4) + matrix.getScreenWidth() / 8, random(matrix.getScreenHeight() / 4) + matrix.getScreenHeight() * 5 / 8,
                                  random(matrix.getScreenHeight() / 8), {0, 0, 0xff}, color);
                backgroundLayer.fillTriangle(random(matrix.getScreenWidth() / 2) + matrix.getScreenWidth() / 2, random(matrix.getScreenHeight() / 2) + matrix.getScreenHeight() / 2,
                                    random(matrix.getScreenWidth() / 2) + matrix.getScreenWidth() / 2, random(matrix.getScreenHeight() / 2) + matrix.getScreenHeight() / 2,
                                    random(matrix.getScreenWidth() / 2) + matrix.getScreenWidth() / 2, random(matrix.getScreenHeight() / 2) + matrix.getScreenHeight() / 2,
                {0, 0, 0xff}, color);
            }

            backgroundLayer.swapBuffers();
            delayCounter += delayBetweenShapes;
            while (millis() < delayCounter);
        }
    }
#endif

    // fill screen
#if (DEMO_FILL_SCREEN == 1)
    {
        // "Drawing Functions"
        scrollingLayer.setColor({0xff, 0xff, 0xff});
        scrollingLayer.setMode(wrapForward);
        scrollingLayer.setSpeed(40);
        scrollingLayer.setFont(font6x10);
        scrollingLayer.start("Fill Screen", 1);

        const uint transitionTime = 3000;

        backgroundLayer.fillScreen({0, 0, 0});
        backgroundLayer.swapBuffers();

        currentMillis = millis();

        while (millis() - currentMillis < transitionTime) {
            rgb24 color;
            float fraction = ((float)millis() - currentMillis) / ((float)transitionTime / 2);

            if (millis() - currentMillis < transitionTime / 2) {
                color.red = 255 - 255.0 * fraction;
                color.green = 255.0 * fraction;
                color.blue = 0;
            }
            else {
                color.red = 0;
                color.green = 255 - 255.0 / 2 * (fraction - 1.0);
                color.blue = 255.0 * (fraction - 1.0);
            }

            for (i = 0; i < 20; i++) {
                backgroundLayer.fillScreen(color);
            }
            backgroundLayer.swapBuffers();
        }
    }
#endif
    // draw characters, then string
#if (DEMO_DRAW_CHARACTERS == 1)
    {
        // "Drawing Functions"
        scrollingLayer.setColor({0xff, 0xff, 0xff});
        scrollingLayer.setMode(wrapForward);
        scrollingLayer.setSpeed(40);
        scrollingLayer.setFont(font6x10);
        scrollingLayer.start("Draw Characters or String", 1);

        const uint transitionTime = 5500;
        const int delayBetweenCharacters = 500;
        const int leftEdgeOffset = 2;
        const int spaceBetweenCharacters = 6;


        currentMillis = millis();

        backgroundLayer.fillScreen({0, 0x80, 0x80});
        backgroundLayer.swapBuffers();


        backgroundLayer.setFont(font5x7);
        backgroundLayer.drawChar(leftEdgeOffset + 0 * spaceBetweenCharacters, matrix.getScreenHeight() / 2, {0xff, 0, 0}, 'H');
        delay(delayBetweenCharacters);
        backgroundLayer.swapBuffers();
        backgroundLayer.drawChar(leftEdgeOffset + 1 * spaceBetweenCharacters, matrix.getScreenHeight() / 2, {0xff, 0, 0}, 'E');
        delay(delayBetweenCharacters);
        backgroundLayer.swapBuffers();
        backgroundLayer.drawChar(leftEdgeOffset + 2 * spaceBetweenCharacters, matrix.getScreenHeight() / 2, {0xff, 0, 0}, 'L');
        delay(delayBetweenCharacters);
        backgroundLayer.swapBuffers();
        backgroundLayer.drawChar(leftEdgeOffset + 3 * spaceBetweenCharacters, matrix.getScreenHeight() / 2, {0xff, 0, 0}, 'L');
        delay(delayBetweenCharacters);
        backgroundLayer.swapBuffers();
        backgroundLayer.drawChar(leftEdgeOffset + 4 * spaceBetweenCharacters, matrix.getScreenHeight() / 2, {0xff, 0, 0}, 'O');
        delay(delayBetweenCharacters);
        backgroundLayer.swapBuffers();

        delay(delayBetweenCharacters);

        backgroundLayer.fillScreen({0, 0x80, 0x80});
        backgroundLayer.swapBuffers();

        backgroundLayer.drawString(leftEdgeOffset, matrix.getScreenHeight() / 2, {0, 0xff, 0}, "Hello!");

        delay(delayBetweenCharacters);
        backgroundLayer.swapBuffers();

        // draw string but clear the background
        backgroundLayer.drawString(leftEdgeOffset, matrix.getScreenHeight() / 2, {0, 0xff, 0}, {0,0,0}, "Hello!");

        delay(delayBetweenCharacters * 2);
        backgroundLayer.swapBuffers();

        while (millis() < currentMillis + transitionTime);
    }
#endif

    // font options
#if (DEMO_FONT_OPTIONS == 1)
    {
        // "Drawing Functions"
        scrollingLayer.setColor({0xff, 0xff, 0xff});
        scrollingLayer.setMode(wrapForward);
        scrollingLayer.setSpeed(40);
        scrollingLayer.setFont(font6x10);
        scrollingLayer.start("Built In Fonts", 1);

        const uint transitionTime = 5500;
        const int delayBetweenCharacters = 1000;
        const int leftEdgeOffset = 1;

        currentMillis = millis();

        backgroundLayer.fillScreen({0, 0x80, 0x80});
        backgroundLayer.swapBuffers();


        backgroundLayer.setFont(font3x5);
        backgroundLayer.fillScreen({0, 0x80, 0x80});
        backgroundLayer.drawString(leftEdgeOffset, matrix.getScreenHeight() / 2, {0xff, 0, 0}, "3x5");
        delay(delayBetweenCharacters);
        backgroundLayer.swapBuffers();

        backgroundLayer.setFont(font5x7);
        backgroundLayer.fillScreen({0, 0x80, 0x80});
        backgroundLayer.drawString(leftEdgeOffset, matrix.getScreenHeight() / 2, {0xff, 0, 0}, "5x7");
        delay(delayBetweenCharacters);
        backgroundLayer.swapBuffers();

        backgroundLayer.setFont(font6x10);
        backgroundLayer.fillScreen({0, 0x80, 0x80});
        backgroundLayer.drawString(leftEdgeOffset, matrix.getScreenHeight() / 2, {0xff, 0, 0}, "6x10");
        delay(delayBetweenCharacters);
        backgroundLayer.swapBuffers();

        backgroundLayer.setFont(font8x13);
        backgroundLayer.fillScreen({0, 0x80, 0x80});
        backgroundLayer.drawString(leftEdgeOffset, matrix.getScreenHeight() / 2, {0xff, 0, 0}, "8x13");
        delay(delayBetweenCharacters);
        backgroundLayer.swapBuffers();

        while (millis() < currentMillis + transitionTime);
    }
#endif
    // draw bitmap
#if (DEMO_MONO_BITMAP == 1)
    {
        const int testBitmapWidth = 15;
        const int testBitmapHeight = 15;
        uint8_t testBitmap[] = {
            _______X, ________,
            ______XX, X_______,
            ______XX, X_______,
            ______XX, X_______,
            _____XXX, XX______,
            XXXXXXXX, XXXXXXX_,
            _XXXXXXX, XXXXXX__,
            __XXXXXX, XXXXX___,
            ___XXXXX, XXXX____,
            ____XXXX, XXX_____,
            ___XXXXX, XXXX____,
            ___XXXX_, XXXX____,
            __XXXX__, _XXXX___,
            __XX____, ___XX___,
            __X_____, ____X___,
        };

        // "Drawing Functions"
        scrollingLayer.setColor({0xff, 0xff, 0xff});
        scrollingLayer.setMode(wrapForward);
        scrollingLayer.setSpeed(40);
        scrollingLayer.setFont(font6x10);
        scrollingLayer.start("Mono Bitmaps", 1);

        const uint transitionTime = 5500;
        currentMillis = millis();

        backgroundLayer.fillScreen({0, 0x80, 0x80});
        backgroundLayer.swapBuffers();

        while (millis() < currentMillis + transitionTime) {
            backgroundLayer.drawMonoBitmap(random(matrix.getScreenWidth() + testBitmapWidth) - testBitmapWidth,
                                  random(matrix.getScreenHeight() + testBitmapHeight) - testBitmapHeight,
                                  testBitmapWidth, testBitmapHeight, {(uint8_t)random(256), (uint8_t)random(256), 0}, testBitmap);
            backgroundLayer.swapBuffers();
            delay(100);
        }
    }
#endif

    // swapBuffers demo?

    // scroll text in any color above the drawing
#if (DEMO_SCROLL_COLOR == 1)
    {
        // "Drawing Functions"
        scrollingLayer.setColor({0xff, 0xff, 0xff});
        scrollingLayer.setMode(wrapForward);
        scrollingLayer.setSpeed(40);
        scrollingLayer.setFont(font6x10);
        scrollingLayer.start("Scroll text above bitmap in any color", 1);

        backgroundLayer.fillScreen(defaultBackgroundColor);
        backgroundLayer.swapBuffers();

        const uint transitionTime = 8000;

        currentMillis = millis();

        while (millis() - currentMillis < transitionTime) {
            rgb24 color;
            float fraction = ((float)millis() - currentMillis) / ((float)transitionTime / 2);

            if (millis() - currentMillis < transitionTime / 2) {
                color.red = 255 - 255.0 * fraction;
                color.green = 255.0 * fraction;
                color.blue = 0;
            }
            else {
                color.red = 0;
                color.green = 255 - 255.0 / 2 * (fraction - 1.0);
                color.blue = 255.0 * (fraction - 1.0);
            }

            scrollingLayer.setColor(color);
        }
    }
#endif
    // different modes
#if (DEMO_SCROLL_MODES == 1)
    {
        uint transitionTime;

        backgroundLayer.fillScreen(defaultBackgroundColor);
        backgroundLayer.swapBuffers();

        scrollingLayer.setColor({0xff, 0xff, 0xff});
        backgroundLayer.setFont(font3x5);
        scrollingLayer.setSpeed(40);
        backgroundLayer.drawString(0, matrix.getScreenHeight() / 2, {0xff, 0xff, 0xff}, "Modes");
        backgroundLayer.swapBuffers();

        scrollingLayer.setMode(wrapForward);
        scrollingLayer.start("Wrap Forward", 2);
        while(scrollingLayer.getStatus());

        // use this mode to start the scrolling from any position, instead of the right edge
        scrollingLayer.setMode(wrapForwardFromLeft);
        scrollingLayer.setStartOffsetFromLeft(matrix.getScreenWidth()/2);
        scrollingLayer.start("Wrap Forward From Left", 1);
        while(scrollingLayer.getStatus());

        scrollingLayer.setMode(bounceForward);
        scrollingLayer.start("Bounce", 2);
        while(scrollingLayer.getStatus());

        scrollingLayer.setMode(bounceReverse);
        scrollingLayer.start("Bounce (rev)", 2);
        while(scrollingLayer.getStatus());

        // this mode doesn't scroll, and the position is set through setScrollStartOffsetFromLeft()
        scrollingLayer.setMode(stopped);
        scrollingLayer.start("Stopped", 1);
        scrollingLayer.setStartOffsetFromLeft(0);
        transitionTime = 3000;
        currentMillis = millis();
        // "stopped" will always have getScrollStatus() > 0, use time to transition)
        while(millis() - currentMillis < transitionTime);

        scrollingLayer.setMode(bounceReverse);
        scrollingLayer.start("Update Text", 2);

        while(scrollingLayer.getStatus() > 1);
        scrollingLayer.update("Update Text While Scrolling");

        while(scrollingLayer.getStatus());

        scrollingLayer.setMode(wrapForward);
        // setup text to scroll infinitely
        scrollingLayer.start("Stop Scrolling", -1);
        
        transitionTime = 4500;
        currentMillis = millis();
        while(millis() - currentMillis < transitionTime);
        scrollingLayer.stop();

        currentMillis = millis();
        transitionTime = 1000;
        while(millis() - currentMillis < transitionTime);


        backgroundLayer.fillScreen(defaultBackgroundColor);
        backgroundLayer.swapBuffers();
    }
#endif

    // speeds
#if (DEMO_SCROLL_SPEED == 1)
    {
        scrollingLayer.setColor({0xff, 0xff, 0xff});
        scrollingLayer.setMode(wrapForward);
        scrollingLayer.setSpeed(1);

        scrollingLayer.start("Scroll Speed", 3);
        backgroundLayer.swapBuffers();

        const int maxScrollSpeed = 100;

        uint transitionTime = 7500;
        currentMillis = millis();
        while (millis() - currentMillis < transitionTime) {
            scrollingLayer.setSpeed(/*maxScrollSpeed -*/ maxScrollSpeed * ((float)millis() - currentMillis) / transitionTime);
        }
    }
#endif

    // fonts
#if (DEMO_SCROLL_FONTS == 1)
    {
        scrollingLayer.setColor({0xff, 0xff, 0xff});
        scrollingLayer.setMode(wrapForward);
        scrollingLayer.setSpeed(40);
        scrollingLayer.setOffsetFromTop(1);

        scrollingLayer.setFont(font3x5);
        scrollingLayer.start("All Fonts", 1);
        while (scrollingLayer.getStatus());

        scrollingLayer.setFont(font5x7);
        scrollingLayer.start("Can Be", 1);
        while (scrollingLayer.getStatus());

        scrollingLayer.setFont(font6x10);
        scrollingLayer.start("Used For", 1);
        while (scrollingLayer.getStatus());

        scrollingLayer.setFont(font8x13);
        scrollingLayer.start("Scrolling", 1);
        while (scrollingLayer.getStatus());

        scrollingLayer.setOffsetFromTop(defaultScrollOffset);
    }
#endif
    // position
#if (DEMO_SCROLL_POSITION == 1)
    {
        scrollingLayer.setColor({0xff, 0xff, 0xff});
        scrollingLayer.setMode(wrapForward);
        scrollingLayer.setSpeed(40);

        scrollingLayer.setFont(font5x7);
        scrollingLayer.start("Position Scrolling Text Anywhere", 1);

        for (i = 0; i < 6; i++) {
            scrollingLayer.setOffsetFromTop(i * (matrix.getScreenHeight() / 6));
            delay(1000);
        }
        scrollingLayer.setOffsetFromTop(defaultScrollOffset);
    }
#endif

    // rotate screen
#if (DEMO_SCROLL_ROTATION == 1)
    {
        backgroundLayer.setFont(font3x5);
        scrollingLayer.setFont(font5x7);

        scrollingLayer.setColor({0xff, 0xff, 0xff});
        scrollingLayer.setMode(wrapForward);
        scrollingLayer.setSpeed(40);

        // rotate 90
        matrix.setRotation(rotation90);
        // delay after rotation to make sure the rotation propogates to the background layer before drawing (1/framerate is probably long enough)
        delay(40);
        scrollingLayer.start("Rotation 90", 1);
        backgroundLayer.fillScreen(defaultBackgroundColor);
        backgroundLayer.drawString(1, matrix.getScreenHeight()/2, {0xff, 0xff, 0xff}, "BACKGND");
        backgroundLayer.swapBuffers();
        while(scrollingLayer.getStatus());

        matrix.setRotation(rotation180);
        delay(40);
        scrollingLayer.start("Rotation 180", 1);
        backgroundLayer.fillScreen(defaultBackgroundColor);
        backgroundLayer.drawString(1, matrix.getScreenHeight()/2, {0xff, 0xff, 0xff}, "BACKGND");
        backgroundLayer.swapBuffers(false);
        while(scrollingLayer.getStatus());

        matrix.setRotation(rotation270);
        delay(40);
        scrollingLayer.start("Rotation 270", 1);
        backgroundLayer.fillScreen(defaultBackgroundColor);
        backgroundLayer.drawString(1, matrix.getScreenHeight()/2, {0xff, 0xff, 0xff}, "BACKGND");
        backgroundLayer.swapBuffers(false);
        while(scrollingLayer.getStatus());

        matrix.setRotation(rotation0);
        delay(40);
        scrollingLayer.start("Rotation 0", 1);
        backgroundLayer.fillScreen(defaultBackgroundColor);
        backgroundLayer.drawString(1, matrix.getScreenHeight()/2, {0xff, 0xff, 0xff}, "BACKGND");
        backgroundLayer.swapBuffers(false);
        while(scrollingLayer.getStatus());
    }
#endif

#if (DEMO_SCROLL_RESET == 1)
    {
        backgroundLayer.setFont(font3x5);
        scrollingLayer.setFont(font5x7);

        scrollingLayer.setColor({0xff, 0xff, 0xff});
        scrollingLayer.setMode(wrapForward);
        scrollingLayer.setSpeed(40);

        backgroundLayer.drawString(0, 0, {0xff, 0xff, 0xff}, "Stop");
        backgroundLayer.drawString(0, 6, {0xff, 0xff, 0xff}, "Scroll");
        backgroundLayer.swapBuffers();

        // rotate 90
        for(i=3; i>=0; i--) {
            char number = '0' + i%10;
            char numberString[] = "0...";
            numberString[0] = number;
            scrollingLayer.start(numberString, 1);
            delay(500);
            scrollingLayer.stop();
            delay(500);
        }
    }
#endif

    // brightness
#if (DEMO_BRIGHTNESS == 1)
    {
        // "Drawing Functions"
        scrollingLayer.setColor({0, 0, 0});
        scrollingLayer.setMode(wrapForward);
        scrollingLayer.setSpeed(40);
        scrollingLayer.setFont(font6x10);
        scrollingLayer.start("Brightness Control", 1);

        const uint transitionTime = 6000;

        backgroundLayer.fillScreen({0xff, 0xff, 0xff});
        backgroundLayer.swapBuffers();

        backgroundLayer.setFont(font5x7);

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

            backgroundLayer.fillScreen({0xff, 0xff, 0xff});
            backgroundLayer.drawString(0, 16, {0, 0, 0}, value);
            backgroundLayer.drawString(0, 24, {0, 0, 0}, percent);
            backgroundLayer.swapBuffers();
        }

        matrix.setBrightness(defaultBrightness);
    }
#endif

#if (DEMO_RAW_BITMAP == 1)
    {
        // "Drawing Functions"
        scrollingLayer.setColor({0xff, 0, 0});
        scrollingLayer.setMode(wrapForward);
        scrollingLayer.setSpeed(40);
        scrollingLayer.setFont(font6x10);
        scrollingLayer.start("24-bit Color, even with low brightness", 1);

        const uint transitionTime = 7000;

        backgroundLayer.setFont(font5x7);

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

            backgroundLayer.fillScreen({0,0,0});
            drawBitmap(0,0,&colorwheel);

            backgroundLayer.drawString(12, 16, {0xff, 0, 0}, value);
            backgroundLayer.drawString(12, 24, {0xff, 0, 0}, percent);
            backgroundLayer.swapBuffers();
        }

        matrix.setBrightness(defaultBrightness);
    }
#endif
    // color correction options
#if (DEMO_COLOR_CORRECTION == 1)
    {
        scrollingLayer.setColor({0x80, 0, 0});
        scrollingLayer.setMode(wrapForward);
        scrollingLayer.setSpeed(40);
        scrollingLayer.setFont(font6x10);
        scrollingLayer.start("Color correction", 1);

        // draw color box to indexed layer (50% green)
        indexedLayer.setIndexedColor(1, {0, 0x80, 0});
        for(i=matrix.getScreenHeight() - 5; i<matrix.getScreenHeight(); i++) {
          for(j=matrix.getScreenWidth() - 5; j<matrix.getScreenWidth(); j++) {
            indexedLayer.drawPixel(i,j,1);
          }
        }
        indexedLayer.swapBuffers();

        const uint transitionTime = 10000;

        backgroundLayer.setFont(font5x7);

        currentMillis = millis();

        for (j = 0; j < 4; j++) {
            backgroundLayer.fillScreen({0,0,0});
            drawBitmap(0,0,&colorwheel);
            if (j%2) {
                backgroundLayer.drawString(1, 16, {0xff, 0, 0}, "CC:ON");
                backgroundLayer.enableColorCorrection(true);
                scrollingLayer.enableColorCorrection(true);
                indexedLayer.enableColorCorrection(true);
            } else {
                backgroundLayer.drawString(1, 16, {0xff, 0, 0}, "CC:OFF");
                backgroundLayer.enableColorCorrection(false);
                scrollingLayer.enableColorCorrection(false);
                indexedLayer.enableColorCorrection(false);
            }
            // use swapBuffers(false) as background bitmap is fully drawn each time, no need to copy buffer to drawing layer after swap
            backgroundLayer.swapBuffers(false);
            delay(transitionTime/4);
        }
        backgroundLayer.enableColorCorrection(true);
        scrollingLayer.enableColorCorrection(true);
        indexedLayer.enableColorCorrection(true);

        indexedLayer.fillScreen(0);
        indexedLayer.swapBuffers();

    }
#endif
#if (DEMO_BACKGND_BRIGHTNESS == 1)
    {
        scrollingLayer.setColor({0xff, 0xff, 0xff});
        scrollingLayer.setMode(wrapForward);
        scrollingLayer.setSpeed(40);
        scrollingLayer.setFont(font6x10);
        scrollingLayer.start("Change Background Brightness", 1);

        backgroundLayer.fillScreen({0,0,0});
        drawBitmap(0,0,&colorwheel);
        backgroundLayer.swapBuffers();

        const uint transitionTime = 7000;

        // background brightness
        currentMillis = millis();

        while (millis() - currentMillis < transitionTime) {
            float fraction = ((float)millis() - currentMillis) / ((float)transitionTime / 2);

            if (fraction < 1.0)
                fraction = 1.0 - fraction;
            if (fraction > 1.0)
                fraction = fraction - 1.0;
            int brightness = fraction * 255.0;
            backgroundLayer.setBrightness(brightness);
        }
    }
#endif
#if (DEMO_INDEXED_LAYER == 1)
    {
        backgroundLayer.fillScreen({0,0,0});
        drawBitmap(0,0,&colorwheel);
        backgroundLayer.setBrightness(50);
        backgroundLayer.swapBuffers();

        const uint transitionTime = 3000;

        const int testBitmapWidth = 15;
        const int testBitmapHeight = 15;
        uint8_t testBitmap[] = {
            _______X, ________,
            ______XX, X_______,
            ______XX, X_______,
            ______XX, X_______,
            _____XXX, XX______,
            XXXXXXXX, XXXXXXX_,
            _XXXXXXX, XXXXXX__,
            __XXXXXX, XXXXX___,
            ___XXXXX, XXXX____,
            ____XXXX, XXX_____,
            ___XXXXX, XXXX____,
            ___XXXX_, XXXX____,
            __XXXX__, _XXXX___,
            __XX____, ___XX___,
            __X_____, ____X___,
        };


        indexedLayer.setFont(font3x5);
        indexedLayer.setIndexedColor(1, {0xff, 0xff, 0xff});
        indexedLayer.fillScreen(0);

        indexedLayer.drawChar(0, 0, 1, 'D');
        indexedLayer.drawChar(4, 0, 1, 'R');
        indexedLayer.drawChar(8, 0, 1, 'A');
        indexedLayer.drawChar(12,0, 1, 'W');
        indexedLayer.drawChar(20,0, 1, 'T');
        indexedLayer.drawChar(24,0, 1, 'O');

        indexedLayer.drawString(0, 6, 1, "FOREGND");

        indexedLayer.swapBuffers();

        delay(2000);

        currentMillis = millis();

        while (millis() - currentMillis < transitionTime) {
            int x0, y0;
            bool opaque;

            for (i = 0; i < 20; i++) {
                x0 = random(matrix.getScreenWidth());
                y0 = random(matrix.getScreenHeight());
                opaque = random(2);

                indexedLayer.drawPixel(x0, y0, opaque);
            }

            x0 = random(matrix.getScreenWidth());
            y0 = random(matrix.getScreenHeight());

            indexedLayer.drawMonoBitmap(x0, y0, testBitmapWidth, testBitmapHeight, 1, testBitmap);

            indexedLayer.swapBuffers();
            delay(100);
        }

        indexedLayer.fillScreen(0);
        scrollingLayer.setOffsetFromTop(defaultScrollOffset);
        indexedLayer.swapBuffers();
        backgroundLayer.setBrightness(255);
    }
#endif
#if (DEMO_READ_PIXEL == 1)
    {
        scrollingLayer.setColor({0xff, 0xff, 0xff});
        scrollingLayer.setMode(wrapForward);
        scrollingLayer.setSpeed(40);
        scrollingLayer.setFont(font6x10);
        scrollingLayer.setOffsetFromTop(0);
        scrollingLayer.start("Read Pixel From Background", 1);

        backgroundLayer.fillScreen({0,0,0});
        drawBitmap(0,0,&colorwheel);
        backgroundLayer.swapBuffers();
        backgroundLayer.setBrightness(50);

        const uint transitionTime = 9000;

        // background brightness
        currentMillis = millis();

        while (millis() - currentMillis < transitionTime) {
            int x0, y0;

            rgb24 color;
            x0 = random(matrix.getScreenWidth());
            y0 = random(matrix.getScreenHeight());

            color = backgroundLayer.readPixel(x0, y0);

            // quick hack to (usually) stay within color wheel
            if(color.red == 0 && color.green == 0 && color.blue == 0)
                continue;

            // draw crosshairs - sometimes they will be obscured by the scrolling text
            indexedLayer.drawPixel(x0+1, y0, true);
            indexedLayer.drawPixel(x0+2, y0, true);
            indexedLayer.drawPixel(x0+3, y0, true);
            indexedLayer.drawPixel(x0+4, y0, true);
            indexedLayer.drawPixel(x0-1, y0, true);
            indexedLayer.drawPixel(x0-2, y0, true);
            indexedLayer.drawPixel(x0-3, y0, true);
            indexedLayer.drawPixel(x0-4, y0, true);
            indexedLayer.drawPixel(x0, y0+1, true);
            indexedLayer.drawPixel(x0, y0+2, true);
            indexedLayer.drawPixel(x0, y0+3, true);
            indexedLayer.drawPixel(x0, y0+4, true);
            indexedLayer.drawPixel(x0, y0-1, true);
            indexedLayer.drawPixel(x0, y0-2, true);
            indexedLayer.drawPixel(x0, y0-3, true);
            indexedLayer.drawPixel(x0, y0-4, true);

            backgroundLayer.fillRectangle(matrix.getScreenWidth() - 5, matrix.getScreenHeight() - 5,
                matrix.getScreenWidth(), matrix.getScreenHeight(), color);
            backgroundLayer.swapBuffers();

            indexedLayer.swapBuffers();
            indexedLayer.fillScreen(0);
            delay(500);
        }

        indexedLayer.swapBuffers();
        scrollingLayer.setOffsetFromTop(defaultScrollOffset);
        backgroundLayer.setBrightness(255);        
    }
#endif
#if (DEMO_REFRESH_RATE == 1)
    {
        const int minRefreshRate = 5;
        const int maxRefreshRate = matrix.getRefreshRate();

        scrollingLayer.setColor({0xff, 0xff, 0xff});
        scrollingLayer.setMode(wrapForward);
        scrollingLayer.setSpeed(40);
        scrollingLayer.setFont(font6x10);
        scrollingLayer.setOffsetFromTop(0);
        scrollingLayer.start("Change Refresh Rate", 1);

        backgroundLayer.fillScreen({0,0,0});
        drawBitmap(0,0,&colorwheel);
        backgroundLayer.swapBuffers();
        backgroundLayer.setBrightness(50);

        const uint transitionTime = 9000;

        // background brightness
        currentMillis = millis();

        while (millis() - currentMillis < transitionTime) {
            float fraction = ((float)millis() - currentMillis) / ((float)transitionTime / 2);
            int refreshRate;

            if (fraction <= 1.0)
                refreshRate = maxRefreshRate - ((maxRefreshRate-minRefreshRate) * fraction);
            else
                refreshRate = minRefreshRate + ((maxRefreshRate-minRefreshRate) * (fraction - 1.0));
            matrix.setRefreshRate(refreshRate);
            delay(50);
            uint8_t gotRefreshRate = matrix.getRefreshRate();

            // scrolling speed is calculated based on refresh rate, update after refresh rate change
            scrollingLayer.setSpeed(40);

            char value[] = "000";
            value[0] = '0' + gotRefreshRate / 100;
            value[1] = '0' + (gotRefreshRate % 100) / 10;
            value[2] = '0' + gotRefreshRate % 10;

            indexedLayer.drawString(12, matrix.getScreenHeight()-1 -5, 1, value);
            indexedLayer.swapBuffers();
            indexedLayer.fillScreen(0);
        }

        matrix.setRefreshRate(maxRefreshRate);
        indexedLayer.swapBuffers();
        scrollingLayer.setOffsetFromTop(defaultScrollOffset);
        backgroundLayer.setBrightness(255);
    }
#endif
}
