/*
FULL COMMENTED CODE IN NOTES: WORKIGN WELL PAGES
*/

#include <HTTPClient.h>

#include <arpa/inet.h>
#include <netinet/in.h>

#include <Arduino.h>
#include "esp_sleep.h" //for sleep
#include "WiFi.h"      //to turn OFF wifi module

#include "Utils.h"
#include "MySensor.h"
#include "OLED.h"
#include "PumpControl.h"

const uint8_t WAKEUP_PIN = 33;      // pin that whne going high stops the sleep of the chip as external wakeup
const uint8_t OLED_PIN = 2;         // pin that controls the VCC of the OLED module
const uint8_t SENSOR_PIN = 4;       // pin that controls the VCC of the sensor
const uint8_t PUMP_PIN_MOSFET = 25; // GPIO driving the XY-MOS TRIG/PWM pin. this control the pump motor

#define uS_TO_S_FACTOR 1000000ULL // Conversion factor for microseconds to seconds
#define TIME_TO_SLEEP 10          // Time ESP32 will go to sleep

// #define PUMPCONTROL_OFF

/*
  all variables declared as RTC_DATA_ATTR are restored after waking up.
*/
RTC_DATA_ATTR uint8_t percent = 0;
RTC_DATA_ATTR bool displayON = true;             // control the display ON and OFF
RTC_DATA_ATTR bool motorON = false;              // control the pump motor
RTC_DATA_ATTR bool OLEDPin_state_changed = true; // if the switch is pressed to turn display ON and OFF, this is set to True
RTC_DATA_ATTR int counter = 0;                   // counter on how many tiems the chip went to sleep
RTC_DATA_ATTR bool pumpOn = false;               // togle for ON OFF for pump
RTC_DATA_ATTR float voltsSensorVal = 0;          // value use whne redign sensor voltage

const uint8_t ADC_PIN = 34;   // GPIO34 that control the VCC power ON and OFF (MySensor)
const uint16_t ADC_ITER = 10; // number of times the pin reading is sampled (MySensor)

// pump control values passed as pointers to PumpControl
const uint8_t lowPercentThreshold = 15;  // if the percent lower than that, pump is turned ON
const uint8_t highPercentThreshold = 60; // When motor pump is ON, it will stop whne percent >= this value
const uint8_t pumpPWMValue = 255;        // this sets the speed of the pump motor

// // used to map the values for percents
const float dryValue = 2.58; // volts
const float wetValue = 0.94; // volts


/*
Whne the wifi is on hte display must be off as 
there is not enough space for the entire code
for both display and wifi
*/

#define WIFI_OUTPUT

#ifdef WIFI_OUTPUT
// Replace with your SSID and Password
const char *ssid = "BT-9TAJSN";
const char *password = "NEtLgxRuhDGh6V";
const char *serverName = "https://script.google.com/macros/s/AKfycbz5co3hnWUnCaz06d4znbZ7AuWzXQa9EL0iNopfrWJyXyOyNjGQtMqCphzPT5yyssGH/exec";
unsigned long timeRead = 0;
#endif

MySensor mySensor(&ADC_PIN, &SENSOR_PIN, &ADC_ITER);
#ifndef WIFI_OUTPUT
OLED OLED_Display(&OLED_PIN);
#endif
PumpControl pumpControl(&PUMP_PIN_MOSFET, &motorON, &pumpOn, &pumpPWMValue, &lowPercentThreshold, &highPercentThreshold);

#ifdef WIFI_OUTPUT
void initWifi()
{
  Serial.print("Connecting to: ");
  Serial.print(ssid);

  WiFi.begin(ssid, password);

  int timeout = 10 * 4; // 10 seconds
  while (WiFi.status() != WL_CONNECTED && (timeout-- > 0))
  {
    delay(250);
    printf(".");
  }
  // Serial.println("");

  printf("  ");

  if (WiFi.status() != WL_CONNECTED)
  {
    printf(("Failed to connect\n"));
  }

  printf("WiFi connected with IP address: Local IP: %s\n", WiFi.localIP());
}

void sendToGoogleSheet()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    HTTPClient http;
    http.begin(serverName);
    http.addHeader("Content-Type", "application/json");

    String jsonData = "{\"volt\":\"" + String(voltsSensorVal, 3) + "\", \"moist\":\"" + String(percent) + "\", \"count\":\"" + String(counter) + "\", \"pumpON\":\"" + String(pumpOn) + "\"}";

    printf("voltsSensorVal: %.3f percent:  %d counter: %d pumpOn %d \n", voltsSensorVal, percent, counter, pumpOn);

    int httpResponseCode = http.POST(jsonData);

    if (httpResponseCode > 0)
    {
      String response = http.getString();

      printf("Request POST Sent \n");
    }
    else
    {
      printf("Wrong request POST->  %d", httpResponseCode);
    }

    http.end();
  }

  delay(1000);
}

#endif

void settingDeepSleep()
{
  // esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * 60 * uS_TO_S_FACTOR); // minutes
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR); // seconds
  //  Enable External Wake Up (ext0: single pin)
  //  1 = High, 0 = Low
  //  esp_err_t esp_sleep_enable_ext0_wakeup(gpio_num_t gpio_num, int level);
  esp_sleep_enable_ext0_wakeup((gpio_num_t)WAKEUP_PIN, 1);
}

