/*
 * Modified example copied from FastLED 3.0 Branch - originally written by Daniel Garcia
 * This example shows how to use some of FastLED's functions with the SmartMatrix Library
 * using the SmartMatrix buffers directly instead of FastLED's buffers.
 * FastLED's dithering and color balance features can't be used this way, but SmartMatrix can draw in
 * 36-bit color and so dithering may not provide much advantage.
 *
 * This version of the example shows how to draw to an APA102 strip connected to the SmartLED Shield
 * for Teensy 3 (V4) or SmartLED Shield for Teensy 4 (V5) in parallel with the matrix panel.
 *
 * The ESP32 platorm is not supported
 *
 * Note that the APA102 matrix is mapped inside the controller with normal rectangular coordiantes,
 *   but is shifted out to the panel in serpentine layout.
 *
 * This example requires FastLED 3.0 or higher.  If you are having trouble compiling, see
 * the troubleshooting instructions here:
 * https://github.com/pixelmatix/SmartMatrix/#external-libraries
 */

// uncomment one line to select your MatrixHardware configuration - configuration header needs to be included before <SmartMatrix.h>
//#include <MatrixHardware_Teensy3_ShieldV4.h>        // SmartLED Shield for Teensy 3 (V4)
#include <MatrixHardware_Teensy4_ShieldV5.h>        // SmartLED Shield for Teensy 4 (V5)
//#include <MatrixHardware_Teensy3_ShieldV1toV3.h>    // SmartMatrix Shield for Teensy 3 V1-V3
//#include <MatrixHardware_Teensy4_ShieldV4Adapter.h> // Teensy 4 Adapter attached to SmartLED Shield for Teensy 3 (V4)
//#include <MatrixHardware_ESP32_V0.h>                // This file contains multiple ESP32 hardware configurations, edit the file to define GPIOPINOUT (or add #define GPIOPINOUT with a hardcoded number before this #include)
//#include "MatrixHardware_Custom.h"                  // Copy an existing MatrixHardware file to your Sketch directory, rename, customize, and you can include it like this
#include <SmartMatrix.h>
#include <FastLED.h>

#include <LEDMatrix.h>

#define ENABLE_APA102_REFRESH   1

#define COLOR_DEPTH 24                  // Choose the color depth used for storing pixels in the layers: 24 or 48 (24 is good for most sketches - If the sketch uses type `rgb24` directly, COLOR_DEPTH must be 24)

#if (ENABLE_APA102_REFRESH == 1)
const uint16_t kApaMatrixWidth = 16;          // adjust this to your APA matrix/strip
const uint16_t kApaMatrixHeight = 8;         // set kApaMatrixHeight to 1 for a strip
const uint8_t kApaRefreshDepth = 36;        // not used for APA matrices as of now
const uint8_t kApaDmaBufferRows = 1;        // not used for APA matrices as of now
const uint8_t kApaPanelType = 0;            // not used for APA matrices as of now
const uint32_t kApaMatrixOptions = (SM_APA102_OPTIONS_COLOR_ORDER_BGR);      // The default color order is BGR, change here to match your LEDs
const uint8_t kApaBackgroundLayerOptions = (SM_BACKGROUND_OPTIONS_NONE);

SMARTMATRIX_APA_ALLOCATE_BUFFERS(apamatrix, kApaMatrixWidth, kApaMatrixHeight, kApaRefreshDepth, kApaDmaBufferRows, kApaPanelType, kApaMatrixOptions);
SMARTMATRIX_ALLOCATE_BACKGROUND_LAYER(apaBackgroundLayer, kApaMatrixWidth, kApaMatrixHeight, COLOR_DEPTH, kApaBackgroundLayerOptions);
#endif

/********* Setup LEDMatrix Mapping Here ***********/

// describe your individual tiles
#define MATRIX_TILE_WIDTH   -8   // width of EACH APA102 MATRIX (not total display), if matrix input is on the right, make width negative
#define MATRIX_TILE_HEIGHT  8   // height of EACH APA102 MATRIX, if matrix input is on the bottom, make width negative
//#define MATRIX_TYPE       HORIZONTAL_MATRIX         // LEDs are connected across matrix in horizontal rows, reversing horizontal direction every row
//#define MATRIX_TYPE       VERTICAL_MATRIX           // LEDs are connected across matrix in horizontal rows, same direction every row
//#define MATRIX_TYPE       HORIZONTAL_ZIGZAG_MATRIX  // LEDs are connected across matrix in vertical columns, reversing vertical direction every row
#define MATRIX_TYPE       VERTICAL_ZIGZAG_MATRIX    // LEDs are connected across matrix in vertical columns, same direction every column

