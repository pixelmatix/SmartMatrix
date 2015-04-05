// Created from bdf2c Version 4, (c) 2009, 2010 by Lutz Sammer
//	License AGPLv3: GNU Affero General Public License version 3

// Source: http://robey.lag.net/2010/01/23/tiny-monospace-font.html

#include "MatrixFontCommon.h"

	/// character bitmap for each encoding
static const unsigned char __tomthumb_bitmap__[] = {
//  32 $20 'space'
//	width 4, bbx 3, bby 4, bbw 1, bbh 1
	________,
	________,
	________,
	________,
	________,
	________,
//  33 $21 'exclam'
//	width 4, bbx 1, bby 0, bbw 1, bbh 5
	_X______,
	_X______,
	_X______,
	________,
	_X______,
	________,
//  34 $22 'quotedbl'
//	width 4, bbx 0, bby 3, bbw 3, bbh 2
	X_X_____,
	X_X_____,
	________,
	________,
	________,
	________,
//  35 $23 'numbersign'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	X_X_____,
	XXX_____,
	X_X_____,
	XXX_____,
	X_X_____,
	________,
//  36 $24 'dollar'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	_XX_____,
	XX______,
	_XX_____,
	XX______,
	_X______,
	________,
//  37 $25 'percent'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	X_______,
	__X_____,
	_X______,
	X_______,
	__X_____,
	________,
//  38 $26 'ampersand'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	XX______,
	XX______,
	XXX_____,
	X_X_____,
	_XX_____,
	________,
//  39 $27 'quotesingle'
//	width 4, bbx 1, bby 3, bbw 1, bbh 2
	_X______,
	_X______,
	________,
	________,
	________,
	________,
//  40 $28 'parenleft'
//	width 4, bbx 1, bby 0, bbw 2, bbh 5
	__X_____,
	_X______,
	_X______,
	_X______,
	__X_____,
	________,
//  41 $29 'parenright'
//	width 4, bbx 0, bby 0, bbw 2, bbh 5
	X_______,
	_X______,
	_X______,
	_X______,
	X_______,
	________,
//  42 $2a 'asterisk'
//	width 4, bbx 0, bby 2, bbw 3, bbh 3
	X_X_____,
	_X______,
	X_X_____,
	________,
	________,
	________,
//  43 $2b 'plus'
//	width 4, bbx 0, bby 1, bbw 3, bbh 3
	________,
	_X______,
	XXX_____,
	_X______,
	________,
	________,
//  44 $2c 'comma'
//	width 4, bbx 0, bby 0, bbw 2, bbh 2
	________,
	________,
	________,
	_X______,
	X_______,
	________,
//  45 $2d 'hyphen'
//	width 4, bbx 0, bby 2, bbw 3, bbh 1
	________,
	________,
	XXX_____,
	________,
	________,
	________,
//  46 $2e 'period'
//	width 4, bbx 1, bby 0, bbw 1, bbh 1
	________,
	________,
	________,
	________,
	_X______,
	________,
//  47 $2f 'slash'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	__X_____,
	__X_____,
	_X______,
	X_______,
	X_______,
	________,
//  48 $30 'zero'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	_XX_____,
	X_X_____,
	X_X_____,
	X_X_____,
	XX______,
	________,
//  49 $31 'one'
//	width 4, bbx 0, bby 0, bbw 2, bbh 5
	_X______,
	XX______,
	_X______,
	_X______,
	_X______,
	________,
//  50 $32 'two'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	XX______,
	__X_____,
	_X______,
	X_______,
	XXX_____,
	________,
//  51 $33 'three'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	XX______,
	__X_____,
	_X______,
	__X_____,
	XX______,
	________,
//  52 $34 'four'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	X_X_____,
	X_X_____,
	XXX_____,
	__X_____,
	__X_____,
	________,
//  53 $35 'five'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	XXX_____,
	X_______,
	XX______,
	__X_____,
	XX______,
	________,
//  54 $36 'six'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	_XX_____,
	X_______,
	XXX_____,
	X_X_____,
	XXX_____,
	________,
//  55 $37 'seven'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	XXX_____,
	__X_____,
	_X______,
	X_______,
	X_______,
	________,
//  56 $38 'eight'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	XXX_____,
	X_X_____,
	XXX_____,
	X_X_____,
	XXX_____,
	________,
//  57 $39 'nine'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	XXX_____,
	X_X_____,
	XXX_____,
	__X_____,
	XX______,
	________,
//  58 $3a 'colon'
//	width 4, bbx 1, bby 1, bbw 1, bbh 3
	________,
	_X______,
	________,
	_X______,
	________,
	________,
//  59 $3b 'semicolon'
//	width 4, bbx 0, bby 0, bbw 2, bbh 4
	________,
	_X______,
	________,
	_X______,
	X_______,
	________,
//  60 $3c 'less'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	__X_____,
	_X______,
	X_______,
	_X______,
	__X_____,
	________,
//  61 $3d 'equal'
//	width 4, bbx 0, bby 1, bbw 3, bbh 3
	________,
	XXX_____,
	________,
	XXX_____,
	________,
	________,
//  62 $3e 'greater'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	X_______,
	_X______,
	__X_____,
	_X______,
	X_______,
	________,
//  63 $3f 'question'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	XXX_____,
	__X_____,
	_X______,
	________,
	_X______,
	________,
//  64 $40 'at'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	_X______,
	X_X_____,
	XXX_____,
	X_______,
	_XX_____,
	________,
//  65 $41 'A'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	_X______,
	X_X_____,
	XXX_____,
	X_X_____,
	X_X_____,
	________,
//  66 $42 'B'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	XX______,
	X_X_____,
	XX______,
	X_X_____,
	XX______,
	________,
//  67 $43 'C'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	_XX_____,
	X_______,
	X_______,
	X_______,
	_XX_____,
	________,
//  68 $44 'D'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	XX______,
	X_X_____,
	X_X_____,
	X_X_____,
	XX______,
	________,
//  69 $45 'E'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	XXX_____,
	X_______,
	XXX_____,
	X_______,
	XXX_____,
	________,
//  70 $46 'F'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	XXX_____,
	X_______,
	XXX_____,
	X_______,
	X_______,
	________,
//  71 $47 'G'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	_XX_____,
	X_______,
	XXX_____,
	X_X_____,
	_XX_____,
	________,
//  72 $48 'H'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	X_X_____,
	X_X_____,
	XXX_____,
	X_X_____,
	X_X_____,
	________,
//  73 $49 'I'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	XXX_____,
	_X______,
	_X______,
	_X______,
	XXX_____,
	________,
//  74 $4a 'J'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	__X_____,
	__X_____,
	__X_____,
	X_X_____,
	_X______,
	________,
//  75 $4b 'K'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	X_X_____,
	X_X_____,
	XX______,
	X_X_____,
	X_X_____,
	________,
//  76 $4c 'L'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	X_______,
	X_______,
	X_______,
	X_______,
	XXX_____,
	________,
//  77 $4d 'M'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	X_X_____,
	XXX_____,
	XXX_____,
	X_X_____,
	X_X_____,
	________,
//  78 $4e 'N'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	X_X_____,
	XXX_____,
	XXX_____,
	XXX_____,
	X_X_____,
	________,
//  79 $4f 'O'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	_X______,
	X_X_____,
	X_X_____,
	X_X_____,
	_X______,
	________,
//  80 $50 'P'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	XX______,
	X_X_____,
	XX______,
	X_______,
	X_______,
	________,
//  81 $51 'Q'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	_X______,
	X_X_____,
	X_X_____,
	XXX_____,
	_XX_____,
	________,
//  82 $52 'R'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	XX______,
	X_X_____,
	XXX_____,
	XX______,
	X_X_____,
	________,
//  83 $53 'S'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	_XX_____,
	X_______,
	_X______,
	__X_____,
	XX______,
	________,
//  84 $54 'T'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	XXX_____,
	_X______,
	_X______,
	_X______,
	_X______,
	________,
//  85 $55 'U'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	X_X_____,
	X_X_____,
	X_X_____,
	X_X_____,
	_XX_____,
	________,
//  86 $56 'V'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	X_X_____,
	X_X_____,
	X_X_____,
	_X______,
	_X______,
	________,
//  87 $57 'W'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	X_X_____,
	X_X_____,
	XXX_____,
	XXX_____,
	X_X_____,
	________,
//  88 $58 'X'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	X_X_____,
	X_X_____,
	_X______,
	X_X_____,
	X_X_____,
	________,
//  89 $59 'Y'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	X_X_____,
	X_X_____,
	_X______,
	_X______,
	_X______,
	________,
//  90 $5a 'Z'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	XXX_____,
	__X_____,
	_X______,
	X_______,
	XXX_____,
	________,
//  91 $5b 'bracketleft'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	XXX_____,
	X_______,
	X_______,
	X_______,
	XXX_____,
	________,
//  92 $5c 'backslash'
//	width 4, bbx 0, bby 1, bbw 3, bbh 3
	________,
	X_______,
	_X______,
	__X_____,
	________,
	________,
//  93 $5d 'bracketright'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	XXX_____,
	__X_____,
	__X_____,
	__X_____,
	XXX_____,
	________,
//  94 $5e 'asciicircum'
//	width 4, bbx 0, bby 3, bbw 3, bbh 2
	_X______,
	X_X_____,
	________,
	________,
	________,
	________,
//  95 $5f 'underscore'
//	width 4, bbx 0, bby 0, bbw 3, bbh 1
	________,
	________,
	________,
	________,
	XXX_____,
	________,
//  96 $60 'grave'
//	width 4, bbx 0, bby 3, bbw 2, bbh 2
	X_______,
	_X______,
	________,
	________,
	________,
	________,
//  97 $61 'a'
//	width 4, bbx 0, bby 0, bbw 3, bbh 4
	________,
	XX______,
	_XX_____,
	X_X_____,
	XXX_____,
	________,
//  98 $62 'b'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	X_______,
	XX______,
	X_X_____,
	X_X_____,
	XX______,
	________,
//  99 $63 'c'
//	width 4, bbx 0, bby 0, bbw 3, bbh 4
	________,
	_XX_____,
	X_______,
	X_______,
	_XX_____,
	________,
// 100 $64 'd'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	__X_____,
	_XX_____,
	X_X_____,
	X_X_____,
	_XX_____,
	________,
// 101 $65 'e'
//	width 4, bbx 0, bby 0, bbw 3, bbh 4
	________,
	_XX_____,
	X_X_____,
	XX______,
	_XX_____,
	________,
// 102 $66 'f'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	__X_____,
	_X______,
	XXX_____,
	_X______,
	_X______,
	________,
// 103 $67 'g'
//	width 4, bbx 0, bby -1, bbw 3, bbh 5
	________,
	_XX_____,
	X_X_____,
	XXX_____,
	__X_____,
	_X______,
// 104 $68 'h'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	X_______,
	XX______,
	X_X_____,
	X_X_____,
	X_X_____,
	________,
// 105 $69 'i'
//	width 4, bbx 1, bby 0, bbw 1, bbh 5
	_X______,
	________,
	_X______,
	_X______,
	_X______,
	________,
// 106 $6a 'j'
//	width 4, bbx 0, bby -1, bbw 3, bbh 6
	__X_____,
	________,
	__X_____,
	__X_____,
	X_X_____,
	_X______,
// 107 $6b 'k'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	X_______,
	X_X_____,
	XX______,
	XX______,
	X_X_____,
	________,
// 108 $6c 'l'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	XX______,
	_X______,
	_X______,
	_X______,
	XXX_____,
	________,
// 109 $6d 'm'
//	width 4, bbx 0, bby 0, bbw 3, bbh 4
	________,
	XXX_____,
	XXX_____,
	XXX_____,
	X_X_____,
	________,
// 110 $6e 'n'
//	width 4, bbx 0, bby 0, bbw 3, bbh 4
	________,
	XX______,
	X_X_____,
	X_X_____,
	X_X_____,
	________,
// 111 $6f 'o'
//	width 4, bbx 0, bby 0, bbw 3, bbh 4
	________,
	_X______,
	X_X_____,
	X_X_____,
	_X______,
	________,
// 112 $70 'p'
//	width 4, bbx 0, bby -1, bbw 3, bbh 5
	________,
	XX______,
	X_X_____,
	X_X_____,
	XX______,
	X_______,
// 113 $71 'q'
//	width 4, bbx 0, bby -1, bbw 3, bbh 5
	________,
	_XX_____,
	X_X_____,
	X_X_____,
	_XX_____,
	__X_____,
// 114 $72 'r'
//	width 4, bbx 0, bby 0, bbw 3, bbh 4
	________,
	_XX_____,
	X_______,
	X_______,
	X_______,
	________,
// 115 $73 's'
//	width 4, bbx 0, bby 0, bbw 3, bbh 4
	________,
	_XX_____,
	XX______,
	_XX_____,
	XX______,
	________,
// 116 $74 't'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	_X______,
	XXX_____,
	_X______,
	_X______,
	_XX_____,
	________,
// 117 $75 'u'
//	width 4, bbx 0, bby 0, bbw 3, bbh 4
	________,
	X_X_____,
	X_X_____,
	X_X_____,
	_XX_____,
	________,
// 118 $76 'v'
//	width 4, bbx 0, bby 0, bbw 3, bbh 4
	________,
	X_X_____,
	X_X_____,
	XXX_____,
	_X______,
	________,
// 119 $77 'w'
//	width 4, bbx 0, bby 0, bbw 3, bbh 4
	________,
	X_X_____,
	XXX_____,
	XXX_____,
	XXX_____,
	________,
// 120 $78 'x'
//	width 4, bbx 0, bby 0, bbw 3, bbh 4
	________,
	X_X_____,
	_X______,
	_X______,
	X_X_____,
	________,
// 121 $79 'y'
//	width 4, bbx 0, bby -1, bbw 3, bbh 5
	________,
	X_X_____,
	X_X_____,
	_XX_____,
	__X_____,
	_X______,
// 122 $7a 'z'
//	width 4, bbx 0, bby 0, bbw 3, bbh 4
	________,
	XXX_____,
	_XX_____,
	XX______,
	XXX_____,
	________,
// 123 $7b 'braceleft'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	_XX_____,
	_X______,
	X_______,
	_X______,
	_XX_____,
	________,
// 124 $7c 'bar'
//	width 4, bbx 1, bby 0, bbw 1, bbh 5
	_X______,
	_X______,
	________,
	_X______,
	_X______,
	________,
// 125 $7d 'braceright'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	XX______,
	_X______,
	__X_____,
	_X______,
	XX______,
	________,
// 126 $7e 'asciitilde'
//	width 4, bbx 0, bby 3, bbw 3, bbh 2
	_XX_____,
	XX______,
	________,
	________,
	________,
	________,
// 161 $a1 'exclamdown'
//	width 4, bbx 1, bby 0, bbw 1, bbh 5
	_X______,
	________,
	_X______,
	_X______,
	_X______,
	________,
// 162 $a2 'cent'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	_X______,
	XXX_____,
	X_______,
	XXX_____,
	_X______,
	________,
// 163 $a3 'sterling'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	_XX_____,
	_X______,
	XXX_____,
	_X______,
	XXX_____,
	________,
// 164 $a4 'currency'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	X_X_____,
	_X______,
	XXX_____,
	_X______,
	X_X_____,
	________,
// 165 $a5 'yen'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	X_X_____,
	X_X_____,
	_X______,
	XXX_____,
	_X______,
	________,
// 166 $a6 'brokenbar'
//	width 4, bbx 1, bby 0, bbw 1, bbh 5
	_X______,
	_X______,
	________,
	_X______,
	_X______,
	________,
// 167 $a7 'section'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	_XX_____,
	_X______,
	X_X_____,
	_X______,
	XX______,
	________,
// 168 $a8 'dieresis'
//	width 4, bbx 0, bby 4, bbw 3, bbh 1
	X_X_____,
	________,
	________,
	________,
	________,
	________,
// 169 $a9 'copyright'
//	width 4, bbx 0, bby 2, bbw 3, bbh 3
	_XX_____,
	X_______,
	_XX_____,
	________,
	________,
	________,
// 170 $aa 'ordfeminine'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	_XX_____,
	X_X_____,
	XXX_____,
	________,
	XXX_____,
	________,
// 171 $ab 'guillemotleft'
//	width 4, bbx 0, bby 2, bbw 2, bbh 3
	_X______,
	X_______,
	_X______,
	________,
	________,
	________,
// 172 $ac 'logicalnot'
//	width 4, bbx 0, bby 2, bbw 3, bbh 2
	________,
	XXX_____,
	__X_____,
	________,
	________,
	________,
// 173 $ad 'softhyphen'
//	width 4, bbx 0, bby 2, bbw 2, bbh 1
	________,
	________,
	XX______,
	________,
	________,
	________,
// 174 $ae 'registered'
//	width 4, bbx 0, bby 2, bbw 3, bbh 3
	XX______,
	XX______,
	X_X_____,
	________,
	________,
	________,
// 175 $af 'macron'
//	width 4, bbx 0, bby 4, bbw 3, bbh 1
	XXX_____,
	________,
	________,
	________,
	________,
	________,
// 176 $b0 'degree'
//	width 4, bbx 0, bby 2, bbw 3, bbh 3
	_X______,
	X_X_____,
	_X______,
	________,
	________,
	________,
// 177 $b1 'plusminus'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	_X______,
	XXX_____,
	_X______,
	________,
	XXX_____,
	________,
// 178 $b2 'twosuperior'
//	width 4, bbx 0, bby 2, bbw 3, bbh 3
	XX______,
	_X______,
	_XX_____,
	________,
	________,
	________,
// 179 $b3 'threesuperior'
//	width 4, bbx 0, bby 2, bbw 3, bbh 3
	XXX_____,
	_XX_____,
	XXX_____,
	________,
	________,
	________,
// 180 $b4 'acute'
//	width 4, bbx 1, bby 3, bbw 2, bbh 2
	__X_____,
	_X______,
	________,
	________,
	________,
	________,
// 181 $b5 'mu'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	X_X_____,
	X_X_____,
	X_X_____,
	XX______,
	X_______,
	________,
// 182 $b6 'paragraph'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	_XX_____,
	X_X_____,
	_XX_____,
	_XX_____,
	_XX_____,
	________,
// 183 $b7 'periodcentered'
//	width 4, bbx 0, bby 1, bbw 3, bbh 3
	________,
	XXX_____,
	XXX_____,
	XXX_____,
	________,
	________,
// 184 $b8 'cedilla'
//	width 4, bbx 0, bby 0, bbw 3, bbh 3
	________,
	________,
	_X______,
	__X_____,
	XX______,
	________,
// 185 $b9 'onesuperior'
//	width 4, bbx 1, bby 2, bbw 1, bbh 3
	_X______,
	_X______,
	_X______,
	________,
	________,
	________,
// 186 $ba 'ordmasculine'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	_X______,
	X_X_____,
	_X______,
	________,
	XXX_____,
	________,
// 187 $bb 'guillemotright'
//	width 4, bbx 1, bby 2, bbw 2, bbh 3
	_X______,
	__X_____,
	_X______,
	________,
	________,
	________,
// 188 $bc 'onequarter'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	X_______,
	X_______,
	________,
	_XX_____,
	__X_____,
	________,
// 189 $bd 'onehalf'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	X_______,
	X_______,
	________,
	XX______,
	_XX_____,
	________,
// 190 $be 'threequarters'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	XX______,
	XX______,
	________,
	_XX_____,
	__X_____,
	________,
// 191 $bf 'questiondown'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	_X______,
	________,
	_X______,
	X_______,
	XXX_____,
	________,
// 192 $c0 'Agrave'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	_X______,
	__X_____,
	_X______,
	XXX_____,
	X_X_____,
	________,
// 193 $c1 'Aacute'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	_X______,
	X_______,
	_X______,
	XXX_____,
	X_X_____,
	________,
// 194 $c2 'Acircumflex'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	XXX_____,
	________,
	_X______,
	XXX_____,
	X_X_____,
	________,
// 195 $c3 'Atilde'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	_XX_____,
	XX______,
	_X______,
	XXX_____,
	X_X_____,
	________,
// 196 $c4 'Adieresis'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	X_X_____,
	_X______,
	X_X_____,
	XXX_____,
	X_X_____,
	________,
// 197 $c5 'Aring'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	XX______,
	XX______,
	X_X_____,
	XXX_____,
	X_X_____,
	________,
// 198 $c6 'AE'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	_XX_____,
	XX______,
	XXX_____,
	XX______,
	XXX_____,
	________,
// 199 $c7 'Ccedilla'
//	width 4, bbx 0, bby -1, bbw 3, bbh 6
	_XX_____,
	X_______,
	X_______,
	_XX_____,
	__X_____,
	_X______,
// 200 $c8 'Egrave'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	_X______,
	__X_____,
	XXX_____,
	XX______,
	XXX_____,
	________,
// 201 $c9 'Eacute'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	_X______,
	X_______,
	XXX_____,
	XX______,
	XXX_____,
	________,
// 202 $ca 'Ecircumflex'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	XXX_____,
	________,
	XXX_____,
	XX______,
	XXX_____,
	________,
// 203 $cb 'Edieresis'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	X_X_____,
	________,
	XXX_____,
	XX______,
	XXX_____,
	________,
// 204 $cc 'Igrave'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	_X______,
	__X_____,
	XXX_____,
	_X______,
	XXX_____,
	________,
// 205 $cd 'Iacute'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	_X______,
	X_______,
	XXX_____,
	_X______,
	XXX_____,
	________,
// 206 $ce 'Icircumflex'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	XXX_____,
	________,
	XXX_____,
	_X______,
	XXX_____,
	________,
// 207 $cf 'Idieresis'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	X_X_____,
	________,
	XXX_____,
	_X______,
	XXX_____,
	________,
// 208 $d0 'Eth'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	XX______,
	X_X_____,
	XXX_____,
	X_X_____,
	XX______,
	________,
// 209 $d1 'Ntilde'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	XX______,
	_XX_____,
	X_X_____,
	XXX_____,
	X_X_____,
	________,
// 210 $d2 'Ograve'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	_X______,
	__X_____,
	XXX_____,
	X_X_____,
	XXX_____,
	________,
// 211 $d3 'Oacute'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	_X______,
	X_______,
	XXX_____,
	X_X_____,
	XXX_____,
	________,
// 212 $d4 'Ocircumflex'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	XXX_____,
	________,
	XXX_____,
	X_X_____,
	XXX_____,
	________,
// 213 $d5 'Otilde'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	XX______,
	_XX_____,
	XXX_____,
	X_X_____,
	XXX_____,
	________,
// 214 $d6 'Odieresis'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	X_X_____,
	________,
	XXX_____,
	X_X_____,
	XXX_____,
	________,
// 215 $d7 'multiply'
//	width 4, bbx 0, bby 1, bbw 3, bbh 3
	________,
	X_X_____,
	_X______,
	X_X_____,
	________,
	________,
// 216 $d8 'Oslash'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	_XX_____,
	X_X_____,
	XXX_____,
	X_X_____,
	XX______,
	________,
// 217 $d9 'Ugrave'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	X_______,
	_X______,
	X_X_____,
	X_X_____,
	XXX_____,
	________,
// 218 $da 'Uacute'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	__X_____,
	_X______,
	X_X_____,
	X_X_____,
	XXX_____,
	________,
// 219 $db 'Ucircumflex'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	XXX_____,
	________,
	X_X_____,
	X_X_____,
	XXX_____,
	________,
// 220 $dc 'Udieresis'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	X_X_____,
	________,
	X_X_____,
	X_X_____,
	XXX_____,
	________,
// 221 $dd 'Yacute'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	__X_____,
	_X______,
	X_X_____,
	XXX_____,
	_X______,
	________,
// 222 $de 'Thorn'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	X_______,
	XXX_____,
	X_X_____,
	XXX_____,
	X_______,
	________,
// 223 $df 'germandbls'
//	width 4, bbx 0, bby -1, bbw 3, bbh 6
	_XX_____,
	X_X_____,
	XX______,
	X_X_____,
	XX______,
	X_______,
// 224 $e0 'agrave'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	_X______,
	__X_____,
	_XX_____,
	X_X_____,
	XXX_____,
	________,
// 225 $e1 'aacute'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	_X______,
	X_______,
	_XX_____,
	X_X_____,
	XXX_____,
	________,
// 226 $e2 'acircumflex'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	XXX_____,
	________,
	_XX_____,
	X_X_____,
	XXX_____,
	________,
// 227 $e3 'atilde'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	_XX_____,
	XX______,
	_XX_____,
	X_X_____,
	XXX_____,
	________,
// 228 $e4 'adieresis'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	X_X_____,
	________,
	_XX_____,
	X_X_____,
	XXX_____,
	________,
// 229 $e5 'aring'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	_XX_____,
	_XX_____,
	_XX_____,
	X_X_____,
	XXX_____,
	________,
// 230 $e6 'ae'
//	width 4, bbx 0, bby 0, bbw 3, bbh 4
	________,
	_XX_____,
	XXX_____,
	XXX_____,
	XX______,
	________,
// 231 $e7 'ccedilla'
//	width 4, bbx 0, bby -1, bbw 3, bbh 5
	________,
	_XX_____,
	X_______,
	_XX_____,
	__X_____,
	_X______,
// 232 $e8 'egrave'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	_X______,
	__X_____,
	_XX_____,
	XXX_____,
	_XX_____,
	________,
// 233 $e9 'eacute'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	_X______,
	X_______,
	_XX_____,
	XXX_____,
	_XX_____,
	________,
// 234 $ea 'ecircumflex'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	XXX_____,
	________,
	_XX_____,
	XXX_____,
	_XX_____,
	________,
// 235 $eb 'edieresis'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	X_X_____,
	________,
	_XX_____,
	XXX_____,
	_XX_____,
	________,
// 236 $ec 'igrave'
//	width 4, bbx 1, bby 0, bbw 2, bbh 5
	_X______,
	__X_____,
	_X______,
	_X______,
	_X______,
	________,
// 237 $ed 'iacute'
//	width 4, bbx 0, bby 0, bbw 2, bbh 5
	_X______,
	X_______,
	_X______,
	_X______,
	_X______,
	________,
// 238 $ee 'icircumflex'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	XXX_____,
	________,
	_X______,
	_X______,
	_X______,
	________,
// 239 $ef 'idieresis'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	X_X_____,
	________,
	_X______,
	_X______,
	_X______,
	________,
// 240 $f0 'eth'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	_XX_____,
	XX______,
	_XX_____,
	X_X_____,
	_XX_____,
	________,
// 241 $f1 'ntilde'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	XX______,
	_XX_____,
	XX______,
	X_X_____,
	X_X_____,
	________,
// 242 $f2 'ograve'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	_X______,
	__X_____,
	_X______,
	X_X_____,
	_X______,
	________,
// 243 $f3 'oacute'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	_X______,
	X_______,
	_X______,
	X_X_____,
	_X______,
	________,
// 244 $f4 'ocircumflex'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	XXX_____,
	________,
	_X______,
	X_X_____,
	_X______,
	________,
// 245 $f5 'otilde'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	XX______,
	_XX_____,
	_X______,
	X_X_____,
	_X______,
	________,
// 246 $f6 'odieresis'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	X_X_____,
	________,
	_X______,
	X_X_____,
	_X______,
	________,
// 247 $f7 'divide'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	_X______,
	________,
	XXX_____,
	________,
	_X______,
	________,
// 248 $f8 'oslash'
//	width 4, bbx 0, bby 0, bbw 3, bbh 4
	________,
	_XX_____,
	XXX_____,
	X_X_____,
	XX______,
	________,
// 249 $f9 'ugrave'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	X_______,
	_X______,
	X_X_____,
	X_X_____,
	_XX_____,
	________,
// 250 $fa 'uacute'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	__X_____,
	_X______,
	X_X_____,
	X_X_____,
	_XX_____,
	________,
// 251 $fb 'ucircumflex'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	XXX_____,
	________,
	X_X_____,
	X_X_____,
	_XX_____,
	________,
// 252 $fc 'udieresis'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	X_X_____,
	________,
	X_X_____,
	X_X_____,
	_XX_____,
	________,
// 253 $fd 'yacute'
//	width 4, bbx 0, bby -1, bbw 3, bbh 6
	__X_____,
	_X______,
	X_X_____,
	_XX_____,
	__X_____,
	_X______,
// 254 $fe 'thorn'
//	width 4, bbx 0, bby -1, bbw 3, bbh 5
	________,
	X_______,
	XX______,
	X_X_____,
	XX______,
	X_______,
// 255 $ff 'ydieresis'
//	width 4, bbx 0, bby -1, bbw 3, bbh 6
	X_X_____,
	________,
	X_X_____,
	_XX_____,
	__X_____,
	_X______,
// 285 $11d 'gcircumflex'
//	width 6, bbx 0, bby 0, bbw 1, bbh 1
	________,
	________,
	________,
	________,
	________,
	________,
// 338 $152 'OE'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	_XX_____,
	XX______,
	XXX_____,
	XX______,
	_XX_____,
	________,
// 339 $153 'oe'
//	width 4, bbx 0, bby 0, bbw 3, bbh 4
	________,
	_XX_____,
	XXX_____,
	XX______,
	XXX_____,
	________,
// 352 $160 'Scaron'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	X_X_____,
	_XX_____,
	XX______,
	_XX_____,
	XX______,
	________,
// 353 $161 'scaron'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	X_X_____,
	_XX_____,
	XX______,
	_XX_____,
	XX______,
	________,
// 376 $178 'Ydieresis'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	X_X_____,
	________,
	X_X_____,
	_X______,
	_X______,
	________,
// 381 $17d 'Zcaron'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	X_X_____,
	XXX_____,
	_XX_____,
	XX______,
	XXX_____,
	________,
// 382 $17e 'zcaron'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	X_X_____,
	XXX_____,
	_XX_____,
	XX______,
	XXX_____,
	________,
// 3748 $ea4 'uni0EA4'
//	width 6, bbx 0, bby 0, bbw 1, bbh 1
	________,
	________,
	________,
	________,
	________,
	________,
// 5024 $13a0 'uni13A0'
//	width 6, bbx 0, bby 0, bbw 1, bbh 1
	________,
	________,
	________,
	________,
	________,
	________,
// 8226 $2022 'bullet'
//	width 4, bbx 1, bby 2, bbw 1, bbh 1
	________,
	________,
	_X______,
	________,
	________,
	________,
// 8230 $2026 'ellipsis'
//	width 4, bbx 0, bby 0, bbw 3, bbh 1
	________,
	________,
	________,
	________,
	X_X_____,
	________,
// 8364 $20ac 'Euro'
//	width 4, bbx 0, bby 0, bbw 3, bbh 5
	_XX_____,
	XXX_____,
	XXX_____,
	XX______,
	_XX_____,
	________,
};

	/// character encoding for each index entry
static const unsigned short __tomthumb_index__[] = {
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
	285,
	338,
	339,
	352,
	353,
	376,
	381,
	382,
	3748,
	5024,
	8226,
	8230,
	8364,
};

	/// bitmap font structure
const struct bitmap_font tomthumb = {
	.Width = 4, .Height = 6,
	.Chars = 203,
	.Widths = 0,
	.Index = __tomthumb_index__,
	.Bitmap = __tomthumb_bitmap__,
};

