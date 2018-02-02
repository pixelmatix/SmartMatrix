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

#ifdef V4HEADER
    #include "MatrixHardware_KitV4.h"
#else
    #include "MatrixHardware_KitV1.h"
#endif

#include "MatrixCommon.h"

#include "Layer_Scrolling.h"
#include "Layer_Indexed.h"
#include "Layer_Background.h"

typedef struct refresh_timerpair {
    uint16_t timer_oe;
    uint16_t timer_period;
} refresh_timerpair;

typedef struct calc_timerpair {
    uint16_t timer_brightness;
    uint16_t timer_refreshrate;
} calc_timerpair;

typedef struct refresh_addresspair {
    uint16_t bits_to_clear;
    uint16_t bits_to_set;
} refresh_addresspair;

typedef void (*matrix_calc_callback)(bool initial);
typedef void (*matrix_underrun_callback)(void);

#define SMARTMATRIX_HUB75_32ROW_MOD16SCAN             0
#define SMARTMATRIX_HUB75_16ROW_MOD8SCAN              1

#define CONVERT_PANELTYPE_TO_MATRIXPANELHEIGHT(x)   ((x == SMARTMATRIX_HUB75_32ROW_MOD16SCAN ? 32 : 0) | \
                                                     (x == SMARTMATRIX_HUB75_16ROW_MOD8SCAN ? 16 : 0))

#define CONVERT_PANELTYPE_TO_MATRIXROWPAIROFFSET(x)   ((x == SMARTMATRIX_HUB75_32ROW_MOD16SCAN ? 16 : 0) | \
                                                     (x == SMARTMATRIX_HUB75_16ROW_MOD8SCAN ? 8 : 0))

#define CONVERT_PANELTYPE_TO_MATRIXROWSPERFRAME(x)   ((x == SMARTMATRIX_HUB75_32ROW_MOD16SCAN ? 16 : 0) | \
                                                     (x == SMARTMATRIX_HUB75_16ROW_MOD8SCAN ? 8 : 0))

#define LATCHES_PER_ROW (refreshDepth/COLOR_CHANNELS_PER_PIXEL)
#define ROWS_PER_FRAME (CONVERT_PANELTYPE_TO_MATRIXROWSPERFRAME(panelType))

#define SMARTMATRIX_OPTIONS_NONE                    0
#define SMARTMATRIX_OPTIONS_C_SHAPE_STACKING        (1 << 0)
#define SMARTMATRIX_OPTIONS_BOTTOM_TO_TOP_STACKING  (1 << 1)

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
class SmartMatrix3RefreshMultiplexed {
public:
    struct rowBitStruct {
        uint8_t data[((((matrixWidth * matrixHeight) / CONVERT_PANELTYPE_TO_MATRIXPANELHEIGHT(panelType)) * DMA_UPDATES_PER_CLOCK))];
        uint8_t rowAddress;
        refresh_timerpair timerValues;
#ifndef ADDX_UPDATE_ON_DATA_PINS
        refresh_addresspair addressValues;
#endif
    };

    struct rowDataStruct {
        rowBitStruct rowbits[refreshDepth/COLOR_CHANNELS_PER_PIXEL];
    };

    // init
    SmartMatrix3RefreshMultiplexed(uint8_t bufferrows, rowDataStruct * rowDataBuffer);
    static void refresh_begin(void);

    static void refresh_setBrightness(uint8_t newBrightness);

    // refresh API
    static rowDataStruct * refresh_getNextRowBufferPtr(void);
    static void refresh_writeRowBuffer(uint8_t currentRow);
    static void refresh_recoverFromDmaUnderrun(void);
    static bool refresh_isRowBufferFree(void);
    static void refresh_setRefreshRate(uint8_t newRefreshRate);
    static void setMatrixCalculationsCallback(matrix_calc_callback f);
    static void setMatrixUnderrunCallback(matrix_underrun_callback f);

private:
    // enable ISR access to private member variables
    template <int refreshDepth1, int matrixWidth1, int matrixHeight1, unsigned char panelType1, unsigned char optionFlags1>
    friend void refresh_rowCalculationISR(void);
    template <int refreshDepth1, int matrixWidth1, int matrixHeight1, unsigned char panelType1, unsigned char optionFlags1>
    friend void refresh_rowShiftCompleteISR(void);

    // configuration helper functions
    static void refresh_calculateTimerLUT(void);

    static int refresh_dimmingFactor;
    static const int refresh_dimmingMaximum = 255;
    static uint8_t refresh_refreshRate;
    static uint8_t refresh_dmaBufferNumRows;
    static rowDataStruct * refresh_matrixUpdateRows;

    static refresh_addresspair refresh_addressLUT[ROWS_PER_FRAME];
    static refresh_timerpair refresh_timerLUT[LATCHES_PER_ROW];
    static refresh_timerpair refresh_timerPairIdle;
    static matrix_calc_callback matrixCalcCallback;
    static matrix_underrun_callback matrixUnderrunCallback;
};

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
class SmartMatrix3 {
public:
    typedef typename SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::rowDataStruct rowDataStruct;

    // init
    SmartMatrix3(uint8_t bufferrows, rowDataStruct * rowDataBuffer);
    void begin(void);
    void addLayer(SM_Layer * newlayer);

    // configuration
    void setRotation(rotationDegrees rotation);
    void setBrightness(uint8_t newBrightness);
    void setRefreshRate(uint8_t newRefreshRate);

    // get info
    uint16_t getScreenWidth(void) const;
    uint16_t getScreenHeight(void) const;
    uint8_t getRefreshRate(void);
    bool getdmaBufferUnderrunFlag(void);
    bool getRefreshRateLoweredFlag(void);

    // debug
    void countFPS(void);

    // functions called by ISR
    static void matrixCalculations(bool initial);
    static void dmaBufferUnderrunCallback(void);

private:
    static SM_Layer * baseLayer;

    // functions for refreshing
    static void loadMatrixBuffers(unsigned char currentRow);
    static void loadMatrixBuffers48(rowDataStruct * currentRowDataPtr, unsigned char currentRow);
    static void loadMatrixBuffers24(rowDataStruct * currentRowDataPtr, unsigned char currentRow);

    // configuration
    static volatile bool brightnessChange;
    static volatile bool rotationChange;
    static volatile bool dmaBufferUnderrun;
    static int brightness;
    static rotationDegrees rotation;
    static uint8_t calc_refreshRate;   
    static bool dmaBufferUnderrunSinceLastCheck;
    static bool refreshRateLowered;
    static bool refreshRateChanged;
};

// single matrixUpdateBlocks buffer is divided up to hold matrixUpdateBlocks, refresh_addressLUT, refresh_timerLUT to simplify user sketch code and reduce constructor parameters
#define SMARTMATRIX_ALLOCATE_BUFFERS(matrix_name, width, height, pwm_depth, buffer_rows, panel_type, option_flags) \
    static DMAMEM SmartMatrix3<pwm_depth, width, height, panel_type, option_flags>::rowDataStruct rowsDataBuffer[buffer_rows]; \
    SmartMatrix3RefreshMultiplexed<pwm_depth, width, height, panel_type, option_flags> matrix_name##Refresh(buffer_rows, rowsDataBuffer); \
    SmartMatrix3<pwm_depth, width, height, panel_type, option_flags> matrix_name(buffer_rows, rowsDataBuffer)

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
#include "SmartMatrix3Calc_Impl.h"

#endif
