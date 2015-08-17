#include <SmartMatrix3.h>

// Change the following value to 24 or 48 to change between storing colors
// with 24 or 48bits. At 24bits, this example sketch shows "stepping" at low
// color values, while at 48bits the gradient is smooth.
#define COLOR_DEPTH 48                      // known working: 48, 24

#define WIDTH 64
#define HEIGHT 32

const uint8_t kMatrixHeight = HEIGHT;     // known working: 16, 32
const uint8_t kMatrixWidth = WIDTH;       // known working: 32, 64
const uint8_t kDmaBufferRows = 4;         // known working: 4
SMARTMATRIX_ALLOCATE_BUFFERS(kMatrixWidth, kMatrixHeight, COLOR_DEPTH, 48, kDmaBufferRows);

void setup() {
  //Serial.begin(115200);
  SMARTMATRIX_SETUP_DEFAULT_LAYERS(kMatrixWidth, kMatrixHeight, COLOR_DEPTH);

  matrix.setBrightness(255);
  matrix.setColorCorrection(cc48);
}

void loop() {
  uint32_t max = pow(2, COLOR_DEPTH/3) - 1;
  
  for (int x=0; x<WIDTH; x++) {
    uint32_t val = max * pow(x / double(WIDTH), 2) / 20;
    for (int y=0; y<HEIGHT; y++) {
      int pos = y * WIDTH + x;
      SM_RGB& pixel = matrix.backBuffer()[pos];
      pixel.red = val;
    }
  }
  matrix.swapBuffers();

  delay(10);
}
