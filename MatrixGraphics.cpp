/*
 * SmartMatrix Library - Methods for interacting with background layer
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

#include <stdlib.h>
#include "SmartMatrix.h"

rgb24 backgroundBuffer[3][MATRIX_HEIGHT][MATRIX_WIDTH];

rgb24 (*drawBufferPtr)[MATRIX_WIDTH] = backgroundBuffer[0];
rgb24 (*currentRefreshBufferPtr)[MATRIX_WIDTH] = backgroundBuffer[1];
rgb24 (*previousRefreshBufferPtr)[MATRIX_WIDTH] = backgroundBuffer[2];
unsigned char SmartMatrix::drawBuffer = 0;
unsigned char SmartMatrix::currentRefreshBuffer = 1;
unsigned char SmartMatrix::previousRefreshBuffer = 2;
volatile bool SmartMatrix::swapPending = false;
bitmap_font *font = (bitmap_font *) &apple3x5;

// coordinates based on screen position, which is between 0-localWidth/localHeight
void SmartMatrix::getPixel(uint8_t x, uint8_t y, rgb24 *xyPixel) {
    copyRgb24(*xyPixel, currentRefreshBufferPtr[y][x]);
}

volatile int totalFramesToInterpolate;
volatile int framesInterpolated;

static const uint16_t lightPowerMap16bit[] = {
    0x00, 0x00, 0x00, 0x01, 0x02, 0x04, 0x06, 0x08,
    0x0b, 0x0f, 0x14, 0x19, 0x1f, 0x26, 0x2e, 0x37,
    0x41, 0x4b, 0x57, 0x63, 0x71, 0x80, 0x8f, 0xa0,
    0xb2, 0xc5, 0xda, 0xef, 0x106, 0x11e, 0x137, 0x152,
    0x16e, 0x18b, 0x1a9, 0x1c9, 0x1eb, 0x20e, 0x232, 0x257,
    0x27f, 0x2a7, 0x2d2, 0x2fd, 0x32b, 0x359, 0x38a, 0x3bc,
    0x3ef, 0x425, 0x45c, 0x494, 0x4cf, 0x50b, 0x548, 0x588,
    0x5c9, 0x60c, 0x651, 0x698, 0x6e0, 0x72a, 0x776, 0x7c4,
    0x814, 0x866, 0x8b9, 0x90f, 0x967, 0x9c0, 0xa1b, 0xa79,
    0xad8, 0xb3a, 0xb9d, 0xc03, 0xc6a, 0xcd4, 0xd3f, 0xdad,
    0xe1d, 0xe8f, 0xf03, 0xf79, 0xff2, 0x106c, 0x10e9, 0x1168,
    0x11e9, 0x126c, 0x12f2, 0x137a, 0x1404, 0x1490, 0x151f, 0x15b0,
    0x1643, 0x16d9, 0x1771, 0x180b, 0x18a7, 0x1946, 0x19e8, 0x1a8b,
    0x1b32, 0x1bda, 0x1c85, 0x1d33, 0x1de2, 0x1e95, 0x1f49, 0x2001,
    0x20bb, 0x2177, 0x2236, 0x22f7, 0x23bb, 0x2481, 0x254a, 0x2616,
    0x26e4, 0x27b5, 0x2888, 0x295e, 0x2a36, 0x2b11, 0x2bef, 0x2cd0,
    0x2db3, 0x2e99, 0x2f81, 0x306d, 0x315a, 0x324b, 0x333f, 0x3435,
    0x352e, 0x3629, 0x3728, 0x3829, 0x392d, 0x3a33, 0x3b3d, 0x3c49,
    0x3d59, 0x3e6b, 0x3f80, 0x4097, 0x41b2, 0x42d0, 0x43f0, 0x4513,
    0x463a, 0x4763, 0x488f, 0x49be, 0x4af0, 0x4c25, 0x4d5d, 0x4e97,
    0x4fd5, 0x5116, 0x525a, 0x53a1, 0x54eb, 0x5638, 0x5787, 0x58da,
    0x5a31, 0x5b8a, 0x5ce6, 0x5e45, 0x5fa7, 0x610d, 0x6276, 0x63e1,
    0x6550, 0x66c2, 0x6837, 0x69af, 0x6b2b, 0x6caa, 0x6e2b, 0x6fb0,
    0x7139, 0x72c4, 0x7453, 0x75e5, 0x777a, 0x7912, 0x7aae, 0x7c4c,
    0x7def, 0x7f94, 0x813d, 0x82e9, 0x8498, 0x864b, 0x8801, 0x89ba,
    0x8b76, 0x8d36, 0x8efa, 0x90c0, 0x928a, 0x9458, 0x9629, 0x97fd,
    0x99d4, 0x9bb0, 0x9d8e, 0x9f70, 0xa155, 0xa33e, 0xa52a, 0xa71a,
    0xa90d, 0xab04, 0xacfe, 0xaefb, 0xb0fc, 0xb301, 0xb509, 0xb715,
    0xb924, 0xbb37, 0xbd4d, 0xbf67, 0xc184, 0xc3a5, 0xc5ca, 0xc7f2,
    0xca1e, 0xcc4d, 0xce80, 0xd0b7, 0xd2f1, 0xd52f, 0xd771, 0xd9b6,
    0xdbfe, 0xde4b, 0xe09b, 0xe2ef, 0xe547, 0xe7a2, 0xea01, 0xec63,
    0xeeca, 0xf134, 0xf3a2, 0xf613, 0xf888, 0xfb02, 0xfd7e, 0xffff
};

uint32_t SmartMatrix::calculateFcInterpCoefficient()
{
    /*
     * Calculate our interpolation coefficient. This is a value between
     * 0x0000 and 0x10000, representing some point in between fbPrev and fbNext.
     *
     * We timestamp each frame at the moment its final packet has been received.
     * In other words, fbNew has no valid timestamp yet, and fbPrev/fbNext both
     * have timestamps in the recent past.
     *
     * fbNext's timestamp indicates when both fbPrev and fbNext entered their current
     * position in the keyframe queue. The difference between fbPrev and fbNext indicate
     * how long the interpolation between those keyframes should take.
     */

    if(framesInterpolated >= totalFramesToInterpolate)
        return 0x10000;

    return (0x10000 * framesInterpolated) / totalFramesToInterpolate;
}


