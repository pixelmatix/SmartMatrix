/*
 * SmartMatrix Library - Teensy 3 HUB75 Panel Calculation Class
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

#define INLINE __attribute__( ( always_inline ) ) inline

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
uint8_t SmartMatrixHub75Calc<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::calc_refreshRate = 120;
template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
SM_Layer * SmartMatrixHub75Calc<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::baseLayer;

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
volatile bool SmartMatrixHub75Calc<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::dmaBufferUnderrun = false;

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
bool SmartMatrixHub75Calc<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::dmaBufferUnderrunSinceLastCheck = false;

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
bool SmartMatrixHub75Calc<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::refreshRateLowered = false;

// set to true initially so all layers get the initial refresh rate
template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
bool SmartMatrixHub75Calc<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::refreshRateChanged = true;

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
int SmartMatrixHub75Calc<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::multiRowRefresh_mapIndex_CurrentRowGroups = 0;

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
int SmartMatrixHub75Calc<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::multiRowRefresh_mapIndex_CurrentPixelGroup = -1;

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
int SmartMatrixHub75Calc<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::multiRowRefresh_PixelOffsetFromPanelsAlreadyMapped = 0;

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
int SmartMatrixHub75Calc<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::multiRowRefresh_NumPanelsAlreadyMapped = 0;

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
SmartMatrixHub75Calc<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::SmartMatrixHub75Calc(uint8_t bufferrows, rowDataStruct * rowDataBuffer) {
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
void SmartMatrixHub75Calc<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::addLayer(SM_Layer * newlayer) {
    if(baseLayer) {
        SM_Layer * templayer = baseLayer;
        while(templayer->nextLayer)
            templayer = templayer->nextLayer;
        templayer->nextLayer = newlayer;
    } else {
        baseLayer = newlayer;
    }
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
void SmartMatrixHub75Calc<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::countFPS(void) {
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

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
void SmartMatrixHub75Calc<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::dmaBufferUnderrunCallback(void) {
    dmaBufferUnderrun = true;
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
void SmartMatrixHub75Calc<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::matrixCalculations(bool initial) {
    static unsigned char currentRow = 0;
    unsigned char numLoopsWithoutExit = 0;

    // only run the loop if there is free space, and fill the entire buffer before returning
    while (SmartMatrixHub75Refresh<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::isRowBufferFree()) {
        // check to see if the refresh rate is too high, and the application doesn't have time to run
        if(++numLoopsWithoutExit > MAX_MATRIXCALCULATIONS_LOOPS_WITHOUT_EXIT) {

            // minimum set to avoid overflowing timer at low refresh rates
            if(!initial && calc_refreshRate > MIN_REFRESH_RATE) {
                calc_refreshRate--;
                SmartMatrixHub75Refresh<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::setRefreshRate(calc_refreshRate);
                refreshRateLowered = true;
                refreshRateChanged = true;
            }

            initial = false;
            numLoopsWithoutExit = 0;
        }

        // do once-per-frame updates
        if (!currentRow) {
            if (rotationChange) {
                SM_Layer * templayer = SmartMatrixHub75Calc<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::baseLayer;
                while(templayer) {
                    templayer->setRotation(rotation);
                    templayer = templayer->nextLayer;
                }
                rotationChange = false;
            }

            SM_Layer * templayer = SmartMatrixHub75Calc<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::baseLayer;
            while(templayer) {
                if(refreshRateChanged) {
                    templayer->setRefreshRate(calc_refreshRate);
                }
                templayer->frameRefreshCallback();
                templayer = templayer->nextLayer;
            }
            refreshRateChanged = false;
            if (brightnessChange) {
                SmartMatrixHub75Refresh<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::setBrightness(brightness);
                brightnessChange = false;
            }
        }

        // do once-per-line updates
        // none right now

        // enqueue row
        SmartMatrixHub75Calc<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::loadMatrixBuffers(currentRow);
        SmartMatrixHub75Refresh<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::writeRowBuffer(currentRow);

        if (++currentRow >= MATRIX_SCAN_MOD)
            currentRow = 0;

        if(dmaBufferUnderrun) {
            // if refreshrate is too high, lower - minimum set to avoid overflowing timer at low refresh rates
            if(calc_refreshRate > MIN_REFRESH_RATE) {
                calc_refreshRate--;
                SmartMatrixHub75Refresh<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::setRefreshRate(calc_refreshRate);
                refreshRateLowered = true;
                refreshRateChanged = true;
            }

            SmartMatrixHub75Refresh<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::recoverFromDmaUnderrun();
            dmaBufferUnderrunSinceLastCheck = true;
            dmaBufferUnderrun = false;
        }
    }
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
void SmartMatrixHub75Calc<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::setRotation(rotationDegrees newrotation) {
    rotation = newrotation;
    rotationChange = true;
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
uint16_t SmartMatrixHub75Calc<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::getScreenWidth(void) const {
    if (rotation == rotation0 || rotation == rotation180) {
        return matrixWidth;
    } else {
        return matrixHeight;
    }
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
uint16_t SmartMatrixHub75Calc<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::getScreenHeight(void) const {
    if (rotation == rotation0 || rotation == rotation180) {
        return matrixHeight;
    } else {
        return matrixWidth;
    }
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
volatile bool SmartMatrixHub75Calc<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::brightnessChange = false;
template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
volatile bool SmartMatrixHub75Calc<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::rotationChange = true;
template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
rotationDegrees SmartMatrixHub75Calc<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::rotation = rotation0;

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
int SmartMatrixHub75Calc<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::brightness;

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
void SmartMatrixHub75Calc<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::setBrightness(uint8_t newBrightness) {
    brightness = newBrightness;
    brightnessChange = true;
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
void SmartMatrixHub75Calc<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::setRefreshRate(uint8_t newRefreshRate) {
    if(newRefreshRate > MIN_REFRESH_RATE)
        calc_refreshRate = newRefreshRate;
    else
        calc_refreshRate = MIN_REFRESH_RATE;
    refreshRateChanged = true;
    SmartMatrixHub75Refresh<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::setRefreshRate(calc_refreshRate);
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
uint8_t SmartMatrixHub75Calc<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::getRefreshRate(void) {
    return calc_refreshRate;
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
bool SmartMatrixHub75Calc<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::getdmaBufferUnderrunFlag(void) {
    if(dmaBufferUnderrunSinceLastCheck) {
        dmaBufferUnderrunSinceLastCheck = false;
        return true;
    }
    return false;
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
bool SmartMatrixHub75Calc<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::getRefreshRateLoweredFlag(void) {
    if(refreshRateLowered) {
        refreshRateLowered = false;
        return true;
    }
    return false;
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
void SmartMatrixHub75Calc<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::begin(void)
{
    SM_Layer * templayer = baseLayer;
    while(templayer) {
        templayer->begin();
        templayer = templayer->nextLayer;
    }

    SmartMatrixHub75Refresh<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::setMatrixCalculationsCallback(matrixCalculations);
    SmartMatrixHub75Refresh<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::setMatrixUnderrunCallback(dmaBufferUnderrunCallback);
    SmartMatrixHub75Refresh<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::begin();
}

#define IS_LAST_PANEL_MAP_ENTRY(x) (!x.rowOffset && !x.bufferOffset && !x.numPixels)

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
void SmartMatrixHub75Calc<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::resetMultiRowRefreshMapPosition(void) {   
    multiRowRefresh_mapIndex_CurrentRowGroups = 0;
    resetMultiRowRefreshMapPositionPixelGroupToStartOfRow();
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
void SmartMatrixHub75Calc<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::resetMultiRowRefreshMapPositionPixelGroupToStartOfRow(void) {   
    multiRowRefresh_mapIndex_CurrentPixelGroup = multiRowRefresh_mapIndex_CurrentRowGroups;
    multiRowRefresh_PixelOffsetFromPanelsAlreadyMapped = 0;
    multiRowRefresh_NumPanelsAlreadyMapped = 0;
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
void SmartMatrixHub75Calc<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::advanceMultiRowRefreshMapToNextRow(void) {   
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

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
void SmartMatrixHub75Calc<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::advanceMultiRowRefreshMapToNextPixelGroup(void) {   
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
template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
int SmartMatrixHub75Calc<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::getMultiRowRefreshRowOffset(void) {   
    static const PanelMappingEntry * map = getMultiRowRefreshPanelMap(panelType);

    if(IS_LAST_PANEL_MAP_ENTRY(map[multiRowRefresh_mapIndex_CurrentRowGroups])){
        return -1;
    }

    return map[multiRowRefresh_mapIndex_CurrentRowGroups].rowOffset;    
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
int SmartMatrixHub75Calc<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::getMultiRowRefreshNumPixelsToMap(void) {        
    static const PanelMappingEntry * map = getMultiRowRefreshPanelMap(panelType);

    return map[multiRowRefresh_mapIndex_CurrentPixelGroup].numPixels;    
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
int SmartMatrixHub75Calc<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::getMultiRowRefreshPixelGroupOffset(void) {        
    static const PanelMappingEntry * map = getMultiRowRefreshPanelMap(panelType);

    return map[multiRowRefresh_mapIndex_CurrentPixelGroup].bufferOffset + multiRowRefresh_PixelOffsetFromPanelsAlreadyMapped;
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags> template <typename RGB_TEMP>
INLINE void SmartMatrixHub75Calc<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::loadMatrixBuffers48(rowDataStruct * currentRowDataPtr, unsigned char currentRow, RGB_TEMP tempBufferType) {
    int i;
    int multiRowRefreshRowOffset = 0;
    const int numPixelsPerTempRow = PIXELS_PER_LATCH/PHYSICAL_ROWS_PER_REFRESH_ROW;

    // static to avoid putting large buffer on the stack
    static RGB_TEMP tempRow0[numPixelsPerTempRow];
    static RGB_TEMP tempRow1[numPixelsPerTempRow];

    int c = 0;

    // multi row refresh isn't very efficient, slowing this function down by ~30% for panels that don't even need multi row refresh.  For now, only enable the code if needed
    if(MULTI_ROW_REFRESH_REQUIRED) { 
        resetMultiRowRefreshMapPosition();
    }

    // go through this process for each physical row that is contained in the refresh row
    do {
        // clear buffer to prevent garbage data showing through transparent layers
        memset(tempRow0, 0x00, sizeof(tempRow0));
        memset(tempRow1, 0x00, sizeof(tempRow1));

        // get pixel data from layers
        SM_Layer * templayer = SmartMatrixHub75Calc<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::baseLayer;
        int y0, y1; // positions of the two rows we need
        while (templayer) {
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
                templayer->fillRefreshRow(y0, &tempRow0[i * matrixWidth]);
                templayer->fillRefreshRow(y1, &tempRow1[i * matrixWidth]);
            }
            templayer = templayer->nextLayer;        
        }

        union {
            uint8_t word;
            struct {
                // order of bits in word matches how GPIO connects to the display
                uint8_t GPIO_WORD_ORDER_8BIT;
            };
            struct {
                uint8_t GPIO_WORD_ORDER_ADDX_8BIT;                
            };
        } o0;
        
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

                reversePixelBlock = false;
                if(numPixelsToMap < 0) {
                    reversePixelBlock = true;
                    numPixelsToMap = abs(numPixelsToMap);
                }

                // get offset where pixels are written in the refresh buffer
                currentMapOffset = getMultiRowRefreshPixelGroupOffset();
            } else {
                numPixelsToMap = matrixWidth;
            }

            // parse through grouping of pixels, loading from temp buffer and writing to refresh buffer
            for(int k=0; k < numPixelsToMap; k++) {
                uint16_t r0, g0, b0, r1, g1, b1;
                int ind;

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
                    o0.word = 0x00;

                    if (r0 & mask)
                        o0.hub75_r0 = 1;
                    if (g0 & mask)
                        o0.hub75_g0 = 1;
                    if (b0 & mask)
                        o0.hub75_b0 = 1;
                    if (r1 & mask)
                        o0.hub75_r1 = 1;
                    if (g1 & mask)
                        o0.hub75_g1 = 1;
                    if (b1 & mask)
                        o0.hub75_b1 = 1;

                    if(optionFlags & SMARTMATRIX_OPTIONS_HUB12_MODE) {
                        // HUB12 format inverts the data (assume we're only using R1 for now)
                        if (r0 & mask)
                            o0.hub75_r0 = 0;
                        else
                            o0.hub75_r0 = 1;                        
                    }               

                    mask <<= 1;

                    // store these pixel bits in the rowDataBuffer, leaving the initial pixels as padding
                    currentRowDataPtr->rowbits[bitindex].data[((refreshBufferPosition)*DMA_UPDATES_PER_CLOCK)] = o0.word;
                    o0.hub75_clk = 1;
                    currentRowDataPtr->rowbits[bitindex].data[((refreshBufferPosition)*DMA_UPDATES_PER_CLOCK)+1] = o0.word;
                }
            }
            i += numPixelsToMap; // keep track of current position on this temp buffer
            if(MULTI_ROW_REFRESH_REQUIRED) { 
                advanceMultiRowRefreshMapToNextPixelGroup();
            }
        }

        for (int bitindex = 0; bitindex < COLOR_DEPTH_BITS; bitindex++)
            currentRowDataPtr->rowbits[bitindex].rowAddress = currentRow;

#ifdef ADDX_UPDATE_ON_DATA_PINS
        o0.word = 0x00000000;
        o0.hub75_addx0 = (currentRow & 0x01) ? 1 : 0;
        o0.hub75_addx1 = (currentRow & 0x02) ? 1 : 0;
        o0.hub75_addx2 = (currentRow & 0x04) ? 1 : 0;
        o0.hub75_addx3 = (currentRow & 0x08) ? 1 : 0;
        o0.hub75_addx4 = (currentRow & 0x10) ? 1 : 0;

        for(int bitindex=0; bitindex<COLOR_DEPTH_BITS; bitindex++) {
            currentRowDataPtr->rowbits[bitindex].rowAddress = o0.word;
        }
#endif

        if(MULTI_ROW_REFRESH_REQUIRED) { 
            c += numPixelsPerTempRow; // keep track of cumulative number of pixels filled in refresh buffer before this temp buffer

            advanceMultiRowRefreshMapToNextRow();
            multiRowRefreshRowOffset = getMultiRowRefreshRowOffset();
        }
    } while (MULTI_ROW_REFRESH_REQUIRED ? (multiRowRefreshRowOffset > 0) : 0);
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
INLINE void SmartMatrixHub75Calc<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::loadMatrixBuffers(unsigned char currentRow) {
    rowDataStruct * currentRowDataPtr = SmartMatrixHub75Refresh<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::getNextRowBufferPtr();

    // same function supports any refresh depth up to 48, choose between rgb24 and rgb48 for temporary storage to save RAM
    if(COLOR_DEPTH_BITS <= 8)
        loadMatrixBuffers48(currentRowDataPtr, currentRow, rgb24(0,0,0));
    else
        loadMatrixBuffers48(currentRowDataPtr, currentRow, rgb48(0,0,0));
}
