/*
 * SmartMatrix Library - Calculation Code for Teensy 3.x Platform
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

#define INLINE __attribute__( ( always_inline ) ) inline

// to avoid 100% CPU usage, we by default don't calculate on every frame.  Calc refresh rate will be a fraction of Refresh refresh rate
template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
uint8_t SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::maxCalcCpuPercentage = 80;

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
uint8_t SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::calc_refreshRateDivider = 2;

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
uint16_t SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::calc_refreshRate = 120/SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::calc_refreshRateDivider;

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
SM_Layer * SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::baseLayer;

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
void * SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::tempRow0Ptr;

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
void * SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::tempRow1Ptr;

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
volatile bool SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::dmaBufferUnderrun = false;

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
bool SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::dmaBufferUnderrunSinceLastCheck = false;

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
bool SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::refreshRateLowered = false;

// set to true initially so all layers get the initial refresh rate
template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
bool SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::refreshRateChanged = true;

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
uint8_t SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::lsbMsbTransitionBit;

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::SmartMatrix3(void) {
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
void SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::dmaBufferUnderrunCallback(void) {
    dmaBufferUnderrun = true;
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
void SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::matrixCalculations() {
    static unsigned long lastMillisStart;
    static unsigned long lastMillisEnd;
    static int refreshFramesSinceLastCalculation = 0;

    if(++refreshFramesSinceLastCalculation < calc_refreshRateDivider)
        return;

    refreshFramesSinceLastCalculation = 0;

    // safety check - if using up too much CPU, increase refresh rate divider to give more time for sketch to run
    unsigned long calculationCpuTime = lastMillisEnd - lastMillisStart;
    unsigned long totalCpuTime = millis() - lastMillisStart;
    if(totalCpuTime && ((calculationCpuTime * 100) / totalCpuTime) > SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::maxCalcCpuPercentage) {
        // increase CPU divider by 1
        SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::setCalcRefreshRateDivider(SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::getCalcRefreshRateDivider() + 1);
        refreshRateLowered = true;
    }

    lastMillisStart = millis();

    // only do calculations if there is free space (should be redundant, as we only get called if there is free space)
    if (!SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::isFrameBufferFree())
        return;

    // do once-per-frame updates
    if (rotationChange) {
        SM_Layer * templayer = SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::baseLayer;
        while(templayer) {
            templayer->setRotation(rotation);
            templayer = templayer->nextLayer;
        }
        rotationChange = false;
    }

    SM_Layer * templayer = SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::baseLayer;
    while(templayer) {
        if(refreshRateChanged) {
            templayer->setRefreshRate(calc_refreshRate);
        }
        templayer->frameRefreshCallback();
        templayer = templayer->nextLayer;
    }
    refreshRateChanged = false;
    if (brightnessChange) {
        SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::setBrightness(brightness);
        brightnessChange = false;
    }

    SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::loadMatrixBuffers(lsbMsbTransitionBit);

    SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::writeFrameBuffer(0);

    lastMillisEnd = millis();
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

// brightness scales from 0-PIXELS_PER_LATCH
template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
int SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::brightness = PIXELS_PER_LATCH;

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
void SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::setBrightness(uint8_t newBrightness) {
    brightness = (PIXELS_PER_LATCH*newBrightness)/255;
    brightnessChange = true;
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
void SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::setRefreshRate(uint16_t newRefreshRate) {
    calc_refreshRate = newRefreshRate / calc_refreshRateDivider;

    refreshRateChanged = true;
    SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::setRefreshRate(newRefreshRate);
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
void SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::setMaxCalculationCpuPercentage(uint8_t newMaxCpuPercentage) {
    if(newMaxCpuPercentage > 100)
        newMaxCpuPercentage = 100;

    maxCalcCpuPercentage = newMaxCpuPercentage;
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
void SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::setCalcRefreshRateDivider(uint8_t newDivider) {
    // TODO: improve so fractional results don't screw up the calc_refreshRate divider
    // TODO: improve to get actual refresh rate from refresh class
    if(newDivider == 0)
        newDivider = 1;

    calc_refreshRate = SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::getRefreshRate() / newDivider;
    calc_refreshRateDivider = newDivider;
    refreshRateChanged = true;
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
uint8_t SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::getCalcRefreshRateDivider(void) {
    return calc_refreshRateDivider;
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
uint16_t SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::getRefreshRate(void) {
    return calc_refreshRate;
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
TaskHandle_t SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::calcTaskHandle;

/* Task2 with priority 2 */
template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
void SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::calcTask(void* pvParameters)
{        
    while(1) {   
        if( xSemaphoreTake(calcTaskSemaphore, portMAX_DELAY) == pdTRUE ) {
#ifdef DEBUG_PINS_ENABLED
            gpio_set_level(DEBUG_1_GPIO, 1);
#endif

            // we usually do this with an ISR in the refresh class, but ESP32 doesn't let us store a templated method in IRAM (at least not easily) so we call this from the calc task
            SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::markRefreshComplete();

            matrixCalculations();

#ifdef DEBUG_PINS_ENABLED
            gpio_set_level(DEBUG_1_GPIO, 0);
#endif
        }
    }
}

