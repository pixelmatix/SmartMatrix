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
#include "application.h"


#define INLINE __attribute__( ( always_inline ) ) inline

#define MATRIX_STACK_HEIGHT (matrixHeight / matrixPanelHeight)

#define ROW_CALCULATION_ISR_PRIORITY   0xFE // 0xFF = lowest priority

// hardware-specific definitions
// prescale of 7 is F_BUS/128
#define LATCH_TIMER_PRESCALE  31
#define TIMER_FREQUENCY     ((SystemCoreClock / 2) / LATCH_TIMER_PRESCALE + 1)
#define NS_TO_TICKS(X)      (uint32_t)(TIMER_FREQUENCY * ((X) / 1000000000.0))
#define TICKS_PER_FRAME   (TIMER_FREQUENCY/refreshRate)

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
const int SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::matrixPanelHeight = CONVERT_PANELTYPE_TO_MATRIXPANELHEIGHT(panelType);
template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
const int SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::matrixRowPairOffset = CONVERT_PANELTYPE_TO_MATRIXROWPAIROFFSET(panelType);


template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
void rowShiftCompleteISR(void);
template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
void rowCalculationISR(void);


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
uint8_t SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::refreshRate = 30;


// todo: just use a single buffer for Blocks/LUT/Data?
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
template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
uint8_t * SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::matrixUpdateDataByte;


template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::SmartMatrix3(uint8_t bufferrows, uint32_t * dataBuffer) {
    SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::globalinstance = this;

    matrixUpdateData = dataBuffer;
    matrixUpdateDataByte = (uint8_t*)matrixUpdateData;
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

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
INLINE void SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::loadMatrixBuffers(unsigned char currentRow) {
    int i,j;

    // static to avoid putting large buffer on the stack
    static rgb48 tempRow0[matrixWidth];

    // clear buffer to prevent garbage data showing through transparent layers
    memset(tempRow0, 0x00, sizeof(tempRow0));

    // get pixel data from layers
    SM_Layer * templayer = globalinstance->baseLayer;
    while(templayer) {
        templayer->fillRefreshRow(currentRow, &tempRow0[0]);
        templayer = templayer->nextLayer;        
    }


    for (j = 0; j < matrixWidth; j++) {
        if(currentRow % 2)
            i=(matrixWidth-j-1);
        else
            i=j;

        uint16_t temp0red,temp0green,temp0blue;

        temp0red = tempRow0[j].red;
        temp0green = tempRow0[j].green;
        temp0blue = tempRow0[j].blue;

        uint8_t globalbrightness = (0x20UL * (dimmingMaximum - dimmingFactor)) / dimmingMaximum;
        uint8_t localshift = 0;

        if(globalbrightness == 0x20)
            globalbrightness = 0x1f;

#if 1
        uint16_t value = temp0red | temp0green | temp0blue;

        // shift until the highest bit of value is set, or until globalbrightness == 1)
        while(!((value << localshift) & 0x8000) && (globalbrightness != 1)) {
            globalbrightness >>= 1;
            localshift++;
        }


        // shift needs to put 16-bit color value into lowest byte, which will be sent over SPI
        localshift = 8 - localshift;
#else
        localshift = 8;
#endif
        // global brightness
        matrixUpdateDataByte[4 + ((currentRow * matrixWidth + i) * 4) + 0] = 0xE0 | globalbrightness;

        matrixUpdateDataByte[4 + ((currentRow * matrixWidth + i) * 4) + 1] = temp0blue >> localshift;
        matrixUpdateDataByte[4 + ((currentRow * matrixWidth + i) * 4) + 2] = temp0green >> localshift;
        matrixUpdateDataByte[4 + ((currentRow * matrixWidth + i) * 4) + 3] = temp0red >> localshift;
    }
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
INLINE void SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::matrixCalculations(bool initial) {
    static unsigned char currentRow = 0;
    int i;

    // fill start and end frame markers
    for(i=0; i<4; i++) {
        matrixUpdateDataByte[i] = 0;
        matrixUpdateDataByte[4 + (matrixWidth * matrixHeight * 4) + i] = 0xFF;
    }

    do {

#ifdef DEBUG_PINS_ENABLED
    digitalWriteFast(DEBUG_PIN_3, HIGH); // oscilloscope trigger
#endif
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
        }

        // do once-per-line updates
        // none right now


        SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::loadMatrixBuffers(currentRow);

#ifdef DEBUG_PINS_ENABLED
    digitalWriteFast(DEBUG_PIN_3, LOW);
#endif

        // enqueue row
        if (++currentRow >= matrixHeight)
            currentRow = 0;


    } while (currentRow);
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
    refreshRate = newRefreshRate;
    refreshRateChanged = true;
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


    // setup SPI and DMA to feed it
    SPI1.begin(D5);
    //SPI1.setBitOrder(order);
    SPI1.setClockSpeed(1000000);
    //SPI1.setClockDivider(divider);
    //SPI1.setDataMode(mode);
    SPI1.setBitOrder(MSBFIRST);
    SPI1.setDataMode(SPI_MODE0);

    // setup Timer
    TIM_TimeBaseInitTypeDef timerInitStructure;
    NVIC_InitTypeDef nvicStructure;

    // TIM clock enable
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);

    uint16_t TIM_Prescaler = LATCH_TIMER_PRESCALE;

    // Timebase configuration
    timerInitStructure.TIM_Prescaler = TIM_Prescaler;
    timerInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    timerInitStructure.TIM_Period = TICKS_PER_FRAME;
    timerInitStructure.TIM_ClockDivision = 0;

    TIM_TimeBaseInit(TIM7, &timerInitStructure);

    TIM_ITConfig(TIM7, TIM_IT_Update, ENABLE);

    TIM_Cmd(TIM7, ENABLE);

    attachSystemInterrupt(SysInterrupt_TIM7_IRQ, rowShiftCompleteISR<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>);

    // Enable Timer Interrupt
    nvicStructure.NVIC_IRQChannel = TIM7_IRQn;
    nvicStructure.NVIC_IRQChannelPreemptionPriority = 10;
    nvicStructure.NVIC_IRQChannelSubPriority = 1;
    nvicStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvicStructure);
}

// called by SPI when transfer is done
// calculate data for next frame
// low priority ISR as it will take a long time to complete
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

// called by timer when frame is ready to be shifted out
// if data isn't ready, set flag so this can be called again later
template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
void rowShiftCompleteISR(void) {
#ifdef DEBUG_PINS_ENABLED
    digitalWriteFast(DEBUG_PIN_1, HIGH); // oscilloscope trigger
#endif

    // TODO: if underrun
        // set flag so other ISR can enable DMA again when data is ready
        //SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::dmaBufferUnderrun = true;
    // else, start SPI
    SPI1.transfer((uint8_t*)SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::matrixUpdateData,
        (uint8_t*)SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::matrixUpdateData,
        ((matrixWidth * matrixHeight)*4) + (4+4), rowCalculationISR<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>);

#ifdef DEBUG_PINS_ENABLED
    digitalWriteFast(DEBUG_PIN_1, LOW); // oscilloscope trigger
#endif
}
