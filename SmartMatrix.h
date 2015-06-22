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

#include "MatrixCommon.h"

#include "Layer_Foreground.h"

#define ENABLE_FADECANDY_GAMMA_CORRECTION               1

// color
#if COLOR_DEPTH_RGB > 24

typedef rgb48 refreshPixel;

#define color_chan_t uint16_t
#else
#define color_chan_t uint8_t
typedef rgb24 refreshPixel;
#endif

typedef enum colorCorrectionModes {
    ccNone,
    cc24,
    cc12,
    cc48
} colorCorrectionModes;

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

// definition telling FastLED that setBackBuffer() and getRealBackBuffer() are available
#define SMART_MATRIX_CAN_TRIPLE_BUFFER 1

// enable true triple buffering and interpolation
#define SMARTMATRIX_TRIPLEBUFFER

#ifdef SMARTMATRIX_TRIPLEBUFFER
#if COLOR_DEPTH_RGB <= 24
#error Must use 36 or 48-bit color for triplebuffering and interpolation
#endif
#endif

class SmartMatrix {
public:
    SmartMatrix(void);
    void begin(void);

    // drawing functions
    void swapBuffers(bool copy = true);
#ifdef SMARTMATRIX_TRIPLEBUFFER
    void swapBuffersWithInterpolation_frames(int framesToInterpolate, bool copy = false);
    void swapBuffersWithInterpolation_ms(int interpolationSpan_ms, bool copy = false);
#endif
    void drawPixel(int16_t x, int16_t y, const rgb24& color);
    void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, const rgb24& color);
    void drawFastVLine(int16_t x, int16_t y0, int16_t y1, const rgb24& color);
    void drawFastHLine(int16_t x0, int16_t x1, int16_t y, const rgb24& color);
    void drawCircle(int16_t x0, int16_t y0, uint16_t radius, const rgb24& color);
    void fillCircle(int16_t x0, int16_t y0, uint16_t radius, const rgb24& outlineColor, const rgb24& fillColor);
    void fillCircle(int16_t x0, int16_t y0, uint16_t radius, const rgb24& color);
    void drawEllipse(int16_t x0, int16_t y0, uint16_t radiusX, uint16_t radiusY, const rgb24& color);
    void drawTriangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3, const rgb24& color);
    void fillTriangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3, const rgb24& fillColor);
    void fillTriangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3,
      const rgb24& outlineColor, const rgb24& fillColor);
    void drawRectangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, const rgb24& color);
    void fillRectangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, const rgb24& color);
    void fillRectangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, const rgb24& outlineColor, const rgb24& fillColor);
    void drawRoundRectangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t radius, const rgb24& outlineColor);
    void fillRoundRectangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t radius, const rgb24& fillColor);
    void fillRoundRectangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t radius,
      const rgb24& outlineColor, const rgb24& fillColor);
    void fillScreen(const rgb24& color);
    void drawChar(int16_t x, int16_t y, const rgb24& charColor, char character);
    void drawString(int16_t x, int16_t y, const rgb24& charColor, const char text[]);
    void drawString(int16_t x, int16_t y, const rgb24& charColor, const rgb24& backColor, const char text[]);
    void drawMonoBitmap(int16_t x, int16_t y, uint8_t width, uint8_t height, const rgb24& bitmapColor, const uint8_t *bitmap);
    const rgb24 readPixel(int16_t x, int16_t y) const;
    rgb24 *backBuffer(void);
    void setBackBuffer(rgb24 *newBuffer);
    rgb24 *getRealBackBuffer(void);

    // scroll text
    void scrollText(const char inputtext[], int numScrolls);
    void setScrollMode(ScrollMode mode);
    void setScrollSpeed(unsigned char pixels_per_second);
    void setScrollFont(fontChoices newFont);
    void setScrollColor(const rgb24 & newColor);
#define setScrollOffsetFromEdge setScrollOffsetFromTop // backwards compatibility
    void setScrollOffsetFromTop(int offset);
    void setScrollStartOffsetFromLeft(int offset);
    void updateScrollText(const char inputtext[]);
    void stopScrollText(void);
    int getScrollStatus(void) const;

    // foreground drawing
    void clearForeground(void);
    void setForegroundFont(fontChoices newFont);
    void drawForegroundPixel(int16_t x, int16_t y, bool opaque = true);
    void drawForegroundChar(int16_t x, int16_t y, char character, bool opaque = true);
    void drawForegroundString(int16_t x, int16_t y, const char text [], bool opaque = true);
    void drawForegroundMonoBitmap(int16_t x, int16_t y, uint8_t width, uint8_t height, uint8_t *bitmap, bool opaque = true);
    void displayForegroundDrawing(bool waitUntilComplete = true);

    // configuration
    void setRotation(rotationDegrees rotation);
    uint16_t getScreenWidth(void) const;
    uint16_t getScreenHeight(void) const;
    void setBrightness(uint8_t brightness);
    void setBackgroundBrightness(uint8_t brightness);
    void setColorCorrection(colorCorrectionModes mode);
    void setFont(fontChoices newFont);

    SMLayerForeground * getForegroundLayer(void);

