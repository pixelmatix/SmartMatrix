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

// these definitions may change if switching major display type
#define MATRIX_ROW_PAIR_OFFSET      (MATRIX_HEIGHT/2)
#define MATRIX_ROWS_PER_FRAME       (MATRIX_HEIGHT/2)
#define PIXELS_UPDATED_PER_CLOCK    2
#define COLOR_CHANNELS_PER_PIXEL    3
#define LATCHES_PER_ROW             (COLOR_DEPTH_RGB/COLOR_CHANNELS_PER_PIXEL)
#define DMA_UPDATES_PER_CLOCK       2

// hardware-specific definitions
// prescale of 0 is F_BUS
#define LATCH_TIMER_PRESCALE  0x00
#define NS_TO_TICKS(X)      (uint32_t)(F_BUS * ((X) / 1000000000.0))
#define LATCH_TIMER_PULSE_WIDTH_TICKS   NS_TO_TICKS(LATCH_TIMER_PULSE_WIDTH_NS)
#define TICKS_PER_ROW   (F_BUS/MATRIX_REFRESH_RATE/MATRIX_ROWS_PER_FRAME)
#define MSB_BLOCK_TICKS     (TICKS_PER_ROW/2)
#define MIN_BLOCK_PERIOD_TICKS  NS_TO_TICKS(MIN_BLOCK_PERIOD_NS)


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

addresspair addressLUT[MATRIX_ROWS_PER_FRAME];

timerpair timerLUT[LATCHES_PER_ROW];

// 2x uint32_t to match size and spacing of values it is updating: GPIOx_PSOR and GPIOx_PCOR are 32-bit and adjacent to each other
typedef struct gpiopair {
    uint32_t  gpio_psor;
    uint32_t  gpio_pcor;
} gpiopair;

gpiopair gpiosync;


SmartMatrix::SmartMatrix() {

}

