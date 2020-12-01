/*
 * Modified example copied from FastLED 3.0 Branch - originally written by Daniel Garcia
 * This example shows how to use some of FastLED's functions with the SmartMatrix Library
 * using the SmartMatrix buffers directly instead of FastLED's buffers.
 * FastLED's dithering and color balance features can't be used this way, but SmartMatrix can draw in
 * 36-bit color and so dithering may not provide much advantage.
 *
 * This version of the example shows how to draw to an APA102 strip connected to the SmartLED Shield
 * for Teensy 3 (V4) or SmartLED Shield for Teensy 4 (V5) in parallel with the matrix panel.
 *
 * The ESP32 platorm is not supported
 *
 * Note that the APA102 matrix is mapped inside the controller with normal rectangular coordiantes,
 *   but is shifted out to the panel in serpentine layout.
 *
 * This example requires FastLED 3.0 or higher.  If you are having trouble compiling, see
 * the troubleshooting instructions here:
 * https://github.com/pixelmatix/SmartMatrix/#external-libraries
 */

// uncomment one line to select your MatrixHardware configuration - configuration header needs to be included before <SmartMatrix.h>
//#include <MatrixHardware_Teensy3_ShieldV4.h>        // SmartLED Shield for Teensy 3 (V4)
//#include <MatrixHardware_Teensy4_ShieldV5.h>        // SmartLED Shield for Teensy 4 (V5)
//#include <MatrixHardware_Teensy3_ShieldV1toV3.h>    // SmartMatrix Shield for Teensy 3 V1-V3
//#include <MatrixHardware_Teensy4_ShieldV4Adapter.h> // Teensy 4 Adapter attached to SmartLED Shield for Teensy 3 (V4)
//#include <MatrixHardware_ESP32_V0.h>                // This file contains multiple ESP32 hardware configurations, edit the file to define GPIOPINOUT (or add #define GPIOPINOUT with a hardcoded number before this #include)
//#include "MatrixHardware_Custom.h"                  // Copy an existing MatrixHardware file to your Sketch directory, rename, customize, and you can include it like this
#include <SmartMatrix.h>
#include <FastLED.h>

#define ENABLE_HUB75_REFRESH    1
#define ENABLE_APA102_REFRESH   1

#define COLOR_DEPTH 24                  // Choose the color depth used for storing pixels in the layers: 24 or 48 (24 is good for most sketches - If the sketch uses type `rgb24` directly, COLOR_DEPTH must be 24)

#if (ENABLE_HUB75_REFRESH == 1)
const uint16_t kMatrixWidth = 32;       // Set to the width of your display, must be a multiple of 8
const uint16_t kMatrixHeight = 32;      // Set to the height of your display
const uint8_t kRefreshDepth = 36;       // Tradeoff of color quality vs refresh rate, max brightness, and RAM usage.  36 is typically good, drop down to 24 if you need to.  On Teensy, multiples of 3, up to 48: 3, 6, 9, 12, 15, 18, 21, 24, 27, 30, 33, 36, 39, 42, 45, 48.  On ESP32: 24, 36, 48
const uint8_t kDmaBufferRows = 4;       // known working: 2-4, use 2 to save RAM, more to keep from dropping frames and automatically lowering refresh rate.  (This isn't used on ESP32, leave as default)
const uint8_t kPanelType = SM_PANELTYPE_HUB75_32ROW_MOD16SCAN;   // Choose the configuration that matches your panels.  See more details in MatrixCommonHub75.h and the docs: https://github.com/pixelmatix/SmartMatrix/wiki
const uint32_t kMatrixOptions = (SM_HUB75_OPTIONS_NONE);        // see docs for options: https://github.com/pixelmatix/SmartMatrix/wiki
const uint8_t kBackgroundLayerOptions = (SM_BACKGROUND_OPTIONS_NONE);
const uint8_t kScrollingLayerOptions = (SM_SCROLLING_OPTIONS_NONE);

SMARTMATRIX_ALLOCATE_BUFFERS(matrix, kMatrixWidth, kMatrixHeight, kRefreshDepth, kDmaBufferRows, kPanelType, kMatrixOptions);
SMARTMATRIX_ALLOCATE_BACKGROUND_LAYER(backgroundLayer, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kBackgroundLayerOptions);
SMARTMATRIX_ALLOCATE_SCROLLING_LAYER(scrollingLayer, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kScrollingLayerOptions);
#endif

