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

template <int refreshDepth, unsigned char optionFlags>
class SmartMatrix3 {
public:
    SmartMatrix3(uint8_t width, uint8_t height, uint8_t depth, uint8_t bufferrows, uint32_t * dataBuffer, uint8_t * blockBuffer);
    void begin(void);

    // configuration
    void setRotation(rotationDegrees rotation);
    uint16_t getScreenWidth(void) const;
    uint16_t getScreenHeight(void) const;
    void setBrightness(uint8_t brightness);
    void enableColorCorrection(bool enabled);
    void setFont(fontChoices newFont);
    void setRefreshRate(uint8_t newRefreshRate);

    void addLayer(SM_Layer * newlayer);

    SM_Layer * baseLayer;

    void countFPS(void);

private:
    // enable ISR access to private member variables
    template <int refreshDepth1, unsigned char optionFlags1>
    friend void rowCalculationISR(void);
    template <int refreshDepth1, unsigned char optionFlags1>
    friend void rowShiftCompleteISR(void);

    // functions called by ISR
    static void matrixCalculations(void);

    // functions for refreshing
    static void loadMatrixBuffers(unsigned char currentRow);
    static void loadMatrixBuffers48(unsigned char currentRow, unsigned char freeRowBuffer);
    static void loadMatrixBuffers36(unsigned char currentRow, unsigned char freeRowBuffer);
    static void loadMatrixBuffers24(unsigned char currentRow, unsigned char freeRowBuffer);

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

    const static uint8_t latchesPerRow = refreshDepth/COLOR_CHANNELS_PER_PIXEL;
    static uint8_t dmaBufferNumRows;
    static uint8_t dmaBufferBytesPerPixel;
    static uint16_t dmaBufferBytesPerRow;

    static uint32_t * matrixUpdateData;
    static matrixUpdateBlock * matrixUpdateBlocks;
    static addresspair * addressLUT;
    static timerpair * timerLUT;

    static SmartMatrix3<refreshDepth, optionFlags>* globalinstance;
};

#define SMARTMATRIX_OPTIONS_NONE            0


// single matrixUpdateBlocks buffer is divided up to hold matrixUpdateBlocks, addressLUT, timerLUT to simplify user sketch code and reduce constructor parameters
#define SMARTMATRIX_ALLOCATE_BUFFERS(width, height, storage_depth, pwm_depth, rows, option_flags) \
    typedef RGB_TYPE(storage_depth) SM_RGB; \
    static DMAMEM uint32_t matrixUpdateData[rows * width * (pwm_depth/COLOR_CHANNELS_PER_PIXEL / sizeof(uint32_t)) * DMA_UPDATES_PER_CLOCK]; \
    static DMAMEM uint8_t matrixUpdateBlocks[(sizeof(matrixUpdateBlock) * rows * pwm_depth/COLOR_CHANNELS_PER_PIXEL) + (sizeof(addresspair) * height/PIXELS_UPDATED_PER_CLOCK) + (sizeof(timerpair) * pwm_depth/COLOR_CHANNELS_PER_PIXEL)]; \
    SmartMatrix3<pwm_depth, option_flags> matrix(width, height, pwm_depth, rows, matrixUpdateData, matrixUpdateBlocks)

#define SMARTMATRIX_ALLOCATE_DEFAULT_LAYERS(width, height, storage_depth) \
    static RGB_TYPE(storage_depth) backgroundBitmap[2*width*height];                              \
    static SMLayerBackground<RGB_TYPE(storage_depth)> backgroundLayer(backgroundBitmap, width, height);  \
    static uint8_t foregroundBitmap[2 * height * (width / 8)];                  \
    static SMLayerForeground<RGB_TYPE(storage_depth)> foregroundLayer(foregroundBitmap, width, height);  

#define SMARTMATRIX_MIN_SETUP_DEFAULT_LAYERS() \
    matrix.addLayer(&backgroundLayer);                                          \
    matrix.addLayer(&foregroundLayer);                                          \
    matrix.begin()

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

#include "MatrixConfiguration_Impl.h"
#include "SmartMatrix_Impl.h"

/* this dummy class and enum is for FastLED backwards compatibility */
typedef enum colorCorrectionModes {
    ccNone,
    cc24,
    cc12,
    cc48
} colorCorrectionModes;

class SmartMatrix {
    public:
        SmartMatrix();
    void begin(void);

    void swapBuffers(bool copy = true);

    void fillScreen(const rgb24& color);

    rgb24 *backBuffer(void);

    void setBrightness(uint8_t brightness);

    void setColorCorrection(colorCorrectionModes mode);
};
/* end FastLED backwards compatibility */

#endif
