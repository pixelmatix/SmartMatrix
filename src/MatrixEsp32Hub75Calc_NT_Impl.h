/*
 * SmartMatrix Library - ESP32 HUB75 Panel Calculation Class (No Template Version)
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
#include "Esp32MemDisplay.h"

#define INLINE __attribute__( ( always_inline ) ) inline

template <int dummyvar>
void SmartMatrixHub75Calc_NT<dummyvar>::addLayer(SM_Layer * newlayer) {
    if(baseLayer) {
        SM_Layer * templayer = baseLayer;
        while(templayer->nextLayer)
            templayer = templayer->nextLayer;
        templayer->nextLayer = newlayer;
    } else {
        baseLayer = newlayer;
    }
}

template <int dummyvar>
int SmartMatrixHub75Calc_NT<dummyvar>::countFPS(void) {
    static long loops = 0;
    static long lastMillis = 0;
    long currentMillis = millis();
    int ret = 0;

    loops++;
    if(currentMillis - lastMillis >= 1000){
        if(Serial) {
            Serial.print("Loops last second:");
            Serial.println(loops);
        }
        ret = loops;
        lastMillis = currentMillis;
        loops = 0;
    }
    
    return ret;
}

template <int dummyvar>
void SmartMatrixHub75Calc_NT<dummyvar>::dmaBufferUnderrunCallback(void) {
    dmaBufferUnderrun = true;
}

template <int dummyvar>
void SmartMatrixHub75Calc_NT<dummyvar>::matrixCalculations() {
    static unsigned long lastMillisStart;
    static unsigned long lastMillisEnd;
    static int refreshFramesSinceLastCalculation = 0;
    SM_Layer * templayer;
    static bool firstRun = true;

    if(++refreshFramesSinceLastCalculation < calc_refreshRateDivider)
        return;

    refreshFramesSinceLastCalculation = 0;

    // safety check - if using up too much CPU, increase refresh rate divider to give more time for sketch to run
    unsigned long calculationCpuTime = lastMillisEnd - lastMillisStart;
    unsigned long totalCpuTime = millis() - lastMillisStart;
    if(totalCpuTime && ((calculationCpuTime * 100) / totalCpuTime) > maxCalcCpuPercentage) {
        // increase CPU divider by 1
        setCalcRefreshRateDivider(getCalcRefreshRateDivider() + 1);
        refreshRateLowered = true;
    }

    // only do calculations if there is free space (should be redundant, as we only get called if there is free space)
    if (!_matrixRefresh->isFrameBufferFree())
        return;

    templayer = baseLayer;
    bool refreshNeeded = false;
    while(templayer) {
        if(templayer->isLayerChanged())
            refreshNeeded = true;

        templayer = templayer->nextLayer;
    }

    if(!refreshNeeded && !firstRun)
        return;

    firstRun = false;

    // now we know we're actually going to update the frame, keep track of the time we started updating
    lastMillisStart = millis();

    // do once-per-frame updates
    if (rotationChange) {
        templayer = baseLayer;
        while(templayer) {
            templayer->setRotation(rotation);
            templayer = templayer->nextLayer;
        }
        rotationChange = false;
    }

    int largestRequestedBrightnessShifts = 0;

    templayer = baseLayer;
    while(templayer) {
        if(refreshRateChanged) {
            templayer->setRefreshRate(calc_refreshRate);
        }

        templayer->frameRefreshCallback();

        int tempval = templayer->getRequestedBrightnessShifts();
        if(tempval > largestRequestedBrightnessShifts)
            largestRequestedBrightnessShifts = tempval;

        templayer = templayer->nextLayer;
    }
    refreshRateChanged = false;

    int tempBrightness = brightness >> largestRequestedBrightnessShifts;

    // scale the overall brightness to accommodate a layer that has its data stored in non MSB bits
    if(tempBrightness != shiftedBrightness) {
        shiftedBrightness = tempBrightness;
        brightnessChange = true;
    }

    if (brightnessChange) {
        _matrixRefresh->setBrightness(shiftedBrightness);
        brightnessChange = false;
    }

    loadMatrixBuffers(lsbMsbTransitionBit, largestRequestedBrightnessShifts);

    _matrixRefresh->writeFrameBuffer(0);

    lastMillisEnd = millis();
}

template <int dummyvar>
void SmartMatrixHub75Calc_NT<dummyvar>::setRotation(rotationDegrees newrotation) {
    rotation = newrotation;
    rotationChange = true;
}

template <int dummyvar>
uint16_t SmartMatrixHub75Calc_NT<dummyvar>::getScreenWidth(void) const {
    if (rotation == rotation0 || rotation == rotation180) {
        return matrixWidth;
    } else {
        return matrixHeight;
    }
}

template <int dummyvar>
uint16_t SmartMatrixHub75Calc_NT<dummyvar>::getScreenHeight(void) const {
    if (rotation == rotation0 || rotation == rotation180) {
        return matrixHeight;
    } else {
        return matrixWidth;
    }
}

// brightness scales from 0-pixels_per_latch
template <int dummyvar>
void SmartMatrixHub75Calc_NT<dummyvar>::setBrightness(uint8_t newBrightness) {
    brightness = (pixels_per_latch*newBrightness)/255;
    brightnessChange = true;
}

template <int dummyvar>
void SmartMatrixHub75Calc_NT<dummyvar>::setRefreshRate(uint16_t newRefreshRate) {
    calc_refreshRate = newRefreshRate / calc_refreshRateDivider;

    refreshRateChanged = true;
    _matrixRefresh->setRefreshRate(newRefreshRate);
}

template <int dummyvar>
void SmartMatrixHub75Calc_NT<dummyvar>::setMaxCalculationCpuPercentage(uint8_t newMaxCpuPercentage) {
    if(newMaxCpuPercentage > 100)
        newMaxCpuPercentage = 100;

    maxCalcCpuPercentage = newMaxCpuPercentage;
}

template <int dummyvar>
void SmartMatrixHub75Calc_NT<dummyvar>::setCalcRefreshRateDivider(uint8_t newDivider) {
    // TODO: improve so fractional results don't screw up the calc_refreshRate divider
    // TODO: improve to get actual refresh rate from refresh class
    if(newDivider == 0)
        newDivider = 1;

    calc_refreshRate = _matrixRefresh->getRefreshRate() / newDivider;
    calc_refreshRateDivider = newDivider;
    refreshRateChanged = true;
}

template <int dummyvar>
uint8_t SmartMatrixHub75Calc_NT<dummyvar>::getCalcRefreshRateDivider(void) {
    return calc_refreshRateDivider;
}

template <int dummyvar>
uint16_t SmartMatrixHub75Calc_NT<dummyvar>::getRefreshRate(void) {
    return calc_refreshRate;
}

template <int dummyvar>
bool SmartMatrixHub75Calc_NT<dummyvar>::getdmaBufferUnderrunFlag(void) {
    if(dmaBufferUnderrunSinceLastCheck) {
        dmaBufferUnderrunSinceLastCheck = false;
        return true;
    }
    return false;
}

template <int dummyvar>
bool SmartMatrixHub75Calc_NT<dummyvar>::getRefreshRateLoweredFlag(void) {
    if(refreshRateLowered) {
        refreshRateLowered = false;
        return true;
    }
    return false;
}

/* Task2 with priority 2 */
template <int dummyvar>
void SmartMatrixHub75Calc_NT<dummyvar>::calcTask(void* pvParameters)
{        
    static long lastMillis = 0;
    SmartMatrixHub75Calc_NT* thisPtr = (SmartMatrixHub75Calc_NT*)pvParameters;
    while(1) {   
        if( xSemaphoreTake(calcTaskSemaphore, portMAX_DELAY) == pdTRUE ) {
#ifdef DEBUG_PINS_ENABLED
            gpio_set_level(DEBUG_1_GPIO, 1);
#endif

            long currentMillis = millis();
            if(currentMillis - lastMillis >= 4500){
                // sleep a bit to reset the watchdog (default is 5000ms between resets)
                vTaskDelay(1);
                lastMillis = currentMillis;
            }

            // we usually do this with an ISR in the refresh class, but ESP32 doesn't let us store a templated method in IRAM (at least not easily) so we call this from the calc task
            thisPtr->_matrixRefresh->markRefreshComplete();

            thisPtr->matrixCalculations();

#ifdef DEBUG_PINS_ENABLED
            gpio_set_level(DEBUG_1_GPIO, 0);
#endif
        }
    }
}

