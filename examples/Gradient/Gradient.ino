#define WIDTH 64
#define HEIGHT 32
#define PATTERN_FPS 30
#define UPDATE_FPS 120
#define BLINK_PIN 13

#include <FastLED.h>

#include <SmartMatrix3.h>
const uint8_t kMatrixHeight = HEIGHT;       // known working: 16, 32
const uint8_t kMatrixWidth = WIDTH;        // known working: 32, 64
//const uint8_t kColorDepthRgb = 48;      // known working: 36, 48 (24 isn't efficient and has color correction issues)
const uint8_t kDmaBufferRows = 4;       // known working: 4
typedef rgb48 rgb;
SMARTMATRIX_ALLOCATE_BUFFERS(kMatrixWidth, kMatrixHeight, 48, kDmaBufferRows);

void setup() {
  delay(2000);
  pinMode(BLINK_PIN, OUTPUT);

  SMARTMATRIX_SETUP_DEFAULT_LAYERS(kMatrixWidth, kMatrixHeight, 48);

  matrix.setBrightness(255);
  matrix.setColorCorrection(cc48);
  //matrix.setRefreshRate(60);
}

void updatePattern() {
  static int red = (2^16)-1;
  static int xgrad = 0;
  
  for (int x=0; x<WIDTH; x++) {
    int val = red * pow((xgrad+x)%WIDTH / double(WIDTH), 2);
    for (int y=0; y<HEIGHT; y++) {
      int pos = y * WIDTH + x;
      rgb& pixel = matrix.backBuffer()[pos];
      pixel.red = val;
    }
  }

  xgrad++;
}

void loop() {
  static uint32_t lastUpdateTime = millis();
  if (millis() - lastUpdateTime >= 1000 / UPDATE_FPS) {
    lastUpdateTime = millis();
    matrix.swapBuffers();  
  }

  static uint32_t lastPatternTime = millis();
  if (millis() - lastPatternTime >= 1000 / PATTERN_FPS) {
    lastPatternTime = millis();
    updatePattern();  
  }
 
  delayMicroseconds(100);

   
//  red = red + dred;
//  if (red == 0 || red == 100)
//    dred = -dred;


//  digitalWrite(BLINK_PIN, HIGH);
//  delayMicroseconds(1000);
//  digitalWrite(BLINK_PIN, LOW);
}
