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

#include "SmartMatrix.h"
#include "DMAChannel.h"

#define INLINE __attribute__( ( always_inline ) ) inline

#ifndef FLASHMEM
#define FLASHMEM // for compatibility with old versions of Teensyduino
#endif

#define LATCH_TIMER_PRESCALE            1 // if min refresh rate is too high, increase this value 
#define TIMER_FREQUENCY                 (F_BUS_ACTUAL>>(LATCH_TIMER_PRESCALE))
#define NS_TO_TICKS(X)                  (uint32_t)(TIMER_FREQUENCY * ((X) / 1000000000.0))

#define LATCH_TIMER_PULSE_WIDTH_TICKS   NS_TO_TICKS(LATCH_TIMER_PULSE_WIDTH_NS)
#define TICKS_PER_ROW                   ((TIMER_FREQUENCY)/refreshRate/(MATRIX_SCAN_MOD))
#define IDEAL_MSB_BLOCK_TICKS           (TICKS_PER_ROW/2) * (1<<LATCHES_PER_ROW) / ((1<<LATCHES_PER_ROW) - 1)
#define MIN_BLOCK_PERIOD_NS             (LATCH_TO_CLK_DELAY_NS + ((PANEL_32_PIXELDATA_TRANSFER_MAXIMUM_NS*(PAD_PIXELS+PIXELS_PER_LATCH))/32))

#define MIN_BLOCK_PERIOD_TICKS          (NS_TO_TICKS(MIN_BLOCK_PERIOD_NS))
//#define MSB_BLOCK_TICKS_ADJUSTMENT_INCREMENT    10
#define MSB_BLOCK_TICKS_ADJUSTMENT_INCREMENT  (TICKS_PER_ROW/512)
#define MIN_REFRESH_RATE                (((TIMER_FREQUENCY)/65535*(1<<LATCHES_PER_ROW)/((1<<LATCHES_PER_ROW) - 1)/(MATRIX_SCAN_MOD)/2) + 1) // cannot refresh slower than this due to PWM register overflow
#define MAX_REFRESH_RATE                ((TIMER_FREQUENCY)/(MIN_BLOCK_PERIOD_TICKS)/(MATRIX_SCAN_MOD)/(LATCHES_PER_ROW) - 1) // cannot refresh faster than this due to output bandwidth

#define ROW_CALCULATION_ISR_PRIORITY    240 // lowest priority for IMXRT1062
#define ROW_SHIFT_COMPLETE_ISR_PRIORITY 96 // one step above USB priority
#define TIMER_REGISTERS_TO_UPDATE       2


extern DMAChannel dmaClockOutData;
extern DMAChannel dmaEnable;
extern DMAChannel dmaUpdateTimer;

// FlexPWM pin/channel structure (from cores/pwm.c)
struct pwm_pin_info_struct {
    uint8_t type;
    uint8_t module;
    uint8_t channel;
    uint8_t muxval;
};
extern "C" const struct pwm_pin_info_struct pwm_pin_info[];


template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
void rowShiftCompleteISR(void);
template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
void rowCalculationISR(void);
template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
CircularBuffer_SM SmartMatrixRefreshT4<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::dmaBuffer;
// dmaBufferNumRows = the size of the buffer that DMA pulls from to refresh the display
// must be minimum 2 rows so one can be updated while the other is refreshed
// increase beyond two to give more time for the update routine to complete
// (increase this number if non-DMA interrupts are causing display problems)
template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
uint8_t SmartMatrixRefreshT4<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::dmaBufferNumRows;
template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
uint16_t SmartMatrixRefreshT4<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::refreshRate = 240;
template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
uint16_t SmartMatrixRefreshT4<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::rowBitStructBytesToShift;
template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
typename SmartMatrixRefreshT4<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::timerpair SmartMatrixRefreshT4<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::timerLUT[LATCHES_PER_ROW];
template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
DMAMEM typename SmartMatrixRefreshT4<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::timerpair SmartMatrixRefreshT4<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::timerPairIdle;
template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
volatile typename SmartMatrixRefreshT4<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::rowDataStruct * SmartMatrixRefreshT4<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::matrixUpdateRows;
template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
typename SmartMatrixRefreshT4<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::matrix_underrun_callback SmartMatrixRefreshT4<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::matrixUnderrunCallback;
template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
typename SmartMatrixRefreshT4<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::matrix_calc_callback SmartMatrixRefreshT4<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::matrixCalcCallback;
template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
IMXRT_FLEXPWM_t * SmartMatrixRefreshT4<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::flexpwm;
template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
uint8_t SmartMatrixRefreshT4<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::submodule;
template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
int SmartMatrixRefreshT4<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::dimmingFactor = dimmingMaximum - (100 * 255) / 100;
template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
volatile uint8_t DMAMEM SmartMatrixRefreshT4<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::enablerSourceByte;
template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
typename SmartMatrixRefreshT4<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::flexPinConfigStruct SmartMatrixRefreshT4<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::flexPinConfig;
template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
typename SmartMatrixRefreshT4<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::flexPinConfigStruct SmartMatrixRefreshT4<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::addxPinConfig;
template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
IMXRT_FLEXIO_t * SmartMatrixRefreshT4<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::flexIO;


template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
FLASHMEM SmartMatrixRefreshT4<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::SmartMatrixRefreshT4(uint8_t bufferrows, volatile rowDataStruct * rowDataBuf) {
    dmaBufferNumRows = bufferrows;
    matrixUpdateRows = rowDataBuf;
    timerPairIdle.timer_period = MIN_BLOCK_PERIOD_TICKS;
    timerPairIdle.timer_oe = MIN_BLOCK_PERIOD_TICKS + 1;
    arm_dcache_flush((void*)&timerPairIdle, sizeof(timerPairIdle));

    // initialize matrixUpdateRows to all zeros to ensure all padding pixels are blank
    for (int row = 0; row < dmaBufferNumRows; row++) {
        memset((void*) &matrixUpdateRows[row], 0, sizeof(rowDataStruct));
        arm_dcache_flush((void*) &matrixUpdateRows[row], sizeof(rowDataStruct));
    }
}


template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
FASTRUN bool SmartMatrixRefreshT4<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::isRowBufferFree(void) {
    if (cbIsFull(&dmaBuffer))
        return false;
    else
        return true;
}


template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
FASTRUN INLINE volatile typename SmartMatrixRefreshT4<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::rowDataStruct * SmartMatrixRefreshT4<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::getNextRowBufferPtr(void) {
    return &(matrixUpdateRows[cbGetNextWrite(&dmaBuffer)]);
}