void SmartMatrix::matrixCalculations(void) {
    static unsigned char currentRow = 0;

    // only run the loop if there is free space, and fill the entire buffer before returning
    while (!cbIsFull(&dmaBuffer)) {
        // do once-per-frame updates
        if (!currentRow) {
            handleBufferSwap();

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

void SmartMatrix::calculateTimerLut(void) {
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

void SmartMatrix::begin()
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
    DMA_CR = 1 << 7;

    // disable DMA
    DMA_ERQ = 0;

    // reset state of DMAMUX
    DMAMUX0_CHCFG0 = 0;
    DMAMUX0_CHCFG1 = 0;
    DMAMUX0_CHCFG2 = 0;
    DMAMUX0_CHCFG3 = 0;

    // DMA channel #0 - on latch rising edge, read address from fixed address temporary buffer, and output address on GPIO
    // using combo of writes to set+clear registers, to only modify the address pins and not other GPIO pins
    // address temporary buffer is refreshed before each DMA trigger (by DMA channel #2)
    // only use single major loop, never disable channel
#define ADDRESS_ARRAY_REGISTERS_TO_UPDATE   2
    DMA_TCD0_SADDR = &gpiosync.gpio_pcor;
    DMA_TCD0_SOFF = (int)&gpiosync.gpio_psor - (int)&gpiosync.gpio_pcor;
    DMA_TCD0_SLAST = (ADDRESS_ARRAY_REGISTERS_TO_UPDATE * ((int)&ADDX_GPIO_CLEAR_REGISTER - (int)&ADDX_GPIO_SET_REGISTER));
    DMA_TCD0_ATTR = DMA_TCD_ATTR_SSIZE(2) | DMA_TCD_ATTR_DSIZE(2);
    // Destination Minor Loop Offset Enabled - transfer appropriate number of bytes per minor loop, and put DADDR back to original value when minor loop is complete
    // Source Minor Loop Offset Enabled - source buffer is same size and offset as destination so values reset after each minor loop
    DMA_TCD0_NBYTES_MLOFFYES = DMA_TCD_NBYTES_SMLOE | DMA_TCD_NBYTES_DMLOE |
                               ((ADDRESS_ARRAY_REGISTERS_TO_UPDATE * ((int)&ADDX_GPIO_CLEAR_REGISTER - (int)&ADDX_GPIO_SET_REGISTER)) << 10) |
                               (ADDRESS_ARRAY_REGISTERS_TO_UPDATE * sizeof(gpiosync.gpio_psor));
    // start on higher value of two registers, and make offset decrement to avoid negative number in NBYTES_MLOFFYES (TODO: can switch order by masking negative offset)
    DMA_TCD0_DADDR = &ADDX_GPIO_CLEAR_REGISTER;
    // update destination address so the second update per minor loop is ADDX_GPIO_SET_REGISTER
    DMA_TCD0_DOFF = (int)&ADDX_GPIO_SET_REGISTER - (int)&ADDX_GPIO_CLEAR_REGISTER;
    DMA_TCD0_DLASTSGA = (ADDRESS_ARRAY_REGISTERS_TO_UPDATE * ((int)&ADDX_GPIO_CLEAR_REGISTER - (int)&ADDX_GPIO_SET_REGISTER));
    // single major loop
    DMA_TCD0_CITER_ELINKNO = 1;
    DMA_TCD0_BITER_ELINKNO = 1;
    // link channel 1, enable major channel-to-channel linking, don't clear enable on major loop complete
    DMA_TCD0_CSR = (1 << 8) | (1 << 5);
    DMAMUX0_CHCFG0 = DMAMUX_SOURCE_LATCH_RISING_EDGE | DMAMUX_ENABLE;

    // DMA channel #1 - copy address values from current position in array to buffer to temporarily hold row values for the next timer cycle
    // only use single major loop, never disable channel
    DMA_TCD1_SADDR = &matrixUpdateBlocks[0][0].addressValues;
    DMA_TCD1_SOFF = sizeof(uint16_t);
    DMA_TCD1_SLAST = sizeof(matrixUpdateBlock) - (ADDRESS_ARRAY_REGISTERS_TO_UPDATE * sizeof(uint16_t));
    DMA_TCD1_ATTR = DMA_TCD_ATTR_SSIZE(1) | DMA_TCD_ATTR_DSIZE(1);
    // 16-bit = 2 bytes transferred
    // transfer two 16-bit values, reset destination address back after each minor loop
    DMA_TCD1_NBYTES_MLOFFNO = (ADDRESS_ARRAY_REGISTERS_TO_UPDATE * sizeof(uint16_t));
    // start with the register that's the highest location in memory and make offset decrement to avoid negative number in NBYTES_MLOFFYES register (TODO: can switch order by masking negative offset)
    DMA_TCD1_DADDR = &gpiosync.gpio_pcor;
    DMA_TCD1_DOFF = (int)&gpiosync.gpio_psor - (int)&gpiosync.gpio_pcor;
    DMA_TCD1_DLASTSGA = (ADDRESS_ARRAY_REGISTERS_TO_UPDATE * ((int)&gpiosync.gpio_pcor - (int)&gpiosync.gpio_psor));
    // no minor loop linking, single major loop, single minor loop, don't clear enable after major loop complete
    DMA_TCD1_CITER_ELINKNO = 1;
    DMA_TCD1_BITER_ELINKNO = 1;
    DMA_TCD1_CSR = 0;

    // DMA channel #2 - on latch falling edge, load FTM1_CV1 and FTM1_MOD with with next values from current block
    // only use single major loop, never disable channel
    // link to channel 3 when complete
#define TIMER_REGISTERS_TO_UPDATE   2
    DMA_TCD2_SADDR = &matrixUpdateBlocks[0][0].timerValues.timer_oe;
    DMA_TCD2_SOFF = sizeof(uint16_t);
    DMA_TCD2_SLAST = sizeof(matrixUpdateBlock) - (TIMER_REGISTERS_TO_UPDATE * sizeof(uint16_t));
    DMA_TCD2_ATTR = DMA_TCD_ATTR_SSIZE(1) | DMA_TCD_ATTR_DSIZE(1);
    // 16-bit = 2 bytes transferred
    DMA_TCD2_NBYTES_MLOFFNO = TIMER_REGISTERS_TO_UPDATE * sizeof(uint16_t);
    DMA_TCD2_DADDR = &FTM1_C1V;
    DMA_TCD2_DOFF = (int)&FTM1_MOD - (int)&FTM1_C1V;
    DMA_TCD2_DLASTSGA = TIMER_REGISTERS_TO_UPDATE * ((int)&FTM1_C1V - (int)&FTM1_MOD);
    // no minor loop linking, single major loop
    DMA_TCD2_CITER_ELINKNO = 1;
    DMA_TCD2_BITER_ELINKNO = 1;
    // link channel 3, enable major channel-to-channel linking, don't clear enable after major loop complete
    DMA_TCD2_CSR = (3 << 8) | (1 << 5);
    DMAMUX0_CHCFG2 = DMAMUX_SOURCE_LATCH_FALLING_EDGE | DMAMUX_ENABLE;

#define DMA_TCD_MLOFF_MASK  (0x3FFFFC00)

    // DMA channel #3 - repeatedly load gpio_array into GPIOD_PDOR, stop and int on major loop complete
    DMA_TCD3_SADDR = matrixUpdateData[0][0];
    DMA_TCD3_SOFF = sizeof(matrixUpdateData[0][0]) / 2;
    // SADDR will get updated by ISR, no need to set SLAST
    DMA_TCD3_SLAST = 0;
    DMA_TCD3_ATTR = DMA_TCD_ATTR_SSIZE(0) | DMA_TCD_ATTR_DSIZE(0);
    // after each minor loop, set source to point back to the beginning of this set of data,
    // but advance by 1 byte to get the next significant bits data
    DMA_TCD3_NBYTES_MLOFFYES = DMA_TCD_NBYTES_SMLOE |
                               (((1 - sizeof(matrixUpdateData[0])) << 10) & DMA_TCD_MLOFF_MASK) |
                               (MATRIX_WIDTH * DMA_UPDATES_PER_CLOCK);
    DMA_TCD3_DADDR = &GPIOD_PDOR;
    DMA_TCD3_DOFF = 0;
    DMA_TCD3_DLASTSGA = 0;
    DMA_TCD3_CITER_ELINKNO = LATCHES_PER_ROW;
    DMA_TCD3_BITER_ELINKNO = LATCHES_PER_ROW;
    // int after major loop is complete
    DMA_TCD3_CSR = DMA_TCD_CSR_INTMAJOR;
    // for debugging - enable bandwidth control (space out GPIO updates so they can be seen easier on a low-bandwidth logic analyzer)
    //DMA_TCD3_CSR |= (0x02 << 14);

    // enable a done interrupt when all DMA operations are complete
    NVIC_ENABLE_IRQ(IRQ_DMA_CH3);

    // enable additional dma interrupt used as software interrupt
    NVIC_SET_PRIORITY(IRQ_DMA_CH1, 0xFF); // 0xFF = lowest priority
    NVIC_ENABLE_IRQ(IRQ_DMA_CH1);

    // enable channels 0, 1, 2, 3
    DMA_ERQ = (1 << 0) | (1 << 1) | (1 << 2) | (1 << 3);

    // at the end after everything is set up: enable timer from system clock, with appropriate prescale
    FTM1_SC = FTM_SC_CLKS(1) | FTM_SC_PS(LATCH_TIMER_PRESCALE);
}

void SmartMatrix::loadMatrixBuffers(unsigned char currentRow) {

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

    for (i = 0; i < MATRIX_WIDTH; i++) {
        if (!getForegroundPixel(i, currentRow, &tempPixel0))
            SmartMatrix::getPixel(i, currentRow, &tempPixel0);
        if (!getForegroundPixel(i, currentRow + MATRIX_ROW_PAIR_OFFSET, &tempPixel1))
            SmartMatrix::getPixel(i, currentRow + MATRIX_ROW_PAIR_OFFSET, &tempPixel1);

        uint8_t temp0red = colorCorrection8bit(tempPixel0.red);
        uint8_t temp0green = colorCorrection8bit(tempPixel0.green);
        uint8_t temp0blue = colorCorrection8bit(tempPixel0.blue);
        uint8_t temp1red = colorCorrection8bit(tempPixel1.red);
        uint8_t temp1green = colorCorrection8bit(tempPixel1.green);
        uint8_t temp1blue = colorCorrection8bit(tempPixel1.blue);

        // this technique is from Fadecandy
        union {
            uint32_t word;
            struct {
                // order of bits in word matches how GPIO connects to the display
                uint32_t GPIO_WORD_ORDER;
            };
        } o0, o1, clkset;

        // set bits starting from LSB brightness moving to MSB brightness with each byte across the word
        // each word contains four brightness levels for single set of pixels above
        o0.p0clk = 0;
        o0.p0pad = 0;
        o0.p0b1 = temp0blue    >> 0;
        o0.p0r1 = temp0red     >> 0;
        o0.p0r2 = temp1red     >> 0;
        o0.p0g1 = temp0green   >> 0;
        o0.p0g2 = temp1green   >> 0;
        o0.p0b2 = temp1blue    >> 0;

        o0.p1clk = 0;
        o0.p1pad = 0;
        o0.p1b1 = temp0blue    >> 1;
        o0.p1r1 = temp0red     >> 1;
        o0.p1r2 = temp1red     >> 1;
        o0.p1g1 = temp0green   >> 1;
        o0.p1g2 = temp1green   >> 1;
        o0.p1b2 = temp1blue    >> 1;

        o0.p2clk = 0;
        o0.p2pad = 0;
        o0.p2b1 = temp0blue    >> 2;
        o0.p2r1 = temp0red     >> 2;
        o0.p2r2 = temp1red     >> 2;
        o0.p2g1 = temp0green   >> 2;
        o0.p2g2 = temp1green   >> 2;
        o0.p2b2 = temp1blue    >> 2;

        o0.p3clk = 0;
        o0.p3pad = 0;
        o0.p3b1 = temp0blue    >> 3;
        o0.p3r1 = temp0red     >> 3;
        o0.p3r2 = temp1red     >> 3;
        o0.p3g1 = temp0green   >> 3;
        o0.p3g2 = temp1green   >> 3;
        o0.p3b2 = temp1blue    >> 3;


        // continue moving from LSB to MSB brightness with the next word
        o1.p0clk = 0;
        o1.p0pad = 0;
        o1.p0b1 = temp0blue    >> (0 + 1 * sizeof(uint32_t));
        o1.p0r1 = temp0red     >> (0 + 1 * sizeof(uint32_t));
        o1.p0r2 = temp1red     >> (0 + 1 * sizeof(uint32_t));
        o1.p0g1 = temp0green   >> (0 + 1 * sizeof(uint32_t));
        o1.p0g2 = temp1green   >> (0 + 1 * sizeof(uint32_t));
        o1.p0b2 = temp1blue    >> (0 + 1 * sizeof(uint32_t));

        o1.p1clk = 0;
        o1.p1pad = 0;
        o1.p1b1 = temp0blue    >> (1 + 1 * sizeof(uint32_t));
        o1.p1r1 = temp0red     >> (1 + 1 * sizeof(uint32_t));
        o1.p1r2 = temp1red     >> (1 + 1 * sizeof(uint32_t));
        o1.p1g1 = temp0green   >> (1 + 1 * sizeof(uint32_t));
        o1.p1g2 = temp1green   >> (1 + 1 * sizeof(uint32_t));
        o1.p1b2 = temp1blue    >> (1 + 1 * sizeof(uint32_t));

        o1.p2clk = 0;
        o1.p2pad = 0;
        o1.p2b1 = temp0blue    >> (2 + 1 * sizeof(uint32_t));
        o1.p2r1 = temp0red     >> (2 + 1 * sizeof(uint32_t));
        o1.p2r2 = temp1red     >> (2 + 1 * sizeof(uint32_t));
        o1.p2g1 = temp0green   >> (2 + 1 * sizeof(uint32_t));
        o1.p2g2 = temp1green   >> (2 + 1 * sizeof(uint32_t));
        o1.p2b2 = temp1blue    >> (2 + 1 * sizeof(uint32_t));

        o1.p3clk = 0;
        o1.p3pad = 0;
        o1.p3b1 = temp0blue    >> (3 + 1 * sizeof(uint32_t));
        o1.p3r1 = temp0red     >> (3 + 1 * sizeof(uint32_t));
        o1.p3r2 = temp1red     >> (3 + 1 * sizeof(uint32_t));
        o1.p3g1 = temp0green   >> (3 + 1 * sizeof(uint32_t));
        o1.p3g2 = temp1green   >> (3 + 1 * sizeof(uint32_t));
        o1.p3b2 = temp1blue    >> (3 + 1 * sizeof(uint32_t));

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
    }

}

void dma_ch1_isr(void) {
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
void dma_ch3_isr(void) {
#ifdef DEBUG_PINS_ENABLED
    digitalWriteFast(DEBUG_PIN_1, HIGH); // oscilloscope trigger
#endif
    // done with previous row, mark it as read
    cbRead(&dmaBuffer);

    // get next row to draw to display and update DMA pointers
    int currentRow = cbGetNextRead(&dmaBuffer);
    DMA_TCD1_SADDR = &matrixUpdateBlocks[currentRow][0].addressValues;
    DMA_TCD2_SADDR = &matrixUpdateBlocks[currentRow][0].timerValues.timer_oe;
    DMA_TCD3_SADDR = matrixUpdateData[currentRow][0];

    // clear pending GPIO int for PORTA before enabling DMA again
    CORE_PIN3_CONFIG |= (1 << 24);

    // trigger software interrupt (DMA channel used instead of actual softint)
    NVIC_SET_PENDING(IRQ_DMA_CH1);

    // clear pending int
    DMA_CINT = 3;

#ifdef DEBUG_PINS_ENABLED
    digitalWriteFast(DEBUG_PIN_1, LOW); // oscilloscope trigger
#endif
}
