/*
 * SmartMatrix Library - Refresh Code for Teensy 3.x Platform
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

#include "SmartMatrix3.h"
#include <SPI.h>
#include "DMAChannel.h"

#define INLINE __attribute__( ( always_inline ) ) inline

#define ROW_CALCULATION_ISR_PRIORITY   240 // M4 acceptable values: 0,16,32,48,64,80,96,112,128,144,160,176,192,208,224,240

// hardware-specific definitions
// prescale of 0x4 is F_BUS/16
#define COPROCESSOR_LATCH_TIMER_PRESCALE  0x04
#define COPROCESSOR_TIMER_FREQUENCY     (F_BUS/16)
#define COPROCESSOR_NS_TO_TICKS(X)      (uint32_t)(COPROCESSOR_TIMER_FREQUENCY * ((X) / 1000000000.0))

#define COPROCESSOR_TICKS_PER_ROW   (COPROCESSOR_TIMER_FREQUENCY/refreshRate/MATRIX_SCAN_MOD)

#define COPROCESSOR_MIN_REFRESH_RATE (((TIMER_FREQUENCY/65535)/MATRIX_SCAN_MOD) + 1)

extern DMAChannel dmaClockOutDataCoprocessorSend;

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
void coprocessorSendRowShiftCompleteISR(void);
template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
void coprocessorSendRowCalculationISR(void);

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
CircularBuffer SmartMatrix3CoprocessorSend<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::dmaBuffer;

// dmaBufferNumRows = the size of the buffer that DMA pulls from to refresh the display
// must be minimum 2 rows so one can be updated while the other is refreshed
// increase beyond two to give more time for the update routine to complete
// (increase this number if non-DMA interrupts are causing display problems)
template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
uint8_t SmartMatrix3CoprocessorSend<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::dmaBufferNumRows;
template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
uint8_t SmartMatrix3CoprocessorSend<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::refreshRate = 120;

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
uint16_t SmartMatrix3CoprocessorSend<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::rowBitStructBytesToShift;

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
typename SmartMatrix3CoprocessorSend<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::rowDataStruct * SmartMatrix3CoprocessorSend<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::matrixUpdateRows;

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
SmartMatrix3CoprocessorSend<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::SmartMatrix3CoprocessorSend(uint8_t bufferrows, rowDataStruct * rowDataBuffer) {
    dmaBufferNumRows = bufferrows;

    matrixUpdateRows = rowDataBuffer;
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
bool SmartMatrix3CoprocessorSend<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::isRowBufferFree(void) {
    if(cbIsFull(&dmaBuffer))
        return false;
    else
        return true;
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
typename SmartMatrix3CoprocessorSend<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::rowDataStruct * SmartMatrix3CoprocessorSend<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::getNextRowBufferPtr(void) {
    return &(matrixUpdateRows[cbGetNextWrite(&dmaBuffer)]);
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
void SmartMatrix3CoprocessorSend<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::writeRowBuffer(uint8_t currentRow) {
        
    SmartMatrix3CoprocessorSend<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::rowDataStruct * currentRowDataPtr = SmartMatrix3CoprocessorSend<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::getNextRowBufferPtr();

    currentRowDataPtr->startOfFrameByte = 0x55;
    currentRowDataPtr->refreshRate = SmartMatrix3CoprocessorSend<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::refreshRate;
    currentRowDataPtr->latchesPerRow = LATCHES_PER_ROW;
    // TODO: send brightness not dimmingFactor
    uint8_t brightness = SmartMatrix3CoprocessorSend<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::dimmingMaximum - SmartMatrix3CoprocessorSend<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::dimmingFactor;
    currentRowDataPtr->brightness = brightness;
    currentRowDataPtr->dataWidth = sizeof(SmartMatrix3CoprocessorSend<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::rowBitStruct);
    currentRowDataPtr->endOfFrameByte = 0xAA;

    for (int i = 0; i < LATCHES_PER_ROW; i++) {
    }

    cbWrite(&dmaBuffer);
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
void SmartMatrix3CoprocessorSend<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::recoverFromDmaUnderrun(void) {

}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
typename SmartMatrix3CoprocessorSend<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::matrix_underrun_callback SmartMatrix3CoprocessorSend<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::matrixUnderrunCallback;

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
typename SmartMatrix3CoprocessorSend<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::matrix_calc_callback SmartMatrix3CoprocessorSend<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::matrixCalcCallback;

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
void SmartMatrix3CoprocessorSend<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::setMatrixCalculationsCallback(matrix_calc_callback f) {
    matrixCalcCallback = f;
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
void SmartMatrix3CoprocessorSend<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::setMatrixUnderrunCallback(matrix_underrun_callback f) {
    matrixUnderrunCallback = f;
}

// large factor = more dim, default is full brightness
template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
int SmartMatrix3CoprocessorSend<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::dimmingFactor = dimmingMaximum - (100 * 255)/100;

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
void SmartMatrix3CoprocessorSend<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::setBrightness(uint8_t newBrightness) {
    dimmingFactor = dimmingMaximum - newBrightness;
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
void SmartMatrix3CoprocessorSend<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::setRefreshRate(uint8_t newRefreshRate) {
    if(newRefreshRate > COPROCESSOR_MIN_REFRESH_RATE)
        refreshRate = newRefreshRate;
    else
        refreshRate = COPROCESSOR_MIN_REFRESH_RATE;
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
void SmartMatrix3CoprocessorSend<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::begin(void) {
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
    SPI.begin();
    SPI.setMOSI(SMARTLED_APA_DAT_PIN);
    dmaClockOutDataCoprocessorSend.begin(false);
    dmaClockOutDataCoprocessorSend.disable();
    dmaClockOutDataCoprocessorSend.destination((volatile uint8_t&)SPI0_PUSHR);
    dmaClockOutDataCoprocessorSend.disableOnCompletion();
    dmaClockOutDataCoprocessorSend.triggerAtHardwareEvent(DMAMUX_SOURCE_SPI0_TX);
    dmaClockOutDataCoprocessorSend.attachInterrupt(coprocessorSendRowCalculationISR<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>);
    dmaClockOutDataCoprocessorSend.interruptAtCompletion();
    NVIC_SET_PRIORITY(IRQ_DMA_CH0 + dmaClockOutDataCoprocessorSend.channel, ROW_CALCULATION_ISR_PRIORITY);

    // setup FTM2
    FTM2_SC = 0;
    FTM2_CNT = 0;
    FTM2_MOD = COPROCESSOR_TICKS_PER_ROW;

#if 1
    // for debug: latch pulse width wide enough to be seen on logic analyzer
    FTM2_C0V = 100;

// out of date, set for FTM1_C0V
#define ENABLE_LATCH_PWM_OUTPUT() {                                     \
        CORE_PIN3_CONFIG |= PORT_PCR_MUX(3) | PORT_PCR_DSE | PORT_PCR_SRE;  \
    }

    // setup PWM outputs
    ENABLE_LATCH_PWM_OUTPUT();
#endif

    // enable timer from system clock, with appropriate prescale, TOF interrupt
    FTM2_SC = FTM_SC_CLKS(1) | FTM_SC_PS(COPROCESSOR_LATCH_TIMER_PRESCALE) | FTM_SC_TOIE;

    attachInterruptVector(IRQ_FTM2, coprocessorSendRowShiftCompleteISR<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>);

    NVIC_ENABLE_IRQ(IRQ_FTM2);
}

// low priority ISR triggered by software interrupt on a DMA channel that doesn't need interrupts otherwise
template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
void coprocessorSendRowCalculationISR(void) {
#ifdef DEBUG_PINS_ENABLED
    digitalWriteFast(DEBUG_PIN_2, HIGH); // oscilloscope trigger
#endif

    dmaClockOutDataCoprocessorSend.clearInterrupt();

    // done with previous row, mark it as read
    cbRead(&SmartMatrix3CoprocessorSend<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::dmaBuffer);

    SmartMatrix3CoprocessorSend<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::matrixCalcCallback(false);

#ifdef DEBUG_PINS_ENABLED
    digitalWriteFast(DEBUG_PIN_2, LOW);
#endif
}

// DMA transfer done (meaning data was shifted and timer value for MSB on current row just got loaded)
// set DMA up for loading the next row, triggered from the next timer latch
template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
void coprocessorSendRowShiftCompleteISR(void) {
#ifdef DEBUG_PINS_ENABLED
    digitalWriteFast(DEBUG_PIN_1, HIGH); // oscilloscope trigger
#endif

    // get next row to draw to display and update DMA pointers
    int currentRow = cbGetNextRead(&SmartMatrix3CoprocessorSend<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::dmaBuffer);

    // TODO: if underrun
        // set flag so other ISR can enable DMA again when data is ready
        //SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::dmaBufferUnderrun = true;
    // else, start SPI
    SPI.endTransaction();

    // disable SPI interrupts
    SPI0_RSER = 0;
    // clear flags
    SPI0_SR = SPI_SR_TCF | SPI_SR_EOQF | SPI_SR_TFUF | SPI_SR_TFFF | SPI_SR_RFOF | SPI_SR_RFDF;
    dmaClockOutDataCoprocessorSend.sourceBuffer((uint8_t*)&SmartMatrix3CoprocessorSend<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::matrixUpdateRows[currentRow],
        sizeof(SmartMatrix3CoprocessorSend<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::matrixUpdateRows[currentRow]));
    // Enable Transmit Fill DMA Requests
    SPI0_RSER = SPI_RSER_TFFF_RE | SPI_RSER_TFFF_DIRS;
    //SPI.beginTransaction(SPISettings());
    SPI.beginTransaction(SPISettings(12000000, MSBFIRST, SPI_MODE0));
    dmaClockOutDataCoprocessorSend.enable();

    // clear timer overflow bit before leaving ISR
    FTM2_SC &= ~FTM_SC_TOF;

#ifdef DEBUG_PINS_ENABLED
    digitalWriteFast(DEBUG_PIN_1, LOW); // oscilloscope trigger
#endif
}
