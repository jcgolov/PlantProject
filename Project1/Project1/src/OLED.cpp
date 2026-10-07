#include "OLED.h"

Adafruit_SSD1306 myDisplay(OLED_RESET); // OLED display

OLED::OLED()
{
}

OLED::OLED(const uint8_t *oled_pin)
{
    OLED_PIN = oled_pin;
    controlPinSet = true;
}

int OLED::setOLED_PIN_GPIO(int state)
{
    // just added this
    // we need to turn off RTC setting before we can switch to GPIO settings
    // just in case it was controlled for sleep cycle
    gpio_deep_sleep_hold_dis();
    gpio_hold_dis((gpio_num_t)*OLED_PIN);

    pinMode(*OLED_PIN, OUTPUT);
    digitalWrite(*OLED_PIN, state);
    delay(100);

    return digitalRead(*OLED_PIN);
}

int OLED::get_GPOI_OLED_State()
{
    return digitalRead((gpio_num_t)*OLED_PIN);
}

int OLED::set_RTC_OLED(int state)
{

    gpio_set_direction((gpio_num_t)*OLED_PIN, GPIO_MODE_OUTPUT);

    gpio_deep_sleep_hold_dis();
    gpio_hold_dis((gpio_num_t)*OLED_PIN);

    // 1. Set pin as output and desired state
    pinMode(*OLED_PIN, OUTPUT);
    digitalWrite(*OLED_PIN, state ? HIGH : LOW); // or LOW

    // 2. Enable deep sleep hold (Critical for retention across reset)
    gpio_deep_sleep_hold_en();

    // 3. Lock the specific pin state
    gpio_hold_en((gpio_num_t)*OLED_PIN);

    return rtc_gpio_get_level((gpio_num_t)*OLED_PIN);
}

void OLED::initialiseOLED()
{
    // Initialize OLED I2C
    Wire.begin();
    // Initialize display
    myDisplay.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
}

void OLED::setStartDisplay()
{
    myDisplay.dim(false);
    // if (counter == 0)
    myDisplay.clearDisplay(); // Nuke the logo before it shows
    myDisplay.display();
}

void OLED::display(float *voltsSensorVal, int *counter, uint8_t *moisturePercent, bool *pumpON)
{
    printf(">>> Updating OLED \n");
    myDisplay.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
    myDisplay.clearDisplay(); // Clear display buffer
    myDisplay.setTextSize(1);
    myDisplay.setCursor(0, 0);
    std::ostringstream oss;
    oss << "Pin:   " << std::fixed << std::setprecision(2) << *voltsSensorVal << " V";
    if (*pumpON)
        oss << "  P: ON";
    else
        oss << "  P: OFF";
    myDisplay.println(oss.str().c_str());
    oss.str("");
    oss.clear();
    oss << "Moist: " << int(*moisturePercent) << " %" << "  C: " << *counter;
    myDisplay.setCursor(0, 10);
    myDisplay.println(oss.str().c_str());
    myDisplay.display();
}