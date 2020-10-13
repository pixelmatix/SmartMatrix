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
#include "ESP32MemDisplay.h"

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
int SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::multiRowRefresh_mapIndex_CurrentRowGroups = 0;

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
int SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::multiRowRefresh_mapIndex_CurrentPixelGroup = -1;

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
int SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::multiRowRefresh_PixelOffsetFromPanelsAlreadyMapped = 0;

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
int SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::multiRowRefresh_NumPanelsAlreadyMapped = 0;

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
int SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::countFPS(void) {
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

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
void SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::dmaBufferUnderrunCallback(void) {
    dmaBufferUnderrun = true;
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
void SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::matrixCalculations() {
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
    if(totalCpuTime && ((calculationCpuTime * 100) / totalCpuTime) > SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::maxCalcCpuPercentage) {
        // increase CPU divider by 1
        SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::setCalcRefreshRateDivider(SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::getCalcRefreshRateDivider() + 1);
        refreshRateLowered = true;
    }

    // only do calculations if there is free space (should be redundant, as we only get called if there is free space)
    if (!SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::isFrameBufferFree())
        return;

    templayer = SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::baseLayer;
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
        templayer = SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::baseLayer;
        while(templayer) {
            templayer->setRotation(rotation);
            templayer = templayer->nextLayer;
        }
        rotationChange = false;
    }

    int largestRequestedBrightnessShifts = 0;

    templayer = SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::baseLayer;
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
        SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::setBrightness(shiftedBrightness);
        brightnessChange = false;
    }

    SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::loadMatrixBuffers(lsbMsbTransitionBit, largestRequestedBrightnessShifts);

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
template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
int SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::shiftedBrightness;

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
    static long lastMillis = 0;
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
            SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::markRefreshComplete();

            matrixCalculations();

#ifdef DEBUG_PINS_ENABLED
            gpio_set_level(DEBUG_1_GPIO, 0);
#endif
        }
    }
}

#define MATRIX_CALC_TASK_DEFAULT_PRIORITY   2
#define MATRIX_CALC_TASK_LOW_PRIORITY      1

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
void SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::begin(uint32_t dmaRamToKeepFreeBytes)
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
    xTaskCreatePinnedToCore(calcTask, "SmartMatrixCalc", 1000, NULL, taskPriority, &calcTaskHandle, calcTaskCore);

    printf("SmartMatrix Layers Allocated from Heap:\r\n");
    show_esp32_heap_mem();

#if defined(ESP32)
    // malloc temporary buffers needed for loadMatrixBuffers
    int numPixelsPerTempRow = PIXELS_PER_LATCH/PHYSICAL_ROWS_PER_REFRESH_ROW;

    if(COLOR_DEPTH_BITS > 8) {
        tempRow0Ptr = malloc(sizeof(rgb48) * numPixelsPerTempRow);
        tempRow1Ptr = malloc(sizeof(rgb48) * numPixelsPerTempRow);
    } else {
        tempRow0Ptr = malloc(sizeof(rgb24) * numPixelsPerTempRow);
        tempRow1Ptr = malloc(sizeof(rgb24) * numPixelsPerTempRow);
    }

    assert(tempRow0Ptr != NULL);
    assert(tempRow1Ptr != NULL);
#endif

    SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::setMatrixCalculationsCallback(matrixCalculationsSignal);
    SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::begin(dmaRamToKeepFreeBytes);

    // refresh rate is now set, update calc refresh rate
    setCalcRefreshRateDivider(calc_refreshRateDivider);
    lsbMsbTransitionBit = SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::getLsbMsbTransitionBit();

    // wait for matrixCalculations to be run for first time inside calcTask - fill initial buffer and set Layer properties that are only set after first pass through matrixCalculations()
    while(rotationChange) {
        delay(1);
    }
}

