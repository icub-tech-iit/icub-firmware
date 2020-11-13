//
// Non-Degree Granting Education License -- for use at non-degree
// granting, nonprofit, educational organizations only. Not for
// government, commercial, or other organizational use.
//
// File: minjerk.h
//
// Code generated for Simulink model 'minjerk'.
//
// Model version                  : 2.12
// Simulink Coder version         : 9.4 (R2020b) 29-Jul-2020
// C/C++ source code generated on : Thu Nov 12 15:29:44 2020
//
// Target selection: ert.tlc
// Embedded hardware selection: Intel->x86-64 (Windows64)
// Code generation objectives:
//    1. Execution efficiency
//    2. RAM efficiency
// Validation result: Not run
//
#ifndef RTW_HEADER_minjerk_h_
#define RTW_HEADER_minjerk_h_
#include <cmath>
#include "rtwtypes.h"
#include "zero_crossing_types.h"
#include "rtw_continuous.h"
#include "rtw_solver.h"

// Model Code Variants

// Macros for accessing real-time model data structure
#ifndef rtmGetErrorStatus
#define rtmGetErrorStatus(rtm)         ((rtm)->errorStatus)
#endif

#ifndef rtmSetErrorStatus
#define rtmSetErrorStatus(rtm, val)    ((rtm)->errorStatus = (val))
#endif

// Class declaration for model minjerk
class minjerkModelClass {
  // public data and function members
 public:
  // Block signals and states (default storage) for system '<Root>'
  typedef struct {
    real_T In;                         // '<S4>/In'
    real_T DelayInput1_DSTATE;         // '<S3>/Delay Input1'
    real_T DiscreteTimeIntegrator2_DSTATE;// '<S1>/Discrete-Time Integrator2'
    real_T DiscreteTimeIntegrator1_DSTATE;// '<S1>/Discrete-Time Integrator1'
    real_T DiscreteTimeIntegrator_DSTATE;// '<S1>/Discrete-Time Integrator'
    real_T Memory_PreviousInput;       // '<S1>/Memory'
    real_T Memory2_PreviousInput;      // '<S1>/Memory2'
    real_T Memory3_PreviousInput;      // '<S1>/Memory3'
  } DW;

  // Zero-crossing (trigger) state
  typedef struct {
    ZCSigState SampleandHold_Trig_ZCE; // '<S1>/Sample and Hold'
  } PrevZCX;

  // External inputs (root inport signals with default storage)
  typedef struct {
    real_T pfin;                       // '<Root>/pfin'
    real_T vavg;                       // '<Root>/vavg'
  } ExtU;

  // External outputs (root outports fed by signals with default storage)
  typedef struct {
    real_T pref;                       // '<Root>/pref'
    real_T vref;                       // '<Root>/vref'
    real_T aref;                       // '<Root>/aref'
    boolean_T EOT;                     // '<Root>/EOT'
  } ExtY;

  // Real-time Model Data Structure
  struct RT_MODEL {
    const char_T * volatile errorStatus;
  };

  // External inputs
  ExtU rtU;

  // External outputs
  ExtY rtY;

  // model initialize function
  void initialize();

  // model step function
  void step();

  // Constructor
  minjerkModelClass();

  // Destructor
  ~minjerkModelClass();

  // Real-Time Model get method
  minjerkModelClass::RT_MODEL * getRTM();

  // private data and function members
 private:
  // Block signals and states
  DW rtDW;
  PrevZCX rtPrevZCX;                   // Triggered events

  // Real-Time Model
  RT_MODEL rtM;
};

//-
//  These blocks were eliminated from the model due to optimizations:
//
//  Block '<Root>/Scope' : Unused code path elimination
//  Block '<Root>/Scope1' : Unused code path elimination
//  Block '<Root>/Scope2' : Unused code path elimination
//  Block '<Root>/Scope3' : Unused code path elimination
//  Block '<S1>/Zero-Order Hold1' : Eliminated since input and output rates are identical
//  Block '<S1>/Zero-Order Hold2' : Eliminated since input and output rates are identical


//-
//  The generated code includes comments that allow you to trace directly
//  back to the appropriate location in the model.  The basic format
//  is <system>/block_name, where system is the system number (uniquely
//  assigned by Simulink) and block_name is the name of the block.
//
//  Use the MATLAB hilite_system command to trace the generated code back
//  to the model.  For example,
//
//  hilite_system('<S3>')    - opens system 3
//  hilite_system('<S3>/Kp') - opens and selects block Kp which resides in S3
//
//  Here is the system hierarchy for this model
//
//  '<Root>' : 'minjerk'
//  '<S1>'   : 'minjerk/minjerk'
//  '<S2>'   : 'minjerk/minjerk/Compare To Zero'
//  '<S3>'   : 'minjerk/minjerk/Detect Change'
//  '<S4>'   : 'minjerk/minjerk/Sample and Hold'

#endif                                 // RTW_HEADER_minjerk_h_

//
// File trailer for generated code.
//
// [EOF]
//
