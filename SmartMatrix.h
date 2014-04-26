/*
 * SmartMatrix Library - Main Header File for SmartMatrix Class
 *
 * Copyright (c) 2014 Louis Beaudoin (Pixelmatix)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef SmartMatrix_h
#define SmartMatrix_h

#include "Arduino.h"
#include <stdint.h>


// include one of the MatrixHardware_*.h files here:
#include "MatrixHardware_KitV1_32x32.h"
//#include "MatrixHardware_KitV1_16x32.h"


// scroll text
const int textLayerMaxStringLength = 50;

typedef enum ScrollMode {
    wrapForward,
    bounceForward,
    bounceReverse,
    stopped,
    off
} ScrollMode;


// font
#include "MatrixFontCommon.h"
extern const bitmap_font apple3x5;
extern const bitmap_font apple5x7;
extern const bitmap_font apple6x10;
extern const bitmap_font apple8x13;

typedef enum fontChoices {
    font3x5,
    font5x7,
    font6x10,
    font8x13,
} fontChoices;


// color
typedef struct rgb24 {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} rgb24;

typedef enum colorCorrectionModes {
    ccNone,
    cc24,
    cc12
} colorCorrectionModes;

#define RGB24_ISEQUAL(a, b)     ((a.red == b.red) && (a.green == b.green) && (a.blue == b.blue))


// config
typedef enum rotationDegrees {
    rotation0,
    rotation90,
    rotation180,
    rotation270
} rotationDegrees;

typedef struct screen_config {
    rotationDegrees rotation;
    uint16_t localWidth;
    uint16_t localHeight;
} screen_config;


class SmartMatrix {
public:
    SmartMatrix();
    void begin();

    // drawing functions
    void swapBuffers(bool copy = true);
    void drawPixel(int16_t x, int16_t y, rgb24 color);
    void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, rgb24 color);
    void drawFastVLine(int16_t x, int16_t y0, int16_t y1, rgb24 color);
    void drawFastHLine(int16_t x0, int16_t x1, int16_t y, rgb24 color);
    void drawCircle(int16_t x0, int16_t y0, uint16_t radius, rgb24 color);
    void fillCircle(int16_t x0, int16_t y0, uint16_t radius, rgb24 outlineColor, rgb24 fillColor);
    void fillCircle(int16_t x0, int16_t y0, uint16_t radius, rgb24 color);
    void drawTriangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3, rgb24 color);
    void fillTriangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3, rgb24 fillColor);
    void fillTriangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3, rgb24 outlineColor, rgb24 fillColor);
    void drawRectangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, rgb24 color);
    void fillRectangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, rgb24 color);
    void fillRectangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, rgb24 outlineColor, rgb24 fillColor);
    void drawRoundRectangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t radius, rgb24 outlineColor);
    void fillRoundRectangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t radius, rgb24 fillColor);
    void fillRoundRectangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t radius, rgb24 outlineColor, rgb24 fillColor);
    void fillScreen(rgb24 color);
    void drawChar(int16_t x, int16_t y, rgb24 charColor, char character);
    void drawString(int16_t x, int16_t y, rgb24 charColor, const char text[]);
    void drawMonoBitmap(int16_t x, int16_t y, uint8_t width, uint8_t height, rgb24 bitmapColor, uint8_t *bitmap);
    rgb24 readPixel(int16_t x, int16_t y);
    rgb24 *backBuffer();

    // scroll text
    void scrollText(const char inputtext[], int numScrolls);
    void setScrollMode(ScrollMode mode);
    void setScrollSpeed(unsigned char pixels_per_second);
    void setScrollFont(fontChoices newFont);
    void setScrollColor(rgb24 newColor);
    void setScrollOffsetFromEdge(int offset);
    void stopScrollText(void);
    int getScrollStatus(void);

    // configuration
    void setRotation(rotationDegrees rotation);
    uint16_t getScreenWidth(void);
    uint16_t getScreenHeight(void);
    void setBrightness(uint8_t brightness);
    void setColorCorrection(colorCorrectionModes mode);
    void setFont(fontChoices newFont);

private:
	// enable ISR access to private member variables
	friend void dma_ch1_isr(void);
	// functions called by ISR
    static void matrixCalculations(void);

    // functions for refreshing
    static void loadMatrixBuffers(unsigned char currentRow);
    static uint8_t colorCorrection8bit(uint8_t inputcolor);
    static void getPixel(uint8_t hardwareX, uint8_t hardwareY, rgb24 *xyPixel);
    static void handleBufferSwap(void);
    static void updateForeground(void);
    static bool getForegroundPixel(uint8_t x, uint8_t y, rgb24 *xyPixel);
    static void redrawForeground(void);

    // drawing functions not meant for user
    void drawHardwareHLine(uint8_t x0, uint8_t x1, uint8_t y, rgb24 color);
    void drawHardwareVLine(uint8_t x, uint8_t y0, uint8_t y1, rgb24 color);
    void bresteepline(int16_t x3, int16_t y3, int16_t x4, int16_t y4, rgb24 color);
    void fillFlatSideTriangleInt(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3, rgb24 color);

    // configuration helper functions
    static void calculateTimerLut(void);

    // color functions (replace with class and copy constructor?)
    static void copyRgb24(rgb24 *dst, rgb24 *src);
    static void copyRgb24(rgb24 *dst, rgb24 src);

    // configuration
    static colorCorrectionModes _ccmode;
    static screen_config screenConfig;
    static volatile bool brightnessChange;
    static int dimmingFactor;
    static const int dimmingMaximum;

    // keeping track of drawing buffer
    static unsigned char currentDrawBuffer;
    static unsigned char currentRefreshBuffer;
    static volatile bool swapPending;
    static bool swapWithCopy;

    // font
    static bool getBitmapFontPixelAtXY(unsigned char letter, unsigned char x, unsigned char y, const bitmap_font *font);
    const bitmap_font *fontLookup(fontChoices font);
    static uint16_t getBitmapFontRowAtXY(unsigned char letter, unsigned char y, const bitmap_font *font);
};

#endif
