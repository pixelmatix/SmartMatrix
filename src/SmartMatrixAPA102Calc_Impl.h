/*
 * SmartMatrix Library - APA102 Calculation Code for Teensy 3.x Platform
 *
 * Copyright (c) 2017 Louis Beaudoin (Pixelmatix)
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
uint8_t SmartMatrixApaCalc<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::refreshRate = 60;
template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
SM_Layer * SmartMatrixApaCalc<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::baseLayer;

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
volatile bool SmartMatrixApaCalc<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::dmaBufferUnderrun = false;

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
bool SmartMatrixApaCalc<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::dmaBufferUnderrunSinceLastCheck = false;

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
bool SmartMatrixApaCalc<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::refreshRateLowered = false;

// set to true initially so all layers get the initial refresh rate
template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
bool SmartMatrixApaCalc<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::refreshRateChanged = true;

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
SmartMatrixApaCalc<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::SmartMatrixApaCalc(uint8_t bufferrows, frameDataStruct * frameDataBuffer) {
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
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

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
void SmartMatrixApaCalc<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::countFPS(void) {
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
void SmartMatrixApaCalc<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::dmaBufferUnderrunCallback(void) {
    dmaBufferUnderrun = true;
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
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

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
void SmartMatrixApaCalc<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::setRotation(rotationDegrees newrotation) {
    rotation = newrotation;
    rotationChange = true;
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
uint16_t SmartMatrixApaCalc<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::getScreenWidth(void) const {
    if (rotation == rotation0 || rotation == rotation180) {
        return matrixWidth;
    } else {
        return matrixHeight;
    }
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
uint16_t SmartMatrixApaCalc<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::getScreenHeight(void) const {
    if (rotation == rotation0 || rotation == rotation180) {
        return matrixHeight;
    } else {
        return matrixWidth;
    }
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
volatile bool SmartMatrixApaCalc<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::rotationChange = true;
template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
rotationDegrees SmartMatrixApaCalc<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::rotation = rotation0;

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
void SmartMatrixApaCalc<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::setBrightness(uint8_t newBrightness) {
    dimmingFactor = dimmingMaximum - newBrightness;
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
void SmartMatrixApaCalc<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::setRefreshRate(uint8_t newRefreshRate) {
    if(newRefreshRate > MIN_REFRESH_RATE)
        refreshRate = newRefreshRate;
    else
        refreshRate = MIN_REFRESH_RATE;
    refreshRateChanged = true;
    SmartMatrixAPA102Refresh<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::setRefreshRate(refreshRate);
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
uint8_t SmartMatrixApaCalc<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::getRefreshRate(void) {
    return refreshRate;
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
bool SmartMatrixApaCalc<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::getdmaBufferUnderrunFlag(void) {
    if(dmaBufferUnderrunSinceLastCheck) {
        dmaBufferUnderrunSinceLastCheck = false;
        return true;
    }
    return false;
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
bool SmartMatrixApaCalc<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::getRefreshRateLoweredFlag(void) {
    if(refreshRateLowered) {
        refreshRateLowered = false;
        return true;
    }
    return false;
}

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
void SmartMatrixApaCalc<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::begin(void)
{
    SmartMatrixAPA102Refresh<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::setMatrixCalculationsCallback(matrixCalculations);
    SmartMatrixAPA102Refresh<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::setMatrixUnderrunCallback(dmaBufferUnderrunCallback);
    SmartMatrixAPA102Refresh<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::begin();
}

// large factor = more dim, default is full brightness
template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
int SmartMatrixApaCalc<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::dimmingFactor = dimmingMaximum - (100 * 255)/100;

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
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
        currentRowDataPtr->data[4 + ((currentRow * matrixWidth + i) * 4) + 0] = 0xE0 | globalbrightness;

        currentRowDataPtr->data[4 + ((currentRow * matrixWidth + i) * 4) + 1] = temp0blue >> localshift;
        currentRowDataPtr->data[4 + ((currentRow * matrixWidth + i) * 4) + 2] = temp0green >> localshift;
        currentRowDataPtr->data[4 + ((currentRow * matrixWidth + i) * 4) + 3] = temp0red >> localshift;
    }
}
