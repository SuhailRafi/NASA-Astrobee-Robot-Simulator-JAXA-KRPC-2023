//
// File: eknofcjenophiecj_xswap.cpp
//
// Code generated for Simulink model 'fam_force_allocation_module'.
//
// Model version                  : 1.1142
// Simulink Coder version         : 8.11 (R2016b) 25-Aug-2016
// C/C++ source code generated on : Tue Oct 16 10:07:06 2018
//
#include "rtwtypes.h"
#include "eknofcjenophiecj_xswap.h"

// Function for MATLAB Function: '<S12>/MATLAB Function'
void eknofcjenophiecj_xswap(real32_T x[36], int32_T ix0, int32_T iy0)
{
  int32_T ix;
  int32_T iy;
  real32_T temp;
  int32_T k;
  ix = (int32_T)(ix0 - 1);
  iy = (int32_T)(iy0 - 1);
  for (k = 0; k < 12; k++) {
    temp = x[ix];
    x[ix] = x[iy];
    x[iy] = temp;
    ix++;
    iy++;
  }
}

//
// File trailer for generated code.
//
// [EOF]
//
