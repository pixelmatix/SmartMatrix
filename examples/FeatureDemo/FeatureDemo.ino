/*
    SmartMatrix Features Demo - Louis Beaudoin (Pixelmatix)
    This example code is released into the public domain
*/

#include <SmartMatrix3.h>
#include "colorwheel.c"
#include "gimpbitmap.h"

const uint8_t kMatrixHeight = 32;       // known working: 16, 32
const uint8_t kMatrixWidth = 32;        // known working: 32, 64
const uint8_t kColorDepthRgb = 36;      // known working: 36, 48 (24 isn't efficient and has color correction issues)
const uint8_t kDmaBufferRows = 4;       // known working: 4
SMARTMATRIX_ALLOCATE_BUFFERS(kMatrixWidth, kMatrixHeight, kColorDepthRgb, kDmaBufferRows);

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

      matrix.drawPixel(x + j, y + i, pixel);
    }
  }
}

// the setup() method runs once, when the sketch starts
void setup() {
    // initialize the digital pin as an output.
    pinMode(ledPin, OUTPUT);

    Serial.begin(38400);

    SMARTMATRIX_SETUP_DEFAULT_LAYERS(kMatrixWidth, kMatrixHeight);

    matrix.setBrightness(defaultBrightness);

    matrix.setScrollOffsetFromTop(defaultScrollOffset);

    matrix.setColorCorrection(cc24);
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
#define DEMO_FOREGROUND_DRAWING 1
#define DEMO_BACKGND_BRIGHTNESS 1

// the loop() method runs over and over again,
// as long as the board has power
void loop() {
    int i, j;
    unsigned long currentMillis;

    // clear screen
    matrix.fillScreen(defaultBackgroundColor);
    matrix.swapBuffers();

#if (DEMO_INTRO == 1)
    // "SmartMatrix Demo"
    matrix.setScrollColor({0xff, 0xff, 0xff});
    matrix.setScrollMode(wrapForward);
    matrix.setScrollSpeed(40);
    matrix.setScrollFont(font6x10);
    matrix.scrollText("SmartMatrix Demo", 1);

    delay(5000);
#endif

#if (DEMO_DRAWING_INTRO == 1)
    {
        // "Drawing Functions"
        matrix.setScrollColor({0xff, 0xff, 0xff});
        matrix.setScrollMode(wrapForward);
        matrix.setScrollSpeed(40);
        matrix.setScrollFont(font6x10);
        matrix.scrollText("Drawing Functions", 1);

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
                matrix.drawPixel(x0, y0, outlineColor);
                break;

            case 1:
                matrix.drawLine(x0, y0, x1, y1, outlineColor);
                break;

            case 2:
                matrix.drawCircle(x0, y0, radius, outlineColor);
                break;

            case 3:
                matrix.drawTriangle(x0, y0, x1, y1, x2, y2, outlineColor);
                break;

            case 4:
                matrix.drawRectangle(x0, y0, x1, y1, outlineColor);
                break;

            case 5:
                matrix.drawRoundRectangle(x0, y0, x1, y1, radius, outlineColor);
                break;

            case 6:
                matrix.fillCircle(x0, y0, radius, fillColor);
                break;

            case 7:
                matrix.fillTriangle(x0, y0, x1, y1, x2, y2, fillColor);
                break;

            case 8:
                matrix.fillRectangle(x0, y0, x1, y1, fillColor);
                break;

            case 9:
                matrix.fillRoundRectangle(x0, y0, x1, y1, radius, fillColor);
                break;

            case 10:
                matrix.fillCircle(x0, y0, radius, outlineColor, fillColor);
                break;

            case 11:
                matrix.fillTriangle(x0, y0, x1, y1, x2, y2, outlineColor, fillColor);
                break;

            case 12:
                matrix.fillRectangle(x0, y0, x1, y1, outlineColor, fillColor);
                break;

            case 13:
                matrix.fillRoundRectangle(x0, y0, x1, y1, radius, outlineColor, fillColor);
                break;

            case 14:
                matrix.drawEllipse(x0, y0, radius, radius2, outlineColor);

            default:
                break;
            }
            matrix.swapBuffers();
            //matrix.fillScreen({0,0,0});
            while (millis() < currentMillis + delayBetweenShapes);
        }
    }
#endif

    // "Drawing Pixels"
#if (DEMO_DRAWING_PIXELS == 1)
    {
        // "Drawing Functions"
        matrix.setScrollColor({0xff, 0xff, 0xff});
        matrix.setScrollMode(wrapForward);
        matrix.setScrollSpeed(40);
        matrix.setScrollFont(font6x10);
        matrix.scrollText("Pixels", 1);

        const uint transitionTime = 3000;

        matrix.fillScreen({0, 0, 0});
        matrix.swapBuffers();

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

                matrix.drawPixel(x0, y0, color);
            }
            matrix.swapBuffers();
        }
    }