#define MATRIX_CALC_TASK_PRIORTY 2

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
void SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::begin(uint32_t dmaRamToKeepFreeBytes)
{
    printf("\r\nStarting SmartMatrix Mallocs\r\n");
    printf("Heap Memory Available: %d bytes total, %d bytes largest free block: \r\n", heap_caps_get_free_size(0), heap_caps_get_largest_free_block(0));
    printf("8-bit Accessible Memory Available: %d bytes total, %d bytes largest free block: \r\n", heap_caps_get_free_size(MALLOC_CAP_8BIT), heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));
    printf("32-bit Memory Available: %d bytes total, %d bytes largest free block: \r\n", heap_caps_get_free_size(MALLOC_CAP_32BIT), heap_caps_get_largest_free_block(MALLOC_CAP_32BIT));
    printf("DMA Memory Available: %d bytes total, %d bytes largest free block: \r\n", heap_caps_get_free_size(MALLOC_CAP_DMA), heap_caps_get_largest_free_block(MALLOC_CAP_DMA));

    SM_Layer * templayer = baseLayer;
    while(templayer) {
        templayer->begin();
        templayer = templayer->nextLayer;
    }

    calcTaskSemaphore = xSemaphoreCreateBinary();
    // TODO: fine tune stack size: 1000 works with 64x64/32-24bit, 500 doesn't, does it change based on matrix size, depth?
    xTaskCreate(calcTask, "SmartMatrixCalc", 1000, NULL, MATRIX_CALC_TASK_PRIORTY, &calcTaskHandle);

    printf("SmartMatrix Layers Allocated from Heap:\r\n");
    printf("Heap Memory Available: %d bytes total, %d bytes largest free block: \r\n", heap_caps_get_free_size(0), heap_caps_get_largest_free_block(0));

#if defined(ESP32)
    // malloc temporary buffers needed for loadMatrixBuffers
    if((COLOR_DEPTH_BITS == 12) || (COLOR_DEPTH_BITS == 16)){
        tempRow0Ptr = malloc(sizeof(rgb48) * PIXELS_PER_LATCH);
        tempRow1Ptr = malloc(sizeof(rgb48) * PIXELS_PER_LATCH);
    } else {
        tempRow0Ptr = malloc(sizeof(rgb24) * PIXELS_PER_LATCH);
        tempRow1Ptr = malloc(sizeof(rgb24) * PIXELS_PER_LATCH);
    }

    assert(tempRow0Ptr != NULL);
    assert(tempRow1Ptr != NULL);
#endif

    SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::setMatrixCalculationsCallback(matrixCalculationsSignal);
    SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::begin(dmaRamToKeepFreeBytes);

    // refresh rate is now set, update calc refresh rate
    setCalcRefreshRateDivider(calc_refreshRateDivider);
    lsbMsbTransitionBit = SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::getLsbMsbTransitionBit();

}