rgb24 *SmartMatrix::getCurrentRefreshRow(uint8_t y) {
  return currentRefreshBufferPtr[y];
}

rgb24 *SmartMatrix::getPreviousRefreshRow(uint8_t y) {
  return previousRefreshBufferPtr[y];
}

// reads pixel from drawing buffer, not refresh buffer
rgb24 SmartMatrix::readPixel(int16_t x, int16_t y) {
    int hwx, hwy;

    // check for out of bounds coordinates
    if (x < 0 || y < 0 || x >= screenConfig.localWidth || y >= screenConfig.localHeight)
        return {0, 0, 0};

    // map pixel into hardware buffer before writing
    if (screenConfig.rotation == rotation0) {
        hwx = x;
        hwy = y;
    } else if (screenConfig.rotation == rotation180) {
        hwx = (MATRIX_WIDTH - 1) - x;
        hwy = (MATRIX_HEIGHT - 1) - y;
    } else if (screenConfig.rotation == rotation90) {
        hwx = (MATRIX_WIDTH - 1) - y;
        hwy = x;
    } else { /* if (screenConfig.rotation == rotation270)*/
        hwx = y;
        hwy = (MATRIX_HEIGHT - 1) - x;
    }

    return drawBufferPtr[hwy][hwx];
}

void SmartMatrix::drawPixel(int16_t x, int16_t y, rgb24 color) {
    int hwx, hwy;

    // check for out of bounds coordinates
    if (x < 0 || y < 0 || x >= screenConfig.localWidth || y >= screenConfig.localHeight)
        return;

    // map pixel into hardware buffer before writing
    if (screenConfig.rotation == rotation0) {
        hwx = x;
        hwy = y;
    } else if (screenConfig.rotation == rotation180) {
        hwx = (MATRIX_WIDTH - 1) - x;
        hwy = (MATRIX_HEIGHT - 1) - y;
    } else if (screenConfig.rotation == rotation90) {
        hwx = (MATRIX_WIDTH - 1) - y;
        hwy = x;
    } else { /* if (screenConfig.rotation == rotation270)*/
        hwx = y;
        hwy = (MATRIX_HEIGHT - 1) - x;
    }

    copyRgb24(drawBufferPtr[hwy][hwx], color);
}

