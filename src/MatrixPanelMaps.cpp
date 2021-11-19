/*
 * SmartMatrix Library - Matrix Panel Map Definitions
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

#include "MatrixCommonHub75.h"
#include "MatrixPanelMaps.h"

// use this for all linear panels (e.g. panels that draw a single left-to-right line for each RGB channel)
const PanelMappingEntry defaultPanelMap[] =
{
    {0, 0, DEFAULT_PANEL_WIDTH_FOR_LINEAR_PANELS},
    {0, 0, 0}   // last entry is all zeros
};

const PanelMappingEntry panelMap32x16Mod2[] =
{
    {0, 71,  -8},
    {0, 87,  -8},
    {0, 103, -8},
    {0, 119, -8},
    {2, 72,   8},
    {2, 88,   8},
    {2, 104,  8},
    {2, 120,  8},
    {4,  7,  -8},
    {4, 23,  -8},
    {4, 39,  -8},
    {4, 55,  -8},
    {6,  8,   8},
    {6, 24,   8},
    {6, 40,   8},
    {6, 56,   8},
    {0, 0, 0}   // last entry is all zeros
};

const PanelMappingEntry panelMapHub12_32x16Mod4[] =
{
    {0,  24,  8},
    {0,  56,  8},
    {0,  88,  8},
    {0,  120, 8},
    {4,  16,  8},
    {4,  48,  8},
    {4,  80,  8},
    {4,  112, 8}, 
    {8,  8,   8},
    {8,  40,  8},
    {8,  72,  8},
    {8,  104, 8},
    {12, 0,   8},
    {12, 32,  8},
    {12, 64,  8},
    {12, 96,  8},
    {0, 0, 0}   // last entry is all zeros
};

const PanelMappingEntry panelMap32x16Mod4[] =
{
    {0, 0,  8},
    {0, 16, 8},
    {0, 32, 8},
    {0, 48, 8},
    {4, 8,  8},
    {4, 24, 8},
    {4, 40, 8},
    {4, 56, 8}, 
    {0, 0, 0}   // last entry is all zeros
};

const PanelMappingEntry panelMap32x16Mod4V2[] =
{
    {0, 15, -8},
    {0, 31, -8},
    {0, 47, -8},
    {0, 63, -8},
    {4, 0,   8},
    {4, 16,  8},
    {4, 32,  8},
    {4, 48,  8}, 
    {0, 0, 0}   // last entry is all zeros
};

const PanelMappingEntry panelMap32x16Mod4V3[] =
{
    {0, 8, 8},
    {0, 24, 8},
    {0, 40, 8},
    {0, 56, 8},
    {4, 0, 8},
    {4, 16, 8},
    {4, 32, 8},
    {4, 48, 8},
    {0, 0, 0} // last entry is all zeros
};

const PanelMappingEntry panelMap64x32Mod8[] =
{
    {0, 64, 64},
    {8, 0, 64},
    {0, 0, 0}   // last entry is all zeros
};

const PanelMappingEntry panelMap64x64Mod16[] =
{
{0,64,64},
{16,0,64},
{0,0,0}
};

// Applied patch from https://community.pixelmatix.com/t/mapping-assistance-32x16-p10/889/23 not fully integrated (ESP32 only)
const PanelMappingEntry panelMap32x16Mod4V4[] =
{
    {0, 7, -8},
    {0, 23, -8},
    {0, 39, -8},
    {0, 55, -8},
    {4, 8,   8},
    {4, 24,  8},
    {4, 40,  8},
    {4, 56,  8}, 
    {0, 0, 0}   // last entry is all zeros
};

const PanelMappingEntry panelMap32x16Mod2V2[] =
{
    {0, 31, -8},
    {0, 63, -8},
    {0, 95, -8},
    {0, 127, -8},
    {2, 16, 8},
    {2, 48, 8},
    {2, 80, 8},
    {2, 112, 8},
    {4, 15, -8},
    {4, 47, -8},
    {4, 79, -8},
    {4, 111, -8},
    {6, 0, 8},
    {6, 32, 8},
    {6, 64, 8},
    {6, 96, 8},
    {0, 0, 0} // last entry is all zeros
};

const PanelMappingEntry * getMultiRowRefreshPanelMap(unsigned char panelType) {
    switch(panelType) {
        case SMARTMATRIX_HUB75_64ROW_64COL_MOD16SCAN:
            return panelMap64x64Mod16;
        case SMARTMATRIX_HUB75_32ROW_64COL_MOD8SCAN:
            return panelMap64x32Mod8;
        case SMARTMATRIX_HUB75_16ROW_32COL_MOD2SCAN:
            return panelMap32x16Mod2;
        case SMARTMATRIX_HUB12_16ROW_32COL_MOD4SCAN:
            return panelMapHub12_32x16Mod4;
        case SMARTMATRIX_HUB75_16ROW_32COL_MOD4SCAN:
            return panelMap32x16Mod4;
        case SMARTMATRIX_HUB75_16ROW_32COL_MOD4SCAN_V2:
            return panelMap32x16Mod4V2;
        case SMARTMATRIX_HUB75_16ROW_32COL_MOD4SCAN_V3:
            return panelMap32x16Mod4V3;
        case SMARTMATRIX_HUB75_16ROW_32COL_MOD4SCAN_V4:
            return panelMap32x16Mod4V4;
        case SMARTMATRIX_HUB75_16ROW_32COL_MOD2SCAN_V2:
            return panelMap32x16Mod2V2;
        default:
            return defaultPanelMap;            
    }
}
