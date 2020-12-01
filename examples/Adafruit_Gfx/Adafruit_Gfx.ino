/*
 * This example shows how to use the Adafruit_GFX compatible Layers in SmartMatrix Library
 * This example was adapted from Adafruit's FX example for the Adafruit ILI9341 Breakout and Shield, see text below
 */

/***************************************************
  This is our GFX example for the Adafruit ILI9341 Breakout and Shield
  ----> http://www.adafruit.com/products/1651
  Check out the links above for our tutorials and wiring diagrams
  These displays use SPI to communicate, 4 or 5 pins are required to
  interface (RST is optional)
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!
  Written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 ****************************************************/

#define USE_ADAFRUIT_GFX_LAYERS

// uncomment one line to select your MatrixHardware configuration - configuration header needs to be included before <SmartMatrix.h>
//#include <MatrixHardware_Teensy3_ShieldV4.h>        // SmartLED Shield for Teensy 3 (V4)
//#include <MatrixHardware_Teensy4_ShieldV5.h>        // SmartLED Shield for Teensy 4 (V5)
//#include <MatrixHardware_Teensy3_ShieldV1toV3.h>    // SmartMatrix Shield for Teensy 3 V1-V3
//#include <MatrixHardware_Teensy4_ShieldV4Adapter.h> // Teensy 4 Adapter attached to SmartLED Shield for Teensy 3 (V4)
//#include <MatrixHardware_ESP32_V0.h>                // This file contains multiple ESP32 hardware configurations, edit the file to define GPIOPINOUT (or add #define GPIOPINOUT with a hardcoded number before this #include)
//#include "MatrixHardware_Custom.h"                  // Copy an existing MatrixHardware file to your Sketch directory, rename, customize, and you can include it like this
#include <SmartMatrix.h>

#define COLOR_DEPTH 24                  // Choose the color depth used for storing pixels in the layers: 24 or 48 (24 is good for most sketches - If the sketch uses type `rgb24` directly, COLOR_DEPTH must be 24)
const uint16_t kMatrixWidth = 32;       // Set to the width of your display, must be a multiple of 8
const uint16_t kMatrixHeight = 32;      // Set to the height of your display
const uint8_t kRefreshDepth = 36;       // Tradeoff of color quality vs refresh rate, max brightness, and RAM usage.  36 is typically good, drop down to 24 if you need to.  On Teensy, multiples of 3, up to 48: 3, 6, 9, 12, 15, 18, 21, 24, 27, 30, 33, 36, 39, 42, 45, 48.  On ESP32: 24, 36, 48
const uint8_t kDmaBufferRows = 4;       // known working: 2-4, use 2 to save RAM, more to keep from dropping frames and automatically lowering refresh rate.  (This isn't used on ESP32, leave as default)
const uint8_t kPanelType = SM_PANELTYPE_HUB75_32ROW_MOD16SCAN;   // Choose the configuration that matches your panels.  See more details in MatrixCommonHub75.h and the docs: https://github.com/pixelmatix/SmartMatrix/wiki
const uint32_t kMatrixOptions = (SM_HUB75_OPTIONS_NONE);        // see docs for options: https://github.com/pixelmatix/SmartMatrix/wiki

SMARTMATRIX_ALLOCATE_BUFFERS(matrix, kMatrixWidth, kMatrixHeight, kRefreshDepth, kDmaBufferRows, kPanelType, kMatrixOptions);

// There are two Adafruit_GFX compatible layers, a Mono layer and an RGB layer, and this example sketch works with either (choose one):
#if 1
  const uint8_t kBackgroundLayerOptions = (SM_BACKGROUND_OPTIONS_NONE);
  SMARTMATRIX_ALLOCATE_BACKGROUND_LAYER(backgroundLayer, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kBackgroundLayerOptions);
#else
  const uint8_t kMonoLayerOptions = (SM_GFX_MONO_OPTIONS_NONE);
  SMARTMATRIX_ALLOCATE_GFX_MONO_LAYER(backgroundLayer, kMatrixWidth, kMatrixHeight, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kMonoLayerOptions);
  
  // these backwards compatible ALLOCATE macros also work:
  //SMARTMATRIX_ALLOCATE_SCROLLING_LAYER(backgroundLayer, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kMonoLayerOptions);
  //SMARTMATRIX_ALLOCATE_INDEXED_LAYER(backgroundLayer, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kMonoLayerOptions);
#endif

// Assign human-readable names to some common 16-bit color values:
#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