#define SWAPint(X,Y) { \
        int temp = X ; \
        X = Y ; \
        Y = temp ; \
    }

// x0, x1, and y must be in bounds (0-localWidth/Height), x1 > x0
void SmartMatrix::drawHardwareHLine(uint8_t x0, uint8_t x1, uint8_t y, rgb24 color) {
    int i;

    for (i = x0; i <= x1; i++) {
        copyRgb24(drawBufferPtr[y][i], color);
    }
}

// x, y0, and y1 must be in bounds (0-localWidth/Height), y1 > y0
void SmartMatrix::drawHardwareVLine(uint8_t x, uint8_t y0, uint8_t y1, rgb24 color) {
    int i;

    for (i = y0; i <= y1; i++) {
        copyRgb24(drawBufferPtr[i][x], color);
    }
}

void SmartMatrix::drawFastHLine(int16_t x0, int16_t x1, int16_t y, rgb24 color) {
    // make sure line goes from x0 to x1
    if (x1 < x0)
        SWAPint(x1, x0);

    // check for completely out of bounds line
    if (x1 < 0 || x0 >= screenConfig.localWidth || y < 0 || y >= screenConfig.localHeight)
        return;

    // truncate if partially out of bounds
    if (x0 < 0)
        x0 = 0;

    if (x1 >= screenConfig.localWidth)
        x1 = screenConfig.localWidth - 1;

    // map to hardware drawline function
    if (screenConfig.rotation == rotation0) {
        drawHardwareHLine(x0, x1, y, color);
    } else if (screenConfig.rotation == rotation180) {
        drawHardwareHLine((MATRIX_WIDTH - 1) - x1, (MATRIX_WIDTH - 1) - x0, (MATRIX_HEIGHT - 1) - y, color);
    } else if (screenConfig.rotation == rotation90) {
        drawHardwareVLine((MATRIX_WIDTH - 1) - y, x0, x1, color);
    } else { /* if (screenConfig.rotation == rotation270)*/
        drawHardwareVLine(y, (MATRIX_HEIGHT - 1) - x1, (MATRIX_HEIGHT - 1) - x0, color);
    }
}

void SmartMatrix::drawFastVLine(int16_t x, int16_t y0, int16_t y1, rgb24 color) {
    // make sure line goes from y0 to y1
    if (y1 < y0)
        SWAPint(y1, y0);

    // check for completely out of bounds line
    if (y1 < 0 || y0 >= screenConfig.localHeight || x < 0 || x >= screenConfig.localWidth)
        return;

    // truncate if partially out of bounds
    if (y0 < 0)
        y0 = 0;

    if (y1 >= screenConfig.localHeight)
        y1 = screenConfig.localHeight - 1;

    // map to hardware drawline function
    if (screenConfig.rotation == rotation0) {
        drawHardwareVLine(x, y0, y1, color);
    } else if (screenConfig.rotation == rotation180) {
        drawHardwareVLine((MATRIX_WIDTH - 1) - x, (MATRIX_HEIGHT - 1) - y1, (MATRIX_HEIGHT - 1) - y0, color);
    } else if (screenConfig.rotation == rotation90) {
        drawHardwareHLine((MATRIX_WIDTH - 1) - y1, (MATRIX_WIDTH - 1) - y0, x, color);
    } else { /* if (screenConfig.rotation == rotation270)*/
        drawHardwareHLine(y0, y1, (MATRIX_HEIGHT - 1) - x, color);
    }
}

