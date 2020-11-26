/*
 * SmartMatrix Library - Generic Layer Class
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

#ifndef _LAYER_H_
#define _LAYER_H_

#include "MatrixCommon.h"

class SM_Layer {
    public:
        virtual void begin() = 0;
        virtual void frameRefreshCallback() = 0;

        // fills refreshRow with matrixWidth values - hardwareY is < matrixHeight, not localHeight
        virtual void fillRefreshRow(uint16_t hardwareY, rgb48 refreshRow[], int brightnessShifts = 0) = 0;
        virtual void fillRefreshRow(uint16_t hardwareY, rgb24 refreshRow[], int brightnessShifts = 0) = 0;

        virtual void setRotation(rotationDegrees newrotation);
        rotationDegrees getLayerRotation(void) const { return layerRotation; };
        uint16_t getLayerWidth(void) const { return layerWidth; };
        uint16_t getLayerHeight(void) const { return layerHeight; };
        uint16_t getLocalWidth(void) const { return localWidth; };
        uint16_t getLocalHeight(void) const { return localHeight; };
        virtual void setRefreshRate(uint8_t newRefreshRate);
        virtual int getRequestedBrightnessShifts();
        virtual bool isLayerChanged();

        SM_Layer * nextLayer;

    protected:
        rotationDegrees layerRotation;
        
        // matrixWidth/Height: the dimensions of the physical hardware, with physical row 0 column 0 always in the upper left, independent of any rotation done in software
        uint16_t matrixWidth, matrixHeight;
        // layerWidth/Height: the dimensions of the pixels stored in this layer, matching the orientation of the physical hardware, with physical row 0 column 0 always in the upper left, independent of any rotation done in software
        uint16_t layerWidth, layerHeight;
        // the local dimensions of this layer with rotation applied, local x=0,y=0 in the upper left
        uint16_t localWidth, localHeight;
        uint8_t refreshRate;
        
    private:
};

#endif