#endif
    // "Drawing Lines"
#if (DEMO_DRAWING_LINES == 1)
    {
        matrix.setScrollColor({0xff, 0xff, 0xff});
        matrix.setScrollMode(wrapForward);
        matrix.setScrollSpeed(40);
        matrix.setScrollFont(font6x10);
        matrix.scrollText("Horizontal and Vertical Lines", 1);

        int transitionTime = 6000;

        //matrix.fillScreen({0, 0, 0});
        //matrix.swapBuffers();

        currentMillis = millis();
        unsigned long delayCounter = currentMillis;

        for (i = 0; i < matrix.getScreenWidth(); i++) {
            rgb24 color;
            float fraction = ((float)millis() - currentMillis) / ((float)transitionTime / 2);

            color.red = 255 - 255.0 * fraction;
            color.green = 255.0 * fraction;
            color.blue = 0;

            matrix.drawFastVLine(i, 0, matrix.getScreenHeight(), color);
            matrix.swapBuffers();
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

            matrix.drawFastHLine(0, matrix.getScreenWidth(), i, color);
            matrix.swapBuffers();
            delayCounter += transitionTime / 2 / matrix.getScreenHeight();
            while (millis() < delayCounter);
        }

        matrix.scrollText("Diagonal Lines", 1);

        transitionTime = 4000;
        currentMillis = millis();
        delayCounter = currentMillis;

        for (i = 0; i < matrix.getScreenWidth() * 2; i++) {
            rgb24 color;
            float fraction = ((float)millis() - currentMillis) / ((float)transitionTime / 2);

            color.red = 255 - 255.0 * fraction;
            color.green = 255.0 * fraction;
            color.blue = 0;

            matrix.drawLine(i, 0, 0, i, color);
            matrix.swapBuffers();
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

            matrix.drawLine(0, matrix.getScreenHeight() - i, matrix.getScreenWidth(), i, color);
            matrix.swapBuffers();
            delayCounter += transitionTime / 2 / (matrix.getScreenWidth() * 2);
            while (millis() < delayCounter);
        }
    }
#endif
    // "Drawing Triangles"
#if (DEMO_DRAWING_TRIANGLES == 1)
    {
        matrix.setScrollColor({0xff, 0xff, 0xff});
        matrix.setScrollMode(wrapForward);
        matrix.setScrollSpeed(40);
        matrix.setScrollFont(font6x10);
        matrix.scrollText("Triangles", 1);

        int transitionTime = 3000;

        //matrix.fillScreen({0, 0, 0});
        //matrix.swapBuffers();

        currentMillis = millis();
        unsigned long delayCounter = currentMillis;

        for (i = 0; i < matrix.getScreenWidth(); i++) {
            rgb24 color;
            float fraction = ((float)millis() - currentMillis) / ((float)transitionTime / 2);

            color.red = 255 - 255.0 * fraction;
            color.green = 255.0 * fraction;
            color.blue = 0;

            matrix.drawTriangle(i, 0, matrix.getScreenWidth(), i, matrix.getScreenWidth() - i, matrix.getScreenHeight(), color);
            matrix.swapBuffers();
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

            matrix.drawTriangle(matrix.getScreenWidth() - i, matrix.getScreenHeight(), 0, matrix.getScreenHeight() - i, matrix.getScreenWidth(), i, color);
            matrix.swapBuffers();
            delayCounter += transitionTime / 2 / matrix.getScreenHeight();
            while (millis() < delayCounter);
        }
    }
#endif
    // "Drawing Circles"
#if (DEMO_DRAWING_CIRCLES == 1)
    {
        matrix.setScrollColor({0xff, 0xff, 0xff});
        matrix.setScrollMode(wrapForward);
        matrix.setScrollSpeed(40);
        matrix.setScrollFont(font6x10);
        matrix.scrollText("Circles", 1);

        int transitionTime = 6000;

        //matrix.fillScreen({0, 0, 0});
        //matrix.swapBuffers();

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

            matrix.drawCircle(i, matrix.getScreenHeight() / 2, radius, color);
            matrix.swapBuffers();
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

            matrix.drawCircle(matrix.getScreenWidth() / 2, matrix.getScreenHeight() / 2, i, color);
            matrix.swapBuffers();
            delayCounter += transitionTime / 2 / (matrix.getScreenWidth() * 2 / 3);
            while (millis() < delayCounter);
        }
    }
