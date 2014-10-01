/*
 * SmartMatrix Library - Refresh Code for Teensy 3.x Platform
 *
 * Copyright (c) 2014 Louis Beaudoin (Pixelmatix)
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
#include "CircularBuffer.h"
#include "DMAChannel.h"

#define INLINE __attribute__( ( always_inline ) ) inline

// these definitions may change if switching major display type
#define MATRIX_ROW_PAIR_OFFSET          (MATRIX_HEIGHT/2)
#define MATRIX_ROWS_PER_FRAME           (MATRIX_HEIGHT/2)
#define PIXELS_UPDATED_PER_CLOCK        2
#define COLOR_CHANNELS_PER_PIXEL        3
#define LATCHES_PER_ROW                 (COLOR_DEPTH_RGB/COLOR_CHANNELS_PER_PIXEL)
#define DMA_UPDATES_PER_CLOCK           2
#define ROW_CALCULATION_ISR_PRIORITY   0xFE // 0xFF = lowest priority

// hardware-specific definitions
// prescale of 0 is F_BUS
#define LATCH_TIMER_PRESCALE  0x00
#define NS_TO_TICKS(X)      (uint32_t)(F_BUS * ((X) / 1000000000.0))
#define LATCH_TIMER_PULSE_WIDTH_TICKS   NS_TO_TICKS(LATCH_TIMER_PULSE_WIDTH_NS)
#define TICKS_PER_ROW   (F_BUS/MATRIX_REFRESH_RATE/MATRIX_ROWS_PER_FRAME)
#define MSB_BLOCK_TICKS     (TICKS_PER_ROW/2)
#define MIN_BLOCK_PERIOD_TICKS  NS_TO_TICKS(MIN_BLOCK_PERIOD_NS)

DMAChannel dmaOutputAddress(false);
DMAChannel dmaUpdateAddress(false);
DMAChannel dmaUpdateTimer(false);
DMAChannel dmaClockOutData(false);

void rowShiftCompleteISR(void);
void rowCalculationISR(void);

typedef struct timerpair {
    uint16_t timer_oe;
    uint16_t timer_period;
} timerpair;

typedef struct addresspair {
    uint16_t bits_to_clear;
    uint16_t bits_to_set;
} addresspair;

typedef struct matrixUpdateBlock {
    timerpair timerValues;
    addresspair addressValues;
} matrixUpdateBlock;

static CircularBuffer dmaBuffer;
static DMAMEM matrixUpdateBlock matrixUpdateBlocks[DMA_BUFFER_NUMBER_OF_ROWS][LATCHES_PER_ROW];

/*
  buffer contains:
    COLOR_DEPTH/sizeof(int32_t) * 2 words for each pair of pixels (pixel data from n, and n+MATRIX_ROW_PAIR_OFFSET)
      first half of the words contain a byte for each shade, going from LSB to MSB
      second half of the words have the same data, plus a high bit in each byte for the clock
    there are MATRIX_WIDTH number of these in order to refresh a row (pair of rows)
 */
static DMAMEM uint32_t matrixUpdateData[DMA_BUFFER_NUMBER_OF_ROWS][MATRIX_WIDTH][(LATCHES_PER_ROW / sizeof(uint32_t)) * DMA_UPDATES_PER_CLOCK];

#define ADDRESS_ARRAY_REGISTERS_TO_UPDATE   2
static addresspair addressLUT[MATRIX_ROWS_PER_FRAME];

static timerpair timerLUT[LATCHES_PER_ROW];

// 2x uint32_t to match size and spacing of values it is updating: GPIOx_PSOR and GPIOx_PCOR are 32-bit and adjacent to each other
typedef struct gpiopair {
    uint32_t  gpio_psor;
    uint32_t  gpio_pcor;
} gpiopair;

static gpiopair gpiosync;


SmartMatrix::SmartMatrix(void) {

}

