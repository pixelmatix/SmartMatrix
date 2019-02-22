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

#if defined(__arm__) && defined(CORE_TEENSY)
    #if defined(V4HEADER)
        #include "MatrixHardware_KitV4.h"
    #else
        #include "MatrixHardware_KitV1.h"
    #endif
#endif

#if defined(ESP32)
    #include "MatrixHardware_ESP32_V0.h"
#endif

#include "MatrixCommon.h"
#include "CircularBuffer.h"

#include "Layer_Scrolling.h"
#include "Layer_Indexed.h"
#include "Layer_Background.h"

#include "SmartMatrixMultiplexedCommon.h"

#include "SmartMatrixPanelMaps.h"

#if defined(__arm__) && defined(CORE_TEENSY)
    #include "SmartMatrixMultiplexedRefreshTeensy.h"
    #include "SmartMatrixMultiplexedCalcTeensy.h"
#endif

#if defined(ESP32)
    #include "SmartMatrixMultiplexedRefreshEsp32.h"
    #include "SmartMatrixMultiplexedCalcEsp32.h"
#endif

#include "SmartMatrixAPA102Refresh.h"
#include "SmartMatrixAPA102Calc.h"

#if defined(__arm__) && defined(CORE_TEENSY)
    #include "SmartMatrixCoprocessorSend.h"
    #include "SmartMatrixCoprocessorCalc.h"
#endif

#if defined(__arm__) && defined(CORE_TEENSY)
#if 1
    #define SMARTMATRIX_ALLOCATE_BUFFERS(matrix_name, width, height, pwm_depth, buffer_rows, panel_type, option_flags) \
        static DMAMEM SmartMatrix3RefreshMultiplexed<pwm_depth, width, height, panel_type, option_flags>::rowDataStruct rowsDataBuffer[buffer_rows]; \
        SmartMatrix3RefreshMultiplexed<pwm_depth, width, height, panel_type, option_flags> matrix_name##Refresh(buffer_rows, rowsDataBuffer); \
        SmartMatrix3<pwm_depth, width, height, panel_type, option_flags> matrix_name(buffer_rows, rowsDataBuffer)

    #define SMARTMATRIX_APA_ALLOCATE_BUFFERS(matrix_name, width, height, pwm_depth, buffer_rows, panel_type, option_flags) \
        static DMAMEM SmartMatrixAPA102Refresh<pwm_depth, width, height, panel_type, option_flags>::frameDataStruct frameDataBuffer[buffer_rows]; \
        SmartMatrixAPA102Refresh<pwm_depth, width, height, panel_type, option_flags> matrix_name##Refresh(buffer_rows, frameDataBuffer); \
        SmartMatrixApaCalc<pwm_depth, width, height, panel_type, option_flags> matrix_name(buffer_rows, frameDataBuffer)

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
            static RGB_TYPE(storage_depth) layer_name##Bitmap[2*width*height];                                        \
            static SMLayerBackground<RGB_TYPE(storage_depth), background_options> layer_name(layer_name##Bitmap, width, height)  
#else
    // SmartMatrix Co-processor test code, assume not working at this point
    #define SMARTMATRIX_ALLOCATE_BUFFERS(matrix_name, width, height, pwm_depth, buffer_rows, panel_type, option_flags) \
        static DMAMEM SmartMatrix3CoprocessorSend<pwm_depth, width, height, panel_type, option_flags>::rowDataStruct rowsDataBuffer[buffer_rows]; \
        SmartMatrix3CoprocessorSend<pwm_depth, width, height, panel_type, option_flags> matrix_name##Refresh(buffer_rows, rowsDataBuffer); \
        SmartMatrixCoprocessorCalc<pwm_depth, width, height, panel_type, option_flags> matrix_name(buffer_rows, rowsDataBuffer)
#endif
#endif

#if defined(ESP32)
    #define SMARTMATRIX_ALLOCATE_BUFFERS(matrix_name, width, height, pwm_depth, buffer_rows, panel_type, option_flags) \
        SmartMatrix3RefreshMultiplexed<pwm_depth, width, height, panel_type, option_flags> matrix_name##Refresh; \
        SmartMatrix3<pwm_depth, width, height, panel_type, option_flags> matrix_name

    #define SMARTMATRIX_ALLOCATE_BACKGROUND_LAYER(layer_name, width, height, storage_depth, background_options) \
        typedef RGB_TYPE(storage_depth) SM_RGB;                                                                 \
        static SMLayerBackground<RGB_TYPE(storage_depth), background_options> layer_name(width, height)  

    #define SMARTMATRIX_ALLOCATE_SCROLLING_LAYER(layer_name, width, height, storage_depth, scrolling_options) \
        typedef RGB_TYPE(storage_depth) SM_RGB;                                                                 \
        static SMLayerScrolling<RGB_TYPE(storage_depth), scrolling_options> layer_name(width, height)  

    #define SMARTMATRIX_ALLOCATE_INDEXED_LAYER(layer_name, width, height, storage_depth, indexed_options) \
        typedef RGB_TYPE(storage_depth) SM_RGB;                                                                 \
        static SMLayerIndexed<RGB_TYPE(storage_depth), indexed_options> layer_name(width, height)  
#endif

// platform-specific
#if defined(__arm__) && defined(CORE_TEENSY)
    #include "SmartMatrixMultiplexedRefreshTeensy_Impl.h"
    #include "SmartMatrixAPA102RefreshTeensy_Impl.h"
    #include "SmartMatrixMultiplexedCalcTeensy_Impl.h"
#endif

#if defined(ESP32)
    #include "SmartMatrixMultiplexedRefreshEsp32_Impl.h"
    //#include "SmartMatrixAPA102RefreshEsp32_Impl.h"
    #include "SmartMatrixMultiplexedCalcEsp32_Impl.h"
#endif


#if defined(__arm__) && defined(CORE_TEENSY)
    #include "SmartMatrixCoprocessorSend_Impl.h"
    #include "SmartMatrixCoprocessorCalc_Impl.h"
    #include "SmartMatrixAPA102Calc_Impl.h"
#endif

#endif