#endif

    // "Drawing Rectangles"
#if (DEMO_DRAWING_RECTANGLES == 1)
    {
        matrix.setScrollColor({0xff, 0xff, 0xff});
        matrix.setScrollMode(wrapForward);
        matrix.setScrollSpeed(40);
        matrix.setScrollFont(font6x10);
        matrix.scrollText("Rectangles", 1);

        int transitionTime = 3000;

        //matrix.fillScreen({0, 0, 0});
        //matrix.swapBuffers();

        currentMillis = millis();
        unsigned long delayCounter = currentMillis;

        for (i = 0; i < matrix.getScreenWidth() / 2; i++) {
            rgb24 color;
            float fraction = ((float)millis() - currentMillis) / ((float)transitionTime / 2);

            color.red = 255 - 255.0 * fraction;
            color.green = 255.0 * fraction;
            color.blue = 0;

            matrix.drawRectangle(matrix.getScreenWidth() / 2 - i - 1, matrix.getScreenHeight() / 2 - i - 1, matrix.getScreenWidth() / 2 + i, matrix.getScreenHeight() / 2 + i, color);
            matrix.swapBuffers();
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

            matrix.drawRectangle(i / 4, i / 2, i, i, color);
            matrix.swapBuffers();
            delayCounter += transitionTime / 2 / matrix.getScreenHeight();
            while (millis() < delayCounter);
        }
    }
#endif
    // "Drawing Round Rectangles"
#if (DEMO_DRAWING_ROUNDRECT == 1)
    {
        matrix.setScrollColor({0xff, 0xff, 0xff});
        matrix.setScrollMode(wrapForward);
        matrix.setScrollSpeed(40);
        matrix.setScrollFont(font6x10);
        matrix.scrollText("Round Rectangles", 1);

        int transitionTime = 4000;

        //matrix.fillScreen({0, 0, 0});
        //matrix.swapBuffers();

        currentMillis = millis();
        unsigned long delayCounter = currentMillis;

        for (i = 0; i < matrix.getScreenWidth() / 2; i++) {
            rgb24 color;
            float fraction = ((float)millis() - currentMillis) / ((float)transitionTime / 2);

            color.red = 255 - 255.0 * fraction;
            color.green = 255.0 * fraction;
            color.blue = 0;

            matrix.drawRoundRectangle(matrix.getScreenWidth() / 2 - i - 1, matrix.getScreenHeight() / 2 - i - 1, matrix.getScreenWidth() / 2 + i, matrix.getScreenHeight() / 2 + i, i / 2, color);
            matrix.swapBuffers();
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

            matrix.drawRoundRectangle(i / 4, i / 2, i, i, i / 2, color);
            matrix.swapBuffers();
            delayCounter += transitionTime / 2 / matrix.getScreenHeight();
            while (millis() < delayCounter);
        }
    }
#endif
    // Filled shapes
    // Filled shapes with outline
