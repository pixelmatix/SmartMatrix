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

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
uint8_t SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::calc_refreshRate = 120;
template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
SM_Layer * SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::baseLayer;

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
int SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::multiRowRefresh_mapIndex_CurrentRowGroups = 0;

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
int SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::multiRowRefresh_mapIndex_CurrentPixelGroup = -1;

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
int SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::multiRowRefresh_PixelOffsetFromPanelsAlreadyMapped = 0;

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
int SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::multiRowRefresh_NumPanelsAlreadyMapped = 0;

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::SmartMatrix3(uint8_t bufferrows, rowDataStruct * rowDataBuffer) {
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
void SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::dmaBufferUnderrunCallback(void) {
    dmaBufferUnderrun = true;
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
void SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::matrixCalculations(bool initial) {
    static unsigned char currentRow = 0;
    unsigned char numLoopsWithoutExit = 0;

    // only run the loop if there is free space, and fill the entire buffer before returning
    while (SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::isRowBufferFree()) {
        // check to see if the refresh rate is too high, and the application doesn't have time to run
        if(++numLoopsWithoutExit > MAX_MATRIXCALCULATIONS_LOOPS_WITHOUT_EXIT) {

            // minimum set to avoid overflowing timer at low refresh rates
            if(!initial && calc_refreshRate > MIN_REFRESH_RATE) {
                calc_refreshRate--;
                SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::setRefreshRate(calc_refreshRate);
                refreshRateLowered = true;
                refreshRateChanged = true;
            }

            initial = false;
            numLoopsWithoutExit = 0;
        }

        // do once-per-frame updates
        if (!currentRow) {
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
        }

        // do once-per-line updates
        // none right now

        // enqueue row
        SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::loadMatrixBuffers(currentRow);
        SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::writeRowBuffer(currentRow);

        if (++currentRow >= MATRIX_SCAN_MOD)
            currentRow = 0;

        if(dmaBufferUnderrun) {
            // if refreshrate is too high, lower - minimum set to avoid overflowing timer at low refresh rates
            if(calc_refreshRate > MIN_REFRESH_RATE) {
                calc_refreshRate--;
                SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::setRefreshRate(calc_refreshRate);
                refreshRateLowered = true;
                refreshRateChanged = true;
            }

            SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::recoverFromDmaUnderrun();
            dmaBufferUnderrunSinceLastCheck = true;
            dmaBufferUnderrun = false;
        }
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
int SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::brightness;

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
void SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::setBrightness(uint8_t newBrightness) {
    brightness = newBrightness;
    brightnessChange = true;
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
void SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::setRefreshRate(uint8_t newRefreshRate) {
    if(newRefreshRate > MIN_REFRESH_RATE)
        calc_refreshRate = newRefreshRate;
    else
        calc_refreshRate = MIN_REFRESH_RATE;
    refreshRateChanged = true;
    SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::setRefreshRate(calc_refreshRate);
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
uint8_t SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::getRefreshRate(void) {
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
void SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::begin(void)
{
    SM_Layer * templayer = baseLayer;
    while(templayer) {
        templayer->begin();
        templayer = templayer->nextLayer;
    }

    SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::setMatrixCalculationsCallback(matrixCalculations);
    SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::setMatrixUnderrunCallback(dmaBufferUnderrunCallback);
    SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::begin();
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

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
INLINE void SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::loadMatrixBuffers48(rowDataStruct * currentRowDataPtr, unsigned char currentRow) {
    int i;
    int multiRowRefreshRowOffset = 0;
    const int numPixelsPerTempRow = PIXELS_PER_LATCH/PHYSICAL_ROWS_PER_REFRESH_ROW;

    // static to avoid putting large buffer on the stack
    static rgb48 tempRow0[numPixelsPerTempRow];
    static rgb48 tempRow1[numPixelsPerTempRow];

    int c = 0;
    resetMultiRowRefreshMapPosition();

    // go through this process for each physical row that is contained in the refresh row
    do {
        // clear buffer to prevent garbage data showing through transparent layers
        memset(tempRow0, 0x00, sizeof(tempRow0));
        memset(tempRow1, 0x00, sizeof(tempRow1));

        // get pixel data from layers
        SM_Layer * templayer = SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::baseLayer;
        while(templayer) {
            for(i=0; i<MATRIX_STACK_HEIGHT; i++) {
                // Z-shape, bottom to top
                if(!(optionFlags & SMARTMATRIX_OPTIONS_C_SHAPE_STACKING) &&
                    (optionFlags & SMARTMATRIX_OPTIONS_BOTTOM_TO_TOP_STACKING)) {
                    // fill data from bottom to top, so bottom panel is the one closest to Teensy
                    templayer->fillRefreshRow((currentRow + multiRowRefreshRowOffset) + (MATRIX_STACK_HEIGHT-i-1)*MATRIX_PANEL_HEIGHT, &tempRow0[i*matrixWidth]);
                    templayer->fillRefreshRow((currentRow + multiRowRefreshRowOffset) + ROW_PAIR_OFFSET + (MATRIX_STACK_HEIGHT-i-1)*MATRIX_PANEL_HEIGHT, &tempRow1[i*matrixWidth]);
                // Z-shape, top to bottom
                } else if(!(optionFlags & SMARTMATRIX_OPTIONS_C_SHAPE_STACKING) &&
                    !(optionFlags & SMARTMATRIX_OPTIONS_BOTTOM_TO_TOP_STACKING)) {
                    // fill data from top to bottom, so top panel is the one closest to Teensy
                    templayer->fillRefreshRow((currentRow + multiRowRefreshRowOffset) + i*MATRIX_PANEL_HEIGHT, &tempRow0[i*matrixWidth]);
                    templayer->fillRefreshRow((currentRow + multiRowRefreshRowOffset) + ROW_PAIR_OFFSET + i*MATRIX_PANEL_HEIGHT, &tempRow1[i*matrixWidth]);
                // C-shape, bottom to top
                } else if((optionFlags & SMARTMATRIX_OPTIONS_C_SHAPE_STACKING) &&
                    (optionFlags & SMARTMATRIX_OPTIONS_BOTTOM_TO_TOP_STACKING)) {
                    // alternate direction of filling (or loading) for each matrixwidth
                    // swap row order from top to bottom for each stack (tempRow1 filled with top half of panel, tempRow0 filled with bottom half)
                    if((MATRIX_STACK_HEIGHT-i+1)%2) {
                        templayer->fillRefreshRow((MATRIX_SCAN_MOD-(currentRow + multiRowRefreshRowOffset)-1) + ROW_PAIR_OFFSET + (i)*MATRIX_PANEL_HEIGHT, &tempRow0[i*matrixWidth]);
                        templayer->fillRefreshRow((MATRIX_SCAN_MOD-(currentRow + multiRowRefreshRowOffset)-1) + (i)*MATRIX_PANEL_HEIGHT, &tempRow1[i*matrixWidth]);
                    } else {
                        templayer->fillRefreshRow((currentRow + multiRowRefreshRowOffset) + (i)*MATRIX_PANEL_HEIGHT, &tempRow0[i*matrixWidth]);
                        templayer->fillRefreshRow((currentRow + multiRowRefreshRowOffset) + ROW_PAIR_OFFSET + (i)*MATRIX_PANEL_HEIGHT, &tempRow1[i*matrixWidth]);
                    }
                // C-shape, top to bottom
                } else if((optionFlags & SMARTMATRIX_OPTIONS_C_SHAPE_STACKING) && 
                    !(optionFlags & SMARTMATRIX_OPTIONS_BOTTOM_TO_TOP_STACKING)) {
                    if((MATRIX_STACK_HEIGHT-i)%2) {
                        templayer->fillRefreshRow((currentRow + multiRowRefreshRowOffset) + (MATRIX_STACK_HEIGHT-i-1)*MATRIX_PANEL_HEIGHT, &tempRow0[i*matrixWidth]);
                        templayer->fillRefreshRow((currentRow + multiRowRefreshRowOffset) + ROW_PAIR_OFFSET + (MATRIX_STACK_HEIGHT-i-1)*MATRIX_PANEL_HEIGHT, &tempRow1[i*matrixWidth]);
                    } else {
                        templayer->fillRefreshRow((MATRIX_SCAN_MOD-(currentRow + multiRowRefreshRowOffset)-1) + ROW_PAIR_OFFSET + (MATRIX_STACK_HEIGHT-i-1)*MATRIX_PANEL_HEIGHT, &tempRow0[i*matrixWidth]);
                        templayer->fillRefreshRow((MATRIX_SCAN_MOD-(currentRow + multiRowRefreshRowOffset)-1) + (MATRIX_STACK_HEIGHT-i-1)*MATRIX_PANEL_HEIGHT, &tempRow1[i*matrixWidth]);
                    }
                }
            }
            templayer = templayer->nextLayer;        
        }

        union {
            uint8_t word;
            struct {
                // order of bits in word matches how GPIO connects to the display
                uint8_t GPIO_WORD_ORDER_8BIT;
            };
        } o0;
        
        for(int j=0; j<COLOR_DEPTH_BITS; j++) {
            int maskoffset = 0;
            if(COLOR_DEPTH_BITS == 12) // 36-bit color
                maskoffset = 4;
            else if (COLOR_DEPTH_BITS == 16) // 48-bit color
                maskoffset = 0;

            uint16_t mask = (1 << (j + maskoffset));
            
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
                    o0.word = 0x00;

                    int refreshBufferPosition;
                    if(reversePixelBlock) {
                        refreshBufferPosition = currentMapOffset-k;
                    } else {
                        refreshBufferPosition = currentMapOffset+k;
                    }

                    if (tempRow0[i+k].red & mask)
                        o0.p0r1 = 1;
                    if (tempRow0[i+k].green & mask)
                        o0.p0g1 = 1;
                    if (tempRow0[i+k].blue & mask)
                        o0.p0b1 = 1;
                    if (tempRow1[i+k].red & mask)
                        o0.p0r2 = 1;
                    if (tempRow1[i+k].green & mask)
                        o0.p0g2 = 1;
                    if (tempRow1[i+k].blue & mask)
                        o0.p0b2 = 1;

                    if(optionFlags & SMARTMATRIX_OPTIONS_HUB12_MODE) {
                        // HUB12 format inverts the data (assume we're only using R1 for now)
                        if (tempRow0[i+k].red & mask)
                            o0.p0r1 = 0;
                        else
                            o0.p0r1 = 1;                        
                    }               

                    if((optionFlags & SMARTMATRIX_OPTIONS_C_SHAPE_STACKING) && !((i/matrixWidth)%2)) {
                        // TODO: fix C_Shape_stacking after applying multi-row-refresh support
                        //currentRowDataPtr->rowbits[j].data[(((i+matrixWidth-1)-k)*DMA_UPDATES_PER_CLOCK)] = o0.word;
                        //o0.p0clk = 1;
                        //currentRowDataPtr->rowbits[j].data[(((i+matrixWidth-1)-k)*DMA_UPDATES_PER_CLOCK)+1] = o0.word;
                    } else {
                        currentRowDataPtr->rowbits[j].data[((refreshBufferPosition)*DMA_UPDATES_PER_CLOCK)] = o0.word;
                        o0.p0clk = 1;
                        currentRowDataPtr->rowbits[j].data[((refreshBufferPosition)*DMA_UPDATES_PER_CLOCK)+1] = o0.word;
                    }
                }
                i += numPixelsToMap; // keep track of current position on this temp buffer
                advanceMultiRowRefreshMapToNextPixelGroup();
            }
            currentRowDataPtr->rowbits[j].rowAddress = currentRow;
        }

#ifdef ADDX_UPDATE_ON_DATA_PINS
        o0.word = 0x00000000;
        o0.p0r1 = (currentRow & 0x01) ? 1 : 0;
        o0.p0g1 = (currentRow & 0x02) ? 1 : 0;
        o0.p0b1 = (currentRow & 0x04) ? 1 : 0;
        o0.p0r2 = (currentRow & 0x08) ? 1 : 0;
        o0.p0g2 = (currentRow & 0x10) ? 1 : 0;

        for(int j=0; j<COLOR_DEPTH_BITS; j++) {
            currentRowDataPtr->rowbits[j].rowAddress = o0.word;
        }
#endif

        c += numPixelsPerTempRow; // keep track of cumulative number of pixels filled in refresh buffer before this temp buffer

        advanceMultiRowRefreshMapToNextRow();
        multiRowRefreshRowOffset = getMultiRowRefreshRowOffset();
    } while (multiRowRefreshRowOffset > 0);
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
INLINE void SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::loadMatrixBuffers24(rowDataStruct * currentRowDataPtr, unsigned char currentRow) {
    int i;
    int multiRowRefreshRowOffset = 0;
    const int numPixelsPerTempRow = PIXELS_PER_LATCH/PHYSICAL_ROWS_PER_REFRESH_ROW;

    // static to avoid putting large buffer on the stack
    static rgb24 tempRow0[numPixelsPerTempRow];
    static rgb24 tempRow1[numPixelsPerTempRow];

    int c = 0;
    resetMultiRowRefreshMapPosition();

    // go through this process for each physical row that is contained in the refresh row
    do {
        // clear buffer to prevent garbage data showing through transparent layers
        memset(tempRow0, 0x00, sizeof(tempRow0));
        memset(tempRow1, 0x00, sizeof(tempRow1));

        // get pixel data from layers
        SM_Layer * templayer = SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::baseLayer;
        while(templayer) {
            for(i=0; i<MATRIX_STACK_HEIGHT; i++) {
                // Z-shape, bottom to top
                if(!(optionFlags & SMARTMATRIX_OPTIONS_C_SHAPE_STACKING) &&
                    (optionFlags & SMARTMATRIX_OPTIONS_BOTTOM_TO_TOP_STACKING)) {
                    // fill data from bottom to top, so bottom panel is the one closest to Teensy
                    templayer->fillRefreshRow((currentRow + multiRowRefreshRowOffset) + (MATRIX_STACK_HEIGHT-i-1)*MATRIX_PANEL_HEIGHT, &tempRow0[i*matrixWidth]);
                    templayer->fillRefreshRow((currentRow + multiRowRefreshRowOffset) + ROW_PAIR_OFFSET + (MATRIX_STACK_HEIGHT-i-1)*MATRIX_PANEL_HEIGHT, &tempRow1[i*matrixWidth]);
                // Z-shape, top to bottom
                } else if(!(optionFlags & SMARTMATRIX_OPTIONS_C_SHAPE_STACKING) &&
                    !(optionFlags & SMARTMATRIX_OPTIONS_BOTTOM_TO_TOP_STACKING)) {
                    // fill data from top to bottom, so top panel is the one closest to Teensy
                    templayer->fillRefreshRow((currentRow + multiRowRefreshRowOffset) + i*MATRIX_PANEL_HEIGHT, &tempRow0[i*matrixWidth]);
                    templayer->fillRefreshRow((currentRow + multiRowRefreshRowOffset) + ROW_PAIR_OFFSET + i*MATRIX_PANEL_HEIGHT, &tempRow1[i*matrixWidth]);
                // C-shape, bottom to top
                } else if((optionFlags & SMARTMATRIX_OPTIONS_C_SHAPE_STACKING) &&
                    (optionFlags & SMARTMATRIX_OPTIONS_BOTTOM_TO_TOP_STACKING)) {
                    // alternate direction of filling (or loading) for each matrixwidth
                    // swap row order from top to bottom for each stack (tempRow1 filled with top half of panel, tempRow0 filled with bottom half)
                    if((MATRIX_STACK_HEIGHT-i+1)%2) {
                        templayer->fillRefreshRow((MATRIX_SCAN_MOD-(currentRow + multiRowRefreshRowOffset)-1) + ROW_PAIR_OFFSET + (i)*MATRIX_PANEL_HEIGHT, &tempRow0[i*matrixWidth]);
                        templayer->fillRefreshRow((MATRIX_SCAN_MOD-(currentRow + multiRowRefreshRowOffset)-1) + (i)*MATRIX_PANEL_HEIGHT, &tempRow1[i*matrixWidth]);
                    } else {
                        templayer->fillRefreshRow((currentRow + multiRowRefreshRowOffset) + (i)*MATRIX_PANEL_HEIGHT, &tempRow0[i*matrixWidth]);
                        templayer->fillRefreshRow((currentRow + multiRowRefreshRowOffset) + ROW_PAIR_OFFSET + (i)*MATRIX_PANEL_HEIGHT, &tempRow1[i*matrixWidth]);
                    }
                // C-shape, top to bottom
                } else if((optionFlags & SMARTMATRIX_OPTIONS_C_SHAPE_STACKING) && 
                    !(optionFlags & SMARTMATRIX_OPTIONS_BOTTOM_TO_TOP_STACKING)) {
                    if((MATRIX_STACK_HEIGHT-i)%2) {
                        templayer->fillRefreshRow((currentRow + multiRowRefreshRowOffset) + (MATRIX_STACK_HEIGHT-i-1)*MATRIX_PANEL_HEIGHT, &tempRow0[i*matrixWidth]);
                        templayer->fillRefreshRow((currentRow + multiRowRefreshRowOffset) + ROW_PAIR_OFFSET + (MATRIX_STACK_HEIGHT-i-1)*MATRIX_PANEL_HEIGHT, &tempRow1[i*matrixWidth]);
                    } else {
                        templayer->fillRefreshRow((MATRIX_SCAN_MOD-(currentRow + multiRowRefreshRowOffset)-1) + ROW_PAIR_OFFSET + (MATRIX_STACK_HEIGHT-i-1)*MATRIX_PANEL_HEIGHT, &tempRow0[i*matrixWidth]);
                        templayer->fillRefreshRow((MATRIX_SCAN_MOD-(currentRow + multiRowRefreshRowOffset)-1) + (MATRIX_STACK_HEIGHT-i-1)*MATRIX_PANEL_HEIGHT, &tempRow1[i*matrixWidth]);
                    }
                }
            }
            templayer = templayer->nextLayer;        
        }

        union {
            uint8_t word;
            struct {
                // order of bits in word matches how GPIO connects to the display
                uint8_t GPIO_WORD_ORDER_8BIT;
            };
        } o0;
        
        for(int j=0; j<COLOR_DEPTH_BITS; j++) {
            int maskoffset = 0;
            if(COLOR_DEPTH_BITS == 12)   // 36-bit color
                maskoffset = 4;
            else if (COLOR_DEPTH_BITS == 16) // 48-bit color
                maskoffset = 0;
            else if (COLOR_DEPTH_BITS == 8)  // 24-bit color
                maskoffset = 0;

            uint16_t mask = (1 << (j + maskoffset));
            
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
                    o0.word = 0x00;

                    int refreshBufferPosition;
                    if(reversePixelBlock) {
                        refreshBufferPosition = currentMapOffset-k;
                    } else {
                        refreshBufferPosition = currentMapOffset+k;
                    }

                    if (tempRow0[i+k].red & mask)
                        o0.p0r1 = 1;
                    if (tempRow0[i+k].green & mask)
                        o0.p0g1 = 1;
                    if (tempRow0[i+k].blue & mask)
                        o0.p0b1 = 1;
                    if (tempRow1[i+k].red & mask)
                        o0.p0r2 = 1;
                    if (tempRow1[i+k].green & mask)
                        o0.p0g2 = 1;
                    if (tempRow1[i+k].blue & mask)
                        o0.p0b2 = 1;

                    if(optionFlags & SMARTMATRIX_OPTIONS_HUB12_MODE) {
                        // HUB12 format inverts the data (assume we're only using R1 for now)
                        if (tempRow0[i+k].red & mask)
                            o0.p0r1 = 0;
                        else
                            o0.p0r1 = 1;                        
                    }               

                    if((optionFlags & SMARTMATRIX_OPTIONS_C_SHAPE_STACKING) && !((i/matrixWidth)%2)) {
                        // TODO: fix C_Shape_stacking after applying multi-row-refresh support
                        //currentRowDataPtr->rowbits[j].data[(((i+matrixWidth-1)-k)*DMA_UPDATES_PER_CLOCK)] = o0.word;
                        //o0.p0clk = 1;
                        //currentRowDataPtr->rowbits[j].data[(((i+matrixWidth-1)-k)*DMA_UPDATES_PER_CLOCK)+1] = o0.word;
                    } else {
                        currentRowDataPtr->rowbits[j].data[((refreshBufferPosition)*DMA_UPDATES_PER_CLOCK)] = o0.word;
                        o0.p0clk = 1;
                        currentRowDataPtr->rowbits[j].data[((refreshBufferPosition)*DMA_UPDATES_PER_CLOCK)+1] = o0.word;
                    }
                }
                i += numPixelsToMap; // keep track of current position on this temp buffer
                advanceMultiRowRefreshMapToNextPixelGroup();
            }
            currentRowDataPtr->rowbits[j].rowAddress = currentRow;
        }

#ifdef ADDX_UPDATE_ON_DATA_PINS
        o0.word = 0x00000000;
        o0.p0r1 = (currentRow & 0x01) ? 1 : 0;
        o0.p0g1 = (currentRow & 0x02) ? 1 : 0;
        o0.p0b1 = (currentRow & 0x04) ? 1 : 0;
        o0.p0r2 = (currentRow & 0x08) ? 1 : 0;
        o0.p0g2 = (currentRow & 0x10) ? 1 : 0;

        for(int j=0; j<LATCHES_PER_ROW; j++) {
            currentRowDataPtr->rowbits[j].rowAddress = o0.word;
        }
#endif

        c += numPixelsPerTempRow; // keep track of cumulative number of pixels filled in refresh buffer before this temp buffer

        advanceMultiRowRefreshMapToNextRow();
        multiRowRefreshRowOffset = getMultiRowRefreshRowOffset();
    } while (multiRowRefreshRowOffset > 0);    
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
INLINE void SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::loadMatrixBuffers(unsigned char currentRow) {
    rowDataStruct * currentRowDataPtr = SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::getNextRowBufferPtr();

    // TODO: support rgb36/48 with same function, copy function to rgb24
    if(LATCHES_PER_ROW == 16)
        loadMatrixBuffers48(currentRowDataPtr, currentRow);
    else if(LATCHES_PER_ROW == 12)
        loadMatrixBuffers48(currentRowDataPtr, currentRow);
    else if(LATCHES_PER_ROW == 8)
        loadMatrixBuffers24(currentRowDataPtr, currentRow);
}
