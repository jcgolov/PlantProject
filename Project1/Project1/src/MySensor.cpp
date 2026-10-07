#include "MySensor.h"

MySensor::MySensor()
{
}

MySensor::MySensor(const uint8_t *adcpin, const uint16_t *iterNb)
{
    ADC_Pin = adcpin;
    ADC_Iter = iterNb;
    pinMode(*ADC_Pin, INPUT);
}

MySensor::MySensor(const uint8_t *adcpin)
{
    ADC_Pin = adcpin;
}

MySensor::MySensor(const uint8_t *adcpin, const uint8_t *adcvccpin, const uint16_t *iterNb)
{
    ADC_Pin = adcpin;
    ADC_VCC_Pin = adcvccpin;
    ADC_Iter = iterNb;
    pinMode(*ADC_Pin, INPUT);
    controlPinSet = true;
}

MySensor::MySensor(const uint8_t *adcpin, const uint8_t *adcvccpin)
{
    ADC_Pin = adcpin;
    ADC_VCC_Pin = adcvccpin;
    pinMode(*ADC_Pin, INPUT);

    controlPinSet = true;
}

void MySensor::setADC(uint8_t *adcpin, uint16_t *iterNb)
{
    ADC_Pin = adcpin;
    ADC_Iter = iterNb;
}

MySensor::~MySensor()
{
}

void MySensor::setControlPin(int state)
{
    if (controlPinSet)
    {
        digitalWrite(*ADC_VCC_Pin, state);
        delay(100);
    }
    else
    {
        printf("Warning: Cant set Pin state as it has not been set\n");
    }
}

// Function to find mode of a float array
void MySensor::getModeForFloats(float arr[], const uint16_t *size, float *mode, double tolerance = 0.01f)
{
    std::sort(arr, arr + *size);
    *mode = arr[0];
    maxCount = 0;
    currCount = 0;
    for (int i = 1; i < *size; i++)
    {
        if (fabsf(arr[i] - arr[i - 1]) < 0.001f)
        { // tolerance
            currCount++;
        }
        else
        {
            if (currCount > maxCount)
            {
                maxCount = currCount;
                *mode = arr[i - 1];
            }
            currCount = 1;
        }
    }
    if (currCount > maxCount)
    {
        maxCount = currCount;
        *mode = arr[*size - 1];
    }
}

void MySensor::readPinVoltage(float *voltsSensorVal, bool sampled = false)
{
    if (sampled)
        readSampledPinVoltage(voltsSensorVal);
    else
        readSinglePinVoltage(voltsSensorVal);
}

void MySensor::readSinglePinVoltage(float *voltsSensorVal)
{
    uint32_t currValRead = analogReadMilliVolts(*ADC_Pin);
    *voltsSensorVal = (currValRead / 1000.0) - 0.042;
}

void MySensor::readSampledPinVoltage(float *voltsSensorVal)
{
    float *_arrayVolts = new float[*ADC_Iter]();
    for (int idx = 0; idx < *ADC_Iter; idx++)
    {
        uint32_t currValRead = analogReadMilliVolts(*ADC_Pin);
        *voltsSensorVal = (currValRead / 1000.0) - 0.042;
        _arrayVolts[idx] = *voltsSensorVal;
    }
    getModeForFloats(_arrayVolts, ADC_Iter, voltsSensorVal);
    delete[] _arrayVolts;
}
