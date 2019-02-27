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
#include "DMAChannel.h"

#define INLINE __attribute__( ( always_inline ) ) inline

#if defined(KINETISL)
    #define ROW_CALCULATION_ISR_PRIORITY   192   // Cortex-M0 Acceptable values: 0,64,128,192
#elif defined(KINETISK)
    #define ROW_CALCULATION_ISR_PRIORITY   240 // M4 acceptable values: 0,16,32,48,64,80,96,112,128,144,160,176,192,208,224,240
#endif

// hardware-specific definitions
// prescale of 1 is F_BUS/2
#define LATCH_TIMER_PRESCALE  0x01
#define NS_TO_TICKS(X)      (uint32_t)(TIMER_FREQUENCY * ((X) / 1000000000.0))

#if defined(KINETISL)
    #define TIMER_FREQUENCY     (F_BUS/1)
#elif defined(KINETISK)
    #define TIMER_FREQUENCY     (F_BUS/2)
#endif

#define LATCH_TIMER_PULSE_WIDTH_TICKS   NS_TO_TICKS(LATCH_TIMER_PULSE_WIDTH_NS)
#define TICKS_PER_ROW   (TIMER_FREQUENCY/refreshRate/MATRIX_SCAN_MOD)
#define IDEAL_MSB_BLOCK_TICKS     (TICKS_PER_ROW/2)
#define MIN_BLOCK_PERIOD_NS (LATCH_TO_CLK_DELAY_NS + ((PANEL_32_PIXELDATA_TRANSFER_MAXIMUM_NS*PIXELS_PER_LATCH)/32))
#define MIN_BLOCK_PERIOD_TICKS NS_TO_TICKS(MIN_BLOCK_PERIOD_NS)

// slower refresh rates require larger timer values - get the min refresh rate from the largest MSB value that will fit in the timer (round up)
#define MIN_REFRESH_RATE    (((TIMER_FREQUENCY/65535)/MATRIX_SCAN_MOD/2) + 1)

#define TIMER_REGISTERS_TO_UPDATE   2

#if defined(KINETISL)
    extern DMAChannel dmaClockOutData;
    extern DMAChannel dmaClockOutData2;
#elif defined(KINETISK)
    #ifndef ADDX_UPDATE_ON_DATA_PINS
        extern DMAChannel dmaOutputAddress;
        extern DMAChannel dmaUpdateAddress;
    #endif
    extern DMAChannel dmaUpdateTimer;
    extern DMAChannel dmaClockOutData;
#endif

#if defined(KINETISL)
    template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
    void rowBitShiftCompleteISR(void);
#elif defined(KINETISK)
    template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
    void rowShiftCompleteISR(void);
#endif
template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
void rowCalculationISR(void);

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
CircularBuffer SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::dmaBuffer;

// dmaBufferNumRows = the size of the buffer that DMA pulls from to refresh the display
// must be minimum 2 rows so one can be updated while the other is refreshed
// increase beyond two to give more time for the update routine to complete
// (increase this number if non-DMA interrupts are causing display problems)
template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
uint8_t SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::dmaBufferNumRows;
template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
uint8_t SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::refreshRate = 120;

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
uint16_t SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::rowBitStructBytesToShift;

#ifndef ADDX_UPDATE_ON_DATA_PINS
    template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
    typename SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::addresspair SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::addressLUT[MATRIX_SCAN_MOD];

    template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
    typename SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::gpiopair SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::gpiosync;
#endif

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
typename SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::timerpair SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::timerLUT[LATCHES_PER_ROW];

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
typename SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::timerpair SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::timerPairIdle;

/*
    THIS IS NOW OUT OF DATE: data is now arranged linearly sorted first by pixels, then by color bit
  buffer contains:
    COLOR_DEPTH/COLOR_CHANNELS_PER_PIXEL/sizeof(int32_t) * (2 words for each pair of pixels: pixel data from n, and n+matrixRowPairOffset)
      first half of the words contain a byte for each shade, going from LSB to MSB
      second half of the words have the same data, plus a high bit in each byte for the clock
    there are MATRIX_WIDTH number of these in order to refresh a row (pair of rows)
    data is organized: matrixUpdateData[row][pixels within row][color depth * 2 updates per clock]

    DMA doesn't shift out the data sequentially, for each row, DMA goes through the buffer matrixUpdateData[currentrow][]

    layout of single matrixUpdateData row:
    in drawing, [] = byte, data is arranged as uint32_t going left to right in the drawing, "clk" is low, "CLK" is high
    DMA goes down one column of the drawing per latch signal trigger, resetting back to the top + shifting 1 byte to the right for the next latch trigger
    [pixel pair  0 - clk - MSB][pixel pair  0 - clk - MSB-1]...[pixel pair  0 - clk - LSB+1][pixel pair  0 - clk - LSB]
    [pixel pair  0 - CLK - MSB][pixel pair  0 - CLK - MSB-1]...[pixel pair  0 - CLK - LSB+1][pixel pair  0 - CLK - LSB]
    [pixel pair  1 - clk - MSB][pixel pair  1 - clk - MSB-1]...[pixel pair  1 - clk - LSB+1][pixel pair  1 - clk - LSB]
    [pixel pair  1 - CLK - MSB][pixel pair  1 - CLK - MSB-1]...[pixel pair  1 - CLK - LSB+1][pixel pair  1 - CLK - LSB]
    ...
    [pixel pair 15 - clk - MSB][pixel pair 15 - clk - MSB-1]...[pixel pair 15 - clk - LSB+1][pixel pair 15 - clk - LSB]
    [pixel pair 15 - CLK - MSB][pixel pair 15 - CLK - MSB-1]...[pixel pair 15 - CLK - LSB+1][pixel pair 15 - CLK - LSB]
 */