#define MATRIX_CALC_TASK_DEFAULT_PRIORITY   2
#define MATRIX_CALC_TASK_LOW_PRIORITY      1

template <int dummyvar>
void SmartMatrixHub75Calc_NT<dummyvar>::begin(uint32_t dmaRamToKeepFreeBytes)
{    
    printf("\r\nStarting SmartMatrix Mallocs\r\n");
    show_esp32_all_mem();

    SM_Layer * templayer = baseLayer;
    while(templayer) {
        templayer->begin();
        templayer = templayer->nextLayer;
    }

    calcTaskSemaphore = xSemaphoreCreateBinary();

    int taskPriority = MATRIX_CALC_TASK_DEFAULT_PRIORITY;
    if(optionFlags & SMARTMATRIX_OPTIONS_MATRIXCALC_LOWPRIORITY)
        taskPriority = MATRIX_CALC_TASK_LOW_PRIORITY;

    // by default run on core 0, leaving more room for the main Arduino task on core 1
    int calcTaskCore = 0;
    if(optionFlags & SMARTMATRIX_OPTIONS_ESP32_CALC_TASK_CORE_1)
        calcTaskCore = 1;

    // TODO: fine tune stack size: 1000 works with 64x64/32-24bit, 500 doesn't, does it change based on matrix size, depth?
    xTaskCreatePinnedToCore(calcTask, "SmartMatrixCalc", 1000, this, taskPriority, &calcTaskHandle, calcTaskCore);

    printf("SmartMatrix Layers Allocated from Heap:\r\n");
    show_esp32_heap_mem();

#if defined(ESP32)
    // malloc temporary buffers needed for loadMatrixBuffers
    int numPixelsPerTempRow = pixels_per_latch/physical_rows_per_refresh_row;

    if((COLOR_DEPTH_BITS == 12) || (COLOR_DEPTH_BITS == 16)){
        tempRow0Ptr = malloc(sizeof(rgb48) * numPixelsPerTempRow);
        tempRow1Ptr = malloc(sizeof(rgb48) * numPixelsPerTempRow);
    } else {
        tempRow0Ptr = malloc(sizeof(rgb24) * numPixelsPerTempRow);
        tempRow1Ptr = malloc(sizeof(rgb24) * numPixelsPerTempRow);
    }

    assert(tempRow0Ptr != NULL);
    assert(tempRow1Ptr != NULL);
#endif

    _matrixRefresh->setMatrixCalculationsCallback(matrixCalculationsSignal);
    _matrixRefresh->begin(dmaRamToKeepFreeBytes);

    // refresh rate is now set, update calc refresh rate
    setCalcRefreshRateDivider(calc_refreshRateDivider);
    lsbMsbTransitionBit = _matrixRefresh->getLsbMsbTransitionBit();

    // wait for matrixCalculations to be run for first time inside calcTask - fill initial buffer and set Layer properties that are only set after first pass through matrixCalculations()
    while(rotationChange) {
        delay(1);
    }
}

#define IS_LAST_PANEL_MAP_ENTRY(x) (!x.rowOffset && !x.bufferOffset && !x.numPixels)