template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
FASTRUN INLINE void SmartMatrixRefreshT4<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::writeRowBuffer(uint8_t currentRow) {
    volatile SmartMatrixRefreshT4<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::rowDataStruct * currentRowDataPtr = SmartMatrixRefreshT4<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::getNextRowBufferPtr();
    for (int i = 0; i < LATCHES_PER_ROW; i++) {
        currentRowDataPtr->rowbits[i].timerValues.timer_period = timerLUT[i].timer_period;
        currentRowDataPtr->rowbits[i].timerValues.timer_oe = timerLUT[i].timer_oe;
    }
    // Now we have refreshed the rowDataStruct for this row and we need to flush cache so that the changes are seen by DMA
    arm_dcache_flush((void*) currentRowDataPtr, sizeof(rowDataStruct));
    cbWrite(&dmaBuffer); // after cache is flushed, mark this row as ready to be displayed
}


template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
FASTRUN void SmartMatrixRefreshT4<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::recoverFromDmaUnderrun(void) {
    // stop timer
    flexpwm->MCTRL &= ~FLEXPWM_MCTRL_RUN(1 << submodule);

    // point DMA addresses to the next buffer
    int currentRow = cbGetNextRead(&dmaBuffer);

    dmaUpdateTimer.TCD->SADDR = &(SmartMatrixRefreshT4<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::matrixUpdateRows[currentRow].rowbits[0].timerValues.timer_oe);
    dmaClockOutData.TCD->SADDR = SmartMatrixRefreshT4<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::matrixUpdateRows[currentRow].rowbits[0].data;

    // enable channel-to-channel linking so data will be shifted out
    dmaUpdateTimer.TCD->CSR &= ~DMA_TCD_CSR_DONE; // must clear DONE flag before enabling
    dmaUpdateTimer.TCD->CSR |= DMA_TCD_CSR_MAJORELINK;

    // set timer increment back to read from matrixUpdateRows
    dmaUpdateTimer.TCD->SLAST = sizeof(matrixUpdateRows[0].rowbits[0]) - sizeof(timerpair);

    // start timer again - next timer period is MIN_BLOCK_PERIOD_TICKS with OE disabled, period after that will be loaded from matrixUpdateBlock
    flexpwm->MCTRL |= FLEXPWM_MCTRL_RUN(1 << submodule);
}


template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
void SmartMatrixRefreshT4<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::setMatrixCalculationsCallback(matrix_calc_callback f) {
    matrixCalcCallback = f;
}


template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
void SmartMatrixRefreshT4<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::setMatrixUnderrunCallback(matrix_underrun_callback f) {
    matrixUnderrunCallback = f;
}


template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
void SmartMatrixRefreshT4<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::calculateTimerLUT(void) {
    int i;
    uint32_t ticksUsed;
    uint16_t msbBlockTicks = IDEAL_MSB_BLOCK_TICKS + MSB_BLOCK_TICKS_ADJUSTMENT_INCREMENT;

    // start with ideal width of the MSB, and keep lowering until the width of all bits fits within TICKS_PER_ROW
    do {
        ticksUsed = 0;
        msbBlockTicks -= MSB_BLOCK_TICKS_ADJUSTMENT_INCREMENT;
        for (i = 0; i < LATCHES_PER_ROW; i++) {
            uint16_t blockTicks = (msbBlockTicks >> (LATCHES_PER_ROW - i - 1)) + LATCH_TIMER_PULSE_WIDTH_TICKS;
            if (blockTicks < MIN_BLOCK_PERIOD_TICKS)
                blockTicks = MIN_BLOCK_PERIOD_TICKS;
            ticksUsed += blockTicks;
        }
    } while (ticksUsed > TICKS_PER_ROW);

    for (i = 0; i < LATCHES_PER_ROW; i++) {
        // set period and OE values for current block - going from smallest timer values to largest
        // order needs to be smallest to largest so the last update of the row has the largest time between
        // the falling edge of the latch and the rising edge of the latch on the next row - an ISR
        // updates the row in this time

        // period is max on time for this block, plus the dead time while the latch is high
        uint16_t period = (msbBlockTicks >> (LATCHES_PER_ROW - i - 1)) + LATCH_TIMER_PULSE_WIDTH_TICKS;
        // on-time is the max on-time * dimming factor, plus the dead time while the latch is high
        uint16_t ontime = (((msbBlockTicks >> (LATCHES_PER_ROW - i - 1)) * dimmingFactor) / dimmingMaximum) + LATCH_TIMER_PULSE_WIDTH_TICKS;

        if (period < MIN_BLOCK_PERIOD_TICKS) {
            uint16_t padding = (MIN_BLOCK_PERIOD_TICKS) - period; // padding is necessary to allow enough time for data to output to the display
            period += padding;
            ontime += padding; // by adding the same padding to the "ontime", the observed intensity is not affected and is still correct
        }

        timerLUT[i].timer_period = period - 1;
        timerLUT[i].timer_oe = ontime;
    }

#if 0
    // print look-up table (for debugging)
    Serial.print("Refresh rate: "); Serial.print(refreshRate); Serial.print(" (Min/Max: "); Serial.print(MIN_REFRESH_RATE); Serial.print("/"); Serial.print(MAX_REFRESH_RATE); Serial.println(")");
    Serial.print("Max brightness limited to "); Serial.print(msbBlockTicks * (200 - (200 >> LATCHES_PER_ROW)) / TICKS_PER_ROW); Serial.println("%");
    for (i = 0; i < LATCHES_PER_ROW; i++) {
        Serial.print("bitplane "); Serial.print(i);
        Serial.print(": period: "); Serial.print(timerLUT[i].timer_period + 1);
        Serial.print(": ontime: "); Serial.println(timerLUT[i].timer_oe);
    }
#endif
}


template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
void SmartMatrixRefreshT4<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::setBrightness(uint8_t newBrightness) {
    dimmingFactor = dimmingMaximum - newBrightness;
    calculateTimerLUT();
}


template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
void SmartMatrixRefreshT4<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::setRefreshRate(uint16_t newRefreshRate) {
    if (newRefreshRate <= MIN_REFRESH_RATE)
        refreshRate = MIN_REFRESH_RATE;
    else if (newRefreshRate >= MAX_REFRESH_RATE)
        refreshRate = MAX_REFRESH_RATE;
    else
        refreshRate = newRefreshRate;
    calculateTimerLUT();
}


