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

#include "SmartMatrix.h"

screen_config SmartMatrix::screenConfig = {
    .rotation = rotation0,
    .localWidth = MATRIX_WIDTH,
    .localHeight = MATRIX_HEIGHT,
};

void SmartMatrix::setRotation(rotationDegrees rotation) {
    screenConfig.rotation = rotation;

    if (rotation == rotation0 || rotation == rotation180) {
        screenConfig.localWidth = MATRIX_WIDTH;
        screenConfig.localHeight = MATRIX_HEIGHT;
    } else {
        screenConfig.localWidth = MATRIX_HEIGHT;
        screenConfig.localHeight = MATRIX_WIDTH;
    }
}

uint16_t SmartMatrix::getScreenWidth(void) const {
    return screenConfig.localWidth;
}

uint16_t SmartMatrix::getScreenHeight(void) const {
    return screenConfig.localHeight;
}

volatile bool SmartMatrix::brightnessChange = false;
const int SmartMatrix::dimmingMaximum = 255;
// large factor = more dim, default is full brightness
int SmartMatrix::dimmingFactor = dimmingMaximum - (100 * 255)/100;

void SmartMatrix::setBrightness(uint8_t brightness) {
    dimmingFactor = dimmingMaximum - brightness;
    brightnessChange = true;
}

uint8_t SmartMatrix::backgroundBrightness = 255;

void SmartMatrix::setBackgroundBrightness(uint8_t brightness) {
    backgroundBrightness = brightness;
}

