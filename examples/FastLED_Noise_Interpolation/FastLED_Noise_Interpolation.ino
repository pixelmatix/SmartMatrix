#include<SmartMatrix_32x32.h>
#include<FastLED.h>

#define kMatrixWidth  32
#define kMatrixHeight 32

SmartMatrix matrix;

// The 32bit version of our coordinates
static uint16_t x;
static uint16_t y;
static uint16_t z;

// We're using the x/y dimensions to map to the x/y pixels on the matrix.  We'll
// use the z-axis for "time".  speed determines how fast time moves forward.  Try
// 1 for a very slow moving effect, or 60 for something that ends up looking like
// water.
// uint16_t speed = 1; // almost looks like a painting, moves very slowly
uint16_t speed = 20; // a nice starting speed, mixes well with a scale of 100
// uint16_t speed = 33;
// uint16_t speed = 100; // wicked fast!

// Scale determines how far apart the pixels in our noise matrix are.  Try
// changing these values around to see how it affects the motion of the display.  The
// higher the value of scale, the more "zoomed out" the noise iwll be.  A value
// of 1 will be so zoomed in, you'll mostly see solid colors.

// uint16_t scale = 1; // mostly just solid colors
// uint16_t scale = 4011; // very zoomed out and shimmery
uint16_t scale = 31;

// This is the array that we keep our computed noise values in
uint8_t noise[kMatrixWidth][kMatrixHeight];

void setup() {
  // uncomment the following lines if you want to see FPS count information
  // Serial.begin(38400);
  // Serial.println("resetting!");
  //delay(3000);

  matrix.begin();

  // Initialize our coordinates to some random values
  x = random16();
  y = random16();
  z = random16();

  // Show off smart matrix scrolling text
  matrix.setScrollMode(wrapForward);
  matrix.setScrollColor({0xff, 0xff, 0xff});
  matrix.setScrollSpeed(50);
  matrix.setScrollFont(font8x13);
  matrix.scrollText("Smart Matrix & FastLED", -1);
  matrix.setScrollOffsetFromEdge(9);
}

// Fill the x/y array of 8-bit noise values using the inoise8 function.
void fillnoise8() {
  for(int i = 0; i < kMatrixWidth; i++) {
    int ioffset = scale * i;
    for(int j = 0; j < kMatrixHeight; j++) {
      int joffset = scale * j;
      noise[i][j] = inoise8(x + ioffset,y + joffset,z);
    }
  }
  z += speed;
}


void loop() {
  static uint8_t ihue=0;
  fillnoise8();

  rgb24 *buffer = matrix.backBuffer();

  for(int i = 0; i < kMatrixWidth; i++) {
    for(int j = 0; j < kMatrixHeight; j++) {
      // We use the value at the (i,j) coordinate in the noise
      // array for our brightness, and the flipped value from (j,i)
      // for our pixel's hue.
      buffer[kMatrixHeight*j + i] = CRGB(CHSV(noise[j][i],255,noise[i][j]));

      // You can also explore other ways to constrain the hue used, like below
      // leds[XY(i,j)] = CHSV(ihue + (noise[j][i]>>2),255,noise[i][j]);
    }
  }
  ihue+=1;

#define FRAME_UPDATE_INTERVAL_MS  400

  // alternate between using swap with interpolation and no interpolation
  if(millis() % 10000 < 5000) {
    matrix.swapBuffersWithInterpolation_ms(FRAME_UPDATE_INTERVAL_MS);
  } else {
    matrix.swapBuffers(false);
    delay(FRAME_UPDATE_INTERVAL_MS);
  }

  LEDS.countFPS();
}