#if (DEMO_DRAWING_FILLED == 1)
    {
        matrix.setScrollColor({0xff, 0xff, 0xff});
        matrix.setScrollMode(wrapForward);
        matrix.setScrollSpeed(40);
        matrix.setScrollFont(font6x10);
        matrix.scrollText("Draw Outline, Filled, or Filled with Outline", 1);

        uint transitionTime = 8500;
        int delayBetweenShapes = 200;

        //matrix.fillScreen({0, 0, 0});
        //matrix.swapBuffers();

        currentMillis = millis();
        unsigned long delayCounter = currentMillis;

        while (millis() - currentMillis < transitionTime) {
            rgb24 color;
            float fraction = ((float)millis() - currentMillis) / ((float)transitionTime);

            color.red = random(256);
            color.green = random(256);
            color.blue = 0;

            if (fraction < 1.0 / 3) {
                matrix.drawRectangle(random(matrix.getScreenWidth() / 4), random(matrix.getScreenHeight() / 4),
                                     random(matrix.getScreenWidth() / 4) + matrix.getScreenWidth() / 4, random(matrix.getScreenHeight() / 4) + matrix.getScreenHeight() / 4,
                                     color);
                matrix.drawRoundRectangle(random(matrix.getScreenWidth() / 4) + matrix.getScreenWidth() / 2, random(matrix.getScreenHeight() / 4),
                                          random(matrix.getScreenWidth() / 4) + matrix.getScreenWidth() * 3 / 4, random(matrix.getScreenHeight() / 4) + matrix.getScreenHeight() / 4,
                                          random(matrix.getScreenWidth() / 4), color);
                matrix.drawCircle(random(matrix.getScreenWidth() / 4) + matrix.getScreenWidth() / 8, random(matrix.getScreenHeight() / 4) + matrix.getScreenHeight() * 5 / 8,
                                  random(matrix.getScreenHeight() / 8), color);
                matrix.drawTriangle(random(matrix.getScreenWidth() / 2) + matrix.getScreenWidth() / 2, random(matrix.getScreenHeight() / 2) + matrix.getScreenHeight() / 2,
                                    random(matrix.getScreenWidth() / 2) + matrix.getScreenWidth() / 2, random(matrix.getScreenHeight() / 2) + matrix.getScreenHeight() / 2,
                                    random(matrix.getScreenWidth() / 2) + matrix.getScreenWidth() / 2, random(matrix.getScreenHeight() / 2) + matrix.getScreenHeight() / 2,
                                    color);
            } else if (fraction < 2.0 / 3) {
                matrix.fillRectangle(random(matrix.getScreenWidth() / 4), random(matrix.getScreenHeight() / 4),
                                     random(matrix.getScreenWidth() / 4) + matrix.getScreenWidth() / 4, random(matrix.getScreenHeight() / 4) + matrix.getScreenHeight() / 4,
                                     color);
                matrix.fillRoundRectangle(random(matrix.getScreenWidth() / 4) + matrix.getScreenWidth() / 2, random(matrix.getScreenHeight() / 4),
                                          random(matrix.getScreenWidth() / 4) + matrix.getScreenWidth() * 3 / 4, random(matrix.getScreenHeight() / 4) + matrix.getScreenHeight() / 4,
                                          random(matrix.getScreenWidth() / 4), color);
                matrix.fillCircle(random(matrix.getScreenWidth() / 4) + matrix.getScreenWidth() / 8, random(matrix.getScreenHeight() / 4) + matrix.getScreenHeight() * 5 / 8,
                                  random(matrix.getScreenHeight() / 8), color);
                matrix.fillTriangle(random(matrix.getScreenWidth() / 2) + matrix.getScreenWidth() / 2, random(matrix.getScreenHeight() / 2) + matrix.getScreenHeight() / 2,
                                    random(matrix.getScreenWidth() / 2) + matrix.getScreenWidth() / 2, random(matrix.getScreenHeight() / 2) + matrix.getScreenHeight() / 2,
                                    random(matrix.getScreenWidth() / 2) + matrix.getScreenWidth() / 2, random(matrix.getScreenHeight() / 2) + matrix.getScreenHeight() / 2,
                                    color);
            } else {
                matrix.fillRectangle(random(matrix.getScreenWidth() / 4), random(matrix.getScreenHeight() / 4),
                                     random(matrix.getScreenWidth() / 4) + matrix.getScreenWidth() / 4, random(matrix.getScreenHeight() / 4) + matrix.getScreenHeight() / 4,
                {0, 0, 0xff}, color);
                matrix.fillRoundRectangle(random(matrix.getScreenWidth() / 4) + matrix.getScreenWidth() / 2, random(matrix.getScreenHeight() / 4),
                                          random(matrix.getScreenWidth() / 4) + matrix.getScreenWidth() * 3 / 4, random(matrix.getScreenHeight() / 4) + matrix.getScreenHeight() / 4,
                                          random(matrix.getScreenWidth() / 4), {0, 0, 0xff}, color);
                matrix.fillCircle(random(matrix.getScreenWidth() / 4) + matrix.getScreenWidth() / 8, random(matrix.getScreenHeight() / 4) + matrix.getScreenHeight() * 5 / 8,
                                  random(matrix.getScreenHeight() / 8), {0, 0, 0xff}, color);
                matrix.fillTriangle(random(matrix.getScreenWidth() / 2) + matrix.getScreenWidth() / 2, random(matrix.getScreenHeight() / 2) + matrix.getScreenHeight() / 2,
                                    random(matrix.getScreenWidth() / 2) + matrix.getScreenWidth() / 2, random(matrix.getScreenHeight() / 2) + matrix.getScreenHeight() / 2,
                                    random(matrix.getScreenWidth() / 2) + matrix.getScreenWidth() / 2, random(matrix.getScreenHeight() / 2) + matrix.getScreenHeight() / 2,
                {0, 0, 0xff}, color);
            }

            matrix.swapBuffers();
            delayCounter += delayBetweenShapes;
            while (millis() < delayCounter);
        }
    }
#endif

    // fill screen
