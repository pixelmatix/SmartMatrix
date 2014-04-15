// Created from bdf2c Version 3, (c) 2009, 2010 by Lutz Sammer
//	License AGPLv3: GNU Affero General Public License version 3

// original public domain font source:
// http://www.opensource.apple.com/source/X11fonts/X11fonts-14/font-misc-misc/font-misc-misc-1.1.2/5x7.bdf

#include "MatrixFontCommon.h"

	/// character bitmap for each encoding
static const unsigned char __apple5x7_bitmap__[] = {
//   0 $00 'char0'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	________,
	X_X_X___,
	________,
	X___X___,
	________,
	X_X_X___,
	________,
//  32 $20 'space'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	________,
	________,
	________,
	________,
	________,
	________,
	________,
//  33 $21 'exclam'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	__X_____,
	__X_____,
	__X_____,
	__X_____,
	________,
	__X_____,
	________,
//  34 $22 'quotedbl'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	_X_X____,
	_X_X____,
	_X_X____,
	________,
	________,
	________,
	________,
//  35 $23 'numbersign'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	________,
	_X_X____,
	XXXXX___,
	_X_X____,
	XXXXX___,
	_X_X____,
	________,
//  36 $24 'dollar'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	________,
	_XXX____,
	X_X_____,
	_XXX____,
	__X_X___,
	_XXX____,
	________,
//  37 $25 'percent'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	X_______,
	X__X____,
	__X_____,
	_X______,
	X__X____,
	___X____,
	________,
//  38 $26 'ampersand'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	________,
	_X______,
	X_X_____,
	_X______,
	X_X_____,
	_X_X____,
	________,
//  39 $27 'quotesingle'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	__X_____,
	__X_____,
	__X_____,
	________,
	________,
	________,
	________,
//  40 $28 'parenleft'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	__X_____,
	_X______,
	_X______,
	_X______,
	_X______,
	__X_____,
	________,
//  41 $29 'parenright'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	_X______,
	__X_____,
	__X_____,
	__X_____,
	__X_____,
	_X______,
	________,
//  42 $2a 'asterisk'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	________,
	_X_X____,
	__X_____,
	_XXX____,
	__X_____,
	_X_X____,
	________,
//  43 $2b 'plus'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	________,
	__X_____,
	__X_____,
	XXXXX___,
	__X_____,
	__X_____,
	________,
//  44 $2c 'comma'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	________,
	________,
	________,
	________,
	__XX____,
	__X_____,
	_X______,
//  45 $2d 'hyphen'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	________,
	________,
	________,
	XXXX____,
	________,
	________,
	________,
//  46 $2e 'period'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	________,
	________,
	________,
	________,
	_XX_____,
	_XX_____,
	________,
//  47 $2f 'slash'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	________,
	___X____,
	__X_____,
	_X______,
	X_______,
	________,
	________,
//  48 $30 'zero'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	__X_____,
	_X_X____,
	_X_X____,
	_X_X____,
	_X_X____,
	__X_____,
	________,
//  49 $31 'one'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	__X_____,
	_XX_____,
	__X_____,
	__X_____,
	__X_____,
	_XXX____,
	________,
//  50 $32 'two'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	_XX_____,
	X__X____,
	___X____,
	__X_____,
	_X______,
	XXXX____,
	________,
//  51 $33 'three'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	XXXX____,
	___X____,
	_XX_____,
	___X____,
	X__X____,
	_XX_____,
	________,
//  52 $34 'four'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	__X_____,
	_XX_____,
	X_X_____,
	XXXX____,
	__X_____,
	__X_____,
	________,
//  53 $35 'five'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	XXXX____,
	X_______,
	XXX_____,
	___X____,
	X__X____,
	_XX_____,
	________,
//  54 $36 'six'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	_XX_____,
	X_______,
	XXX_____,
	X__X____,
	X__X____,
	_XX_____,
	________,
//  55 $37 'seven'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	XXXX____,
	___X____,
	__X_____,
	__X_____,
	_X______,
	_X______,
	________,
//  56 $38 'eight'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	_XX_____,
	X__X____,
	_XX_____,
	X__X____,
	X__X____,
	_XX_____,
	________,
//  57 $39 'nine'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	_XX_____,
	X__X____,
	X__X____,
	_XXX____,
	___X____,
	_XX_____,
	________,
//  58 $3a 'colon'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	________,
	_XX_____,
	_XX_____,
	________,
	_XX_____,
	_XX_____,
	________,
//  59 $3b 'semicolon'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	________,
	_XX_____,
	_XX_____,
	________,
	_XX_____,
	_X______,
	X_______,
//  60 $3c 'less'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	________,
	___X____,
	__X_____,
	_X______,
	__X_____,
	___X____,
	________,
//  61 $3d 'equal'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	________,
	________,
	XXXX____,
	________,
	XXXX____,
	________,
	________,
//  62 $3e 'greater'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	________,
	_X______,
	__X_____,
	___X____,
	__X_____,
	_X______,
	________,
//  63 $3f 'question'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	__X_____,
	_X_X____,
	___X____,
	__X_____,
	________,
	__X_____,
	________,
//  64 $40 'at'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	_XX_____,
	X__X____,
	X_XX____,
	X_XX____,
	X_______,
	_XX_____,
	________,
//  65 $41 'A'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	_XX_____,
	X__X____,
	X__X____,
	XXXX____,
	X__X____,
	X__X____,
	________,
//  66 $42 'B'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	XXX_____,
	X__X____,
	XXX_____,
	X__X____,
	X__X____,
	XXX_____,
	________,
//  67 $43 'C'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	_XX_____,
	X__X____,
	X_______,
	X_______,
	X__X____,
	_XX_____,
	________,
//  68 $44 'D'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	XXX_____,
	X__X____,
	X__X____,
	X__X____,
	X__X____,
	XXX_____,
	________,
//  69 $45 'E'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	XXXX____,
	X_______,
	XXX_____,
	X_______,
	X_______,
	XXXX____,
	________,
//  70 $46 'F'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	XXXX____,
	X_______,
	XXX_____,
	X_______,
	X_______,
	X_______,
	________,
//  71 $47 'G'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	_XX_____,
	X__X____,
	X_______,
	X_XX____,
	X__X____,
	_XXX____,
	________,
//  72 $48 'H'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	X__X____,
	X__X____,
	XXXX____,
	X__X____,
	X__X____,
	X__X____,
	________,
//  73 $49 'I'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	_XXX____,
	__X_____,
	__X_____,
	__X_____,
	__X_____,
	_XXX____,
	________,
//  74 $4a 'J'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	___X____,
	___X____,
	___X____,
	___X____,
	X__X____,
	_XX_____,
	________,
//  75 $4b 'K'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	X__X____,
	X_X_____,
	XX______,
	XX______,
	X_X_____,
	X__X____,
	________,
//  76 $4c 'L'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	X_______,
	X_______,
	X_______,
	X_______,
	X_______,
	XXXX____,
	________,
//  77 $4d 'M'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	X__X____,
	XXXX____,
	XXXX____,
	X__X____,
	X__X____,
	X__X____,
	________,
//  78 $4e 'N'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	X__X____,
	XX_X____,
	XX_X____,
	X_XX____,
	X_XX____,
	X__X____,
	________,
//  79 $4f 'O'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	_XX_____,
	X__X____,
	X__X____,
	X__X____,
	X__X____,
	_XX_____,
	________,
//  80 $50 'P'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	XXX_____,
	X__X____,
	X__X____,
	XXX_____,
	X_______,
	X_______,
	________,
//  81 $51 'Q'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	_XX_____,
	X__X____,
	X__X____,
	X__X____,
	XX_X____,
	_XX_____,
	___X____,
//  82 $52 'R'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	XXX_____,
	X__X____,
	X__X____,
	XXX_____,
	X_X_____,
	X__X____,
	________,
//  83 $53 'S'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	_XX_____,
	X__X____,
	_X______,
	__X_____,
	X__X____,
	_XX_____,
	________,
//  84 $54 'T'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	_XXX____,
	__X_____,
	__X_____,
	__X_____,
	__X_____,
	__X_____,
	________,
//  85 $55 'U'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	X__X____,
	X__X____,
	X__X____,
	X__X____,
	X__X____,
	_XX_____,
	________,
//  86 $56 'V'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	X__X____,
	X__X____,
	X__X____,
	X__X____,
	_XX_____,
	_XX_____,
	________,
//  87 $57 'W'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	X__X____,
	X__X____,
	X__X____,
	XXXX____,
	XXXX____,
	X__X____,
	________,
//  88 $58 'X'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	X__X____,
	X__X____,
	_XX_____,
	_XX_____,
	X__X____,
	X__X____,
	________,
//  89 $59 'Y'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	_X_X____,
	_X_X____,
	_X_X____,
	__X_____,
	__X_____,
	__X_____,
	________,
//  90 $5a 'Z'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	XXXX____,
	___X____,
	__X_____,
	_X______,
	X_______,
	XXXX____,
	________,
//  91 $5b 'bracketleft'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	_XXX____,
	_X______,
	_X______,
	_X______,
	_X______,
	_XXX____,
	________,
//  92 $5c 'backslash'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	________,
	X_______,
	_X______,
	__X_____,
	___X____,
	________,
	________,
//  93 $5d 'bracketright'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	_XXX____,
	___X____,
	___X____,
	___X____,
	___X____,
	_XXX____,
	________,
//  94 $5e 'asciicircum'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	__X_____,
	_X_X____,
	________,
	________,
	________,
	________,
	________,
//  95 $5f 'underscore'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	________,
	________,
	________,
	________,
	________,
	XXXX____,
	________,
//  96 $60 'grave'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	_X______,
	__X_____,
	________,
	________,
	________,
	________,
	________,
//  97 $61 'a'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	________,
	________,
	_XXX____,
	X__X____,
	X_XX____,
	_X_X____,
	________,
//  98 $62 'b'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	X_______,
	X_______,
	XXX_____,
	X__X____,
	X__X____,
	XXX_____,
	________,
//  99 $63 'c'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	________,
	________,
	_XX_____,
	X_______,
	X_______,
	_XX_____,
	________,
// 100 $64 'd'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	___X____,
	___X____,
	_XXX____,
	X__X____,
	X__X____,
	_XXX____,
	________,
// 101 $65 'e'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	________,
	________,
	_XX_____,
	X_XX____,
	XX______,
	_XX_____,
	________,
// 102 $66 'f'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	__X_____,
	_X_X____,
	_X______,
	XXX_____,
	_X______,
	_X______,
	________,
// 103 $67 'g'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	________,
	________,
	_XXX____,
	X__X____,
	_XX_____,
	X_______,
	_XXX____,
// 104 $68 'h'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	X_______,
	X_______,
	XXX_____,
	X__X____,
	X__X____,
	X__X____,
	________,
// 105 $69 'i'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	__X_____,
	________,
	_XX_____,
	__X_____,
	__X_____,
	_XXX____,
	________,
// 106 $6a 'j'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	___X____,
	________,
	___X____,
	___X____,
	___X____,
	_X_X____,
	__X_____,
// 107 $6b 'k'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	X_______,
	X_______,
	X_X_____,
	XX______,
	X_X_____,
	X__X____,
	________,
// 108 $6c 'l'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	_XX_____,
	__X_____,
	__X_____,
	__X_____,
	__X_____,
	_XXX____,
	________,
// 109 $6d 'm'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	________,
	________,
	X_X_____,
	XXXX____,
	X__X____,
	X__X____,
	________,
// 110 $6e 'n'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	________,
	________,
	XXX_____,
	X__X____,
	X__X____,
	X__X____,
	________,
// 111 $6f 'o'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	________,
	________,
	_XX_____,
	X__X____,
	X__X____,
	_XX_____,
	________,
// 112 $70 'p'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	________,
	________,
	XXX_____,
	X__X____,
	X__X____,
	XXX_____,
	X_______,
// 113 $71 'q'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	________,
	________,
	_XXX____,
	X__X____,
	X__X____,
	_XXX____,
	___X____,
// 114 $72 'r'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	________,
	________,
	XXX_____,
	X__X____,
	X_______,
	X_______,
	________,
// 115 $73 's'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	________,
	________,
	_XXX____,
	XX______,
	__XX____,
	XXX_____,
	________,
// 116 $74 't'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	_X______,
	_X______,
	XXX_____,
	_X______,
	_X______,
	__XX____,
	________,
// 117 $75 'u'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	________,
	________,
	X__X____,
	X__X____,
	X__X____,
	_XXX____,
	________,
// 118 $76 'v'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	________,
	________,
	_X_X____,
	_X_X____,
	_X_X____,
	__X_____,
	________,
// 119 $77 'w'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	________,
	________,
	X__X____,
	X__X____,
	XXXX____,
	XXXX____,
	________,
// 120 $78 'x'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	________,
	________,
	X__X____,
	_XX_____,
	_XX_____,
	X__X____,
	________,
// 121 $79 'y'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	________,
	________,
	X__X____,
	X__X____,
	_X_X____,
	__X_____,
	_X______,
// 122 $7a 'z'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	________,
	________,
	XXXX____,
	__X_____,
	_X______,
	XXXX____,
	________,
// 123 $7b 'braceleft'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	___X____,
	__X_____,
	_XX_____,
	__X_____,
	__X_____,
	___X____,
	________,
// 124 $7c 'bar'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	__X_____,
	__X_____,
	__X_____,
	__X_____,
	__X_____,
	__X_____,
	________,
// 125 $7d 'braceright'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	_X______,
	__X_____,
	__XX____,
	__X_____,
	__X_____,
	_X______,
	________,
// 126 $7e 'asciitilde'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	_X_X____,
	X_X_____,
	________,
	________,
	________,
	________,
	________,
// 160 $a0 'space'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	________,
	________,
	________,
	________,
	________,
	________,
	________,
// 161 $a1 'exclamdown'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	__X_____,
	________,
	__X_____,
	__X_____,
	__X_____,
	__X_____,
	________,
// 162 $a2 'cent'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	________,
	__X_____,
	_XXX____,
	X_X_____,
	X_X_____,
	_XXX____,
	__X_____,
// 163 $a3 'sterling'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	________,
	__XX____,
	_X______,
	XXX_____,
	_X______,
	X_XX____,
	________,
// 164 $a4 'currency'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	________,
	X___X___,
	_XXX____,
	_X_X____,
	_XXX____,
	X___X___,
	________,
// 165 $a5 'yen'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	_X_X____,
	_X_X____,
	__X_____,
	_XXX____,
	__X_____,
	__X_____,
	________,
// 166 $a6 'brokenbar'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	________,
	__X_____,
	__X_____,
	________,
	__X_____,
	__X_____,
	________,
// 167 $a7 'section'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	__XX____,
	_X______,
	_XX_____,
	_X_X____,
	__XX____,
	___X____,
	_XX_____,
// 168 $a8 'dieresis'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	_X_X____,
	________,
	________,
	________,
	________,
	________,
	________,
// 169 $a9 'copyright'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	_XXX____,
	X___X___,
	X_X_X___,
	XX__X___,
	X_X_X___,
	X___X___,
	_XXX____,
// 170 $aa 'ordfeminine'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	_XX_____,
	X_X_____,
	_XX_____,
	________,
	________,
	________,
	________,
// 171 $ab 'guillemotleft'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	________,
	________,
	_X__X___,
	X__X____,
	_X__X___,
	________,
	________,
// 172 $ac 'logicalnot'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	________,
	________,
	________,
	XXXX____,
	___X____,
	________,
	________,
// 173 $ad 'hyphen'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	________,
	________,
	________,
	_XXX____,
	________,
	________,
	________,
// 174 $ae 'registered'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	_XXX____,
	X___X___,
	XXX_X___,
	XX__X___,
	XX__X___,
	X___X___,
	_XXX____,
// 175 $af 'macron'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	XXXX____,
	________,
	________,
	________,
	________,
	________,
	________,
// 176 $b0 'degree'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	__X_____,
	_X_X____,
	__X_____,
	________,
	________,
	________,
	________,
// 177 $b1 'plusminus'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	__X_____,
	__X_____,
	XXXXX___,
	__X_____,
	__X_____,
	XXXXX___,
	________,
// 178 $b2 'twosuperior'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	_XX_____,
	__X_____,
	_X______,
	_XX_____,
	________,
	________,
	________,
// 179 $b3 'threesuperior'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	_XX_____,
	_XX_____,
	__X_____,
	_XX_____,
	________,
	________,
	________,
// 180 $b4 'acute'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	__X_____,
	_X______,
	________,
	________,
	________,
	________,
	________,
// 181 $b5 'mu'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	________,
	________,
	X__X____,
	X__X____,
	X__X____,
	XXX_____,
	X_______,
// 182 $b6 'paragraph'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	_XXX____,
	XX_X____,
	XX_X____,
	_X_X____,
	_X_X____,
	_X_X____,
	________,
// 183 $b7 'periodcentered'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	________,
	________,
	_XX_____,
	_XX_____,
	________,
	________,
	________,
// 184 $b8 'cedilla'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	________,
	________,
	________,
	________,
	________,
	__X_____,
	_X______,
// 185 $b9 'onesuperior'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	__X_____,
	_XX_____,
	__X_____,
	_XXX____,
	________,
	________,
	________,
// 186 $ba 'ordmasculine'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	_X______,
	X_X_____,
	_X______,
	________,
	________,
	________,
	________,
// 187 $bb 'guillemotright'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	________,
	________,
	X__X____,
	_X__X___,
	X__X____,
	________,
	________,
// 188 $bc 'onequarter'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	X_______,
	X_______,
	X_______,
	X__X____,
	__XX____,
	_XXX____,
	___X____,
// 189 $bd 'onehalf'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	X_______,
	X_______,
	X_______,
	X_XX____,
	___X____,
	__X_____,
	__XX____,
// 190 $be 'threequarters'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	XX______,
	XX______,
	_X______,
	XX_X____,
	__XX____,
	_XXX____,
	___X____,
// 191 $bf 'questiondown'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	__X_____,
	________,
	__X_____,
	_X______,
	_X_X____,
	__X_____,
	________,
// 192 $c0 'Agrave'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	_XX_____,
	X__X____,
	X__X____,
	XXXX____,
	X__X____,
	X__X____,
	________,
// 193 $c1 'Aacute'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	_XX_____,
	X__X____,
	X__X____,
	XXXX____,
	X__X____,
	X__X____,
	________,
// 194 $c2 'Acircumflex'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	_XX_____,
	X__X____,
	X__X____,
	XXXX____,
	X__X____,
	X__X____,
	________,
// 195 $c3 'Atilde'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	_XX_____,
	X__X____,
	X__X____,
	XXXX____,
	X__X____,
	X__X____,
	________,
// 196 $c4 'Adieresis'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	X__X____,
	_XX_____,
	X__X____,
	XXXX____,
	X__X____,
	X__X____,
	________,
// 197 $c5 'Aring'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	_XX_____,
	_XX_____,
	X__X____,
	XXXX____,
	X__X____,
	X__X____,
	________,
// 198 $c6 'AE'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	_XXX____,
	X_X_____,
	X_XX____,
	XXX_____,
	X_X_____,
	X_XX____,
	________,
// 199 $c7 'Ccedilla'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	_XX_____,
	X__X____,
	X_______,
	X_______,
	X__X____,
	_XX_____,
	_X______,
// 200 $c8 'Egrave'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	XXXX____,
	X_______,
	XXX_____,
	X_______,
	X_______,
	XXXX____,
	________,
// 201 $c9 'Eacute'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	XXXX____,
	X_______,
	XXX_____,
	X_______,
	X_______,
	XXXX____,
	________,
// 202 $ca 'Ecircumflex'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	XXXX____,
	X_______,
	XXX_____,
	X_______,
	X_______,
	XXXX____,
	________,
// 203 $cb 'Edieresis'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	XXXX____,
	X_______,
	XXX_____,
	X_______,
	X_______,
	XXXX____,
	________,
// 204 $cc 'Igrave'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	_XXX____,
	__X_____,
	__X_____,
	__X_____,
	__X_____,
	_XXX____,
	________,
// 205 $cd 'Iacute'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	_XXX____,
	__X_____,
	__X_____,
	__X_____,
	__X_____,
	_XXX____,
	________,
// 206 $ce 'Icircumflex'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	_XXX____,
	__X_____,
	__X_____,
	__X_____,
	__X_____,
	_XXX____,
	________,
// 207 $cf 'Idieresis'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	_XXX____,
	__X_____,
	__X_____,
	__X_____,
	__X_____,
	_XXX____,
	________,
// 208 $d0 'Eth'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	XXX_____,
	_X_X____,
	XX_X____,
	_X_X____,
	_X_X____,
	XXX_____,
	________,
// 209 $d1 'Ntilde'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	X_XX____,
	X__X____,
	XX_X____,
	X_XX____,
	X_XX____,
	X__X____,
	________,
// 210 $d2 'Ograve'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	_XX_____,
	X__X____,
	X__X____,
	X__X____,
	X__X____,
	_XX_____,
	________,
// 211 $d3 'Oacute'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	_XX_____,
	X__X____,
	X__X____,
	X__X____,
	X__X____,
	_XX_____,
	________,
// 212 $d4 'Ocircumflex'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	_XX_____,
	X__X____,
	X__X____,
	X__X____,
	X__X____,
	_XX_____,
	________,
// 213 $d5 'Otilde'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	_XX_____,
	X__X____,
	X__X____,
	X__X____,
	X__X____,
	_XX_____,
	________,
// 214 $d6 'Odieresis'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	X__X____,
	_XX_____,
	X__X____,
	X__X____,
	X__X____,
	_XX_____,
	________,
// 215 $d7 'multiply'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	________,
	________,
	X__X____,
	_XX_____,
	_XX_____,
	X__X____,
	________,
// 216 $d8 'Oslash'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	_XXX____,
	X_XX____,
	X_XX____,
	XX_X____,
	XX_X____,
	XXX_____,
	________,
// 217 $d9 'Ugrave'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	X__X____,
	X__X____,
	X__X____,
	X__X____,
	X__X____,
	_XX_____,
	________,
// 218 $da 'Uacute'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	X__X____,
	X__X____,
	X__X____,
	X__X____,
	X__X____,
	_XX_____,
	________,
// 219 $db 'Ucircumflex'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	X__X____,
	X__X____,
	X__X____,
	X__X____,
	X__X____,
	_XX_____,
	________,
// 220 $dc 'Udieresis'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	X__X____,
	________,
	X__X____,
	X__X____,
	X__X____,
	_XX_____,
	________,
// 221 $dd 'Yacute'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	_X_X____,
	_X_X____,
	_X_X____,
	__X_____,
	__X_____,
	__X_____,
	________,
// 222 $de 'Thorn'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	X_______,
	XXX_____,
	X__X____,
	XXX_____,
	X_______,
	X_______,
	________,
// 223 $df 'germandbls'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	_XX_____,
	X__X____,
	X_X_____,
	X__X____,
	X__X____,
	X_X_____,
	________,
// 224 $e0 'agrave'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	_X______,
	__X_____,
	_XXX____,
	X__X____,
	X_XX____,
	_X_X____,
	________,
// 225 $e1 'aacute'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	__X_____,
	_X______,
	_XXX____,
	X__X____,
	X_XX____,
	_X_X____,
	________,
// 226 $e2 'acircumflex'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	__X_____,
	_X_X____,
	_XXX____,
	X__X____,
	X_XX____,
	_X_X____,
	________,
// 227 $e3 'atilde'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	_X_X____,
	X_X_____,
	_XXX____,
	X__X____,
	X_XX____,
	_X_X____,
	________,
// 228 $e4 'adieresis'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	_X_X____,
	________,
	_XXX____,
	X__X____,
	X_XX____,
	_X_X____,
	________,
// 229 $e5 'aring'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	_XX_____,
	_XX_____,
	_XXX____,
	X__X____,
	X_XX____,
	_X_X____,
	________,
// 230 $e6 'ae'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	________,
	________,
	_XXX____,
	X_XX____,
	X_X_____,
	_XXX____,
	________,
// 231 $e7 'ccedilla'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	________,
	________,
	__XX____,
	_X______,
	_X______,
	__XX____,
	__X_____,
// 232 $e8 'egrave'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	_X______,
	__X_____,
	_XX_____,
	X_XX____,
	XX______,
	_XX_____,
	________,
// 233 $e9 'eacute'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	__X_____,
	_X______,
	_XX_____,
	X_XX____,
	XX______,
	_XX_____,
	________,
// 234 $ea 'ecircumflex'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	_X______,
	X_X_____,
	_XX_____,
	X_XX____,
	XX______,
	_XX_____,
	________,
// 235 $eb 'edieresis'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	X_X_____,
	________,
	_XX_____,
	X_XX____,
	XX______,
	_XX_____,
	________,
// 236 $ec 'igrave'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	_X______,
	__X_____,
	_XX_____,
	__X_____,
	__X_____,
	_XXX____,
	________,
// 237 $ed 'iacute'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	__X_____,
	_X______,
	_XX_____,
	__X_____,
	__X_____,
	_XXX____,
	________,
// 238 $ee 'icircumflex'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	__X_____,
	_X_X____,
	_XX_____,
	__X_____,
	__X_____,
	_XXX____,
	________,
// 239 $ef 'idieresis'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	_X_X____,
	________,
	_XX_____,
	__X_____,
	__X_____,
	_XXX____,
	________,
// 240 $f0 'eth'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	_X______,
	__XX____,
	_XX_____,
	X__X____,
	X__X____,
	_XX_____,
	________,
// 241 $f1 'ntilde'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	_X_X____,
	X_X_____,
	XXX_____,
	X__X____,
	X__X____,
	X__X____,
	________,
// 242 $f2 'ograve'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	_X______,
	__X_____,
	_XX_____,
	X__X____,
	X__X____,
	_XX_____,
	________,
// 243 $f3 'oacute'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	__X_____,
	_X______,
	_XX_____,
	X__X____,
	X__X____,
	_XX_____,
	________,
// 244 $f4 'ocircumflex'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	_XX_____,
	________,
	_XX_____,
	X__X____,
	X__X____,
	_XX_____,
	________,
// 245 $f5 'otilde'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	_X_X____,
	X_X_____,
	_XX_____,
	X__X____,
	X__X____,
	_XX_____,
	________,
// 246 $f6 'odieresis'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	_X_X____,
	________,
	_XX_____,
	X__X____,
	X__X____,
	_XX_____,
	________,
// 247 $f7 'divide'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	________,
	_XX_____,
	________,
	XXXX____,
	________,
	_XX_____,
	________,
// 248 $f8 'oslash'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	________,
	________,
	_XXX____,
	X_XX____,
	XX_X____,
	XXX_____,
	________,
// 249 $f9 'ugrave'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	_X______,
	__X_____,
	X__X____,
	X__X____,
	X__X____,
	_XXX____,
	________,
// 250 $fa 'uacute'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	__X_____,
	_X______,
	X__X____,
	X__X____,
	X__X____,
	_XXX____,
	________,
// 251 $fb 'ucircumflex'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	_XX_____,
	________,
	X__X____,
	X__X____,
	X__X____,
	_XXX____,
	________,
// 252 $fc 'udieresis'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	_X_X____,
	________,
	X__X____,
	X__X____,
	X__X____,
	_XXX____,
	________,
// 253 $fd 'yacute'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	__X_____,
	_X______,
	X__X____,
	X__X____,
	_X_X____,
	__X_____,
	_X______,
// 254 $fe 'thorn'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	________,
	X_______,
	XXX_____,
	X__X____,
	X__X____,
	XXX_____,
	X_______,
// 255 $ff 'ydieresis'
//	width 5, bbx 0, bby -1, bbw 5, bbh 7
	_X_X____,
	________,
	X__X____,
	X__X____,
	_X_X____,
	__X_____,
	_X______,
};


	/// character encoding for each index entry
