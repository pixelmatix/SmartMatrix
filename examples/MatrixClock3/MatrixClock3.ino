/*
 * This clock example uses the RTC of a Teensy3 (Teensy 3.0 or 3.1).
 *
 * RTC clock is set in response to serial port time message 
 * A Processing example sketch to set the time is included in the download
 * On Linux, you can use "date +T%s > /dev/ttyUSB0" (UTC time zone)
 * 
 * Or, if a time zone is to be set (and setting TZ doesn't do the trick):
 *
 * Example: UTC+2
 *   date -d "+2 hours" +T%s > /dev/ttyUSB0
 *
 * Time code is taken from example TimeTeensy3.ino.
 *
 * SmartMatrix Features Demo - Wolfgang Astleitner mrwastl@users.sf.net
 * This example code is released into the public domain
 */

#include <Time.h>
#include <SmartMatrix.h>

//#define DEBUG_SERIAL

rgb24   bgCol       = {0x00, 0x00, 0x00};  // background colour

rgb24   timeCol     = {0x98, 0x40, 0xa4};  // time: segment colour
rgb24   dateCol     = {0x00, 0x00, 0x9f};  // date: segment colour
//rgb24   timeCol     = {0x2f, 0x2f, 0xff};  // time: segment colour
//rgb24   dateCol     = {0x00, 0x00, 0x9f};  // date: segment colour

uint8_t showSecs    = 2;                   // display seconds: 0: no, 1: yes, 2: no seconds, but blinking dots
uint8_t showYear    = 0;                   // show year: 0 no, 1: yes



uint8_t timeColPerc = 10;                  // time: brightness of 'passive segment' effect in percent
uint8_t dateColPerc = 10;                  // date: brightness of 'passive segment' effect in percent


uint8_t timeSegT    = 4;                   // time: thickness of a segment (in pixel), reduce for smaller displays
uint8_t dateSegT    = 3;                   // date: thickness of a segment (in pixel), reduce for smaller displays

uint8_t timeStartX  = 0xFF;                // x start position of time segments, 0xFF: auto center time
uint8_t timeStartY  = timeSegT;            // y start position of time segments
uint8_t timeMaxH    = 30;                  // maximal height of time segments, reduce for smaller displays

uint8_t dateStartX  = 0xFF;                // x start position of date segments, 0xFF: auto center date segments horizontally
uint8_t dateStartY  = 0xFF;                // y start position of date segments, 0xFF: auto center date segments vertically
uint8_t dateMaxH    = 20;                  // maximal height of date segments, reduce for smaller displays



// values not to be changed
SmartMatrix matrix;

byte    currSec    = 60;

uint8_t w          = 0;
uint8_t h          = 0;
uint8_t timeSegW   = 0;
uint8_t dateSegW   = 0;

uint8_t timeSegH   = 0;
uint8_t dateSegH   = 0;

uint8_t timeW      = 0;
uint8_t dateW      = 0;

uint8_t colonPos   = 0;

rgb24 timeColFx;  // time: calculated colour of 'non active segment' effect
rgb24 dateColFx;  // date: calculated colour of 'non active segment' effect