#if (ENABLE_APA102_REFRESH == 1)
const uint8_t kApaMatrixWidth = 8;          // adjust this to your APA matrix/strip
const uint8_t kApaMatrixHeight = 8;         // set kApaMatrixHeight to 1 for a strip
const uint8_t kApaRefreshDepth = 36;        // not used for APA matrices as of now
const uint8_t kApaDmaBufferRows = 1;        // not used for APA matrices as of now
const uint8_t kApaPanelType = 0;            // not used for APA matrices as of now
const uint32_t kApaMatrixOptions = (SM_APA102_OPTIONS_COLOR_ORDER_BGR);      // The default color order is BGR, change here to match your LEDs
const uint8_t kApaBackgroundLayerOptions = (SM_BACKGROUND_OPTIONS_NONE);

SMARTMATRIX_APA_ALLOCATE_BUFFERS(apamatrix, kApaMatrixWidth, kApaMatrixHeight, kApaRefreshDepth, kApaDmaBufferRows, kApaPanelType, kApaMatrixOptions);
SMARTMATRIX_ALLOCATE_BACKGROUND_LAYER(apaBackgroundLayer, kApaMatrixWidth, kApaMatrixHeight, COLOR_DEPTH, kApaBackgroundLayerOptions);
#endif

// The 32bit version of our coordinates
static uint16_t x;
static uint16_t y;
static uint16_t z;

// We're using the x/y dimensions to map to the x/y pixels on the matrix.  We'll
// use the z-axis for "time".  speed determines how fast time moves forward.  Try
// 1 for a very slow moving effect, or 60 for something that ends up looking like
// water.
uint16_t speed = 1; // almost looks like a painting, moves very slowly
//uint16_t speed = 20; // a nice starting speed, mixes well with a scale of 100
// uint16_t speed = 33;
// uint16_t speed = 100; // wicked fast!

// Scale determines how far apart the pixels in our noise matrix are.  Try
// changing these values around to see how it affects the motion of the display.  The
// higher the value of scale, the more "zoomed out" the noise iwll be.  A value
// of 1 will be so zoomed in, you'll mostly see solid colors.

// uint16_t scale = 1; // mostly just solid colors
// uint16_t scale = 4011; // very zoomed out and shimmery
uint16_t scale = 31;

#if (ENABLE_APA102_REFRESH == 1)
#define MAX_DIMENSION_APA ((kApaMatrixWidth>kApaMatrixHeight) ? kApaMatrixWidth : kApaMatrixHeight)
#else
#define MAX_DIMENSION_APA 0
#endif

#if (ENABLE_HUB75_REFRESH == 1)
#define MAX_DIMENSION_PANEL ((kMatrixWidth>kMatrixHeight) ? kMatrixWidth : kMatrixHeight)
#else
#define MAX_DIMENSION_PANEL 0
#endif

#define MAX_DIMENSION_OVERALL ((MAX_DIMENSION_APA>MAX_DIMENSION_PANEL) ? MAX_DIMENSION_APA : MAX_DIMENSION_PANEL)

// This is the array that we keep our computed noise values in
uint8_t noise[MAX_DIMENSION_OVERALL][MAX_DIMENSION_OVERALL];

uint16_t XY(uint8_t x, uint8_t y) {
  return kApaMatrixWidth * y + x;
}

rgb24 *buffer;

const uint16_t NUM_LEDS = kApaMatrixWidth * kApaMatrixHeight;

// scale the brightness of all pixels down
void dimAll(byte value)
{
  for (int i = 0; i < NUM_LEDS; i++) {
    CRGB c = CRGB(buffer[i].red, buffer[i].green, buffer[i].blue);
    c.nscale8(value);
    buffer[i] = c;
  }
}


void setup() {
  // Enable printing FPS count information
  Serial.begin(115200);

#if (ENABLE_HUB75_REFRESH == 1)
  matrix.addLayer(&backgroundLayer); 
  matrix.addLayer(&scrollingLayer); 
  //matrix.setBrightness(0);
  matrix.begin();

  // lower the brightness
  matrix.setBrightness(128);
#endif

  // Wait for Serial to be ready
  delay(1000);

#if (ENABLE_APA102_REFRESH == 1)
  // enable the APA102 buffers to drive out the SPI signals
  if(!SMARTLED_APA_ENABLED_BY_DEFAULT) {
  //if(1) {
    pinMode(SMARTLED_APA_ENABLE_PIN, OUTPUT);
    digitalWrite(SMARTLED_APA_ENABLE_PIN, HIGH);  // enable access to LEDs
  }

  apamatrix.addLayer(&apaBackgroundLayer);

  // The default SPI clock speed is 5MHz.  If you want to use a different clock speed, call this function _before_ begin()
  //apamatrix.setSpiClockSpeed(15000000); // 16MHz is about as fast as the Teensy 3 can support reliably
  //apamatrix.setSpiClockSpeed(40000000); // The Teensy 4 can go much faster, too fast for APA102 LEDs
  //apamatrix.setSpiClockSpeed(20000000); // 20MHz is a reasonable speed for a short chain (e.g. 256 LEDs)
  //apamatrix.setSpiClockSpeed(5000000); // A long chain of APA102 LEDs can't support a high clock rate, hence the default of 5MHz

  apamatrix.begin();

  // lower the brightness
  //apamatrix.setBrightness(128);
  apamatrix.setBrightness(32);
#endif

  // Initialize our coordinates to some random values
  x = random16();
  y = random16();
  z = random16();

  // Show off smart matrix scrolling text
#if (ENABLE_HUB75_REFRESH == 1)
  scrollingLayer.setMode(wrapForward);
  scrollingLayer.setColor({0xff, 0xff, 0xff});
  scrollingLayer.setSpeed(15);
  scrollingLayer.setFont(font6x10);
  scrollingLayer.start("SmartMatrix & FastLED", -1);
  scrollingLayer.setOffsetFromTop((kMatrixHeight/2) - 5);
#endif
}