template <int dummyvar>
void SmartMatrixHub75Calc_NT<dummyvar>::resetMultiRowRefreshMapPosition(void) {   
    multiRowRefresh_mapIndex_CurrentRowGroups = 0;
    resetMultiRowRefreshMapPositionPixelGroupToStartOfRow();
}

template <int dummyvar>
void SmartMatrixHub75Calc_NT<dummyvar>::resetMultiRowRefreshMapPositionPixelGroupToStartOfRow(void) {   
    multiRowRefresh_mapIndex_CurrentPixelGroup = multiRowRefresh_mapIndex_CurrentRowGroups;
    multiRowRefresh_PixelOffsetFromPanelsAlreadyMapped = 0;
    multiRowRefresh_NumPanelsAlreadyMapped = 0;
}

template <int dummyvar>
void SmartMatrixHub75Calc_NT<dummyvar>::advanceMultiRowRefreshMapToNextRow(void) {   
    static const PanelMappingEntry * map = getMultiRowRefreshPanelMap(panelType);

    int currentRowOffset = map[multiRowRefresh_mapIndex_CurrentRowGroups].rowOffset;

    // advance until end of table, or entry with new row nubmer is found
    while(!IS_LAST_PANEL_MAP_ENTRY(map[multiRowRefresh_mapIndex_CurrentRowGroups])) {
        multiRowRefresh_mapIndex_CurrentRowGroups++;

        if(map[multiRowRefresh_mapIndex_CurrentRowGroups].rowOffset != currentRowOffset)
            break;
    }

    resetMultiRowRefreshMapPositionPixelGroupToStartOfRow();
}

template <int dummyvar>
void SmartMatrixHub75Calc_NT<dummyvar>::advanceMultiRowRefreshMapToNextPixelGroup(void) {   
    static const PanelMappingEntry * map = getMultiRowRefreshPanelMap(panelType);

    int currentRowOffset = map[multiRowRefresh_mapIndex_CurrentPixelGroup].rowOffset;

    // don't change if we're already on the end
    if(IS_LAST_PANEL_MAP_ENTRY(map[multiRowRefresh_mapIndex_CurrentPixelGroup])) {
        return;
    }

    if(!IS_LAST_PANEL_MAP_ENTRY(map[multiRowRefresh_mapIndex_CurrentPixelGroup + 1]) &&
        // go to the next entry if it's in the same row offset
        (map[multiRowRefresh_mapIndex_CurrentPixelGroup + 1].rowOffset == currentRowOffset)) {
        multiRowRefresh_mapIndex_CurrentPixelGroup++;
    } else {
        // else we just finished mapping a panel and we're wrapping to the beginning of this row in the list
        // keep going back until we get to the first entry, or the first entry in this row
        while((multiRowRefresh_mapIndex_CurrentPixelGroup > 0) && (map[multiRowRefresh_mapIndex_CurrentPixelGroup - 1].rowOffset == currentRowOffset))
            multiRowRefresh_mapIndex_CurrentPixelGroup--;

        // we need to set the total offset to the beginning offset of the next panel.  Calculate what that would be
        multiRowRefresh_NumPanelsAlreadyMapped++;
        multiRowRefresh_PixelOffsetFromPanelsAlreadyMapped = multiRowRefresh_NumPanelsAlreadyMapped * cols_per_panel * physical_rows_per_refresh_row;
    }
}

// returns the row offset from the map, or -1 if we've gone through the whole map already
template <int dummyvar>
int SmartMatrixHub75Calc_NT<dummyvar>::getMultiRowRefreshRowOffset(void) {   
    static const PanelMappingEntry * map = getMultiRowRefreshPanelMap(panelType);

    if(IS_LAST_PANEL_MAP_ENTRY(map[multiRowRefresh_mapIndex_CurrentRowGroups])){
        return -1;
    }

    return map[multiRowRefresh_mapIndex_CurrentRowGroups].rowOffset;    
}

template <int dummyvar>
int SmartMatrixHub75Calc_NT<dummyvar>::getMultiRowRefreshNumPixelsToMap(void) {        
    static const PanelMappingEntry * map = getMultiRowRefreshPanelMap(panelType);

    return map[multiRowRefresh_mapIndex_CurrentPixelGroup].numPixels;    
}

template <int dummyvar>
int SmartMatrixHub75Calc_NT<dummyvar>::getMultiRowRefreshPixelGroupOffset(void) {        
    static const PanelMappingEntry * map = getMultiRowRefreshPanelMap(panelType);

    return map[multiRowRefresh_mapIndex_CurrentPixelGroup].bufferOffset + multiRowRefresh_PixelOffsetFromPanelsAlreadyMapped;
}

#define REFRESH_PRINTFS 0

//#define OEPWM_TEST_ENABLE // this is likely broken now
#define OEPWM_THRESHOLD_BIT 1

