#include "SmartMatrix3.h"

#define INLINE __attribute__( ( always_inline ) ) inline
#define MAX_MATRIXCALCULATIONS_LOOPS_WITHOUT_EXIT  5

#ifndef FLASHMEM
#define FLASHMEM // for compatibility with old versions of Teensyduino
#endif


template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
SM_Layer * SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::baseLayer;
template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
volatile bool SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::dmaBufferUnderrun = false;
template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
bool SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::dmaBufferUnderrunSinceLastCheck = false;
template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
uint16_t SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::calc_refreshRate = 240;
template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
bool SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::refreshRateLowered = false;
// set to true initially so all layers get the initial refresh rate
template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
bool SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::refreshRateChanged = true;
template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
volatile bool SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::brightnessChange = false;
template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
volatile bool SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::rotationChange = true;
template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
rotationDegrees SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::rotation = rotation0;
template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
uint8_t SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::brightness;


template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::SmartMatrix3(uint8_t bufferrows, volatile rowDataStruct * rowDataBuf) {
}


template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
void SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::addLayer(SM_Layer * newlayer) {
    if (baseLayer) {
        SM_Layer * templayer = baseLayer;
        while (templayer->nextLayer)
            templayer = templayer->nextLayer;
        templayer->nextLayer = newlayer;
    } else {
        baseLayer = newlayer;
    }
}