// Fill the x/y array of 8-bit noise values using the inoise8 function.
void fillnoise8() {
  for(int i = 0; i < MAX_DIMENSION_OVERALL; i++) {
    int ioffset = scale * i;
    for(int j = 0; j < MAX_DIMENSION_OVERALL; j++) {
      int joffset = scale * j;
      noise[i][j] = inoise8(x + ioffset,y + joffset,z);
    }
  }
  z += speed;
}

#if 1
void loop() {
  static uint8_t ihue=0;

  fillnoise8();

  // if sketch uses swapBuffers(false), wait to get a new backBuffer() pointer after the swap is done:
#if (ENABLE_HUB75_REFRESH == 1)
  static uint8_t circlex = 0;
  static uint8_t circley = 0;

  while(backgroundLayer.isSwapPending());
  rgb24 *buffer = backgroundLayer.backBuffer();

  for(int i = 0; i < kMatrixWidth; i++) {
    for(int j = 0; j < kMatrixHeight; j++) {
      // We use the value at the (i,j) coordinate in the noise
      // array for our brightness, and the flipped value from (j,i)
      // for our pixel's hue.
      buffer[kMatrixWidth*j + i] = CRGB(CHSV(noise[j][i],255,noise[i][j]));

      // You can also explore other ways to constrain the hue used, like below
      // buffer[kMatrixHeight*j + i] = CRGB(CHSV(ihue + (noise[j][i]>>2),255,noise[i][j]));
    }
  }

  backgroundLayer.fillCircle(circlex % kMatrixWidth,circley % kMatrixHeight,6,CRGB(CHSV(ihue+128,255,255)));
  circlex += random16(2);
  circley += random16(2);

  // buffer is filled completely each time, use swapBuffers without buffer copy to save CPU cycles
  backgroundLayer.swapBuffers(false);
#endif

#if (ENABLE_APA102_REFRESH == 1)
  while(apaBackgroundLayer.isSwapPending());
  rgb24 *apabuffer = apaBackgroundLayer.backBuffer();
    
  for(int i = 0; i < kApaMatrixWidth; i++) {
    for(int j = 0; j < kApaMatrixHeight; j++) {
      // We use the value at the (i,j) coordinate in the noise
      // array for our brightness, and the flipped value from (j,i)
      // for our pixel's hue.
      apabuffer[kApaMatrixWidth*j + i] = CRGB(CHSV(noise[j][i],255,noise[i][j]));

      // You can also explore other ways to constrain the hue used, like below
      // apabuffer[kApaMatrixWidth*j + i] = CHSV(ihue + (noise[j][i]>>2),255,noise[i][j]);
    }
  }

  // buffer is filled completely each time, use swapBuffers without buffer copy to save CPU cycles
  apaBackgroundLayer.swapBuffers(false);

#endif

  ihue+=1;

  matrix.countFPS();      // print the loop() frames per second to Serial
}
#else
void loop() {
  EVERY_N_MILLISECONDS(1000/30) {

    while(apaBackgroundLayer.isSwapPending());
    buffer = apaBackgroundLayer.backBuffer();

    dimAll(250);

    static uint8_t theta = 0;
    static uint8_t hue = 0;
    
    for (uint8_t x = 0; x < kApaMatrixWidth; x++) {
      uint8_t y = quadwave8(x * 2 + theta) / scale;
      if(y < kApaMatrixHeight) {
        buffer[XY(x, y)] = CRGB(CHSV(x + hue, 255, 255));
        buffer[XY(x, y)].red = 0;
      }
    }

    theta++;
    hue++;

    apaBackgroundLayer.swapBuffers(true);
  }
}
#endif
