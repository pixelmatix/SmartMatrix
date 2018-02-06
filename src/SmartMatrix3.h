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
#include "CircularBuffer.h"

#include "Layer_Scrolling.h"
#include "Layer_Indexed.h"
#include "Layer_Background.h"

#include "SmartMatrixMultiplexedCommon.h"
#include "SmartMatrixMultiplexedRefresh.h"
#include "SmartMatrixMultiplexedCalc.h"

#include "SmartMatrixAPA102Refresh.h"
#include "SmartMatrixAPA102Calc.h"

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

#include "SmartMatrixMultiplexedRefresh_Impl.h"
#include "SmartMatrixMultiplexedCalc_Impl.h"

#include "SmartMatrixAPA102Refresh_Impl.h"
#include "SmartMatrixAPA102Calc_Impl.h"

#endif