// enable this to see the intermediate drawing steps, otherwise screen will only be updated at the end of each test (this slows the tests down considerably)
const bool swapAfterEveryDraw = false;

unsigned long testFillScreen() {
  unsigned long start = micros();
  backgroundLayer.fillScreen(BLACK);
  backgroundLayer.fillScreen(RED);
  backgroundLayer.fillScreen(GREEN);
  backgroundLayer.fillScreen(BLUE);
  backgroundLayer.fillScreen(BLACK);
  return micros() - start;
}

unsigned long testText() {
  backgroundLayer.fillScreen(BLACK);
  unsigned long start = micros();
  backgroundLayer.setCursor(0, 0);
  backgroundLayer.setTextColor(WHITE);  backgroundLayer.setTextSize(1);
  backgroundLayer.println("Hello World!");
  backgroundLayer.setTextColor(YELLOW); backgroundLayer.setTextSize(2);
  backgroundLayer.println(1234.56);
  backgroundLayer.setTextColor(RED);    backgroundLayer.setTextSize(3);
//  backgroundLayer.println(0xDEADBEEF, HEX);
//  backgroundLayer.println();
  backgroundLayer.setTextColor(GREEN);
  backgroundLayer.setTextSize(5);
  backgroundLayer.println("Groop");
  backgroundLayer.setTextSize(2);
  backgroundLayer.println("I implore thee,");
  backgroundLayer.setTextSize(1);
  backgroundLayer.println("my foonting turlingdromes.");
  backgroundLayer.println("And hooptiously drangle me");
  backgroundLayer.println("with crinkly bindlewurdles,");
  backgroundLayer.println("Or I will rend thee");
  backgroundLayer.println("in the gobberwarts");
  backgroundLayer.println("with my blurglecruncheon,");
  backgroundLayer.println("see if I don't!");
  return micros() - start;
}

unsigned long testLines(uint16_t color) {
  unsigned long start, t;
  int           x1, y1, x2, y2,
                w = backgroundLayer.width(),
                h = backgroundLayer.height();

  backgroundLayer.fillScreen(BLACK);

  x1 = y1 = 0;
  y2    = h - 1;
  start = micros();
  for(x2=0; x2<w; x2+=6) {
    backgroundLayer.drawLine(x1, y1, x2, y2, color);
    if(swapAfterEveryDraw) backgroundLayer.swapBuffers();
  }
  x2    = w - 1;
  for(y2=0; y2<h; y2+=6) {
    backgroundLayer.drawLine(x1, y1, x2, y2, color);
    if(swapAfterEveryDraw) backgroundLayer.swapBuffers();
  }
  t     = micros() - start; // fillScreen doesn't count against timing
  backgroundLayer.swapBuffers();

  backgroundLayer.fillScreen(BLACK);

  x1    = w - 1;
  y1    = 0;
  y2    = h - 1;
  start = micros();
  for(x2=0; x2<w; x2+=6) {
    backgroundLayer.drawLine(x1, y1, x2, y2, color);
    if(swapAfterEveryDraw) backgroundLayer.swapBuffers();
  }
  x2    = 0;
  for(y2=0; y2<h; y2+=6) {
    backgroundLayer.drawLine(x1, y1, x2, y2, color);
    if(swapAfterEveryDraw) backgroundLayer.swapBuffers();
  }
  t    += micros() - start;
  backgroundLayer.swapBuffers();

  backgroundLayer.fillScreen(BLACK);

  x1    = 0;
  y1    = h - 1;
  y2    = 0;
  start = micros();
  for(x2=0; x2<w; x2+=6) {
    backgroundLayer.drawLine(x1, y1, x2, y2, color);
    if(swapAfterEveryDraw) backgroundLayer.swapBuffers();
  }
  x2    = w - 1;
  for(y2=0; y2<h; y2+=6) {
    backgroundLayer.drawLine(x1, y1, x2, y2, color);
    if(swapAfterEveryDraw) backgroundLayer.swapBuffers();
  }
  t    += micros() - start;
  backgroundLayer.swapBuffers();

  backgroundLayer.fillScreen(BLACK);

  x1    = w - 1;
  y1    = h - 1;
  y2    = 0;
  start = micros();
  for(x2=0; x2<w; x2+=6) {
    backgroundLayer.drawLine(x1, y1, x2, y2, color);
    if(swapAfterEveryDraw) backgroundLayer.swapBuffers();
  }
  x2    = 0;
  for(y2=0; y2<h; y2+=6) {
    backgroundLayer.drawLine(x1, y1, x2, y2, color);
    if(swapAfterEveryDraw) backgroundLayer.swapBuffers();
  }
  t    += micros() - start;
  backgroundLayer.swapBuffers();

  return t;
}
unsigned long testFastLines(uint16_t color1, uint16_t color2) {
  unsigned long start;
  int           x, y, w = backgroundLayer.width(), h = backgroundLayer.height();

  backgroundLayer.fillScreen(BLACK);
  start = micros();
  for(y=0; y<h; y+=5) {
    backgroundLayer.drawFastHLine(0, y, w, color1);
    if(swapAfterEveryDraw) backgroundLayer.swapBuffers();
  }
  for(x=0; x<w; x+=5) {
    backgroundLayer.drawFastVLine(x, 0, h, color2);
    if(swapAfterEveryDraw) backgroundLayer.swapBuffers();
  }

  return micros() - start;
}

