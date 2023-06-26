//
// File: nohlcjekmohddjmg_abs.cpp
//
// Code generated for Simulink model 'est_estimator'.
//
// Model version                  : 1.1142
// Simulink Coder version         : 8.11 (R2016b) 25-Aug-2016
// C/C++ source code generated on : Tue Oct 16 10:06:07 2018
//
#include "rtwtypes.h"
#include <math.h>
#include "nohlcjekmohddjmg_abs.h"

// Function for MATLAB Function: '<S16>/Compute Residual and H'
void nohlcjekmohddjmg_abs(const real32_T x_data[], const int32_T x_sizes,
  real32_T y_data[], int32_T *y_sizes)
{
  int32_T k;
  *y_sizes = (int32_T)(uint8_T)x_sizes;
  for (k = 0; (int32_T)(k + 1) <= x_sizes; k++) {
    y_data[k] = (real32_T)fabs((real_T)x_data[k]);
  }
}

//
// File trailer for generated code.
//
// [EOF]
//
