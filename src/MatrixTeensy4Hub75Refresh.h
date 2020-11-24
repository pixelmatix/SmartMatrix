/*
 * SmartMatrix Library - Teensy 4 HUB75 Panel Refresh Class
 *
 * Copyright (c) 2020 Eric Eason and Louis Beaudoin (Pixelmatix)
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

#ifndef SMARTMATRIXREFRESHT4_H
#define SMARTMATRIXREFRESHT4_H

#include <lib/FlexIO_t4/FlexIO_t4.h> // requires FlexIO_t4 library from https://github.com/KurtE/FlexIO_t4

// Number of 32-bit FlexIO shifters to use for data buffering.
// Larger numbers decrease DMA usage.
// Known working: 1, 2, 3, 4
#define RGBDATA_SHIFTERS                4

// Padding added to the row data struct to ensure it is divided evenly by the FlexIO buffer size plus extra padding for robustness
#define PAD_PIXELS                      (((-PIXELS_PER_LATCH) % SHIFTER_PIXELS + SHIFTER_PIXELS) % SHIFTER_PIXELS + SHIFTER_PIXELS)

#define PIXELS_PER_WORD                 2
#define SHIFTER_PIXELS                  (RGBDATA_SHIFTERS*PIXELS_PER_WORD)

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
class SmartMatrixRefreshT4 {
    public:
        struct __attribute__((packed, aligned(2))) timerpair {
            uint16_t timer_oe;
            uint16_t timer_period;
        };

        struct __attribute__((packed, aligned(4))) rowBitStruct {
            uint16_t data[PAD_PIXELS + PIXELS_PER_LATCH];
            uint32_t rowAddress;
            timerpair timerValues __attribute__((aligned(2)));
        };

        struct rowDataStruct {
            rowBitStruct rowbits[refreshDepth / COLOR_CHANNELS_PER_PIXEL];
        };

        // struct to store bit offsets based on FlexIO hardware pin numbers
        struct flexPinConfigStruct {
            union { uint8_t r0; uint8_t addx0; };
            union { uint8_t g0; uint8_t addx1; };
            union { uint8_t b0; uint8_t addx2; };
            union { uint8_t r1; uint8_t addx3; };
            union { uint8_t g1; uint8_t addx4; };
            uint8_t b1;
        };

        typedef void (*matrix_underrun_callback)(void);
        typedef void (*matrix_calc_callback)(bool initial);

        // init
        SmartMatrixRefreshT4(uint8_t bufferrows, volatile rowDataStruct * rowDataBuf);
        static void begin(void);

        // refresh API
        static volatile rowDataStruct * getNextRowBufferPtr(void);
        static void writeRowBuffer(uint8_t currentRow);
        static void recoverFromDmaUnderrun(void);
        static bool isRowBufferFree(void);
        static void setRefreshRate(uint16_t newRefreshRate);
        static void setBrightness(uint8_t newBrightness);
        static void setMatrixCalculationsCallback(matrix_calc_callback f);
        static void setMatrixUnderrunCallback(matrix_underrun_callback f);
        static const flexPinConfigStruct & getFlexPinConfig(void);
        static void setRowAddress(unsigned int row);

    private:
        // enable ISR access to private member variables
        template <int refreshDepth1, int matrixWidth1, int matrixHeight1, unsigned char panelType1, uint32_t optionFlags1>
        friend void rowShiftCompleteISR(void);
        template <int refreshDepth1, int matrixWidth1, int matrixHeight1, unsigned char panelType1, uint32_t optionFlags1>
        friend void rowCalculationISR(void);

        // init helper function called by begin()
        static void hardwareSetup(void);

        // configuration helper function
        static void calculateTimerLUT(void);

        static int dimmingFactor;
        static const int dimmingMaximum = 255;
        static uint16_t rowBitStructBytesToShift;
        static uint16_t refreshRate;
        static uint8_t dmaBufferNumRows;
        static volatile rowDataStruct * matrixUpdateRows;

        static timerpair timerLUT[LATCHES_PER_ROW];
        static timerpair timerPairIdle;
        static matrix_calc_callback matrixCalcCallback;
        static matrix_underrun_callback matrixUnderrunCallback;

        static CircularBuffer_SM dmaBuffer;

        static IMXRT_FLEXIO_t *flexIO;
        static IMXRT_FLEXPWM_t * flexpwm;
        static uint8_t submodule;
        static volatile uint8_t enablerSourceByte;
        static flexPinConfigStruct flexPinConfig;
        static flexPinConfigStruct addxPinConfig;
};

#endif
