/*
 * SmartMatrix Library - Methods for setting screen options
 *
 * Copyright (c) 2014 Louis Beaudoin (Pixelmatix)
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

template <int refreshDepth, int matrixWidth, unsigned char optionFlags>
void SmartMatrix3<refreshDepth, matrixWidth, optionFlags>::setRotation(rotationDegrees newrotation) {
    rotation = newrotation;
    rotationChange = true;
}

template <int refreshDepth, int matrixWidth, unsigned char optionFlags>
uint16_t SmartMatrix3<refreshDepth, matrixWidth, optionFlags>::getScreenWidth(void) const {
    if (rotation == rotation0 || rotation == rotation180) {
        return matrixWidth;
    } else {
        return matrixHeight;
    }
}

template <int refreshDepth, int matrixWidth, unsigned char optionFlags>
uint16_t SmartMatrix3<refreshDepth, matrixWidth, optionFlags>::getScreenHeight(void) const {
    if (rotation == rotation0 || rotation == rotation180) {
        return matrixHeight;
    } else {
        return matrixWidth;
    }
}

template <int refreshDepth, int matrixWidth, unsigned char optionFlags>
volatile bool SmartMatrix3<refreshDepth, matrixWidth, optionFlags>::brightnessChange = false;
template <int refreshDepth, int matrixWidth, unsigned char optionFlags>
volatile bool SmartMatrix3<refreshDepth, matrixWidth, optionFlags>::rotationChange = true;
template <int refreshDepth, int matrixWidth, unsigned char optionFlags>
rotationDegrees SmartMatrix3<refreshDepth, matrixWidth, optionFlags>::rotation = rotation0;

template <int refreshDepth, int matrixWidth, unsigned char optionFlags>
const int SmartMatrix3<refreshDepth, matrixWidth, optionFlags>::dimmingMaximum = 255;
// large factor = more dim, default is full brightness
template <int refreshDepth, int matrixWidth, unsigned char optionFlags>
int SmartMatrix3<refreshDepth, matrixWidth, optionFlags>::dimmingFactor = dimmingMaximum - (100 * 255)/100;

template <int refreshDepth, int matrixWidth, unsigned char optionFlags>
void SmartMatrix3<refreshDepth, matrixWidth, optionFlags>::setBrightness(uint8_t brightness) {
    dimmingFactor = dimmingMaximum - brightness;
    brightnessChange = true;
}

template <int refreshDepth, int matrixWidth, unsigned char optionFlags>
void SmartMatrix3<refreshDepth, matrixWidth, optionFlags>::setRefreshRate(uint8_t newRefreshRate) {
    refreshRate = newRefreshRate;
    calculateTimerLut();
}

