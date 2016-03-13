#pragma SPARK_NO_PREPROCESSOR
#include "application.h"


// hardware-specific definitions
// prescale of 7 is F_BUS/128
#define LATCH_TIMER_PRESCALE  31
#define TIMER_FREQUENCY     ((SystemCoreClock / 2) / LATCH_TIMER_PRESCALE + 1)
#define NS_TO_TICKS(X)      (uint32_t)(TIMER_FREQUENCY * ((X) / 1000000000.0))
#define TICKS_PER_FRAME   (TIMER_FREQUENCY/refreshRate)



volatile bool transferDone = true;

#define SPIBUFFERSIZE 1500

uint8_t src[SPIBUFFERSIZE];

uint16_t led16bit_r;
uint16_t led16bit_g;
uint16_t led16bit_b;

#define setRGBOut(r,g,b) {led16bit_r = r; led16bit_g = g; led16bit_b = b; }

/******** color_funcs.h -- color sliding
//
// 2012, Tod E. Kurt, http://todbot.com/blog/
*********/
// RGB triplet of 16-bit vals for input/output use
typedef struct {
    uint16_t r;
    uint16_t g;
    uint16_t b;
} rgbuint16_t;

// RGB triplet signed int32s for internal use of 100x scale
// used instead of floating point
typedef struct {
    int32_t r;
    int32_t g;
    int32_t b;
} rgbint32_t;

//
typedef struct {
    rgbuint16_t color;
    uint16_t dmillis; // hundreths of a sec
} patternline_t;



rgbint32_t dest100x;  // the eventual destination color we want to hit
rgbint32_t step100x;  // the amount of to move each tick
rgbint32_t curr100x;  // the current color, times 100 (to lessen int trunc issue)
int stepcnt;

#ifndef setRGBOut
#error "setRGBOut(r,g,b) not defined"
#endif

// set the current color
void rgb_setCurr( rgbuint16_t* newcolor )
{
    curr100x.r = newcolor->r * 100;
    curr100x.g = newcolor->g * 100;
    curr100x.b = newcolor->b * 100;

    dest100x.r = curr100x.r;
    dest100x.g = curr100x.g;
    dest100x.b = curr100x.b;
    stepcnt = 0;

    setRGBOut( newcolor->r, newcolor->g, newcolor->b );
}

// set a new destination color
void rgb_setDest( rgbuint16_t* newcolor, int steps  )
{
    dest100x.r = newcolor->r*100;
    dest100x.g = newcolor->g*100;
    dest100x.b = newcolor->b*100;

    stepcnt = steps+1;

    step100x.r = (dest100x.r - curr100x.r ) / steps;
    step100x.g = (dest100x.g - curr100x.g ) / steps;
    step100x.b = (dest100x.b - curr100x.b ) / steps;
}

// call at every tick
void rgb_updateCurrent(void)
{
    if( !stepcnt ) {
        return;
    }
    stepcnt--;
    if( stepcnt ) {
        curr100x.r += step100x.r;
        curr100x.g += step100x.g;
        curr100x.b += step100x.b;
    } else {
        curr100x.r = dest100x.r;
        curr100x.g = dest100x.g;
        curr100x.b = dest100x.b;
    }

    setRGBOut( curr100x.r/100, curr100x.g/100, curr100x.b/100 );
}

/******** end color_funcs.h
*********/



rgbuint16_t c1 = {0,0,0};
rgbuint16_t c2;
int steps = 100;

int spilength = 0;

void myCallbackFunction() {
    transferDone = true;
}

// timer ISR is run every 1/refreshrate - start sending queued up SPI data
void timerCallback(void) {
    // start next DMA SPI transfer if ready
    if(spilength)
        SPI1.transfer(src, src, spilength, myCallbackFunction);
}


void startTimer(void) {
    int refreshRate = 30;

    TIM_TimeBaseInitTypeDef timerInitStructure;
    NVIC_InitTypeDef nvicStructure;

    // TIM clock enable
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);

    uint16_t TIM_Prescaler = LATCH_TIMER_PRESCALE;

    // Timebase configuration
    timerInitStructure.TIM_Prescaler = TIM_Prescaler;
    timerInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    timerInitStructure.TIM_Period = TICKS_PER_FRAME;
    timerInitStructure.TIM_ClockDivision = 0;

    TIM_TimeBaseInit(TIM7, &timerInitStructure);

    TIM_ITConfig(TIM7, TIM_IT_Update, ENABLE);

    TIM_Cmd(TIM7, ENABLE);

    attachSystemInterrupt(SysInterrupt_TIM7_IRQ, timerCallback);

    // Enable Timer Interrupt
    nvicStructure.NVIC_IRQChannel = TIM7_IRQn;
    nvicStructure.NVIC_IRQChannelPreemptionPriority = 10;
    nvicStructure.NVIC_IRQChannelSubPriority = 1;
    nvicStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvicStructure);
}

