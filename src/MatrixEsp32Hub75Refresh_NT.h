/*
 * SmartMatrix Library - ESP32 HUB75 Panel Refresh Class (No Template Version)
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

#ifndef SmartMatrixHUB75Refresh_NT_h
#define SmartMatrixHUB75Refresh_NT_h

#include "esp32_i2s_parallel.h"

#define ESP32_NUM_FRAME_BUFFERS   2

// note "SIZE_OF" refers to size in bytes, equivilent to sizeof(rowbitStruct), etc
#define SIZE_OF_ROWBITSTRUCT ((PIXELS_PER_LATCH + CLKS_DURING_LATCH) * sizeof(MATRIX_DATA_STORAGE_TYPE))
#define SIZE_OF_ROWDATASTRUCT (SIZE_OF_ROWBITSTRUCT * COLOR_DEPTH_BITS)
#define SIZE_OF_FRAMESTRUCT (SIZE_OF_ROWDATASTRUCT * MATRIX_SCAN_MOD)

#define GET_DATA_OFFSET_FROM_ROW_AND_COLOR_DEPTH_BIT(row, bit) ((row * SIZE_OF_ROWDATASTRUCT / sizeof(MATRIX_DATA_STORAGE_TYPE)) + (bit * SIZE_OF_ROWBITSTRUCT / sizeof(MATRIX_DATA_STORAGE_TYPE)))

// use a dummy template, as a way to allow class to be defined in header and not separate .cpp file (to help manage #include matrixhardware* files)
template <int dummyvar>
class SmartMatrixHub75Refresh_NT {
public:
    typedef void (*matrix_calc_callback)(void);

    // init
    SmartMatrixHub75Refresh_NT(uint16_t width, uint16_t height, uint8_t depth, uint8_t type, uint32_t options) :
        matrixWidth(width), matrixHeight(height), optionFlags(options), panelType(type), refreshDepth(depth) {
            // load parameter defaults
            refreshRate = 120;
            minRefreshRate = 120;
            lsbMsbTransitionBit = 0;
        };
    void begin(uint32_t dmaRamToKeepFreeBytes = 0);

    // refresh API
    MATRIX_DATA_STORAGE_TYPE * getNextFrameBufferPtr(void);
    void writeFrameBuffer(uint8_t currentFrame);
    void recoverFromDmaUnderrun(void);
    bool isFrameBufferFree(void);
    void setRefreshRate(uint16_t newRefreshRate);
    uint16_t getRefreshRate(void);
    void setBrightness(uint8_t newBrightness);
    void setMatrixCalculationsCallback(matrix_calc_callback f);
    void markRefreshComplete(void);
    uint8_t getLsbMsbTransitionBit(void);

private:
    uint16_t refreshRate;
    uint16_t minRefreshRate;
    uint8_t lsbMsbTransitionBit;
    MATRIX_DATA_STORAGE_TYPE * matrixUpdateFrames[ESP32_NUM_FRAME_BUFFERS];

    matrix_calc_callback matrixCalcCallback;

    CircularBuffer_SM dmaBuffer;

    const uint32_t optionFlags;
    const uint8_t panelType;
    const uint16_t matrixWidth;
    const uint16_t matrixHeight;
    const uint8_t refreshDepth;
};

#endif