static const unsigned short __apple5x7_index__[] = {
	0,
	32,
	33,
	34,
	35,
	36,
	37,
	38,
	39,
	40,
	41,
	42,
	43,
	44,
	45,
	46,
	47,
	48,
	49,
	50,
	51,
	52,
	53,
	54,
	55,
	56,
	57,
	58,
	59,
	60,
	61,
	62,
	63,
	64,
	65,
	66,
	67,
	68,
	69,
	70,
	71,
	72,
	73,
	74,
	75,
	76,
	77,
	78,
	79,
	80,
	81,
	82,
	83,
	84,
	85,
	86,
	87,
	88,
	89,
	90,
	91,
	92,
	93,
	94,
	95,
	96,
	97,
	98,
	99,
	100,
	101,
	102,
	103,
	104,
	105,
	106,
	107,
	108,
	109,
	110,
	111,
	112,
	113,
	114,
	115,
	116,
	117,
	118,
	119,
	120,
	121,
	122,
	123,
	124,
	125,
	126,
	160,
	161,
	162,
	163,
	164,
	165,
	166,
	167,
	168,
	169,
	170,
	171,
	172,
	173,
	174,
	175,
	176,
	177,
	178,
	179,
	180,
	181,
	182,
	183,
	184,
	185,
	186,
	187,
	188,
	189,
	190,
	191,
	192,
	193,
	194,
	195,
	196,
	197,
	198,
	199,
	200,
	201,
	202,
	203,
	204,
	205,
	206,
	207,
	208,
	209,
	210,
	211,
	212,
	213,
	214,
	215,
	216,
	217,
	218,
	219,
	220,
	221,
	222,
	223,
	224,
	225,
	226,
	227,
	228,
	229,
	230,
	231,
	232,
	233,
	234,
	235,
	236,
	237,
	238,
	239,
	240,
	241,
	242,
	243,
	244,
	245,
	246,
	247,
	248,
	249,
	250,
	251,
	252,
	253,
	254,
	255,
};

	/// bitmap font structure
const struct bitmap_font apple5x7 = {
	.Width = 5, .Height = 7,
	.Chars = 192,
	.Widths = 0,
	.Index = __apple5x7_index__,
	.Bitmap = __apple5x7_bitmap__,
};

