#include <SmartMatrix3.h>

// Change the following value to 24 or 48 to change between storing colors
// with 24 or 48bits. At 24bits, this example sketch shows "stepping" at low
// color values, while at 48bits the gradient is smooth.

#define COLOR_DEPTH 48                  // known working: 24, 48 - If the sketch uses type `rgb24` directly, COLOR_DEPTH must be 24
const uint8_t kMatrixHeight = 32;       // known working: 16, 32
const uint8_t kMatrixWidth = 64;        // known working: 32, 64
const uint8_t kDmaBufferRows = 4;       // known working: 4
const uint8_t kRefreshDepth = 48;       // known working: 24, 36, 48
const uint8_t kPanelType = SMARTMATRIX_HUB75_32ROW_MOD16SCAN; // use SMARTMATRIX_HUB75_16ROW_MOD8SCAN for common 16x32 panels
const uint8_t kMatrixOptions = (SMARTMATRIX_OPTIONS_NONE);
const uint8_t kBackgroundLayerOptions = (SM_BACKGROUND_OPTIONS_NONE);

SMARTMATRIX_ALLOCATE_BUFFERS(matrix, kMatrixWidth, kMatrixHeight, kRefreshDepth, kDmaBufferRows, kPanelType, kMatrixOptions);
SMARTMATRIX_ALLOCATE_BACKGROUND_LAYER(backgroundLayer, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kBackgroundLayerOptions);

void setup() {
  //Serial.begin(115200);
  matrix.addLayer(&backgroundLayer); 
  matrix.begin();
  backgroundLayer.enableColorCorrection(false);
}

void loop() {
  uint32_t max = pow(2, COLOR_DEPTH/3) - 1;
  
  for (int x=0; x<kMatrixWidth; x++) {
    uint32_t val = max * pow(x / double(kMatrixWidth), 2) / 20;
    for (int y=0; y<kMatrixHeight; y++) {
      int pos = y * kMatrixWidth + x;
      SM_RGB& pixel = backgroundLayer.backBuffer()[pos];
      pixel.red = val;
    }
  }
  backgroundLayer.swapBuffers();

  delay(10);
}
