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
#include "CircularBuffer.h"
#include "DMAChannel.h"

#define INLINE __attribute__( ( always_inline ) ) inline

#define MATRIX_STACK_HEIGHT (matrixHeight / matrixPanelHeight)

#define ROW_CALCULATION_ISR_PRIORITY   0xFE // 0xFF = lowest priority

// hardware-specific definitions
// prescale of 1 is F_BUS/2
#define LATCH_TIMER_PRESCALE  0x01
#define TIMER_FREQUENCY     (F_BUS/2)
#define NS_TO_TICKS(X)      (uint32_t)(TIMER_FREQUENCY * ((X) / 1000000000.0))
#define LATCH_TIMER_PULSE_WIDTH_TICKS   NS_TO_TICKS(LATCH_TIMER_PULSE_WIDTH_NS)
#define TICKS_PER_ROW   (TIMER_FREQUENCY/refreshRate/matrixRowsPerFrame)
#define IDEAL_MSB_BLOCK_TICKS     (TICKS_PER_ROW/2)
#define MIN_BLOCK_PERIOD_NS (LATCH_TO_CLK_DELAY_NS + ((PANEL_32_PIXELDATA_TRANSFER_MAXIMUM_NS*PIXELS_PER_LATCH)/32))
#define MIN_BLOCK_PERIOD_TICKS NS_TO_TICKS(MIN_BLOCK_PERIOD_NS)
#define PIXELS_PER_LATCH    ((matrixWidth * matrixHeight) / matrixPanelHeight)

// slower refresh rates require larger timer values - get the min refresh rate from the largest MSB value that will fit in the timer (round up)
#define MIN_REFRESH_RATE    (((TIMER_FREQUENCY/65535)/16/2) + 1)

#define TIMER_REGISTERS_TO_UPDATE   2

extern DMAChannel dmaOutputAddress;
extern DMAChannel dmaUpdateAddress;
extern DMAChannel dmaUpdateTimer;
extern DMAChannel dmaClockOutData;

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
const int SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::matrixPanelHeight = CONVERT_PANELTYPE_TO_MATRIXPANELHEIGHT(panelType);
template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
const int SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::matrixRowPairOffset = CONVERT_PANELTYPE_TO_MATRIXROWPAIROFFSET(panelType);
template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
const int SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::matrixRowsPerFrame = CONVERT_PANELTYPE_TO_MATRIXROWSPERFRAME(panelType);


template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
void rowShiftCompleteISR(void);
template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
void rowCalculationISR(void);


extern CircularBuffer dmaBuffer;

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>* SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::globalinstance;
// dmaBufferNumRows = the size of the buffer that DMA pulls from to refresh the display
// must be minimum 2 rows so one can be updated while the other is refreshed
// increase beyond two to give more time for the update routine to complete
// (increase this number if non-DMA interrupts are causing display problems)
template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
uint8_t SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::dmaBufferNumRows;
template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
uint8_t SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::dmaBufferBytesPerPixel;
template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
uint16_t SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::dmaBufferBytesPerRow;
template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
uint8_t SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::refreshRate = 120;


// todo: just use a single buffer for Blocks/LUT/Data?
template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
matrixUpdateBlock * SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::matrixUpdateBlocks;    // array is size dmaBufferNumRows * latchesPerRow
template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
addresspair * SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::addressLUT;      // array is size rowsPerFrame
template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
timerpair * SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::timerLUT;          // array is size latchesPerRow

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
timerpair * SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::timerPairIdle;

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
volatile bool SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::dmaBufferUnderrun = false;

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
bool SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::dmaBufferUnderrunSinceLastCheck = false;

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
bool SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::refreshRateLowered = false;

// set to true initially so all layers get the initial refresh rate
template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
bool SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::refreshRateChanged = true;