template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
int SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::countFPS(void) {
    static long loops = 0;
    static long lastMillis = 0;
    long currentMillis = millis();
    int ret = 0;

    loops++;
    if (currentMillis - lastMillis >= 1000) {
#if defined(USB_SERIAL)
        if (Serial) {
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


template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
void SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::dmaBufferUnderrunCallback(void) {
    dmaBufferUnderrun = true;
}


// Refill the row buffer (called by rowCalculationISR). It may be interrupted by rowShiftCompleteISR.
template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
FASTRUN void SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::matrixCalculations(bool initial) {
    static unsigned int currentRow = 0;   // keeps track of the next row to write into the buffer
    unsigned char numLoopsWithoutExit = 0;

    // only run the loop if there is free space, and fill the entire buffer before returning
    while (SmartMatrixRefreshT4<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::isRowBufferFree()) {

        // check to see if the refresh rate is too high, and the application doesn't have time to run
        if (++numLoopsWithoutExit > MAX_MATRIXCALCULATIONS_LOOPS_WITHOUT_EXIT) {
            // minimum set to avoid overflowing timer at low refresh rates
            if (!initial && calc_refreshRate > MIN_REFRESH_RATE) {
                calc_refreshRate--;
                SmartMatrixRefreshT4<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::setRefreshRate(calc_refreshRate);
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
                while (templayer) {
                    templayer->setRotation(rotation);
                    templayer = templayer->nextLayer;
                }
                rotationChange = false;
            }
            SM_Layer * templayer = SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::baseLayer;
            while (templayer) {
                if (refreshRateChanged) {
                    templayer->setRefreshRate(calc_refreshRate);
                }
                templayer->frameRefreshCallback();
                templayer = templayer->nextLayer;
            }
            refreshRateChanged = false;
            if (brightnessChange) {
                SmartMatrixRefreshT4<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::setBrightness(brightness);
                brightnessChange = false;
            }
        }

        // do once-per-line updates
        // none right now

        // enqueue row
        SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::loadMatrixBuffers(currentRow);
        SmartMatrixRefreshT4<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::writeRowBuffer(currentRow);

        if (++currentRow >= MATRIX_SCAN_MOD) currentRow = 0;

        if (dmaBufferUnderrun) {
            // if refreshrate is too high, lower - minimum set to avoid overflowing timer at low refresh rates
            if (calc_refreshRate > MIN_REFRESH_RATE) {
                calc_refreshRate--;
                SmartMatrixRefreshT4<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::setRefreshRate(calc_refreshRate);
                refreshRateLowered = true;
                refreshRateChanged = true;
            }
            SmartMatrixRefreshT4<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::recoverFromDmaUnderrun();
            dmaBufferUnderrunSinceLastCheck = true;
            dmaBufferUnderrun = false;
        }
    }
}


template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
void SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::setRotation(rotationDegrees newrotation) {
    rotation = newrotation;
    rotationChange = true;
}


template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
uint16_t SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::getScreenWidth(void) const {
    if (rotation == rotation0 || rotation == rotation180) {
        return matrixWidth;
    } else {
        return matrixHeight;
    }
}


template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
uint16_t SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::getScreenHeight(void) const {
    if (rotation == rotation0 || rotation == rotation180) {
        return matrixHeight;
    } else {
        return matrixWidth;
    }
}


template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
void SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::setBrightness(uint8_t newBrightness) {
    brightness = newBrightness;
    brightnessChange = true;
}


template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
void SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::setRefreshRate(uint16_t newRefreshRate) {
    if (newRefreshRate <= MIN_REFRESH_RATE)
        calc_refreshRate = MIN_REFRESH_RATE;
    else if (newRefreshRate >= MAX_REFRESH_RATE)
        calc_refreshRate = MAX_REFRESH_RATE;
    else
        calc_refreshRate = newRefreshRate;
    refreshRateChanged = true;
    SmartMatrixRefreshT4<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::setRefreshRate(calc_refreshRate);
}


template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
uint16_t SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::getRefreshRate(void) {
    return calc_refreshRate;
}


template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
bool SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::getdmaBufferUnderrunFlag(void) {
    if (dmaBufferUnderrunSinceLastCheck) {
        dmaBufferUnderrunSinceLastCheck = false;
        return true;
    }
    return false;
}


template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
bool SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::getRefreshRateLoweredFlag(void) {
    if (refreshRateLowered) {
        refreshRateLowered = false;
        return true;
    }
    return false;
}


template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
FLASHMEM void SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::begin(void) {
    SM_Layer * templayer = baseLayer;
    while (templayer) {
        templayer->begin();
        templayer = templayer->nextLayer;
    }

    SmartMatrixRefreshT4<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::setMatrixCalculationsCallback(matrixCalculations);
    SmartMatrixRefreshT4<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::setMatrixUnderrunCallback(dmaBufferUnderrunCallback);
    SmartMatrixRefreshT4<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::begin();
}


template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
FASTRUN INLINE void SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::loadMatrixBuffers48(volatile rowDataStruct * currentRowDataPtr, unsigned int currentRow) {
    /*  Read a new row of pixel data from the layers, extract the bitplanes for each pixel, reformat
        the data into the format needed for FlexIO, and store that in the rowDataBuffer.
        Bit depths are supported from 1 bit per color channel (3 bits per pixel) to 16 bits per color channel (48 bits per pixel).
        For now, multi-row "multiplexed" panels are not supported. */

    // Temporary buffers to store rgb pixel data for reformatting (static to avoid putting large buffer on the stack)
    static rgb48 tempRow0[PIXELS_PER_LATCH];
    static rgb48 tempRow1[PIXELS_PER_LATCH];

    // clear buffer to prevent garbage data showing
    memset(tempRow0, 0, sizeof(tempRow0));
    memset(tempRow1, 0, sizeof(tempRow1));

    // Get pixel data from layers and store in tempRow0 and tempRow1
    // Scan through the entire chain of panels and extract rows from each one
    // using the stacking options to get the correct rows (some panels can be upside down).
    SM_Layer * templayer = SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::baseLayer;
    int y0, y1; // positions of the two rows we need
    while (templayer) {
        for (int i = 0; i < MATRIX_STACK_HEIGHT; i++) {
            // Z-shape, bottom to top
            if (!(optionFlags & SMARTMATRIX_OPTIONS_C_SHAPE_STACKING) &&
                    (optionFlags & SMARTMATRIX_OPTIONS_BOTTOM_TO_TOP_STACKING)) {
                // Bottom to Top Stacking: load data buffer with top panels first, bottom panels last, as top panels are at the furthest end of the chain (initial data is shifted out the furthest)
                y0 = currentRow + i * MATRIX_PANEL_HEIGHT;
                y1 = y0 + ROW_PAIR_OFFSET;
            // Z-shape, top to bottom
            } else if (!(optionFlags & SMARTMATRIX_OPTIONS_C_SHAPE_STACKING) &&
                       !(optionFlags & SMARTMATRIX_OPTIONS_BOTTOM_TO_TOP_STACKING)) {
                // Top to Bottom Stacking: load data buffer with bottom panels first, top panels last, as bottom panels are at the furthest end of the chain (initial data is shifted out the furthest)
                y0 = currentRow + (MATRIX_STACK_HEIGHT - i - 1) * MATRIX_PANEL_HEIGHT;
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
                    y0 = currentRow + (i) * MATRIX_PANEL_HEIGHT;
                    y1 = y0 + ROW_PAIR_OFFSET;
                } else {
                    y1 = (MATRIX_SCAN_MOD - currentRow - 1) + (i) * MATRIX_PANEL_HEIGHT;
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
                    y0 = currentRow + (MATRIX_STACK_HEIGHT - i - 1) * MATRIX_PANEL_HEIGHT;
                    y1 = y0 + ROW_PAIR_OFFSET;
                } else {
                    y1 = (MATRIX_SCAN_MOD - currentRow - 1) + (MATRIX_STACK_HEIGHT - i - 1) * MATRIX_PANEL_HEIGHT;
                    y0 = y1 + ROW_PAIR_OFFSET;
                }
            }
            templayer->fillRefreshRow(y0, &tempRow0[i * matrixWidth]);
            templayer->fillRefreshRow(y1, &tempRow1[i * matrixWidth]);
        }
        templayer = templayer->nextLayer;
    }

    // go through the entire row pixel by pixel to reformat bits
    for (int i = 0; i < PIXELS_PER_LATCH; i++) {
        uint16_t r0, g0, b0, r1, g1, b1;
        int ind;

        // for upside down stacks, flip order
        int currentStack = i/matrixWidth;
        if((optionFlags & SMARTMATRIX_OPTIONS_C_SHAPE_STACKING) && !((currentStack % 2) == ((MATRIX_STACK_HEIGHT - 1) % 2))) {
            // reverse order of this stack's data if it's reversed (if (i/matrixWidth) the last stack, or an even number of stacks away from the last stack?)
            int tempPosition = (currentStack*matrixWidth) + (matrixWidth-1) - (i%matrixWidth);
            ind = tempPosition;
        } else {
            // load data to buffer in normal order
            ind = i;
        }
        r0 = tempRow0[ind].red;
        g0 = tempRow0[ind].green;
        b0 = tempRow0[ind].blue;
        r1 = tempRow1[ind].red;
        g1 = tempRow1[ind].green;
        b1 = tempRow1[ind].blue;

        // loop through each bitplane in the current pixel's RGB values and format the bits to match the FlexIO pin configuration
        uint32_t rgbdata;
        uint8_t shift = (16 - COLOR_DEPTH_BITS);
        uint16_t mask = 1 << shift;

        for (int bitindex = 0; bitindex < COLOR_DEPTH_BITS; bitindex++) {

            rgbdata  = (r0 & mask) << (SmartMatrixRefreshT4<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::getFlexPinConfig().r0);
            rgbdata |= (g0 & mask) << (SmartMatrixRefreshT4<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::getFlexPinConfig().g0);
            rgbdata |= (b0 & mask) << (SmartMatrixRefreshT4<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::getFlexPinConfig().b0);
            rgbdata |= (r1 & mask) << (SmartMatrixRefreshT4<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::getFlexPinConfig().r1);
            rgbdata |= (g1 & mask) << (SmartMatrixRefreshT4<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::getFlexPinConfig().g1);
            rgbdata |= (b1 & mask) << (SmartMatrixRefreshT4<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::getFlexPinConfig().b1);
            rgbdata >>= shift;

            shift++;
            mask <<= 1;

            // store these pixel bits in the rowDataBuffer, leaving the initial pixels as padding
            currentRowDataPtr->rowbits[bitindex].data[PAD_PIXELS + i] = rgbdata;
        }
    }
    // record the address in the first rowAddress field in the rowBitStruct (other rowAddress fields are unused)
    currentRowDataPtr->rowbits[0].rowAddress = currentRow;
}


template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, uint32_t optionFlags>
FASTRUN INLINE void SmartMatrix3<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::loadMatrixBuffers(unsigned int currentRow) {
    volatile rowDataStruct * currentRowDataPtr = SmartMatrixRefreshT4<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::getNextRowBufferPtr();
    // same function supports any refresh depth up to 48
    loadMatrixBuffers48(currentRowDataPtr, currentRow);
}
