/*
 * This example shows how to display bitmaps that are exported from GIMP and
 * compiled into the sketch and stored in the Teensy's Flash memory
 * See more details here:
 * http://docs.pixelmatix.com/SmartMatrix/library.html#smartmatrix-library-how-to-use-the-library-drawing-raw-bitmaps
 *
 * This example uses only the SmartMatrix Background layer
 */

//#include <SmartLEDShieldV4.h>  // uncomment this line for SmartLED Shield V4 (needs to be before #include <SmartMatrix3.h>)
#include <SmartMatrix3.h>

#include "gimpbitmap.h"

// pixelmatix.c is a bitmap exported from GIMP without modification
#include "pixelmatix.c"

// colorwheel.c has been modified to use the gimp32x32bitmap struct
#include "colorwheel.c"

// chrome16 is a 16x16 pixel bitmap, exported from GIMP without modification
#include "chrome16.c"

#define COLOR_DEPTH 24                  // known working: 24, 48 - If the sketch uses type `rgb24` directly, COLOR_DEPTH must be 24
const uint8_t kMatrixWidth = 64;        // known working: 16, 32, 48, 64
const uint8_t kMatrixHeight = 64;       // known working: 32, 64, 96, 128
const uint8_t kRefreshDepth = 36;       // known working: 24, 36, 48
const uint8_t kDmaBufferRows = 4;       // known working: 2-4, use 2 to save memory, more to keep from dropping frames and automatically lowering refresh rate
const uint8_t kPanelType = SMARTMATRIX_HUB75_64ROW_MOD32SCAN;   // use SMARTMATRIX_HUB75_16ROW_MOD8SCAN for common 16x32 panels
const uint8_t kMatrixOptions = (SMARTMATRIX_OPTIONS_NONE);      // see http://docs.pixelmatix.com/SmartMatrix for options
const uint8_t kBackgroundLayerOptions = (SM_BACKGROUND_OPTIONS_NONE);

SMARTMATRIX_ALLOCATE_BUFFERS(matrix, kMatrixWidth, kMatrixHeight, kRefreshDepth, kDmaBufferRows, kPanelType, kMatrixOptions);
SMARTMATRIX_ALLOCATE_BACKGROUND_LAYER(backgroundLayer, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kBackgroundLayerOptions);

int led = 13;

void drawBitmap(int16_t x, int16_t y, const gimp32x32bitmap* bitmap) {
  for(unsigned int i=0; i < bitmap->height; i++) {
    for(unsigned int j=0; j < bitmap->width; j++) {
      SM_RGB pixel = { bitmap->pixel_data[(i*bitmap->width + j)*3 + 0],
                      bitmap->pixel_data[(i*bitmap->width + j)*3 + 1],
                      bitmap->pixel_data[(i*bitmap->width + j)*3 + 2] };

      backgroundLayer.drawPixel(x + j, y + i, pixel);
    }
  }
}

static void gpio_setup_out(int gpio) {
    if (gpio==-1) return;
    PIN_FUNC_SELECT(GPIO_PIN_MUX_REG[gpio], PIN_FUNC_GPIO);
    gpio_set_direction((gpio_num_t)gpio, GPIO_MODE_OUTPUT);
    //gpio_matrix_out(gpio, sig, false, false);
}

