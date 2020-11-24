// Adafruit GFX font Apple4x6 size: 4x6

const uint8_t Apple4x6Bitmaps[] PROGMEM = {
  0xE8,	// exclam
  0xB4,	// quotedbl
  0xAF,0xAF,0xA0,	// numbersign
  0x5F,0x1E,0x80,	// dollar
  0x85,0x42,	// percent
  0x4A,0x4A,0x50,	// ampersand
  0xC0,	// quotesingle
  0x6A,0x90,	// parenleft
  0x95,0x60,	// parenright
  0xAB,0xAA,	// asterisk
  0x4B,0xA4,	// plus
  0x60,	// comma
  0xE0,	// hyphen
  0x80,	// period
  0x25,0x48,	// slash
  0x57,0xD4,	// zero
  0x59,0x2E,	// one
  0x54,0xAE,	// two
  0xE5,0x1C,	// three
  0xB7,0x92,	// four
  0xF3,0x1C,	// five
  0x73,0x54,	// six
  0xE5,0x48,	// seven
  0x75,0x5C,	// eight
  0x55,0x9C,	// nine
  0x90,	// colon
  0x41,0x80,	// semicolon
  0x2A,0x22,	// less
  0xE3,0x80,	// equal
  0x88,0xA8,	// greater
  0xC5,0x04,	// question
  0x76,0xC6,	// at
  0x57,0xDA,	// A
  0xD7,0x5C,	// B
  0x56,0x54,	// C
  0xD6,0xDC,	// D
  0xF3,0x4E,	// E
  0xF3,0x48,	// F
  0x72,0xD6,	// G
  0xB7,0xDA,	// H
  0xE9,0x2E,	// I
  0x24,0xD4,	// J
  0xB7,0x5A,	// K
  0x92,0x4E,	// L
  0xBF,0xDA,	// M
  0x37,0xD8,	// N
  0x56,0xD4,	// O
  0xD7,0x48,	// P
  0x56,0xD4,0x40,	// Q
  0xD7,0x5A,	// R
  0x71,0x1C,	// S
  0xE9,0x24,	// T
  0xB6,0xDE,	// U
  0xB6,0xF4,	// V
  0xB7,0xFA,	// W
  0xB5,0x5A,	// X
  0xB5,0x24,	// Y
  0xE5,0x4E,	// Z
  0xEA,0xC0,	// bracketleft
  0x91,0x12,	// backslash
  0xD5,0xC0,	// bracketright
  0x54,	// asciicircum
  0xE0,	// underscore
  0x90,	// grave
  0x76,0xB0,	// a
  0x9A,0xDC,	// b
  0x72,0x30,	// c
  0x2E,0xD6,	// d
  0x57,0x30,	// e
  0x2B,0xA4,	// f
  0x75,0x9C,	// g
  0x9A,0xDA,	// h
  0x43,0x2E,	// i
  0x20,0x93,0x80,	// j
  0x97,0x5A,	// k
  0xC9,0x2E,	// l
  0xBE,0xD0,	// m
  0xD6,0xD0,	// n
  0x56,0xA0,	// o
  0xD7,0x48,	// p
  0x76,0xB2,	// q
  0xBA,0x40,	// r
  0x78,0xE0,	// s
  0x5D,0x22,	// t
  0xB6,0xB0,	// u
  0xB6,0xA0,	// v
  0xB7,0xD0,	// w
  0xA9,0x50,	// x
  0xB5,0x9C,	// y
  0xE5,0x70,	// z
  0x2B,0x24,0x40,	// braceleft
  0xF8,	// bar
  0x89,0xA5,0x00,	// braceright
  0x5A	// asciitilde
};

