/*
 * This example is pulled from the DS1307RTC library with slight
 * modifications to demonstrate how to use the alternate I2C pins
 * accessible on the SmartMatrix Shield.
 *
 * Original DS1307RTC Library:
 * https://www.pjrc.com/teensy/td_libs_DS1307RTC.html
 *
 * Requires a DS1307, DS1337 or DS3231 RTC chip connected to
 * Teensy pins 16 (SCL) and 17 (SDA)
 * 
 * This SmartMatrix example uses just the indexed color layer
 */

#include <Wire.h>
#include <Time.h>
#include <DS1307RTC.h>
#include <SmartMatrix3.h>

#define COLOR_DEPTH 24                  // known working: 24, 48 - If the sketch uses type `rgb24` directly, COLOR_DEPTH must be 24
const uint8_t kMatrixWidth = 32;        // known working: 32, 64, 96, 128
const uint8_t kMatrixHeight = 32;       // known working: 16, 32, 48, 64
const uint8_t kRefreshDepth = 36;       // known working: 24, 36, 48
const uint8_t kDmaBufferRows = 4;       // known working: 2-4, use 2 to save memory, more to keep from dropping frames and automatically lowering refresh rate
const uint8_t kPanelType = SMARTMATRIX_HUB75_32ROW_MOD16SCAN;   // use SMARTMATRIX_HUB75_16ROW_MOD8SCAN for common 16x32 panels
const uint8_t kMatrixOptions = (SMARTMATRIX_OPTIONS_NONE);      // see http://docs.pixelmatix.com/SmartMatrix for options
const uint8_t kIndexedLayerOptions = (SM_INDEXED_OPTIONS_NONE);

SMARTMATRIX_ALLOCATE_BUFFERS(matrix, kMatrixWidth, kMatrixHeight, kRefreshDepth, kDmaBufferRows, kPanelType, kMatrixOptions);
SMARTMATRIX_ALLOCATE_INDEXED_LAYER(indexedLayer, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kIndexedLayerOptions);

const int defaultBrightness = 100*(255/100);    // full brightness
//const int defaultBrightness = 15*(255/100);    // dim: 15% brightness

const SM_RGB clockColor = {0xff, 0xff, 0xff};

void setup() {
  Serial.begin(9600);
  delay(200);
  Serial.println("DS1307RTC Read Test");
  Serial.println("-------------------");

  // setup matrix
  matrix.addLayer(&indexedLayer); 
  matrix.begin();

  /* I2C Changes Needed for SmartMatrix Shield */
  // switch pins to use 16/17 for I2C instead of 18/19, after calling matrix.begin()//
  pinMode(18, INPUT);
  pinMode(19, INPUT);
  CORE_PIN16_CONFIG = (PORT_PCR_MUX(2) | PORT_PCR_PE | PORT_PCR_PS);
  CORE_PIN17_CONFIG = (PORT_PCR_MUX(2) | PORT_PCR_PE | PORT_PCR_PS);

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