unsigned long testRects(uint16_t color) {
  unsigned long start;
  int           n, i, i2,
                cx = backgroundLayer.width()  / 2,
                cy = backgroundLayer.height() / 2;

  backgroundLayer.fillScreen(BLACK);
  n     = min(backgroundLayer.width(), backgroundLayer.height());
  start = micros();
  for(i=2; i<n; i+=6) {
    i2 = i / 2;
    backgroundLayer.drawRect(cx-i2, cy-i2, i, i, color);
    if(swapAfterEveryDraw) backgroundLayer.swapBuffers();
  }

  return micros() - start;
}

unsigned long testFilledRects(uint16_t color1, uint16_t color2) {
  unsigned long start, t = 0;
  int           n, i, i2,
                cx = backgroundLayer.width()  / 2 - 1,
                cy = backgroundLayer.height() / 2 - 1;

  backgroundLayer.fillScreen(BLACK);
  n = min(backgroundLayer.width(), backgroundLayer.height());
  for(i=n; i>0; i-=6) {
    i2    = i / 2;
    start = micros();
    backgroundLayer.fillRect(cx-i2, cy-i2, i, i, color1);
    if(swapAfterEveryDraw) backgroundLayer.swapBuffers();
    t    += micros() - start;
    // Outlines are not included in timing results
    backgroundLayer.drawRect(cx-i2, cy-i2, i, i, color2);
    if(swapAfterEveryDraw) backgroundLayer.swapBuffers();
  }

  return t;
}

unsigned long testFilledCircles(uint8_t radius, uint16_t color) {
  unsigned long start;
  int x, y, w = backgroundLayer.width(), h = backgroundLayer.height(), r2 = radius * 2;

  backgroundLayer.fillScreen(BLACK);
  start = micros();
  for(x=radius; x<w; x+=r2) {
    for(y=radius; y<h; y+=r2) {
      backgroundLayer.fillCircle(x, y, radius, color);
      if(swapAfterEveryDraw) backgroundLayer.swapBuffers();
    }
  }

  return micros() - start;
}

unsigned long testCircles(uint8_t radius, uint16_t color) {
  unsigned long start;
  int           x, y, r2 = radius * 2,
                w = backgroundLayer.width()  + radius,
                h = backgroundLayer.height() + radius;

  // Screen is not cleared for this one -- this is
  // intentional and does not affect the reported time.
  start = micros();
  for(x=0; x<w; x+=r2) {
    for(y=0; y<h; y+=r2) {
      backgroundLayer.drawCircle(x, y, radius, color);
      if(swapAfterEveryDraw) backgroundLayer.swapBuffers();
    }
  }

  return micros() - start;
}

unsigned long testTriangles() {
  unsigned long start;
  int           n, i, cx = backgroundLayer.width()  / 2 - 1,
                      cy = backgroundLayer.height() / 2 - 1;

  backgroundLayer.fillScreen(BLACK);
  n     = min(cx, cy);
  start = micros();
  for(i=0; i<n; i+=5) {
    backgroundLayer.drawTriangle(
      cx    , cy - i, // peak
      cx - i, cy + i, // bottom left
      cx + i, cy + i, // bottom right
      backgroundLayer.color565(0, 0, i));
    if(swapAfterEveryDraw) backgroundLayer.swapBuffers();
  }

  return micros() - start;
}