//#define OEPWM_TEST_ENABLE
#define OEPWM_THRESHOLD_BIT 1

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
INLINE void SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::loadMatrixBuffers48(frameStruct * frameBuffer, int currentRow, int lsbMsbTransitionBit) {
    int i;

#if defined(ESP32)
    // use buffers malloc'd previously
    rgb48 * tempRow0 = (rgb48*)tempRow0Ptr;
    rgb48 * tempRow1 = (rgb48*)tempRow1Ptr;
#else
    // static to avoid putting large buffer on the stack
    static rgb48 tempRow0[PIXELS_PER_LATCH];
    static rgb48 tempRow1[PIXELS_PER_LATCH];
#endif
    // clear buffer to prevent garbage data showing through transparent layers
    memset(tempRow0, 0x00, sizeof(rgb48) * PIXELS_PER_LATCH);
    memset(tempRow1, 0x00, sizeof(rgb48) * PIXELS_PER_LATCH);

    // get pixel data from layers
    SM_Layer * templayer = SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::baseLayer;
    while(templayer) {
        for(i=0; i<MATRIX_STACK_HEIGHT; i++) {
            // Z-shape, bottom to top
            if(!(optionFlags & SMARTMATRIX_OPTIONS_C_SHAPE_STACKING) &&
                (optionFlags & SMARTMATRIX_OPTIONS_BOTTOM_TO_TOP_STACKING)) {
                // fill data from bottom to top, so bottom panel is the one closest to Teensy
                templayer->fillRefreshRow(currentRow + (MATRIX_STACK_HEIGHT-i-1)*MATRIX_PANEL_HEIGHT, &tempRow0[i*matrixWidth]);
                templayer->fillRefreshRow(currentRow + ROW_PAIR_OFFSET + (MATRIX_STACK_HEIGHT-i-1)*MATRIX_PANEL_HEIGHT, &tempRow1[i*matrixWidth]);
            // Z-shape, top to bottom
            } else if(!(optionFlags & SMARTMATRIX_OPTIONS_C_SHAPE_STACKING) &&
                !(optionFlags & SMARTMATRIX_OPTIONS_BOTTOM_TO_TOP_STACKING)) {
                // fill data from top to bottom, so top panel is the one closest to Teensy
                templayer->fillRefreshRow(currentRow + i*MATRIX_PANEL_HEIGHT, &tempRow0[i*matrixWidth]);
                templayer->fillRefreshRow(currentRow + ROW_PAIR_OFFSET + i*MATRIX_PANEL_HEIGHT, &tempRow1[i*matrixWidth]);
            // C-shape, bottom to top
            } else if((optionFlags & SMARTMATRIX_OPTIONS_C_SHAPE_STACKING) &&
                (optionFlags & SMARTMATRIX_OPTIONS_BOTTOM_TO_TOP_STACKING)) {
                // alternate direction of filling (or loading) for each matrixwidth
                // swap row order from top to bottom for each stack (tempRow1 filled with top half of panel, tempRow0 filled with bottom half)
                if((MATRIX_STACK_HEIGHT-i+1)%2) {
                    templayer->fillRefreshRow((ROWS_PER_FRAME-currentRow-1) + ROW_PAIR_OFFSET + (i)*MATRIX_PANEL_HEIGHT, &tempRow0[i*matrixWidth]);
                    templayer->fillRefreshRow((ROWS_PER_FRAME-currentRow-1) + (i)*MATRIX_PANEL_HEIGHT, &tempRow1[i*matrixWidth]);
                } else {
                    templayer->fillRefreshRow(currentRow + (i)*MATRIX_PANEL_HEIGHT, &tempRow0[i*matrixWidth]);
                    templayer->fillRefreshRow(currentRow + ROW_PAIR_OFFSET + (i)*MATRIX_PANEL_HEIGHT, &tempRow1[i*matrixWidth]);
                }
            // C-shape, top to bottom
            } else if((optionFlags & SMARTMATRIX_OPTIONS_C_SHAPE_STACKING) && 
                !(optionFlags & SMARTMATRIX_OPTIONS_BOTTOM_TO_TOP_STACKING)) {
                if((MATRIX_STACK_HEIGHT-i)%2) {
                    templayer->fillRefreshRow(currentRow + (MATRIX_STACK_HEIGHT-i-1)*MATRIX_PANEL_HEIGHT, &tempRow0[i*matrixWidth]);
                    templayer->fillRefreshRow(currentRow + ROW_PAIR_OFFSET + (MATRIX_STACK_HEIGHT-i-1)*MATRIX_PANEL_HEIGHT, &tempRow1[i*matrixWidth]);
                } else {
                    templayer->fillRefreshRow((ROWS_PER_FRAME-currentRow-1) + ROW_PAIR_OFFSET + (MATRIX_STACK_HEIGHT-i-1)*MATRIX_PANEL_HEIGHT, &tempRow0[i*matrixWidth]);
                    templayer->fillRefreshRow((ROWS_PER_FRAME-currentRow-1) + (MATRIX_STACK_HEIGHT-i-1)*MATRIX_PANEL_HEIGHT, &tempRow1[i*matrixWidth]);
                }
            }
        }
        templayer = templayer->nextLayer;        
    }

    /* 
    union {
        uint8_t word;
        struct {
            // order of bits in word matches how GPIO connects to the display
            uint8_t GPIO_WORD_ORDER_8BIT;
        };
    } o0; 
    */
    
    for(int j=0; j<COLOR_DEPTH_BITS; j++) {
        int maskoffset = 0;
        if(COLOR_DEPTH_BITS == 12)   // 36-bit color
            maskoffset = 4;
        else if (COLOR_DEPTH_BITS == 16) // 48-bit color
            maskoffset = 0;
        else if (COLOR_DEPTH_BITS == 8)  // 24-bit color
            maskoffset = 0;

        uint16_t mask = (1 << (j + maskoffset));
        
        SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::rowBitStruct *p=&(frameBuffer->rowdata[currentRow].rowbits[j]); //bitplane location to write to
        
        int i=0;
        while(i < PIXELS_PER_LATCH) {

            // parse through matrixWith block of pixels, from left to right, or right to left, depending on C_SHAPE_STACKING options
            for(int k=0; k < matrixWidth; k++) {
                int v=0;

#if (CLKS_DURING_LATCH == 0)
                // if there is no latch to hold address, output ADDX lines directly to GPIO and latch data at end of cycle
                int gpioRowAddress = currentRow;
                // normally output current rows ADDX, special case for LSB, output previous row's ADDX (as previous row is being displayed for one latch cycle)
                if(j == 0)
                    gpioRowAddress = currentRow-1;

                if (gpioRowAddress & 0x01) v|=BIT_A;
                if (gpioRowAddress & 0x02) v|=BIT_B;
                if (gpioRowAddress & 0x04) v|=BIT_C;
                if (gpioRowAddress & 0x08) v|=BIT_D;
                if (gpioRowAddress & 0x10) v|=BIT_E;

                // need to disable OE after latch to hide row transition
                if((i+k) == 0) v|=BIT_OE;

                // drive latch while shifting out last bit of RGB data
                if((i+k) == PIXELS_PER_LATCH-1) v|=BIT_LAT;
#endif

                // turn off OE after brightness value is reached when displaying MSBs
                // MSBs always output normal brightness
                // LSB (!j) outputs normal brightness as MSB from previous row is being displayed
                if((j > lsbMsbTransitionBit || !j) && ((i+k) >= brightness)) v|=BIT_OE;

#ifndef OEPWM_TEST_ENABLE
                // special case for the bits *after* LSB through (lsbMsbTransitionBit) - OE is output after data is shifted, so need to set OE to fractional brightness
                if(j && j <= lsbMsbTransitionBit) {
                    // divide brightness in half for each bit below lsbMsbTransitionBit
                    int lsbBrightness = brightness >> (lsbMsbTransitionBit - j + 1);
                    if((i+k) >= lsbBrightness) v|=BIT_OE;
                }
#else
                // special case for the bits *after* LSB through (lsbMsbTransitionBit) - OE is output after data is shifted, so need to set OE to fractional brightness
                if(j && j <= lsbMsbTransitionBit) {
                    // all bits through OEPWM_THRESHOLD_BIT we handle by toggling short PWM pulses smaller than one clock cycle
                    if(j >= 1 && j <= OEPWM_THRESHOLD_BIT) {
                        // width of pwm OE pulse is ~1/2 the width of a DMA OE pulse (so shift lsbPwmBrightnessPulses one fewer times than lsbBrightness)
                        int lsbPwmBrightnessPulses = (brightness) >> (lsbMsbTransitionBit - j + 1 - 1);
                        // now setting brightness for LSB, use PWM OE
                        if((k%2) || k >= (2 * lsbPwmBrightnessPulses)) v|=BIT_OE;
                    } else {
                        // divide brightness in half for each bit below lsbMsbTransitionBit
                        int lsbBrightness = brightness >> (lsbMsbTransitionBit - j + 1);
                        if((i+k) >= lsbBrightness) v|=BIT_OE;
                    }
                }                
#endif
                // need to turn off OE one clock before latch, otherwise can get ghosting
                if((i+k)==PIXELS_PER_LATCH-1) v|=BIT_OE;

                if (tempRow0[i+k].red & mask)
                    v|=BIT_R1;
                if (tempRow0[i+k].green & mask)
                    v|=BIT_G1;
                if (tempRow0[i+k].blue & mask)
                    v|=BIT_B1;
                if (tempRow1[i+k].red & mask)
                    v|=BIT_R2;
                if (tempRow1[i+k].green & mask)
                    v|=BIT_G2;
                if (tempRow1[i+k].blue & mask)
                    v|=BIT_B2;

                if((optionFlags & SMARTMATRIX_OPTIONS_C_SHAPE_STACKING) && !((i/matrixWidth)%2)) {
                    //currentRowDataPtr->rowbits[j].data[(((i+matrixWidth-1)-k)*DMA_UPDATES_PER_CLOCK)] = o0.word;
                    //TODO: support C-shape stacking
                } else {
                    if(MATRIX_I2S_MODE == I2S_PARALLEL_BITS_8) {
                        //Save the calculated value to the bitplane memory in 16-bit reversed order to account for I2S Tx FIFO mode1 ordering
                        if(k%4 == 0){
                            p->data[(i+k)+2] = v;
                        } else if(k%4 == 1) {
                            p->data[(i+k)+2] = v;
                        } else if(k%4 == 2) {
                            p->data[(i+k)-2] = v;
                        } else { //if(k%4 == 3)
                            p->data[(i+k)-2] = v;
                        }
                    } else {
                        //Save the calculated value to the bitplane memory in reverse order to account for I2S Tx FIFO mode1 ordering
                        if(k%2){
                            p->data[(i+k)-1] = v;
                        } else {
                            p->data[(i+k)+1] = v;
                        }
                    }
                }
            }

            i += matrixWidth;
        }

        // TODO: insert latch data for all color depth bits all at once at the end, saving a few cycles?
        // TODO: prefill latch across all frames during begin() and only need to update when brightness/refreshrate changed?
#if (CLKS_DURING_LATCH > 0)
        // if external latch is used to hold ADDX lines, load the ADDX latch and latch the RGB data here
        for(int k=PIXELS_PER_LATCH; k < PIXELS_PER_LATCH + CLKS_DURING_LATCH; k++) {
            int v = 0;
            // after data is shifted in, pulse latch for one clock cycle
            if(k == PIXELS_PER_LATCH) {
                v|=BIT_LAT;
            }

            //Do not show image while the line bits are changing
            v|=BIT_OE;

            // set ADDX values to high while latch is high, keep them high while latch drops to clock it in to ADDX latch
            if(k >= PIXELS_PER_LATCH) {               
                if (currentRow & 0x01) v|=BIT_R1;
                if (currentRow & 0x02) v|=BIT_G1;
                if (currentRow & 0x04) v|=BIT_B1;
                if (currentRow & 0x08) v|=BIT_R2;
                if (currentRow & 0x10) v|=BIT_G2;
                // reserve B2 for OE SWITCH
#ifdef OEPWM_TEST_ENABLE
                //if(j == 0) {
                // set the MUX to output PWM_OE instead of DMA_OE, for the latches corresponding to bit 0 - OEPWM_THRESHOLD_BIT
                if(j < OEPWM_THRESHOLD_BIT) {
                    // now setting brightness for LSB, use PWM OE
                    v|=BIT_B2;
                }
#endif
            }

            if(MATRIX_I2S_MODE == I2S_PARALLEL_BITS_8) {
                //Save the calculated value to the bitplane memory in 16-bit reversed order to account for I2S Tx FIFO mode1 ordering
                if(k%4 == 0){
                    p->data[k+2] = v;
                } else if(k%4 == 1) {
                    p->data[k+2] = v;
                } else if(k%4 == 2) {
                    p->data[k-2] = v;
                } else { //if(k%4 == 3)
                    p->data[k-2] = v;
                }
            } else {
                //Save the calculated value to the bitplane memory in reverse order to account for I2S Tx FIFO mode1 ordering
                if(k%2){
                    p->data[k-1] = v;
                } else {
                    p->data[k+1] = v;
                }
            }
        }
#endif
    }
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
INLINE void SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::loadMatrixBuffers24(frameStruct * frameBuffer, int currentRow, int lsbMsbTransitionBit) {
    int i;

#if defined(ESP32)
    // use buffers malloc'd previously
    rgb24 * tempRow0 = (rgb24*)tempRow0Ptr;
    rgb24 * tempRow1 = (rgb24*)tempRow1Ptr;
#else
    // static to avoid putting large buffer on the stack
    static rgb24 tempRow0[PIXELS_PER_LATCH];
    static rgb24 tempRow1[PIXELS_PER_LATCH];
#endif

    // clear buffer to prevent garbage data showing through transparent layers
    memset(tempRow0, 0x00, sizeof(rgb24) * PIXELS_PER_LATCH);
    memset(tempRow1, 0x00, sizeof(rgb24) * PIXELS_PER_LATCH);

    // get pixel data from layers
    SM_Layer * templayer = SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::baseLayer;
    while(templayer) {
        for(i=0; i<MATRIX_STACK_HEIGHT; i++) {
            // Z-shape, bottom to top
            if(!(optionFlags & SMARTMATRIX_OPTIONS_C_SHAPE_STACKING) &&
                (optionFlags & SMARTMATRIX_OPTIONS_BOTTOM_TO_TOP_STACKING)) {
                // fill data from bottom to top, so bottom panel is the one closest to Teensy
                templayer->fillRefreshRow(currentRow + (MATRIX_STACK_HEIGHT-i-1)*MATRIX_PANEL_HEIGHT, &tempRow0[i*matrixWidth]);
                templayer->fillRefreshRow(currentRow + ROW_PAIR_OFFSET + (MATRIX_STACK_HEIGHT-i-1)*MATRIX_PANEL_HEIGHT, &tempRow1[i*matrixWidth]);
            // Z-shape, top to bottom
            } else if(!(optionFlags & SMARTMATRIX_OPTIONS_C_SHAPE_STACKING) &&
                !(optionFlags & SMARTMATRIX_OPTIONS_BOTTOM_TO_TOP_STACKING)) {
                // fill data from top to bottom, so top panel is the one closest to Teensy
                templayer->fillRefreshRow(currentRow + i*MATRIX_PANEL_HEIGHT, &tempRow0[i*matrixWidth]);
                templayer->fillRefreshRow(currentRow + ROW_PAIR_OFFSET + i*MATRIX_PANEL_HEIGHT, &tempRow1[i*matrixWidth]);
            // C-shape, bottom to top
            } else if((optionFlags & SMARTMATRIX_OPTIONS_C_SHAPE_STACKING) &&
                (optionFlags & SMARTMATRIX_OPTIONS_BOTTOM_TO_TOP_STACKING)) {
                // alternate direction of filling (or loading) for each matrixwidth
                // swap row order from top to bottom for each stack (tempRow1 filled with top half of panel, tempRow0 filled with bottom half)
                if((MATRIX_STACK_HEIGHT-i+1)%2) {
                    templayer->fillRefreshRow((ROWS_PER_FRAME-currentRow-1) + ROW_PAIR_OFFSET + (i)*MATRIX_PANEL_HEIGHT, &tempRow0[i*matrixWidth]);
                    templayer->fillRefreshRow((ROWS_PER_FRAME-currentRow-1) + (i)*MATRIX_PANEL_HEIGHT, &tempRow1[i*matrixWidth]);
                } else {
                    templayer->fillRefreshRow(currentRow + (i)*MATRIX_PANEL_HEIGHT, &tempRow0[i*matrixWidth]);
                    templayer->fillRefreshRow(currentRow + ROW_PAIR_OFFSET + (i)*MATRIX_PANEL_HEIGHT, &tempRow1[i*matrixWidth]);
                }
            // C-shape, top to bottom
            } else if((optionFlags & SMARTMATRIX_OPTIONS_C_SHAPE_STACKING) && 
                !(optionFlags & SMARTMATRIX_OPTIONS_BOTTOM_TO_TOP_STACKING)) {
                if((MATRIX_STACK_HEIGHT-i)%2) {
                    templayer->fillRefreshRow(currentRow + (MATRIX_STACK_HEIGHT-i-1)*MATRIX_PANEL_HEIGHT, &tempRow0[i*matrixWidth]);
                    templayer->fillRefreshRow(currentRow + ROW_PAIR_OFFSET + (MATRIX_STACK_HEIGHT-i-1)*MATRIX_PANEL_HEIGHT, &tempRow1[i*matrixWidth]);
                } else {
                    templayer->fillRefreshRow((ROWS_PER_FRAME-currentRow-1) + ROW_PAIR_OFFSET + (MATRIX_STACK_HEIGHT-i-1)*MATRIX_PANEL_HEIGHT, &tempRow0[i*matrixWidth]);
                    templayer->fillRefreshRow((ROWS_PER_FRAME-currentRow-1) + (MATRIX_STACK_HEIGHT-i-1)*MATRIX_PANEL_HEIGHT, &tempRow1[i*matrixWidth]);
                }
            }
        }
        templayer = templayer->nextLayer;        
    }

    /*
    union {
        uint8_t word;
        struct {
            // order of bits in word matches how GPIO connects to the display
            uint8_t GPIO_WORD_ORDER_8BIT;
        };
    } o0;
    */
    
    for(int j=0; j<COLOR_DEPTH_BITS; j++) {
        int maskoffset = 0;
        if(COLOR_DEPTH_BITS == 12)   // 36-bit color
            maskoffset = 4;
        else if (COLOR_DEPTH_BITS == 16) // 48-bit color
            maskoffset = 0;
        else if (COLOR_DEPTH_BITS == 8)  // 24-bit color
            maskoffset = 0;

        uint16_t mask = (1 << (j + maskoffset));
        
        SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::rowBitStruct *p=&(frameBuffer->rowdata[currentRow].rowbits[j]); //bitplane location to write to
        
        int i=0;
        while(i < PIXELS_PER_LATCH) {

            // parse through matrixWith block of pixels, from left to right, or right to left, depending on C_SHAPE_STACKING options
            for(int k=0; k < matrixWidth; k++) {
                int v=0;

#if (CLKS_DURING_LATCH == 0)
                // if there is no latch to hold address, output ADDX lines directly to GPIO and latch data at end of cycle
                int gpioRowAddress = currentRow;
                // normally output current rows ADDX, special case for LSB, output previous row's ADDX (as previous row is being displayed for one latch cycle)
                if(j == 0)
                    gpioRowAddress = currentRow-1;

                if (gpioRowAddress & 0x01) v|=BIT_A;
                if (gpioRowAddress & 0x02) v|=BIT_B;
                if (gpioRowAddress & 0x04) v|=BIT_C;
                if (gpioRowAddress & 0x08) v|=BIT_D;
                if (gpioRowAddress & 0x10) v|=BIT_E;

                // need to disable OE after latch to hide row transition
                if((i+k) == 0) v|=BIT_OE;

                // drive latch while shifting out last bit of RGB data
                if((i+k) == PIXELS_PER_LATCH-1) v|=BIT_LAT;
#endif

                // turn off OE after brightness value is reached when displaying MSBs
                // MSBs always output normal brightness
                // LSB (!j) outputs normal brightness as MSB from previous row is being displayed
                if((j > lsbMsbTransitionBit || !j) && ((i+k) >= brightness)) v|=BIT_OE;

                // special case for the bits *after* LSB through (lsbMsbTransitionBit) - OE is output after data is shifted, so need to set OE to fractional brightness
                if(j && j <= lsbMsbTransitionBit) {
                    // divide brightness in half for each bit below lsbMsbTransitionBit
                    int lsbBrightness = brightness >> (lsbMsbTransitionBit - j + 1);
                    if((i+k) >= lsbBrightness) v|=BIT_OE;
                }

                // need to turn off OE one clock before latch, otherwise can get ghosting
                if((i+k)==PIXELS_PER_LATCH-1) v|=BIT_OE;

                if (tempRow0[i+k].red & mask)
                    v|=BIT_R1;
                if (tempRow0[i+k].green & mask)
                    v|=BIT_G1;
                if (tempRow0[i+k].blue & mask)
                    v|=BIT_B1;
                if (tempRow1[i+k].red & mask)
                    v|=BIT_R2;
                if (tempRow1[i+k].green & mask)
                    v|=BIT_G2;
                if (tempRow1[i+k].blue & mask)
                    v|=BIT_B2;

                if((optionFlags & SMARTMATRIX_OPTIONS_C_SHAPE_STACKING) && !((i/matrixWidth)%2)) {
                    //currentRowDataPtr->rowbits[j].data[(((i+matrixWidth-1)-k)*DMA_UPDATES_PER_CLOCK)] = o0.word;
                    //TODO: support C-shape stacking
                } else {
                    if(MATRIX_I2S_MODE == I2S_PARALLEL_BITS_8) {
                        //Save the calculated value to the bitplane memory in 16-bit reversed order to account for I2S Tx FIFO mode1 ordering
                        if(k%4 == 0){
                            p->data[(i+k)+2] = v;
                        } else if(k%4 == 1) {
                            p->data[(i+k)+2] = v;
                        } else if(k%4 == 2) {
                            p->data[(i+k)-2] = v;
                        } else { //if(k%4 == 3)
                            p->data[(i+k)-2] = v;
                        }
                    } else {
                        //Save the calculated value to the bitplane memory in reverse order to account for I2S Tx FIFO mode1 ordering
                        if(k%2){
                            p->data[(i+k)-1] = v;
                        } else {
                            p->data[(i+k)+1] = v;
                        }
                    }
                }
            }

            i += matrixWidth;
        }

#if (CLKS_DURING_LATCH > 0)
        // if external latch is used to hold ADDX lines, load the ADDX latch and latch the RGB data here
        for(int k=PIXELS_PER_LATCH; k < PIXELS_PER_LATCH + CLKS_DURING_LATCH; k++) {
            int v = 0;
            // after data is shifted in, pulse latch for one clock cycle
            if(k == PIXELS_PER_LATCH) {
                v|=BIT_LAT;
            }

            //Do not show image while the line bits are changing
            v|=BIT_OE;

            // set ADDX values to high while latch is high, keep them high while latch drops to clock it in to ADDX latch
            if(k >= PIXELS_PER_LATCH) {               
                if (currentRow & 0x01) v|=BIT_R1;
                if (currentRow & 0x02) v|=BIT_G1;
                if (currentRow & 0x04) v|=BIT_B1;
                if (currentRow & 0x08) v|=BIT_R2;
                if (currentRow & 0x10) v|=BIT_G2;
                // reserve B2 for OE SWITCH
            }

            if(MATRIX_I2S_MODE == I2S_PARALLEL_BITS_8) {
                //Save the calculated value to the bitplane memory in 16-bit reversed order to account for I2S Tx FIFO mode1 ordering
                if(k%4 == 0){
                    p->data[k+2] = v;
                } else if(k%4 == 1) {
                    p->data[k+2] = v;
                } else if(k%4 == 2) {
                    p->data[k-2] = v;
                } else { //if(k%4 == 3)
                    p->data[k-2] = v;
                }
            } else {
                //Save the calculated value to the bitplane memory in reverse order to account for I2S Tx FIFO mode1 ordering
                if(k%2){
                    p->data[k-1] = v;
                } else {
                    p->data[k+1] = v;
                }
            }
        }
#endif
    }
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
INLINE void SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::loadMatrixBuffers(int lsbMsbTransitionBit) {
#if 1
    unsigned char currentRow;

    frameStruct * currentFrameDataPtr = SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::getNextFrameBufferPtr();

    for(currentRow = 0; currentRow < ROWS_PER_FRAME; currentRow++) {
        // TODO: support rgb36/48 with same function, copy function to rgb24
        if(COLOR_DEPTH_BITS == 16)
            loadMatrixBuffers48(currentFrameDataPtr, currentRow, lsbMsbTransitionBit);
        else if(COLOR_DEPTH_BITS == 12)
            loadMatrixBuffers48(currentFrameDataPtr, currentRow, lsbMsbTransitionBit);
        else if(COLOR_DEPTH_BITS == 8)
            loadMatrixBuffers24(currentFrameDataPtr, currentRow, lsbMsbTransitionBit);
    }
#endif
}
