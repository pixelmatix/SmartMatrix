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

uint16_t noisex;
uint16_t noisey;
uint16_t noisez;

int noisespeedx;
int noisespeedy;
int noisespeedz;

uint16_t noisescale;

typedef void (*Pattern)();
typedef Pattern PatternList[];

const PatternList patterns = {
  fireNoise,
  fireNoise2,
  lavaNoise,
  rainbowNoise,
  rainbowStripeNoise,
  partyNoise,
  forestNoise,
  cloudNoise,
  oceanNoise,
};

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))
const uint8_t patternCount = ARRAY_SIZE(patterns);

uint8_t patternIndex = 8;

void setup() {
  // uncomment the following lines if you want to see FPS count information
  // Serial.begin(38400);
  // Serial.println("resetting!");

  matrix.addLayer(&backgroundLayer);
  matrix.addLayer(&scrollingLayer);
  matrix.begin();

  matrix.setBrightness(16);

  //scrollingLayer.start("SmartMatrix & APA102", -1);
}

void loop() {
  buffer = backgroundLayer.backBuffer();

  patterns[patternIndex]();

  backgroundLayer.swapBuffers(false);
  //matrix.countFPS();      // print the loop() frames per second to Serial

  EVERY_N_SECONDS( 10 ) {
    nextPattern();  // change patterns periodically
  }
}

void nextPattern()
{
  // add one to the current pattern number, and wrap around at the end
  patternIndex = (patternIndex + 1) % patternCount;
}

void fireNoise() {
  noisespeedx = 0;
  noisespeedy = 64;
  noisespeedz = 0;
  noisescale = 64;
  drawNoise(HeatColors_p);
}

void fireNoise2() {
  noisespeedx = 0;
  noisespeedy = 192;
  noisespeedz = 4;
  noisescale = 128;
  drawNoise(HeatColors_p);
}

void lavaNoise() {
  noisespeedx = 0;
  noisespeedy = -3;
  noisespeedz = 2;
  noisescale = 8;
  drawNoise(LavaColors_p);
}

void rainbowNoise() {
  noisespeedx = 8;
  noisespeedy = 0;
  noisespeedz = 0;
  noisescale = 16;
  drawNoise(RainbowColors_p);
}

void rainbowStripeNoise() {
  noisespeedx = 2;
  noisespeedy = 0;
  noisespeedz = 0;
  noisescale = 4;
  drawNoise(RainbowStripeColors_p);
}

void partyNoise() {
  noisespeedx = 0;
  noisespeedy = 8;
  noisespeedz = 1;
  noisescale = 16;
  drawNoise(PartyColors_p);
}

void forestNoise() {
  noisespeedx = 2;
  noisespeedy = 0;
  noisespeedz = 0;
  noisescale = 4;
  drawNoise(ForestColors_p);
}

void cloudNoise() {
  noisespeedx = 0;
  noisespeedy = 4;
  noisespeedz = 0;
  noisescale = 16;
  drawNoise(CloudColors_p);
}

void oceanNoise() {
  noisespeedx = 0;
  noisespeedy = 0;
  noisespeedz = 16;
  noisescale = 16;
  drawNoise(OceanColors_p);
}

void drawNoise(CRGBPalette16 palette)
{
  for (uint8_t x = 0; x < kMatrixWidth; x++) {
    for (uint8_t y = 0; y < kMatrixHeight; y++) {
      uint16_t xoffset = noisescale * x;
      uint16_t yoffset = noisescale * y;

      uint8_t index = inoise8(x + xoffset + noisex, y + yoffset + noisey, noisez);

      // The range of the inoise8 function is roughly 16-238.
      // These two operations expand those values out to roughly 0..255
      // You can comment them out if you want the raw noise data.
      index = qsub8(index, 16);
      index = qadd8(index, scale8(index, 39));

      CRGB c = ColorFromPalette(palette, index);

      // fix color order (my panel is bgr)
      CRGB color = CRGB(c.b, c.g, c.r);

      buffer[XY(x, y)] = color;
    }
  }

  noisex += noisespeedx;
  noisey += noisespeedy;
  noisez += noisespeedz;
}

uint16_t XY(uint8_t x, uint8_t y) {
  return kMatrixWidth * x + y;
}
