#include <SmartMatrix3.h>
#include <FastLED.h>

// TODO: make SmartMatrix library include other libraries, remove from sketch, is that possible?
#include <SPI.h>

#define COLOR_DEPTH 24                  // known working: 24, 48 - If the sketch uses type `rgb24` directly, COLOR_DEPTH must be 24
const uint8_t kMatrixWidth = 16;        // known working: 16, 32, 48, 64
const uint8_t kMatrixHeight = 16;       // known working: 32, 64, 96, 128
const uint8_t kRefreshDepth = 36;       // known working: 24, 36, 48
const uint8_t kDmaBufferRows = 4;       // known working: 2-4, use 2 to save memory, more to keep from dropping frames and automatically lowering refresh rate
const uint8_t kPanelType = 0;   // use SMARTMATRIX_HUB75_16ROW_MOD8SCAN for common 16x32 panels
const uint8_t kMatrixOptions = (SMARTMATRIX_OPTIONS_NONE);      // see http://docs.pixelmatix.com/SmartMatrix for options
const uint8_t kBackgroundLayerOptions = (SM_BACKGROUND_OPTIONS_NONE);
const uint8_t kScrollingLayerOptions = (SM_SCROLLING_OPTIONS_NONE);

SMARTMATRIX_ALLOCATE_BUFFERS(matrix, kMatrixWidth, kMatrixHeight, kRefreshDepth, kDmaBufferRows, kPanelType, kMatrixOptions);
SMARTMATRIX_ALLOCATE_BACKGROUND_LAYER(backgroundLayer, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kBackgroundLayerOptions);
SMARTMATRIX_ALLOCATE_SCROLLING_LAYER(scrollingLayer, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kScrollingLayerOptions);

rgb24 *buffer;

const uint16_t NUM_LEDS = kMatrixWidth * kMatrixHeight;
const uint8_t kMatrixCenterX = kMatrixWidth / 2;

uint8_t theta = 0;
uint8_t hueoffset = 0;

void setup() {
  // uncomment the following lines if you want to see FPS count information
  // Serial.begin(38400);
  // Serial.println("resetting!");

  matrix.addLayer(&backgroundLayer);
  matrix.addLayer(&scrollingLayer);
  matrix.begin();

  matrix.setBrightness(64);

  //scrollingLayer.start("SmartMatrix & APA102", -1);
}

uint8_t mapsin8(uint8_t theta, uint8_t lowest = 0, uint8_t highest = 255) {
  uint8_t beatsin = sin8(theta);
  uint8_t rangewidth = highest - lowest;
  uint8_t scaledbeat = scale8(beatsin, rangewidth);
  uint8_t result = lowest + scaledbeat;
  return result;
}

uint8_t mapcos8(uint8_t theta, uint8_t lowest = 0, uint8_t highest = 255) {
  uint8_t beatcos = cos8(theta);
  uint8_t rangewidth = highest - lowest;
  uint8_t scaledbeat = scale8(beatcos, rangewidth);
  uint8_t result = lowest + scaledbeat;
  return result;
}

void loop() {
  buffer = backgroundLayer.backBuffer();

  dimAll(252);

  for (uint8_t offset = 0; offset < kMatrixCenterX; offset++) {
    uint8_t hue = 255 - (offset * 30 + hueoffset);
    CRGB color = CHSV(hue, 255, 255);
    uint8_t x = mapcos8(theta, offset, (kMatrixWidth - 1) - offset);
    uint8_t y = mapsin8(theta, offset, (kMatrixHeight - 1) - offset);
    uint16_t xy = XY(x, y);
    buffer[xy] = color;
  }

  EVERY_N_MILLIS(25) {
    theta += 2;
    hueoffset += 1;
  }
  
  backgroundLayer.swapBuffers(false);
  //matrix.countFPS();      // print the loop() frames per second to Serial
}

uint16_t XY(uint8_t x, uint8_t y) {
  return kMatrixWidth * x + y;
}

// scale the brightness of all pixels down
void dimAll(byte value)
{
  for (int i = 0; i < NUM_LEDS; i++) {
    CRGB c = CRGB(buffer[i].red, buffer[i].green, buffer[i].blue);
    c.nscale8(value);
    buffer[i] = c;
  }
}