void setup() {
    SPI1.begin(D5);
    //SPI1.setBitOrder(order);
    SPI1.setClockSpeed(1000000);
    //SPI1.setClockDivider(divider);
    //SPI1.setDataMode(mode);
      SPI1.setBitOrder(MSBFIRST);
  SPI1.setDataMode(SPI_MODE0);


    int i;
    for(i=0; i<SPIBUFFERSIZE; i++) {
        src[i] = 0xAA;
    }

    rgb_setCurr( &c1 );

  startTimer();

}

uint16_t brightness = 10;

void loop() {
    int i;

    c2.r = random(0xFFFF);
    c2.g = random(0xFFFF);
    c2.b = random(0xFFFF);

    c2.r = (c2.r * brightness) / 256;
    c2.g = (c2.g * brightness) / 256;
    c2.b = (c2.b * brightness) / 256;

    rgb_setDest( &c2, steps );  

    for( int j=0; j<steps+1; j++ ) {

        while(!transferDone);
    
        delay(1000/30);

        spilength = 0;

        for(i=0; i<4; i++) src[spilength++] = 0;    // Start-frame marker

        // first LED in chain with 8-bit color - global brightness max
        src[spilength++] = 0xFF;                
        src[spilength++] = led16bit_r >> 8; // R,G,B
        src[spilength++] = led16bit_g >> 8;
        src[spilength++] = led16bit_b >> 8;

        uint8_t globalbrightness;
        uint8_t localshift;
        uint16_t value = led16bit_r | led16bit_g | led16bit_b;

        if((value & 0x8000) != 0) {
            globalbrightness = 0x1F;
            localshift = 8;
        }
        else if((value & 0x4000) != 0) {
            globalbrightness = 0x10;
            localshift = 7;
        }
        else if((value & 0x2000) != 0) {
            globalbrightness = 0x08;
            localshift = 6;
        }
        else if((value & 0x1000) != 0) {
            globalbrightness = 0x04;
            localshift = 5;
        }
        else if((value & 0x0800) != 0) {
            globalbrightness = 0x02;
            localshift = 4;
        }
        else {
            globalbrightness = 0x01;
            localshift = 3;
        }

        int k;

        // load pixels with 8-bit color plus 5-bit global brightness
        for(k=0; k<256; k++) {
            src[spilength++] = 0xE0 | globalbrightness;                //  Pixel start with global brightness
            src[spilength++] = led16bit_r >> localshift; // R,G,B
            src[spilength++] = led16bit_g >> localshift;
            src[spilength++] = led16bit_b >> localshift;
        }
        for(i=0; i<4; i++) src[spilength++] = 0xFF; // End-frame marker 

        // sending happens in the background, just set flag so we know when to do next calculations
        transferDone = false;

        rgb_updateCurrent();
    }

}

#if 0
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

// The 32bit version of our coordinates
static uint16_t x;
static uint16_t y;
static uint16_t z;

// We're using the x/y dimensions to map to the x/y pixels on the matrix.  We'll
// use the z-axis for "time".  speed determines how fast time moves forward.  Try
// 1 for a very slow moving effect, or 60 for something that ends up looking like
// water.
// uint16_t speed = 1; // almost looks like a painting, moves very slowly
uint16_t speed = 5; // a nice starting speed, mixes well with a scale of 100
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

    matrix.addLayer(&backgroundLayer); 
    matrix.addLayer(&scrollingLayer); 
    matrix.begin();

    matrix.setBrightness(64);
    //backgroundLayer.setBrightness(10);

  // Initialize our coordinates to some random values
  x = random16();
  y = random16();
  z = random16();

    //scrollingLayer.start("SmartMatrix & APA102", -1);

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

  rgb24 *buffer = backgroundLayer.backBuffer();

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

  //backgroundLayer.fillCircle(circlex % kMatrixWidth,circley % kMatrixHeight,6,CRGB(CHSV(ihue+128,255,255)));
  circlex += random16(2);
  circley += random16(2);
  backgroundLayer.swapBuffers(false);
  //matrix.countFPS();      // print the loop() frames per second to Serial
}
#endif