#if (DEMO_FILL_SCREEN == 1)
    {
        // "Drawing Functions"
        matrix.setScrollColor({0xff, 0xff, 0xff});
        matrix.setScrollMode(wrapForward);
        matrix.setScrollSpeed(40);
        matrix.setScrollFont(font6x10);
        matrix.scrollText("Fill Screen", 1);

        const uint transitionTime = 3000;

        matrix.fillScreen({0, 0, 0});
        matrix.swapBuffers();

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
                matrix.fillScreen(color);
            }
            matrix.swapBuffers();
        }
    }
#endif
    // draw characters, then string
#if (DEMO_DRAW_CHARACTERS == 1)
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


        currentMillis = millis();

        matrix.fillScreen({0, 0x80, 0x80});
        matrix.swapBuffers();


        matrix.setFont(font5x7);
        matrix.drawChar(leftEdgeOffset + 0 * spaceBetweenCharacters, matrix.getScreenHeight() / 2, {0xff, 0, 0}, 'H');
        delay(delayBetweenCharacters);
        matrix.swapBuffers();
        matrix.drawChar(leftEdgeOffset + 1 * spaceBetweenCharacters, matrix.getScreenHeight() / 2, {0xff, 0, 0}, 'E');
        delay(delayBetweenCharacters);
        matrix.swapBuffers();
        matrix.drawChar(leftEdgeOffset + 2 * spaceBetweenCharacters, matrix.getScreenHeight() / 2, {0xff, 0, 0}, 'L');
        delay(delayBetweenCharacters);
        matrix.swapBuffers();
        matrix.drawChar(leftEdgeOffset + 3 * spaceBetweenCharacters, matrix.getScreenHeight() / 2, {0xff, 0, 0}, 'L');
        delay(delayBetweenCharacters);
        matrix.swapBuffers();
        matrix.drawChar(leftEdgeOffset + 4 * spaceBetweenCharacters, matrix.getScreenHeight() / 2, {0xff, 0, 0}, 'O');
        delay(delayBetweenCharacters);
        matrix.swapBuffers();

        delay(delayBetweenCharacters);

        matrix.fillScreen({0, 0x80, 0x80});
        matrix.swapBuffers();

        matrix.drawString(leftEdgeOffset, matrix.getScreenHeight() / 2, {0, 0xff, 0}, "Hello!");

        delay(delayBetweenCharacters);
        matrix.swapBuffers();

        // draw string but clear the background
        matrix.drawString(leftEdgeOffset, matrix.getScreenHeight() / 2, {0, 0xff, 0}, {0,0,0}, "Hello!");

        delay(delayBetweenCharacters * 2);
        matrix.swapBuffers();

        while (millis() < currentMillis + transitionTime);
    }
#endif

    // font options
#if (DEMO_FONT_OPTIONS == 1)
    {
        // "Drawing Functions"
        matrix.setScrollColor({0xff, 0xff, 0xff});
        matrix.setScrollMode(wrapForward);
        matrix.setScrollSpeed(40);
        matrix.setScrollFont(font6x10);
        matrix.scrollText("Built In Fonts", 1);

        const uint transitionTime = 5500;
        const int delayBetweenCharacters = 1000;
        const int leftEdgeOffset = 1;

        currentMillis = millis();

        matrix.fillScreen({0, 0x80, 0x80});
        matrix.swapBuffers();


        matrix.setFont(font3x5);
        matrix.fillScreen({0, 0x80, 0x80});
        matrix.drawString(leftEdgeOffset, matrix.getScreenHeight() / 2, {0xff, 0, 0}, "3x5");
        delay(delayBetweenCharacters);
        matrix.swapBuffers();

        matrix.setFont(font5x7);
        matrix.fillScreen({0, 0x80, 0x80});
        matrix.drawString(leftEdgeOffset, matrix.getScreenHeight() / 2, {0xff, 0, 0}, "5x7");
        delay(delayBetweenCharacters);
        matrix.swapBuffers();

        matrix.setFont(font6x10);
        matrix.fillScreen({0, 0x80, 0x80});
        matrix.drawString(leftEdgeOffset, matrix.getScreenHeight() / 2, {0xff, 0, 0}, "6x10");
        delay(delayBetweenCharacters);
        matrix.swapBuffers();

        matrix.setFont(font8x13);
        matrix.fillScreen({0, 0x80, 0x80});
        matrix.drawString(leftEdgeOffset, matrix.getScreenHeight() / 2, {0xff, 0, 0}, "8x13");
        delay(delayBetweenCharacters);
        matrix.swapBuffers();

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
        matrix.setScrollColor({0xff, 0xff, 0xff});
        matrix.setScrollMode(wrapForward);
        matrix.setScrollSpeed(40);
        matrix.setScrollFont(font6x10);
        matrix.scrollText("Mono Bitmaps", 1);

        const uint transitionTime = 5500;
        currentMillis = millis();

        matrix.fillScreen({0, 0x80, 0x80});
        matrix.swapBuffers();

        while (millis() < currentMillis + transitionTime) {
            matrix.drawMonoBitmap(random(matrix.getScreenWidth() + testBitmapWidth) - testBitmapWidth,
                                  random(matrix.getScreenHeight() + testBitmapHeight) - testBitmapHeight,
                                  testBitmapWidth, testBitmapHeight, {(uint8_t)random(256), (uint8_t)random(256), 0}, testBitmap);
            matrix.swapBuffers();
            delay(100);
        }
    }
