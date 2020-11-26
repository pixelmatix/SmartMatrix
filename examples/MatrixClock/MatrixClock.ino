/*
 * This example is pulled from the DS1307RTC library with slight
 * modifications to demonstrate how to use the alternate I2C pins
 * accessible on the SmartMatrix Shield.
 *
 * Original DS1307RTC Library:
 * https://www.pjrc.com/teensy/td_libs_DS1307RTC.html
 *
 * Requires a DS1307, DS1337 or DS3231 RTC chip connected to
 * Teensy pins 19 (SCL) and 18 (SDA)
 * 
 * Use 3.3V power for RTC module and I2S Pullup Resistors as some Teensy modules aren't 5V tolerant
 *
 * If using an old (V1-V3) SmartMatrix Shield or bare Teensy 3 with address pins connected to your matrix
 * you'll need to connect Teensy pins 16 (SCL) and 17 (SDA), and uncomment the alternate pin code below
 *
 * Not tested, and most likely doesn't work on ESP32
 * 
 * This SmartMatrix example uses just the indexed color layer
 */

#include <Wire.h>
#include <Time.h>
#include <DS1307RTC.h>

// uncomment one line to select your MatrixHardware configuration - configuration header needs to be included before <SmartMatrix.h>
//#include <MatrixHardware_Teensy3_ShieldV4.h>        // SmartLED Shield for Teensy 3 (V4)
//#include <MatrixHardware_Teensy4_ShieldV5.h>        // SmartLED Shield for Teensy 4 (V5)
//#include <MatrixHardware_Teensy3_ShieldV1toV3.h>    // SmartMatrix Shield for Teensy 3 V1-V3
//#include <MatrixHardware_Teensy4_ShieldV4Adapter.h> // Teensy 4 Adapter attached to SmartLED Shield for Teensy 3 (V4)
//#include <MatrixHardware_ESP32_V0.h>                // This file contains multiple ESP32 hardware configurations, edit the file to define GPIOPINOUT (or add #define GPIOPINOUT with a hardcoded number before this #include)
//#include "MatrixHardware_Custom.h"                  // Copy an existing MatrixHardware file to your Sketch directory, rename, customize, and you can include it like this
#include <SmartMatrix.h>

#define COLOR_DEPTH 24                  // Choose the color depth used for storing pixels in the layers: 24 or 48 (24 is good for most sketches - If the sketch uses type `rgb24` directly, COLOR_DEPTH must be 24)
const uint16_t kMatrixWidth = 32;       // Set to the width of your display, must be a multiple of 8
const uint16_t kMatrixHeight = 32;      // Set to the height of your display
const uint8_t kRefreshDepth = 36;       // Tradeoff of color quality vs refresh rate, max brightness, and RAM usage.  36 is typically good, drop down to 24 if you need to.  On Teensy, multiples of 3, up to 48: 3, 6, 9, 12, 15, 18, 21, 24, 27, 30, 33, 36, 39, 42, 45, 48.  On ESP32: 24, 36, 48
const uint8_t kDmaBufferRows = 4;       // known working: 2-4, use 2 to save RAM, more to keep from dropping frames and automatically lowering refresh rate.  (This isn't used on ESP32, leave as default)
const uint8_t kPanelType = SM_PANELTYPE_HUB75_32ROW_MOD16SCAN;   // Choose the configuration that matches your panels.  See more details in MatrixCommonHub75.h and the docs: https://github.com/pixelmatix/SmartMatrix/wiki
const uint32_t kMatrixOptions = (SM_HUB75_OPTIONS_NONE);        // see docs for options: https://github.com/pixelmatix/SmartMatrix/wiki
const uint8_t kIndexedLayerOptions = (SM_INDEXED_OPTIONS_NONE);

SMARTMATRIX_ALLOCATE_BUFFERS(matrix, kMatrixWidth, kMatrixHeight, kRefreshDepth, kDmaBufferRows, kPanelType, kMatrixOptions);
SMARTMATRIX_ALLOCATE_INDEXED_LAYER(indexedLayer, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kIndexedLayerOptions);

const int defaultBrightness = (100*255)/100;      // full (100%) brightness
//const int defaultBrightness = (15*255)/100;     // dim: 15% brightness

const SM_RGB clockColor = {0xff, 0xff, 0xff};

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("DS1307RTC Read Test");
  Serial.println("-------------------");

  // setup matrix
  matrix.addLayer(&indexedLayer); 
  matrix.begin();

  /* I2C Changes Needed for old SmartMatrix Shield (V1-V3) or bare Teensy 3 with address pins connected to matrix */
  // switch pins to use 16/17 for I2C instead of 18/19, after calling matrix.begin()//
#if 0
  pinMode(18, INPUT);
  pinMode(19, INPUT);
  CORE_PIN16_CONFIG = (PORT_PCR_MUX(2) | PORT_PCR_PE | PORT_PCR_PS);
  CORE_PIN17_CONFIG = (PORT_PCR_MUX(2) | PORT_PCR_PE | PORT_PCR_PS);

  // These code might work instead, but haven't been tested
  //Wire.setSDA(17);
  //Wire.setSCL(16);
#endif

  // display a simple message - will stay on the screen if calls to the RTC library fail later
  indexedLayer.fillScreen(0);
  indexedLayer.setFont(gohufont11b);
  indexedLayer.drawString(0, kMatrixHeight / 2 - 6, 1, "CLOCK");
  indexedLayer.swapBuffers(false);

  matrix.setBrightness(defaultBrightness);
}

void loop() {
  tmElements_t tm;
    int x = kMatrixWidth/2-15;
    char timeBuffer[9];

  // clear screen before writing new text
  indexedLayer.fillScreen(0);

  if (RTC.read(tm)) {
    Serial.print("Ok, Time = ");
    print2digits(tm.Hour);
    Serial.write(':');
    print2digits(tm.Minute);
    Serial.write(':');
    print2digits(tm.Second);
    Serial.print(", Date (D/M/Y) = ");
    Serial.print(tm.Day);
    Serial.write('/');
    Serial.print(tm.Month);
    Serial.write('/');
    Serial.print(tmYearToCalendar(tm.Year));
    Serial.println();

    /* Draw Clock to SmartMatrix */
    uint8_t hour = tm.Hour;
    if (hour > 12)
        hour -= 12;
    sprintf(timeBuffer, "%d:%02d", hour, tm.Minute);
    if (hour < 10)
        x += 3;
    indexedLayer.setFont(gohufont11b);
    indexedLayer.drawString(x, kMatrixHeight / 2 - 6, 1, timeBuffer);
    indexedLayer.swapBuffers();
  } else {
    if (RTC.chipPresent()) {
      Serial.println("The DS1307 is stopped.  Please run the SetTime");
      Serial.println("example to initialize the time and begin running.");
      Serial.println();

      /* Draw Error Message to SmartMatrix */
      indexedLayer.setFont(font3x5);
      sprintf(timeBuffer, "Stopped");
      indexedLayer.drawString(x, kMatrixHeight / 2 - 3, 1, "Stopped");
    } else {
      Serial.println("DS1307 read error!  Please check the circuitry.");
      Serial.println();

      /* Draw Error Message to SmartMatrix */
      indexedLayer.setFont(font3x5);
      indexedLayer.drawString(x, kMatrixHeight / 2 - 3, 1, "No Clock");
    }
    indexedLayer.swapBuffers();
    delay(9000);
  }
  delay(1000);
}

void print2digits(int number) {
  if (number >= 0 && number < 10) {
    Serial.write('0');
  }
  Serial.print(number);
}
