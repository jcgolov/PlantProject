#include <HTTPClient.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "WiFi.h"
#include <Arduino.h>
#include "esp_sleep.h" //for sleep

#include "Utils.h"
#include "MySensor.h"
#include "OLED.h"
#include "PumpControl.h"
#include "Wifi_Google.h"

/*
  This overrides the pump On and OFF to set it to OFF
  all the time. This is for testing the code
*/
// #define PUMPCONTROL_OFF

/*
  This stops the debug printf statement when not defiend
*/
// #define DEBUG_OUTPUT
/*

When the wifi is on the display must be off as
there is not enough space for the entire code
for both display and wifi
*/
#define WIFI_OUTPUT

const uint8_t WAKEUP_PIN = 33;      // pin that whne going high stops the sleep of the chip as external wakeup
const uint8_t SENSOR_PIN = 4;       // pin that controls the VCC of the sensor
const uint8_t PUMP_PIN_MOSFET = 25; // GPIO driving the XY-MOS TRIG/PWM pin. this control the pump motor

#define uS_TO_S_FACTOR 1000000ULL // Conversion factor for microseconds to seconds
#define TIME_TO_SLEEP 30           // Time ESP32 will go to sleep

// #define MINUTES

/*
  all variables declared as RTC_DATA_ATTR are restored after waking up.
*/
RTC_DATA_ATTR uint8_t percent = 0;
RTC_DATA_ATTR bool motorON = false;     // control the pump motor
RTC_DATA_ATTR int counter = 0;          // counter on how many tiems the chip went to sleep
RTC_DATA_ATTR bool pumpOn = false;      // togle for ON OFF for pump
RTC_DATA_ATTR float voltsSensorVal = 0; // value use whne redign sensor voltage
RTC_DATA_ATTR float last_voltsSensorVal = 0;

const uint8_t ADC_PIN = 34;   // GPIO34 that control the VCC power ON and OFF (MySensor)
const uint16_t ADC_ITER = 10; // number of times the pin reading is sampled (MySensor)

// pump control values passed as pointers to PumpControl
const uint8_t lowPercentThreshold = 40;  // if the percent lower than that, pump is turned ON
const uint8_t highPercentThreshold = 70; // When motor pump is ON, it will stop whne percent >= this value
const uint8_t pumpPWMValue = 255;        // this sets the speed of the pump motor


// // used to map the values for percents
const float dryValue = 2.58; // volts -> dry whne out of the pot
// const float dryValue = 1.695; //volts -> dry pot earth touch

const float wetValue = 0.94; // volts

#ifdef WIFI_OUTPUT
const char *ssid = "BT-9TAJSN";
const char *password = "NEtLgxRuhDGh6V";
const char *serverName = "https://script.google.com/macros/s/AKfycbz5co3hnWUnCaz06d4znbZ7AuWzXQa9EL0iNopfrWJyXyOyNjGQtMqCphzPT5yyssGH/exec";
unsigned long timeRead = 0;
Wifi_Google wifi_Google(ssid, password, serverName);
#endif

MySensor mySensor(&ADC_PIN, &SENSOR_PIN, &ADC_ITER);
#ifndef WIFI_OUTPUT
const uint8_t OLED_PIN = 2;                       // pin that controls the VCC of the OLED module
RTC_DATA_ATTR bool displayON = true;              // control the display ON and OFF
RTC_DATA_ATTR bool OLEDPin_state_changed = false; // if the switch is pressed to turn display ON and OFF, this is set to True
OLED OLED_Display(&OLED_PIN);
#endif

PumpControl pumpControl(&PUMP_PIN_MOSFET, &motorON, &pumpOn, &pumpPWMValue, &lowPercentThreshold, &highPercentThreshold);

void settingDeepSleep()
{
#ifdef MINUTES
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * 60 * uS_TO_S_FACTOR); // minutes
#else
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR); // seconds
#endif
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
#ifdef DEBUG_OUTPUT
    printf("--> Wakeup caused by timer\n");
#endif

#ifndef WIFI_OUTPUT
    OLEDPin_state_changed = false;
#endif
  case ESP_SLEEP_WAKEUP_EXT0:
#ifdef DEBUG_OUTPUT
    printf("--> Wakeup caused by external signal using RTC_IO\n");
#endif
#ifndef WIFI_OUTPUT
    displayON = !displayON;
    OLEDPin_state_changed = true;
#endif
    break;

  default:
#ifdef DEBUG_OUTPUT
    printf("--> Wakeup was not caused by deep sleep: %d\n", wakeup_reason);
#endif
#ifndef WIFI_OUTPUT
    OLEDPin_state_changed = true;