#define IS_LAST_PANEL_MAP_ENTRY(x) (!x.rowOffset && !x.bufferOffset && !x.numPixels)

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
void SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::resetMultiRowRefreshMapPosition(void) {   
    multiRowRefresh_mapIndex_CurrentRowGroups = 0;
    resetMultiRowRefreshMapPositionPixelGroupToStartOfRow();
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
void SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::resetMultiRowRefreshMapPositionPixelGroupToStartOfRow(void) {   
    multiRowRefresh_mapIndex_CurrentPixelGroup = multiRowRefresh_mapIndex_CurrentRowGroups;
    multiRowRefresh_PixelOffsetFromPanelsAlreadyMapped = 0;
    multiRowRefresh_NumPanelsAlreadyMapped = 0;
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
void SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::advanceMultiRowRefreshMapToNextRow(void) {   
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

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
void SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::advanceMultiRowRefreshMapToNextPixelGroup(void) {   
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
        multiRowRefresh_PixelOffsetFromPanelsAlreadyMapped = multiRowRefresh_NumPanelsAlreadyMapped * COLS_PER_PANEL * PHYSICAL_ROWS_PER_REFRESH_ROW;
    }
}

// returns the row offset from the map, or -1 if we've gone through the whole map already
template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
int SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::getMultiRowRefreshRowOffset(void) {   
    static const PanelMappingEntry * map = getMultiRowRefreshPanelMap(panelType);

    if(IS_LAST_PANEL_MAP_ENTRY(map[multiRowRefresh_mapIndex_CurrentRowGroups])){
        return -1;
    }

    return map[multiRowRefresh_mapIndex_CurrentRowGroups].rowOffset;    
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
int SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::getMultiRowRefreshNumPixelsToMap(void) {        
    static const PanelMappingEntry * map = getMultiRowRefreshPanelMap(panelType);

    return map[multiRowRefresh_mapIndex_CurrentPixelGroup].numPixels;    
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
int SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::getMultiRowRefreshPixelGroupOffset(void) {        
    static const PanelMappingEntry * map = getMultiRowRefreshPanelMap(panelType);

    return map[multiRowRefresh_mapIndex_CurrentPixelGroup].bufferOffset + multiRowRefresh_PixelOffsetFromPanelsAlreadyMapped;
}

#define REFRESH_PRINTFS 0

//#define OEPWM_TEST_ENABLE // this is likely broken now
#define OEPWM_THRESHOLD_BIT 1

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags> template <typename RGB_TEMP>
INLINE void SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::loadMatrixBuffers48(frameStruct * currentFrameDataPtr, int currentRow, int lsbMsbTransitionBit, int numBrightnessShifts, RGB_TEMP tempBufferType) {
    int i;
    int multiRowRefreshRowOffset = 0;
    int numPixelsPerTempRow = PIXELS_PER_LATCH/PHYSICAL_ROWS_PER_REFRESH_ROW;

#if (REFRESH_PRINTFS >= 1)
    printf("numPixelsPerTempRow = %d\r\n", numPixelsPerTempRow);
#endif

#if defined(ESP32)
    // use buffers malloc'd previously
    RGB_TEMP * tempRow0 = (RGB_TEMP*)tempRow0Ptr;
    RGB_TEMP * tempRow1 = (RGB_TEMP*)tempRow1Ptr;
#else
    // static to avoid putting large buffer on the stack
    static RGB_TEMP tempRow0[numPixelsPerTempRow];
    static RGB_TEMP tempRow1[numPixelsPerTempRow];
#endif

    int c = 0;

    // multi row refresh isn't very efficient, slowing this function down by ~30% for panels that don't even need multi row refresh.  For now, only enable the code if needed
    if(MULTI_ROW_REFRESH_REQUIRED) { 
        resetMultiRowRefreshMapPosition();
    }

    // go through this process for each physical row that is contained in the refresh row
    do {
        // clear buffer to prevent garbage data showing through transparent layers
        memset(tempRow0, 0x00, sizeof(RGB_TEMP) * numPixelsPerTempRow);
        memset(tempRow1, 0x00, sizeof(RGB_TEMP) * numPixelsPerTempRow);

#if (REFRESH_PRINTFS >= 1)
        printf("multiRowRefreshRowOffset = %d\r\n", multiRowRefreshRowOffset);
#endif

        // get a row of physical pixel data (HUB75 paired) from the layers
        SM_Layer * templayer = SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::baseLayer;
        int y0, y1; // positions of the two rows we need
        while(templayer) {
            for (i = 0; i < MATRIX_STACK_HEIGHT; i++) {
                // Z-shape, bottom to top
                if (!(optionFlags & SMARTMATRIX_OPTIONS_C_SHAPE_STACKING) &&
                        (optionFlags & SMARTMATRIX_OPTIONS_BOTTOM_TO_TOP_STACKING)) {
                    // Bottom to Top Stacking: load data buffer with top panels first, bottom panels last, as top panels are at the furthest end of the chain (initial data is shifted out the furthest)
                    y0 = currentRow + multiRowRefreshRowOffset + i * MATRIX_PANEL_HEIGHT;
                    y1 = y0 + ROW_PAIR_OFFSET;
                // Z-shape, top to bottom
                } else if (!(optionFlags & SMARTMATRIX_OPTIONS_C_SHAPE_STACKING) &&
                           !(optionFlags & SMARTMATRIX_OPTIONS_BOTTOM_TO_TOP_STACKING)) {
                    // Top to Bottom Stacking: load data buffer with bottom panels first, top panels last, as bottom panels are at the furthest end of the chain (initial data is shifted out the furthest)
                    y0 = currentRow + multiRowRefreshRowOffset + (MATRIX_STACK_HEIGHT - i - 1) * MATRIX_PANEL_HEIGHT;
                    y1 = y0 + ROW_PAIR_OFFSET;
                // C-shape, bottom to top
                } else if ((optionFlags & SMARTMATRIX_OPTIONS_C_SHAPE_STACKING) &&
                           (optionFlags & SMARTMATRIX_OPTIONS_BOTTOM_TO_TOP_STACKING)) {
                    // C-shaped stacking: alternate direction of filling (or loading) for each matrixwidth-sized stack, stack closest to Teensy is right-side up
                    //   swap row order from top to bottom for each stack (tempRow1 filled with top half of panel, tempRow0 filled with bottom half when upside down)
                    //   the last stack is always right-side up, figure out orientation of other stacks based on that
                    // Bottom to Top Stacking: load data buffer with top panels first, bottom panels last, as top panels are at the furthest end of the chain (initial data is shifted out the furthest)

                    // is i the last stack, or an even number of stacks away from the last stack?
                    if((i % 2) == ((MATRIX_STACK_HEIGHT - 1) % 2)) {
                        y0 = currentRow + multiRowRefreshRowOffset + (i) * MATRIX_PANEL_HEIGHT;
                        y1 = y0 + ROW_PAIR_OFFSET;
                    } else {
                        y1 = (MATRIX_SCAN_MOD - currentRow + multiRowRefreshRowOffset - 1) + (i) * MATRIX_PANEL_HEIGHT;
                        y0 = y1 + ROW_PAIR_OFFSET;
                    }
                // C-shape, top to bottom
                } else if ((optionFlags & SMARTMATRIX_OPTIONS_C_SHAPE_STACKING) &&
                           !(optionFlags & SMARTMATRIX_OPTIONS_BOTTOM_TO_TOP_STACKING)) {
                    // C-shaped stacking: alternate direction of filling (or loading) for each matrixwidth-sized stack, stack closest to Teensy is right-side up
                    //   swap row order from top to bottom for each stack (tempRow1 filled with top half of panel, tempRow0 filled with bottom half when upside down)
                    //   the last stack is always right-side up, figure out orientation of other stacks based on that
                    // Top to Bottom Stacking: load data buffer with bottom panels first, top panels last, as bottom panels are at the furthest end of the chain (initial data is shifted out the furthest)

                    // is i the last stack, or an even number of stacks away from the last stack?
                    if((i % 2) == ((MATRIX_STACK_HEIGHT - 1) % 2)) {
                        y0 = currentRow + multiRowRefreshRowOffset + (MATRIX_STACK_HEIGHT - i - 1) * MATRIX_PANEL_HEIGHT;
                        y1 = y0 + ROW_PAIR_OFFSET;
                    } else {
                        y1 = (MATRIX_SCAN_MOD - currentRow + multiRowRefreshRowOffset - 1) + (MATRIX_STACK_HEIGHT - i - 1) * MATRIX_PANEL_HEIGHT;
                        y0 = y1 + ROW_PAIR_OFFSET;
                    }
                }
                templayer->fillRefreshRow(y0, &tempRow0[i * matrixWidth], numBrightnessShifts);
                templayer->fillRefreshRow(y1, &tempRow1[i * matrixWidth], numBrightnessShifts);
            }
            templayer = templayer->nextLayer;        
        }

        int i=0;

        if(MULTI_ROW_REFRESH_REQUIRED) { 
            // reset pixel map offset so we start filling from the first panel again
            resetMultiRowRefreshMapPositionPixelGroupToStartOfRow();
        }

        while(i < numPixelsPerTempRow) {
            int numPixelsToMap;
            int currentMapOffset;
            bool reversePixelBlock;

            if(MULTI_ROW_REFRESH_REQUIRED) { 
                // get number of pixels to go through with current pass
                numPixelsToMap = getMultiRowRefreshNumPixelsToMap();

#if (REFRESH_PRINTFS >= 1)
                printf("numPixelsToMap = %d\r\n", numPixelsToMap);
#endif

                reversePixelBlock = false;
                if(numPixelsToMap < 0) {
                    reversePixelBlock = true;
                    numPixelsToMap = abs(numPixelsToMap);
                }

                // get offset where pixels are written in the refresh buffer
                currentMapOffset = getMultiRowRefreshPixelGroupOffset();

#if (REFRESH_PRINTFS >= 1)
                printf("currentMapOffset = %d\r\n", currentMapOffset);
#endif
            } else {
                numPixelsToMap = matrixWidth;
            }

            // parse through grouping of pixels, loading from temp buffer and writing to refresh buffer
            for(int k=0; k < numPixelsToMap; k++) {
                uint16_t r0, g0, b0, r1, g1, b1;
                int ind;
                int v;

                int refreshBufferPosition;
                if(MULTI_ROW_REFRESH_REQUIRED) { 
                    if(reversePixelBlock) {
                        refreshBufferPosition = currentMapOffset-k;
                    } else {
                        refreshBufferPosition = currentMapOffset+k;
                    }
                } else {
                    refreshBufferPosition = i+k;
                }

#if (REFRESH_PRINTFS >= 2)
                printf("bitindex = %02d, i = %03d, c = %03d, k = %03d, pos = %03d\r\n", bitindex, i, c, k, refreshBufferPosition);
#endif

                // for upside down stacks, flip order
                int currentStack = i/matrixWidth;
                if((optionFlags & SMARTMATRIX_OPTIONS_C_SHAPE_STACKING) && !((currentStack % 2) == ((MATRIX_STACK_HEIGHT - 1) % 2))) {
                    // reverse order of this stack's data if it's reversed (if currentStack is the last stack, or an even number of stacks away from the last stack)
                    ind = (currentStack*matrixWidth) + (matrixWidth-1) - ((i+k)%matrixWidth);
                } else {
                    // load data to buffer in normal order
                    ind = i+k;
                }
                r0 = tempRow0[ind].red;
                g0 = tempRow0[ind].green;
                b0 = tempRow0[ind].blue;
                r1 = tempRow1[ind].red;
                g1 = tempRow1[ind].green;
                b1 = tempRow1[ind].blue;

                // loop through each bitplane in the current pixel's RGB values and format the bits to match the FlexIO pin configuration
                int sizeOfSourceColor = (sizeof(RGB_TEMP) <= 3) ? 8 : 16;
                uint8_t shift = (sizeOfSourceColor - COLOR_DEPTH_BITS);
                uint16_t mask = 1 << shift;

                for (int bitindex = 0; bitindex < COLOR_DEPTH_BITS; bitindex++) {
                    v=0;
#if (CLKS_DURING_LATCH == 0)
                    // if there is no latch to hold address, output ADDX lines directly to GPIO and latch data at end of cycle
                    int gpioRowAddress = currentRow;
                    // normally output current rows ADDX, special case for LSB, output previous row's ADDX (as previous row is being displayed for one latch cycle)
                    if(bitindex == 0)
                        gpioRowAddress = currentRow-1;

                    if (gpioRowAddress & 0x01) v|=BIT_A;
                    if (gpioRowAddress & 0x02) v|=BIT_B;
                    if (gpioRowAddress & 0x04) v|=BIT_C;
                    if (gpioRowAddress & 0x08) v|=BIT_D;
                    if (gpioRowAddress & 0x10) v|=BIT_E;

                    // need to disable OE after latch to hide row transition
                    if((refreshBufferPosition) == 0) v|=BIT_OE;

                    // drive latch while shifting out last bit of RGB data
                    if((refreshBufferPosition) == PIXELS_PER_LATCH-1) v|=BIT_LAT;

                    // experimental FM6126A support on ESP32 without external latch: make LAT pulse 3x clocks wide, matching the FM6126A "DATA_LATCH" command (and not the "RESET_OEN" command)
                    if(optionFlags & SMARTMATRIX_OPTIONS_FM6126A_RESET_AT_START) {
                        if((refreshBufferPosition) == PIXELS_PER_LATCH-2) v|=BIT_LAT;
                        if((refreshBufferPosition) == PIXELS_PER_LATCH-3) v|=BIT_LAT;
                    }
#endif

                    // turn off OE after brightness value is reached when displaying MSBs
                    // MSBs always output normal brightness
                    // LSB (!j) outputs normal brightness as MSB from previous row is being displayed
                    if((bitindex > lsbMsbTransitionBit || !bitindex) && ((refreshBufferPosition) >= shiftedBrightness)) v|=BIT_OE;

                    // special case for the bits *after* LSB through (lsbMsbTransitionBit) - OE is output after data is shifted, so need to set OE to fractional brightness
                    if(bitindex && bitindex <= lsbMsbTransitionBit) {
                        // divide brightness in half for each bit below lsbMsbTransitionBit
                        int lsbBrightness = shiftedBrightness >> (lsbMsbTransitionBit - bitindex + 1);
                        if((refreshBufferPosition) >= lsbBrightness) v|=BIT_OE;
                    }

                    // need to turn off OE one clock before latch, otherwise can get ghosting
#if (CLKS_DURING_LATCH > 0)
                    if((refreshBufferPosition)==PIXELS_PER_LATCH-1) v|=BIT_OE;
#else
                    if((refreshBufferPosition)>=PIXELS_PER_LATCH-2) v|=BIT_OE;
#endif

                    if (r0 & mask)
                        v|=BIT_R1;
                    if (g0 & mask)
                        v|=BIT_G1;
                    if (b0 & mask)
                        v|=BIT_B1;
                    if (r1 & mask)
                        v|=BIT_R2;
                    if (g1 & mask)
                        v|=BIT_G2;
                    if (b1 & mask)
                        v|=BIT_B2;

                    mask <<= 1;

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

                    // TODO: can this be done more efficiently?
                    SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::rowBitStruct *p=&(currentFrameDataPtr->rowdata[currentRow].rowbits[bitindex]); //bitplane location to write to

                    if(MATRIX_I2S_MODE == I2S_PARALLEL_BITS_8) {
                        //Save the calculated value to the bitplane memory in 16-bit reversed order to account for I2S Tx FIFO mode1 ordering
                        if(refreshBufferPosition%4 == 0){
                            p->data[(refreshBufferPosition)+2] = v;
                        } else if(refreshBufferPosition%4 == 1) {
                            p->data[(refreshBufferPosition)+2] = v;
                        } else if(refreshBufferPosition%4 == 2) {
                            p->data[(refreshBufferPosition)-2] = v;
                        } else { //if(refreshBufferPosition%4 == 3)
                            p->data[(refreshBufferPosition)-2] = v;
                        }
                    } else {
                        //Save the calculated value to the bitplane memory in reverse order to account for I2S Tx FIFO mode1 ordering
                        if(refreshBufferPosition%2){
                            p->data[(refreshBufferPosition)-1] = v;
                        } else {
                            p->data[(refreshBufferPosition)+1] = v;
                        }
                    }
                }
            }
            i += numPixelsToMap; // keep track of current position on this temp buffer
            if(MULTI_ROW_REFRESH_REQUIRED) { 
                advanceMultiRowRefreshMapToNextPixelGroup();
            }
        }

        for (int bitindex = 0; bitindex < COLOR_DEPTH_BITS; bitindex++) {
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
                }

                if(optionFlags & SMARTMATRIX_OPTIONS_HUB12_MODE) {
                    // HUB12 inverts data (irrelevant here) and OE signals
                    if(v & BIT_OE) {
                        v = v & ~(BIT_OE);
                    } else {
                        v |= BIT_OE;
                    }
                }

                // TODO: can this be done more efficiently?
                SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::rowBitStruct *p=&(currentFrameDataPtr->rowdata[currentRow].rowbits[bitindex]); //bitplane location to write to

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

        if(MULTI_ROW_REFRESH_REQUIRED) { 
            c += numPixelsPerTempRow; // keep track of cumulative number of pixels filled in refresh buffer before this temp buffer

            advanceMultiRowRefreshMapToNextRow();
            multiRowRefreshRowOffset = getMultiRowRefreshRowOffset();
        }
    } while (MULTI_ROW_REFRESH_REQUIRED ? (multiRowRefreshRowOffset > 0) : 0);
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
INLINE void SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::loadMatrixBuffers(int lsbMsbTransitionBit, int numBrightnessShifts) {
    unsigned char currentRow;

    frameStruct * currentFrameDataPtr = SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::getNextFrameBufferPtr();

    for(currentRow = 0; currentRow < MATRIX_SCAN_MOD; currentRow++) {
        // same function supports any refresh depth up to 48, choose between rgb24 and rgb48 for temporary storage to save RAM
        if(COLOR_DEPTH_BITS <= 8)
            loadMatrixBuffers48(currentFrameDataPtr, currentRow, lsbMsbTransitionBit, numBrightnessShifts, rgb24(0,0,0));
        else
            loadMatrixBuffers48(currentFrameDataPtr, currentRow, lsbMsbTransitionBit, numBrightnessShifts, rgb48(0,0,0));
    }
}