void checkWakeupReason()
{
  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause();
  switch (wakeup_reason)
  {
  case ESP_SLEEP_WAKEUP_TIMER:
    printf("--> Wakeup caused by timer\n");
    OLEDPin_state_changed = false;
    break;
  case ESP_SLEEP_WAKEUP_EXT0:
    printf("--> Wakeup caused by external signal using RTC_IO\n");
    displayON = !displayON;
    OLEDPin_state_changed = true;
    break;

  default:
    printf("--> Wakeup was not caused by deep sleep: %d\n", wakeup_reason);
    OLEDPin_state_changed = true;
    break;
  }
}

void ReadSensor_SetPump()
{
  printf("--> Setting Sensor HIGH <-- \n");
  mySensor.setControlPin(HIGH);
  mySensor.readPinVoltage(&voltsSensorVal, true);
  printf("--> Setting Sensor LOW <-- \n");
  mySensor.setControlPin(LOW);
  float percent1 = Utils::map_Float(&voltsSensorVal, &wetValue, &dryValue, 100.0, 0.0);
  percent = (uint8_t)percent1;
  printf("percent1: %.3f percent:  %d voltsSensorVal: %.3f\n", percent1, percent, voltsSensorVal);
  pumpControl.controlPump(&percent);

#ifdef PUMPCONTROL_OFF
  pumpOn = false;
  printf("********* Pump turned off for testing ****************\n");
#endif
}

void setup()
{
  printf(">>> Setup-> Counter: %d\n", counter);

#ifdef WIFI_OUTPUT
  initWifi();
#endif

  pinMode(WAKEUP_PIN, INPUT_PULLDOWN);
  pinMode(SENSOR_PIN, OUTPUT);
  digitalWrite(SENSOR_PIN, LOW);
  analogSetAttenuation(ADC_11db); // full 0–3.3 V range

#ifndef WIFI_OUTPUT
  WiFi.mode(WIFI_OFF);
#endif

  checkWakeupReason();

  if (!OLEDPin_state_changed)
  {
    int state = rtc_gpio_get_level((gpio_num_t)OLED_PIN);

#ifndef WIFI_OUTPUT
    if (state == HIGH)
      OLED_Display.initialiseOLED();
#endif

#ifndef WIFI_OUTPUT
    if (state == HIGH)
    {
      OLED_Display.display(&voltsSensorVal, &counter, &percent, &pumpOn);
      // counter++;
      delay(1000);
    }
#endif
  }

#ifndef WIFI_OUTPUT

  if (OLEDPin_state_changed)
  {
    ReadSensor_SetPump();

    int state = OLED_Display.setOLED_PIN_GPIO(HIGH);
    // even if state HIGH, we do not want to diaply anything if displayON = False
    if (state == HIGH && displayON)
    {
      OLED_Display.initialiseOLED();
      OLED_Display.setStartDisplay();
      OLED_Display.display(&voltsSensorVal, &counter, &percent, &pumpOn);
    }
    state = OLED_Display.set_RTC_OLED(displayON);
  }
#else
  printf("line: 240 \n");
  ReadSensor_SetPump();
  sendToGoogleSheet();
#endif

  if (!pumpOn && !OLEDPin_state_changed)
  {
    counter++;
#ifndef WIFI_OUTPUT
    int state = OLED_Display.set_RTC_OLED(displayON);
#endif
    printf("Going to sleep SETUP \n\n");
    settingDeepSleep();
    esp_deep_sleep_start();
  }
}

void loop()
{
  printf(">>> Running Loop \n");
  ReadSensor_SetPump();
  int state = rtc_gpio_get_level((gpio_num_t)OLED_PIN);
  if (state == HIGH && !OLEDPin_state_changed)
  {
#ifndef WIFI_OUTPUT
    OLED_Display.display(&voltsSensorVal, &counter, &percent, &pumpOn);
#else
    printf("line: 266 \n");
    sendToGoogleSheet(); // not sure we shoud do that whne pump is one
#endif
  }
  if (!pumpOn)
  {
    printf("Going to sleep LOOP \n\n");
    settingDeepSleep();
    esp_deep_sleep_start();
  }
}

/*
function doPost(e) {
var sheet =
SpreadsheetApp.getActiveSpreadsheet().getActiveSheet();
var data = JSON.parse(e.postData.contents);
sheet.appendRow([new Date(), data.volt, data.moist, data.count, data.pumpON]);
}

How to connect your ESP32 to EXCEL on Google Drive and monitor SENSORS in real time
https://www.youtube.com/watch?v=ciBs0VemqqQ

https://github.com/Becircuit/excel_esp32


*/

/*
Deployment successfully updated.
Version 1 on 9 Oct 2026, 13:16
Deployment ID
AKfycbz5co3hnWUnCaz06d4znbZ7AuWzXQa9EL0iNopfrWJyXyOyNjGQtMqCphzPT5yyssGH
Web app
URL
https://script.google.com/macros/s/AKfycbz5co3hnWUnCaz06d4znbZ7AuWzXQa9EL0iNopfrWJyXyOyNjGQtMqCphzPT5yyssGH/exec

*/

/*
https://docs.google.com/spreadsheets/d/1GgDyGy9jkBOppHcSErpTlavwA9kVKEKuaYguQy_0bEw/edit?gid=0#gid=0
*/

/*
https://docs.google.com/spreadsheets/d/1GgDyGy9jkBOppHcSErpTlavwA9kVKEKuaYguQy_0bEw/edit?gid=0#gid=0
*/
