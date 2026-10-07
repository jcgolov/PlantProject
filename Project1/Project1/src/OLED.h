#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include "WiFi.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include "driver/rtc_io.h"

#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C
#define SSD1306_NO_SPLASH

class OLED
{

public:
    OLED();

    OLED(const uint8_t *oled_pin);

    void display(float *voltsSensorVal, int *counter, uint8_t *moisturePercent, bool *pumpON);

    void initialiseOLED();
    void setStartDisplay();

    int setOLED_PIN_GPIO(int state);
    int get_GPOI_OLED_State();
    int set_RTC_OLED(int state);

private:
    /*
        We could be in a case where we want to switch off the display by using a mosfet module
        controlling the VCC, thus switching display power ON & OFF
        To do this we need a pin that will be the trigger of the mosfet
        OLED_PIN is this pin.
        If the chip is send to sleep, the pin must remain in the same state through out the sleep cycle
        and whne it wakes up. Otherwise it will go LOW.

    */
    const uint8_t OLED_PIN_Default = 2;
    const uint8_t *OLED_PIN = &OLED_PIN_Default;

    bool controlPinSet = false;
};