const GFXglyph Apple4x6Glyphs[] PROGMEM = {
  { 0, 0, 0, 4, 0, 0 },	// space
  { 0, 1, 5, 4, 1, -5 },	// exclam
  { 1, 3, 2, 4, 0, -5 },	// quotedbl
  { 2, 4, 5, 4, 0, -5 },	// numbersign
  { 5, 3, 6, 4, 0, -5 },	// dollar
  { 8, 3, 5, 4, 0, -5 },	// percent
  { 10, 4, 5, 4, 0, -5 },	// ampersand
  { 13, 1, 2, 4, 1, -5 },	// quotesingle
  { 14, 2, 6, 4, 1, -5 },	// parenleft
  { 16, 2, 6, 4, 0, -5 },	// parenright
  { 18, 3, 5, 4, 0, -5 },	// asterisk
  { 20, 3, 5, 4, 0, -5 },	// plus
  { 22, 2, 2, 4, 0, -1 },	// comma
  { 23, 3, 1, 4, 0, -3 },	// hyphen
  { 24, 1, 1, 4, 1, -1 },	// period
  { 25, 3, 5, 4, 0, -5 },	// slash
  { 27, 3, 5, 4, 0, -5 },	// zero
  { 29, 3, 5, 4, 0, -5 },	// one
  { 31, 3, 5, 4, 0, -5 },	// two
  { 33, 3, 5, 4, 0, -5 },	// three
  { 35, 3, 5, 4, 0, -5 },	// four
  { 37, 3, 5, 4, 0, -5 },	// five
  { 39, 3, 5, 4, 0, -5 },	// six
  { 41, 3, 5, 4, 0, -5 },	// seven
  { 43, 3, 5, 4, 0, -5 },	// eight
  { 45, 3, 5, 4, 0, -5 },	// nine
  { 47, 1, 4, 4, 1, -4 },	// colon
  { 48, 2, 5, 4, 0, -4 },	// semicolon
  { 50, 3, 5, 4, 0, -5 },	// less
  { 52, 3, 3, 4, 0, -4 },	// equal
  { 54, 3, 5, 4, 0, -5 },	// greater
  { 56, 3, 5, 4, 0, -5 },	// question
  { 58, 3, 5, 4, 0, -5 },	// at
  { 60, 3, 5, 4, 0, -5 },	// A
  { 62, 3, 5, 4, 0, -5 },	// B
  { 64, 3, 5, 4, 0, -5 },	// C
  { 66, 3, 5, 4, 0, -5 },	// D
  { 68, 3, 5, 4, 0, -5 },	// E
  { 70, 3, 5, 4, 0, -5 },	// F
  { 72, 3, 5, 4, 0, -5 },	// G
  { 74, 3, 5, 4, 0, -5 },	// H
  { 76, 3, 5, 4, 0, -5 },	// I
  { 78, 3, 5, 4, 0, -5 },	// J
  { 80, 3, 5, 4, 0, -5 },	// K
  { 82, 3, 5, 4, 0, -5 },	// L
  { 84, 3, 5, 4, 0, -5 },	// M
  { 86, 3, 5, 4, 0, -5 },	// N
  { 88, 3, 5, 4, 0, -5 },	// O
  { 90, 3, 5, 4, 0, -5 },	// P
  { 92, 3, 6, 4, 0, -5 },	// Q
  { 95, 3, 5, 4, 0, -5 },	// R
  { 97, 3, 5, 4, 0, -5 },	// S
  { 99, 3, 5, 4, 0, -5 },	// T
  { 101, 3, 5, 4, 0, -5 },	// U
  { 103, 3, 5, 4, 0, -5 },	// V
  { 105, 3, 5, 4, 0, -5 },	// W
  { 107, 3, 5, 4, 0, -5 },	// X
  { 109, 3, 5, 4, 0, -5 },	// Y
  { 111, 3, 5, 4, 0, -5 },	// Z
  { 113, 2, 5, 4, 1, -5 },	// bracketleft
  { 115, 3, 5, 4, 0, -5 },	// backslash
  { 117, 2, 5, 4, 0, -5 },	// bracketright
  { 119, 3, 2, 4, 0, -5 },	// asciicircum
  { 120, 3, 1, 4, 0, 0 },	// underscore
  { 121, 2, 2, 4, 1, -5 },	// grave
  { 122, 3, 4, 4, 0, -4 },	// a
  { 124, 3, 5, 4, 0, -5 },	// b
  { 126, 3, 4, 4, 0, -4 },	// c
  { 128, 3, 5, 4, 0, -5 },	// d
  { 130, 3, 4, 4, 0, -4 },	// e
  { 132, 3, 5, 4, 0, -5 },	// f
  { 134, 3, 5, 4, 0, -4 },	// g
  { 136, 3, 5, 4, 0, -5 },	// h
  { 138, 3, 5, 4, 0, -5 },	// i
  { 140, 3, 6, 4, 0, -5 },	// j
  { 143, 3, 5, 4, 0, -5 },	// k
  { 145, 3, 5, 4, 0, -5 },	// l
  { 147, 3, 4, 4, 0, -4 },	// m
  { 149, 3, 4, 4, 0, -4 },	// n
  { 151, 3, 4, 4, 0, -4 },	// o
  { 153, 3, 5, 4, 0, -4 },	// p
  { 155, 3, 5, 4, 0, -4 },	// q
  { 157, 3, 4, 4, 0, -4 },	// r
  { 159, 3, 4, 4, 0, -4 },	// s
  { 161, 3, 5, 4, 0, -5 },	// t
  { 163, 3, 4, 4, 0, -4 },	// u
  { 165, 3, 4, 4, 0, -4 },	// v
  { 167, 3, 4, 4, 0, -4 },	// w
  { 169, 3, 4, 4, 0, -4 },	// x
  { 171, 3, 5, 4, 0, -4 },	// y
  { 173, 3, 4, 4, 0, -4 },	// z
  { 175, 3, 6, 4, 0, -5 },	// braceleft
  { 178, 1, 5, 4, 1, -5 },	// bar
  { 179, 3, 6, 4, 0, -5 },	// braceright
  { 182, 4, 2, 4, 0, -5 }	// asciitilde
};

const GFXfont Apple4x6 PROGMEM = {
 (uint8_t *)Apple4x6Bitmaps,
 (GFXglyph *)Apple4x6Glyphs,
 32, 126, 6};