/*
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
uint32_t * SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::matrixUpdateData;

#define ADDRESS_ARRAY_REGISTERS_TO_UPDATE   2

// 2x uint32_t to match size and spacing of values it is updating: GPIOx_PSOR and GPIOx_PCOR are 32-bit and adjacent to each other
typedef struct gpiopair {
    uint32_t  gpio_psor;
    uint32_t  gpio_pcor;
} gpiopair;

static gpiopair gpiosync;


template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::SmartMatrix3(uint8_t bufferrows, uint32_t * dataBuffer, uint8_t * blockBuffer) {
    SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::globalinstance = this;
    dmaBufferNumRows = bufferrows;
    dmaBufferBytesPerPixel = latchesPerRow * DMA_UPDATES_PER_CLOCK;
    dmaBufferBytesPerRow = dmaBufferBytesPerPixel * PIXELS_PER_LATCH;

    matrixUpdateData = dataBuffer;
    // single buffer is divided up to hold matrixUpdateBlocks, addressLUT, timerLUT to simplify user sketch code and reduce constructor parameters
    matrixUpdateBlocks = (matrixUpdateBlock*)blockBuffer;
    blockBuffer += sizeof(matrixUpdateBlock) * dmaBufferNumRows * latchesPerRow;
    addressLUT = (addresspair*)blockBuffer;
    blockBuffer += sizeof(addresspair) * matrixRowsPerFrame;
    timerLUT = (timerpair*)blockBuffer;
    blockBuffer += sizeof(timerpair) * latchesPerRow;
    timerPairIdle = (timerpair*)blockBuffer;
    timerPairIdle->timer_period = MIN_BLOCK_PERIOD_TICKS;
    timerPairIdle->timer_oe = MIN_BLOCK_PERIOD_TICKS;
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
void SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::addLayer(SM_Layer * newlayer) {
    if(baseLayer) {
        SM_Layer * templayer = baseLayer;
        while(templayer->nextLayer)
            templayer = templayer->nextLayer;
        templayer->nextLayer = newlayer;
    } else {
        baseLayer = newlayer;
    }
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
void SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::countFPS(void) {
  static long loops = 0;
  static long lastMillis = 0;
  long currentMillis = millis();

  loops++;
  if(currentMillis - lastMillis >= 1000){
#if defined(USB_SERIAL)
    if(Serial) {
        Serial.print("Loops last second:");
        Serial.println(loops);
    }
#endif    
    lastMillis = currentMillis;
    loops = 0;
  }
}

#define MAX_MATRIXCALCULATIONS_LOOPS_WITHOUT_EXIT  5

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
INLINE void SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::matrixCalculations(bool initial) {
    static unsigned char currentRow = 0;
    unsigned char numLoopsWithoutExit = 0;

    // only run the loop if there is free space, and fill the entire buffer before returning
    while (!cbIsFull(&dmaBuffer)) {
        // check to see if the refresh rate is too high, and the application doesn't have time to run
        if(++numLoopsWithoutExit > MAX_MATRIXCALCULATIONS_LOOPS_WITHOUT_EXIT) {

            // minimum set to avoid overflowing timer at low refresh rates
            if(!initial && refreshRate > MIN_REFRESH_RATE) {
                refreshRate--;
                calculateTimerLut();
                refreshRateLowered = true;
                refreshRateChanged = true;
            }

            initial = false;
            numLoopsWithoutExit = 0;
        }

        // do once-per-frame updates
        if (!currentRow) {
            if (rotationChange) {
                SM_Layer * templayer = globalinstance->baseLayer;
                while(templayer) {
                    templayer->setRotation(rotation);
                    templayer = templayer->nextLayer;
                }
                rotationChange = false;
            }

            SM_Layer * templayer = globalinstance->baseLayer;
            while(templayer) {
                if(refreshRateChanged) {
                    templayer->setRefreshRate(refreshRate);
                }
                templayer->frameRefreshCallback();
                templayer = templayer->nextLayer;
            }
            refreshRateChanged = false;
            if (brightnessChange) {
                calculateTimerLut();
                brightnessChange = false;
            }
        }

        // do once-per-line updates
        // none right now

        // enqueue row
        SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::loadMatrixBuffers(currentRow);
        cbWrite(&dmaBuffer);

        if (++currentRow >= matrixRowsPerFrame)
            currentRow = 0;

        if(dmaBufferUnderrun) {
            // if refreshrate is too high, lower - minimum set to avoid overflowing timer at low refresh rates
            if(refreshRate > MIN_REFRESH_RATE) {
                refreshRate--;
                calculateTimerLut();
                refreshRateLowered = true;
                refreshRateChanged = true;
            }

            // stop timer
            FTM1_SC = FTM_SC_CLKS(0) | FTM_SC_PS(LATCH_TIMER_PRESCALE);

            // point DMA addresses to the next buffer
            int currentRow = cbGetNextRead(&dmaBuffer);
            dmaUpdateAddress.TCD->SADDR = &((matrixUpdateBlock*)SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::matrixUpdateBlocks + (currentRow * SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::latchesPerRow))->addressValues;
            dmaUpdateTimer.TCD->SADDR = &((matrixUpdateBlock*)SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::matrixUpdateBlocks + (currentRow * SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::latchesPerRow))->timerValues.timer_oe;
            dmaClockOutData.TCD->SADDR = (uint8_t*)SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::matrixUpdateData + (currentRow * SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::dmaBufferBytesPerRow);

            // enable channel-to-channel linking so data will be shifted out
            dmaUpdateTimer.TCD->CSR &= ~(1 << 7);  // must clear DONE flag before enabling
            dmaUpdateTimer.TCD->CSR |= (1 << 5);
            // set timer increment back to read from matrixUpdateBlocks
            dmaUpdateTimer.TCD->SLAST = sizeof(matrixUpdateBlock) - (TIMER_REGISTERS_TO_UPDATE * sizeof(uint16_t));

            dmaBufferUnderrunSinceLastCheck = true;
            dmaBufferUnderrun = false;

            // start timer again - next timer period is MIN_BLOCK_PERIOD_TICKS with OE disabled, period after that will be loaded from matrixUpdateBlock
            FTM1_SC = FTM_SC_CLKS(1) | FTM_SC_PS(LATCH_TIMER_PRESCALE);
        }
    }
}

#define MSB_BLOCK_TICKS_ADJUSTMENT_INCREMENT    10

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
void SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::calculateTimerLut(void) {
    int i;
    uint32_t ticksUsed;
    uint16_t msbBlockTicks = IDEAL_MSB_BLOCK_TICKS + MSB_BLOCK_TICKS_ADJUSTMENT_INCREMENT;

    // start with ideal width of the MSB, and keep lowering until the width of all bits fits within TICKS_PER_ROW
    do {
        ticksUsed = 0;
        msbBlockTicks -= MSB_BLOCK_TICKS_ADJUSTMENT_INCREMENT;
        for (i = 0; i < latchesPerRow; i++) {
            uint16_t blockTicks = (msbBlockTicks >> (latchesPerRow - i - 1)) + LATCH_TIMER_PULSE_WIDTH_TICKS;

            if (blockTicks < MIN_BLOCK_PERIOD_TICKS)
                blockTicks = MIN_BLOCK_PERIOD_TICKS;

            ticksUsed += blockTicks;
        }
    } while (ticksUsed > TICKS_PER_ROW);

    for (i = 0; i < latchesPerRow; i++) {
        // set period and OE values for current block - going from smallest timer values to largest
        // order needs to be smallest to largest so the last update of the row has the largest time between
        // the falling edge of the latch and the rising edge of the latch on the next row - an ISR
        // updates the row in this time

        // period is max on time for this block, plus the dead time while the latch is high
        uint16_t period = (msbBlockTicks >> (latchesPerRow - i - 1)) + LATCH_TIMER_PULSE_WIDTH_TICKS;
        // on-time is the max on-time * dimming factor, plus the dead time while the latch is high
        uint16_t ontime = (((msbBlockTicks >> (latchesPerRow - i - 1)) * dimmingFactor) / dimmingMaximum) + LATCH_TIMER_PULSE_WIDTH_TICKS;

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

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
void SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::setRotation(rotationDegrees newrotation) {
    rotation = newrotation;
    rotationChange = true;
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
uint16_t SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::getScreenWidth(void) const {
    if (rotation == rotation0 || rotation == rotation180) {
        return matrixWidth;
    } else {
        return matrixHeight;
    }
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
uint16_t SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::getScreenHeight(void) const {
    if (rotation == rotation0 || rotation == rotation180) {
        return matrixHeight;
    } else {
        return matrixWidth;
    }
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
volatile bool SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::brightnessChange = false;
template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
volatile bool SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::rotationChange = true;
template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
rotationDegrees SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::rotation = rotation0;

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
const int SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::dimmingMaximum = 255;
// large factor = more dim, default is full brightness
template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
int SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::dimmingFactor = dimmingMaximum - (100 * 255)/100;

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
void SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::setBrightness(uint8_t brightness) {
    dimmingFactor = dimmingMaximum - brightness;
    brightnessChange = true;
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
void SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::setRefreshRate(uint8_t newRefreshRate) {
    if(newRefreshRate > MIN_REFRESH_RATE)
        refreshRate = newRefreshRate;
    else
        refreshRate = MIN_REFRESH_RATE;
    refreshRateChanged = true;
    calculateTimerLut();
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
uint8_t SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::getRefreshRate(void) {
    return refreshRate;
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
bool SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::getdmaBufferUnderrunFlag(void) {
    if(dmaBufferUnderrunSinceLastCheck) {
        dmaBufferUnderrunSinceLastCheck = false;
        return true;
    }
    return false;
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
bool SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::getRefreshRateLoweredFlag(void) {
    if(refreshRateLowered) {
        refreshRateLowered = false;
        return true;
    }
    return false;
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
void SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::begin(void)
{
    int i;
    cbInit(&dmaBuffer, dmaBufferNumRows);

    // fill addressLUT
    for (i = 0; i < matrixRowsPerFrame; i++) {

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

    // completely fill buffer with data before enabling DMA
    matrixCalculations(true);

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
    FTM1_MOD = IDEAL_MSB_BLOCK_TICKS;

    // setup FTM1 compares:
    // latch pulse width set based on max time to update address pins
    FTM1_C0V = LATCH_TIMER_PULSE_WIDTH_TICKS;
    // output OE signal - set to max at first to disable OE
    FTM1_C1V = IDEAL_MSB_BLOCK_TICKS;

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
    dmaUpdateAddress.TCD->SADDR = &((matrixUpdateBlock*)matrixUpdateBlocks)->addressValues;
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
    dmaUpdateTimer.TCD->SADDR = &((matrixUpdateBlock*)matrixUpdateBlocks)->timerValues.timer_oe;
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
    dmaClockOutData.TCD->SADDR = matrixUpdateData;
    dmaClockOutData.TCD->SOFF = latchesPerRow;
    // SADDR will get updated by ISR, no need to set SLAST
    dmaClockOutData.TCD->SLAST = 0;
    dmaClockOutData.TCD->ATTR = DMA_TCD_ATTR_SSIZE(0) | DMA_TCD_ATTR_DSIZE(0);
    // after each minor loop, set source to point back to the beginning of this set of data,
    // but advance by 1 byte to get the next significant bits data
    dmaClockOutData.TCD->NBYTES_MLOFFYES = DMA_TCD_NBYTES_SMLOE |
                               (((1 - (dmaBufferBytesPerPixel * PIXELS_PER_LATCH)) << 10) & DMA_TCD_MLOFF_MASK) |
                               (PIXELS_PER_LATCH * DMA_UPDATES_PER_CLOCK);
    dmaClockOutData.TCD->DADDR = &GPIOD_PDOR;
    dmaClockOutData.TCD->DOFF = 0;
    dmaClockOutData.TCD->DLASTSGA = 0;
    dmaClockOutData.TCD->CITER_ELINKNO = latchesPerRow;
    dmaClockOutData.TCD->BITER_ELINKNO = latchesPerRow;
    // int after major loop is complete
    dmaClockOutData.TCD->CSR = DMA_TCD_CSR_INTMAJOR;
    // for debugging - enable bandwidth control (space out GPIO updates so they can be seen easier on a low-bandwidth logic analyzer)
    //dmaClockOutData.TCD->CSR |= (0x02 << 14);

    // enable a done interrupt when all DMA operations are complete
    dmaClockOutData.attachInterrupt(rowShiftCompleteISR<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>);

    // enable additional dma interrupt used as software interrupt
    NVIC_SET_PRIORITY(IRQ_DMA_CH0 + dmaUpdateAddress.channel, ROW_CALCULATION_ISR_PRIORITY);
    dmaUpdateAddress.attachInterrupt(rowCalculationISR<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>);

    dmaOutputAddress.enable();
    dmaUpdateAddress.enable();
    dmaUpdateTimer.enable();
    dmaClockOutData.enable();

    // at the end after everything is set up: enable timer from system clock, with appropriate prescale
    FTM1_SC = FTM_SC_CLKS(1) | FTM_SC_PS(LATCH_TIMER_PRESCALE);
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
INLINE void SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::loadMatrixBuffers48(unsigned char currentRow, unsigned char freeRowBuffer) {
    int i;

    // static to avoid putting large buffer on the stack
    static rgb48 tempRow0[PIXELS_PER_LATCH];
    static rgb48 tempRow1[PIXELS_PER_LATCH];

    // clear buffer to prevent garbage data showing through transparent layers
    memset(tempRow0, 0x00, sizeof(tempRow0));
    memset(tempRow1, 0x00, sizeof(tempRow1));

    // get pixel data from layers
    SM_Layer * templayer = globalinstance->baseLayer;
    while(templayer) {
        for(i=0; i<MATRIX_STACK_HEIGHT; i++) {
            // Z-shape, bottom to top
            if(!(optionFlags & SMARTMATRIX_OPTIONS_C_SHAPE_STACKING) &&
                (optionFlags & SMARTMATRIX_OPTIONS_BOTTOM_TO_TOP_STACKING)) {
                // fill data from bottom to top, so bottom panel is the one closest to Teensy
                templayer->fillRefreshRow(currentRow + (MATRIX_STACK_HEIGHT-i-1)*matrixPanelHeight, &tempRow0[i*matrixWidth]);
                templayer->fillRefreshRow(currentRow + matrixRowPairOffset + (MATRIX_STACK_HEIGHT-i-1)*matrixPanelHeight, &tempRow1[i*matrixWidth]);
            // Z-shape, top to bottom
            } else if(!(optionFlags & SMARTMATRIX_OPTIONS_C_SHAPE_STACKING) &&
                !(optionFlags & SMARTMATRIX_OPTIONS_BOTTOM_TO_TOP_STACKING)) {
                // fill data from top to bottom, so top panel is the one closest to Teensy
                templayer->fillRefreshRow(currentRow + i*matrixPanelHeight, &tempRow0[i*matrixWidth]);
                templayer->fillRefreshRow(currentRow + matrixRowPairOffset + i*matrixPanelHeight, &tempRow1[i*matrixWidth]);
            // C-shape, bottom to top
            } else if((optionFlags & SMARTMATRIX_OPTIONS_C_SHAPE_STACKING) &&
                (optionFlags & SMARTMATRIX_OPTIONS_BOTTOM_TO_TOP_STACKING)) {
                // alternate direction of filling (or loading) for each matrixwidth
                // swap row order from top to bottom for each stack (tempRow1 filled with top half of panel, tempRow0 filled with bottom half)
                if((MATRIX_STACK_HEIGHT-i+1)%2) {
                    templayer->fillRefreshRow((matrixRowsPerFrame-currentRow-1) + matrixRowPairOffset + (i)*matrixPanelHeight, &tempRow0[i*matrixWidth]);
                    templayer->fillRefreshRow((matrixRowsPerFrame-currentRow-1) + (i)*matrixPanelHeight, &tempRow1[i*matrixWidth]);
                } else {
                    templayer->fillRefreshRow(currentRow + (i)*matrixPanelHeight, &tempRow0[i*matrixWidth]);
                    templayer->fillRefreshRow(currentRow + matrixRowPairOffset + (i)*matrixPanelHeight, &tempRow1[i*matrixWidth]);
                }
            // C-shape, top to bottom
            } else if((optionFlags & SMARTMATRIX_OPTIONS_C_SHAPE_STACKING) && 
                !(optionFlags & SMARTMATRIX_OPTIONS_BOTTOM_TO_TOP_STACKING)) {
                if((MATRIX_STACK_HEIGHT-i)%2) {
                    templayer->fillRefreshRow(currentRow + (MATRIX_STACK_HEIGHT-i-1)*matrixPanelHeight, &tempRow0[i*matrixWidth]);
                    templayer->fillRefreshRow(currentRow + matrixRowPairOffset + (MATRIX_STACK_HEIGHT-i-1)*matrixPanelHeight, &tempRow1[i*matrixWidth]);
                } else {
                    templayer->fillRefreshRow((matrixRowsPerFrame-currentRow-1) + matrixRowPairOffset + (MATRIX_STACK_HEIGHT-i-1)*matrixPanelHeight, &tempRow0[i*matrixWidth]);
                    templayer->fillRefreshRow((matrixRowsPerFrame-currentRow-1) + (MATRIX_STACK_HEIGHT-i-1)*matrixPanelHeight, &tempRow1[i*matrixWidth]);
                }
            }
        }
        templayer = templayer->nextLayer;        
    }

    for (i = 0; i < PIXELS_PER_LATCH; i++) {
        uint16_t temp0red,temp0green,temp0blue,temp1red,temp1green,temp1blue;

        // for upside down stacks, flip order
        if((optionFlags & SMARTMATRIX_OPTIONS_C_SHAPE_STACKING) && ((PIXELS_PER_LATCH-i-1)/matrixWidth)) {
            int tempPosition = ((i/matrixWidth) * matrixWidth) + matrixWidth - i%matrixWidth - 1;
            temp0red = tempRow0[tempPosition].red;
            temp0green = tempRow0[tempPosition].green;
            temp0blue = tempRow0[tempPosition].blue;
            temp1red = tempRow1[tempPosition].red;
            temp1green = tempRow1[tempPosition].green;
            temp1blue = tempRow1[tempPosition].blue;
        } else {
            temp0red = tempRow0[i].red;
            temp0green = tempRow0[i].green;
            temp0blue = tempRow0[i].blue;
            temp1red = tempRow1[i].red;
            temp1green = tempRow1[i].green;
            temp1blue = tempRow1[i].blue;
        }


#if 0
        if(latchesPerRow == 12) {
            temp0red >>= 4;
            temp0green >>= 4;
            temp0blue >>= 4;

            temp1red >>= 4;
            temp1green >>= 4;
            temp1blue >>= 4;
        }

        if(latchesPerRow == 8) {
            temp0red >>= 8;
            temp0green >>= 8;
            temp0blue >>= 8;

            temp1red >>= 8;
            temp1green >>= 8;
            temp1blue >>= 8;
        }
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

        clkset.word = 0x00;
        clkset.p0clk = 1;
        clkset.p1clk = 1;
        clkset.p2clk = 1;
        clkset.p3clk = 1;

        // copy words to DMA buffer as a pair, one with clock set low, next with clock set high

        uint32_t * tempptr = (uint32_t*)matrixUpdateData + ((freeRowBuffer*dmaBufferBytesPerRow)/sizeof(uint32_t)) + ((i*dmaBufferBytesPerPixel)/sizeof(uint32_t));
        *tempptr = o0.word;
        *(tempptr + latchesPerRow/sizeof(uint32_t)) = o0.word | clkset.word;
        *(++tempptr) = o1.word;
        *(tempptr + latchesPerRow/sizeof(uint32_t)) = o1.word | clkset.word;

        //if(latchesPerRow >= 12) {
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

            *(++tempptr) = o2.word;
            *(tempptr + latchesPerRow/sizeof(uint32_t)) = o2.word | clkset.word;
        //}

        //if(latchesPerRow == 16) {
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

            *(++tempptr) = o3.word;
            *(tempptr + latchesPerRow/sizeof(uint32_t)) = o3.word | clkset.word;
        //}
    }
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
INLINE void SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::loadMatrixBuffers36(unsigned char currentRow, unsigned char freeRowBuffer) {
    int i;

    // static to avoid putting large buffer on the stack
    static rgb48 tempRow0[PIXELS_PER_LATCH];
    static rgb48 tempRow1[PIXELS_PER_LATCH];

    // clear buffer to prevent garbage data showing through transparent layers
    memset(tempRow0, 0x00, sizeof(tempRow0));
    memset(tempRow1, 0x00, sizeof(tempRow1));

    // get pixel data from layers
    SM_Layer * templayer = globalinstance->baseLayer;
    while(templayer) {
        for(i=0; i<MATRIX_STACK_HEIGHT; i++) {
            // Z-shape, bottom to top
            if(!(optionFlags & SMARTMATRIX_OPTIONS_C_SHAPE_STACKING) &&
                (optionFlags & SMARTMATRIX_OPTIONS_BOTTOM_TO_TOP_STACKING)) {
                // fill data from bottom to top, so bottom panel is the one closest to Teensy
                templayer->fillRefreshRow(currentRow + (MATRIX_STACK_HEIGHT-i-1)*matrixPanelHeight, &tempRow0[i*matrixWidth]);
                templayer->fillRefreshRow(currentRow + matrixRowPairOffset + (MATRIX_STACK_HEIGHT-i-1)*matrixPanelHeight, &tempRow1[i*matrixWidth]);
            // Z-shape, top to bottom
            } else if(!(optionFlags & SMARTMATRIX_OPTIONS_C_SHAPE_STACKING) &&
                !(optionFlags & SMARTMATRIX_OPTIONS_BOTTOM_TO_TOP_STACKING)) {
                // fill data from top to bottom, so top panel is the one closest to Teensy
                templayer->fillRefreshRow(currentRow + i*matrixPanelHeight, &tempRow0[i*matrixWidth]);
                templayer->fillRefreshRow(currentRow + matrixRowPairOffset + i*matrixPanelHeight, &tempRow1[i*matrixWidth]);
            // C-shape, bottom to top
            } else if((optionFlags & SMARTMATRIX_OPTIONS_C_SHAPE_STACKING) &&
                (optionFlags & SMARTMATRIX_OPTIONS_BOTTOM_TO_TOP_STACKING)) {
                // alternate direction of filling (or loading) for each matrixwidth
                // swap row order from top to bottom for each stack (tempRow1 filled with top half of panel, tempRow0 filled with bottom half)
                if((MATRIX_STACK_HEIGHT-i+1)%2) {
                    templayer->fillRefreshRow((matrixRowsPerFrame-currentRow-1) + matrixRowPairOffset + (i)*matrixPanelHeight, &tempRow0[i*matrixWidth]);
                    templayer->fillRefreshRow((matrixRowsPerFrame-currentRow-1) + (i)*matrixPanelHeight, &tempRow1[i*matrixWidth]);
                } else {
                    templayer->fillRefreshRow(currentRow + (i)*matrixPanelHeight, &tempRow0[i*matrixWidth]);
                    templayer->fillRefreshRow(currentRow + matrixRowPairOffset + (i)*matrixPanelHeight, &tempRow1[i*matrixWidth]);
                }
            // C-shape, top to bottom
            } else if((optionFlags & SMARTMATRIX_OPTIONS_C_SHAPE_STACKING) && 
                !(optionFlags & SMARTMATRIX_OPTIONS_BOTTOM_TO_TOP_STACKING)) {
                if((MATRIX_STACK_HEIGHT-i)%2) {
                    templayer->fillRefreshRow(currentRow + (MATRIX_STACK_HEIGHT-i-1)*matrixPanelHeight, &tempRow0[i*matrixWidth]);
                    templayer->fillRefreshRow(currentRow + matrixRowPairOffset + (MATRIX_STACK_HEIGHT-i-1)*matrixPanelHeight, &tempRow1[i*matrixWidth]);
                } else {
                    templayer->fillRefreshRow((matrixRowsPerFrame-currentRow-1) + matrixRowPairOffset + (MATRIX_STACK_HEIGHT-i-1)*matrixPanelHeight, &tempRow0[i*matrixWidth]);
                    templayer->fillRefreshRow((matrixRowsPerFrame-currentRow-1) + (MATRIX_STACK_HEIGHT-i-1)*matrixPanelHeight, &tempRow1[i*matrixWidth]);
                }
            }
        }
        templayer = templayer->nextLayer;        
    }

    for (i = 0; i < PIXELS_PER_LATCH; i++) {
        uint16_t temp0red,temp0green,temp0blue,temp1red,temp1green,temp1blue;

#ifdef DEBUG_PINS_ENABLED
    digitalWriteFast(DEBUG_PIN_3, HIGH); // oscilloscope trigger
#endif

        // for upside down stacks, flip order
        if((optionFlags & SMARTMATRIX_OPTIONS_C_SHAPE_STACKING) && ((PIXELS_PER_LATCH-i-1)/matrixWidth)) {
            int tempPosition = ((i/matrixWidth) * matrixWidth) + matrixWidth - i%matrixWidth - 1;
            temp0red = tempRow0[tempPosition].red;
            temp0green = tempRow0[tempPosition].green;
            temp0blue = tempRow0[tempPosition].blue;
            temp1red = tempRow1[tempPosition].red;
            temp1green = tempRow1[tempPosition].green;
            temp1blue = tempRow1[tempPosition].blue;
        } else {
            temp0red = tempRow0[i].red;
            temp0green = tempRow0[i].green;
            temp0blue = tempRow0[i].blue;
            temp1red = tempRow1[i].red;
            temp1green = tempRow1[i].green;
            temp1blue = tempRow1[i].blue;
        }

        //if(latchesPerRow == 12) {
            temp0red >>= 4;
            temp0green >>= 4;
            temp0blue >>= 4;

            temp1red >>= 4;
            temp1green >>= 4;
            temp1blue >>= 4;
        //}

#if 0
        if(latchesPerRow == 8) {
            temp0red >>= 8;
            temp0green >>= 8;
            temp0blue >>= 8;

            temp1red >>= 8;
            temp1green >>= 8;
            temp1blue >>= 8;
        }
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

        clkset.word = 0x00;
        clkset.p0clk = 1;
        clkset.p1clk = 1;
        clkset.p2clk = 1;
        clkset.p3clk = 1;

        // copy words to DMA buffer as a pair, one with clock set low, next with clock set high

        uint32_t * tempptr = (uint32_t*)matrixUpdateData + ((freeRowBuffer*dmaBufferBytesPerRow)/sizeof(uint32_t)) + ((i*dmaBufferBytesPerPixel)/sizeof(uint32_t));
        *tempptr = o0.word;
        *(tempptr + latchesPerRow/sizeof(uint32_t)) = o0.word | clkset.word;
        *(++tempptr) = o1.word;
        *(tempptr + latchesPerRow/sizeof(uint32_t)) = o1.word | clkset.word;

        //if(latchesPerRow >= 12) {
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

            *(++tempptr) = o2.word;
            *(tempptr + latchesPerRow/sizeof(uint32_t)) = o2.word | clkset.word;
        //}
#ifdef DEBUG_PINS_ENABLED
    digitalWriteFast(DEBUG_PIN_3, LOW); // oscilloscope trigger
#endif

#if 0
        if(latchesPerRow == 16) {
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

            *(++tempptr) = o3.word;
            *(tempptr + latchesPerRow/sizeof(uint32_t)) = o3.word | clkset.word;
        }
#endif
    }
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
INLINE void SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::loadMatrixBuffers24(unsigned char currentRow, unsigned char freeRowBuffer) {
    int i;

    // static to avoid putting large buffer on the stack
    static rgb24 tempRow0[PIXELS_PER_LATCH];
    static rgb24 tempRow1[PIXELS_PER_LATCH];

    // clear buffer to prevent garbage data showing through transparent layers
    memset(tempRow0, 0x00, sizeof(tempRow0));
    memset(tempRow1, 0x00, sizeof(tempRow1));

    // get pixel data from layers
    SM_Layer * templayer = globalinstance->baseLayer;
    while(templayer) {
        for(i=0; i<MATRIX_STACK_HEIGHT; i++) {
            // Z-shape, bottom to top
            if(!(optionFlags & SMARTMATRIX_OPTIONS_C_SHAPE_STACKING) &&
                (optionFlags & SMARTMATRIX_OPTIONS_BOTTOM_TO_TOP_STACKING)) {
                // fill data from bottom to top, so bottom panel is the one closest to Teensy
                templayer->fillRefreshRow(currentRow + (MATRIX_STACK_HEIGHT-i-1)*matrixPanelHeight, &tempRow0[i*matrixWidth]);
                templayer->fillRefreshRow(currentRow + matrixRowPairOffset + (MATRIX_STACK_HEIGHT-i-1)*matrixPanelHeight, &tempRow1[i*matrixWidth]);
            // Z-shape, top to bottom
            } else if(!(optionFlags & SMARTMATRIX_OPTIONS_C_SHAPE_STACKING) &&
                !(optionFlags & SMARTMATRIX_OPTIONS_BOTTOM_TO_TOP_STACKING)) {
                // fill data from top to bottom, so top panel is the one closest to Teensy
                templayer->fillRefreshRow(currentRow + i*matrixPanelHeight, &tempRow0[i*matrixWidth]);
                templayer->fillRefreshRow(currentRow + matrixRowPairOffset + i*matrixPanelHeight, &tempRow1[i*matrixWidth]);
            // C-shape, bottom to top
            } else if((optionFlags & SMARTMATRIX_OPTIONS_C_SHAPE_STACKING) &&
                (optionFlags & SMARTMATRIX_OPTIONS_BOTTOM_TO_TOP_STACKING)) {
                // alternate direction of filling (or loading) for each matrixwidth
                // swap row order from top to bottom for each stack (tempRow1 filled with top half of panel, tempRow0 filled with bottom half)
                if((MATRIX_STACK_HEIGHT-i+1)%2) {
                    templayer->fillRefreshRow((matrixRowsPerFrame-currentRow-1) + matrixRowPairOffset + (i)*matrixPanelHeight, &tempRow0[i*matrixWidth]);
                    templayer->fillRefreshRow((matrixRowsPerFrame-currentRow-1) + (i)*matrixPanelHeight, &tempRow1[i*matrixWidth]);
                } else {
                    templayer->fillRefreshRow(currentRow + (i)*matrixPanelHeight, &tempRow0[i*matrixWidth]);
                    templayer->fillRefreshRow(currentRow + matrixRowPairOffset + (i)*matrixPanelHeight, &tempRow1[i*matrixWidth]);
                }
            // C-shape, top to bottom
            } else if((optionFlags & SMARTMATRIX_OPTIONS_C_SHAPE_STACKING) && 
                !(optionFlags & SMARTMATRIX_OPTIONS_BOTTOM_TO_TOP_STACKING)) {
                if((MATRIX_STACK_HEIGHT-i)%2) {
                    templayer->fillRefreshRow(currentRow + (MATRIX_STACK_HEIGHT-i-1)*matrixPanelHeight, &tempRow0[i*matrixWidth]);
                    templayer->fillRefreshRow(currentRow + matrixRowPairOffset + (MATRIX_STACK_HEIGHT-i-1)*matrixPanelHeight, &tempRow1[i*matrixWidth]);
                } else {
                    templayer->fillRefreshRow((matrixRowsPerFrame-currentRow-1) + matrixRowPairOffset + (MATRIX_STACK_HEIGHT-i-1)*matrixPanelHeight, &tempRow0[i*matrixWidth]);
                    templayer->fillRefreshRow((matrixRowsPerFrame-currentRow-1) + (MATRIX_STACK_HEIGHT-i-1)*matrixPanelHeight, &tempRow1[i*matrixWidth]);
                }
            }
        }
        templayer = templayer->nextLayer;        
    }

    for (i = 0; i < PIXELS_PER_LATCH; i++) {
        uint8_t temp0red,temp0green,temp0blue,temp1red,temp1green,temp1blue;

        // for upside down stacks, flip order
        if((optionFlags & SMARTMATRIX_OPTIONS_C_SHAPE_STACKING) && ((PIXELS_PER_LATCH-i-1)/matrixWidth)) {
            int tempPosition = ((i/matrixWidth) * matrixWidth) + matrixWidth - i%matrixWidth - 1;
            temp0red = tempRow0[tempPosition].red;
            temp0green = tempRow0[tempPosition].green;
            temp0blue = tempRow0[tempPosition].blue;
            temp1red = tempRow1[tempPosition].red;
            temp1green = tempRow1[tempPosition].green;
            temp1blue = tempRow1[tempPosition].blue;
        } else {
            temp0red = tempRow0[i].red;
            temp0green = tempRow0[i].green;
            temp0blue = tempRow0[i].blue;
            temp1red = tempRow1[i].red;
            temp1green = tempRow1[i].green;
            temp1blue = tempRow1[i].blue;
        }

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

        clkset.word = 0x00;
        clkset.p0clk = 1;
        clkset.p1clk = 1;
        clkset.p2clk = 1;
        clkset.p3clk = 1;

        // copy words to DMA buffer as a pair, one with clock set low, next with clock set high

        uint32_t * tempptr = (uint32_t*)matrixUpdateData + ((freeRowBuffer*dmaBufferBytesPerRow)/sizeof(uint32_t)) + ((i*dmaBufferBytesPerPixel)/sizeof(uint32_t));
        *tempptr = o0.word;
        *(tempptr + latchesPerRow/sizeof(uint32_t)) = o0.word | clkset.word;
        *(++tempptr) = o1.word;
        *(tempptr + latchesPerRow/sizeof(uint32_t)) = o1.word | clkset.word;
    }
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
INLINE void SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::loadMatrixBuffers(unsigned char currentRow) {
    int i;

    addresspair rowAddressPair;

    rowAddressPair.bits_to_set = addressLUT[currentRow].bits_to_set;
    rowAddressPair.bits_to_clear = addressLUT[currentRow].bits_to_clear;

    unsigned char freeRowBuffer = cbGetNextWrite(&dmaBuffer);

    for (i = 0; i < latchesPerRow; i++) {
        matrixUpdateBlock* tempptr = (matrixUpdateBlock*)matrixUpdateBlocks + (freeRowBuffer * latchesPerRow) + i;
        // copy bits to set and clear to generate address for current block
        tempptr->addressValues.bits_to_clear = rowAddressPair.bits_to_clear;
        tempptr->addressValues.bits_to_set = rowAddressPair.bits_to_set;

        tempptr->timerValues.timer_period = timerLUT[i].timer_period;
        tempptr->timerValues.timer_oe = timerLUT[i].timer_oe;
    }

    if(latchesPerRow == 16)
        loadMatrixBuffers48(currentRow, freeRowBuffer);
    else if(latchesPerRow == 12)
        loadMatrixBuffers36(currentRow, freeRowBuffer);
    else if(latchesPerRow == 8)
        loadMatrixBuffers24(currentRow, freeRowBuffer);
}

// low priority ISR triggered by software interrupt on a DMA channel that doesn't need interrupts otherwise
template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
void rowCalculationISR(void) {
#ifdef DEBUG_PINS_ENABLED
    digitalWriteFast(DEBUG_PIN_2, HIGH); // oscilloscope trigger
#endif

    SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::matrixCalculations();

#ifdef DEBUG_PINS_ENABLED
    digitalWriteFast(DEBUG_PIN_2, LOW);
#endif
}

// DMA transfer done (meaning data was shifted and timer value for MSB on current row just got loaded)
// set DMA up for loading the next row, triggered from the next timer latch
template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
void rowShiftCompleteISR(void) {
#ifdef DEBUG_PINS_ENABLED
    digitalWriteFast(DEBUG_PIN_1, HIGH); // oscilloscope trigger
#endif
    // done with previous row, mark it as read
    cbRead(&dmaBuffer);

    if(cbIsEmpty(&dmaBuffer)) {
#ifdef DEBUG_PINS_ENABLED
    digitalWriteFast(DEBUG_PIN_1, LOW); // oscilloscope trigger
#endif
        // point dmaUpdateTimer to repeatedly load from values that set mod to MIN_BLOCK_PERIOD_TICKS and disable OE
        dmaUpdateTimer.TCD->SADDR = SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::timerPairIdle;
        // set timer increment to repeat timerPairIdle
        dmaUpdateTimer.TCD->SLAST = -(TIMER_REGISTERS_TO_UPDATE*sizeof(uint16_t));
        // disable channel-to-channel linking - don't link dmaClockOutData until buffer is ready
        dmaUpdateTimer.TCD->CSR &= ~(1 << 5);

        // set flag so other ISR can enable DMA again when data is ready
        SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::dmaBufferUnderrun = true;

#ifdef DEBUG_PINS_ENABLED
    digitalWriteFast(DEBUG_PIN_1, HIGH); // oscilloscope trigger
#endif
    } else {
        // get next row to draw to display and update DMA pointers
        int currentRow = cbGetNextRead(&dmaBuffer);
        dmaUpdateAddress.TCD->SADDR = &((matrixUpdateBlock*)SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::matrixUpdateBlocks + (currentRow * SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::latchesPerRow))->addressValues;
        dmaUpdateTimer.TCD->SADDR = &((matrixUpdateBlock*)SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::matrixUpdateBlocks + (currentRow * SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::latchesPerRow))->timerValues.timer_oe;
        dmaClockOutData.TCD->SADDR = (uint8_t*)SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::matrixUpdateData + (currentRow * SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::dmaBufferBytesPerRow);
    }

    // trigger software interrupt (DMA channel interrupt used instead of actual softint)
    NVIC_SET_PENDING(IRQ_DMA_CH0 + dmaUpdateAddress.channel);

    // clear pending int
    dmaClockOutData.clearInterrupt();

#ifdef DEBUG_PINS_ENABLED
    digitalWriteFast(DEBUG_PIN_1, LOW); // oscilloscope trigger
#endif
}
