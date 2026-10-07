
#include "Utils.h"

/*
The 2 variables must in in the cpp file as the function usign them is static
*/

int maxCount = 1;
int currCount = 1;

// float  Utils::map_Float(float x, float in_min, float in_max, float out_min, float out_max)
// {
//   return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
// }

float Utils::map_Float(float *x, const float *in_min, const float *in_max, const float out_min, const float out_max)
{
  return (*x - *in_min) * (out_max - out_min) / (*in_max - *in_min) + out_min;
}
