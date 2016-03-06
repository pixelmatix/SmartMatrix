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
const uint8_t maxX = kMatrixWidth - 1;

const uint8_t scale = 256 / kMatrixWidth;

void setup() {
  // uncomment the following lines if you want to see FPS count information
  delay(1000);
  Serial.begin(38400);
  Serial.println("resetting!");

  matrix.addLayer(&backgroundLayer);
  matrix.addLayer(&scrollingLayer);
  matrix.begin();

  matrix.setBrightness(64);

  //scrollingLayer.start("SmartMatrix & APA102", -1);
}

void loop() {
  buffer = backgroundLayer.backBuffer();

  dimAll(250);

  static uint8_t theta = 0;
  static uint8_t hue = 0;
  static uint8_t rotation = random(0, 4);
  static uint8_t waveCount = random(1, 3);
  
  switch (rotation) {
    case 0:
      for (uint8_t x = 0; x < kMatrixWidth; x++) {
        uint8_t y = quadwave8(x * 2 + theta) / scale;
        buffer[XY(x, y)] = CRGB(CHSV(x + hue, 255, 255));
      }
      break;

    case 1:
      for (uint8_t y = 0; y < kMatrixHeight; y++) {
        uint8_t x = quadwave8(y * 2 + theta) / scale;
        buffer[XY(x, y)] = CRGB(CHSV(y + hue, 255, 255));
      }
      break;

    case 2:
      for (uint8_t x = 0; x < kMatrixWidth; x++) {
        uint8_t y = quadwave8(x * 2 - theta) / scale;
        buffer[XY(x, y)] = CRGB(CHSV(x + hue, 255, 255));
      }
      break;

    case 3:
      for (uint8_t y = 0; y < kMatrixHeight; y++) {
        uint8_t x = quadwave8(y * 2 - theta) / scale;
        buffer[XY(x, y)] = CRGB(CHSV(y + hue, 255, 255));
        if (waveCount == 2)
          buffer[XY(maxX - x, y)] = CRGB(CHSV(y + hue, 255, 255));
      }
      break;
  }

  theta++;
  hue++;

  EVERY_N_SECONDS(10) {
    rotation = random(0, 4);
    waveCount = random(1, 3);
  }

  backgroundLayer.swapBuffers(false);
  matrix.countFPS();      // print the loop() frames per second to Serial
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
