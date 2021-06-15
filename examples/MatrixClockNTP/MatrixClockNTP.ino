/*
 * This example is based on SmartMatrix example MatrixClock but replaced RTC in favor of NTP time
 * note: DS1307RTC lib as it is seems to even crash on ESP32, might need to check pins though ...
 *
 * Requires my for of the NTPClient:
 * https://github.com/lefty01/NTPClient
 *
 * Samples for debug_print.h and wifi_mqtt_creds.h file can be found here:
 * https://github.com/lefty01/mysnippez/blob/master/arduino/wifi_mqtt_creds.h
 *
 * Only tested on ESP32 with a 64x64 (P3) RGB Panel ... which works ;)
 * 
 * This SmartMatrix example uses just the indexed color layer
 *
 */

#include <Wire.h>
#include <Time.h>

#include <WiFi.h>
#include <HTTPClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#include <MatrixHardware_ESP32_V0.h>
#include <SmartMatrix.h>

#define DEBUG 1
#define NOMQTTCERTS 1
#define USE_ANDROID_AP 1
#include "debug_print.h"
#include "wifi_mqtt_creds.h"

//const long gmtOffset = 3600; // UTC to CET offset in sec Winterzeit
const long gmtOffset = 7200; // UTC to CET offset in sec Sommerzeit
// Central European Time (Frankfurt, Paris)
TimeChangeRule CEST = {Last, Sun, Mar, 2, 120};     // Central European Summer Time
TimeChangeRule CET = {Last, Sun, Oct, 3, 60};       // Central European Standard Time
//struct DateTime cur_dateTime;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, gmtOffset, CEST, CET);

// SmartMatrix Defines
#define COLOR_DEPTH   24            // known working: 24, 48 - If the sketch uses type `rgb24` directly, COLOR_DEPTH must be 24
#define kMatrixWidth  64            // known working: 32, 64, 96, 128
#define kMatrixHeight 64            // known working: 16, 32, 48, 64
const uint8_t kRefreshDepth  = 24;  // known working: 24, 36, 48
const uint8_t kDmaBufferRows =  2;  // known working: 2-4, use 2 to save memory, more to keep from dropping frames and automatically lowering refresh rate

const uint8_t kPanelType              = SMARTMATRIX_HUB75_64ROW_MOD32SCAN;
const uint8_t kMatrixOptions          = (SMARTMATRIX_OPTIONS_NONE);      // see http://docs.pixelmatix.com/SmartMatrix for options
const uint8_t kBackgroundLayerOptions = (SM_BACKGROUND_OPTIONS_NONE);
const uint8_t kIndexedLayerOptions    = (SM_INDEXED_OPTIONS_NONE);

SMARTMATRIX_ALLOCATE_BUFFERS(matrixLayer, kMatrixWidth, kMatrixHeight, kRefreshDepth, kDmaBufferRows, kPanelType, kMatrixOptions);
SMARTMATRIX_ALLOCATE_BACKGROUND_LAYER(backgroundLayer, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kBackgroundLayerOptions);
SMARTMATRIX_ALLOCATE_INDEXED_LAYER(indexedLayer, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kIndexedLayerOptions);

//const int defaultBrightness = (100*255)/100;      // full (100%) brightness
//const int defaultBrightness = (15*255)/100;       // dim: 15% brightness
const int defaultBrightness = (90*255)/100;         // dim: 90% brightness


const SM_RGB clockColor = {0xff, 0xff, 0xff};


String ipAddr;
String dnsAddr;
const unsigned maxWifiWaitSeconds = 60;
bool isWifiAvailable = false;


void print2digits(int number) {
  if (number >= 0 && number < 10) {
    DEBUG_PRINT('0');
  }
  DEBUG_PRINT(number);
}


int setupWifi() {
  DEBUG_PRINTLN();
  DEBUG_PRINTLN("Connecting to wifi");

  unsigned retry_counter = 0;
  WiFi.begin(wifi_ssid, wifi_pass);

  while (WiFi.status() != WL_CONNECTED) {

    delay(500);
    DEBUG_PRINT(".");

    retry_counter++;
    if (retry_counter > maxWifiWaitSeconds) {
      DEBUG_PRINTLN(" TIMEOUT!");
      return 1;
    }
  }
  ipAddr  = WiFi.localIP().toString();
  dnsAddr = WiFi.dnsIP().toString();

  DEBUG_PRINTLN("");
  DEBUG_PRINTLN("WiFi connected");
  DEBUG_PRINTLN("IP address: ");
  DEBUG_PRINTLN(ipAddr);
  DEBUG_PRINTLN("DNS address: ");
  DEBUG_PRINTLN(dnsAddr);

  return 0;
}



void setup() {
  DEBUG_BEGIN(115200);
  delay(200);
  DEBUG_PRINTLN("Matrix Clock Setup - NTP Version");
  DEBUG_PRINTLN("-------------------");
  isWifiAvailable = setupWifi() ? false : true;

  timeClient.begin();
  timeClient.update();


  // setup matrix
  matrixLayer.addLayer(&indexedLayer); 
  matrixLayer.begin();
  matrixLayer.setBrightness(defaultBrightness);
  
  // display a simple message - will stay on the screen if calls to the RTC library fail later
  indexedLayer.fillScreen(0);
  indexedLayer.setFont(font3x5); //gohufont11); // apple3x5
  indexedLayer.drawString(0, kMatrixHeight / 2 - 6, 1, "CLOCK");
  indexedLayer.swapBuffers(false);

  // print network info ...
  indexedLayer.drawString(0, kMatrixHeight / 2 + 6, 1, ipAddr.c_str());

  delay(3000);
}

void loop() {
  int x = kMatrixWidth/2-22;

  bool rc = timeClient.update();

  // output time to console
  DEBUG_PRINT("Ok, Time = ");
  DEBUG_PRINTLN(timeClient.getFormattedTime());
  DEBUG_PRINT("Ok, Date = ");
  DEBUG_PRINTLN(timeClient.getFormattedDate());

  // clear screen before writing new text
  indexedLayer.fillScreen(0);

  /* Draw Clock to SmartMatrix */
  uint8_t hour = timeClient.getHours(); //cur_dateTime.dt_hours;
  if (hour > 12)
    hour -= 12;

  if (hour < 10) // move to right if double digit hour
    x += 3;

  indexedLayer.setFont(gohufont11b);
  indexedLayer.drawString(x, kMatrixHeight / 2 - 10, 1, timeClient.getFormattedTime().c_str());
  indexedLayer.drawString(x, kMatrixHeight / 2 + 10, 1, timeClient.getFormattedDate().c_str());

  indexedLayer.swapBuffers();

  delay(1000);
}