#endif

    // swapBuffers demo?

    // scroll text in any color above the drawing
#if (DEMO_SCROLL_COLOR == 1)
    {
        // "Drawing Functions"
        matrix.setScrollColor({0xff, 0xff, 0xff});
        matrix.setScrollMode(wrapForward);
        matrix.setScrollSpeed(40);
        matrix.setScrollFont(font6x10);
        matrix.scrollText("Scroll text above bitmap in any color", 1);

        matrix.fillScreen(defaultBackgroundColor);
        matrix.swapBuffers();

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

            matrix.setScrollColor(color);
        }
    }
#endif
    // different modes
#if (DEMO_SCROLL_MODES == 1)
    {
        uint transitionTime;

        matrix.fillScreen(defaultBackgroundColor);
        matrix.swapBuffers();

        matrix.setScrollColor({0xff, 0xff, 0xff});
        matrix.setFont(font3x5);
        matrix.setScrollSpeed(40);
        matrix.drawString(0, matrix.getScreenHeight() / 2, {0xff, 0xff, 0xff}, "Modes");
        matrix.swapBuffers();

        matrix.setScrollMode(wrapForward);
        matrix.scrollText("Wrap Forward", 2);
        while(matrix.getScrollStatus());

        // use this mode to start the scrolling from any position, instead of the right edge
        matrix.setScrollMode(wrapForwardFromLeft);
        matrix.setScrollStartOffsetFromLeft(matrix.getScreenWidth()/2);
        matrix.scrollText("Wrap Forward From Left", 1);
        while(matrix.getScrollStatus());

        matrix.setScrollMode(bounceForward);
        matrix.scrollText("Bounce", 2);
        while(matrix.getScrollStatus());

        matrix.setScrollMode(bounceReverse);
        matrix.scrollText("Bounce (rev)", 2);
        while(matrix.getScrollStatus());

        // this mode doesn't scroll, and the position is set through setScrollStartOffsetFromLeft()
        matrix.setScrollMode(stopped);
        matrix.scrollText("Stopped", 1);
        matrix.setScrollStartOffsetFromLeft(0);
        transitionTime = 3000;
        currentMillis = millis();
        // "stopped" will always have getScrollStatus() > 0, use time to transition)
        while(millis() - currentMillis < transitionTime);

        matrix.setScrollMode(bounceReverse);
        matrix.scrollText("Update Text", 2);

        while(matrix.getScrollStatus() > 1);
        matrix.updateScrollText("Update Text While Scrolling");

        while(matrix.getScrollStatus());

        matrix.setScrollMode(wrapForward);
        // setup text to scroll infinitely
        matrix.scrollText("Stop Scrolling", -1);
        
        transitionTime = 4500;
        currentMillis = millis();
        while(millis() - currentMillis < transitionTime);
        matrix.stopScrollText();

        currentMillis = millis();
        transitionTime = 1000;
        while(millis() - currentMillis < transitionTime);


        matrix.fillScreen(defaultBackgroundColor);
        matrix.swapBuffers();
    }
#endif

    // speeds
#if (DEMO_SCROLL_SPEED == 1)
    {
        matrix.setScrollColor({0xff, 0xff, 0xff});
        matrix.setScrollMode(wrapForward);
        matrix.setScrollSpeed(1);

        matrix.scrollText("Scroll Speed", 3);
        matrix.swapBuffers();

        const int maxScrollSpeed = 100;

        uint transitionTime = 7500;
        currentMillis = millis();
        while (millis() - currentMillis < transitionTime) {
            matrix.setScrollSpeed(/*maxScrollSpeed -*/ maxScrollSpeed * ((float)millis() - currentMillis) / transitionTime);
        }
    }
#endif

    // fonts