template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
FLASHMEM void SmartMatrixRefreshT4<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::begin(void) {
    cbInit(&dmaBuffer, dmaBufferNumRows);

    // set refresh rate and fill timerLUT
    setRefreshRate(refreshRate);

    // completely fill buffer with data before enabling DMA
    matrixCalcCallback(true);

    uint8_t selected_clk_pin = FLEXIO_PIN_CLK_TEENSY_PIN;
    if (optionFlags & SMARTMATRIX_OPTIONS_T4_CLK_PIN_ALT) {
        selected_clk_pin = FLEXIO_PIN_CLK_TEENSY_PIN_ALT;
    }

    // configure 7 output pins and 2 PWM pins
    pinMode(selected_clk_pin, OUTPUT);
    pinMode(FLEXIO_PIN_B0_TEENSY_PIN, OUTPUT);
    pinMode(FLEXIO_PIN_R0_TEENSY_PIN, OUTPUT);
    pinMode(FLEXIO_PIN_R1_TEENSY_PIN, OUTPUT);
    pinMode(FLEXIO_PIN_G0_TEENSY_PIN, OUTPUT);
    pinMode(FLEXIO_PIN_G1_TEENSY_PIN, OUTPUT);
    pinMode(FLEXIO_PIN_B1_TEENSY_PIN, OUTPUT);
    pinMode(FLEXPWM_PIN_OE_TEENSY_PIN, OUTPUT);
    pinMode(FLEXPWM_PIN_LATCH_TEENSY_PIN, OUTPUT);

#ifdef T4_RESERVED_PINS_SMARTLED_SHIELD_V4
    // these pins cannot be used as output because they are tied to other pins using jumper wires
    pinMode(T4_RESERVED_LATCH_TEENSY_PIN, INPUT);
    pinMode(T4_RESERVED_CLK_TEENSY_PIN, INPUT);
    pinMode(T4_RESERVED_R0_TEENSY_PIN, INPUT);
    pinMode(T4_RESERVED_R1_TEENSY_PIN, INPUT);
    pinMode(T4_RESERVED_G0_TEENSY_PIN, INPUT);
    pinMode(T4_RESERVED_B1_TEENSY_PIN, INPUT);
#endif

    // high speed and drive strength configuration
    *(portControlRegister(selected_clk_pin)) = 0xFF;
    *(portControlRegister(FLEXIO_PIN_B0_TEENSY_PIN)) = 0xFF;
    *(portControlRegister(FLEXIO_PIN_R0_TEENSY_PIN)) = 0xFF;
    *(portControlRegister(FLEXIO_PIN_R1_TEENSY_PIN)) = 0xFF;
    *(portControlRegister(FLEXIO_PIN_G0_TEENSY_PIN)) = 0xFF;
    *(portControlRegister(FLEXIO_PIN_G1_TEENSY_PIN)) = 0xFF;
    *(portControlRegister(FLEXIO_PIN_B1_TEENSY_PIN)) = 0xFF;
    *(portControlRegister(FLEXPWM_PIN_OE_TEENSY_PIN)) = 0xFF;
    *(portControlRegister(FLEXPWM_PIN_LATCH_TEENSY_PIN)) = 0xFF;

    // send FM6126A chipset reset sequence, which is ignored by other chipsets that don't need it
    // Thanks to Bob Davis: http://bobdavis321.blogspot.com/2019/02/p3-64x32-hub75e-led-matrix-panels-with.html
    if (optionFlags & SMARTMATRIX_OPTIONS_FM6126A_RESET_AT_START) {
        // TODO: any harm in sending a longer sequence to cover a possible wider case?
        int maxLeds = 256;
        int C12[16] = {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
        int C13[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0};

        // keep display off
        digitalWriteFast(FLEXPWM_PIN_OE_TEENSY_PIN, HIGH);

        // set CLK/LAT to idle state
        digitalWriteFast(FLEXPWM_PIN_LATCH_TEENSY_PIN, LOW);
        digitalWriteFast(selected_clk_pin, LOW);
        delay(1);

        // Send Data to control register 11
        for (int i = 0; i < maxLeds; i++) {
            int y = i % 16;
            digitalWriteFast(FLEXIO_PIN_B0_TEENSY_PIN, LOW);
            digitalWriteFast(FLEXIO_PIN_R0_TEENSY_PIN, LOW);
            digitalWriteFast(FLEXIO_PIN_R1_TEENSY_PIN, LOW);
            digitalWriteFast(FLEXIO_PIN_G0_TEENSY_PIN, LOW);
            digitalWriteFast(FLEXIO_PIN_G1_TEENSY_PIN, LOW);
            digitalWriteFast(FLEXIO_PIN_B1_TEENSY_PIN, LOW);

            if (C12[y] == 1) {
                digitalWriteFast(FLEXIO_PIN_B0_TEENSY_PIN, HIGH);
                digitalWriteFast(FLEXIO_PIN_R0_TEENSY_PIN, HIGH);
                digitalWriteFast(FLEXIO_PIN_R1_TEENSY_PIN, HIGH);
                digitalWriteFast(FLEXIO_PIN_G0_TEENSY_PIN, HIGH);
                digitalWriteFast(FLEXIO_PIN_G1_TEENSY_PIN, HIGH);
                digitalWriteFast(FLEXIO_PIN_B1_TEENSY_PIN, HIGH);
            }

            if (i > maxLeds - 12)
                digitalWriteFast(FLEXPWM_PIN_LATCH_TEENSY_PIN, HIGH);
            else
                digitalWriteFast(FLEXPWM_PIN_LATCH_TEENSY_PIN, LOW);

            digitalWriteFast(selected_clk_pin, HIGH);
            delay(1);
            digitalWriteFast(selected_clk_pin, LOW);
            delay(1);
        }

        digitalWriteFast(FLEXPWM_PIN_LATCH_TEENSY_PIN, LOW);

        // Send Data to control register 12
        for (int i = 0; i < maxLeds; i++) {
            int y = i % 16;
            digitalWriteFast(FLEXIO_PIN_B0_TEENSY_PIN, LOW);
            digitalWriteFast(FLEXIO_PIN_R0_TEENSY_PIN, LOW);
            digitalWriteFast(FLEXIO_PIN_R1_TEENSY_PIN, LOW);
            digitalWriteFast(FLEXIO_PIN_G0_TEENSY_PIN, LOW);
            digitalWriteFast(FLEXIO_PIN_G1_TEENSY_PIN, LOW);
            digitalWriteFast(FLEXIO_PIN_B1_TEENSY_PIN, LOW);

            if (C13[y] == 1) {
                digitalWriteFast(FLEXIO_PIN_B0_TEENSY_PIN, HIGH);
                digitalWriteFast(FLEXIO_PIN_R0_TEENSY_PIN, HIGH);
                digitalWriteFast(FLEXIO_PIN_R1_TEENSY_PIN, HIGH);
                digitalWriteFast(FLEXIO_PIN_G0_TEENSY_PIN, HIGH);
                digitalWriteFast(FLEXIO_PIN_G1_TEENSY_PIN, HIGH);
                digitalWriteFast(FLEXIO_PIN_B1_TEENSY_PIN, HIGH);
            }

            if (i > maxLeds - 13)
                digitalWriteFast(FLEXPWM_PIN_LATCH_TEENSY_PIN, HIGH);
            else
                digitalWriteFast(FLEXPWM_PIN_LATCH_TEENSY_PIN, LOW);

            digitalWriteFast(selected_clk_pin, HIGH);
            delay(1);
            digitalWriteFast(selected_clk_pin, LOW);
            delay(1);
        }

        digitalWriteFast(FLEXPWM_PIN_LATCH_TEENSY_PIN, LOW);

    }

    // set up hardware peripherals
    hardwareSetup();

    // configure initial row address to send to the panel
    setRowAddress(cbGetNextRead(&dmaBuffer));

    // at the end after everything is set up: enable FlexPWM timer to start display process
    flexpwm->MCTRL |= FLEXPWM_MCTRL_RUN(1 << submodule);
}


template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
FLASHMEM void SmartMatrixRefreshT4<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::hardwareSetup() {
    /*  Set up FlexIO peripheral for clocking out data to LED matrix panel.

        FlexIO enables parallel output to the panel's RGB data inputs and also generates the clock signal for the panel in hardware.
        For Teensy 3.x, SmartMatrix uses an 8-bit GPIO port to generate RGB and clock signals, but this is not possible or convenient
        on Teensy 4.0/4.1 because the GPIO ports only have 32-bit access.
        FlexIO provides four 32-bit buffered shift registers which can shift 1, 2, 4, 8, 16, or 32 bits in parallel, which can be assigned
        to a group of contiguous pins on one FlexIO peripheral. On Teensy 4.0 and 4.1, we need to use 16 bit shift width to obtain
        a group containing 6 external FlexIO pins. The clock signal can be assigned to a non-contiguous FlexIO pin.

        The process of shifting out display data works (without using the CPU) as follows:
        1. First, a burst of RGB data is transferred by DMA into the FlexIO shifter buffers.
        2. When the buffers receive data, this triggers FlexIO to load the buffers into the shift registers and start clocking out data.
        3. Once the shift registers are loaded from the buffers, this triggers DMA to transfer the next burst of data to the buffers.
        4. The DMA transfer occurs simultaneously as the FlexIO shifters clock out the data.
        5. Once the data is clocked out, FlexIO reloads the shifters and this process repeats seamlessly.
        6. When the DMA channel reaches the end of the RGB data for the current display row, it stops refreshing the shift buffers.
        7. The FlexIO shifters output the last RGB data and then the clock disables.
        8. After the clock stops, FlexIO outputs the address data from an extra shifter without a clock signal. This only occurs
          when the buffers for the RGB data shifters are completely empty. These signals are not clocked into the LED panel but are
          instead used to control the row address latches on the SmartLED shield. */

    uint32_t timerSelect, timerPolarity, pinConfig, pinSelect, pinPolarity, shifterMode, parallelWidth, inputSource, stopBit, startBit,
             triggerSelect, triggerPolarity, triggerSource, timerMode, timerOutput, timerDecrement, timerReset, timerDisable, timerEnable;
    uint8_t clkFlexPin, b0FlexPin, r0FlexPin, r1FlexPin, g0FlexPin, g1FlexPin, b1FlexPin, lowestFlexPin, highestFlexPin;

    uint8_t selected_clk_pin = FLEXIO_PIN_CLK_TEENSY_PIN;
    if (optionFlags & SMARTMATRIX_OPTIONS_T4_CLK_PIN_ALT) {
        selected_clk_pin = FLEXIO_PIN_CLK_TEENSY_PIN_ALT;
    }

    FlexIOHandler * pFlex = FlexIOHandler::mapIOPinToFlexIOHandler(selected_clk_pin, clkFlexPin); // get FlexIO handler
    flexIO = &pFlex->port(); // Pointer to the port structure in the FlexIO channel

    // Configure FlexIO clock source
    // FlexIO clock is independent from CPU and bus clocks (not affected by CPU overclocking)
    pFlex->setClockSettings(3, 0, 0); // 480 MHz PLL3_SW_CLK clock

    // Set up pin muxes
    pFlex->setIOPinToFlexMode(selected_clk_pin);
    pFlex->setIOPinToFlexMode(FLEXIO_PIN_B0_TEENSY_PIN);
    pFlex->setIOPinToFlexMode(FLEXIO_PIN_R0_TEENSY_PIN);
    pFlex->setIOPinToFlexMode(FLEXIO_PIN_R1_TEENSY_PIN);
    pFlex->setIOPinToFlexMode(FLEXIO_PIN_G0_TEENSY_PIN);
    pFlex->setIOPinToFlexMode(FLEXIO_PIN_G1_TEENSY_PIN);
    pFlex->setIOPinToFlexMode(FLEXIO_PIN_B1_TEENSY_PIN);

    // Enable the clock
    pFlex->hardware().clock_gate_register |= pFlex->hardware().clock_gate_mask;

    // Enable FlexIO with fast register access
    flexIO->CTRL = FLEXIO_CTRL_FLEXEN | FLEXIO_CTRL_FASTACC;

    // Determine FlexIO hardware pin numbers
    b0FlexPin = pFlex->mapIOPinToFlexPin(FLEXIO_PIN_B0_TEENSY_PIN);
    r0FlexPin = pFlex->mapIOPinToFlexPin(FLEXIO_PIN_R0_TEENSY_PIN);
    r1FlexPin = pFlex->mapIOPinToFlexPin(FLEXIO_PIN_R1_TEENSY_PIN);
    g0FlexPin = pFlex->mapIOPinToFlexPin(FLEXIO_PIN_G0_TEENSY_PIN);
    g1FlexPin = pFlex->mapIOPinToFlexPin(FLEXIO_PIN_G1_TEENSY_PIN);
    b1FlexPin = pFlex->mapIOPinToFlexPin(FLEXIO_PIN_B1_TEENSY_PIN);
    lowestFlexPin = b0FlexPin;
    lowestFlexPin = min(lowestFlexPin, r0FlexPin);
    lowestFlexPin = min(lowestFlexPin, r1FlexPin);
    lowestFlexPin = min(lowestFlexPin, g0FlexPin);
    lowestFlexPin = min(lowestFlexPin, g1FlexPin);
    lowestFlexPin = min(lowestFlexPin, b1FlexPin);
    highestFlexPin = b0FlexPin;
    highestFlexPin = max(highestFlexPin, r0FlexPin);
    highestFlexPin = max(highestFlexPin, r1FlexPin);
    highestFlexPin = max(highestFlexPin, g0FlexPin);
    highestFlexPin = max(highestFlexPin, g1FlexPin);
    highestFlexPin = max(highestFlexPin, b1FlexPin);

    // To ensure that 16 bit shifting is used, we must use at least 9 contiguous pins (no more than 16)
    highestFlexPin = max(highestFlexPin, lowestFlexPin + 8);

    // Validate that the pin configuration is correct
    const bool correctFlexConfig = (highestFlexPin < lowestFlexPin + 16) & ((clkFlexPin < lowestFlexPin) | (clkFlexPin > highestFlexPin));
    if (!correctFlexConfig) {
        Serial.println("Error: incorrect FlexIO pin configuration!");
    }

    // calculate the bit offsets of the data signals that are output on each pin
    flexPinConfig.r0 = pFlex->mapIOPinToFlexPin(R_0_SIGNAL) - lowestFlexPin;
    flexPinConfig.g0 = pFlex->mapIOPinToFlexPin(G_0_SIGNAL) - lowestFlexPin;
    flexPinConfig.b0 = pFlex->mapIOPinToFlexPin(B_0_SIGNAL) - lowestFlexPin;
    flexPinConfig.r1 = pFlex->mapIOPinToFlexPin(R_1_SIGNAL) - lowestFlexPin;
    flexPinConfig.g1 = pFlex->mapIOPinToFlexPin(G_1_SIGNAL) - lowestFlexPin;
    flexPinConfig.b1 = pFlex->mapIOPinToFlexPin(B_1_SIGNAL) - lowestFlexPin;

    // determine the bit offsets of the address signals
    addxPinConfig.addx0 = pFlex->mapIOPinToFlexPin(ADDX_0_SIGNAL) - lowestFlexPin;
    addxPinConfig.addx1 = pFlex->mapIOPinToFlexPin(ADDX_1_SIGNAL) - lowestFlexPin;
    addxPinConfig.addx2 = pFlex->mapIOPinToFlexPin(ADDX_2_SIGNAL) - lowestFlexPin;
    addxPinConfig.addx3 = pFlex->mapIOPinToFlexPin(ADDX_3_SIGNAL) - lowestFlexPin;
    addxPinConfig.addx4 = pFlex->mapIOPinToFlexPin(ADDX_4_SIGNAL) - lowestFlexPin;

    // Configure FlexIO Shifters for RGB data buffering:
    // Shifter 0 outputs to the external pins and the other shifters output in sequence.
    // The configuration for Shifters 1 through 3 are the same.

    // Shifter 0 registers
    parallelWidth = FLEXIO_SHIFTCFG_PWIDTH(highestFlexPin - lowestFlexPin); // number of pins to output in parallel
    inputSource = FLEXIO_SHIFTCFG_INSRC * (1); // Input source from next shifter
    stopBit = FLEXIO_SHIFTCFG_SSTOP(0); // Stop bit disabled
    startBit = FLEXIO_SHIFTCFG_SSTART(0); // Start bit disabled, transmitter loads data on enable
    timerSelect = FLEXIO_SHIFTCTL_TIMSEL(0); // Use timer 0
    timerPolarity = FLEXIO_SHIFTCTL_TIMPOL * (1); // Shift on negedge of clock
    pinConfig = FLEXIO_SHIFTCTL_PINCFG(3); // Shifter pin output
    pinSelect = FLEXIO_SHIFTCTL_PINSEL(lowestFlexPin); // Select group of pins starting with lowestFlexPin
    pinPolarity = FLEXIO_SHIFTCTL_PINPOL * (0); // Shifter pin active high polarity
    shifterMode = FLEXIO_SHIFTCTL_SMOD(2); // Shifter transmit mode
    flexIO->SHIFTCFG[0] = parallelWidth | inputSource | stopBit | startBit;
    flexIO->SHIFTCTL[0] = timerSelect | timerPolarity | pinConfig | pinSelect | pinPolarity | shifterMode;

    // Shifter 1-3 registers
    parallelWidth = FLEXIO_SHIFTCFG_PWIDTH(highestFlexPin - lowestFlexPin); // number of pins to output in parallel
    inputSource = FLEXIO_SHIFTCFG_INSRC * (1); // Input source from next shifter
    stopBit = FLEXIO_SHIFTCFG_SSTOP(0); // Stop bit disabled
    startBit = FLEXIO_SHIFTCFG_SSTART(0); // Start bit disabled, transmitter loads data on enable
    timerSelect = FLEXIO_SHIFTCTL_TIMSEL(0); // Use timer 0
    timerPolarity = FLEXIO_SHIFTCTL_TIMPOL * (1); // Shift on negedge of clock
    pinConfig = FLEXIO_SHIFTCTL_PINCFG(0); // Shifter pin output disabled
    shifterMode = FLEXIO_SHIFTCTL_SMOD(2); // Shifter transmit mode
    for (int i = 1; i < RGBDATA_SHIFTERS; i++) {
        flexIO->SHIFTCFG[i] = parallelWidth | inputSource | stopBit | startBit;
        flexIO->SHIFTCTL[i] = timerSelect | timerPolarity | pinConfig | shifterMode;
    }

    // Configure last FlexIO Shifter for row addressing (same configuration as previous)
    if (RGBDATA_SHIFTERS < 4) {
        flexIO->SHIFTCFG[RGBDATA_SHIFTERS] = parallelWidth | inputSource | stopBit | startBit;
        flexIO->SHIFTCTL[RGBDATA_SHIFTERS] = timerSelect | timerPolarity | pinConfig | shifterMode;
    } else {
        // SHIFTCFG[4] and SHIFTCTL[4] are above array bounds, since only shifters 0 through 3 are specified in the manual.
        // However, the unused registers at these addresses can be configured as if there was an extra shifter present.
        // This appears to be an undocumented feature or a vestige of a different chip with more FlexIO shifters...
        // The extra shifter might not be full-featured but it works fine to output row address data.
        volatile uint32_t * addxCfg, * addxCtl;
        addxCfg = flexIO->SHIFTCFG + RGBDATA_SHIFTERS;
        addxCtl = flexIO->SHIFTCTL + RGBDATA_SHIFTERS;
        *addxCfg = parallelWidth | inputSource | stopBit | startBit;
        *addxCtl = timerSelect | timerPolarity | pinConfig | shifterMode;
    }

    // Configure FlexIO Timer 0 to generate the LED panel clock signal. This timer also controls the shifters. The
    // timer is triggered each time DMA writes a burst of data into the shifter buffers. The trigger loads the contents of
    // the buffers into Shifters 0 and 1 and the full contents of the two shifters are clocked out as the timer decrements to zero.
    timerOutput = FLEXIO_TIMCFG_TIMOUT(1); // Timer output is logic zero when enabled and is not affected by the Timer reset
    timerDecrement = FLEXIO_TIMCFG_TIMDEC(0); // Timer decrements on FlexIO clock, shift clock equals timer output
    timerReset = FLEXIO_TIMCFG_TIMRST(0); // Timer never reset
    timerDisable = FLEXIO_TIMCFG_TIMDIS(2); // Timer disabled on Timer compare
    timerEnable = FLEXIO_TIMCFG_TIMENA(2); // Timer enabled on Trigger assert
    stopBit = FLEXIO_TIMCFG_TSTOP(0); // Stop bit disabled
    startBit = FLEXIO_TIMCFG_TSTART * (0); // Start bit disabled
    triggerSelect = FLEXIO_TIMCTL_TRGSEL(1 + 4 * (RGBDATA_SHIFTERS - 1)); // Trigger select shifter status flag
    triggerPolarity = FLEXIO_TIMCTL_TRGPOL * (1); // Trigger active low
    triggerSource = FLEXIO_TIMCTL_TRGSRC * (1); // Internal trigger selected
    pinConfig = FLEXIO_TIMCTL_PINCFG(3); // Timer pin output
    pinSelect = FLEXIO_TIMCTL_PINSEL(clkFlexPin); // Select pin to output clock signal
    pinPolarity = FLEXIO_TIMCTL_PINPOL * (0); // Timer pin polarity active high
    timerMode = FLEXIO_TIMCTL_TIMOD(1); // Dual 8-bit counters baud mode
    flexIO->TIMCFG[0] = timerOutput | timerDecrement | timerReset | timerDisable | timerEnable | stopBit | startBit;
    flexIO->TIMCTL[0] = triggerSelect | triggerPolarity | triggerSource | pinConfig | pinSelect | pinPolarity | timerMode;

    // Configure timer compare register:
    // Lower 8 bits configure the FlexIO clock divide ratio to generate the pixel clock
    // Upper 8 bits configure the number of pixel clock cycles to be generated each time the shifters are reloaded
    uint8_t shiftsPerReload = RGBDATA_SHIFTERS * PIXELS_PER_WORD;
    flexIO->TIMCMP[0] = ((shiftsPerReload * 2 - 1) << 8) | ((FLEXIO_CLOCK_DIVIDER / 2 - 1) << 0);

    // Enable DMA trigger when data is loaded into the last data shifter so that reloading will occur automatically
    flexIO->SHIFTSDEN |= (1 << (RGBDATA_SHIFTERS - 1));

    /* Set up FlexPWM to generate synchronized Latch and OE signals. */

    // Get hardware info structs associated with the LATCH and OE pins
    const pwm_pin_info_struct latchPin = pwm_pin_info[FLEXPWM_PIN_LATCH_TEENSY_PIN];
    const pwm_pin_info_struct OEPin = pwm_pin_info[FLEXPWM_PIN_OE_TEENSY_PIN];

    // Validate that the two pins are a complementary PWM pair
    const bool correctPWMConfig = (latchPin.module == OEPin.module) & (((latchPin.channel == 2) & (OEPin.channel == 1)) | ((latchPin.channel == 1) & (OEPin.channel == 2)));
    if (!correctPWMConfig) {
        Serial.println("Error: Latch and OE must be complementary channels from a single FlexPWM submodule!");
    }

    // Get the register structure associated with this PWM module
    switch ((latchPin.module >> 4) & 3) {
        case 0: flexpwm = &IMXRT_FLEXPWM1; break;
        case 1: flexpwm = &IMXRT_FLEXPWM2; break;
        case 2: flexpwm = &IMXRT_FLEXPWM3; break;
        default: flexpwm = &IMXRT_FLEXPWM4;
    }
    submodule = latchPin.module & 0x03;
    uint16_t bitmask = 1 << submodule;

    // stop timer until after setup is complete
    flexpwm->MCTRL &= ~FLEXPWM_MCTRL_RUN(bitmask);

    // set the pin mux for FlexPWM
    *(portConfigRegister(FLEXPWM_PIN_OE_TEENSY_PIN)) = OEPin.muxval;
    *(portConfigRegister(FLEXPWM_PIN_LATCH_TEENSY_PIN)) = latchPin.muxval;

    // check if OE and Latch pins are switched in hardware
    volatile uint16_t * timerRegisterPeriod, * timerRegisterLatch, * timerRegisterOE;
    if ((OEPin.channel == 1) & (latchPin.channel == 2)) {
        timerRegisterPeriod = & (flexpwm->SM[submodule].VAL1);
        timerRegisterOE = & (flexpwm->SM[submodule].VAL3);
        timerRegisterLatch = & (flexpwm->SM[submodule].VAL5);
    } else {
        timerRegisterPeriod = & (flexpwm->SM[submodule].VAL1);
        timerRegisterOE = & (flexpwm->SM[submodule].VAL5);
        timerRegisterLatch = & (flexpwm->SM[submodule].VAL3);
    }

    // Starting PWM period and duty cycle
    int period = MIN_BLOCK_PERIOD_TICKS;
    int dutyCycleOE = MIN_BLOCK_PERIOD_TICKS;
    int dutyCycleLatch = LATCH_TIMER_PULSE_WIDTH_TICKS;

    // set up PWM with initial settings
    flexpwm->MCTRL |= FLEXPWM_MCTRL_CLDOK(bitmask);
    flexpwm->SM[submodule].CTRL = FLEXPWM_SMCTRL_FULL | FLEXPWM_SMCTRL_PRSC(LATCH_TIMER_PRESCALE);
    *timerRegisterPeriod = period;
    *timerRegisterOE = dutyCycleOE;
    *timerRegisterLatch = dutyCycleLatch;
    flexpwm->OUTEN |= FLEXPWM_OUTEN_PWMA_EN(bitmask);
    flexpwm->OUTEN |= FLEXPWM_OUTEN_PWMB_EN(bitmask);
    flexpwm->MCTRL |= FLEXPWM_MCTRL_LDOK(bitmask);

    // Generate a DMA trigger at the beginning of each cycle (when the latch and OE signals go high)
    // This is used to trigger the dmaUpdateTimer to reload a new period and
    // duty cycle into the registers, which take effect on the next cycle.
    flexpwm->SM[submodule].DMAEN |= FLEXPWM_SMDMAEN_VALDE;

    // Determine the DMA trigger signal
    uint8_t pwmDmaTrigger;
    switch ((latchPin.module >> 4) & 3) {
        case 0: pwmDmaTrigger = DMAMUX_SOURCE_FLEXPWM1_WRITE0 + submodule; break;
        case 1: pwmDmaTrigger = DMAMUX_SOURCE_FLEXPWM2_WRITE0 + submodule; break;
        case 2: pwmDmaTrigger = DMAMUX_SOURCE_FLEXPWM3_WRITE0 + submodule; break;
        default: pwmDmaTrigger = DMAMUX_SOURCE_FLEXPWM4_WRITE0 + submodule;
    }

    /* Set up DMA transfers. */

    unsigned int minorLoopBytes, minorLoopIterations, majorLoopBytes, majorLoopIterations;
    int destinationAddressOffset, destinationAddressLastOffset, sourceAddressOffset, sourceAddressLastOffset, minorLoopOffset;
    volatile uint32_t *destinationAddress1, *destinationAddress2, *sourceAddress;

    rowBitStructBytesToShift = sizeof(SmartMatrixRefreshT4<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::matrixUpdateRows[0].rowbits[0].data);

    // allocate DMA channels
    dmaUpdateTimer.begin(false);
    dmaEnable.begin(false);
    dmaClockOutData.begin(false);

    // Disable DMA channels so they don't start transferring yet
    dmaUpdateTimer.disable();
    dmaEnable.disable();
    dmaClockOutData.disable();

    // Set up dmaUpdateTimer.
    // dmaUpdateTimer updates the FlexPWM registers from the timerValues structs within the rowDataStruct.
    // It is triggered by the FlexPWM cycle start (when the latch signal goes high) and subsequently links to the dmaEnable.
    // The source address is set to read from timerValues.timer_oe, then timerValues.timer_period, then move to the next bitplane.
    // The destination address is set to write to the OE duty cycle register, then the period register, then reset.
    minorLoopBytes = TIMER_REGISTERS_TO_UPDATE * sizeof(uint16_t);
    majorLoopIterations = 1;
    sourceAddress = (volatile uint32_t*) & (matrixUpdateRows[0].rowbits[0].timerValues.timer_oe);
    sourceAddressOffset = sizeof(uint16_t); // address offset from timer_oe to timer_period
    sourceAddressLastOffset = -TIMER_REGISTERS_TO_UPDATE * sourceAddressOffset + sizeof(matrixUpdateRows[0].rowbits[0]);
    destinationAddress1 = (volatile uint32_t*) timerRegisterOE;
    destinationAddress2 = (volatile uint32_t*) timerRegisterPeriod;
    destinationAddressOffset = (int)destinationAddress2 - (int)destinationAddress1;
    destinationAddressLastOffset = -TIMER_REGISTERS_TO_UPDATE * destinationAddressOffset;
    dmaUpdateTimer.TCD->SADDR = sourceAddress;
    dmaUpdateTimer.TCD->SOFF = sourceAddressOffset;
    dmaUpdateTimer.TCD->ATTR_SRC = DMA_TCD_ATTR_SIZE_16BIT;
    dmaUpdateTimer.TCD->NBYTES = minorLoopBytes;
    dmaUpdateTimer.TCD->SLAST = sourceAddressLastOffset;
    dmaUpdateTimer.TCD->DADDR = destinationAddress1;
    dmaUpdateTimer.TCD->ATTR_DST = DMA_TCD_ATTR_SIZE_16BIT;
    dmaUpdateTimer.TCD->DOFF = destinationAddressOffset;
    dmaUpdateTimer.TCD->DLASTSGA = destinationAddressLastOffset;
    dmaUpdateTimer.TCD->BITER = majorLoopIterations;
    dmaUpdateTimer.TCD->CITER = majorLoopIterations;
    dmaUpdateTimer.triggerAtHardwareEvent(pwmDmaTrigger);

    // Set up dmaEnable.
    // dmaEnable simply writes a bit to the Set Enable register of dmaClockOutData to enable it so it can respond to its trigger.
    // dmaClockOutData is normally disabled because the trigger is active almost all the time and we don't want it to transfer data continuously.
    // We could link dmaClockOutData to dmaEnable but testing shows that this does not make it any faster.
    enablerSourceByte = DMA_SERQ_SERQ(dmaClockOutData.channel);
    arm_dcache_flush((void*)&enablerSourceByte, sizeof(enablerSourceByte)); // Flush cache because enablerSourceByte is in DMAMEM
    dmaEnable.source(enablerSourceByte);
    dmaEnable.destination(DMA_SERQ);
    dmaEnable.transferCount(LATCHES_PER_ROW); // keep an iteration count to track which bitplane we are dealing with
    dmaEnable.triggerAtCompletionOf(dmaUpdateTimer); // Link dmaEnable to dmaUpdateTimer

    // Set up dmaClockOutData.
    // dmaClockOutData transfers 32-bit words from the rowDataBuffer to the 32-bit FlexIO shifter registers. For improved speed and reduced DMA usage,
    // we configure it to transfer in bursts to fill up all the shifters each time it is triggered. The shifter destination registers are contiguous
    // in memory space. A minor loop offset is used to reset the destination address after each burst.
    DMA_CR |= DMA_CR_EMLM; // Enable minor loop mapping so that we can have a minor loop offset
    minorLoopIterations = RGBDATA_SHIFTERS;
    minorLoopBytes = minorLoopIterations * sizeof(uint32_t);
    majorLoopBytes = rowBitStructBytesToShift;
    majorLoopIterations = majorLoopBytes / minorLoopBytes;
    sourceAddress = (uint32_t*) & (matrixUpdateRows[0].rowbits[0].data[0]);
    sourceAddressOffset = sizeof(uint32_t);
    sourceAddressLastOffset = sizeof(matrixUpdateRows[0].rowbits[0]) - majorLoopBytes; // at completion, move on to next bitplane
    destinationAddress1 = &(flexIO->SHIFTBUF[0]);
    destinationAddressOffset = sizeof(uint32_t);
    minorLoopOffset = -minorLoopIterations * destinationAddressOffset;
    destinationAddressLastOffset = minorLoopOffset;
    dmaClockOutData.TCD->SADDR = sourceAddress;
    dmaClockOutData.TCD->SOFF = sourceAddressOffset;
    dmaClockOutData.TCD->SLAST = sourceAddressLastOffset;
    dmaClockOutData.TCD->ATTR_SRC = DMA_TCD_ATTR_SIZE_32BIT;
    dmaClockOutData.TCD->DADDR = destinationAddress1;
    dmaClockOutData.TCD->DOFF = destinationAddressOffset;
    dmaClockOutData.TCD->ATTR_DST = DMA_TCD_ATTR_SIZE_32BIT;
    dmaClockOutData.TCD->DLASTSGA = destinationAddressLastOffset;
    dmaClockOutData.TCD->NBYTES_MLOFFYES = DMA_TCD_NBYTES_DMLOE | DMA_TCD_NBYTES_MLOFFYES_MLOFF(minorLoopOffset) | DMA_TCD_NBYTES_MLOFFYES_NBYTES(minorLoopBytes);
    dmaClockOutData.TCD->BITER = majorLoopIterations;
    dmaClockOutData.TCD->CITER = majorLoopIterations;
    dmaClockOutData.disableOnCompletion(); // Set dmaClockOutData to automatically disable on completion (we have to enable it using dmaEnable)
    dmaClockOutData.triggerAtHardwareEvent(pFlex->hardware().shifters_dma_channel[RGBDATA_SHIFTERS - 1]); // Use FlexIO Shifter 1 trigger

    // Enable interrupt on completion of DMA transfer. This interrupt is used to update the DMA addresses to point to the next row.
    dmaClockOutData.interruptAtCompletion();
    dmaClockOutData.attachInterrupt(rowShiftCompleteISR<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>);
    NVIC_SET_PRIORITY(IRQ_DMA_CH0 + dmaClockOutData.channel, ROW_SHIFT_COMPLETE_ISR_PRIORITY);
    
    // Borrow the interrupt allocated to dmaUpdateTimer to use as a software interrupt
    // It is triggered manually inside rowShiftCompleteISR - never triggered by dmaUpdateTimer
    NVIC_SET_PRIORITY(IRQ_DMA_CH0 + dmaUpdateTimer.channel, ROW_CALCULATION_ISR_PRIORITY);
    dmaUpdateTimer.attachInterrupt(rowCalculationISR<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>);

    // Enable DMA channels (except dmaClockOutData which stays disabled, or else it would trigger continuously)
    dmaEnable.enable();
    dmaUpdateTimer.enable();
}


// low priority ISR triggered by software interrupt on a DMA channel that doesn't need interrupts otherwise
template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
FASTRUN void rowCalculationISR(void) {
    SmartMatrixRefreshT4<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::matrixCalcCallback(false);
}


template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
FASTRUN void rowShiftCompleteISR(void) {
    /*  This interrupt runs at the completion of dmaClockOutData when a complete bitplane has been output to the panel.
        If we have finished all the bitplanes for this row, it's time to update the source address of the dmaClockOutData to
        point to the next row in the rowDataBuffer. Otherwise, the interrupt does nothing if we are not done with all the bitplanes yet.
        To determine whether all the bitplanes are done, we look at dmaEnable which keeps an iteration count that
        corresponds to the bitplanes. In each cycle, dmaEnable is serviced first, before rowUpdateISR. On the last bitplane,
        CITER (current iteration) = 1. When the transaction finishes, CITER is reset to BITER (beginning iteration).
        So if CITER == BITER that means that the final bitplane was just completed.
        In this case it's time to update dmaClockOutData to the next row.*/

    // clear pending interrupt. If this is at the end of the ISR then it does not clear fast enough and the ISR can be triggered twice.
    dmaClockOutData.clearInterrupt();

    if ((dmaEnable.TCD->CITER) == (dmaEnable.TCD->BITER)) {
        cbRead(&SmartMatrixRefreshT4<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::dmaBuffer);

        if (cbIsEmpty(&SmartMatrixRefreshT4<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::dmaBuffer)) { // underrun
            // point dmaUpdateTimer to repeatedly load from values that set mod to MIN_BLOCK_PERIOD_TICKS and disable OE
            dmaUpdateTimer.TCD->SADDR = &SmartMatrixRefreshT4<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::timerPairIdle;
            // set timer increment to repeat timerPairIdle
            dmaUpdateTimer.TCD->SLAST = -TIMER_REGISTERS_TO_UPDATE * sizeof(uint16_t);
            // disable channel-to-channel linking - don't link dmaClockOutData until buffer is ready
            dmaUpdateTimer.TCD->CSR &= ~DMA_TCD_CSR_MAJORELINK;

            // set flag so other ISR can enable DMA again when data is ready
            SmartMatrixRefreshT4<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::matrixUnderrunCallback();
        } else {
            // get next row to draw to display and update DMA pointers
            int currentRow = cbGetNextRead(&SmartMatrixRefreshT4<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::dmaBuffer);
            dmaClockOutData.TCD->SADDR = SmartMatrixRefreshT4<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::matrixUpdateRows[currentRow].rowbits[0].data;
            dmaUpdateTimer.TCD->SADDR = &(SmartMatrixRefreshT4<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::matrixUpdateRows[currentRow].rowbits[0].timerValues.timer_oe);
            SmartMatrixRefreshT4<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::setRowAddress(currentRow); // change the row address we send to the panel
        }

        // trigger software interrupt to call rowCalculationISR() (DMA channel interrupt used instead of actual softint)
        NVIC_SET_PENDING(IRQ_DMA_CH0 + dmaUpdateTimer.channel);
    } // if the last bitplane was not just completed, do nothing
}


template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
FASTRUN INLINE const typename SmartMatrixRefreshT4<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::flexPinConfigStruct & SmartMatrixRefreshT4<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::getFlexPinConfig(void) {
    return flexPinConfig;
}


template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
FASTRUN void SmartMatrixRefreshT4<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::setRowAddress(unsigned int row) {
    // Row addressing makes use of the same pins that output RGB color data. This is enabled by additional hardware on the SmartLED Shield.
    // The row address signals are latched when the BUFFER_LATCH pin goes high. We need to output the address data without any clock pulses
    // to avoid garbage pixel data. We can do this by putting the address data into a final FlexIO shifter which outputs when the data shifters
    // are emptied (at the end of the row transfer after the DMA channel completes). Only the lower 16 bits will output. */
    unsigned int currentRowAddress = SmartMatrixRefreshT4<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::matrixUpdateRows[row].rowbits[0].rowAddress;

    uint32_t addressData = 0;
    addressData |= (currentRowAddress & 0x01) ? (1 << addxPinConfig.addx0) : 0;
    addressData |= (currentRowAddress & 0x02) ? (1 << addxPinConfig.addx1) : 0;
    addressData |= (currentRowAddress & 0x04) ? (1 << addxPinConfig.addx2) : 0;
    addressData |= (currentRowAddress & 0x08) ? (1 << addxPinConfig.addx3) : 0;
    addressData |= (currentRowAddress & 0x10) ? (1 << addxPinConfig.addx4) : 0;

    volatile uint32_t * addxBuf;
    addxBuf = flexIO->SHIFTBUF + RGBDATA_SHIFTERS;
    *addxBuf = addressData; // load final FlexIO shift register with address data
}