void SmartMatrix::bresteepline(int16_t x3, int16_t y3, int16_t x4, int16_t y4, rgb24 color) {
    // if point x3, y3 is on the right side of point x4, y4, change them
    if ((x3 - x4) > 0) {
        bresteepline(x4, y4, x3, y3, color);
        return;
    }

    int x = x3, y = y3, sum = x4 - x3,  Dx = 2 * (x4 - x3), Dy = abs(2 * (y4 - y3));
    int prirastokDy = ((y4 - y3) > 0) ? 1 : -1;

    for (int i = 0; i <= x4 - x3; i++) {
        drawPixel(y, x, color);
        x++;
        sum -= Dy;
        if (sum < 0) {
            y = y + prirastokDy;
            sum += Dx;
        }
    }
}

// algorithm from http://www.netgraphics.sk/bresenham-algorithm-for-a-line
void SmartMatrix::drawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, rgb24 color) {
    // if point x1, y1 is on the right side of point x2, y2, change them
    if ((x1 - x2) > 0) {
        drawLine(x2, y2, x1, y1, color);
        return;
    }
    // test inclination of line
    // function Math.abs(y) defines absolute value y
    if (abs(y2 - y1) > abs(x2 - x1)) {
        // line and y axis angle is less then 45 degrees
        // that swhy go on the next procedure
        bresteepline(y1, x1, y2, x2, color); return;
    }
    // line and x axis angle is less then 45 degrees, so x is guiding
    // auxiliary variables
    int x = x1, y = y1, sum = x2 - x1, Dx = 2 * (x2 - x1), Dy = abs(2 * (y2 - y1));
    int prirastokDy = ((y2 - y1) > 0) ? 1 : -1;
    // draw line
    for (int i = 0; i <= x2 - x1; i++) {
        drawPixel(x, y, color);
        x++;
        sum -= Dy;
        if (sum < 0) {
            y = y + prirastokDy;
            sum += Dx;
        }
    }
}

// algorithm from http://en.wikipedia.org/wiki/Midpoint_circle_algorithm
void SmartMatrix::drawCircle(int16_t x0, int16_t y0, uint16_t radius, rgb24 color)
{
    int a = radius, b = 0;
    int radiusError = 1 - a;

    if (radius == 0)
        return;

    while (a >= b)
    {
        drawPixel(a + x0, b + y0, color);
        drawPixel(b + x0, a + y0, color);
        drawPixel(-a + x0, b + y0, color);
        drawPixel(-b + x0, a + y0, color);
        drawPixel(-a + x0, -b + y0, color);
        drawPixel(-b + x0, -a + y0, color);
        drawPixel(a + x0, -b + y0, color);
        drawPixel(b + x0, -a + y0, color);

        b++;
        if (radiusError < 0)
            radiusError += 2 * b + 1;
        else
        {
            a--;
            radiusError += 2 * (b - a + 1);
        }
    }
}

// algorithm from drawCircle rearranged with hlines drawn between points on the raidus
void SmartMatrix::fillCircle(int16_t x0, int16_t y0, uint16_t radius, rgb24 outlineColor, rgb24 fillColor)
{
    int a = radius, b = 0;
    int radiusError = 1 - a;

    if (radius == 0)
        return;

    // only draw one line per row, skipping the top and bottom
    bool hlineDrawn = true;

    while (a >= b)
    {
        // this pair sweeps from horizontal center down
        drawPixel(a + x0, b + y0, outlineColor);
        drawPixel(-a + x0, b + y0, outlineColor);
        drawFastHLine((a - 1) + x0, (-a + 1) + x0, b + y0, fillColor);

        // this pair sweeps from bottom up
        drawPixel(b + x0, a + y0, outlineColor);
        drawPixel(-b + x0, a + y0, outlineColor);

        // this pair sweeps from horizontal center up
        drawPixel(-a + x0, -b + y0, outlineColor);
        drawPixel(a + x0, -b + y0, outlineColor);
        drawFastHLine((a - 1) + x0, (-a + 1) + x0, -b + y0, fillColor);

        // this pair sweeps from top down
        drawPixel(-b + x0, -a + y0, outlineColor);
        drawPixel(b + x0, -a + y0, outlineColor);

        if (b > 1 && !hlineDrawn) {
            drawFastHLine((b - 1) + x0, (-b + 1) + x0, a + y0, fillColor);
            drawFastHLine((b - 1) + x0, (-b + 1) + x0, -a + y0, fillColor);
            hlineDrawn = true;
        }

        b++;
        if (radiusError < 0) {
            radiusError += 2 * b + 1;
        } else {
            a--;
            hlineDrawn = false;
            radiusError += 2 * (b - a + 1);
        }
    }
}



