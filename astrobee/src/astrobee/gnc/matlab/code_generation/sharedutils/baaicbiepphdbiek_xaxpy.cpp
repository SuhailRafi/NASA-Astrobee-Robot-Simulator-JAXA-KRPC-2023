//
// File: baaicbiepphdbiek_xaxpy.cpp
//
// Code generated for Simulink model 'est_estimator'.
//
// Model version                  : 1.1142
// Simulink Coder version         : 8.11 (R2016b) 25-Aug-2016
// C/C++ source code generated on : Tue Oct 16 10:06:07 2018
//
#include "rtwtypes.h"
#include "baaicbiepphdbiek_xaxpy.h"

// Function for MATLAB Function: '<S24>/compute_of_global_points'
void baaicbiepphdbiek_xaxpy(int32_T n, real32_T a, const real32_T x_data[],
  int32_T ix0, real32_T y_data[], int32_T iy0)
{
  int32_T ix;
  int32_T iy;
  int32_T k;
  if (!((n < 1) || (a == 0.0F))) {
    ix = (int32_T)(ix0 - 1);
    iy = (int32_T)(iy0 - 1);
    for (k = 0; k <= (int32_T)(n - 1); k++) {
      y_data[iy] += a * x_data[ix];
      ix++;
      iy++;
    }
  }
}

//
// File trailer for generated code.
//
// [EOF]
//
