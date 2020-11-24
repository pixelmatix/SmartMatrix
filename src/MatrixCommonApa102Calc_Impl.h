/*
 * SmartMatrix Library - APA102 Calculation Code for Teensy Platforms
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
uint8_t SmartMatrixApaCalc<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::refreshRate = 60;

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
uint32_t SmartMatrixApaCalc<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::spiClockSpeed = 5000000;

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
SM_Layer * SmartMatrixApaCalc<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::baseLayer;

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
volatile bool SmartMatrixApaCalc<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::dmaBufferUnderrun = false;

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
bool SmartMatrixApaCalc<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::dmaBufferUnderrunSinceLastCheck = false;

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
bool SmartMatrixApaCalc<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::refreshRateLowered = false;

// set to true initially so all layers get the initial refresh rate
template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
bool SmartMatrixApaCalc<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::refreshRateChanged = true;

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
SmartMatrixApaCalc<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::SmartMatrixApaCalc(uint8_t bufferrows, frameDataStruct * frameDataBuffer) {
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
void SmartMatrixApaCalc<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::addLayer(SM_Layer * newlayer) {
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
int SmartMatrixApaCalc<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::countFPS(void) {
    static long loops = 0;
    static long lastMillis = 0;
    long currentMillis = millis();
    int ret = 0;

    loops++;
    if(currentMillis - lastMillis >= 1000){
#if defined(USB_SERIAL)
        if(Serial) {
            Serial.print("Loops last second:");
            Serial.println(loops);
        }
#endif    
        ret = loops;
        lastMillis = currentMillis;
        loops = 0;
    }
    
    return ret;
}

#define MAX_MATRIXCALCULATIONS_LOOPS_WITHOUT_EXIT  5

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
void SmartMatrixApaCalc<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::dmaBufferUnderrunCallback(void) {
    dmaBufferUnderrun = true;
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
void SmartMatrixApaCalc<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::matrixCalculations(bool initial) {
    static unsigned char currentRow;

    // TODO: handle underrun, and too high refresh rate
    // only run the loop if there is free space, and fill the entire buffer before returning
    while (SmartMatrixAPA102Refresh<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::isRowBufferFree()) {
        currentRow = 0;
        frameDataStruct * currentRowDataPtr = SmartMatrixAPA102Refresh<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::getNextRowBufferPtr();

        do {
#ifdef DEBUG_PINS_ENABLED
            digitalWriteFast(DEBUG_PIN_3, HIGH); // oscilloscope trigger
#endif
            // do once-per-frame updates
            if (!currentRow) {
                if (rotationChange) {
                    SM_Layer * templayer = SmartMatrixApaCalc<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::baseLayer;
                    while(templayer) {
                        templayer->setRotation(rotation);
                        templayer = templayer->nextLayer;
                    }
                    rotationChange = false;
                }

                SM_Layer * templayer = SmartMatrixApaCalc<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::baseLayer;
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

            SmartMatrixApaCalc<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::loadMatrixBuffers(currentRowDataPtr, currentRow);

#ifdef DEBUG_PINS_ENABLED
    digitalWriteFast(DEBUG_PIN_3, LOW);
#endif

            // enqueue row
            if (++currentRow >= matrixHeight) {
                currentRow = 0;
                SmartMatrixAPA102Refresh<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::writeRowBuffer(currentRow);
            }

        } while (currentRow);
    }
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
void SmartMatrixApaCalc<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::setRotation(rotationDegrees newrotation) {
    rotation = newrotation;
    rotationChange = true;
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
uint16_t SmartMatrixApaCalc<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::getScreenWidth(void) const {
    if (rotation == rotation0 || rotation == rotation180) {
        return matrixWidth;
    } else {
        return matrixHeight;
    }
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
uint16_t SmartMatrixApaCalc<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::getScreenHeight(void) const {
    if (rotation == rotation0 || rotation == rotation180) {
        return matrixHeight;
    } else {
        return matrixWidth;
    }
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
volatile bool SmartMatrixApaCalc<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::rotationChange = true;
template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
rotationDegrees SmartMatrixApaCalc<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::rotation = rotation0;

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
void SmartMatrixApaCalc<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::setBrightness(uint8_t newBrightness) {
    dimmingFactor = dimmingMaximum - newBrightness;
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
void SmartMatrixApaCalc<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::setRefreshRate(uint8_t newRefreshRate) {
    if(newRefreshRate > MIN_REFRESH_RATE)
        refreshRate = newRefreshRate;
    else
        refreshRate = MIN_REFRESH_RATE;
    refreshRateChanged = true;
    SmartMatrixAPA102Refresh<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::setRefreshRate(refreshRate);
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
void SmartMatrixApaCalc<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::setSpiClockSpeed(uint32_t newClockSpeed) {
    spiClockSpeed = newClockSpeed;
    SmartMatrixAPA102Refresh<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::setSpiClockSpeed(spiClockSpeed);
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
uint8_t SmartMatrixApaCalc<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::getRefreshRate(void) {
    return refreshRate;
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
bool SmartMatrixApaCalc<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::getdmaBufferUnderrunFlag(void) {
    if(dmaBufferUnderrunSinceLastCheck) {
        dmaBufferUnderrunSinceLastCheck = false;
        return true;
    }
    return false;
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
bool SmartMatrixApaCalc<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::getRefreshRateLoweredFlag(void) {
    if(refreshRateLowered) {
        refreshRateLowered = false;
        return true;
    }
    return false;
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
void SmartMatrixApaCalc<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::begin(void)
{
    SM_Layer * templayer = baseLayer;
    while(templayer) {
        templayer->begin();
        templayer = templayer->nextLayer;
    }

    SmartMatrixAPA102Refresh<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::setMatrixCalculationsCallback(matrixCalculations);
    SmartMatrixAPA102Refresh<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::setMatrixUnderrunCallback(dmaBufferUnderrunCallback);
    SmartMatrixAPA102Refresh<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::begin();
}

// large factor = more dim, default is full brightness
template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
int SmartMatrixApaCalc<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::dimmingFactor = 0;

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
INLINE void SmartMatrixApaCalc<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::loadMatrixBuffers(frameDataStruct * currentRowDataPtr, unsigned char currentRow) {
    int i,j;

    // static to avoid putting large buffer on the stack
    static rgb48 tempRow0[matrixWidth];

    // clear buffer to prevent garbage data showing through transparent layers
    memset(tempRow0, 0x00, sizeof(tempRow0));

    // get pixel data from layers
    SM_Layer * templayer = SmartMatrixApaCalc<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::baseLayer;
    while(templayer) {
        templayer->fillRefreshRow(currentRow, &tempRow0[0]);
        templayer = templayer->nextLayer;        
    }

    if(!currentRow) {
        // fill start and end frame markers
        for(i=0; i<4; i++) {
            currentRowDataPtr->data[i] = 0;
            currentRowDataPtr->data[4 + (matrixWidth * matrixHeight * 4) + i] = 0xFF;
        }
    }

    for (j = 0; j < matrixWidth; j++) {
        // we send data out in serpentine layout only
        if(currentRow % 2)
            i=(matrixWidth-j-1);
        else
            i=j;

        uint16_t tempPixel1, tempPixel2, tempPixel3;

        switch(optionFlags & SM_APA102_OPTIONS_COLOR_ORDER_MASK) {
            case SM_APA102_OPTIONS_COLOR_ORDER_RGB:
                tempPixel1 = tempRow0[j].red;
                tempPixel2 = tempRow0[j].green;
                tempPixel3 = tempRow0[j].blue;
                break;
            case SM_APA102_OPTIONS_COLOR_ORDER_RBG:
                tempPixel1 = tempRow0[j].red;
                tempPixel2 = tempRow0[j].blue;
                tempPixel3 = tempRow0[j].green;
                break;
                
            case SM_APA102_OPTIONS_COLOR_ORDER_GRB:
                tempPixel1 = tempRow0[j].green;
                tempPixel2 = tempRow0[j].red;
                tempPixel3 = tempRow0[j].blue;
                break;
                
            case SM_APA102_OPTIONS_COLOR_ORDER_GBR:
                tempPixel1 = tempRow0[j].green;
                tempPixel2 = tempRow0[j].blue;
                tempPixel3 = tempRow0[j].red;
                break;
                
            case SM_APA102_OPTIONS_COLOR_ORDER_BGR:
                tempPixel1 = tempRow0[j].blue;
                tempPixel2 = tempRow0[j].green;
                tempPixel3 = tempRow0[j].red;
                break;
                
            case SM_APA102_OPTIONS_COLOR_ORDER_BRG:
                tempPixel1 = tempRow0[j].blue;
                tempPixel2 = tempRow0[j].red;
                tempPixel3 = tempRow0[j].green;
                break;
        }

        // "DEFAULT" mode attempts to get 13-bit color per channel using the full range of GBC bits, this looks better than "SIMPLE" mode, but takes longer, and there are still non-linearity issues
        if((optionFlags & SM_APA102_OPTIONS_GBC_MODE_MASK) == SM_APA102_OPTIONS_GBC_MODE_DEFAULT) {
            uint8_t globalbrightness = (0x1F * (dimmingMaximum - dimmingFactor)) / dimmingMaximum;

            uint16_t maxrgb = max(max(tempPixel1, tempPixel2), tempPixel3);

            uint16_t value  = (maxrgb * 31 * globalbrightness) / 0x10000 / 31;

            currentRowDataPtr->data[4 + ((currentRow * matrixWidth + i) * 4) + 0] = 0xE0 | (value+1);
            currentRowDataPtr->data[4 + ((currentRow * matrixWidth + i) * 4) + 1] = ((tempPixel1 * globalbrightness) / (value + 1)) >> 8;
            currentRowDataPtr->data[4 + ((currentRow * matrixWidth + i) * 4) + 2] = ((tempPixel2 * globalbrightness) / (value + 1)) >> 8;
            currentRowDataPtr->data[4 + ((currentRow * matrixWidth + i) * 4) + 3] = ((tempPixel3 * globalbrightness) / (value + 1)) >> 8;
        }

        // "SIMPLE" mode attempts to get 13-bit color per channel by first applying the setBrightness() value to the GBC bits, then dividing by two to attempt to get more bits for dimmer colors, this is not as good as "DEFAULT" mode, but is more efficient
        if((optionFlags & SM_APA102_OPTIONS_GBC_MODE_MASK) == SM_APA102_OPTIONS_GBC_MODE_SIMPLE) {
            uint8_t globalbrightness = (0x20UL * (dimmingMaximum - dimmingFactor)) / dimmingMaximum;
            uint8_t localshift = 0;

            if(globalbrightness == 0x20)
                globalbrightness = 0x1f;

            uint16_t value = tempPixel1 | tempPixel2 | tempPixel3;

            // shift until the highest bit of value is set, or until globalbrightness == 1)
            while(!((value << localshift) & 0x8000) && (globalbrightness > 1)) {
                globalbrightness >>= 1;
                localshift++;
            }

            // shift needs to put 16-bit color value into lowest byte, which will be sent over SPI
            localshift = 8 - localshift;

            // global brightness
            currentRowDataPtr->data[4 + ((currentRow * matrixWidth + i) * 4) + 0] = 0xE0 | globalbrightness;

            currentRowDataPtr->data[4 + ((currentRow * matrixWidth + i) * 4) + 1] = tempPixel1 >> localshift;
            currentRowDataPtr->data[4 + ((currentRow * matrixWidth + i) * 4) + 2] = tempPixel2 >> localshift;
            currentRowDataPtr->data[4 + ((currentRow * matrixWidth + i) * 4) + 3] = tempPixel3 >> localshift;
        }

        // "BRIGHTONLY" applies the setBrightness() value to the GBC bits, so the same GBC is used across all LEDs
        if((optionFlags & SM_APA102_OPTIONS_GBC_MODE_MASK) == SM_APA102_OPTIONS_GBC_MODE_BRIGHTONLY) {
            uint8_t globalbrightness = (0x20UL * (dimmingMaximum - dimmingFactor)) / dimmingMaximum;

            if(globalbrightness == 0x20)
                globalbrightness = 0x1f;

            // global brightness
            currentRowDataPtr->data[4 + ((currentRow * matrixWidth + i) * 4) + 0] = 0xE0 | globalbrightness;

            currentRowDataPtr->data[4 + ((currentRow * matrixWidth + i) * 4) + 1] = tempPixel1 >> 8;
            currentRowDataPtr->data[4 + ((currentRow * matrixWidth + i) * 4) + 2] = tempPixel2 >> 8;
            currentRowDataPtr->data[4 + ((currentRow * matrixWidth + i) * 4) + 3] = tempPixel3 >> 8;
        }

        // "NONE" mode doesn't use GBC at all, the LED output is 24-bit color
        if((optionFlags & SM_APA102_OPTIONS_GBC_MODE_MASK) == SM_APA102_OPTIONS_GBC_MODE_NONE) {
            // global brightness
            currentRowDataPtr->data[4 + ((currentRow * matrixWidth + i) * 4) + 0] = 0xFF;

            tempPixel3 = (tempPixel3 * (dimmingMaximum - dimmingFactor)) / dimmingMaximum;
            tempPixel2 = (tempPixel2 * (dimmingMaximum - dimmingFactor)) / dimmingMaximum;
            tempPixel1 = (tempPixel1 * (dimmingMaximum - dimmingFactor)) / dimmingMaximum;

            currentRowDataPtr->data[4 + ((currentRow * matrixWidth + i) * 4) + 1] = tempPixel1 >> 8;
            currentRowDataPtr->data[4 + ((currentRow * matrixWidth + i) * 4) + 2] = tempPixel2 >> 8;
            currentRowDataPtr->data[4 + ((currentRow * matrixWidth + i) * 4) + 3] = tempPixel3 >> 8;
        }
    }
}