// algorithm from drawCircle rearranged with hlines drawn between points on the raidus
void SmartMatrix::fillCircle(int16_t x0, int16_t y0, uint16_t radius, rgb24 fillColor)
{
    int a = radius, b = 0;
    int radiusError = 1 - a;

    if (radius == 0)
        return;

    // only draw one line per row, skipping the top and bottom
    bool hlineDrawn = true;

    while (a >= b)
    {
        // this pair sweeps from horizontal center down
        drawFastHLine((a - 1) + x0, (-a + 1) + x0, b + y0, fillColor);

        // this pair sweeps from horizontal center up
        drawFastHLine((a - 1) + x0, (-a + 1) + x0, -b + y0, fillColor);

        if (b > 1 && !hlineDrawn) {
            drawFastHLine((b - 1) + x0, (-b + 1) + x0, a + y0, fillColor);
            drawFastHLine((b - 1) + x0, (-b + 1) + x0, -a + y0, fillColor);
            hlineDrawn = true;
        }

        b++;
        if (radiusError < 0) {
            radiusError += 2 * b + 1;
        } else {
            a--;
            hlineDrawn = false;
            radiusError += 2 * (b - a + 1);
        }
    }
}

void SmartMatrix::fillRoundRectangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t radius, rgb24 fillColor) {
    fillRoundRectangle(x0, y0, x1, y1, radius, fillColor, fillColor);
}


void SmartMatrix::fillRoundRectangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t radius, rgb24 outlineColor, rgb24 fillColor) {
    if (x1 < x0)
        SWAPint(x1, x0);

    if (y1 < y0)
        SWAPint(y1, y0);

    // decrease large radius that would break shape
    if(radius > (x1-x0)/2)
        radius = (x1-x0)/2;
    if(radius > (y1-y0)/2)
        radius = (y1-y0)/2;

    int a = radius, b = 0;
    int radiusError = 1 - a;

    if (radius == 0) {
        fillRectangle(x0, y0, x1, y1, outlineColor, fillColor);
    }

    // draw straight part of outline
    drawFastHLine(x0 + radius, x1 - radius, y0, outlineColor);
    drawFastHLine(x0 + radius, x1 - radius, y1, outlineColor);
    drawFastVLine(x0, y0 + radius, y1 - radius, outlineColor);
    drawFastVLine(x1, y0 + radius, y1 - radius, outlineColor);

    // convert coordinates to point at center of rounded sections
    x0 += radius;
    x1 -= radius;
    y0 += radius;
    y1 -= radius;

    // only draw one line per row/column, skipping the sides
    bool hlineDrawn = true;
    bool vlineDrawn = true;

    while (a >= b)
    {
        // this pair sweeps from far left towards right
        drawPixel(-a + x0, -b + y0, outlineColor);
        drawPixel(-a + x0, b + y1, outlineColor);

        // this pair sweeps from far right towards left
        drawPixel(a + x1, -b + y0, outlineColor);
        drawPixel(a + x1, b + y1, outlineColor);

        if (!vlineDrawn) {
            drawFastVLine(-a + x0, (-b + 1) + y0, (b - 1) + y1, fillColor);
            drawFastVLine(a + x1, (-b + 1) + y0, (b - 1) + y1, fillColor);
            vlineDrawn = true;
        }

        // this pair sweeps from very top towards bottom
        drawPixel(-b + x0, -a + y0, outlineColor);
        drawPixel(b + x1, -a + y0, outlineColor);

        // this pair sweeps from bottom up
        drawPixel(-b + x0, a + y1, outlineColor);
        drawPixel(b + x1, a + y1, outlineColor);

        if (!hlineDrawn) {
            drawFastHLine((-b + 1) + x0, (b - 1) + x1, -a + y0, fillColor);
            drawFastHLine((-b + 1) + x0, (b - 1) + x1, a + y1, fillColor);
            hlineDrawn = true;
        }

        b++;
        if (radiusError < 0) {
            radiusError += 2 * b + 1;
        } else {
            a--;
            hlineDrawn = false;
            vlineDrawn = false;
            radiusError += 2 * (b - a + 1);
        }
    }

    // draw rectangle in center
    fillRectangle(x0 - a, y0 - a, x1 + a, y1 + a, fillColor);
}

