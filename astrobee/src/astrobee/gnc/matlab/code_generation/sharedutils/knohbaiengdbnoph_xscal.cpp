//
// File: knohbaiengdbnoph_xscal.cpp
//
// Code generated for Simulink model 'est_estimator'.
//
// Model version                  : 1.1142
// Simulink Coder version         : 8.11 (R2016b) 25-Aug-2016
// C/C++ source code generated on : Tue Oct 16 10:06:07 2018
//
#include "rtwtypes.h"
#include "knohbaiengdbnoph_xscal.h"

// Function for MATLAB Function: '<S17>/Compute Residual and H'
void knohbaiengdbnoph_xscal(int32_T n, real32_T a, real32_T x_data[], int32_T
  ix0)
{
  int32_T b;
  int32_T k;
  b = (int32_T)((int32_T)(ix0 + n) - 1);
  for (k = ix0; k <= b; k++) {
    x_data[(int32_T)(k - 1)] *= a;
  }
}

//
// File trailer for generated code.
//
// [EOF]
//
