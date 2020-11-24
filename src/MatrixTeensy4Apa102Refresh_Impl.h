/*
 * SmartMatrix Library - Teensy 4 APA102 Panel Refresh Class
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

#include "SmartMatrix.h"
#include <lib/FlexIO_t4/FlexIO_t4.h> // requires FlexIO_t4 library from https://github.com/KurtE/FlexIO_t4
#include <lib/FlexIO_t4/FlexIOSPI.h>

#define TIME_PER_FRAME_US      (1000000/refreshRate)

#define APA_MIN_REFRESH_RATE_HZ 1

// set to lowest priority as it is a very time consuming ISR
#define ROW_CALC_ISR_PRIORITY       240

// set to next lowest priority as it can take 10s of microseconds to complete
#define SHIFT_COMPLETE_ISR_PRIORITY 224

extern IntervalTimer myTimer;
extern EventResponder apa102ShiftCompleteEvent;
extern FlexIOSPI SPIFLEX;

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
void apaRowShiftCompleteISR(void);
template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
void apaRowCalculationISR(void);

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
CircularBuffer_SM SmartMatrixAPA102Refresh<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::dmaBuffer;

// dmaBufferNumRows = the size of the buffer that DMA pulls from to refresh the display
// must be minimum 2 rows so one can be updated while the other is refreshed
// increase beyond two to give more time for the update routine to complete
// (increase this number if non-DMA interrupts are causing display problems)
template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
uint8_t SmartMatrixAPA102Refresh<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::dmaBufferNumRows;

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
uint8_t SmartMatrixAPA102Refresh<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::refreshRate = 60;

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
uint32_t SmartMatrixAPA102Refresh<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::spiClockSpeed = 5000000;

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
uint16_t SmartMatrixAPA102Refresh<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::rowBitStructBytesToShift;

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
typename SmartMatrixAPA102Refresh<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::frameDataStruct * SmartMatrixAPA102Refresh<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::matrixUpdateFrame;

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
SmartMatrixAPA102Refresh<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::SmartMatrixAPA102Refresh(uint8_t bufferrows, frameDataStruct * frameDataBuffer) {
    dmaBufferNumRows = bufferrows;

    matrixUpdateFrame = frameDataBuffer;
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
bool SmartMatrixAPA102Refresh<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::isRowBufferFree(void) {
    if(cbIsFull(&dmaBuffer))
        return false;
    else
        return true;
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
typename SmartMatrixAPA102Refresh<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::frameDataStruct * SmartMatrixAPA102Refresh<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::getNextRowBufferPtr(void) {
    return &(matrixUpdateFrame[cbGetNextWrite(&dmaBuffer)]);
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
void SmartMatrixAPA102Refresh<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::writeRowBuffer(uint8_t currentRow) {
    cbWrite(&dmaBuffer);
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
void SmartMatrixAPA102Refresh<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::recoverFromDmaUnderrun(void) {

}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
typename SmartMatrixAPA102Refresh<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::matrix_underrun_callback SmartMatrixAPA102Refresh<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::matrixUnderrunCallback;

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
typename SmartMatrixAPA102Refresh<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::matrix_calc_callback SmartMatrixAPA102Refresh<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::matrixCalcCallback;

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
void SmartMatrixAPA102Refresh<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::setMatrixCalculationsCallback(matrix_calc_callback f) {
    matrixCalcCallback = f;
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
void SmartMatrixAPA102Refresh<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::setMatrixUnderrunCallback(matrix_underrun_callback f) {
    matrixUnderrunCallback = f;
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
void SmartMatrixAPA102Refresh<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::calculateTimerLUT(void) {

}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
void SmartMatrixAPA102Refresh<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::setRefreshRate(uint8_t newRefreshRate) {
    if(newRefreshRate > APA_MIN_REFRESH_RATE_HZ)
        refreshRate = newRefreshRate;
    else
        refreshRate = APA_MIN_REFRESH_RATE_HZ;

   myTimer.update(TIME_PER_FRAME_US);
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
void SmartMatrixAPA102Refresh<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::setSpiClockSpeed(uint32_t newClockSpeed) {
    spiClockSpeed = newClockSpeed;

    // TODO: update clock speed after begin() called?
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
void SmartMatrixAPA102Refresh<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::begin(void) {
    cbInit(&dmaBuffer, dmaBufferNumRows);

    // setup debug output
#ifdef DEBUG_PINS_ENABLED
    pinMode(DEBUG_PIN_1, OUTPUT);
    digitalWriteFast(DEBUG_PIN_1, HIGH); // oscilloscope trigger
    digitalWriteFast(DEBUG_PIN_1, LOW);
    pinMode(DEBUG_PIN_2, OUTPUT);
    digitalWriteFast(DEBUG_PIN_2, HIGH); // oscilloscope trigger
    digitalWriteFast(DEBUG_PIN_2, LOW);
    pinMode(DEBUG_PIN_3, OUTPUT);
    digitalWriteFast(DEBUG_PIN_3, HIGH); // oscilloscope trigger
    digitalWriteFast(DEBUG_PIN_3, LOW);
#endif

    // completely fill buffer with data before enabling DMA
    matrixCalcCallback(true);

    // setup SPI and DMA to feed it
    SPIFLEX.begin();
    SPIFLEX.flexIOHandler()->setClockSettings(3, 0, 0); // not exactly sure what this does, but without it the clock seems limited to ~7.5MHz
    SPIFLEX.beginTransaction(FlexIOSPISettings(spiClockSpeed, MSBFIRST, SPI_MODE0));

    // set interrupt with low priority for long compute time ISR
    apa102ShiftCompleteEvent.attachInterrupt((EventResponderFunction)&apaRowCalculationISR<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>, ROW_CALC_ISR_PRIORITY);

    // Give apaRowShiftCompleteISR a low priority, as it can some time (10s of microseconds) to return
    myTimer.priority(SHIFT_COMPLETE_ISR_PRIORITY);
    myTimer.begin(apaRowShiftCompleteISR<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>, TIME_PER_FRAME_US);
}

// low priority ISR
template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
void apaRowCalculationISR(void) {
#ifdef DEBUG_PINS_ENABLED
    digitalWriteFast(DEBUG_PIN_2, HIGH); // oscilloscope trigger
#endif

    // done with previous row, mark it as read
    cbRead(&SmartMatrixAPA102Refresh<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::dmaBuffer);

    SmartMatrixAPA102Refresh<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::matrixCalcCallback(false);

#ifdef DEBUG_PINS_ENABLED
    digitalWriteFast(DEBUG_PIN_2, LOW);
#endif
}

// DMA transfer done (meaning data was shifted and timer value for MSB on current row just got loaded)
// set DMA up for loading the next row, triggered from the next timer latch
template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
void apaRowShiftCompleteISR(void) {
#ifdef DEBUG_PINS_ENABLED
    digitalWriteFast(DEBUG_PIN_1, HIGH); // oscilloscope trigger
#endif
    int currentRow = cbGetNextRead(&SmartMatrixAPA102Refresh<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::dmaBuffer);

    // TODO: if underrun
        // set flag so other ISR can enable DMA again when data is ready
        //SmartMatrixAPA102Refresh<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::dmaBufferUnderrun = true;
    // else, start SPI

    // SPIFLEX.transfer calls arm_dcache_flush() which can take 10s of microseconds to complete - this is why this ISR has low priority
    // TODO: modify FlexIOSPI to allow for calling arm_dcache_flush() from less time sensitive location
    SPIFLEX.transfer(SmartMatrixAPA102Refresh<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::matrixUpdateFrame[currentRow].data,
        NULL, ((matrixWidth * matrixHeight)*4) + (4+4), apa102ShiftCompleteEvent);

#ifdef DEBUG_PINS_ENABLED
    digitalWriteFast(DEBUG_PIN_1, LOW); // oscilloscope trigger
#endif
}
