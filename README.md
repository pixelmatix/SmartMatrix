# SmartMatrix Library for Teensy 3

Alpha version of 3.0

Only FeatureDemo is supported right now

Use `#include <SmartMatrix3.h>` in your Arduino sketch to allow this version of the library to coexist with a previous SmartMatrix Library release, and use this minimal code to get started:

```
#include <SmartMatrix3.h>

const uint8_t kMatrixHeight = 32;       // known working: 16, 32
const uint8_t kMatrixWidth = 32;        // known working: 32, 64
const uint8_t kColorDepthRgb = 36;      // known working: 36, 48 (24 isn't efficient and has color correction issues)
const uint8_t kDmaBufferRows = 4;       // known working: 4
SMARTMATRIX_ALLOCATE_BUFFERS(kMatrixWidth, kMatrixHeight, kColorDepthRgb, kDmaBufferRows);

void setup() {
    SMARTMATRIX_SETUP_DEFAULT_LAYERS(kMatrixWidth, kMatrixHeight);
}

void loop() {
}

```