void setup() {
  // set the Time library to use Teensy 3.x's RTC to keep time
  setSyncProvider(getTeensy3Time);
  
  Serial.begin(9600);
  delay(200);
  if (timeStatus()!= timeSet) {
    Serial.println("Unable to sync with the RTC");
  } else {
    Serial.println("RTC has set the system time");
  }

  // setup matrix
  matrix.begin();

  w = matrix.getScreenWidth();
  h = matrix.getScreenHeight();

  // out-of-bound checks
  if (timeColPerc > 100) timeColPerc = 100;
  if (dateColPerc > 100) dateColPerc = 100;
  if (showSecs > 2) showSecs = 2;
  if (showYear > 1) showYear = 1;

  // calculate passive segment colours
  timeColFx.red   = (uint8_t)((uint16_t)timeCol.red   * timeColPerc / 100);
  timeColFx.green = (uint8_t)((uint16_t)timeCol.green * timeColPerc / 100);
  timeColFx.blue  = (uint8_t)((uint16_t)timeCol.blue  * timeColPerc / 100);
  dateColFx.red   = (uint8_t)((uint16_t)dateCol.red   * dateColPerc / 100);
  dateColFx.green = (uint8_t)((uint16_t)dateCol.green * dateColPerc / 100);
  dateColFx.blue  = (uint8_t)((uint16_t)dateCol.blue  * dateColPerc / 100);


  // calculate width and height of segments
  if (timeMaxH == 0 && timeStartX == 0xFF) { // auto width and auto center
    timeSegW = ( w - ((showSecs == 1) ? 9 : 5) * timeSegT ) / ((showSecs == 1) ? 6 : 4);
  } else {
    timeSegW = (timeMaxH + timeSegT) / 2;    
  }
  timeW    = timeSegW * ((showSecs == 1) ? 6 : 4) + timeSegT * ( (showSecs == 1) ? 9 : 5);                              
  timeSegH = timeSegW * 2 - timeSegT;

  if (dateMaxH == 0 && dateStartX == 0xFF) { // auto width and auto center
    dateSegW = ( w - ((showYear == 1) ? 9 : 4) * dateSegT ) / ((showYear == 1) ? 10 : 5);
  } else {
    dateSegW = (dateMaxH + dateSegT) / 2;
  }

  dateW = dateSegW * ((showYear == 1) ? 10 : 5) + dateSegT * ( (showYear == 1) ? 9 : 4);                              
  dateSegH = dateSegW * 2 - dateSegT;

  // y-offset of 2 colon points
  colonPos = (timeSegH - 2* timeSegT ) / 3;
}

void loop() {
  time_t t = processSyncMessage();
  if (t != 0) {
    Teensy3Clock.set(t); // set the RTC
    setTime(t);
  }
  
  uint8_t x;
  uint8_t y;
  
  int tSec = second();
  int tMin = minute();
  int tHour = hour();
  int tYear = year();
  int tMon = month();
  int tDay = day();


  if (tSec != currSec) {
    // clear screen before writing new text
    matrix.fillScreen(bgCol);

    currSec = tSec;

#ifdef DEBUG_SERIAL
    Serial.print("Ok, Time = ");
    print2digits(tHour);
    Serial.write(':');
    print2digits(tMin);
    Serial.write(':');
    print2digits(tSec);
    Serial.print(", Date (D/M/Y) = ");
    Serial.print(tDay);
    Serial.write('/');
    Serial.print(tMon);
    Serial.write('/');
    Serial.print(tYear);
    Serial.println();
#endif

    // start position of time segments
    if (timeStartX == 0xFF) { // auto center
      x = ( w - timeW ) / 2;
    } else {
      x = timeStartX;
    }

    y = timeStartY;
    
    x += drawNumber(x, y, tHour, timeSegW, timeSegT, 2, timeCol, timeColFx);
    x += timeSegT;
    if (showSecs != 2 || (tSec % 2) ) {
      drawBar(x, y + colonPos, timeSegT, timeSegT, 0, timeCol);
      drawBar(x, y + timeSegH - timeSegT - colonPos, timeSegT, timeSegT, 0, timeCol);
    } else {
      drawBar(x, y + colonPos, timeSegT, timeSegT, 0, timeColFx);
      drawBar(x, y + timeSegH - timeSegT - colonPos, timeSegT, timeSegT, 0, timeColFx);
    }
    x += timeSegT * 2;
    x += drawNumber(x, y, tMin, timeSegW, timeSegT, 2, timeCol, timeColFx);
    if (showSecs == 1) {
      x += timeSegT;
      drawBar(x, y + colonPos, timeSegT, timeSegT, 0, timeCol);
      drawBar(x, y + timeSegH - timeSegT - colonPos, timeSegT, timeSegT, 0, timeCol);
      x += timeSegT * 2;
      x += drawNumber(x, y, tSec, timeSegW, timeSegT, 2, timeCol, timeColFx);
    }
    y += timeSegH;
    
    // start position of time segments
    if (dateStartX == 0xFF) { // auto center
      x = ( w - dateW ) / 2;
    } else {
      x = dateStartX;
    }

    if (dateStartY == 0xFF) { // auto place
      y += (h - y - dateSegH) / 2;
    } else {
      y = dateStartY;
    }

    if (showYear) {    
      x += drawNumber(x, y, tYear, dateSegW, dateSegT, 4, dateCol, dateColFx);
      x += dateSegT;
      drawBar(x, y + (dateSegH / 2) - 1, dateSegW, dateSegT, 1, dateCol);
      x += dateSegW + dateSegT;
      x += drawNumber(x, y, tMon, dateSegW, dateSegT, 2, dateCol, dateColFx);
      x += dateSegT;
      drawBar(x, y + (dateSegH / 2) - 1, dateSegW, dateSegT, 1, dateCol);
      x += dateSegW + dateSegT;
      x += drawNumber(x, y, tDay, dateSegW, dateSegT, 2, dateCol, dateColFx);
    } else {
      x += drawNumber(x, y, tDay, dateSegW, dateSegT, 2, dateCol, dateColFx);
      x += dateSegT;
      drawBar(x + (dateSegW - dateSegT) / 2, y + dateSegH - dateSegT, dateSegT, dateSegT, 1, dateCol);
      x += dateSegW + dateSegT;
      x += drawNumber(x, y, tMon, dateSegW, dateSegT, 2, dateCol, dateColFx);
    }

    matrix.swapBuffers();
  } else {
    /*delay(1000);*/
    delay(10);
  }
}


