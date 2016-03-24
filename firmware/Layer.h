/*
 * SmartMatrix Library - Generic Layer Class
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

#ifndef _LAYER_H_
#define _LAYER_H_

#include "MatrixCommon.h"

class SM_Layer {
    public:
        virtual void frameRefreshCallback();

        // fills refreshRow with matrixWidth values - hardwareY is < matrixHeight, not localHeight
        virtual void fillRefreshRow(uint16_t hardwareY, rgb48 refreshRow[]);
        virtual void fillRefreshRow(uint16_t hardwareY, rgb24 refreshRow[]);

        void setRotation(rotationDegrees newrotation);
        virtual void setRefreshRate(uint8_t newRefreshRate);

        SM_Layer * nextLayer;

    protected:
        rotationDegrees rotation;
        uint16_t matrixWidth, matrixHeight;
        uint16_t localWidth, localHeight;
        uint8_t refreshRate;
        
    private:
};

#endif
