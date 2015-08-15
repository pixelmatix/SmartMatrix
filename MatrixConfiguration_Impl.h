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

template <typename RGB>
void SmartMatrix<RGB>::setRotation(rotationDegrees newrotation) {
    rotation = newrotation;
    rotationChange = true;
}

template <typename RGB>
uint16_t SmartMatrix<RGB>::getScreenWidth(void) const {
    if (rotation == rotation0 || rotation == rotation180) {
        return matrixWidth;
    } else {
        return matrixHeight;
    }
}

template <typename RGB>
uint16_t SmartMatrix<RGB>::getScreenHeight(void) const {
    if (rotation == rotation0 || rotation == rotation180) {
        return matrixHeight;
    } else {
        return matrixWidth;
    }
}

template <typename RGB>
volatile bool SmartMatrix<RGB>::brightnessChange = false;
template <typename RGB>
volatile bool SmartMatrix<RGB>::rotationChange = true;
template <typename RGB>
rotationDegrees SmartMatrix<RGB>::rotation = rotation0;

template <typename RGB>
const int SmartMatrix<RGB>::dimmingMaximum = 255;
// large factor = more dim, default is full brightness
template <typename RGB>
int SmartMatrix<RGB>::dimmingFactor = dimmingMaximum - (100 * 255)/100;

template <typename RGB>
void SmartMatrix<RGB>::setBrightness(uint8_t brightness) {
    dimmingFactor = dimmingMaximum - brightness;
    brightnessChange = true;
}

template <typename RGB>
void SmartMatrix<RGB>::setRefreshRate(uint8_t newRefreshRate) {
    refreshRate = newRefreshRate;
    calculateTimerLut();
}