// describe how your tiles are connected together
#define MATRIX_TILE_H       -2   // number of matrices arranged horizontally (negative if wired right to left)
#define MATRIX_TILE_V       1   // number of matrices arranged vertically (negative if wired bottom to top)
#define BLOCK_TYPE          HORIZONTAL_BLOCKS         // Tiles are connected in the horizontal direction first, (same horizontal direction each row if abs(MATRIX_TILE_V) > 1)
//#define BLOCK_TYPE          VERTICAL_BLOCKS           // Tiles are connected in the vertical direction first, (same vertical direction each column if abs(MATRIX_TILE_H) > 1)
//#define BLOCK_TYPE          HORIZONTAL_ZIGZAG_BLOCKS  // Tiles are connected in the horizontal direction first, (reversing horizontal direction and mirroring 180 across each row if abs(MATRIX_TILE_V) > 1)
//#define BLOCK_TYPE          VERTICAL_ZIGZAG_BLOCKS    // Tiles are connected in the vertical direction first, (reversing vertical direction and mirroring tiles across each column if abs(MATRIX_TILE_H) > 1)

#define MATRIX_WIDTH        kApaMatrixWidth
#define MATRIX_HEIGHT       kApaMatrixHeight

// create our matrix based on matrix definition
cLEDMatrix<MATRIX_TILE_WIDTH, MATRIX_TILE_HEIGHT, MATRIX_TYPE, MATRIX_TILE_H, MATRIX_TILE_V, BLOCK_TYPE> leds;

/**********************************************/

uint16_t XYTableTest[kApaMatrixWidth*kApaMatrixHeight];

uint16_t getPixelFromXYCallback(int16_t x, int16_t y) {
  // any out of bounds address maps to the first hidden pixel
  if ( (x >= kApaMatrixWidth) || (y >= kApaMatrixHeight) ) {
    return (kApaMatrixWidth*kApaMatrixHeight);
  }


  uint16_t i = (y * kApaMatrixWidth) + x;
  uint16_t j = XYTableTest[i];
  return j;
}

#if (ENABLE_APA102_REFRESH == 1)
#define MAX_DIMENSION_APA ((kApaMatrixWidth>kApaMatrixHeight) ? kApaMatrixWidth : kApaMatrixHeight)
#else
#define MAX_DIMENSION_APA 0
#endif

#define MAX_DIMENSION_OVERALL MAX_DIMENSION_APA

