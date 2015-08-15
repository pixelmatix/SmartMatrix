#define WIDTH 64
#define HEIGHT 32
#define PATTERN_FPS 30
#define UPDATE_FPS 120
#define BLINK_PIN 13


#include <SmartMatrix3.h>
#define DEPTH 48                          // known working: 24, 48 (why not 36?)
const uint8_t kMatrixHeight = HEIGHT;     // known working: 16, 32
const uint8_t kMatrixWidth = WIDTH;       // known working: 32, 64
const uint8_t kDmaBufferRows = 4;         // known working: 4
SMARTMATRIX_ALLOCATE_BUFFERS(kMatrixWidth, kMatrixHeight, DEPTH, kDmaBufferRows);
typedef RGB_TYPE(DEPTH) rgb;

void setup() {
  delay(2000);
  pinMode(BLINK_PIN, OUTPUT);

  SMARTMATRIX_SETUP_DEFAULT_LAYERS(kMatrixWidth, kMatrixHeight, DEPTH);

  matrix.setBrightness(255);
  matrix.setColorCorrection(cc48);
}

void updatePattern() {
  int max = (2^(DEPTH/3))-1;
  
  for (int x=0; x<WIDTH; x++) {
    int val = max * pow(x%WIDTH / double(WIDTH), 2);
    for (int y=0; y<HEIGHT; y++) {
      int pos = y * WIDTH + x;
      rgb& pixel = matrix.backBuffer()[pos];
      pixel.red = val;
    }
  }
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
