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
 
void SmartMatrix3::setRotation(rotationDegrees newrotation) {
    rotation = newrotation;
    rotationChange = true;
}

uint16_t SmartMatrix3::getScreenWidth(void) const {
    if (rotation == rotation0 || rotation == rotation180) {
        return matrixWidth;
    } else {
        return matrixHeight;
    }
}

uint16_t SmartMatrix3::getScreenHeight(void) const {
    if (rotation == rotation0 || rotation == rotation180) {
        return matrixHeight;
    } else {
        return matrixWidth;
    }
}

volatile bool SmartMatrix3::brightnessChange = false;
volatile bool SmartMatrix3::rotationChange = true;
rotationDegrees SmartMatrix3::rotation = rotation0;

const int SmartMatrix3::dimmingMaximum = 255;
// large factor = more dim, default is full brightness
int SmartMatrix3::dimmingFactor = dimmingMaximum - (100 * 255)/100;

void SmartMatrix3::setBrightness(uint8_t brightness) {
    dimmingFactor = dimmingMaximum - brightness;
    brightnessChange = true;
}

void SmartMatrix3::setRefreshRate(uint8_t newRefreshRate) {
    refreshRate = newRefreshRate;
    calculateTimerLut();
}

