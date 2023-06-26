//
// File: djeclngddbimoppp_cat.cpp
//
// Code generated for Simulink model 'sim_model_lib0'.
//
// Model version                  : 1.1051
// Simulink Coder version         : 8.9 (R2015b) 13-Aug-2015
// C/C++ source code generated on : Tue Nov  8 14:36:07 2016
//
#include "rtwtypes.h"
#include "djeclngddbimoppp_cat.h"

// Function for MATLAB Function: '<S49>/generate_output'
void djeclngddbimoppp_cat(const int32_T varargin_1_sizes, const int32_T
  varargin_2_sizes[2], uint8_T y_data[], int32_T y_sizes[2])
{
  int32_T iy;
  int32_T j;
  int32_T b_j;
  y_sizes[0] = (int16_T)varargin_1_sizes;
  y_sizes[1] = 16;
  iy = -1;
  for (j = 1; j <= varargin_1_sizes; j++) {
    iy++;
    y_data[iy] = 1U;
  }

  j = varargin_2_sizes[0] * 15;
  for (b_j = 1; b_j <= j; b_j++) {
    iy++;
    y_data[iy] = 0U;
  }
}

//
// File trailer for generated code.
//
// [EOF]
//
