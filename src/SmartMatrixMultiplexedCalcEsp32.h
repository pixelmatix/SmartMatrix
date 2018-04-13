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

#ifndef SmartMatrixMultiplexedCalc_h
#define SmartMatrixMultiplexedCalc_h

template <int refreshDepth, int matrixWidth, int matrixHeight, unsigned char panelType, unsigned char optionFlags>
class SmartMatrix3 {
public:
    typedef typename SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::frameStruct frameStruct;
    typedef typename SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::rowDataStruct rowDataStruct;
    typedef typename SmartMatrix3RefreshMultiplexed<refreshDepth, matrixWidth, matrixHeight, panelType, optionFlags>::rowBitStruct rowBitStruct;

    // init
    SmartMatrix3(void);
    void begin(void);
    void addLayer(SM_Layer * newlayer);

    // configuration
    void setRotation(rotationDegrees rotation);
    void setBrightness(uint8_t newBrightness);
    void setRefreshRate(uint8_t newRefreshRate);

    // get info
    uint16_t getScreenWidth(void) const;
    uint16_t getScreenHeight(void) const;
    uint8_t getRefreshRate(void);
    bool getdmaBufferUnderrunFlag(void);
    bool getRefreshRateLoweredFlag(void);

    // debug
    void countFPS(void);

    // functions called by ISR
    static void matrixCalculations(int lsbMsbTransitionBit);
    static void dmaBufferUnderrunCallback(void);

private:
    static SM_Layer * baseLayer;

    // functions for refreshing
    static void loadMatrixBuffers(int lsbMsbTransitionBit);
    static void loadMatrixBuffers48(frameStruct * currentFrameDataPtr, int currentRow, int lsbMsbTransitionBit);
    static void loadMatrixBuffers24(frameStruct * currentFrameDataPtr, int currentRow, int lsbMsbTransitionBit);

    // configuration
    static volatile bool brightnessChange;
    static volatile bool rotationChange;
    static volatile bool dmaBufferUnderrun;
    static int brightness;
    static rotationDegrees rotation;
    static uint8_t calc_refreshRate;   
    static bool dmaBufferUnderrunSinceLastCheck;
    static bool refreshRateLowered;
    static bool refreshRateChanged;
};

#endif