template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
typename SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::rowDataStruct * SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::matrixUpdateRows;

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::SmartMatrix3RefreshMultiplexed(uint8_t bufferrows, rowDataStruct * rowDataBuffer) {
    dmaBufferNumRows = bufferrows;

    matrixUpdateRows = rowDataBuffer;

    timerPairIdle.timer_period = MIN_BLOCK_PERIOD_TICKS;
    timerPairIdle.timer_oe = MIN_BLOCK_PERIOD_TICKS;
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
bool SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::isRowBufferFree(void) {
    if(cbIsFull(&dmaBuffer))
        return false;
    else
        return true;
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
typename SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::rowDataStruct * SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::getNextRowBufferPtr(void) {
    return &(matrixUpdateRows[cbGetNextWrite(&dmaBuffer)]);
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
void SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::writeRowBuffer(uint8_t currentRow) {
#ifndef ADDX_UPDATE_ON_DATA_PINS
    addresspair rowAddressPair;
    rowAddressPair.bits_to_set = addressLUT[currentRow].bits_to_set;
    rowAddressPair.bits_to_clear = addressLUT[currentRow].bits_to_clear;
#endif
        
    SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::rowDataStruct * currentRowDataPtr = SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::getNextRowBufferPtr();

    for (int i = 0; i < LATCHES_PER_ROW; i++) {
#ifndef ADDX_UPDATE_ON_DATA_PINS
        // copy bits to set and clear to generate address for current block
        currentRowDataPtr->rowbits[i].addressValues.bits_to_clear = rowAddressPair.bits_to_clear;
        currentRowDataPtr->rowbits[i].addressValues.bits_to_set = rowAddressPair.bits_to_set;
#endif
        currentRowDataPtr->rowbits[i].timerValues.timer_period = timerLUT[i].timer_period;
        currentRowDataPtr->rowbits[i].timerValues.timer_oe = timerLUT[i].timer_oe;
    }

    cbWrite(&dmaBuffer);
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
void SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::recoverFromDmaUnderrun(void) {
#if defined(KINETISL)
    // restart timer, rest will get setup in ISR
    FTM2_SC = 0;
    FTM2_CNT = 0;
    FTM2_SC = (FTM_SC_CLKS(1) | FTM_SC_PS(LATCH_TIMER_PRESCALE)) | FTM_SC_TOIE | FTM_SC_TOF | FTM_SC_DMA;

#elif defined(KINETISK)
    // stop timer
    FTM1_SC = FTM_SC_CLKS(0) | FTM_SC_PS(LATCH_TIMER_PRESCALE);

    // point DMA addresses to the next buffer
    int currentRow = cbGetNextRead(&dmaBuffer);
#ifndef ADDX_UPDATE_ON_DATA_PINS
    dmaUpdateAddress.TCD->SADDR = &(SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::matrixUpdateRows[0].rowbits[0].addressValues);
#endif
    dmaUpdateTimer.TCD->SADDR = &(SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::matrixUpdateRows[currentRow].rowbits[0].timerValues.timer_oe);
    dmaClockOutData.TCD->SADDR = (uint8_t*)&SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::matrixUpdateRows[currentRow].rowbits[0].data;

    // enable channel-to-channel linking so data will be shifted out
    dmaUpdateTimer.TCD->CSR &= ~(1 << 7);  // must clear DONE flag before enabling
    dmaUpdateTimer.TCD->CSR |= (1 << 5);
    // set timer increment back to read from matrixUpdateRows
    dmaUpdateTimer.TCD->SLAST = sizeof(matrixUpdateRows[0].rowbits[0]) - (TIMER_REGISTERS_TO_UPDATE * sizeof(uint16_t));

    // start timer again - next timer period is MIN_BLOCK_PERIOD_TICKS with OE disabled, period after that will be loaded from matrixUpdateBlock
    FTM1_SC = FTM_SC_CLKS(1) | FTM_SC_PS(LATCH_TIMER_PRESCALE);
#endif
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
typename SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::matrix_underrun_callback SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::matrixUnderrunCallback;

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
typename SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::matrix_calc_callback SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::matrixCalcCallback;

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
void SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::setMatrixCalculationsCallback(matrix_calc_callback f) {
    matrixCalcCallback = f;
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
void SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::setMatrixUnderrunCallback(matrix_underrun_callback f) {
    matrixUnderrunCallback = f;
}


#define MSB_BLOCK_TICKS_ADJUSTMENT_INCREMENT    10

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
void SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::calculateTimerLUT(void) {
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
            uint16_t padding = (MIN_BLOCK_PERIOD_TICKS) - period;
            period += padding;
            ontime += padding;
        }

        // add extra padding once per latch to match refreshRate exactly?  Doesn't seem to make a big difference
#if 0        
        if(!i) {
            uint16_t padding = TICKS_PER_ROW/2 - msbBlockTicks;
            period += padding;
            ontime += padding;
        }
#endif
        timerLUT[i].timer_period = period;
        timerLUT[i].timer_oe = ontime;
    }
}

// large factor = more dim, default is full brightness
template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
int SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::dimmingFactor = dimmingMaximum - (100 * 255)/100;

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
void SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::setBrightness(uint8_t newBrightness) {
    dimmingFactor = dimmingMaximum - newBrightness;
    calculateTimerLUT();
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
void SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::setRefreshRate(uint8_t newRefreshRate) {
    if(newRefreshRate > MIN_REFRESH_RATE)
        refreshRate = newRefreshRate;
    else
        refreshRate = MIN_REFRESH_RATE;
    calculateTimerLUT();
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
void SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::begin(void) {
    cbInit(&dmaBuffer, dmaBufferNumRows);

#ifndef ADDX_UPDATE_ON_DATA_PINS
    int i;
    // fill addressLUT
    for (i = 0; i < MATRIX_SCAN_MOD; i++) {

        // set all bits that are 1 in address
        addressLUT[i].bits_to_set = 0x00;
        if (i & 0x01)
            addressLUT[i].bits_to_set |= (1 << ADDX_PIN_0);
        if (i & 0x02)
            addressLUT[i].bits_to_set |= (1 << ADDX_PIN_1);
        if (i & 0x04)
            addressLUT[i].bits_to_set |= (1 << ADDX_PIN_2);
#ifdef ADDX_PIN_3
        if (i & 0x08)
            addressLUT[i].bits_to_set |= (1 << ADDX_PIN_3);
#endif

        // set all bits that are clear in address
        addressLUT[i].bits_to_clear = (~addressLUT[i].bits_to_set) & ADDX_PIN_MASK;
    }
#endif

    // fill timerLUT
    calculateTimerLUT();

    // completely fill buffer with data before enabling DMA
    matrixCalcCallback(true);

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

    // configure the 7 output pins (one pin is left as input, though it can't be used as GPIO output)
    pinMode(GPIO_PIN_CLK_TEENSY_PIN, OUTPUT);
    pinMode(GPIO_PIN_B0_TEENSY_PIN, OUTPUT);
    pinMode(GPIO_PIN_R0_TEENSY_PIN, OUTPUT);
    pinMode(GPIO_PIN_R1_TEENSY_PIN, OUTPUT);
    pinMode(GPIO_PIN_G0_TEENSY_PIN, OUTPUT);
    pinMode(GPIO_PIN_G1_TEENSY_PIN, OUTPUT);
    pinMode(GPIO_PIN_B1_TEENSY_PIN, OUTPUT);

#ifdef ADDX_TEENSY_PIN_0
    // configure the address pins
    pinMode(ADDX_TEENSY_PIN_0, OUTPUT);
#endif
#ifdef ADDX_TEENSY_PIN_1
    pinMode(ADDX_TEENSY_PIN_1, OUTPUT);
#endif
#ifdef ADDX_TEENSY_PIN_2
    pinMode(ADDX_TEENSY_PIN_2, OUTPUT);
#endif
#ifdef ADDX_TEENSY_PIN_3
    pinMode(ADDX_TEENSY_PIN_3, OUTPUT);
#endif

#ifdef ADDX_UPDATE_ON_DATA_PINS
    rowBitStructBytesToShift = sizeof(SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::matrixUpdateRows[0].rowbits[0].data) + ADDX_UPDATE_BEFORE_LATCH_BYTES;
#else
    rowBitStructBytesToShift = sizeof(SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::matrixUpdateRows[0].rowbits[0].data);
#endif

#if defined(KINETISL)
    // setup FTM2 (Teensy LC)
    FTM2_SC = 0;
    FTM2_CNT = 0;
    FTM2_MOD = timerLUT[0].timer_period;

    // setup FTM2 compares:
    // latch pulse width set based on max time to update address pins
    FTM2_C0V = LATCH_TIMER_PULSE_WIDTH_TICKS;
    FTM2_C1V = timerLUT[0].timer_oe;

    // Cortex-M0 Acceptable values: 0,64,128,192
    NVIC_SET_PRIORITY(IRQ_FTM2, 64);
    NVIC_ENABLE_IRQ(IRQ_FTM2);

    // setup PWM outputs
    ENABLE_LATCH_PWM_OUTPUT();
    ENABLE_OE_PWM_OUTPUT();

    // setup two DMA channels, but only enable one
    dmaClockOutData.begin();
    dmaClockOutData2.begin();

    dmaClockOutData.sourceBuffer((uint8_t*)matrixUpdateRows, rowBitStructBytesToShift);
    dmaClockOutData2.sourceBuffer((uint8_t*)matrixUpdateRows, rowBitStructBytesToShift);
    dmaClockOutData.destination(GPIOD_PDOR);
    dmaClockOutData2.destination(GPIOD_PDOR);

    // need to adjust destination size, we need 8-bit, no the size of GPIOD_PDOR - also transferSize() has a bug, so do it manually
    dmaClockOutData.CFG->DCR = (dmaClockOutData.CFG->DCR & 0xF0F8FFFF) | DMA_DCR_DSIZE(1);
    dmaClockOutData2.CFG->DCR = (dmaClockOutData2.CFG->DCR & 0xF0F8FFFF) | DMA_DCR_DSIZE(1);
    dmaClockOutData.transferCount(64);
    dmaClockOutData2.transferCount(64);

    // Can't use triggerAtHardwareEvent as is, as it sets the CS (Cycle Steal) bit to only transfer one byte instead of the whole transfer
    dmaClockOutData.triggerAtHardwareEvent(DMAMUX_SOURCE_FTM2_OV);
    dmaClockOutData2.triggerAtHardwareEvent(DMAMUX_SOURCE_LATCH_FALLING_EDGE);
    uint32_t dcr;
    dcr = dmaClockOutData.CFG->DCR;
    dcr &= ~(DMA_DCR_CS);
    dmaClockOutData.CFG->DCR = dcr;
    dcr = dmaClockOutData2.CFG->DCR;
    dcr &= ~(DMA_DCR_CS);
    dmaClockOutData2.CFG->DCR = dcr;
    
    // prevent an error when transfer is complete, BCR is zero, and timer triggers transfer
    dmaClockOutData.disableOnCompletion();
    dmaClockOutData2.disableOnCompletion();

    // enable additional dma interrupt used as software interrupt
    NVIC_SET_PRIORITY(IRQ_DMA_CH0 + dmaClockOutData.channel, ROW_CALCULATION_ISR_PRIORITY);
    dmaClockOutData.attachInterrupt(rowCalculationISR<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>);

    // only enable first DMA channel, second will be enabled in timer ISR
    dmaClockOutData.enable();

    attachInterruptVector(IRQ_FTM2, rowBitShiftCompleteISR<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>);

    // at the end after everything is set up: enable timer from system clock with appropriate prescale, enable interrupts, clear any pending overflow bits (otherwise get two ISR calls back to back in beginning), trigger DMA
    FTM2_SC = (FTM_SC_CLKS(1) | FTM_SC_PS(LATCH_TIMER_PRESCALE)) | FTM_SC_TOIE | FTM_SC_TOF | FTM_SC_DMA;

#elif defined(KINETISK)

    // setup FTM1
    FTM1_SC = 0;
    FTM1_CNT = 0;
    FTM1_MOD = IDEAL_MSB_BLOCK_TICKS;

    // setup FTM1 compares:
    // latch pulse width set based on max time to update address pins
    FTM1_C0V = LATCH_TIMER_PULSE_WIDTH_TICKS;
    // output OE signal - set to max at first to disable OE
    FTM1_C1V = IDEAL_MSB_BLOCK_TICKS;

    if(optionFlags & SMARTMATRIX_OPTIONS_HUB12_MODE) {
        // HUB12 format inverts the OE channel
        FTM1_POL = FTM_POL_POL1;
    }

    // setup PWM outputs
    ENABLE_LATCH_PWM_OUTPUT();
    ENABLE_OE_PWM_OUTPUT();

    // setup GPIO interrupts
    ENABLE_LATCH_RISING_EDGE_GPIO_INT();
    ENABLE_LATCH_FALLING_EDGE_GPIO_INT();


    // enable clocks to the DMA controller and DMAMUX
    SIM_SCGC7 |= SIM_SCGC7_DMA;
    SIM_SCGC6 |= SIM_SCGC6_DMAMUX;

    // enable minor loop mapping so addresses can get reset after minor loops
    DMA_CR |= DMA_CR_EMLM;

    // allocate all DMA channels up front so channels can link to each other
#ifndef ADDX_UPDATE_ON_DATA_PINS
    dmaOutputAddress.begin(false);
    dmaUpdateAddress.begin(false);
#endif
    dmaUpdateTimer.begin(false);
    dmaClockOutData.begin(false);

#ifndef ADDX_UPDATE_ON_DATA_PINS
#define ADDRESS_ARRAY_REGISTERS_TO_UPDATE   2
    // dmaOutputAddress - on latch rising edge, read address from fixed address temporary buffer, and output address on GPIO
    // using combo of writes to set+clear registers, to only modify the address pins and not other GPIO pins
    // address temporary buffer is refreshed before each DMA trigger (by DMA channel dmaUpdateAddress)
    // only use single major loop, never disable channel
    dmaOutputAddress.source(gpiosync.gpio_pcor);
    dmaOutputAddress.TCD->SOFF = (int)&gpiosync.gpio_psor - (int)&gpiosync.gpio_pcor;
    dmaOutputAddress.TCD->SLAST = (ADDRESS_ARRAY_REGISTERS_TO_UPDATE * ((int)&ADDX_GPIO_CLEAR_REGISTER - (int)&ADDX_GPIO_SET_REGISTER));
    dmaOutputAddress.TCD->ATTR = DMA_TCD_ATTR_SSIZE(2) | DMA_TCD_ATTR_DSIZE(2);
    // Destination Minor Loop Offset Enabled - transfer appropriate number of bytes per minor loop, and put DADDR back to original value when minor loop is complete
    // Source Minor Loop Offset Enabled - source buffer is same size and offset as destination so values reset after each minor loop
    dmaOutputAddress.TCD->NBYTES_MLOFFYES = DMA_TCD_NBYTES_SMLOE | DMA_TCD_NBYTES_DMLOE |
                               ((ADDRESS_ARRAY_REGISTERS_TO_UPDATE * ((int)&ADDX_GPIO_CLEAR_REGISTER - (int)&ADDX_GPIO_SET_REGISTER)) << 10) |
                               (ADDRESS_ARRAY_REGISTERS_TO_UPDATE * sizeof(gpiosync.gpio_psor));
    // start on higher value of two registers, and make offset decrement to avoid negative number in NBYTES_MLOFFYES (TODO: can switch order by masking negative offset)
    dmaOutputAddress.TCD->DADDR = &ADDX_GPIO_CLEAR_REGISTER;
    // update destination address so the second update per minor loop is ADDX_GPIO_SET_REGISTER
    dmaOutputAddress.TCD->DOFF = (int)&ADDX_GPIO_SET_REGISTER - (int)&ADDX_GPIO_CLEAR_REGISTER;
    dmaOutputAddress.TCD->DLASTSGA = (ADDRESS_ARRAY_REGISTERS_TO_UPDATE * ((int)&ADDX_GPIO_CLEAR_REGISTER - (int)&ADDX_GPIO_SET_REGISTER));
    // single major loop
    dmaOutputAddress.TCD->CITER_ELINKNO = 1;
    dmaOutputAddress.TCD->BITER_ELINKNO = 1;
    // link channel dmaUpdateAddress, enable major channel-to-channel linking, don't clear enable on major loop complete
    dmaOutputAddress.TCD->CSR = (dmaUpdateAddress.channel << 8) | (1 << 5);
    dmaOutputAddress.triggerAtHardwareEvent(DMAMUX_SOURCE_LATCH_RISING_EDGE);

    // dmaUpdateAddress - copy address values from current position in array to buffer to temporarily hold row values for the next timer cycle
    // only use single major loop, never disable channel
    dmaUpdateAddress.TCD->SADDR = &(matrixUpdateRows[0].rowbits[0].addressValues);
    dmaUpdateAddress.TCD->SOFF = sizeof(uint16_t);
    dmaUpdateAddress.TCD->SLAST = sizeof(matrixUpdateRows[0].rowbits[0]) - (ADDRESS_ARRAY_REGISTERS_TO_UPDATE * sizeof(uint16_t));
    dmaUpdateAddress.TCD->ATTR = DMA_TCD_ATTR_SSIZE(1) | DMA_TCD_ATTR_DSIZE(1);
    // 16-bit = 2 bytes transferred
    // transfer two 16-bit values, reset destination address back after each minor loop
    dmaUpdateAddress.TCD->NBYTES_MLOFFNO = (ADDRESS_ARRAY_REGISTERS_TO_UPDATE * sizeof(uint16_t));
    // start with the register that's the highest location in memory and make offset decrement to avoid negative number in NBYTES_MLOFFYES register (TODO: can switch order by masking negative offset)
    dmaUpdateAddress.TCD->DADDR = &gpiosync.gpio_pcor;
    dmaUpdateAddress.TCD->DOFF = (int)&gpiosync.gpio_psor - (int)&gpiosync.gpio_pcor;
    dmaUpdateAddress.TCD->DLASTSGA = (ADDRESS_ARRAY_REGISTERS_TO_UPDATE * ((int)&gpiosync.gpio_pcor - (int)&gpiosync.gpio_psor));
    // no minor loop linking, single major loop, single minor loop, don't clear enable after major loop complete
    dmaUpdateAddress.TCD->CITER_ELINKNO = 1;
    dmaUpdateAddress.TCD->BITER_ELINKNO = 1;
    dmaUpdateAddress.TCD->CSR = 0;
#endif

#define DMA_TCD_MLOFF_MASK  (0x3FFFFC00)

    // dmaUpdateTimer - on latch falling edge, load FTM1_CV1 and FTM1_MOD with with next values from current block
    // only use single major loop, never disable channel
    // link to dmaClockOutData channel when complete
    dmaUpdateTimer.TCD->SADDR = &(matrixUpdateRows[0].rowbits[0].timerValues.timer_oe);
    dmaUpdateTimer.TCD->SOFF = sizeof(uint16_t);
    dmaUpdateTimer.TCD->SLAST = sizeof(matrixUpdateRows[0].rowbits[0]) - (TIMER_REGISTERS_TO_UPDATE * sizeof(uint16_t));
    dmaUpdateTimer.TCD->ATTR = DMA_TCD_ATTR_SSIZE(1) | DMA_TCD_ATTR_DSIZE(1);
    // 16-bit = 2 bytes transferred
    dmaUpdateTimer.TCD->NBYTES_MLOFFNO = TIMER_REGISTERS_TO_UPDATE * sizeof(uint16_t);
    dmaUpdateTimer.TCD->DADDR = &FTM1_C1V;
    dmaUpdateTimer.TCD->DOFF = (int)&FTM1_MOD - (int)&FTM1_C1V;
    dmaUpdateTimer.TCD->DLASTSGA = TIMER_REGISTERS_TO_UPDATE * ((int)&FTM1_C1V - (int)&FTM1_MOD);
    // no minor loop linking, single major loop
    dmaUpdateTimer.TCD->CITER_ELINKNO = 1;
    dmaUpdateTimer.TCD->BITER_ELINKNO = 1;
    // link dmaClockOutData channel, enable major channel-to-channel linking, don't clear enable after major loop complete
    dmaUpdateTimer.TCD->CSR = (dmaClockOutData.channel << 8) | (1 << 5);
    dmaUpdateTimer.triggerAtHardwareEvent(DMAMUX_SOURCE_LATCH_FALLING_EDGE);

    // this is the number of bytes in the gap between each sequential rowBitStruct.data arrays
    uint16_t rowBitStructDataOffset = sizeof(SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::matrixUpdateRows[0].rowbits[0]) - rowBitStructBytesToShift;

    // dmaClockOutData - repeatedly load gpio_array into GPIOD_PDOR, stop and int on major loop complete
    dmaClockOutData.TCD->SADDR = matrixUpdateRows[0].rowbits[0].data;
    dmaClockOutData.TCD->SOFF = 1;
    // SADDR will get updated by ISR, no need to set SLAST
    dmaClockOutData.TCD->SLAST = 0;
    dmaClockOutData.TCD->ATTR = DMA_TCD_ATTR_SSIZE(0) | DMA_TCD_ATTR_DSIZE(0);
    // after each minor loop, apply no offset to source data, it's pointing to the next buffer already
    // clock out (PIXELS_PER_LATCH * DMA_UPDATES_PER_CLOCK + ADDX_UPDATE_BEFORE_LATCH_BYTES) number of bytes per loop
    dmaClockOutData.TCD->NBYTES_MLOFFYES = DMA_TCD_NBYTES_SMLOE |
                                ((rowBitStructDataOffset << 10) & DMA_TCD_MLOFF_MASK) |
                                rowBitStructBytesToShift;
    dmaClockOutData.TCD->DADDR = &GPIOD_PDOR;
    dmaClockOutData.TCD->DOFF = 0;
    dmaClockOutData.TCD->DLASTSGA = 0;
    dmaClockOutData.TCD->CITER_ELINKNO = LATCHES_PER_ROW;
    dmaClockOutData.TCD->BITER_ELINKNO = LATCHES_PER_ROW;
    // int after major loop is complete
    dmaClockOutData.TCD->CSR = DMA_TCD_CSR_INTMAJOR;
    
    // for debugging - enable bandwidth control (space out GPIO updates so they can be seen easier on a low-bandwidth logic analyzer)
    // enable for now, until DMA sharing complications (brought to light by Teensy 3.6 SDIO) can be worked out - use bandwidth control to space out our DMA access and allow SD reads to not slow down shifting to the matrix
    // also enable for now, until it can be selectively enabled for higher clock speeds (140MHz+) where the data rate is too high for the panel
    dmaClockOutData.TCD->CSR |= (0x02 << 14);

    // enable a done interrupt when all DMA operations are complete
    dmaClockOutData.attachInterrupt(rowShiftCompleteISR<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>);

    // enable additional dma interrupt used as software interrupt
    NVIC_SET_PRIORITY(IRQ_DMA_CH0 + dmaUpdateTimer.channel, ROW_CALCULATION_ISR_PRIORITY);
    dmaUpdateTimer.attachInterrupt(rowCalculationISR<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>);

#ifndef ADDX_UPDATE_ON_DATA_PINS
    dmaOutputAddress.enable();
    dmaUpdateAddress.enable();
#endif
    dmaUpdateTimer.enable();
    dmaClockOutData.enable();

    // at the end after everything is set up: enable timer from system clock, with appropriate prescale
    FTM1_SC = FTM_SC_CLKS(1) | FTM_SC_PS(LATCH_TIMER_PRESCALE);
#endif
}

// low priority ISR triggered by software interrupt on a DMA channel that doesn't need interrupts otherwise
template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
void rowCalculationISR(void) {
#ifdef DEBUG_PINS_ENABLED
    digitalWriteFast(DEBUG_PIN_2, HIGH); // oscilloscope trigger
#endif

    SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::matrixCalcCallback(false);

#ifdef DEBUG_PINS_ENABLED
    digitalWriteFast(DEBUG_PIN_2, LOW);
#endif
}

#ifdef KINETISL
template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
void rowBitShiftCompleteISR(void) {
    static bool alternateDmaBuffer = false;
    static int currentLatchBit = 0;
    static int currentRow = 0;

    digitalWriteFast(DEBUG_PIN_1, HIGH); // oscilloscope trigger

    if(currentLatchBit >= LATCHES_PER_ROW) {
        currentLatchBit = 0;

        // done with previous row, mark it as read
        cbRead(&SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::dmaBuffer);
    }

    if(currentLatchBit == 0) {
        // need new row, see if it is available yet
        if(cbIsEmpty(&SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::dmaBuffer)) {
            // new row is not available, handle DMA underrun
#ifdef DEBUG_PINS_ENABLED
            digitalWriteFast(DEBUG_PIN_1, LOW); // oscilloscope trigger
#endif
            // setup timer to overflow as fast as safely possible, with no interrupt, and no DMA channel linking (clear TOF flag)
            FTM2_MOD = SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::timerPairIdle.timer_period;
            FTM2_C1V = SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::timerPairIdle.timer_oe;
            FTM2_SC = (FTM_SC_CLKS(1) | FTM_SC_PS(LATCH_TIMER_PRESCALE)) | FTM_SC_TOF;

#ifdef DEBUG_PINS_ENABLED
            digitalWriteFast(DEBUG_PIN_1, HIGH); // oscilloscope trigger
#endif

            // disable PORTA DMA request on edge
            CORE_PIN3_CONFIG &= ~PORT_PCR_IRQC_MASK;

            // set flag so other ISR can enable DMA again when data is ready
            SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::matrixUnderrunCallback();

#ifdef DEBUG_PINS_ENABLED
            digitalWriteFast(DEBUG_PIN_1, LOW);
#endif
            // return without setting up timers below
            return;

        } else {
            // get next row to draw to display
            currentRow = cbGetNextRead(&SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::dmaBuffer);
        }
    }

    FTM2_MOD = SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::matrixUpdateRows[currentRow].rowbits[currentLatchBit].timerValues.timer_period;
    FTM2_C1V = SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::matrixUpdateRows[currentRow].rowbits[currentLatchBit].timerValues.timer_oe;

    // clear Timer Overflow Flag while keeping timer running with interrupts enabled - this also clears the FTM_SC_DMA flag, so dmaClockOutData won't trigger unless we want it to (it can be set later in the ISR)
    FTM2_SC = (FTM_SC_CLKS(1) | FTM_SC_PS(LATCH_TIMER_PRESCALE)) | FTM_SC_TOIE | FTM_SC_TOF;

    if(!alternateDmaBuffer) {
        dmaClockOutData2.clearComplete();
        dmaClockOutData2.CFG->SAR = (uint8_t*)&SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::matrixUpdateRows[currentRow].rowbits[currentLatchBit].data[0];
        dmaClockOutData2.transferCount(SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::rowBitStructBytesToShift);
        
        // enable PORTA DMA request on edge again - enabling trigger for dmaClockOutData2, now that we've passed the previous trigger we wanted to ignore
        ENABLE_LATCH_RISING_EDGE_GPIO_INT();
        
        dmaClockOutData2.enable();
    } else {
        // disable PORTA DMA request on edge - otherwise Interrupt Status Flag will stay asserted until the *completion* of the next DMA transfer, and will trigger the next time the channel is enabled, even though we don't want to be triggered on a previously seen edge
        CORE_PIN3_CONFIG &= ~PORT_PCR_IRQC_MASK;
    
        dmaClockOutData.clearComplete();
        dmaClockOutData.CFG->SAR = (uint8_t*)&SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::matrixUpdateRows[currentRow].rowbits[currentLatchBit].data[0];
        dmaClockOutData.transferCount(SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::rowBitStructBytesToShift);
    
        // enable DMA flag for FTM2 - enabling trigger for dmaClockOutData
        FTM2_SC |= FTM_SC_DMA;
    
        dmaClockOutData.enable();
    }

    alternateDmaBuffer = !alternateDmaBuffer;

    // update row buffers and triger software interrupt when done with row
    if(currentLatchBit == 0) {
        // trigger software interrupt to call rowCalculationISR() (DMA channel interrupt used instead of actual softint)
        NVIC_SET_PENDING(IRQ_DMA_CH0 + dmaClockOutData.channel);
    }

    currentLatchBit++;

    digitalWriteFast(DEBUG_PIN_1, LOW);
}

#elif defined(KINETISK)

// DMA transfer done (meaning data was shifted and timer value for MSB on current row just got loaded)
// set DMA up for loading the next row, triggered from the next timer latch
template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
void rowShiftCompleteISR(void) {
#ifdef DEBUG_PINS_ENABLED
    digitalWriteFast(DEBUG_PIN_1, HIGH); // oscilloscope trigger
#endif
    // done with previous row, mark it as read
    cbRead(&SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::dmaBuffer);

    if(cbIsEmpty(&SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::dmaBuffer)) {
#ifdef DEBUG_PINS_ENABLED
        digitalWriteFast(DEBUG_PIN_1, LOW); // oscilloscope trigger
#endif

        // point dmaUpdateTimer to repeatedly load from values that set mod to MIN_BLOCK_PERIOD_TICKS and disable OE
        dmaUpdateTimer.TCD->SADDR = &SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::timerPairIdle;
        // set timer increment to repeat timerPairIdle
        dmaUpdateTimer.TCD->SLAST = -(TIMER_REGISTERS_TO_UPDATE*sizeof(uint16_t));
        // disable channel-to-channel linking - don't link dmaClockOutData until buffer is ready
        dmaUpdateTimer.TCD->CSR &= ~(1 << 5);

        // set flag so other ISR can enable DMA again when data is ready
        SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::matrixUnderrunCallback();

#ifdef DEBUG_PINS_ENABLED
        digitalWriteFast(DEBUG_PIN_1, HIGH); // oscilloscope trigger
#endif
    } else {
        // get next row to draw to display and update DMA pointers
        int currentRow = cbGetNextRead(&SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::dmaBuffer);
#ifndef ADDX_UPDATE_ON_DATA_PINS
        dmaUpdateAddress.TCD->SADDR = &(SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::matrixUpdateRows[currentRow].rowbits[0].addressValues);
#endif
        dmaUpdateTimer.TCD->SADDR = &(SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::matrixUpdateRows[currentRow].rowbits[0].timerValues.timer_oe);
        dmaClockOutData.TCD->SADDR = (uint8_t*)&SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::matrixUpdateRows[currentRow].rowbits[0].data;
    }

    // trigger software interrupt to call rowCalculationISR() (DMA channel interrupt used instead of actual softint)
    NVIC_SET_PENDING(IRQ_DMA_CH0 + dmaUpdateTimer.channel);

    // clear pending int
    dmaClockOutData.clearInterrupt();

#ifdef DEBUG_PINS_ENABLED
    digitalWriteFast(DEBUG_PIN_1, LOW); // oscilloscope trigger
#endif
}
#endif
