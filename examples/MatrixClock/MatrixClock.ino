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
 */

#include <Wire.h>
#include <Time.h>
#include <DS1307RTC.h>
#include <SmartMatrix_32x32.h>

SmartMatrix matrix;

const rgb24 clockColor = {0xff, 0xff, 0xff};

void setup() {
  Serial.begin(9600);
  delay(200);
  Serial.println("DS1307RTC Read Test");
  Serial.println("-------------------");

  /* I2C Changes Needed for SmartMatrix Shield */
  // switch pins to use 16/17 for I2C instead of 18/19
  pinMode(18, INPUT);
  pinMode(19, INPUT);
  CORE_PIN16_CONFIG = (PORT_PCR_MUX(2) | PORT_PCR_PE | PORT_PCR_PS);
  CORE_PIN17_CONFIG = (PORT_PCR_MUX(2) | PORT_PCR_PE | PORT_PCR_PS);

  // setup matrix
  matrix.begin();
  matrix.setScrollFont(font3x5);
}

void loop() {
  tmElements_t tm;
    int x=1;
    char timeBuffer[9];

  // clear screen before writing new text
  matrix.fillScreen({0,0,0});

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
        x = 4;
    matrix.setFont(gohufont11b);
    matrix.drawString(x, MATRIX_HEIGHT/2 - 6, clockColor, timeBuffer);
    matrix.swapBuffers();
  } else {
    if (RTC.chipPresent()) {
      Serial.println("The DS1307 is stopped.  Please run the SetTime");
      Serial.println("example to initialize the time and begin running.");
      Serial.println();

      /* Draw Error Message to SmartMatrix */
      matrix.setFont(font3x5);
      sprintf(timeBuffer, "Stopped");
      matrix.drawString(x, MATRIX_HEIGHT/2 - 3, clockColor, "Stopped");
    } else {
      Serial.println("DS1307 read error!  Please check the circuitry.");
      Serial.println();

      /* Draw Error Message to SmartMatrix */
      matrix.setFont(font3x5);
      matrix.drawString(x, MATRIX_HEIGHT/2 - 3, clockColor, "No Clock");
    }
    matrix.swapBuffers();
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
