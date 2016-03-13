/*
 * SmartMatrix Library - Methods for accessing bitmap fonts
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

#ifndef MATRIXFONTCOMMON_H_
#define MATRIXFONTCOMMON_H_

#include <stdbool.h>
#include <stdint.h>

	/// bitmap font structure
typedef struct bitmap_font {
	unsigned char Width;		///< max. character width
	unsigned char Height;		///< character height
	unsigned short Chars;		///< number of characters in font
	const unsigned char *Widths;	///< width of each character
	const unsigned short *Index;	///< encoding to character index
	const unsigned char *Bitmap;	///< bitmap of all characters
} bitmap_font;


extern const bitmap_font apple3x5;
extern const bitmap_font apple5x7;
extern const bitmap_font apple6x10;
extern const bitmap_font apple8x13;
extern const bitmap_font gohufont6x11;
extern const bitmap_font gohufont6x11b;

typedef enum fontChoices {
    font3x5,
    font5x7,
    font6x10,
    font8x13,
    gohufont11,
    gohufont11b
} fontChoices;

bool getBitmapFontPixelAtXY(unsigned char letter, unsigned char x, unsigned char y, const bitmap_font *font);
const bitmap_font *fontLookup(fontChoices font);
uint16_t getBitmapFontRowAtXY(unsigned char letter, unsigned char y, const bitmap_font *font);

/// @{ defines to have human readable font files
#define ________ 0x00
#define _______X 0x01
#define ______X_ 0x02
#define ______XX 0x03
#define _____X__ 0x04
#define _____X_X 0x05
#define _____XX_ 0x06
#define _____XXX 0x07
#define ____X___ 0x08
#define ____X__X 0x09
#define ____X_X_ 0x0A
#define ____X_XX 0x0B
#define ____XX__ 0x0C
#define ____XX_X 0x0D
#define ____XXX_ 0x0E
#define ____XXXX 0x0F
#define ___X____ 0x10
#define ___X___X 0x11
#define ___X__X_ 0x12
#define ___X__XX 0x13
#define ___X_X__ 0x14
#define ___X_X_X 0x15
#define ___X_XX_ 0x16
#define ___X_XXX 0x17
#define ___XX___ 0x18
#define ___XX__X 0x19
#define ___XX_X_ 0x1A
#define ___XX_XX 0x1B
#define ___XXX__ 0x1C
#define ___XXX_X 0x1D
#define ___XXXX_ 0x1E
#define ___XXXXX 0x1F
#define __X_____ 0x20
#define __X____X 0x21
#define __X___X_ 0x22
#define __X___XX 0x23
#define __X__X__ 0x24
#define __X__X_X 0x25
#define __X__XX_ 0x26
#define __X__XXX 0x27
#define __X_X___ 0x28
#define __X_X__X 0x29
#define __X_X_X_ 0x2A
#define __X_X_XX 0x2B
#define __X_XX__ 0x2C
#define __X_XX_X 0x2D
#define __X_XXX_ 0x2E
#define __X_XXXX 0x2F
#define __XX____ 0x30
#define __XX___X 0x31
#define __XX__X_ 0x32
#define __XX__XX 0x33
#define __XX_X__ 0x34
#define __XX_X_X 0x35
#define __XX_XX_ 0x36
#define __XX_XXX 0x37
#define __XXX___ 0x38
#define __XXX__X 0x39
#define __XXX_X_ 0x3A
#define __XXX_XX 0x3B
#define __XXXX__ 0x3C
#define __XXXX_X 0x3D
#define __XXXXX_ 0x3E
#define __XXXXXX 0x3F
#define _X______ 0x40
#define _X_____X 0x41
#define _X____X_ 0x42
#define _X____XX 0x43
#define _X___X__ 0x44
#define _X___X_X 0x45
#define _X___XX_ 0x46
#define _X___XXX 0x47
#define _X__X___ 0x48
#define _X__X__X 0x49
#define _X__X_X_ 0x4A
#define _X__X_XX 0x4B
#define _X__XX__ 0x4C
#define _X__XX_X 0x4D
#define _X__XXX_ 0x4E
#define _X__XXXX 0x4F
#define _X_X____ 0x50
#define _X_X___X 0x51
#define _X_X__X_ 0x52
#define _X_X__XX 0x53
#define _X_X_X__ 0x54
#define _X_X_X_X 0x55
#define _X_X_XX_ 0x56
#define _X_X_XXX 0x57
#define _X_XX___ 0x58
#define _X_XX__X 0x59
#define _X_XX_X_ 0x5A
#define _X_XX_XX 0x5B
#define _X_XXX__ 0x5C
#define _X_XXX_X 0x5D
#define _X_XXXX_ 0x5E
#define _X_XXXXX 0x5F
#define _XX_____ 0x60
#define _XX____X 0x61
#define _XX___X_ 0x62
#define _XX___XX 0x63
#define _XX__X__ 0x64
#define _XX__X_X 0x65
#define _XX__XX_ 0x66
#define _XX__XXX 0x67
#define _XX_X___ 0x68
#define _XX_X__X 0x69
#define _XX_X_X_ 0x6A
#define _XX_X_XX 0x6B
#define _XX_XX__ 0x6C
#define _XX_XX_X 0x6D
#define _XX_XXX_ 0x6E
#define _XX_XXXX 0x6F
#define _XXX____ 0x70
#define _XXX___X 0x71
#define _XXX__X_ 0x72
#define _XXX__XX 0x73
#define _XXX_X__ 0x74
#define _XXX_X_X 0x75
#define _XXX_XX_ 0x76
#define _XXX_XXX 0x77
#define _XXXX___ 0x78
#define _XXXX__X 0x79
#define _XXXX_X_ 0x7A
#define _XXXX_XX 0x7B
#define _XXXXX__ 0x7C
#define _XXXXX_X 0x7D
#define _XXXXXX_ 0x7E
#define _XXXXXXX 0x7F
#define X_______ 0x80
#define X______X 0x81
#define X_____X_ 0x82
#define X_____XX 0x83
#define X____X__ 0x84
#define X____X_X 0x85
#define X____XX_ 0x86
#define X____XXX 0x87
#define X___X___ 0x88
#define X___X__X 0x89
#define X___X_X_ 0x8A
#define X___X_XX 0x8B
#define X___XX__ 0x8C
#define X___XX_X 0x8D
#define X___XXX_ 0x8E
#define X___XXXX 0x8F
#define X__X____ 0x90
#define X__X___X 0x91
#define X__X__X_ 0x92
#define X__X__XX 0x93
#define X__X_X__ 0x94
#define X__X_X_X 0x95
#define X__X_XX_ 0x96
#define X__X_XXX 0x97
#define X__XX___ 0x98
#define X__XX__X 0x99
#define X__XX_X_ 0x9A
#define X__XX_XX 0x9B
#define X__XXX__ 0x9C
#define X__XXX_X 0x9D
#define X__XXXX_ 0x9E
#define X__XXXXX 0x9F
#define X_X_____ 0xA0
#define X_X____X 0xA1
#define X_X___X_ 0xA2
#define X_X___XX 0xA3
#define X_X__X__ 0xA4
#define X_X__X_X 0xA5
#define X_X__XX_ 0xA6
#define X_X__XXX 0xA7
#define X_X_X___ 0xA8
#define X_X_X__X 0xA9
#define X_X_X_X_ 0xAA
#define X_X_X_XX 0xAB
#define X_X_XX__ 0xAC
#define X_X_XX_X 0xAD
#define X_X_XXX_ 0xAE
#define X_X_XXXX 0xAF
#define X_XX____ 0xB0
#define X_XX___X 0xB1
#define X_XX__X_ 0xB2
#define X_XX__XX 0xB3
#define X_XX_X__ 0xB4
#define X_XX_X_X 0xB5
#define X_XX_XX_ 0xB6
#define X_XX_XXX 0xB7
#define X_XXX___ 0xB8
#define X_XXX__X 0xB9
#define X_XXX_X_ 0xBA
#define X_XXX_XX 0xBB
#define X_XXXX__ 0xBC
#define X_XXXX_X 0xBD
#define X_XXXXX_ 0xBE
#define X_XXXXXX 0xBF
#define XX______ 0xC0
#define XX_____X 0xC1
#define XX____X_ 0xC2
#define XX____XX 0xC3
#define XX___X__ 0xC4
#define XX___X_X 0xC5
#define XX___XX_ 0xC6
#define XX___XXX 0xC7
#define XX__X___ 0xC8
#define XX__X__X 0xC9
#define XX__X_X_ 0xCA
#define XX__X_XX 0xCB
#define XX__XX__ 0xCC
#define XX__XX_X 0xCD
#define XX__XXX_ 0xCE
#define XX__XXXX 0xCF
#define XX_X____ 0xD0
#define XX_X___X 0xD1
#define XX_X__X_ 0xD2
#define XX_X__XX 0xD3
#define XX_X_X__ 0xD4
#define XX_X_X_X 0xD5
#define XX_X_XX_ 0xD6
#define XX_X_XXX 0xD7
#define XX_XX___ 0xD8
#define XX_XX__X 0xD9
#define XX_XX_X_ 0xDA
#define XX_XX_XX 0xDB
#define XX_XXX__ 0xDC
#define XX_XXX_X 0xDD
#define XX_XXXX_ 0xDE
#define XX_XXXXX 0xDF
#define XXX_____ 0xE0
#define XXX____X 0xE1
#define XXX___X_ 0xE2
#define XXX___XX 0xE3
#define XXX__X__ 0xE4
#define XXX__X_X 0xE5
#define XXX__XX_ 0xE6
#define XXX__XXX 0xE7
#define XXX_X___ 0xE8
#define XXX_X__X 0xE9
#define XXX_X_X_ 0xEA
#define XXX_X_XX 0xEB
#define XXX_XX__ 0xEC
#define XXX_XX_X 0xED
#define XXX_XXX_ 0xEE
#define XXX_XXXX 0xEF
#define XXXX____ 0xF0
#define XXXX___X 0xF1
#define XXXX__X_ 0xF2
#define XXXX__XX 0xF3
#define XXXX_X__ 0xF4
#define XXXX_X_X 0xF5
#define XXXX_XX_ 0xF6
#define XXXX_XXX 0xF7
#define XXXXX___ 0xF8
#define XXXXX__X 0xF9
#define XXXXX_X_ 0xFA
#define XXXXX_XX 0xFB
#define XXXXXX__ 0xFC
#define XXXXXX_X 0xFD
#define XXXXXXX_ 0xFE
#define XXXXXXXX 0xFF
	/// @}

#endif /* MATRIXFONTCOMMON_H_ */