void SmartMatrix::drawRoundRectangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t radius, rgb24 outlineColor) {
    if (x1 < x0)
        SWAPint(x1, x0);

    if (y1 < y0)
        SWAPint(y1, y0);

    // decrease large radius that would break shape
    if(radius > (x1-x0)/2)
        radius = (x1-x0)/2;
    if(radius > (y1-y0)/2)
        radius = (y1-y0)/2;

    int a = radius, b = 0;
    int radiusError = 1 - a;

    // draw straight part of outline
    drawFastHLine(x0 + radius, x1 - radius, y0, outlineColor);
    drawFastHLine(x0 + radius, x1 - radius, y1, outlineColor);
    drawFastVLine(x0, y0 + radius, y1 - radius, outlineColor);
    drawFastVLine(x1, y0 + radius, y1 - radius, outlineColor);

    // convert coordinates to point at center of rounded sections
    x0 += radius;
    x1 -= radius;
    y0 += radius;
    y1 -= radius;

    while (a >= b)
    {
        // this pair sweeps from far left towards right
        drawPixel(-a + x0, -b + y0, outlineColor);
        drawPixel(-a + x0, b + y1, outlineColor);

        // this pair sweeps from far right towards left
        drawPixel(a + x1, -b + y0, outlineColor);
        drawPixel(a + x1, b + y1, outlineColor);

        // this pair sweeps from very top towards bottom
        drawPixel(-b + x0, -a + y0, outlineColor);
        drawPixel(b + x1, -a + y0, outlineColor);

        // this pair sweeps from bottom up
        drawPixel(-b + x0, a + y1, outlineColor);
        drawPixel(b + x1, a + y1, outlineColor);

        b++;
        if (radiusError < 0) {
            radiusError += 2 * b + 1;
        } else {
            a--;
            radiusError += 2 * (b - a + 1);
        }
    }
}


// Code from http://www.sunshine2k.de/coding/java/TriangleRasterization/TriangleRasterization.html
void SmartMatrix::fillFlatSideTriangleInt(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3, rgb24 color) {
    int16_t t1x, t2x, t1y, t2y;
    bool changed1 = false;
    bool changed2 = false;
    int8_t signx1, signx2, signy1, signy2, dx1, dy1, dx2, dy2;
    int i;
    int8_t e1, e2;

    t1x = t2x = x1; t1y = t2y = y1; // Starting points

    dx1 = abs(x2 - x1);
    dy1 = abs(y2 - y1);
    dx2 = abs(x3 - x1);
    dy2 = abs(y3 - y1);

    if (x2 - x1 < 0) {
        signx1 = -1;
    } else signx1 = 1;
    if (x3 - x1 < 0) {
        signx2 = -1;
    } else signx2 = 1;
    if (y2 - y1 < 0) {
        signy1 = -1;
    } else signy1 = 1;
    if (y3 - y1 < 0) {
        signy2 = -1;
    } else signy2 = 1;

    if (dy1 > dx1) {   // swap values
        SWAPint(dx1, dy1);
        changed1 = true;
    }
    if (dy2 > dx2) {   // swap values
        SWAPint(dy2, dx2);
        changed2 = true;
    }

    e1 = 2 * dy1 - dx1;
    e2 = 2 * dy2 - dx2;

    for (i = 0; i <= dx1; i++)
    {
        drawFastHLine(t1x, t2x, t1y, color);

        while (e1 >= 0)
        {
            if (changed1)
                t1x += signx1;
            else
                t1y += signy1;
            e1 = e1 - 2 * dx1;
        }

        if (changed1)
            t1y += signy1;
        else
            t1x += signx1;

        e1 = e1 + 2 * dy1;

        /* here we rendered the next point on line 1 so follow now line 2
         * until we are on the same y-value as line 1.
         */
        while (t2y != t1y)
        {
            while (e2 >= 0)
            {
                if (changed2)
                    t2x += signx2;
                else
                    t2y += signy2;
                e2 = e2 - 2 * dx2;
            }

            if (changed2)
                t2y += signy2;
            else
                t2x += signx2;

            e2 = e2 + 2 * dy2;
        }
    }
}

