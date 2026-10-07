#include <Arduino.h>

class Utils
{

public:
    static float map_Float(float *x, const float *in_min, const float *in_max, const float out_min, const float out_max);
};
