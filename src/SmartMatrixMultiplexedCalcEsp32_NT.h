/*
 * SmartMatrix Library - Multiplexed Panel Calculation Class
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

#ifndef SmartMatrixMultiplexedCalc_NT_h
#define SmartMatrixMultiplexedCalc_NT_h

extern SemaphoreHandle_t calcTaskSemaphore;
extern void matrixCalculationsSignal(void);

template <int dummyvar>
class SmartMatrix3_NT {
public:
    // init
    SmartMatrix3_NT(SmartMatrix3RefreshMultiplexed_NT<0>* matrixRefresh, int width, int height, unsigned char depth, unsigned char type, unsigned char options) :
        _matrixRefresh(matrixRefresh), matrixWidth(width), matrixHeight(height), optionFlags(options), panelType(type), refreshDepth(depth), pixels_per_latch(PIXELS_PER_LATCH),
        matrix_panel_height(MATRIX_PANEL_HEIGHT), matrix_stack_height(MATRIX_STACK_HEIGHT), color_depth_bits(COLOR_DEPTH_BITS), matrix_scan_mod(MATRIX_SCAN_MOD),
        cols_per_panel(COLS_PER_PANEL), physical_rows_per_refresh_row(PHYSICAL_ROWS_PER_REFRESH_ROW), row_pair_offset(ROW_PAIR_OFFSET) {};
    void begin(uint32_t dmaRamToKeepFreeBytes = 0);
    void addLayer(SM_Layer * newlayer);

    // configuration
    void setRotation(rotationDegrees rotation);
    void setBrightness(uint8_t newBrightness);
    void setRefreshRate(uint16_t newRefreshRate);

    // get info
    uint16_t getScreenWidth(void) const;
    uint16_t getScreenHeight(void) const;
    uint16_t getRefreshRate(void);
    bool getdmaBufferUnderrunFlag(void);
    bool getRefreshRateLoweredFlag(void);
    void setMaxCalculationCpuPercentage(uint8_t newMaxCpuPercentage);

    // debug
    void countFPS(void);

    // functions called by ISR
    void matrixCalculations(void);
    void dmaBufferUnderrunCallback(void);
    void setCalcRefreshRateDivider(uint8_t newDivider);
    uint8_t getCalcRefreshRateDivider(void);

private:
    SM_Layer * baseLayer;

    void * tempRow0Ptr;
    void * tempRow1Ptr;

    // functions for refreshing
    void loadMatrixBuffers(int lsbMsbTransitionBit, int numBrightnessShifts = 0);
    void loadMatrixBuffers48(MATRIX_DATA_STORAGE_TYPE * currentFrameDataPtr, int currentRow, int lsbMsbTransitionBit, int numBrightnessShifts = 0);
    void loadMatrixBuffers24(MATRIX_DATA_STORAGE_TYPE * currentFrameDataPtr, int currentRow, int lsbMsbTransitionBit, int numBrightnessShifts = 0);
    static void calcTask(void* pvParameters);
    void resetMultiRowRefreshMapPosition(void);
    void resetMultiRowRefreshMapPositionPixelGroupToStartOfRow(void);
    void advanceMultiRowRefreshMapToNextRow(void);
    void advanceMultiRowRefreshMapToNextPixelGroup(void);
    int getMultiRowRefreshRowOffset(void);
    int getMultiRowRefreshNumPixelsToMap(void);
    int getMultiRowRefreshPixelGroupOffset(void);
    
    // configuration
    volatile bool brightnessChange;
    volatile bool rotationChange;
    volatile bool dmaBufferUnderrun;
    int brightness;
    int shiftedBrightness;
    rotationDegrees rotation;
    uint16_t calc_refreshRate;   
    uint8_t calc_refreshRateDivider;
    bool dmaBufferUnderrunSinceLastCheck;
    // to avoid 100% CPU usage, we by default don't calculate on every frame.  Calc refresh rate will be a fraction of Refresh refresh rate
    uint8_t maxCalcCpuPercentage;
    bool refreshRateLowered;
    bool refreshRateChanged;
    uint8_t lsbMsbTransitionBit;
    TaskHandle_t calcTaskHandle;
    
    int multiRowRefresh_mapIndex_CurrentRowGroups;
    int multiRowRefresh_mapIndex_CurrentPixelGroup;
    int multiRowRefresh_PixelOffsetFromPanelsAlreadyMapped;
    int multiRowRefresh_NumPanelsAlreadyMapped;

    SmartMatrix3RefreshMultiplexed_NT<0> * _matrixRefresh;
    const uint32_t optionFlags;
    const uint8_t panelType;
    const int matrixWidth;
    const int matrixHeight;
    const uint8_t refreshDepth;

    // these are usually calculated by all caps macros, but it's better to precalculate (at least some of) these values for efficiency for this class that doesn't know its parameters at compile time
    const uint16_t matrix_panel_height;
    const uint16_t matrix_stack_height;
    const uint8_t color_depth_bits;
    const uint8_t matrix_scan_mod;
    const uint16_t cols_per_panel;
    const uint16_t physical_rows_per_refresh_row;
    const uint8_t row_pair_offset;
    const uint16_t pixels_per_latch;
};

#endif
