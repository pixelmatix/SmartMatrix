// Adafruit GFX font Fixed size: 4x6

const uint8_t FixedBitmaps[] PROGMEM = {
  0xE8,	// exclam
  0xB4,	// quotedbl
  0xBE,0xFA,	// numbersign
  0x79,0xE4,	// dollar
  0x85,0x42,	// percent
  0xDB,0xD6,	// ampersand
  0xC0,	// quotesingle
  0x6A,0x40,	// parenleft
  0x95,0x80,	// parenright
  0xAA,0x80,	// asterisk
  0x5D,0x00,	// plus
  0x60,	// comma
  0xE0,	// hyphen
  0x80,	// period
  0x25,0x48,	// slash
  0x76,0xDC,	// zero
  0x75,0x40,	// one
  0xC5,0x4E,	// two
  0xC5,0x1C,	// three
  0xB7,0x92,	// four
  0xF3,0x1C,	// five
  0x73,0xDE,	// six
  0xE5,0x48,	// seven
  0xF7,0xDE,	// eight
  0xF7,0x9C,	// nine
  0xA0,	// colon
  0x46,	// semicolon
  0x2A,0x22,	// less
  0xE3,0x80,	// equal
  0x88,0xA8,	// greater
  0xE5,0x04,	// question
  0x57,0xC6,	// at
  0x57,0xDA,	// A
  0xD7,0x5C,	// B
  0x72,0x46,	// C
  0xD6,0xDC,	// D
  0xF3,0xCE,	// E
  0xF3,0xC8,	// F
  0x73,0xD6,	// G
  0xB7,0xDA,	// H
  0xE9,0x2E,	// I
  0x24,0xD4,	// J
  0xB7,0x5A,	// K
  0x92,0x4E,	// L
  0xBF,0xDA,	// M
  0xBF,0xFA,	// N
  0x56,0xD4,	// O
  0xD7,0x48,	// P
  0x56,0xF6,	// Q
  0xD7,0xEA,	// R
  0x71,0x1C,	// S
  0xE9,0x24,	// T
  0xB6,0xD6,	// U
  0xB6,0xA4,	// V
  0xB7,0xFA,	// W
  0xB5,0x5A,	// X
  0xB5,0x24,	// Y
  0xE5,0x4E,	// Z
  0xF2,0x4E,	// bracketleft
  0x88,0x80,	// backslash
  0xE4,0x9E,	// bracketright
  0x54,	// asciicircum
  0xE0,	// underscore
  0x90,	// grave
  0xCE,0xF0,	// a
  0x9A,0xDC,	// b
  0x72,0x30,	// c
  0x2E,0xD6,	// d
  0x77,0x30,	// e
  0x2B,0xA4,	// f
  0x77,0x94,	// g
  0x9A,0xDA,	// h
  0xB8,	// i
  0x20,0x9A,0x80,	// j
  0x97,0x6A,	// k
  0xC9,0x2E,	// l
  0xFF,0xD0,	// m
  0xD6,0xD0,	// n
  0x56,0xA0,	// o
  0xD6,0xE8,	// p
  0x76,0xB2,	// q
  0x72,0x40,	// r
  0x79,0xE0,	// s
  0x5D,0x26,	// t
  0xB6,0xB0,	// u
  0xB7,0xA0,	// v
  0xBF,0xF0,	// w
  0xA9,0x50,	// x
  0xB5,0x94,	// y
  0xEF,0x70,	// z
  0x6A,0x26,	// braceleft
  0xD8,	// bar
  0xC8,0xAC,	// braceright
  0x78	// asciitilde
};

