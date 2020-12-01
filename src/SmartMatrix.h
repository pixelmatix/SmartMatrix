/*
 * SmartMatrix Library - Main Refresh Class
 *
 * Copyright (c) 2020 Louis Beaudoin (Pixelmatix)
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

#ifndef SmartMatrix4_h
#define SmartMatrix4_h

// Temporary definition for FastLED Compatibility: to get the CRGB<->rgb24 conversion, FastLED expects SmartMatrix_h or SmartMatrix3_h to be definied, and defining SmartMatrix_h causes problems
#ifndef SmartMatrix3_h
#define SmartMatrix3_h
#endif

// I got this version code from FastLED, thanks!
#if (__GNUC__ > 4) || (__GNUC__ == 4 && __GNUC_MINOR__ >= 4)
#define SM_HAS_PRAGMA_MESSAGE
#endif

#define SM_VERSION 4000003
#ifndef SM_INTERNAL
#  ifdef SM_HAS_PRAGMA_MESSAGE
#    pragma message "SmartMatrix Library version 4.000.003"
#  else
#    warning SmartMatrix Library version 4.000.003  (Not really a warning, just telling you here.)
#  endif
#endif

#include <stdint.h>

#include "Arduino.h"

#include "MatrixCommon.h"
#include "CircularBuffer_SM.h"

#include "Layer_Scrolling.h"
#include "Layer_Indexed.h"
#include "Layer_Background.h"

// For backwards compatiblity, this needs to be defined at the top of the sketch, so that "Adafruit_GFX.h" is only included if desired
#ifdef USE_ADAFRUIT_GFX_LAYERS
#include "Layer_BackgroundGfx.h"
#include "Layer_Gfx_Mono.h"
#endif

#if (defined(SUPPORT_ADAFRUIT_GFX_LIBRARY) || defined(SMARTMATRIX_ALLOCATE_BACKGROUND_GFX_LAYER) || defined(SMARTMATRIX_ALLOCATE_SCROLLING_GFX_LAYER) || defined(SMARTMATRIX_ALLOCATE_INDEXED_GFX_LAYER))
#pragma GCC error "API change: don't use `SUPPORT_ADAFRUIT_GFX_LIBRARY` or the `ALLOCATE*GFX*` macros, use  `USE_ADAFRUIT_GFX_LAYERS` and the normal ALLOCATE macros instead"
#endif

#include "MatrixCommonHub75.h"

#include "MatrixCommonApa102.h"
#include "MatrixCommonApa102Refresh.h"
#include "MatrixCommonApa102Calc.h"

#ifndef MATRIX_HARDWARE_H
#pragma GCC error "No MatrixHardware*.h file included - You must include one at the top of your sketch"
#endif

#include "MatrixPanelMaps.h"

#if defined(__arm__) && defined(CORE_TEENSY) && !defined(__IMXRT1062__)  // Teensy 3.x
    #include "MatrixTeensy3Hub75Refresh.h"
    #include "MatrixTeensy3Hub75Calc.h"
#endif

#if defined(__IMXRT1062__) // Teensy 4.0/4.1
    #include "MatrixTeensy4Hub75Refresh.h"
    #include "MatrixTeensy4Hub75Calc.h"
#endif

#if defined(ESP32)
    #include "MatrixEsp32Hub75Refresh.h"
    #include "MatrixEsp32Hub75Calc.h"
    #include "MatrixEsp32Hub75Refresh_NT.h"
    #include "MatrixEsp32Hub75Calc_NT.h"
#endif

#include "MatrixCommonApa102Refresh.h"
#include "MatrixCommonApa102Calc.h"

#if defined(SMARTMATRIX_USE_PSRAM) && defined(ARDUINO_TEENSY41) // On Teensy 4.1 with PSRAM expansion
    #define BACKGROUND_MEMSECTION EXTMEM // Use PSRAM to store background layer drawing and refresh buffers (slower but saves RAM)
#else
    #define BACKGROUND_MEMSECTION
#endif

#if defined(__arm__) && defined(CORE_TEENSY)
    // TODO: use same definition for Teensy 3.x and 4.x HUB75 SMARTMATRIX_ALLOCATE_BUFFERS() if possible 
    #if !defined(__IMXRT1062__) // Teensy 3.x
        #define SMARTMATRIX_ALLOCATE_BUFFERS(matrix_name, width, height, pwm_depth, buffer_rows, panel_type, option_flags) \
            static DMAMEM SmartMatrixHub75Refresh<pwm_depth, width, height, panel_type, option_flags>::rowDataStruct rowsDataBuffer[buffer_rows]; \
            SmartMatrixHub75Refresh<pwm_depth, width, height, panel_type, option_flags> matrix_name##Refresh(buffer_rows, rowsDataBuffer); \
            SmartMatrixHub75Calc<pwm_depth, width, height, panel_type, option_flags> matrix_name(buffer_rows, rowsDataBuffer)
        #define SMARTMATRIX_APA_ALLOCATE_BUFFERS(matrix_name, width, height, pwm_depth, buffer_rows, panel_type, option_flags) \
            static DMAMEM SmartMatrixAPA102Refresh<pwm_depth, width, height, panel_type, option_flags>::frameDataStruct frameDataBuffer[buffer_rows]; \
            SmartMatrixAPA102Refresh<pwm_depth, width, height, panel_type, option_flags> matrix_name##Refresh(buffer_rows, frameDataBuffer); \
            SmartMatrixApaCalc<pwm_depth, width, height, panel_type, option_flags> matrix_name(buffer_rows, frameDataBuffer)
    #else   // Teensy 4.x
        #define SMARTMATRIX_ALLOCATE_BUFFERS(matrix_name, width, height, pwm_depth, buffer_rows, panel_type, option_flags) \
            static volatile DMAMEM SmartMatrixRefreshT4<pwm_depth, width, height, panel_type, option_flags>::rowDataStruct rowsDataBuffer[buffer_rows]; \
            SmartMatrixRefreshT4<pwm_depth, width, height, panel_type, option_flags> matrix_name##Refresh(buffer_rows, rowsDataBuffer); \
            SmartMatrixHub75Calc<pwm_depth, width, height, panel_type, option_flags> matrix_name(buffer_rows, rowsDataBuffer)
        #define SMARTMATRIX_APA_ALLOCATE_BUFFERS(matrix_name, width, height, pwm_depth, buffer_rows, panel_type, option_flags) \
            FlexIOSPI SPIFLEX(FLEXIO_PIN_APA102_DAT, FLEXIO_PIN_APA102_DAT, FLEXIO_PIN_APA102_CLK); /* overlapping MOSI pin on MISO as we don't need MISO */ \
            static DMAMEM SmartMatrixAPA102Refresh<pwm_depth, width, height, panel_type, option_flags>::frameDataStruct frameDataBuffer[buffer_rows]; \
            SmartMatrixAPA102Refresh<pwm_depth, width, height, panel_type, option_flags> matrix_name##Refresh(buffer_rows, frameDataBuffer); \
            SmartMatrixApaCalc<pwm_depth, width, height, panel_type, option_flags> matrix_name(buffer_rows, frameDataBuffer)
    #endif