unsigned long testFilledTriangles() {
  unsigned long start, t = 0;
  int           i, cx = backgroundLayer.width()  / 2 - 1,
                   cy = backgroundLayer.height() / 2 - 1;

  backgroundLayer.fillScreen(BLACK);
  start = micros();
  for(i=min(cx,cy); i>10; i-=5) {
    start = micros();
    backgroundLayer.fillTriangle(cx, cy - i, cx - i, cy + i, cx + i, cy + i,
      backgroundLayer.color565(0, i, i));
    t += micros() - start;
    if(swapAfterEveryDraw) backgroundLayer.swapBuffers();
    backgroundLayer.drawTriangle(cx, cy - i, cx - i, cy + i, cx + i, cy + i,
      backgroundLayer.color565(i, i, 0));
    if(swapAfterEveryDraw) backgroundLayer.swapBuffers();
  }

  return t;
}

unsigned long testRoundRects() {
  unsigned long start;
  int           w, i, i2,
                cx = backgroundLayer.width()  / 2 - 1,
                cy = backgroundLayer.height() / 2 - 1;

  backgroundLayer.fillScreen(BLACK);
  w     = min(backgroundLayer.width(), backgroundLayer.height());
  start = micros();
  for(i=0; i<w; i+=6) {
    i2 = i / 2;
    backgroundLayer.drawRoundRect(cx-i2, cy-i2, i, i, i/8, backgroundLayer.color565(i, 0, 0));
    if(swapAfterEveryDraw) backgroundLayer.swapBuffers();
  }

  return micros() - start;
}

unsigned long testFilledRoundRects() {
  unsigned long start;
  int           i, i2,
                cx = backgroundLayer.width()  / 2 - 1,
                cy = backgroundLayer.height() / 2 - 1;

  backgroundLayer.fillScreen(BLACK);
  start = micros();
  for(i=min(backgroundLayer.width(), backgroundLayer.height()); i>20; i-=6) {
    i2 = i / 2;
    backgroundLayer.fillRoundRect(cx-i2, cy-i2, i, i, i/8, backgroundLayer.color565(0, i, 0));
    if(swapAfterEveryDraw) backgroundLayer.swapBuffers();
  }

  return micros() - start;
}

void setup() {
  Serial.begin(115200);
  matrix.addLayer(&backgroundLayer); 
  matrix.begin();

  // wait for Serial to be ready
  delay(1000);

  matrix.setBrightness(128);

  Serial.println(F("Benchmark                Time (microseconds)"));

  Serial.print(F("Screen fill              "));
  Serial.println(testFillScreen());
  backgroundLayer.swapBuffers();
  delay(500);

  Serial.print(F("Text                     "));
  Serial.println(testText());
  backgroundLayer.swapBuffers();
  delay(3000);

  Serial.print(F("Lines                    "));
  Serial.println(testLines(CYAN));
  backgroundLayer.swapBuffers();
  delay(500);

  Serial.print(F("Horiz/Vert Lines         "));
  Serial.println(testFastLines(RED, BLUE));
  backgroundLayer.swapBuffers();
  delay(500);

  Serial.print(F("Rectangles (outline)     "));
  Serial.println(testRects(GREEN));
  backgroundLayer.swapBuffers();
  delay(500);

  Serial.print(F("Rectangles (filled)      "));
  Serial.println(testFilledRects(YELLOW, MAGENTA));
  backgroundLayer.swapBuffers();
  delay(500);

  Serial.print(F("Circles (filled)         "));
  Serial.println(testFilledCircles(10, MAGENTA));
  backgroundLayer.swapBuffers();
  delay(500);

  Serial.print(F("Circles (outline)        "));
  Serial.println(testCircles(10, WHITE));
  backgroundLayer.swapBuffers();
  delay(500);

  Serial.print(F("Triangles (outline)      "));
  Serial.println(testTriangles());
  backgroundLayer.swapBuffers();
  delay(500);

  Serial.print(F("Triangles (filled)       "));
  Serial.println(testFilledTriangles());
  backgroundLayer.swapBuffers();
  delay(500);

  Serial.print(F("Rounded rects (outline)  "));
  Serial.println(testRoundRects());
  backgroundLayer.swapBuffers();
  delay(500);

  Serial.print(F("Rounded rects (filled)   "));
  Serial.println(testFilledRoundRects());
  backgroundLayer.swapBuffers();
  delay(500);

  Serial.println(F("Done!"));
}

void loop() {
  for(uint8_t rotation=0; rotation<4; rotation++) {
    backgroundLayer.setRotation(rotation);
    testText();
    backgroundLayer.swapBuffers();
    delay(1000);
  }
}