INLINE void SmartMatrix::matrixCalculations(void) {
    static unsigned char currentRow = 0;

    // only run the loop if there is free space, and fill the entire buffer before returning
    while (!cbIsFull(&dmaBuffer)) {
        // do once-per-frame updates
        if (!currentRow) {
            handleBufferSwap();
            handleForegroundDrawingCopy();

            calculateBackgroundLUT();

#ifdef DEBUG_PINS_ENABLED
    digitalWriteFast(DEBUG_PIN_3, HIGH); // oscilloscope trigger
#endif
            updateForeground();
#ifdef DEBUG_PINS_ENABLED
    digitalWriteFast(DEBUG_PIN_3, LOW);
#endif

            if (brightnessChange) {
                calculateTimerLut();
                brightnessChange = false;
            }
        }

        // do once-per-line updates
        // none right now

        // enqueue row
        if (++currentRow >= MATRIX_ROWS_PER_FRAME)
            currentRow = 0;

        SmartMatrix::loadMatrixBuffers(currentRow);
        cbWrite(&dmaBuffer);
    }
}

INLINE void SmartMatrix::calculateTimerLut(void) {
    int i;

    for (i = 0; i < LATCHES_PER_ROW; i++) {
        // set period and OE values for current block - going from smallest timer values to largest
        // order needs to be smallest to largest so the last update of the row has the largest time between
        // the falling edge of the latch and the rising edge of the latch on the next row - an ISR
        // updates the row in this time

        // period is max on time for this block, plus the dead time while the latch is high
        uint16_t period = (MSB_BLOCK_TICKS >> (LATCHES_PER_ROW - i - 1)) + LATCH_TIMER_PULSE_WIDTH_TICKS;
        // on-time is the max on-time * dimming factor, plus the dead time while the latch is high
        uint16_t ontime = (((MSB_BLOCK_TICKS >> (LATCHES_PER_ROW - i - 1)) * dimmingFactor) / dimmingMaximum) + LATCH_TIMER_PULSE_WIDTH_TICKS;

        if (period < MIN_BLOCK_PERIOD_TICKS) {
            uint16_t padding = MIN_BLOCK_PERIOD_TICKS - period;
            period += padding;
            ontime += padding;
        }

        timerLUT[i].timer_period = period;
        timerLUT[i].timer_oe = ontime;
    }
}

