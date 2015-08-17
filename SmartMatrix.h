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

#include <stdint.h>

#include "MatrixHardware_KitV1.h"

#include "MatrixCommon.h"

#include "Layer_Foreground.h"
#include "Layer_Background.h"

#define ENABLE_FADECANDY_GAMMA_CORRECTION               1

// definition telling FastLED that setBackBuffer() and getRealBackBuffer() are available
#define SMART_MATRIX_CAN_TRIPLE_BUFFER 1

// enable true triple buffering and interpolation
//#define SMARTMATRIX_TRIPLEBUFFER

typedef struct timerpair {
    uint16_t timer_oe;
    uint16_t timer_period;
} timerpair;

typedef struct addresspair {
    uint16_t bits_to_clear;
    uint16_t bits_to_set;
} addresspair;

typedef struct matrixUpdateBlock {
    timerpair timerValues;
    addresspair addressValues;
} matrixUpdateBlock;

template <typename RGB>
class SmartMatrix {
public:
    SmartMatrix(uint8_t width, uint8_t height, uint8_t depth, uint8_t bufferrows, uint32_t * dataBuffer, uint8_t * blockBuffer);
    void begin(void);

    // drawing functions
    void swapBuffers(bool copy = true);
#ifdef SMARTMATRIX_TRIPLEBUFFER
    void swapBuffersWithInterpolation_frames(int framesToInterpolate, bool copy = false);
    void swapBuffersWithInterpolation_ms(int interpolationSpan_ms, bool copy = false);
#endif
    void drawPixel(int16_t x, int16_t y, const RGB& color);
    void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, const RGB& color);
    void drawFastVLine(int16_t x, int16_t y0, int16_t y1, const RGB& color);
    void drawFastHLine(int16_t x0, int16_t x1, int16_t y, const RGB& color);
    void drawCircle(int16_t x0, int16_t y0, uint16_t radius, const RGB& color);
    void fillCircle(int16_t x0, int16_t y0, uint16_t radius, const RGB& outlineColor, const RGB& fillColor);
    void fillCircle(int16_t x0, int16_t y0, uint16_t radius, const RGB& color);
    void drawEllipse(int16_t x0, int16_t y0, uint16_t radiusX, uint16_t radiusY, const RGB& color);
    void drawTriangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3, const RGB& color);
    void fillTriangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3, const RGB& fillColor);
    void fillTriangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3,
        const RGB& outlineColor, const RGB& fillColor);
    void drawRectangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, const RGB& color);
    void fillRectangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, const RGB& color);
    void fillRectangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, const RGB& outlineColor, const RGB& fillColor);
    void drawRoundRectangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t radius, const RGB& outlineColor);
    void fillRoundRectangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t radius, const RGB& fillColor);
    void fillRoundRectangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t radius,
        const RGB& outlineColor, const RGB& fillColor);
    void fillScreen(const RGB& color);
    void drawChar(int16_t x, int16_t y, const RGB& charColor, char character);
    void drawString(int16_t x, int16_t y, const RGB& charColor, const char text[]);
    void drawString(int16_t x, int16_t y, const RGB& charColor, const RGB& backColor, const char text[]);
    void drawMonoBitmap(int16_t x, int16_t y, uint8_t width, uint8_t height, const RGB& bitmapColor, const uint8_t *bitmap);

    RGB readPixel(int16_t x, int16_t y);
    RGB *backBuffer(void);

    void setBackBuffer(RGB *newBuffer);

    RGB *getRealBackBuffer();

    // scroll text
    void scrollText(const char inputtext[], int numScrolls);
    void setScrollMode(ScrollMode mode);
    void setScrollSpeed(unsigned char pixels_per_second);
    void setScrollFont(fontChoices newFont);
    void setScrollColor(const RGB & newColor);
#define setScrollOffsetFromEdge setScrollOffsetFromTop // backwards compatibility
    void setScrollOffsetFromTop(int offset);
    void setScrollStartOffsetFromLeft(int offset);
    void updateScrollText(const char inputtext[]);
    void stopScrollText(void);
    int getScrollStatus(void);

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
    void setRefreshRate(uint8_t newRefreshRate);

    SMLayerForeground<RGB> * foregroundLayer;
    SMLayerBackground<RGB> * backgroundLayer;

    void useDefaultLayers(void);
    void addLayer(SM_Layer<RGB> * newlayer);

    SM_Layer<RGB> * baseLayer;

    void countFPS(void);

private:
    // enable ISR access to private member variables
    template <typename RGB1>
    friend void rowCalculationISR(void);
    template <typename RGB1>
    friend void rowShiftCompleteISR(void);

    // functions called by ISR
    static void matrixCalculations(void);

    // functions for refreshing
    static void loadMatrixBuffers(unsigned char currentRow);

    // configuration helper functions
    static void calculateTimerLut(void);

    // configuration
    static volatile bool brightnessChange;
    static volatile bool rotationChange;
    static int dimmingFactor;
    static const int dimmingMaximum;
    static rotationDegrees rotation;
    static uint8_t matrixWidth, matrixHeight;
    static uint8_t colorDepthRgb;
    static uint8_t refreshRate;

    static uint8_t latchesPerRow;
    static uint8_t dmaBufferNumRows;
    static uint8_t dmaBufferBytesPerPixel;
    static uint16_t dmaBufferBytesPerRow;

    static uint32_t * matrixUpdateData;
    static matrixUpdateBlock * matrixUpdateBlocks;
    static addresspair * addressLUT;
    static timerpair * timerLUT;

    static SmartMatrix<RGB>* globalinstance;
};

// single matrixUpdateBlocks buffer is divided up to hold matrixUpdateBlocks, addressLUT, timerLUT to simplify user sketch code and reduce constructor parameters
#define SMARTMATRIX_ALLOCATE_BUFFERS(width, height, storage_depth, pwm_depth, rows) \
    typedef RGB_TYPE(COLOR_DEPTH) SM_RGB; \
    static DMAMEM uint32_t matrixUpdateData[rows * width * (pwm_depth/3 / sizeof(uint32_t)) * 2]; \
    static DMAMEM uint8_t matrixUpdateBlocks[(sizeof(matrixUpdateBlock) * rows * pwm_depth/3) + (sizeof(addresspair) * height/2) + (sizeof(timerpair) * pwm_depth/3)]; \
    SmartMatrix<RGB_TYPE(storage_depth)> matrix(width, height, pwm_depth, rows, matrixUpdateData, matrixUpdateBlocks)

#define SMARTMATRIX_SETUP_DEFAULT_LAYERS(width, height, storage_depth) \
    static RGB_TYPE(storage_depth) backgroundBitmap[2*width*height];                              \
    static SMLayerBackground<RGB_TYPE(storage_depth)> backgroundLayer(backgroundBitmap, width, height);  \
    matrix.addLayer(&backgroundLayer);                                          \
    static uint8_t foregroundBitmap[2 * height * (width / 8)];                  \
    static SMLayerForeground<RGB_TYPE(storage_depth)> foregroundLayer(foregroundBitmap, width, height);  \
    matrix.addLayer(&foregroundLayer);                                          \
    matrix.useDefaultLayers();                                                  \
    matrix.begin()

#define SMARTMATRIX_ALLOCATE_FOREGROUND_LAYER(layername, width, height) \
    static uint8_t foregroundBitmap[2 * width * (height / 8)];    \
    static SMLayerForeground layername(foregroundBitmap, width, height)

#include "MatrixGraphics_Impl.h"
#include "MatrixConfiguration_Impl.h"
#include "MatrixForeground_Impl.h"
#include "SmartMatrix_Impl.h"

#endif
