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

#ifndef SmartMatrixMultiplexedRefresh_NT_h
#define SmartMatrixMultiplexedRefresh_NT_h

#include "esp32_i2s_parallel.h"

#define ESP32_NUM_FRAME_BUFFERS   2

#define SIZE_OF_ROWBITSTRUCT ((PIXELS_PER_LATCH + CLKS_DURING_LATCH))
#define SIZE_OF_ROWDATASTRUCT (SIZE_OF_ROWBITSTRUCT * COLOR_DEPTH_BITS)
#define SIZE_OF_FRAMESTRUCT (SIZE_OF_ROWDATASTRUCT * MATRIX_SCAN_MOD)

#define GET_DATA_OFFSET_FROM_ROW_AND_COLOR_DEPTH_BIT(row, bit) ((row * SIZE_OF_ROWDATASTRUCT) + (bit * SIZE_OF_ROWBITSTRUCT))

class SmartMatrix3RefreshMultiplexed_NT {
public:
    typedef void (*matrix_calc_callback)(void);

    // init
    SmartMatrix3RefreshMultiplexed_NT(int width, int height, unsigned char depth, unsigned char type, unsigned char options);
    static void begin(uint32_t dmaRamToKeepFreeBytes = 0);

    // refresh API
    static MATRIX_DATA_STORAGE_TYPE * getNextFrameBufferPtr(void);
    static void writeFrameBuffer(uint8_t currentFrame);
    static void recoverFromDmaUnderrun(void);
    static bool isFrameBufferFree(void);
    static void setRefreshRate(uint16_t newRefreshRate);
    static uint16_t getRefreshRate(void);
    static void setBrightness(uint8_t newBrightness);
    static void setMatrixCalculationsCallback(matrix_calc_callback f);
    static void markRefreshComplete(void);
    static uint8_t getLsbMsbTransitionBit(void);

private:
    static uint16_t refreshRate;
    static uint16_t minRefreshRate;
    static uint8_t lsbMsbTransitionBit;
    static MATRIX_DATA_STORAGE_TYPE * matrixUpdateFrames[ESP32_NUM_FRAME_BUFFERS];

    static matrix_calc_callback matrixCalcCallback;

    static CircularBuffer_SM dmaBuffer;

    static uint32_t optionFlags;
    static uint8_t panelType;
    static int matrixWidth;
    static int matrixHeight;
    static uint8_t refreshDepth;
};

#endif