time_t getTeensy3Time() {
  return Teensy3Clock.get();
}


/*  code to process time sync messages from the serial port   */
#define TIME_HEADER  "T"   // Header tag for serial time sync message

unsigned long processSyncMessage() {
  unsigned long pctime = 0L;
  const unsigned long DEFAULT_TIME = 1357041600; // Jan 1 2013 

  if(Serial.find(TIME_HEADER)) {
     pctime = Serial.parseInt();
     return pctime;
     if( pctime < DEFAULT_TIME) { // check the value is a valid time (greater than Jan 1 2013)
       pctime = 0L; // return 0 to indicate that the time is not valid
     }
  }
  return pctime;
}


void print2digits(int number) {
  if (number >= 0 && number < 10) {
    Serial.write('0');
  }
  Serial.print(number);
}



/* taken from my library 'serdisplib', testserdisp,c */

void drawBar(int x, int y, int w, int h, int hor, rgb24 col) {
  int i,j;
  for (j = y; j < y + ((hor)? h : w); j ++)
    for (i = x; i < x + ((hor)? w : h); i ++)
      matrix.drawPixel(i, j, col);
}


void drawDigit(int x, int y, int digit, int segwidth, int thick, rgb24 col, rgb24 colFX) {
  if (digit < 0 || digit > 9) return;

  // draw 'passive' 8
  drawBar(x,                    y,                        segwidth, thick, 1, colFX);
  drawBar(x,                    y + segwidth - thick,     segwidth, thick, 1, colFX);
  drawBar(x,                    y + 2*(segwidth - thick), segwidth, thick, 1, colFX);

  drawBar(x,                    y,                        segwidth, thick, 0, colFX);
  drawBar(x + segwidth - thick, y,                        segwidth, thick, 0, colFX);
  drawBar(x,                    y + segwidth - thick,     segwidth, thick, 0, colFX);
  drawBar(x + segwidth - thick, y + segwidth - thick,     segwidth, thick, 0, colFX);

  // draw active digit
  if ( digit != 1 && digit != 4 )
    drawBar(x,                    y,                        segwidth, thick, 1, col);
  if ( digit != 1 && digit != 7 && digit != 0 )
    drawBar(x,                    y + segwidth - thick,     segwidth, thick, 1, col);
  if ( digit != 1 && digit != 4 && digit != 7 )
    drawBar(x,                    y + 2*(segwidth - thick), segwidth, thick, 1, col);

  if ( digit != 1 && digit != 2 && digit != 3 && digit != 7 )
    drawBar(x,                    y,                        segwidth, thick, 0, col);
  if ( digit != 5 && digit != 6 )
    drawBar(x + segwidth - thick, y,                        segwidth, thick, 0, col);
  if ( digit == 2 || digit == 6 || digit == 8 || digit == 0 )
    drawBar(x,                    y + segwidth - thick,     segwidth, thick, 0, col);
  if ( digit != 2 )
    drawBar(x + segwidth - thick, y + segwidth - thick,     segwidth, thick, 0, col);
}


int drawNumber(int x, int y, int number, int segwidth, int thick, int digits, rgb24 col, rgb24 colFX) {
  int i, posx = x + (segwidth + thick ) * (digits - 1);

  int num = number;

  if (num < 0 || num > 9999) return 0;

  for (i = digits-1; i >= 0; i--) {
    drawDigit(posx, y, num % 10, segwidth, thick, col, colFX);
    posx -= segwidth + thick;
    num /= 10;
  }
  return ( segwidth * digits + thick * (digits - 1) );
}