// Code from http://www.sunshine2k.de/coding/java/TriangleRasterization/TriangleRasterization.html
void SmartMatrix::fillTriangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3, rgb24 fillColor) {
    // Sort vertices
    if (y1 > y2) {
        SWAPint(y1, y2);
        SWAPint(x1, x2);
    }
    if (y1 > y3) {
        SWAPint(y1, y3);
        SWAPint(x1, x3);
    }
    if (y2 > y3) {
        SWAPint(y2, y3);
        SWAPint(x2, x3);
    }

    if (y2 == y3)
    {
        fillFlatSideTriangleInt(x1, y1, x2, y2, x3, y3, fillColor);
    }
    /* check for trivial case of top-flat triangle */
    else if (y1 == y2)
    {
        fillFlatSideTriangleInt(x3, y3, x1, y1, x2, y2, fillColor);
    }
    else
    {
        /* general case - split the triangle in a topflat and bottom-flat one */
        int16_t xtmp, ytmp;
        xtmp = (int)(x1 + ((float)(y2 - y1) / (float)(y3 - y1)) * (x3 - x1));
        ytmp = y2;
        fillFlatSideTriangleInt(x1, y1, x2, y2, xtmp, ytmp, fillColor);
        fillFlatSideTriangleInt(x3, y3, x2, y2, xtmp, ytmp, fillColor);
    }
}

void SmartMatrix::fillTriangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3, rgb24 outlineColor, rgb24 fillColor) {
    fillTriangle(x1, y1, x2, y2, x3, y3, fillColor);
    drawTriangle(x1, y1, x2, y2, x3, y3, outlineColor);
}

void SmartMatrix::drawTriangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3, rgb24 color) {
    drawLine(x1, y1, x2, y2, color);
    drawLine(x2, y2, x3, y3, color);
    drawLine(x1, y1, x3, y3, color);
}

void SmartMatrix::drawRectangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, rgb24 color) {
    drawFastHLine(x0, x1, y0, color);
    drawFastHLine(x0, x1, y1, color);
    drawFastVLine(x0, y0, y1, color);
    drawFastVLine(x1, y0, y1, color);
}

void SmartMatrix::fillRectangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, rgb24 color) {
    int i;
    for (i = y0; i <= y1; i++) {
        drawFastHLine(x0, x1, i, color);
    }
}

void SmartMatrix::fillScreen(rgb24 color) {
    fillRectangle(0, 0, screenConfig.localWidth, screenConfig.localHeight, color);
}

void SmartMatrix::fillRectangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, rgb24 outlineColor, rgb24 fillColor) {
    fillRectangle(x0, y0, x1, y1, fillColor);
    drawRectangle(x0, y0, x1, y1, outlineColor);
}

bool getBitmapPixelAtXY(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t *bitmap) {
    int cell = (y * ((width / 8) + 1)) + (x / 8);

    uint8_t mask = 0x80 >> (x % 8);
    return (mask & bitmap[cell]);
}