const GFXglyph FixedGlyphs[] PROGMEM = {
  { 0, 0, 0, 4, 0, 0 },	// space
  { 0, 1, 5, 4, 1, -5 },	// exclam
  { 1, 3, 2, 4, 0, -5 },	// quotedbl
  { 2, 3, 5, 4, 0, -5 },	// numbersign
  { 4, 3, 5, 4, 0, -5 },	// dollar
  { 6, 3, 5, 4, 0, -5 },	// percent
  { 8, 3, 5, 4, 0, -5 },	// ampersand
  { 10, 1, 2, 4, 1, -5 },	// quotesingle
  { 11, 2, 5, 4, 1, -5 },	// parenleft
  { 13, 2, 5, 4, 0, -5 },	// parenright
  { 15, 3, 3, 4, 0, -5 },	// asterisk
  { 17, 3, 3, 4, 0, -4 },	// plus
  { 19, 2, 2, 4, 0, -2 },	// comma
  { 20, 3, 1, 4, 0, -3 },	// hyphen
  { 21, 1, 1, 4, 1, -1 },	// period
  { 22, 3, 5, 4, 0, -5 },	// slash
  { 24, 3, 5, 4, 0, -5 },	// zero
  { 26, 2, 5, 4, 0, -5 },	// one
  { 28, 3, 5, 4, 0, -5 },	// two
  { 30, 3, 5, 4, 0, -5 },	// three
  { 32, 3, 5, 4, 0, -5 },	// four
  { 34, 3, 5, 4, 0, -5 },	// five
  { 36, 3, 5, 4, 0, -5 },	// six
  { 38, 3, 5, 4, 0, -5 },	// seven
  { 40, 3, 5, 4, 0, -5 },	// eight
  { 42, 3, 5, 4, 0, -5 },	// nine
  { 44, 1, 3, 4, 1, -4 },	// colon
  { 45, 2, 4, 4, 0, -4 },	// semicolon
  { 46, 3, 5, 4, 0, -5 },	// less
  { 48, 3, 3, 4, 0, -4 },	// equal
  { 50, 3, 5, 4, 0, -5 },	// greater
  { 52, 3, 5, 4, 0, -5 },	// question
  { 54, 3, 5, 4, 0, -5 },	// at
  { 56, 3, 5, 4, 0, -5 },	// A
  { 58, 3, 5, 4, 0, -5 },	// B
  { 60, 3, 5, 4, 0, -5 },	// C
  { 62, 3, 5, 4, 0, -5 },	// D
  { 64, 3, 5, 4, 0, -5 },	// E
  { 66, 3, 5, 4, 0, -5 },	// F
  { 68, 3, 5, 4, 0, -5 },	// G
  { 70, 3, 5, 4, 0, -5 },	// H
  { 72, 3, 5, 4, 0, -5 },	// I
  { 74, 3, 5, 4, 0, -5 },	// J
  { 76, 3, 5, 4, 0, -5 },	// K
  { 78, 3, 5, 4, 0, -5 },	// L
  { 80, 3, 5, 4, 0, -5 },	// M
  { 82, 3, 5, 4, 0, -5 },	// N
  { 84, 3, 5, 4, 0, -5 },	// O
  { 86, 3, 5, 4, 0, -5 },	// P
  { 88, 3, 5, 4, 0, -5 },	// Q
  { 90, 3, 5, 4, 0, -5 },	// R
  { 92, 3, 5, 4, 0, -5 },	// S
  { 94, 3, 5, 4, 0, -5 },	// T
  { 96, 3, 5, 4, 0, -5 },	// U
  { 98, 3, 5, 4, 0, -5 },	// V
  { 100, 3, 5, 4, 0, -5 },	// W
  { 102, 3, 5, 4, 0, -5 },	// X
  { 104, 3, 5, 4, 0, -5 },	// Y
  { 106, 3, 5, 4, 0, -5 },	// Z
  { 108, 3, 5, 4, 0, -5 },	// bracketleft
  { 110, 3, 3, 4, 0, -4 },	// backslash
  { 112, 3, 5, 4, 0, -5 },	// bracketright
  { 114, 3, 2, 4, 0, -5 },	// asciicircum
  { 115, 3, 1, 4, 0, -1 },	// underscore
  { 116, 2, 2, 4, 0, -5 },	// grave
  { 117, 3, 4, 4, 0, -4 },	// a
  { 119, 3, 5, 4, 0, -5 },	// b
  { 121, 3, 4, 4, 0, -4 },	// c
  { 123, 3, 5, 4, 0, -5 },	// d
  { 125, 3, 4, 4, 0, -4 },	// e
  { 127, 3, 5, 4, 0, -5 },	// f
  { 129, 3, 5, 4, 0, -4 },	// g
  { 131, 3, 5, 4, 0, -5 },	// h
  { 133, 1, 5, 4, 1, -5 },	// i
  { 134, 3, 6, 4, 0, -5 },	// j
  { 137, 3, 5, 4, 0, -5 },	// k
  { 139, 3, 5, 4, 0, -5 },	// l
  { 141, 3, 4, 4, 0, -4 },	// m
  { 143, 3, 4, 4, 0, -4 },	// n
  { 145, 3, 4, 4, 0, -4 },	// o
  { 147, 3, 5, 4, 0, -4 },	// p
  { 149, 3, 5, 4, 0, -4 },	// q
  { 151, 3, 4, 4, 0, -4 },	// r
  { 153, 3, 4, 4, 0, -4 },	// s
  { 155, 3, 5, 4, 0, -5 },	// t
  { 157, 3, 4, 4, 0, -4 },	// u
  { 159, 3, 4, 4, 0, -4 },	// v
  { 161, 3, 4, 4, 0, -4 },	// w
  { 163, 3, 4, 4, 0, -4 },	// x
  { 165, 3, 5, 4, 0, -4 },	// y
  { 167, 3, 4, 4, 0, -4 },	// z
  { 169, 3, 5, 4, 0, -5 },	// braceleft
  { 171, 1, 5, 4, 1, -5 },	// bar
  { 172, 3, 5, 4, 0, -5 },	// braceright
  { 174, 3, 2, 4, 0, -5 }	// asciitilde
};

const GFXfont Fixed PROGMEM = {
 (uint8_t *)FixedBitmaps,
 (GFXglyph *)FixedGlyphs,
 32, 126, 6};