#ifdef USE_ADAFRUIT_GFX_LAYERS
        #define SMARTMATRIX_ALLOCATE_BACKGROUND_LAYER(layer_name, width, height, storage_depth, background_options) \
            typedef RGB_TYPE(storage_depth) SM_RGB;                                                                 \
            static BACKGROUND_MEMSECTION RGB_TYPE(storage_depth) layer_name##Bitmap[2*width*height];                                        \
            static color_chan_t layer_name##colorCorrectionLUT[sizeof(SM_RGB) <= 3 ? 256 : 4096];                          \
            static SMLayerBackgroundGFX<RGB_TYPE(storage_depth), background_options> layer_name(layer_name##Bitmap, width, height, layer_name##colorCorrectionLUT)  

        #define SMARTMATRIX_ALLOCATE_SCROLLING_LAYER(layer_name, width, height, storage_depth, adafruitgfxlayer_options) \
            typedef RGB_TYPE(storage_depth) SM_RGB;                                                                 \
            static uint8_t layer_name##Bitmap[2 * ROUND_UP_TO_MULTIPLE_OF_8(width) * (ROUND_UP_TO_MULTIPLE_OF_8(height) / 8)];                                              \
            static SMLayerGFXMono<RGB_TYPE(storage_depth), rgb1, adafruitgfxlayer_options> layer_name(layer_name##Bitmap, width, height, ROUND_UP_TO_MULTIPLE_OF_8(width), ROUND_UP_TO_MULTIPLE_OF_8(height))  

        #define SMARTMATRIX_ALLOCATE_INDEXED_LAYER(layer_name, width, height, storage_depth, adafruitgfxlayer_options) \
            typedef RGB_TYPE(storage_depth) SM_RGB;                                                                 \
            static uint8_t layer_name##Bitmap[2 * width * (ROUND_UP_TO_MULTIPLE_OF_8(height) / 8)];                                              \
            static SMLayerGFXMono<RGB_TYPE(storage_depth), rgb1, adafruitgfxlayer_options> layer_name(layer_name##Bitmap, width, height, ROUND_UP_TO_MULTIPLE_OF_8(width), ROUND_UP_TO_MULTIPLE_OF_8(height))  

        #define SMARTMATRIX_ALLOCATE_GFX_MONO_LAYER(layer_name, width, height, layerwidth, layerheight, storage_depth, adafruitgfxlayer_options) \
            typedef RGB_TYPE(storage_depth) SM_RGB;                                                                 \
            static uint8_t layer_name##Bitmap[2 * ROUND_UP_TO_MULTIPLE_OF_8(layerwidth) * (ROUND_UP_TO_MULTIPLE_OF_8(layerheight) / 8)];                                              \
            static SMLayerGFXMono<RGB_TYPE(storage_depth), rgb1, adafruitgfxlayer_options> layer_name(layer_name##Bitmap, width, height, ROUND_UP_TO_MULTIPLE_OF_8(layerwidth), ROUND_UP_TO_MULTIPLE_OF_8(layerheight))  
#else
        #define SMARTMATRIX_ALLOCATE_BACKGROUND_LAYER(layer_name, width, height, storage_depth, background_options) \
            typedef RGB_TYPE(storage_depth) SM_RGB;                                                                 \
            static BACKGROUND_MEMSECTION RGB_TYPE(storage_depth) layer_name##Bitmap[2*width*height];                                        \
            static color_chan_t layer_name##colorCorrectionLUT[sizeof(SM_RGB) <= 3 ? 256 : 4096];                          \
            static SMLayerBackground<RGB_TYPE(storage_depth), background_options> layer_name(layer_name##Bitmap, width, height, layer_name##colorCorrectionLUT)  

        #define SMARTMATRIX_ALLOCATE_SCROLLING_LAYER(layer_name, width, height, storage_depth, scrolling_options) \
            typedef RGB_TYPE(storage_depth) SM_RGB;                                                                 \
            static uint8_t layer_name##Bitmap[width * (height / 8)];                                              \
            static SMLayerScrolling<RGB_TYPE(storage_depth), scrolling_options> layer_name(layer_name##Bitmap, width, height)  

        #define SMARTMATRIX_ALLOCATE_INDEXED_LAYER(layer_name, width, height, storage_depth, indexed_options) \
            typedef RGB_TYPE(storage_depth) SM_RGB;                                                                 \
            static uint8_t layer_name##Bitmap[2 * width * (height / 8)];                                              \
            static SMLayerIndexed<RGB_TYPE(storage_depth), indexed_options> layer_name(layer_name##Bitmap, width, height)  
#endif
#endif

#if defined(ESP32)
    #define SMARTMATRIX_ALLOCATE_BUFFERS(matrix_name, width, height, pwm_depth, buffer_rows, panel_type, option_flags) \
        SmartMatrixHub75Refresh<pwm_depth, width, height, panel_type, option_flags> matrix_name##Refresh; \
        SmartMatrixHub75Calc<pwm_depth, width, height, panel_type, option_flags> matrix_name

    #define SMARTMATRIX_ALLOCATE_BUFFERS_NT(matrix_name, width, height, pwm_depth, buffer_rows, panel_type, option_flags) \
        static SmartMatrixHub75Refresh_NT<0> matrix_name##Refresh(width, height, pwm_depth, panel_type, option_flags); \
        static SmartMatrixHub75Calc_NT<0> matrix_name(&matrix_name##Refresh, width, height, pwm_depth, panel_type, option_flags)

#ifdef USE_ADAFRUIT_GFX_LAYERS
    #define SMARTMATRIX_ALLOCATE_BACKGROUND_LAYER(layer_name, width, height, storage_depth, background_options) \
        typedef RGB_TYPE(storage_depth) SM_RGB;                                                                 \
        static SMLayerBackgroundGFX<RGB_TYPE(storage_depth), background_options> layer_name(width, height)  

    #define SMARTMATRIX_ALLOCATE_SCROLLING_LAYER(layer_name, width, height, storage_depth, adafruitgfxlayer_options) \
        typedef RGB_TYPE(storage_depth) SM_RGB;                                                                 \
        static SMLayerGFXMono<RGB_TYPE(storage_depth), rgb1, adafruitgfxlayer_options> layer_name(width, height, ROUND_UP_TO_MULTIPLE_OF_8(width), ROUND_UP_TO_MULTIPLE_OF_8(height))  

    #define SMARTMATRIX_ALLOCATE_INDEXED_LAYER(layer_name, width, height, storage_depth, adafruitgfxlayer_options) \
        typedef RGB_TYPE(storage_depth) SM_RGB;                                                                 \
        static SMLayerGFXMono<RGB_TYPE(storage_depth), rgb1, adafruitgfxlayer_options> layer_name(width, height, ROUND_UP_TO_MULTIPLE_OF_8(width), ROUND_UP_TO_MULTIPLE_OF_8(height))  

    #define SMARTMATRIX_ALLOCATE_GFX_MONO_LAYER(layer_name, width, height, layerwidth, layerheight, storage_depth, adafruitgfxlayer_options) \
        typedef RGB_TYPE(storage_depth) SM_RGB;                                                                 \
        static uint8_t layer_name##Bitmap[2 * ROUND_UP_TO_MULTIPLE_OF_8(layerwidth) * (ROUND_UP_TO_MULTIPLE_OF_8(layerheight) / 8)];                                              \
        static SMLayerGFXMono<RGB_TYPE(storage_depth), rgb1, adafruitgfxlayer_options> layer_name(width, height, ROUND_UP_TO_MULTIPLE_OF_8(layerwidth), ROUND_UP_TO_MULTIPLE_OF_8(layerheight))  
#else
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
#endif

// platform-specific
#if defined(__arm__) && defined(CORE_TEENSY) && !defined(__IMXRT1062__)  // Teensy 3.x
    #include "MatrixTeensy3Hub75Refresh_Impl.h"
    #include "MatrixTeensy3Apa102Refresh_Impl.h"
    #include "MatrixTeensy3Hub75Calc_Impl.h"
    #include "MatrixCommonApa102Calc_Impl.h"
#endif

#if defined(__IMXRT1062__) // Teensy 4.0/4.1
    #include "MatrixTeensy4Hub75Refresh_Impl.h"
    #include "MatrixTeensy4Hub75Calc_Impl.h"
    #include "MatrixTeensy4Apa102Refresh_Impl.h"
    #include "MatrixCommonApa102Calc_Impl.h"
#endif

#if defined(ESP32)
    #include "MatrixEsp32Hub75Refresh_Impl.h"
    #include "MatrixEsp32Hub75Calc_Impl.h"
    #include "MatrixEsp32Hub75Refresh_NT_Impl.h"
    #include "MatrixEsp32Hub75Calc_NT_Impl.h"
#endif

#endif
