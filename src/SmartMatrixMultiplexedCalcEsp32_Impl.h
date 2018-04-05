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
uint8_t SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::calc_refreshRate = 60;
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
SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::SmartMatrix3(frameStruct * frameBuffer) {
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
void SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::matrixCalculations() {
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

    SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::loadMatrixBuffers();

    SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::writeFrameBuffer(0);
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
    SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::setMatrixCalculationsCallback(matrixCalculations);
    SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::begin();
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
INLINE void SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::loadMatrixBuffers48(frameStruct * currentFrameDataPtr, int currentRow) {
#if 0
    int i;

    // static to avoid putting large buffer on the stack
    static rgb48 tempRow0[PIXELS_PER_LATCH];
    static rgb48 tempRow1[PIXELS_PER_LATCH];

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

        while(i < PIXELS_PER_LATCH) {
            // parse through matrixWith block of pixels, from left to right, or right to left, depending on C_SHAPE_STACKING options
            for(int k=0; k < matrixWidth; k++) {
                o0.word = 0x00;

                //fill temp0red, etc, or work directly from buffer?
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

                if((optionFlags & SMARTMATRIX_OPTIONS_C_SHAPE_STACKING) && !((i/matrixWidth)%2)) {
                    currentRowDataPtr->rowbits[j].data[(((i+matrixWidth-1)-k)*DMA_UPDATES_PER_CLOCK)] = o0.word;
                    o0.p0clk = 1;
                    currentRowDataPtr->rowbits[j].data[(((i+matrixWidth-1)-k)*DMA_UPDATES_PER_CLOCK)+1] = o0.word;
                } else {
                    currentRowDataPtr->rowbits[j].data[((i+k)*DMA_UPDATES_PER_CLOCK)] = o0.word;
                    o0.p0clk = 1;
                    currentRowDataPtr->rowbits[j].data[((i+k)*DMA_UPDATES_PER_CLOCK)+1] = o0.word;
                }
            }
            i += matrixWidth;
        }
        currentRowDataPtr->rowbits[j].rowAddress = currentRow;
    }

#ifdef ADDX_UPDATE_ON_DATA_PINS
        o0.word = 0x00000000;
        o0.p0r1 = (currentRow & 0x01) ? 1 : 0;
        o0.p0g1 = (currentRow & 0x02) ? 1 : 0;
        o0.p0b1 = (currentRow & 0x04) ? 1 : 0;
        o0.p0r2 = (currentRow & 0x08) ? 1 : 0;

        for(int j=0; j<COLOR_DEPTH_BITS; j++) {
            currentRowDataPtr->rowbits[j].rowAddress = o0.word;
        }
#endif
#endif
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
INLINE void SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::loadMatrixBuffers24(frameStruct * frameBuffer, int currentRow) {
#if 1
    int i;

    // static to avoid putting large buffer on the stack
    static rgb24 tempRow0[PIXELS_PER_LATCH];
    static rgb24 tempRow1[PIXELS_PER_LATCH];

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
        
        SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::rowBitStruct *p=&(frameBuffer->rowdata[currentRow].rowbits[j]); //bitplane location to write to
        
        int i=0;
        while(i < PIXELS_PER_LATCH) {

            // parse through matrixWith block of pixels, from left to right, or right to left, depending on C_SHAPE_STACKING options
            for(int k=0; k < matrixWidth; k++) {
                int v=0;

                // TODO: use actual brightness setting
                brightness = 16;

                // stop showing image after a number of CLK cycles that scales with brightness
                if (k>=brightness) v|=BIT_OE;

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
                    //Save the calculated value to the bitplane memory in reverse order to account for I2S Tx FIFO mode1 ordering
                    if(k%2){
                        p->data[k-1] = v;
                    } else {
                        p->data[k+1] = v;
                    }
                }
            }

            // TODO: insert latch data all at once at the end
            for(int k=matrixWidth; k < matrixWidth + CLKS_DURING_LATCH; k++) {
                int v = 0;
                // after data is shifted in, pulse latch for one clock cycle
                if(k == matrixWidth) {
                    v|=BIT_LAT;
                }

                //Do not show image while the line bits are changing
                v|=BIT_OE;

                // set ADDX values to high while latch is high, keep them high while latch drops to clock it in to ADDX latch
                if(k >= matrixWidth) {               
                    if (currentRow&1) v|=BIT_R1;
                    if (currentRow&2) v|=BIT_G1;
                    if (currentRow&4) v|=BIT_B1;
                    if (currentRow&8) v|=BIT_R2;
                    // reserve G2 for currentRow&16
                    // reserve B2 for OE SWITCH
                }

                //Save the calculated value to the bitplane memory in reverse order to account for I2S Tx FIFO mode1 ordering
                if(k%2){
                    p->data[k-1] = v;
                } else {
                    p->data[k+1] = v;
                }
            }

            i += matrixWidth;
        }
    }
#endif
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
INLINE void SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::loadMatrixBuffers() {
#if 1
    unsigned char currentRow;

    frameStruct * currentFrameDataPtr = SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::getNextFrameBufferPtr();

    for(currentRow = 0; currentRow < ROWS_PER_FRAME; currentRow++) {
        // TODO: support rgb36/48 with same function, copy function to rgb24
        if(COLOR_DEPTH_BITS == 16)
            loadMatrixBuffers48(currentFrameDataPtr, currentRow);
        else if(COLOR_DEPTH_BITS == 12)
            loadMatrixBuffers48(currentFrameDataPtr, currentRow);
        else if(COLOR_DEPTH_BITS == 8)
            loadMatrixBuffers24(currentFrameDataPtr, currentRow);
    }
#endif
}
