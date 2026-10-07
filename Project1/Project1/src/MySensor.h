#include <Arduino.h>
#include <functional>
#include <iostream>
using namespace std;

#define ADC_BITS_NUM 4095.0 // 12-bit ADC

class MySensor
{

public:
    MySensor();
    MySensor(const uint8_t *adcpin, const uint16_t *iterNb);
    MySensor(const uint8_t *adcpin);
    MySensor(const uint8_t *adcpin, const uint8_t *adcvccpin, const uint16_t *iterNb);
    MySensor(const uint8_t *adcpin, const uint8_t *adcvccpin);

    ~MySensor();

    void setADC(uint8_t *adcpin, uint16_t *iterNb);
    void readSampledPinVoltage(float *voltsSensorVal);
    void readSinglePinVoltage(float *voltsSensorVal);
    void readPinVoltage(float *voltsSensorVal, bool sampled);

    void setControlPin(int state);

private:
    void getModeForFloats(float arr[], const uint16_t *size, float *mode, double tolerance);

    const uint8_t ADC_Pin_default = 34;
    const uint8_t ADC_VCC_Pin_default = 2;
    const uint16_t ADC_Iter_default = 10;

    const uint8_t *ADC_Pin = &ADC_Pin_default;    // = 34;  // GPIO34 (ADC1_CH6)
    const uint16_t *ADC_Iter = &ADC_Iter_default; // = 10; // number of times the pin reading is sampled
    const uint8_t *ADC_VCC_Pin = &ADC_VCC_Pin_default;
    int maxCount = 1;
    int currCount = 1;
    long currValRead = 0;
    bool controlPinSet = false;
};
