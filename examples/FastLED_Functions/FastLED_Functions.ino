/*
 * Modified example copied from FastLED 3.0 Branch - originally written by Daniel Garcia
 * This example shows how to use some of FastLEDs functions with the SmartMatrix Library
 * using the SmartMatrix buffers directly instead of FastLED's buffers.
 * FastLED's dithering and color balance features can't be used this way, but SmartMatrix can draw in
 * 36-bit color and so dithering may not provide much advantage.  There's no one 'right' way to use these two
 * libraries together, try this example and FastLED_Controller and figure out what is 'right' for you
 *
 * This example requires FastLED 3.0 or higher.  If you are having trouble compiling, see
 * the troubleshooting instructions here:
 * http://docs.pixelmatix.com/SmartMatrix/#external-libraries
 */

#include <SmartMatrix_32x32.h>
#include <FastLED.h>

#define kMatrixWidth  MATRIX_WIDTH
#define kMatrixHeight MATRIX_HEIGHT

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
  delay(3000);

  matrix.begin();

  matrix.setBackgroundBrightness(96);

  // Initialize our coordinates to some random values
  x = random16();
  y = random16();
  z = random16();

  // Show off smart matrix scrolling text
  matrix.setScrollMode(wrapForward);
  matrix.setScrollColor({0xff, 0xff, 0xff});
  matrix.setScrollSpeed(15);
  matrix.setScrollFont(font6x10);
  matrix.scrollText("SmartMatrix & FastLED", -1);
  matrix.setScrollOffsetFromEdge((kMatrixHeight/2) - 5);
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
  static uint8_t circlex = 0;
  static uint8_t circley = 0;

  rgb24 *buffer = matrix.backBuffer();

  static uint8_t ihue=0;
  fillnoise8();
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
  ihue+=1;

  matrix.fillCircle(circlex % 32,circley % 32,6,CRGB(CHSV(ihue+128,255,255)));
  circlex += random16(2);
  circley += random16(2);
  matrix.swapBuffers(false);
  LEDS.countFPS();
  // delay(10);
}
