/*
 * SmartMatrix Library - Multiplexed Panel Refresh Class
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

#ifndef SmartMatrixMultiplexedRefresh_h
#define SmartMatrixMultiplexedRefresh_h

#include "esp32_i2s_parallel.h"

#define CLKS_DURING_LATCH   2
#define ESP32_NUM_FRAME_BUFFERS   2

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
class SmartMatrix3RefreshMultiplexed {
public:
    struct rowBitStruct {
        uint16_t data[((matrixWidth * matrixHeight) / CONVERT_PANELTYPE_TO_MATRIXPANELHEIGHT(panelType)) + CLKS_DURING_LATCH];
    };

    struct frameBitStruct {
        rowBitStruct rowbits[ROWS_PER_FRAME];
    };

    struct frameStruct {
        frameBitStruct framebits[COLOR_DEPTH_BITS];
    };

    typedef void (*matrix_calc_callback)(void);

    // init
    SmartMatrix3RefreshMultiplexed(frameStruct * frameBuffer);
    static void begin(void);

    // refresh API
    static frameStruct * getNextFrameBufferPtr(void);
    static void writeFrameBuffer(uint8_t currentFrame);
    static void recoverFromDmaUnderrun(void);
    static bool isFrameBufferFree(void);
    static void setRefreshRate(uint8_t newRefreshRate);
    static void setBrightness(uint8_t newBrightness);
    static void setMatrixCalculationsCallback(matrix_calc_callback f);

private:
    // enable ISR access to private member variables
    template <int refreshDepth1, int matrixWidth1, int matrixHeight1, unsigned char panelType1, unsigned char optionFlags1>
    friend void frameCalculationISR(void);

    template <int refreshDepth1, int matrixWidth1, int matrixHeight1, unsigned char panelType1, unsigned char optionFlags1>
    friend void frameShiftCompleteISR(void);    

    static int dimmingFactor;
    static const int dimmingMaximum = 255;
    static uint8_t refreshRate;
    static frameStruct * matrixUpdateFrames;

    static matrix_calc_callback matrixCalcCallback;

    static CircularBuffer dmaBuffer;

    static i2s_parallel_buffer_desc_t bufdesc[ESP32_NUM_FRAME_BUFFERS][1<<(COLOR_DEPTH_BITS)];
};

#endif