private:
    // enable ISR access to private member variables
    friend void rowCalculationISR(void);
    friend void rowShiftCompleteISR(void);

    // functions called by ISR
    static void matrixCalculations(void);
    static void frameRefreshCallback_Background(void);
    void frameRefreshCallback_Foreground(void);

    // functions for refreshing
    static void loadMatrixBuffers(unsigned char currentRow);

    static color_chan_t colorCorrection(uint8_t inputcolor);
    static color_chan_t backgroundColorCorrection(uint8_t inputcolor);

    static void getPixel(uint8_t hardwareX, uint8_t hardwareY, rgb24 *xyPixel);
    static rgb24 *getCurrentRefreshRow(uint8_t y);
#ifdef SMARTMATRIX_TRIPLEBUFFER
    static rgb24 *getPreviousRefreshRow(uint8_t y);
    static uint32_t calculateFcInterpCoefficient();
#endif
#if COLOR_DEPTH_RGB > 24
    static void getBackgroundRefreshPixel(uint8_t x, uint8_t y, rgb48 &refreshPixel);
    bool getForegroundRefreshPixel(uint8_t x, uint8_t y, rgb48 &xyPixel);
#else
    static void getBackgroundRefreshPixel(uint8_t x, uint8_t y, rgb24 &refreshPixel);
    bool getForegroundRefreshPixel(uint8_t x, uint8_t y, rgb24 &xyPixel);
#endif
    static void handleBufferSwap(void);
    void handleForegroundDrawingCopy(void);
    void updateForeground(void);
    bool getForegroundPixel(uint8_t x, uint8_t y, rgb24 &xyPixel);
    void redrawForeground(void);
    void setScrollMinMax(void);

    // drawing functions not meant for user
    void drawHardwareHLine(uint8_t x0, uint8_t x1, uint8_t y, const rgb24& color);
    void drawHardwareVLine(uint8_t x, uint8_t y0, uint8_t y1, const rgb24& color);
    void bresteepline(int16_t x3, int16_t y3, int16_t x4, int16_t y4, const rgb24& color);
    void fillFlatSideTriangleInt(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3, const rgb24& color);
    static bool getBitmapPixelAtXY(uint8_t x, uint8_t y, uint8_t width, uint8_t height, const uint8_t *bitmap);

    // configuration helper functions
    static void calculateTimerLut(void);
    static void calculateBackgroundLUT(void);

    // configuration
    static colorCorrectionModes _ccmode;
    static screen_config screenConfig;
    static volatile bool brightnessChange;
    static int dimmingFactor;
    static uint8_t backgroundBrightness;
    static const int dimmingMaximum;

    // keeping track of drawing buffers
    static unsigned char currentDrawBuffer;
    static unsigned char currentRefreshBuffer;
#ifdef SMARTMATRIX_TRIPLEBUFFER    
    static unsigned char previousRefreshBuffer;
#endif
    static volatile bool swapPending;
    static bool swapWithCopy;

    // font
    static bool getBitmapFontPixelAtXY(unsigned char letter, unsigned char x, unsigned char y, const bitmap_font *font);
    const bitmap_font *fontLookup(fontChoices font) const;
    static uint16_t getBitmapFontRowAtXY(unsigned char letter, unsigned char y, const bitmap_font *font);

    // options
    char text[textLayerMaxStringLength];

    unsigned char textlen;
    int scrollcounter = 0;

    rgb24 textcolor = {0xff, 0xff, 0xff};
    int fontTopOffset = 1;
    int fontLeftOffset = 1;
    bool majorScrollFontChange = false;

    // scrolling
    ScrollMode scrollmode = bounceForward;
    unsigned char framesperscroll = 4;

    //bitmap size is 32 rows (supporting maximum dimension of screen height in all rotations), by 32 bits
    // double buffered to prevent flicker while drawing
    uint32_t foregroundBitmap[2][MATRIX_WIDTH][MATRIX_WIDTH / 32];
    bool foregroundCopyPending = false;

    const bitmap_font *scrollFont = &apple5x7;

    // these variables describe the text bitmap: size, location on the screen, and bounds of where it moves
    unsigned int textWidth;
    int scrollMin, scrollMax;
    int scrollPosition;

};

#endif