void setup() {
  // Enable printing FPS count information
  Serial.begin(115200);

  // Wait for Serial to be ready
  delay(1000);

  // TODO: print error if 
  //#if (kApaMatrixWidth != (abs(MATRIX_TILE_WIDTH) * abs(MATRIX_TILE_H)))
  //#error "Check your cLEDMatrix Definitions, MATRIX_WIDTH != (ABS(MATRIX_TILE_WIDTH) * ABS(MATRIX_TILE_H)"

#if (ENABLE_APA102_REFRESH == 1)
  // enable the APA102 buffers to drive out the SPI signals
  if(!SMARTLED_APA_ENABLED_BY_DEFAULT) {
  //if(1) {
    pinMode(SMARTLED_APA_ENABLE_PIN, OUTPUT);
    digitalWrite(SMARTLED_APA_ENABLE_PIN, HIGH);  // enable access to LEDs
  }

  apamatrix.addLayer(&apaBackgroundLayer);

  // The default SPI clock speed is 5MHz.  If you want to use a different clock speed, call this function _before_ begin()
  //apamatrix.setSpiClockSpeed(15000000); // 16MHz is about as fast as the Teensy 3 can support reliably
  //apamatrix.setSpiClockSpeed(40000000); // The Teensy 4 can go much faster, too fast for APA102 LEDs
  //apamatrix.setSpiClockSpeed(20000000); // 20MHz is a reasonable speed for a short chain (e.g. 256 LEDs)
  //apamatrix.setSpiClockSpeed(5000000); // A long chain of APA102 LEDs can't support a high clock rate, hence the default of 5MHz

  // as we're trying to replace the mapping, send out the LED data as is without correction
  apamatrix.setSerpentineLayout(false);

  apamatrix.begin();

  // lower the brightness
  //apamatrix.setBrightness(128);
  apamatrix.setBrightness(32);
#endif

  rgb24 *apabuffer = apaBackgroundLayer.backBuffer();
  leds.SetLEDArray((CRGB*)apabuffer);
    
  // draw a red line from (0,0) to (kApaMatrixWidth-1,kApaMatrixHeight-1)
  leds.DrawLine(0, 0, kApaMatrixWidth-1, kApaMatrixHeight-1, CRGB(0x80,0,0));

  // draw a green line from (kApaMatrixWidth-1,0) to (0,kApaMatrixHeight-1)  // 
  leds.DrawLine(kApaMatrixWidth-1, 0, 0, kApaMatrixHeight-1, CRGB(0,0x80,0));

  // Draw F, a recognizable shape that shows mirroring and rotation easily, centered in tile
  for(int i=0; i<abs(MATRIX_TILE_H); i++) {
    for(int j=0; j<abs(MATRIX_TILE_V); j++) {
      drawF((abs(MATRIX_TILE_WIDTH)*i) + abs(MATRIX_TILE_WIDTH)/2, (abs(MATRIX_TILE_HEIGHT)*j) + abs(MATRIX_TILE_HEIGHT)/2);
    }
  }
  apaBackgroundLayer.swapBuffers();
  apabuffer = apaBackgroundLayer.backBuffer();
  leds.SetLEDArray((CRGB*)apabuffer);

  Serial.println("Press enter to generate map");
  Serial.flush(); //flush all previous received and transmitted data
  while(!Serial.available()) ; // hang program until a byte is received notice the ; after the while()

  Serial.println("map");

  for(int i=0; i<kApaMatrixWidth; i++) {
    for(int j=0; j<kApaMatrixHeight; j++) {
      leds.DrawPixel(i, j, CRGB(i, j, 0));
    }
  }

  // we shift out pixels to the matrix a row at a time, so the pixel index is (width + height*(kMatrixWidth))
  // we've loaded the RGB values in the current apaBackgroundLayer drawing buffer with the X/Y coordinates of the pixel that is drawn to at that pixel's index (R=X, G=Y)
  // we want to generate a map that returns an index given X/Y coordinates.
  // Starting with X=0, Y=0, we need to search through all the pixels in the buffer for one with R=X, G=Y, and then print that pixel's index

#if 1
  Serial.println("**** Copy the below code to FastLed_Panel_Plus_Apa102 or a similar sketch ****");

  Serial.println("uint16_t getPixelFromXYCallback(int16_t x, int16_t y) {");
  Serial.println("  // any out of bounds address maps to the first hidden pixel");
  Serial.println("  if ( (x >= kApaMatrixWidth) || (y >= kApaMatrixHeight) ) {");
  Serial.println("    return (kApaMatrixWidth*kApaMatrixHeight);");
  Serial.println("  }");
  Serial.println();
  Serial.println("const uint16_t XYTable[] = {");
  Serial.print("    ");

  int currentIndex = 0;
  for(int y=0; y<kApaMatrixHeight; y++) {
    for(int x=0; x<kApaMatrixWidth; x++) {
      // we're looking for the pixel with R=x, G=y in the buffer
      for(int j=0; j<kApaMatrixHeight; j++) {
        for(int i=0; i<kApaMatrixWidth; i++) {
          rgb24 tempPixel = apaBackgroundLayer.readPixel(i, j);
          if(tempPixel.red == x && tempPixel.green == y){
            Serial.print(j*kApaMatrixWidth + i);
            XYTableTest[currentIndex] = j*kApaMatrixWidth + i;
          }
        }
      }
      currentIndex++;
      if(currentIndex < kApaMatrixWidth*kApaMatrixHeight) {
        Serial.print(",");
        if(currentIndex%16 == 0) {
          Serial.println();
          Serial.print("    ");
        }
      }
    }
  }

  Serial.println();
  Serial.println("  };");
  Serial.println();
  Serial.println("  uint16_t i = (y * kApaMatrixWidth) + x;");
  Serial.println("  uint16_t j = XYTable[i];");
  Serial.println("  return j;");
  Serial.println("}");

  Serial.println("**** Add these lines into setup() before apamatrix.begin();  ****");
  Serial.println("  apamatrix.setSerpentineLayout(false);");
  Serial.println("  apamatrix.setGetPixelFromXYCallback(getPixelFromXYCallback);");

  Serial.println("**** Use the same kApaMatrixWidth and kApaMatrixHeight values you used in this sketch ****");
  Serial.println();
  Serial.println();
  Serial.println();
#endif

  // Verifying that the mapping works, you should see two lines, matching the lines drawn with LEDMatrix, but this time drawn with the backgroundLayer's drawing functions, and using the mapping in SmartMatrix Library
  apaBackgroundLayer.fillScreen({0,0,0});
  // draw a blue line from (0,0) to (kApaMatrixWidth-1,kApaMatrixHeight-1)
  apaBackgroundLayer.drawLine(0, 0, kApaMatrixWidth-1, kApaMatrixHeight-1, CRGB(0,0,0x80));

  // draw a yellow line from (kApaMatrixWidth-1,0) to (0,kApaMatrixHeight-1)
  apaBackgroundLayer.drawLine(kApaMatrixWidth-1, 0, 0, kApaMatrixHeight-1, CRGB(0x80,0x80,0));

  apamatrix.setGetPixelFromXYCallback(getPixelFromXYCallback);
  apaBackgroundLayer.swapBuffers();

}

void drawF(int x, int y) {
  leds.DrawLine(x-2, y-3, x+2, y-3, CRGB(0xff,0xff,0xff)); // top horizontal line
  leds.DrawLine(x-2, y, x+1, y, CRGB(0xff,0xff,0xff)); // middle horizontal line
  leds.DrawLine(x-2, y-3, x-2, y+3, CRGB(0xff,0xff,0xff)); // left vertical line
}

void loop() {
  
}