void SmartMatrix::setFont(fontChoices newFont) {
    font = (bitmap_font *)fontLookup(newFont);
}

void SmartMatrix::drawChar(int16_t x, int16_t y, rgb24 charColor, char character) {
    int xcnt, ycnt;

    for (ycnt = 0; ycnt < font->Height; ycnt++) {
        for (xcnt = 0; xcnt < font->Width; xcnt++) {
            if (getBitmapFontPixelAtXY(character, xcnt, ycnt, font)) {
                drawPixel(x + xcnt, y + ycnt, charColor);
            }
        }
    }
}

void SmartMatrix::drawString(int16_t x, int16_t y, rgb24 charColor, const char text[]) {
    int xcnt, ycnt, i = 0, offset = 0;
    char character;

    // limit text to 10 chars, why?
    for (i = 0; i < 10; i++) {
        character = text[offset++];
        if (character == '\0')
            return;

        for (ycnt = 0; ycnt < font->Height; ycnt++) {
            for (xcnt = 0; xcnt < font->Width; xcnt++) {
                if (getBitmapFontPixelAtXY(character, xcnt, ycnt, font)) {
                    drawPixel(x + xcnt, y + ycnt, charColor);
                }
            }
        }
        x += font->Width;
    }
}

void SmartMatrix::drawMonoBitmap(int16_t x, int16_t y, uint8_t width, uint8_t height, rgb24 bitmapColor, uint8_t *bitmap) {
    int xcnt, ycnt;

    for (ycnt = 0; ycnt < height; ycnt++) {
        for (xcnt = 0; xcnt < width; xcnt++) {
            if (getBitmapPixelAtXY(xcnt, ycnt, width, height, bitmap)) {
                drawPixel(x + xcnt, y + ycnt, bitmapColor);
            }
        }
    }
}

int newFramesToInterpolate;

void SmartMatrix::handleBufferSwap(void) {
    if(framesInterpolated < totalFramesToInterpolate)
        framesInterpolated++;

    if (!swapPending)
        return;

    framesInterpolated = 0;
    totalFramesToInterpolate = newFramesToInterpolate;

    unsigned char newDrawBuffer = previousRefreshBuffer;

    previousRefreshBuffer = currentRefreshBuffer;
    currentRefreshBuffer = drawBuffer;
    drawBuffer = newDrawBuffer;

    currentRefreshBufferPtr = backgroundBuffer[currentRefreshBuffer];
    previousRefreshBufferPtr = backgroundBuffer[previousRefreshBuffer];
    drawBufferPtr = backgroundBuffer[drawBuffer];

    swapPending = false;
}

// waits until previous swap is complete
// waits until current swap is complete if copy is enabled
void SmartMatrix::swapBuffers(bool copy) {
    while (swapPending);

    swapPending = true;

    if (copy) {
        while (swapPending);
        memcpy(drawBufferPtr, currentRefreshBufferPtr, sizeof(backgroundBuffer[0]));
    }
}

// waits until previous swap is complete
void SmartMatrix::swapBuffersWithInterpolation(int framesToInterpolate) {
    while (swapPending);

    newFramesToInterpolate = framesToInterpolate;

    swapPending = true;
}

// waits until previous swap is complete
void SmartMatrix::swapBuffersWithCopyAndInterpolation(int framesToInterpolate) {
    while (swapPending);

    newFramesToInterpolate = framesToInterpolate;

    swapPending = true;
    while (swapPending);
    memcpy(drawBufferPtr, currentRefreshBufferPtr, sizeof(backgroundBuffer[0]));
}


// return pointer to start of drawBuffer, so application can do efficient loading of bitmaps
rgb24 *SmartMatrix::backBuffer() {
    return drawBufferPtr[0];
}

void SmartMatrix::setBackBuffer(rgb24 *newBuffer) {
  drawBufferPtr = (rgb24 (*)[MATRIX_WIDTH])newBuffer;
}

rgb24 *SmartMatrix::getRealBackBuffer() {
  return &backgroundBuffer[drawBuffer][0][0];
}