template <int dummyvar>
INLINE void SmartMatrixHub75Calc_NT<dummyvar>::loadMatrixBuffers48(MATRIX_DATA_STORAGE_TYPE * frameBuffer, int currentRow, int lsbMsbTransitionBit, int numBrightnessShifts) {
    int i;
    int multiRowRefreshRowOffset = 0;
    int numPixelsPerTempRow = pixels_per_latch/physical_rows_per_refresh_row;

#if (REFRESH_PRINTFS >= 1)
    printf("numPixelsPerTempRow = %d\r\n", numPixelsPerTempRow);
#endif

#if defined(ESP32)
    // use buffers malloc'd previously
    rgb48 * tempRow0 = (rgb48*)tempRow0Ptr;
    rgb48 * tempRow1 = (rgb48*)tempRow1Ptr;
#else
    // static to avoid putting large buffer on the stack
    static rgb48 tempRow0[numPixelsPerTempRow];
    static rgb48 tempRow1[numPixelsPerTempRow];
#endif

    int c = 0;
    resetMultiRowRefreshMapPosition();

    // go through this process for each physical row that is contained in the refresh row
    do {
        // clear buffer to prevent garbage data showing through transparent layers
        memset(tempRow0, 0x00, sizeof(rgb48) * numPixelsPerTempRow);
        memset(tempRow1, 0x00, sizeof(rgb48) * numPixelsPerTempRow);

#if (REFRESH_PRINTFS >= 1)
        printf("multiRowRefreshRowOffset = %d\r\n", multiRowRefreshRowOffset);
#endif

        // get a row of physical pixel data (HUB75 paired) from the layers
        SM_Layer * templayer = baseLayer;
        while(templayer) {
            for(i=0; i<matrix_stack_height; i++) {
                // Z-shape, bottom to top
                if(!(optionFlags & SMARTMATRIX_OPTIONS_C_SHAPE_STACKING) &&
                    (optionFlags & SMARTMATRIX_OPTIONS_BOTTOM_TO_TOP_STACKING)) {
                    // fill data from bottom to top, so bottom panel is the one closest to Teensy
                    templayer->fillRefreshRow((currentRow + multiRowRefreshRowOffset) + (matrix_stack_height-i-1)*matrix_panel_height, &tempRow0[i*matrixWidth], numBrightnessShifts);
                    templayer->fillRefreshRow((currentRow + multiRowRefreshRowOffset) + row_pair_offset + (matrix_stack_height-i-1)*matrix_panel_height, &tempRow1[i*matrixWidth], numBrightnessShifts);
                // Z-shape, top to bottom
                } else if(!(optionFlags & SMARTMATRIX_OPTIONS_C_SHAPE_STACKING) &&
                    !(optionFlags & SMARTMATRIX_OPTIONS_BOTTOM_TO_TOP_STACKING)) {
                    // fill data from top to bottom, so top panel is the one closest to Teensy
                    templayer->fillRefreshRow((currentRow + multiRowRefreshRowOffset) + i*matrix_panel_height, &tempRow0[i*matrixWidth], numBrightnessShifts);
                    templayer->fillRefreshRow((currentRow + multiRowRefreshRowOffset) + row_pair_offset + i*matrix_panel_height, &tempRow1[i*matrixWidth], numBrightnessShifts);
                // C-shape, bottom to top
                } else if((optionFlags & SMARTMATRIX_OPTIONS_C_SHAPE_STACKING) &&
                    (optionFlags & SMARTMATRIX_OPTIONS_BOTTOM_TO_TOP_STACKING)) {
                    // alternate direction of filling (or loading) for each matrixwidth
                    // swap row order from top to bottom for each stack (tempRow1 filled with top half of panel, tempRow0 filled with bottom half)
                    if((matrix_stack_height-i+1)%2) {
                        templayer->fillRefreshRow((matrix_scan_mod-(currentRow + multiRowRefreshRowOffset)-1) + row_pair_offset + (i)*matrix_panel_height, &tempRow0[i*matrixWidth], numBrightnessShifts);
                        templayer->fillRefreshRow((matrix_scan_mod-(currentRow + multiRowRefreshRowOffset)-1) + (i)*matrix_panel_height, &tempRow1[i*matrixWidth], numBrightnessShifts);
                    } else {
                        templayer->fillRefreshRow((currentRow + multiRowRefreshRowOffset) + (i)*matrix_panel_height, &tempRow0[i*matrixWidth], numBrightnessShifts);
                        templayer->fillRefreshRow((currentRow + multiRowRefreshRowOffset) + row_pair_offset + (i)*matrix_panel_height, &tempRow1[i*matrixWidth], numBrightnessShifts);
                    }
                // C-shape, top to bottom
                } else if((optionFlags & SMARTMATRIX_OPTIONS_C_SHAPE_STACKING) && 
                    !(optionFlags & SMARTMATRIX_OPTIONS_BOTTOM_TO_TOP_STACKING)) {
                    if((matrix_stack_height-i)%2) {
                        templayer->fillRefreshRow((currentRow + multiRowRefreshRowOffset) + (matrix_stack_height-i-1)*matrix_panel_height, &tempRow0[i*matrixWidth], numBrightnessShifts);
                        templayer->fillRefreshRow((currentRow + multiRowRefreshRowOffset) + row_pair_offset + (matrix_stack_height-i-1)*matrix_panel_height, &tempRow1[i*matrixWidth], numBrightnessShifts);
                    } else {
                        templayer->fillRefreshRow((matrix_scan_mod-(currentRow + multiRowRefreshRowOffset)-1) + row_pair_offset + (matrix_stack_height-i-1)*matrix_panel_height, &tempRow0[i*matrixWidth], numBrightnessShifts);
                        templayer->fillRefreshRow((matrix_scan_mod-(currentRow + multiRowRefreshRowOffset)-1) + (matrix_stack_height-i-1)*matrix_panel_height, &tempRow1[i*matrixWidth], numBrightnessShifts);
                    }
                }
            }
            templayer = templayer->nextLayer;        
        }

        for(int j=0; j<COLOR_DEPTH_BITS; j++) {
            int maskoffset = 0;
            if(COLOR_DEPTH_BITS == 12)   // 36-bit color
                maskoffset = 4;
            else if (COLOR_DEPTH_BITS == 16) // 48-bit color
                maskoffset = 0;
            else if (COLOR_DEPTH_BITS == 8)  // 24-bit color
                maskoffset = 0;

            uint16_t mask = (1 << (j + maskoffset));
            
            MATRIX_DATA_STORAGE_TYPE *p=&(frameBuffer[GET_DATA_OFFSET_FROM_ROW_AND_COLOR_DEPTH_BIT(currentRow, j)]); //bitplane location to write to

            int i=0;

            // reset pixel map offset so we start filling from the first panel again
            resetMultiRowRefreshMapPositionPixelGroupToStartOfRow();

            while(i < numPixelsPerTempRow) {
                // get number of pixels to go through with current pass
                int numPixelsToMap = getMultiRowRefreshNumPixelsToMap();

#if (REFRESH_PRINTFS >= 1)
                printf("numPixelsToMap = %d\r\n", numPixelsToMap);
#endif

                bool reversePixelBlock = false;
                if(numPixelsToMap < 0) {
                    reversePixelBlock = true;
                    numPixelsToMap = abs(numPixelsToMap);
                }

                // get offset where pixels are written in the refresh buffer
                int currentMapOffset = getMultiRowRefreshPixelGroupOffset();

#if (REFRESH_PRINTFS >= 1)
                printf("currentMapOffset = %d\r\n", currentMapOffset);
#endif

                // parse through grouping of pixels, loading from temp buffer and writing to refresh buffer
                for(int k=0; k < numPixelsToMap; k++) {
                    int v=0;

                    int refreshBufferPosition;
                    if(reversePixelBlock) {
                        refreshBufferPosition = currentMapOffset-k;
                    } else {
                        refreshBufferPosition = currentMapOffset+k;
                    }
#if 1
#if (REFRESH_PRINTFS >= 2)
                printf("j = %02d, i = %03d, c = %03d, k = %03d, pos = %03d\r\n", j, i, c, k, refreshBufferPosition);
#endif

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
                    if((refreshBufferPosition) == 0) v|=BIT_OE;

                    // drive latch while shifting out last bit of RGB data
                    if((refreshBufferPosition) == pixels_per_latch-1) v|=BIT_LAT;

                    // experimental FM6126A support on ESP32 without external latch: make LAT pulse 3x clocks wide, matching the FM6126A "DATA_LATCH" command (and not the "RESET_OEN" command)
                    if(optionFlags & SMARTMATRIX_OPTIONS_FM6126A_RESET_AT_START) {
                        if((refreshBufferPosition) == pixels_per_latch-2) v|=BIT_LAT;
                        if((refreshBufferPosition) == pixels_per_latch-3) v|=BIT_LAT;
                    }
#endif

                    // turn off OE after brightness value is reached when displaying MSBs
                    // MSBs always output normal brightness
                    // LSB (!j) outputs normal brightness as MSB from previous row is being displayed
                    if((j > lsbMsbTransitionBit || !j) && ((refreshBufferPosition) >= shiftedBrightness)) v|=BIT_OE;

#ifndef OEPWM_TEST_ENABLE
                    // special case for the bits *after* LSB through (lsbMsbTransitionBit) - OE is output after data is shifted, so need to set OE to fractional brightness
                    if(j && j <= lsbMsbTransitionBit) {
                        // divide brightness in half for each bit below lsbMsbTransitionBit
                        int lsbBrightness = shiftedBrightness >> (lsbMsbTransitionBit - j + 1);
                        if((refreshBufferPosition) >= lsbBrightness) v|=BIT_OE;
                    }
#else
                    // TODO: this is probably not working after adding support for multi-row refresh panels
                    // special case for the bits *after* LSB through (lsbMsbTransitionBit) - OE is output after data is shifted, so need to set OE to fractional brightness
                    if(j && j <= lsbMsbTransitionBit) {
                        // all bits through OEPWM_THRESHOLD_BIT we handle by toggling short PWM pulses smaller than one clock cycle
                        if(j >= 1 && j <= OEPWM_THRESHOLD_BIT) {
                            // width of pwm OE pulse is ~1/2 the width of a DMA OE pulse (so shift lsbPwmBrightnessPulses one fewer times than lsbBrightness)
                            int lsbPwmBrightnessPulses = (shiftedBrightness) >> (lsbMsbTransitionBit - j + 1 - 1);
                            // now setting brightness for LSB, use PWM OE
                            if((k%2) || k >= (2 * lsbPwmBrightnessPulses)) v|=BIT_OE;
                        } else {
                            // divide brightness in half for each bit below lsbMsbTransitionBit
                            int lsbBrightness = shiftedBrightness >> (lsbMsbTransitionBit - j + 1);
                            if((refreshBufferPosition) >= lsbBrightness) v|=BIT_OE;
                        }
                    }                
#endif
                    
                    // need to turn off OE one clock before latch, otherwise can get ghosting
#if (CLKS_DURING_LATCH > 0)
                    if((refreshBufferPosition)==pixels_per_latch-1) v|=BIT_OE;
#else
                    if((refreshBufferPosition)>=pixels_per_latch-2) v|=BIT_OE;
#endif

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

                    if(optionFlags & SMARTMATRIX_OPTIONS_HUB12_MODE) {
                        // HUB12 format inverts the data (assume we're only using R1 for now), and OE signals

                        if(v & BIT_OE) {
                            v = v & ~(BIT_OE);
                        } else {
                            v |= BIT_OE;
                        }

                        if(v & BIT_R1) {
                            v = v & ~(BIT_R1);
                        } else {
                            v |= BIT_R1;
                        }
                    }               
#else
                    v|=BIT_G1;
#endif

                    if((optionFlags & SMARTMATRIX_OPTIONS_C_SHAPE_STACKING) && !((i/matrixWidth)%2)) {
                        //currentRowDataPtr->rowbits[j].data[(((i+matrixWidth-1)-k)*DMA_UPDATES_PER_CLOCK)] = o0.word;
                        //TODO: support C-shape stacking
                    } else {
                        if(MATRIX_I2S_MODE == I2S_PARALLEL_BITS_8) {
                            //Save the calculated value to the bitplane memory in 16-bit reversed order to account for I2S Tx FIFO mode1 ordering
                            if(refreshBufferPosition%4 == 0){
                                //p->data[(refreshBufferPosition)+2] = v;
                                p[(refreshBufferPosition)+2] = v;
                            } else if(refreshBufferPosition%4 == 1) {
                                //p->data[(refreshBufferPosition)+2] = v;
                                p[(refreshBufferPosition)+2] = v;
                            } else if(refreshBufferPosition%4 == 2) {
                                p[(refreshBufferPosition)-2] = v;
                            } else { //if(refreshBufferPosition%4 == 3)
                                p[(refreshBufferPosition)-2] = v;
                            }
                        } else {
                            //Save the calculated value to the bitplane memory in reverse order to account for I2S Tx FIFO mode1 ordering
                            if(refreshBufferPosition%2){
                                p[(refreshBufferPosition)-1] = v;
                            } else {
                                p[(refreshBufferPosition)+1] = v;
                            }
                        }
                    }
                }

                i += numPixelsToMap; // keep track of current position on this temp buffer
                advanceMultiRowRefreshMapToNextPixelGroup();
            }

            // TODO: insert latch data for all color depth bits all at once at the end, saving a few cycles?
            // TODO: prefill latch across all frames during begin() and only need to update when brightness/refreshrate changed?
#if (CLKS_DURING_LATCH > 0)
            // if external latch is used to hold ADDX lines, load the ADDX latch and latch the RGB data here
            for(int k=pixels_per_latch; k < pixels_per_latch + CLKS_DURING_LATCH; k++) {
                int v = 0;
#if 1
                // after data is shifted in, pulse latch for one clock cycle
                if(k == pixels_per_latch) {
                    v|=BIT_LAT;
                }

                //Do not show image while the line bits are changing
                v|=BIT_OE;

                // set ADDX values to high while latch is high, keep them high while latch drops to clock it in to ADDX latch
                if(k >= pixels_per_latch) {               
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

                if(optionFlags & SMARTMATRIX_OPTIONS_HUB12_MODE) {
                    // HUB12 inverts data (irrelevant here) and OE signals
                    if(v & BIT_OE) {
                        v = v & ~(BIT_OE);
                    } else {
                        v |= BIT_OE;
                    }
                }
#else
                v|=BIT_G1;
#endif

                if(MATRIX_I2S_MODE == I2S_PARALLEL_BITS_8) {
                    //Save the calculated value to the bitplane memory in 16-bit reversed order to account for I2S Tx FIFO mode1 ordering
                    if(k%4 == 0){
                        p[k+2] = v;
                    } else if(k%4 == 1) {
                        p[k+2] = v;
                    } else if(k%4 == 2) {
                        p[k-2] = v;
                    } else { //if(k%4 == 3)
                        p[k-2] = v;
                    }
                } else {
                    //Save the calculated value to the bitplane memory in reverse order to account for I2S Tx FIFO mode1 ordering
                    if(k%2){
                        p[k-1] = v;
                    } else {
                        p[k+1] = v;
                    }
                }
            }
#endif
        }

        c += numPixelsPerTempRow; // keep track of cumulative number of pixels filled in refresh buffer before this temp buffer

        advanceMultiRowRefreshMapToNextRow();
        multiRowRefreshRowOffset = getMultiRowRefreshRowOffset();
    } while (multiRowRefreshRowOffset > 0);
}

template <int dummyvar>
INLINE void SmartMatrixHub75Calc_NT<dummyvar>::loadMatrixBuffers24(MATRIX_DATA_STORAGE_TYPE * frameBuffer, int currentRow, int lsbMsbTransitionBit, int numBrightnessShifts) {
    int i;
    int multiRowRefreshRowOffset = 0;
    int numPixelsPerTempRow = pixels_per_latch/physical_rows_per_refresh_row;

#if defined(ESP32)
    // use buffers malloc'd previously
    rgb24 * tempRow0 = (rgb24*)tempRow0Ptr;
    rgb24 * tempRow1 = (rgb24*)tempRow1Ptr;
#else
    // static to avoid putting large buffer on the stack
    static rgb24 tempRow0[numPixelsPerTempRow];
    static rgb24 tempRow1[numPixelsPerTempRow];
#endif

    int c = 0;
    resetMultiRowRefreshMapPosition();

    // go through this process for each physical row that is contained in the refresh row
    do {
        // clear buffer to prevent garbage data showing through transparent layers
        memset(tempRow0, 0x00, sizeof(rgb24) * numPixelsPerTempRow);
        memset(tempRow1, 0x00, sizeof(rgb24) * numPixelsPerTempRow);

        // get a row of physical pixel data (HUB75 paired) from the layers
        SM_Layer * templayer = baseLayer;
        while(templayer) {
            for(i=0; i<matrix_stack_height; i++) {
                // Z-shape, bottom to top
                if(!(optionFlags & SMARTMATRIX_OPTIONS_C_SHAPE_STACKING) &&
                    (optionFlags & SMARTMATRIX_OPTIONS_BOTTOM_TO_TOP_STACKING)) {
                    // fill data from bottom to top, so bottom panel is the one closest to Teensy
                    templayer->fillRefreshRow((currentRow + multiRowRefreshRowOffset) + (matrix_stack_height-i-1)*matrix_panel_height, &tempRow0[i*matrixWidth], numBrightnessShifts);
                    templayer->fillRefreshRow((currentRow + multiRowRefreshRowOffset) + row_pair_offset + (matrix_stack_height-i-1)*matrix_panel_height, &tempRow1[i*matrixWidth], numBrightnessShifts);
                // Z-shape, top to bottom
                } else if(!(optionFlags & SMARTMATRIX_OPTIONS_C_SHAPE_STACKING) &&
                    !(optionFlags & SMARTMATRIX_OPTIONS_BOTTOM_TO_TOP_STACKING)) {
                    // fill data from top to bottom, so top panel is the one closest to Teensy
                    templayer->fillRefreshRow((currentRow + multiRowRefreshRowOffset) + i*matrix_panel_height, &tempRow0[i*matrixWidth], numBrightnessShifts);
                    templayer->fillRefreshRow((currentRow + multiRowRefreshRowOffset) + row_pair_offset + i*matrix_panel_height, &tempRow1[i*matrixWidth], numBrightnessShifts);
                // C-shape, bottom to top
                } else if((optionFlags & SMARTMATRIX_OPTIONS_C_SHAPE_STACKING) &&
                    (optionFlags & SMARTMATRIX_OPTIONS_BOTTOM_TO_TOP_STACKING)) {
                    // alternate direction of filling (or loading) for each matrixwidth
                    // swap row order from top to bottom for each stack (tempRow1 filled with top half of panel, tempRow0 filled with bottom half)
                    if((matrix_stack_height-i+1)%2) {
                        templayer->fillRefreshRow((matrix_scan_mod-(currentRow + multiRowRefreshRowOffset)-1) + row_pair_offset + (i)*matrix_panel_height, &tempRow0[i*matrixWidth], numBrightnessShifts);
                        templayer->fillRefreshRow((matrix_scan_mod-(currentRow + multiRowRefreshRowOffset)-1) + (i)*matrix_panel_height, &tempRow1[i*matrixWidth], numBrightnessShifts);
                    } else {
                        templayer->fillRefreshRow((currentRow + multiRowRefreshRowOffset) + (i)*matrix_panel_height, &tempRow0[i*matrixWidth], numBrightnessShifts);
                        templayer->fillRefreshRow((currentRow + multiRowRefreshRowOffset) + row_pair_offset + (i)*matrix_panel_height, &tempRow1[i*matrixWidth], numBrightnessShifts);
                    }
                // C-shape, top to bottom
                } else if((optionFlags & SMARTMATRIX_OPTIONS_C_SHAPE_STACKING) && 
                    !(optionFlags & SMARTMATRIX_OPTIONS_BOTTOM_TO_TOP_STACKING)) {
                    if((matrix_stack_height-i)%2) {
                        templayer->fillRefreshRow((currentRow + multiRowRefreshRowOffset) + (matrix_stack_height-i-1)*matrix_panel_height, &tempRow0[i*matrixWidth], numBrightnessShifts);
                        templayer->fillRefreshRow((currentRow + multiRowRefreshRowOffset) + row_pair_offset + (matrix_stack_height-i-1)*matrix_panel_height, &tempRow1[i*matrixWidth], numBrightnessShifts);
                    } else {
                        templayer->fillRefreshRow((matrix_scan_mod-(currentRow + multiRowRefreshRowOffset)-1) + row_pair_offset + (matrix_stack_height-i-1)*matrix_panel_height, &tempRow0[i*matrixWidth], numBrightnessShifts);
                        templayer->fillRefreshRow((matrix_scan_mod-(currentRow + multiRowRefreshRowOffset)-1) + (matrix_stack_height-i-1)*matrix_panel_height, &tempRow1[i*matrixWidth], numBrightnessShifts);
                    }
                }
            }
            templayer = templayer->nextLayer;        
        }
  
        for(int j=0; j<COLOR_DEPTH_BITS; j++) {
            int maskoffset = 0;
            if(COLOR_DEPTH_BITS == 12)   // 36-bit color
                maskoffset = 4;
            else if (COLOR_DEPTH_BITS == 16) // 48-bit color
                maskoffset = 0;
            else if (COLOR_DEPTH_BITS == 8)  // 24-bit color
                maskoffset = 0;

            uint16_t mask = (1 << (j + maskoffset));
            
            MATRIX_DATA_STORAGE_TYPE *p=&(frameBuffer[GET_DATA_OFFSET_FROM_ROW_AND_COLOR_DEPTH_BIT(currentRow, j)]); //bitplane location to write to
            
            int i=0;

            // reset pixel map offset so we start filling from the first panel again
            resetMultiRowRefreshMapPositionPixelGroupToStartOfRow();

            while(i < numPixelsPerTempRow) {
                // get number of pixels to go through with current pass
                int numPixelsToMap = getMultiRowRefreshNumPixelsToMap();

                bool reversePixelBlock = false;
                if(numPixelsToMap < 0) {
                    reversePixelBlock = true;
                    numPixelsToMap = abs(numPixelsToMap);
                }

                // get offset where pixels are written in the refresh buffer
                int currentMapOffset = getMultiRowRefreshPixelGroupOffset();

                // parse through grouping of pixels, loading from temp buffer and writing to refresh buffer
                for(int k=0; k < numPixelsToMap; k++) {
                    int v=0;

                    int refreshBufferPosition;
                    if(reversePixelBlock) {
                        refreshBufferPosition = currentMapOffset-k;
                    } else {
                        refreshBufferPosition = currentMapOffset+k;
                    }

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
                    if((refreshBufferPosition) == 0) v|=BIT_OE;

                    // drive latch while shifting out last bit of RGB data
                    if((refreshBufferPosition) == pixels_per_latch-1) v|=BIT_LAT;

                    // experimental FM6126A support on ESP32 without external latch: make LAT pulse 3x clocks wide, matching the FM6126A "DATA_LATCH" command (and not the "RESET_OEN" command)
                    if(optionFlags & SMARTMATRIX_OPTIONS_FM6126A_RESET_AT_START) {
                        if((refreshBufferPosition) == pixels_per_latch-2) v|=BIT_LAT;
                        if((refreshBufferPosition) == pixels_per_latch-3) v|=BIT_LAT;
                    }
#endif

                    // turn off OE after brightness value is reached when displaying MSBs
                    // MSBs always output normal brightness
                    // LSB (!j) outputs normal brightness as MSB from previous row is being displayed
                    if((j > lsbMsbTransitionBit || !j) && ((refreshBufferPosition) >= shiftedBrightness)) v|=BIT_OE;

                    // special case for the bits *after* LSB through (lsbMsbTransitionBit) - OE is output after data is shifted, so need to set OE to fractional brightness
                    if(j && j <= lsbMsbTransitionBit) {
                        // divide brightness in half for each bit below lsbMsbTransitionBit
                        int lsbBrightness = shiftedBrightness >> (lsbMsbTransitionBit - j + 1);
                        if((refreshBufferPosition) >= lsbBrightness) v|=BIT_OE;
                    }

                    // need to turn off OE one clock before latch, otherwise can get ghosting
#if (CLKS_DURING_LATCH > 0)
                    if((refreshBufferPosition)==pixels_per_latch-1) v|=BIT_OE;
#else
                    if((refreshBufferPosition)>=pixels_per_latch-2) v|=BIT_OE;
#endif

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

                    if(optionFlags & SMARTMATRIX_OPTIONS_HUB12_MODE) {
                        // HUB12 format inverts the data (assume we're only using R1 for now), and OE signals

                        if(v & BIT_OE) {
                            v = v & ~(BIT_OE);
                        } else {
                            v |= BIT_OE;
                        }

                        if(v & BIT_R1) {
                            v = v & ~(BIT_R1);
                        } else {
                            v |= BIT_R1;
                        }
                    }               

                    if((optionFlags & SMARTMATRIX_OPTIONS_C_SHAPE_STACKING) && !((i/matrixWidth)%2)) {
                        //currentRowDataPtr->rowbits[j].data[(((i+matrixWidth-1)-k)*DMA_UPDATES_PER_CLOCK)] = o0.word;
                        //TODO: support C-shape stacking
                    } else {
                        if(MATRIX_I2S_MODE == I2S_PARALLEL_BITS_8) {
                            //Save the calculated value to the bitplane memory in 16-bit reversed order to account for I2S Tx FIFO mode1 ordering
                            if(refreshBufferPosition%4 == 0){
                                p[(refreshBufferPosition)+2] = v;
                            } else if(refreshBufferPosition%4 == 1) {
                                p[(refreshBufferPosition)+2] = v;
                            } else if(refreshBufferPosition%4 == 2) {
                                p[(refreshBufferPosition)-2] = v;
                            } else { //if(refreshBufferPosition%4 == 3)
                                p[(refreshBufferPosition)-2] = v;
                            }
                        } else {
                            //Save the calculated value to the bitplane memory in reverse order to account for I2S Tx FIFO mode1 ordering
                            if(refreshBufferPosition%2){
                                p[(refreshBufferPosition)-1] = v;
                            } else {
                                p[(refreshBufferPosition)+1] = v;
                            }
                        }
                    }
                }

                i += numPixelsToMap; // keep track of current position on this temp buffer
                advanceMultiRowRefreshMapToNextPixelGroup();
            }

#if (CLKS_DURING_LATCH > 0)
            // if external latch is used to hold ADDX lines, load the ADDX latch and latch the RGB data here
            for(int k=pixels_per_latch; k < pixels_per_latch + CLKS_DURING_LATCH; k++) {
                int v = 0;
                // after data is shifted in, pulse latch for one clock cycle
                if(k == pixels_per_latch) {
                    v|=BIT_LAT;
                }

                //Do not show image while the line bits are changing
                v|=BIT_OE;

                // set ADDX values to high while latch is high, keep them high while latch drops to clock it in to ADDX latch
                if(k >= pixels_per_latch) {               
                    if (currentRow & 0x01) v|=BIT_R1;
                    if (currentRow & 0x02) v|=BIT_G1;
                    if (currentRow & 0x04) v|=BIT_B1;
                    if (currentRow & 0x08) v|=BIT_R2;
                    if (currentRow & 0x10) v|=BIT_G2;
                    // reserve B2 for OE SWITCH
                }

                if(optionFlags & SMARTMATRIX_OPTIONS_HUB12_MODE) {
                    // HUB12 inverts data (irrelevant here) and OE signals
                    if(v & BIT_OE) {
                        v = v & ~(BIT_OE);
                    } else {
                        v |= BIT_OE;
                    }
                }

                if(MATRIX_I2S_MODE == I2S_PARALLEL_BITS_8) {
                    //Save the calculated value to the bitplane memory in 16-bit reversed order to account for I2S Tx FIFO mode1 ordering
                    if(k%4 == 0){
                        p[k+2] = v;
                    } else if(k%4 == 1) {
                        p[k+2] = v;
                    } else if(k%4 == 2) {
                        p[k-2] = v;
                    } else { //if(k%4 == 3)
                        p[k-2] = v;
                    }
                } else {
                    //Save the calculated value to the bitplane memory in reverse order to account for I2S Tx FIFO mode1 ordering
                    if(k%2){
                        p[k-1] = v;
                    } else {
                        p[k+1] = v;
                    }
                }
            }
#endif
        }

        c += numPixelsPerTempRow; // keep track of cumulative number of pixels filled in refresh buffer before this temp buffer

        advanceMultiRowRefreshMapToNextRow();
        multiRowRefreshRowOffset = getMultiRowRefreshRowOffset();
    } while (multiRowRefreshRowOffset > 0);
}

template <int dummyvar>
INLINE void SmartMatrixHub75Calc_NT<dummyvar>::loadMatrixBuffers(int lsbMsbTransitionBit, int numBrightnessShifts) {
#if 1
    unsigned char currentRow;

    MATRIX_DATA_STORAGE_TYPE * currentFrameDataPtr = _matrixRefresh->getNextFrameBufferPtr();

    for(currentRow = 0; currentRow < matrix_scan_mod; currentRow++) {
        // TODO: support rgb36/48 with same function, copy function to rgb24
        if(COLOR_DEPTH_BITS == 16)
            loadMatrixBuffers48(currentFrameDataPtr, currentRow, lsbMsbTransitionBit, numBrightnessShifts);
        else if(COLOR_DEPTH_BITS == 12)
            loadMatrixBuffers48(currentFrameDataPtr, currentRow, lsbMsbTransitionBit, numBrightnessShifts);
        else if(COLOR_DEPTH_BITS == 8)
            loadMatrixBuffers24(currentFrameDataPtr, currentRow, lsbMsbTransitionBit, numBrightnessShifts);
    }
#endif
}
