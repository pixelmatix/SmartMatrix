/*
 * SmartMatrix Library - Main Refresh Class
 *
 * Copyright (c) 2015 Louis Beaudoin (Pixelmatix)
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

#ifndef SmartMatrix3_h
#define SmartMatrix3_h

#include <stdint.h>

#include "Arduino.h"

#include "MatrixHardware_KitV1.h"

#include "MatrixCommon.h"

#include "Layer_Scrolling.h"
#include "Layer_Indexed.h"
#include "Layer_Background.h"

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

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
class SmartMatrix3 {
public:
    // init
    SmartMatrix3(uint8_t bufferrows, uint32_t * dataBuffer, uint8_t * blockBuffer);
    void begin(void);
    void addLayer(SM_Layer * newlayer);

    // configuration
    void setRotation(rotationDegrees rotation);
    void setBrightness(uint8_t brightness);
    void setRefreshRate(uint8_t newRefreshRate);

    // get info
    uint16_t getScreenWidth(void) const;
    uint16_t getScreenHeight(void) const;
    uint8_t getRefreshRate(void);
    bool getdmaBufferUnderrunFlag(void);
    bool getRefreshRateLoweredFlag(void);

    // debug
    void countFPS(void);

private:
    SM_Layer * baseLayer;

    // enable ISR access to private member variables
    template <int refreshDepth1, int matrixWidth1, int matrixHeight1, unsigned char panelType1, unsigned char optionFlags1>
    friend void rowCalculationISR(void);
    template <int refreshDepth1, int matrixWidth1, int matrixHeight1, unsigned char panelType1, unsigned char optionFlags1>
    friend void rowShiftCompleteISR(void);

    // functions called by ISR
    static void matrixCalculations(bool initial = false);

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
    static volatile bool dmaBufferUnderrun;
    static int dimmingFactor;
    static const int dimmingMaximum;
    static rotationDegrees rotation;
    static uint8_t colorDepthRgb;
    static uint8_t refreshRate;
    static const int matrixPanelHeight;    
    static const int matrixRowPairOffset;    
    static const int matrixRowsPerFrame;    

    const static uint8_t latchesPerRow = refreshDepth/COLOR_CHANNELS_PER_PIXEL;
    static uint8_t dmaBufferNumRows;
    static uint8_t dmaBufferBytesPerPixel;
    static uint16_t dmaBufferBytesPerRow;
    static bool dmaBufferUnderrunSinceLastCheck;
    static bool refreshRateLowered;
    static bool refreshRateChanged;

    static uint32_t * matrixUpdateData;
    static matrixUpdateBlock * matrixUpdateBlocks;
    static addresspair * addressLUT;
    static timerpair * timerLUT;
    static timerpair * timerPairIdle;

    static SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>* globalinstance;
};

#define SMARTMATRIX_HUB75_32ROW_MOD16SCAN             0
#define SMARTMATRIX_HUB75_16ROW_MOD8SCAN              1

#define CONVERT_PANELTYPE_TO_MATRIXPANELHEIGHT(x)   ((x == SMARTMATRIX_HUB75_32ROW_MOD16SCAN ? 32 : 0) | \
                                                     (x == SMARTMATRIX_HUB75_16ROW_MOD8SCAN ? 16 : 0))

#define CONVERT_PANELTYPE_TO_MATRIXROWPAIROFFSET(x)   ((x == SMARTMATRIX_HUB75_32ROW_MOD16SCAN ? 16 : 0) | \
                                                     (x == SMARTMATRIX_HUB75_16ROW_MOD8SCAN ? 8 : 0))

#define CONVERT_PANELTYPE_TO_MATRIXROWSPERFRAME(x)   ((x == SMARTMATRIX_HUB75_32ROW_MOD16SCAN ? 16 : 0) | \
                                                     (x == SMARTMATRIX_HUB75_16ROW_MOD8SCAN ? 8 : 0))

#define SMARTMATRIX_OPTIONS_NONE                    0
#define SMARTMATRIX_OPTIONS_C_SHAPE_STACKING        (1 << 0)
#define SMARTMATRIX_OPTIONS_BOTTOM_TO_TOP_STACKING  (1 << 1)


// single matrixUpdateBlocks buffer is divided up to hold matrixUpdateBlocks, addressLUT, timerLUT to simplify user sketch code and reduce constructor parameters
#define SMARTMATRIX_ALLOCATE_BUFFERS(matrix_name, width, height, pwm_depth, buffer_rows, panel_type, option_flags) \
    static DMAMEM uint32_t matrixUpdateData[buffer_rows * ((width * height) / CONVERT_PANELTYPE_TO_MATRIXPANELHEIGHT(panel_type)) * (pwm_depth/COLOR_CHANNELS_PER_PIXEL / sizeof(uint32_t)) * DMA_UPDATES_PER_CLOCK]; \
    static DMAMEM uint8_t matrixUpdateBlocks[(sizeof(matrixUpdateBlock) * buffer_rows * pwm_depth/COLOR_CHANNELS_PER_PIXEL) + (sizeof(addresspair) * CONVERT_PANELTYPE_TO_MATRIXROWSPERFRAME(panel_type)) + (sizeof(timerpair) * pwm_depth/COLOR_CHANNELS_PER_PIXEL) + sizeof(timerpair)]; \
    SmartMatrix3<pwm_depth, width, height, panel_type, option_flags> matrix_name(buffer_rows, matrixUpdateData, matrixUpdateBlocks)

#define SMARTMATRIX_ALLOCATE_SCROLLING_LAYER(layer_name, width, height, storage_depth, scrolling_options) \
    typedef RGB_TYPE(storage_depth) SM_RGB;                                                                 \
    static uint8_t layer_name##Bitmap[width * (height / 8)];                                              \
    static SMLayerScrolling<RGB_TYPE(storage_depth), scrolling_options> layer_name(layer_name##Bitmap, width, height)  

#define SMARTMATRIX_ALLOCATE_INDEXED_LAYER(layer_name, width, height, storage_depth, indexed_options) \
    typedef RGB_TYPE(storage_depth) SM_RGB;                                                                 \
    static uint8_t layer_name##Bitmap[2 * width * (height / 8)];                                              \
    static SMLayerIndexed<RGB_TYPE(storage_depth), indexed_options> layer_name(layer_name##Bitmap, width, height)  

#define SMARTMATRIX_ALLOCATE_BACKGROUND_LAYER(layer_name, width, height, storage_depth, background_options) \
    typedef RGB_TYPE(storage_depth) SM_RGB;                                                                 \
    static RGB_TYPE(storage_depth) backgroundBitmap[2*width*height];                                        \
    static SMLayerBackground<RGB_TYPE(storage_depth), background_options> layer_name(backgroundBitmap, width, height)  


#include "SmartMatrix_Impl.h"

#endif
