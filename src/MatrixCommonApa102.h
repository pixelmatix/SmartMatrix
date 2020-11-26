/*
 * SmartMatrix Library - Common Definitions for APA102
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

#ifndef SmartMatrixAPA102_h
#define SmartMatrixAPA102_h

#define SM_APA102_OPTIONS_NONE                 0
#define SM_APA102_OPTIONS_GBC_MODE_DEFAULT     (0x0 << 0)
#define SM_APA102_OPTIONS_GBC_MODE_SIMPLE      (0x1 << 0)
#define SM_APA102_OPTIONS_GBC_MODE_NONE        (0x2 << 0)
#define SM_APA102_OPTIONS_GBC_MODE_BRIGHTONLY  (0x3 << 0)
#define SM_APA102_OPTIONS_GBC_MODE_MASK        (0x3 << 0)

#define SM_APA102_OPTIONS_COLOR_ORDER_BGR      (0x0 << 2)
#define SM_APA102_OPTIONS_COLOR_ORDER_RBG      (0x1 << 2)
#define SM_APA102_OPTIONS_COLOR_ORDER_GRB      (0x2 << 2)
#define SM_APA102_OPTIONS_COLOR_ORDER_GBR      (0x3 << 2)
#define SM_APA102_OPTIONS_COLOR_ORDER_RGB      (0x4 << 2)
#define SM_APA102_OPTIONS_COLOR_ORDER_BRG      (0x5 << 2)
#define SM_APA102_OPTIONS_COLOR_ORDER_MASK     (0x7 << 2)

#endif