#include <Arduino.h>

class PumpControl
{

    /*
        this class models a pump control that is controlled using PWM
        the function ledcAttach sets teh pin and frequency and resolution of the PWM
        in setLEDPin_PWM() function.
        In the circuit a Mosfet module is used to control the pump and the
        pump_pin_Mosfet is the ping the trigger for the mosfet module is conencted to.

        Most of the values are passed in the constructor.

        The control works like this:
        Check if the percent < low theshold. If it is turn the pump ON
        if the motor is already ON dont do nothing. However if it is not, trun it on
        if the motor is ON and if the he percent >= high theshold, then we can turn the motor OFF.
        if however the motor is ON but the percent < high theshold, then the pump is ON
        and if the motor is not yet On turn it ON
    */

public:
    PumpControl(const uint8_t *pump_pin_Mosfet, bool *motorON, bool *pumpOn, const uint8_t *pumpPWMValue, const uint8_t *lowPercentThreshold, const uint8_t *highPercentThreshold);

    void controlPump(uint8_t *percent);

private:
    const uint8_t *pump_pin_Mosfet;
    bool *motorON;
    bool *pumpOn;

    const uint32_t PWM_FREQ = 5000; // PWM frequency
    const uint8_t PWM_RES = 8;      // PWM resolution

    /*
        this sets the speed of the motor through the trigger of the Mosfet module.
        ex: max is 255 and 0 steps.
    */
    const uint8_t *pumpPWMValue;

    /*
        these two values are the low and high percent which controls how the pump
        is controlled.
    */
    const uint8_t *lowPercentThreshold;
    const uint8_t *highPercentThreshold;
};