#endif
    break;
  }
}

void ReadSensor_SetPump()
{

  /*
  I haver dried the earth in pot in over
  pretty dry to teh touch and it is very dried
  with current voltage computation i get 2.162 volts
  this set the percent to 24%
  */

#ifdef DEBUG_OUTPUT
  printf("--> Setting Sensor HIGH <-- \n");
#endif
  mySensor.setControlPin(HIGH);
  mySensor.readPinVoltage(&voltsSensorVal, true);
#ifdef DEBUG_OUTPUT
  printf("--> Setting Sensor LOW <-- \n");
#endif
  mySensor.setControlPin(LOW);

  /*
    as the voltsSensorVal fluctuates so does the moisture percent
    here we ensure that teh difference between the last reading and the new one
    is significant enough to recompute everything
    This alos ensure that we do not waste process time etc.
    byt only doing things whne really needed
  */

  /*
  we dont want that if the pump is running
  the step between whole percent appears to be 0.016 volt (58 raw values)
  Whne heater is on this value shift greatly so instead of > 0.016 we use twice that amount
  for more stability i.e. 0.033
  */
  if (abs(voltsSensorVal - last_voltsSensorVal) > 0.033 && !motorON)
  {
    float percent1 = Utils::map_Float(&voltsSensorVal, &wetValue, &dryValue, 100.0, 0.0);
    pumpControl.controlPump(&percent1, &percent);
    last_voltsSensorVal = voltsSensorVal;
    counter = 1;
  }
  else if (pumpOn)
  {
    float percent1 = Utils::map_Float(&voltsSensorVal, &wetValue, &dryValue, 100.0, 0.0);
    pumpControl.controlPump(&percent1, &percent);
    counter = 2;
  }
  else
  {
    counter = 0;
  }
#ifdef DEBUG_OUTPUT
  printf("percent:  %d voltsSensorVal: %.3f last_voltsSensorVal: %.3f\n", percent, voltsSensorVal, last_voltsSensorVal);
#endif

#ifdef PUMPCONTROL_OFF
  pumpOn = false;
  printf("********* Pump turned off for testing ****************\n");
#endif
}

void setup()
{
#ifdef DEBUG_OUTPUT
  printf(">>> Setup-> Counter: %d\n", counter);
#endif

  /*
  Under load around 80ma whne wifi kicks in 240
  65ms / 69ma at 80mhz.
  40 wifi does not work
  Does not make much diffrence whne in deep sleep though
  */
  setCpuFrequencyMhz(80); // Set to 80 MHz

  pinMode(WAKEUP_PIN, INPUT_PULLDOWN);
  pinMode(SENSOR_PIN, OUTPUT);
  digitalWrite(SENSOR_PIN, LOW);
  analogSetAttenuation(ADC_11db); // full 0–3.3 V range

  checkWakeupReason();

  ReadSensor_SetPump();

#ifdef WIFI_OUTPUT
  wifi_Google.setVariablesPrt(&voltsSensorVal, &percent, &counter, &pumpOn);
  wifi_Google.initWifi();
  wifi_Google.sendToGoogleSheet();
#else
  WiFi.mode(WIFI_OFF);
  if (OLEDPin_state_changed)
  {
    int state = OLED_Display.setOLED_PIN_GPIO(HIGH);
    if (state == HIGH && displayON)
    {
      OLED_Display.initialiseOLED();
      OLED_Display.setStartDisplay();
      OLED_Display.display(&voltsSensorVal, &counter, &percent, &pumpOn);
    }
  }
  else
  {
    int state = OLED_Display.setOLED_PIN_GPIO(HIGH);
    if (state == HIGH)
    {
      OLED_Display.initialiseOLED();
      OLED_Display.display(&voltsSensorVal, &counter, &percent, &pumpOn);
    }
  }

  int state = OLED_Display.set_RTC_OLED(displayON);
#endif

  if (!pumpOn)
  {
    // counter++;
    // #ifdef DEBUG_OUTPUT
    printf("Going to sleep SETUP \n\n");
    // #endif
    settingDeepSleep();
    esp_deep_sleep_start();
  }
}

void loop()
{
  printf(">>> Running Loop \n");
  ReadSensor_SetPump();

#ifndef WIFI_OUTPUT
  int state = rtc_gpio_get_level((gpio_num_t)OLED_PIN);
  if (state == HIGH && displayON)
    OLED_Display.display(&voltsSensorVal, &counter, &percent, &pumpOn);
#endif

  if (!pumpOn)
  {
#ifdef DEBUG_OUTPUT
    printf("Going to sleep LOOP \n\n");
#endif
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
