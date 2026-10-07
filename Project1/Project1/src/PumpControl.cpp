#include "PumpControl.h"

PumpControl::PumpControl(const uint8_t *pump_pin_Mosfet, bool *motorON, bool *pumpOn, const uint8_t *pumpPWMValue, const uint8_t *lowPercentThreshold, const uint8_t *highPercentThreshold)
{

    this->pump_pin_Mosfet = pump_pin_Mosfet;
    this->motorON = motorON;
    this->pumpOn = pumpOn;
    this->pumpPWMValue = pumpPWMValue;
    this->lowPercentThreshold = lowPercentThreshold;
    this->highPercentThreshold = highPercentThreshold;

    ledcAttach(*pump_pin_Mosfet, PWM_FREQ, PWM_RES);
}

void PumpControl::controlPump(uint8_t *percent)
{
    if (*percent <= *lowPercentThreshold)
    {
        *pumpOn = true;
        if (!*motorON)
            ledcWrite(*pump_pin_Mosfet, *pumpPWMValue);
    }
    else
    {
        if (*pumpOn == true && *percent >= *highPercentThreshold)
        {
            *pumpOn = false;
            ledcWrite(*pump_pin_Mosfet, 0);
        }
        else if (*pumpOn == true && *percent < *highPercentThreshold)
        {
            *pumpOn = true;
            if (!*motorON)
                ledcWrite(*pump_pin_Mosfet, *pumpPWMValue);
        }
        else
        {
        }
    }
}