#if (DEMO_SCROLL_FONTS == 1)
    {
        matrix.setScrollColor({0xff, 0xff, 0xff});
        matrix.setScrollMode(wrapForward);
        matrix.setScrollSpeed(40);
        matrix.setScrollOffsetFromTop(1);

        matrix.setScrollFont(font3x5);
        matrix.scrollText("All Fonts", 1);
        while (matrix.getScrollStatus());

        matrix.setScrollFont(font5x7);
        matrix.scrollText("Can Be", 1);
        while (matrix.getScrollStatus());

        matrix.setScrollFont(font6x10);
        matrix.scrollText("Used For", 1);
        while (matrix.getScrollStatus());

        matrix.setScrollFont(font8x13);
        matrix.scrollText("Scrolling", 1);
        while (matrix.getScrollStatus());

        matrix.setScrollOffsetFromTop(defaultScrollOffset);
    }
#endif
    // position
#if (DEMO_SCROLL_POSITION == 1)
    {
        matrix.setScrollColor({0xff, 0xff, 0xff});
        matrix.setScrollMode(wrapForward);
        matrix.setScrollSpeed(40);

        matrix.setScrollFont(font5x7);
        matrix.scrollText("Position Scrolling Text Anywhere", 1);

        for (i = 0; i < 6; i++) {
            matrix.setScrollOffsetFromTop(i * (matrix.getScreenHeight() / 6));
            delay(1000);
        }
        matrix.setScrollOffsetFromTop(defaultScrollOffset);
    }
#endif

    // rotate screen
#if (DEMO_SCROLL_ROTATION == 1)
    {
        matrix.setFont(font3x5);
        matrix.setScrollFont(font5x7);

        matrix.setScrollColor({0xff, 0xff, 0xff});
        matrix.setScrollMode(wrapForward);
        matrix.setScrollSpeed(40);

        // rotate 90
        matrix.setRotation(rotation90);
        // delay after rotation to make sure the rotation propogates to the background layer before drawing (1/framerate is probably long enough)
        delay(40);
        matrix.scrollText("Rotation 90", 1);
        matrix.fillScreen(defaultBackgroundColor);
        matrix.drawString(1, matrix.getScreenHeight()/2, {0xff, 0xff, 0xff}, "BACKGND");
        matrix.swapBuffers();
        while(matrix.getScrollStatus());

        matrix.setRotation(rotation180);
        delay(40);
        matrix.scrollText("Rotation 180", 1);
        matrix.fillScreen(defaultBackgroundColor);
        matrix.drawString(1, matrix.getScreenHeight()/2, {0xff, 0xff, 0xff}, "BACKGND");
        matrix.swapBuffers(false);
        while(matrix.getScrollStatus());

        matrix.setRotation(rotation270);
        delay(40);
        matrix.scrollText("Rotation 270", 1);
        matrix.fillScreen(defaultBackgroundColor);
        matrix.drawString(1, matrix.getScreenHeight()/2, {0xff, 0xff, 0xff}, "BACKGND");
        matrix.swapBuffers(false);
        while(matrix.getScrollStatus());

        matrix.setRotation(rotation0);
        delay(40);
        matrix.scrollText("Rotation 0", 1);
        matrix.fillScreen(defaultBackgroundColor);
        matrix.drawString(1, matrix.getScreenHeight()/2, {0xff, 0xff, 0xff}, "BACKGND");
        matrix.swapBuffers(false);
        while(matrix.getScrollStatus());
    }
#endif

#if (DEMO_SCROLL_RESET == 1)
    {
        matrix.setFont(font3x5);
        matrix.setScrollFont(font5x7);

        matrix.setScrollColor({0xff, 0xff, 0xff});
        matrix.setScrollMode(wrapForward);
        matrix.setScrollSpeed(40);

        matrix.drawString(0, 0, {0xff, 0xff, 0xff}, "Stop");
        matrix.drawString(0, 6, {0xff, 0xff, 0xff}, "Scroll");
        matrix.swapBuffers();

        // rotate 90
        for(i=3; i>=0; i--) {
            char number = '0' + i%10;
            char numberString[] = "0...";
            numberString[0] = number;
            matrix.scrollText(numberString, 1);
            delay(500);
            matrix.stopScrollText();
            delay(500);
        }
    }
#endif

    // brightness
#if (DEMO_BRIGHTNESS == 1)
    {
        // "Drawing Functions"
        matrix.setScrollColor({0, 0, 0});
        matrix.setScrollMode(wrapForward);
        matrix.setScrollSpeed(40);
        matrix.setScrollFont(font6x10);
        matrix.scrollText("Brightness Control", 1);

        const uint transitionTime = 6000;

        matrix.fillScreen({0xff, 0xff, 0xff});
        matrix.swapBuffers();

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

            matrix.fillScreen({0xff, 0xff, 0xff});
            matrix.drawString(0, 16, {0, 0, 0}, value);
            matrix.drawString(0, 24, {0, 0, 0}, percent);
            matrix.swapBuffers();
        }

        matrix.setBrightness(defaultBrightness);
    }