void sendFm6126aResetSequence(void) {
  // TODO: set this number more intelligently?
  int maxLed = 256;

  int C12[16] = {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
  int C13[16] = {0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0};

  gpio_setup_out(R1_PIN);
  gpio_setup_out(G1_PIN);
  gpio_setup_out(B1_PIN);
  gpio_setup_out(R2_PIN);
  gpio_setup_out(G2_PIN);
  gpio_setup_out(B2_PIN);

  gpio_setup_out(LAT_PIN);
  gpio_setup_out(OE_PIN);

  gpio_setup_out(A_PIN);
  gpio_setup_out(B_PIN);
  gpio_setup_out(C_PIN);
  gpio_setup_out(D_PIN);
  gpio_setup_out(E_PIN);

  gpio_setup_out(CLK_PIN);

  gpio_set_level((gpio_num_t)OE_PIN, 1);  // blank display

  gpio_setup_out(18);


  // Send Data to control register 11
  gpio_set_level((gpio_num_t)LAT_PIN, 0);
  gpio_set_level((gpio_num_t)CLK_PIN, 0);
  for (int l=0; l<maxLed; l++) {    
    int y=l%16;
    gpio_set_level((gpio_num_t)R1_PIN,0);
    gpio_set_level((gpio_num_t)G1_PIN,0);
    gpio_set_level((gpio_num_t)B1_PIN,0);
    gpio_set_level((gpio_num_t)R2_PIN,0);
    gpio_set_level((gpio_num_t)G2_PIN,0);
    gpio_set_level((gpio_num_t)B2_PIN,0);
    if (C12[y]==1) {
        gpio_set_level((gpio_num_t)R1_PIN,1);
        gpio_set_level((gpio_num_t)G1_PIN,1);
        gpio_set_level((gpio_num_t)B1_PIN,1);
        gpio_set_level((gpio_num_t)R2_PIN,1);
        gpio_set_level((gpio_num_t)G2_PIN,1);
        gpio_set_level((gpio_num_t)B2_PIN,1);
    }
    if (l>maxLed-12){
      gpio_set_level((gpio_num_t)LAT_PIN, 1);

      // with latch high, CLK is gated, and 18 is used to toggle CLK instead
      gpio_set_level((gpio_num_t)18,1);
      gpio_set_level((gpio_num_t)18,0);
    } else {
      gpio_set_level((gpio_num_t)LAT_PIN, 0);
    }
    gpio_set_level((gpio_num_t)CLK_PIN, 1);
    gpio_set_level((gpio_num_t)CLK_PIN, 0);
  }
  gpio_set_level((gpio_num_t)LAT_PIN, 0);
  gpio_set_level((gpio_num_t)CLK_PIN, 0);

  // Send Data to control register 12
  for (int l=0; l<maxLed; l++) {    
    int y=l%16;
    gpio_set_level((gpio_num_t)R1_PIN,0);
    gpio_set_level((gpio_num_t)G1_PIN,0);
    gpio_set_level((gpio_num_t)B1_PIN,0);
    gpio_set_level((gpio_num_t)R2_PIN,0);
    gpio_set_level((gpio_num_t)G2_PIN,0);
    gpio_set_level((gpio_num_t)B2_PIN,0);
    if (C13[y]==1){
      gpio_set_level((gpio_num_t)R1_PIN,1);
      gpio_set_level((gpio_num_t)G1_PIN,1);
      gpio_set_level((gpio_num_t)B1_PIN,1);
      gpio_set_level((gpio_num_t)R2_PIN,1);
      gpio_set_level((gpio_num_t)G2_PIN,1);
      gpio_set_level((gpio_num_t)B2_PIN,1);
    }  
    if (l>maxLed-13){
      gpio_set_level((gpio_num_t)LAT_PIN, 1);

      // with latch high, CLK is gated, and 18 is used to toggle CLK instead
      gpio_set_level((gpio_num_t)18,1);
      gpio_set_level((gpio_num_t)18,0);

    } else {
      gpio_set_level((gpio_num_t)LAT_PIN, 0);
    }
    gpio_set_level((gpio_num_t)CLK_PIN, 1);
    gpio_set_level((gpio_num_t)CLK_PIN, 0);
  }
  gpio_set_level((gpio_num_t)LAT_PIN, 0);
  gpio_set_level((gpio_num_t)CLK_PIN, 0);

  gpio_set_level((gpio_num_t)18,0);

}

void setup() {
  //sendFm6126aResetSequence();

  matrix.addLayer(&backgroundLayer); 
  matrix.begin();

  matrix.setBrightness(128);

  pinMode(led, OUTPUT);
}

void loop() {
  int x, y;
  backgroundLayer.fillScreen({0,0,0});
  x = (kMatrixWidth / 2) - (pixelmatixlogo.width/2);
  y = (kMatrixHeight / 2) - (pixelmatixlogo.height/2);
  // to use drawBitmap, must cast the pointer to pixelmatixlogo as (const gimp32x32bitmap*)
  drawBitmap(x,y,(const gimp32x32bitmap*)&pixelmatixlogo);
  backgroundLayer.swapBuffers();

  digitalWrite(led, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);
  digitalWrite(led, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);

  backgroundLayer.fillScreen({0,0,0});
  x = (kMatrixWidth / 2) - (colorwheel.width/2);
  y = (kMatrixHeight / 2) - (colorwheel.height/2);
  // can pass &colorwheel in directly as the bitmap source is already gimp32x32bitmap
  drawBitmap(x,y,&colorwheel);
  backgroundLayer.swapBuffers();

  digitalWrite(led, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);
  digitalWrite(led, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);

  backgroundLayer.fillScreen({0,0,0});
  x = (kMatrixWidth / 2) - (chrome16.width/2);
  y = (kMatrixHeight / 2) - (chrome16.height/2);
  drawBitmap(x, y, (const gimp32x32bitmap*)&chrome16);
  backgroundLayer.swapBuffers();

  digitalWrite(led, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);
  digitalWrite(led, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);
}