void SmartMatrix::begin(void)
{
    int i;
    cbInit(&dmaBuffer, DMA_BUFFER_NUMBER_OF_ROWS);

    // fill addressLUT
    for (i = 0; i < MATRIX_ROWS_PER_FRAME; i++) {

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

    // fill timerLUT
    calculateTimerLut();

    // fill buffer with data before enabling DMA
    matrixCalculations();

    // load color correction table
    calculateBackgroundLUT();

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

    // configure the address pins
    pinMode(ADDX_TEENSY_PIN_0, OUTPUT);
    pinMode(ADDX_TEENSY_PIN_1, OUTPUT);
    pinMode(ADDX_TEENSY_PIN_2, OUTPUT);
#ifdef ADDX_TEENSY_PIN_3
    pinMode(ADDX_TEENSY_PIN_3, OUTPUT);
#endif

    // setup FTM1
    FTM1_SC = 0;
    FTM1_CNT = 0;
    FTM1_MOD = MSB_BLOCK_TICKS;

    // setup FTM1 compares:
    // latch pulse width set based on max time to update address pins
    FTM1_C0V = LATCH_TIMER_PULSE_WIDTH_TICKS;
    // output OE signal - set to max at first to disable OE
    FTM1_C1V = MSB_BLOCK_TICKS;

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
    dmaOutputAddress.begin(false);
    dmaUpdateAddress.begin(false);
    dmaUpdateTimer.begin(false);
    dmaClockOutData.begin(false);

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
    dmaUpdateAddress.TCD->SADDR = &matrixUpdateBlocks[0][0].addressValues;
    dmaUpdateAddress.TCD->SOFF = sizeof(uint16_t);
    dmaUpdateAddress.TCD->SLAST = sizeof(matrixUpdateBlock) - (ADDRESS_ARRAY_REGISTERS_TO_UPDATE * sizeof(uint16_t));
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

    // dmaUpdateTimer - on latch falling edge, load FTM1_CV1 and FTM1_MOD with with next values from current block
    // only use single major loop, never disable channel
    // link to dmaClockOutData channel when complete
#define TIMER_REGISTERS_TO_UPDATE   2
    dmaUpdateTimer.source(matrixUpdateBlocks[0][0].timerValues.timer_oe);
    dmaUpdateTimer.TCD->SOFF = sizeof(uint16_t);
    dmaUpdateTimer.TCD->SLAST = sizeof(matrixUpdateBlock) - (TIMER_REGISTERS_TO_UPDATE * sizeof(uint16_t));
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

#define DMA_TCD_MLOFF_MASK  (0x3FFFFC00)

    // dmaClockOutData - repeatedly load gpio_array into GPIOD_PDOR, stop and int on major loop complete
    dmaClockOutData.TCD->SADDR = matrixUpdateData[0][0];
    dmaClockOutData.TCD->SOFF = sizeof(matrixUpdateData[0][0]) / 2;
    // SADDR will get updated by ISR, no need to set SLAST
    dmaClockOutData.TCD->SLAST = 0;
    dmaClockOutData.TCD->ATTR = DMA_TCD_ATTR_SSIZE(0) | DMA_TCD_ATTR_DSIZE(0);
    // after each minor loop, set source to point back to the beginning of this set of data,
    // but advance by 1 byte to get the next significant bits data
    dmaClockOutData.TCD->NBYTES_MLOFFYES = DMA_TCD_NBYTES_SMLOE |
                               (((1 - sizeof(matrixUpdateData[0])) << 10) & DMA_TCD_MLOFF_MASK) |
                               (MATRIX_WIDTH * DMA_UPDATES_PER_CLOCK);
    dmaClockOutData.TCD->DADDR = &GPIOD_PDOR;
    dmaClockOutData.TCD->DOFF = 0;
    dmaClockOutData.TCD->DLASTSGA = 0;
    dmaClockOutData.TCD->CITER_ELINKNO = LATCHES_PER_ROW;
    dmaClockOutData.TCD->BITER_ELINKNO = LATCHES_PER_ROW;
    // int after major loop is complete
    dmaClockOutData.TCD->CSR = DMA_TCD_CSR_INTMAJOR;
    // for debugging - enable bandwidth control (space out GPIO updates so they can be seen easier on a low-bandwidth logic analyzer)
    //dmaClockOutData.TCD->CSR |= (0x02 << 14);

    // enable a done interrupt when all DMA operations are complete
    dmaClockOutData.attachInterrupt(rowShiftCompleteISR);

    // enable additional dma interrupt used as software interrupt
    NVIC_SET_PRIORITY(IRQ_DMA_CH0 + dmaUpdateAddress.channel, ROW_CALCULATION_ISR_PRIORITY);
    dmaUpdateAddress.attachInterrupt(rowCalculationISR);

    dmaOutputAddress.enable();
    dmaUpdateAddress.enable();
    dmaUpdateTimer.enable();
    dmaClockOutData.enable();

    // at the end after everything is set up: enable timer from system clock, with appropriate prescale
    FTM1_SC = FTM_SC_CLKS(1) | FTM_SC_PS(LATCH_TIMER_PRESCALE);
}

extern bool hasForeground;
INLINE void SmartMatrix::loadMatrixBuffers(unsigned char currentRow) {
    int i, j;

    addresspair rowAddressPair;

    rowAddressPair.bits_to_set = addressLUT[currentRow].bits_to_set;
    rowAddressPair.bits_to_clear = addressLUT[currentRow].bits_to_clear;

    unsigned char freeRowBuffer = cbGetNextWrite(&dmaBuffer);

    // for each color bit, fill buffer with pixel data for all columns in current rows
    for (j = 0; j < LATCHES_PER_ROW; j++) {
        // copy bits to set and clear to generate address for current block
        matrixUpdateBlocks[freeRowBuffer][j].addressValues.bits_to_clear = rowAddressPair.bits_to_clear;
        matrixUpdateBlocks[freeRowBuffer][j].addressValues.bits_to_set = rowAddressPair.bits_to_set;

        matrixUpdateBlocks[freeRowBuffer][j].timerValues.timer_period = timerLUT[j].timer_period;
        matrixUpdateBlocks[freeRowBuffer][j].timerValues.timer_oe = timerLUT[j].timer_oe;
    }

    rgb24 tempPixel0;
    rgb24 tempPixel1;

    bool bHasForeground = hasForeground;
    bool bHasCC = SmartMatrix::_ccmode != ccNone;
    rgb24 *pRow = SmartMatrix::getRefreshRow(currentRow);
    rgb24 *pRow2 = SmartMatrix::getRefreshRow(currentRow + MATRIX_ROW_PAIR_OFFSET);

    for (i = 0; i < MATRIX_WIDTH; i++) {

#if LATCHES_PER_ROW >= 12
        uint16_t temp0red,temp0green,temp0blue,temp1red,temp1green,temp1blue;
#else
        uint8_t temp0red,temp0green,temp0blue,temp1red,temp1green,temp1blue;
#endif

        if (bHasForeground && getForegroundPixel(i, currentRow, &tempPixel0)) {
            if(bHasCC) {
                // load foreground pixel with color correction
                temp0red = colorCorrection(tempPixel0.red);
                temp0green = colorCorrection(tempPixel0.green);
                temp0blue = colorCorrection(tempPixel0.blue);
            } else {
                // load foreground pixel without color correction
                temp0red = tempPixel0.red;
                temp0green = tempPixel0.green;
                temp0blue = tempPixel0.blue;
            }
        } else {
            if(bHasCC) {
                // load background pixel with color correction
                temp0red = backgroundColorCorrection(pRow[i].red);
                temp0green = backgroundColorCorrection(pRow[i].green);
                temp0blue = backgroundColorCorrection(pRow[i].blue);
            } else {
                // load background pixel without color correction
                temp0red = pRow[i].red;
                temp0green = pRow[i].green;
                temp0blue = pRow[i].blue;
            }
        }

        if (bHasForeground && getForegroundPixel(i, currentRow + MATRIX_ROW_PAIR_OFFSET, &tempPixel1)) {
            if(bHasCC) {
                // load foreground pixel with color correction
                temp1red = colorCorrection(tempPixel1.red);
                temp1green = colorCorrection(tempPixel1.green);
                temp1blue = colorCorrection(tempPixel1.blue);
            } else {
                // load foreground pixel without color correction
                temp1red = tempPixel1.red;
                temp1green = tempPixel1.green;
                temp1blue = tempPixel1.blue;
            }
        } else {
            if(bHasCC) {
                // load background pixel with color correction
                temp1red = backgroundColorCorrection(pRow2[i].red);
                temp1green = backgroundColorCorrection(pRow2[i].green);
                temp1blue = backgroundColorCorrection(pRow2[i].blue);
            } else {
                // load background pixel without color correction
                temp1red = pRow2[i].red;
                temp1green = pRow2[i].green;
                temp1blue = pRow2[i].blue;
            }
        }

#if LATCHES_PER_ROW >= 12
        if(!bHasCC) {
            temp0red = temp0red << 8;
            temp0green = temp0green << 8;
            temp0blue = temp0blue << 8;
            temp1red = temp1red << 8;
            temp1green = temp1green << 8;
            temp1blue = temp1blue << 8;
        }
#endif

#if LATCHES_PER_ROW == 12
            temp0red >>= 4;
            temp0green >>= 4;
            temp0blue >>= 4;

            temp1red >>= 4;
            temp1green >>= 4;
            temp1blue >>= 4;
#endif
        // this technique is from Fadecandy
        union {
            uint32_t word;
            struct {
                // order of bits in word matches how GPIO connects to the display
                uint32_t GPIO_WORD_ORDER;
            };
        } o0, o1, clkset;


        o0.word = 0;
        // set bits starting from LSB brightness moving to MSB brightness with each byte across the word
        // each word contains four brightness levels for single set of pixels above
        // o0.p0clk = 0;
        // o0.p0pad = 0;
        o0.p0b1 = temp0blue    >> 0;
        o0.p0r1 = temp0red     >> 0;
        o0.p0r2 = temp1red     >> 0;
        o0.p0g1 = temp0green   >> 0;
        o0.p0g2 = temp1green   >> 0;
        o0.p0b2 = temp1blue    >> 0;

        // o0.p1clk = 0;
        // o0.p1pad = 0;
        o0.p1b1 = temp0blue    >> 1;
        o0.p1r1 = temp0red     >> 1;
        o0.p1r2 = temp1red     >> 1;
        o0.p1g1 = temp0green   >> 1;
        o0.p1g2 = temp1green   >> 1;
        o0.p1b2 = temp1blue    >> 1;

        // o0.p2clk = 0;
        // o0.p2pad = 0;
        o0.p2b1 = temp0blue    >> 2;
        o0.p2r1 = temp0red     >> 2;
        o0.p2r2 = temp1red     >> 2;
        o0.p2g1 = temp0green   >> 2;
        o0.p2g2 = temp1green   >> 2;
        o0.p2b2 = temp1blue    >> 2;

        // o0.p3clk = 0;
        // o0.p3pad = 0;
        o0.p3b1 = temp0blue    >> 3;
        o0.p3r1 = temp0red     >> 3;
        o0.p3r2 = temp1red     >> 3;
        o0.p3g1 = temp0green   >> 3;
        o0.p3g2 = temp1green   >> 3;
        o0.p3b2 = temp1blue    >> 3;


        // continue moving from LSB to MSB brightness with the next word
        o1.word = 0;
        // o1.p0clk = 0;
        // o1.p0pad = 0;
        o1.p0b1 = temp0blue    >> (0 + 1 * sizeof(uint32_t));
        o1.p0r1 = temp0red     >> (0 + 1 * sizeof(uint32_t));
        o1.p0r2 = temp1red     >> (0 + 1 * sizeof(uint32_t));
        o1.p0g1 = temp0green   >> (0 + 1 * sizeof(uint32_t));
        o1.p0g2 = temp1green   >> (0 + 1 * sizeof(uint32_t));
        o1.p0b2 = temp1blue    >> (0 + 1 * sizeof(uint32_t));

        // o1.p1clk = 0;
        // o1.p1pad = 0;
        o1.p1b1 = temp0blue    >> (1 + 1 * sizeof(uint32_t));
        o1.p1r1 = temp0red     >> (1 + 1 * sizeof(uint32_t));
        o1.p1r2 = temp1red     >> (1 + 1 * sizeof(uint32_t));
        o1.p1g1 = temp0green   >> (1 + 1 * sizeof(uint32_t));
        o1.p1g2 = temp1green   >> (1 + 1 * sizeof(uint32_t));
        o1.p1b2 = temp1blue    >> (1 + 1 * sizeof(uint32_t));

        // o1.p2clk = 0;
        // o1.p2pad = 0;
        o1.p2b1 = temp0blue    >> (2 + 1 * sizeof(uint32_t));
        o1.p2r1 = temp0red     >> (2 + 1 * sizeof(uint32_t));
        o1.p2r2 = temp1red     >> (2 + 1 * sizeof(uint32_t));
        o1.p2g1 = temp0green   >> (2 + 1 * sizeof(uint32_t));
        o1.p2g2 = temp1green   >> (2 + 1 * sizeof(uint32_t));
        o1.p2b2 = temp1blue    >> (2 + 1 * sizeof(uint32_t));

        // o1.p3clk = 0;
        // o1.p3pad = 0;
        o1.p3b1 = temp0blue    >> (3 + 1 * sizeof(uint32_t));
        o1.p3r1 = temp0red     >> (3 + 1 * sizeof(uint32_t));
        o1.p3r2 = temp1red     >> (3 + 1 * sizeof(uint32_t));
        o1.p3g1 = temp0green   >> (3 + 1 * sizeof(uint32_t));
        o1.p3g2 = temp1green   >> (3 + 1 * sizeof(uint32_t));
        o1.p3b2 = temp1blue    >> (3 + 1 * sizeof(uint32_t));

#if LATCHES_PER_ROW >= 12
        union {
            uint32_t word;
            struct {
                // order of bits in word matches how GPIO connects to the display
                uint32_t GPIO_WORD_ORDER;
            };
        } o2;

        o2.word = 0;
        //o2.p0clk = 0;
        //o2.p0pad = 0;
        o2.p0b1 = temp0blue    >> (0 + 2 * sizeof(uint32_t));
        o2.p0r1 = temp0red     >> (0 + 2 * sizeof(uint32_t));
        o2.p0r2 = temp1red     >> (0 + 2 * sizeof(uint32_t));
        o2.p0g1 = temp0green   >> (0 + 2 * sizeof(uint32_t));
        o2.p0g2 = temp1green   >> (0 + 2 * sizeof(uint32_t));
        o2.p0b2 = temp1blue    >> (0 + 2 * sizeof(uint32_t));

        //o2.p1clk = 0;
        //o2.p1pad = 0;
        o2.p1b1 = temp0blue    >> (1 + 2 * sizeof(uint32_t));
        o2.p1r1 = temp0red     >> (1 + 2 * sizeof(uint32_t));
        o2.p1r2 = temp1red     >> (1 + 2 * sizeof(uint32_t));
        o2.p1g1 = temp0green   >> (1 + 2 * sizeof(uint32_t));
        o2.p1g2 = temp1green   >> (1 + 2 * sizeof(uint32_t));
        o2.p1b2 = temp1blue    >> (1 + 2 * sizeof(uint32_t));

        //o2.p2clk = 0;
        //o2.p2pad = 0;
        o2.p2b1 = temp0blue    >> (2 + 2 * sizeof(uint32_t));
        o2.p2r1 = temp0red     >> (2 + 2 * sizeof(uint32_t));
        o2.p2r2 = temp1red     >> (2 + 2 * sizeof(uint32_t));
        o2.p2g1 = temp0green   >> (2 + 2 * sizeof(uint32_t));
        o2.p2g2 = temp1green   >> (2 + 2 * sizeof(uint32_t));
        o2.p2b2 = temp1blue    >> (2 + 2 * sizeof(uint32_t));


        //o2.p3clk = 0;
        //o2.p3pad = 0;
        o2.p3b1 = temp0blue    >> (3 + 2 * sizeof(uint32_t));
        o2.p3r1 = temp0red     >> (3 + 2 * sizeof(uint32_t));
        o2.p3r2 = temp1red     >> (3 + 2 * sizeof(uint32_t));
        o2.p3g1 = temp0green   >> (3 + 2 * sizeof(uint32_t));
        o2.p3g2 = temp1green   >> (3 + 2 * sizeof(uint32_t));
        o2.p3b2 = temp1blue    >> (3 + 2 * sizeof(uint32_t));
#endif

#if LATCHES_PER_ROW == 16
        union {
            uint32_t word;
            struct {
                // order of bits in word matches how GPIO connects to the display
                uint32_t GPIO_WORD_ORDER;
            };
        } o3;

        o3.word = 0;
        //o3.p0clk = 0;
        //o3.p0pad = 0;
        o3.p0b1 = temp0blue    >> (0 + 3 * sizeof(uint32_t));
        o3.p0r1 = temp0red     >> (0 + 3 * sizeof(uint32_t));
        o3.p0r2 = temp1red     >> (0 + 3 * sizeof(uint32_t));
        o3.p0g1 = temp0green   >> (0 + 3 * sizeof(uint32_t));
        o3.p0g2 = temp1green   >> (0 + 3 * sizeof(uint32_t));
        o3.p0b2 = temp1blue    >> (0 + 3 * sizeof(uint32_t));

        //o3.p1clk = 0;
        //o3.p1pad = 0;
        o3.p1b1 = temp0blue    >> (1 + 3 * sizeof(uint32_t));
        o3.p1r1 = temp0red     >> (1 + 3 * sizeof(uint32_t));
        o3.p1r2 = temp1red     >> (1 + 3 * sizeof(uint32_t));
        o3.p1g1 = temp0green   >> (1 + 3 * sizeof(uint32_t));
        o3.p1g2 = temp1green   >> (1 + 3 * sizeof(uint32_t));
        o3.p1b2 = temp1blue    >> (1 + 3 * sizeof(uint32_t));

        //o3.p2clk = 0;
        //o3.p2pad = 0;
        o3.p2b1 = temp0blue    >> (2 + 3 * sizeof(uint32_t));
        o3.p2r1 = temp0red     >> (2 + 3 * sizeof(uint32_t));
        o3.p2r2 = temp1red     >> (2 + 3 * sizeof(uint32_t));
        o3.p2g1 = temp0green   >> (2 + 3 * sizeof(uint32_t));
        o3.p2g2 = temp1green   >> (2 + 3 * sizeof(uint32_t));
        o3.p2b2 = temp1blue    >> (2 + 3 * sizeof(uint32_t));


        //o3.p3clk = 0;
        //o3.p3pad = 0;
        o3.p3b1 = temp0blue    >> (3 + 3 * sizeof(uint32_t));
        o3.p3r1 = temp0red     >> (3 + 3 * sizeof(uint32_t));
        o3.p3r2 = temp1red     >> (3 + 3 * sizeof(uint32_t));
        o3.p3g1 = temp0green   >> (3 + 3 * sizeof(uint32_t));
        o3.p3g2 = temp1green   >> (3 + 3 * sizeof(uint32_t));
        o3.p3b2 = temp1blue    >> (3 + 3 * sizeof(uint32_t));
#endif

        clkset.word = 0x00;
        clkset.p0clk = 1;
        clkset.p1clk = 1;
        clkset.p2clk = 1;
        clkset.p3clk = 1;

        // copy words to DMA buffer
        matrixUpdateData[freeRowBuffer][i][0] = o0.word;
        matrixUpdateData[freeRowBuffer][i][1] = o1.word;

        // copy the next set of words with the same data, but clock set high
        matrixUpdateData[freeRowBuffer][i][LATCHES_PER_ROW / sizeof(uint32_t) + 0] = o0.word | clkset.word;
        matrixUpdateData[freeRowBuffer][i][LATCHES_PER_ROW / sizeof(uint32_t) + 1] = o1.word | clkset.word;

#if LATCHES_PER_ROW >= 12
        matrixUpdateData[freeRowBuffer][i][2] = o2.word;
        matrixUpdateData[freeRowBuffer][i][LATCHES_PER_ROW / sizeof(uint32_t) + 2] = o2.word | clkset.word;
#endif
#if LATCHES_PER_ROW == 16
        matrixUpdateData[freeRowBuffer][i][3] = o3.word;
        matrixUpdateData[freeRowBuffer][i][LATCHES_PER_ROW / sizeof(uint32_t) + 3] = o3.word | clkset.word;
#endif
    }

}

void rowCalculationISR(void) {
#ifdef DEBUG_PINS_ENABLED
    digitalWriteFast(DEBUG_PIN_2, HIGH); // oscilloscope trigger
#endif

    SmartMatrix::matrixCalculations();

#ifdef DEBUG_PINS_ENABLED
    digitalWriteFast(DEBUG_PIN_2, LOW);
#endif
}

// DMA transfer done (meaning data was shifted and timer value for MSB on current row just got loaded)
// set DMA up for loading the next row, triggered from the next timer latch
void rowShiftCompleteISR(void) {
#ifdef DEBUG_PINS_ENABLED
    digitalWriteFast(DEBUG_PIN_1, HIGH); // oscilloscope trigger
#endif
    // done with previous row, mark it as read
    cbRead(&dmaBuffer);

    // get next row to draw to display and update DMA pointers
    int currentRow = cbGetNextRead(&dmaBuffer);
    dmaUpdateAddress.TCD->SADDR = &matrixUpdateBlocks[currentRow][0].addressValues;
    dmaUpdateTimer.TCD->SADDR = &matrixUpdateBlocks[currentRow][0].timerValues.timer_oe;
    dmaClockOutData.TCD->SADDR = matrixUpdateData[currentRow][0];

    // clear pending GPIO int for PORTA before enabling DMA again
    CORE_PIN3_CONFIG |= (1 << 24);

    // trigger software interrupt (DMA channel interrupt used instead of actual softint)
    NVIC_SET_PENDING(IRQ_DMA_CH0 + dmaUpdateAddress.channel);

    // clear pending int
    dmaClockOutData.clearInterrupt();

#ifdef DEBUG_PINS_ENABLED
    digitalWriteFast(DEBUG_PIN_1, LOW); // oscilloscope trigger
#endif
}