#endif

#if (DEMO_RAW_BITMAP == 1)
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

            matrix.fillScreen({0,0,0});
            drawBitmap(0,0,&colorwheel);

            matrix.drawString(12, 16, {0xff, 0, 0}, value);
            matrix.drawString(12, 24, {0xff, 0, 0}, percent);
            matrix.swapBuffers();
        }

        matrix.setBrightness(defaultBrightness);
    }
#endif
    // color correction options
#if (DEMO_COLOR_CORRECTION == 1)
    {
        // "Drawing Functions"
        matrix.setScrollColor({0xff, 0, 0});
        matrix.setScrollMode(wrapForward);
        matrix.setScrollSpeed(40);
        matrix.setScrollFont(font6x10);
        matrix.scrollText("Color correction", 1);

        const uint transitionTime = 10000;

        matrix.setFont(font5x7);

        currentMillis = millis();

        for (j = 0; j < 4; j++) {
            matrix.fillScreen({0,0,0});
            drawBitmap(0,0,&colorwheel);
            if (j%2) {
                matrix.drawString(1, 16, {0xff, 0, 0}, "CC:ON");
                matrix.setColorCorrection(cc24);
            } else {
                matrix.drawString(1, 16, {0xff, 0, 0}, "CC:OFF");
                matrix.setColorCorrection(ccNone);
            }
            // use swapBuffers(false) as background bitmap is fully drawn each time, no need to copy buffer to drawing layer after swap
            matrix.swapBuffers(false);
            delay(transitionTime/4);
        }
        matrix.setColorCorrection(cc24);
    }
#endif
#if (DEMO_FOREGROUND_DRAWING == 1)
    {
        matrix.fillScreen({0,0,0});
        drawBitmap(0,0,&colorwheel);
        matrix.swapBuffers();

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


        // scrolling text interferes with the foreground drawing even when not actively scrolling
        // move the scrolling text off of the screen
        matrix.setScrollOffsetFromTop(matrix.getScreenHeight());

        matrix.setForegroundFont(font3x5);
        matrix.setScrollColor({0xff, 0xff, 0xff});
        matrix.clearForeground();

        matrix.drawForegroundChar(0, 0,'D', true);
        matrix.drawForegroundChar(4, 0,'R', true);
        matrix.drawForegroundChar(8, 0,'A', true);
        matrix.drawForegroundChar(12,0,'W', true);
        matrix.drawForegroundChar(20,0,'T', true);
        matrix.drawForegroundChar(24,0,'O', true);

        matrix.drawForegroundString(0, 6, "FOREGND",true);

        matrix.displayForegroundDrawing();

        delay(1000);

        currentMillis = millis();

        while (millis() - currentMillis < transitionTime) {
            int x0, y0;
            bool opaque;

            for (i = 0; i < 20; i++) {
                x0 = random(matrix.getScreenWidth());
                y0 = random(matrix.getScreenHeight());
                opaque = random(2);

                matrix.drawForegroundPixel(x0, y0, opaque);
            }

            x0 = random(matrix.getScreenWidth());
            y0 = random(matrix.getScreenHeight());

            matrix.drawForegroundMonoBitmap(x0, y0, testBitmapWidth, testBitmapHeight, testBitmap, true);

            matrix.displayForegroundDrawing();
            delay(100);
        }

        matrix.setBackgroundBrightness(defaultBrightness);
        matrix.clearForeground();
        matrix.setScrollOffsetFromTop(defaultScrollOffset);
        matrix.displayForegroundDrawing();
    }
#endif
#if (DEMO_BACKGND_BRIGHTNESS == 1)
{
        matrix.setScrollColor({0xff, 0xff, 0xff});
        matrix.setScrollMode(wrapForward);
        matrix.setScrollSpeed(40);
        matrix.setScrollFont(font6x10);
        matrix.scrollText("Change Background Brightness Independently", 1);

        matrix.fillScreen({0,0,0});
        drawBitmap(0,0,&colorwheel);
        matrix.swapBuffers();

        const uint transitionTime = 9000;

        // background brightness
        currentMillis = millis();

        while (millis() - currentMillis < transitionTime) {
            float fraction = ((float)millis() - currentMillis) / ((float)transitionTime / 2);

            if (fraction < 1.0)
                fraction = 1.0 - fraction;
            if (fraction > 1.0)
                fraction = fraction - 1.0;
            int brightness = fraction * 255.0;
            matrix.setBackgroundBrightness(brightness);
        }
    }
#endif
}
