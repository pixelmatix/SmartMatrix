/*
 * SmartMatrix Library - Common Definitions for Driving Multiplexed Panels
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

#ifndef SmartMatrixMultiplexedCommon_h
#define SmartMatrixMultiplexedCommon_h

#define DEFAULT_PANEL_WIDTH_FOR_LINEAR_PANELS       32
#define HUB75_RGB_COLOR_CHANNELS_IN_PARALLEL        2

#define SMARTMATRIX_HUB75_32ROW_MOD16SCAN           0
#define SMARTMATRIX_HUB75_16ROW_MOD8SCAN            1
#define SMARTMATRIX_HUB75_64ROW_MOD32SCAN           2
#define SMARTMATRIX_HUB75_4ROW_MOD2SCAN             3
#define SMARTMATRIX_HUB75_8ROW_MOD4SCAN             4
#define SMARTMATRIX_HUB75_16ROW_32COL_MOD2SCAN      5
#define SMARTMATRIX_HUB12_16ROW_32COL_MOD4SCAN      6   // HUB12 panels have limited support, the height needs to be doubled to account for the fact they only use one RGB channel, though SmartMatrix Library will fill the second channel
#define SMARTMATRIX_HUB75_16ROW_32COL_MOD4SCAN      7
#define SMARTMATRIX_HUB75_2ROW_MOD1SCAN             8
#define SMARTMATRIX_HUB75_16ROW_32COL_MOD4SCAN_V2   9

#define CONVERT_PANELTYPE_TO_MATRIXPANELHEIGHT(x)   ((x == SMARTMATRIX_HUB75_32ROW_MOD16SCAN ? 32 : 0) | \
                                                    (x == SMARTMATRIX_HUB75_16ROW_MOD8SCAN ? 16 : 0) | \
                                                    (x == SMARTMATRIX_HUB75_16ROW_32COL_MOD2SCAN ? 16 : 0) | \
                                                    (x == SMARTMATRIX_HUB75_4ROW_MOD2SCAN ? 4 : 0) | \
                                                    (x == SMARTMATRIX_HUB75_8ROW_MOD4SCAN ? 8 : 0) | \
                                                    (x == SMARTMATRIX_HUB12_16ROW_32COL_MOD4SCAN ? 16*2 : 0) | \
                                                    (x == SMARTMATRIX_HUB75_16ROW_32COL_MOD4SCAN ? 16 : 0) | \
                                                    (x == SMARTMATRIX_HUB75_16ROW_32COL_MOD4SCAN_V2 ? 16 : 0) | \
                                                    (x == SMARTMATRIX_HUB75_2ROW_MOD1SCAN ? 2 : 0) | \
                                                    (x == SMARTMATRIX_HUB75_64ROW_MOD32SCAN ? 64 : 0))

#define CONVERT_PANELTYPE_TO_MATRIXROWPAIROFFSET(x) ((x == SMARTMATRIX_HUB75_32ROW_MOD16SCAN ? 16 : 0) | \
                                                    (x == SMARTMATRIX_HUB75_16ROW_MOD8SCAN ? 8 : 0) | \
                                                    (x == SMARTMATRIX_HUB75_16ROW_32COL_MOD2SCAN ? 8 : 0) | \
                                                    (x == SMARTMATRIX_HUB75_4ROW_MOD2SCAN ? 2 : 0) | \
                                                    (x == SMARTMATRIX_HUB75_8ROW_MOD4SCAN ? 4 : 0) | \
                                                    (x == SMARTMATRIX_HUB12_16ROW_32COL_MOD4SCAN ? 8*2 : 0) | \
                                                    (x == SMARTMATRIX_HUB75_16ROW_32COL_MOD4SCAN ? 8 : 0) | \
                                                    (x == SMARTMATRIX_HUB75_16ROW_32COL_MOD4SCAN_V2 ? 8 : 0) | \
                                                    (x == SMARTMATRIX_HUB75_2ROW_MOD1SCAN ? 1 : 0) | \
                                                    (x == SMARTMATRIX_HUB75_64ROW_MOD32SCAN ? 32 : 0))

#define CONVERT_PANELTYPE_TO_MATRIXSCANMOD(x)  ((x == SMARTMATRIX_HUB75_32ROW_MOD16SCAN ? 16 : 0) | \
                                                    (x == SMARTMATRIX_HUB75_16ROW_MOD8SCAN ? 8 : 0) | \
                                                    (x == SMARTMATRIX_HUB75_16ROW_32COL_MOD2SCAN ? 2 : 0) | \
                                                    (x == SMARTMATRIX_HUB75_4ROW_MOD2SCAN ? 2 : 0) | \
                                                    (x == SMARTMATRIX_HUB75_8ROW_MOD4SCAN ? 4 : 0) | \
                                                    (x == SMARTMATRIX_HUB12_16ROW_32COL_MOD4SCAN ? 4 : 0) | \
                                                    (x == SMARTMATRIX_HUB75_16ROW_32COL_MOD4SCAN ? 4 : 0) | \
                                                    (x == SMARTMATRIX_HUB75_16ROW_32COL_MOD4SCAN_V2 ? 4 : 0) | \
                                                    (x == SMARTMATRIX_HUB75_2ROW_MOD1SCAN ? 1 : 0) | \
                                                    (x == SMARTMATRIX_HUB75_64ROW_MOD32SCAN ? 32 : 0))

#define CONVERT_PANELTYPE_TO_MATRIXPANELWIDTH(x)  ((x == SMARTMATRIX_HUB75_32ROW_MOD16SCAN ? DEFAULT_PANEL_WIDTH_FOR_LINEAR_PANELS : 0) | \
                                                    (x == SMARTMATRIX_HUB75_16ROW_MOD8SCAN ? DEFAULT_PANEL_WIDTH_FOR_LINEAR_PANELS : 0) | \
                                                    (x == SMARTMATRIX_HUB75_16ROW_32COL_MOD2SCAN ? 32 : 0) | \
                                                    (x == SMARTMATRIX_HUB75_4ROW_MOD2SCAN ? DEFAULT_PANEL_WIDTH_FOR_LINEAR_PANELS : 0) | \
                                                    (x == SMARTMATRIX_HUB75_8ROW_MOD4SCAN ? DEFAULT_PANEL_WIDTH_FOR_LINEAR_PANELS : 0) | \
                                                    (x == SMARTMATRIX_HUB12_16ROW_32COL_MOD4SCAN ? 32 : 0) | \
                                                    (x == SMARTMATRIX_HUB75_16ROW_32COL_MOD4SCAN ? 32 : 0) | \
                                                    (x == SMARTMATRIX_HUB75_16ROW_32COL_MOD4SCAN_V2 ? 32 : 0) | \
                                                    (x == SMARTMATRIX_HUB75_2ROW_MOD1SCAN ? DEFAULT_PANEL_WIDTH_FOR_LINEAR_PANELS : 0) | \
                                                    (x == SMARTMATRIX_HUB75_64ROW_MOD32SCAN ? DEFAULT_PANEL_WIDTH_FOR_LINEAR_PANELS : 0))

#define MATRIX_PANEL_HEIGHT (CONVERT_PANELTYPE_TO_MATRIXPANELHEIGHT(panelType))
#define MATRIX_STACK_HEIGHT (matrixHeight / MATRIX_PANEL_HEIGHT)

#define COLOR_CHANNELS_PER_PIXEL        3
#define LATCHES_PER_ROW (refreshDepth/COLOR_CHANNELS_PER_PIXEL)
#define COLOR_DEPTH_BITS (refreshDepth/COLOR_CHANNELS_PER_PIXEL)
#define MATRIX_SCAN_MOD (CONVERT_PANELTYPE_TO_MATRIXSCANMOD(panelType))
#define COLS_PER_PANEL (CONVERT_PANELTYPE_TO_MATRIXPANELWIDTH(panelType))
#define PHYSICAL_ROWS_PER_REFRESH_ROW (MATRIX_PANEL_HEIGHT / MATRIX_SCAN_MOD / HUB75_RGB_COLOR_CHANNELS_IN_PARALLEL)
#define ROW_PAIR_OFFSET (CONVERT_PANELTYPE_TO_MATRIXROWPAIROFFSET(panelType))

#define PIXELS_PER_LATCH    ((matrixWidth * matrixHeight) / MATRIX_PANEL_HEIGHT * PHYSICAL_ROWS_PER_REFRESH_ROW)

#define SMARTMATRIX_OPTIONS_NONE                    0
#define SMARTMATRIX_OPTIONS_C_SHAPE_STACKING        (1 << 0)
#define SMARTMATRIX_OPTIONS_BOTTOM_TO_TOP_STACKING  (1 << 1)
#define SMARTMATRIX_OPTIONS_HUB12_MODE              (1 << 2)
#define SMARTMATRIX_OPTIONS_MATRIXCALC_LOWPRIORITY  (1 << 3)
#define SMARTMATRIX_OPTIONS_ESP32_INVERT_CLK        (1 << 4)
#define SMARTMATRIX_OPTIONS_ESP32_CALC_TASK_CORE_1  (1 << 5)
#define SMARTMATRIX_OPTIONS_FM6126A_RESET_AT_START  (1 << 6)

// defines data bit order from bit 0-7, four times to fit in uint32_t
#define PACKED_HUB75_WORD_ORDER p0r1:1, p0g1:1, p0b1:1, p0r2:1, p0g2:1, p0b2:1, p1r1:1, p1g1:1, \
    p1b1:1, p1r2:1, p1g2:1, p1b2:1, p2r1:1, p2g1:1, p2b1:1, p2r2:1, \
    p2g2:1, p2b2:1, p3r1:1, p3g1:1, p3b1:1, p3r2:1, p3g2:1, p3b2:1 
    
#endif
