//
// Non-Degree Granting Education License -- for use at non-degree
// granting, nonprofit, educational organizations only. Not for
// government, commercial, or other organizational use.
//
// File: minjerk.cpp
//
// Code generated for Simulink model 'minjerk'.
//
// Model version                  : 2.13
// Simulink Coder version         : 9.4 (R2020b) 29-Jul-2020
// C/C++ source code generated on : Mon Nov 16 12:11:17 2020
//
// Target selection: ert.tlc
// Embedded hardware selection: ARM Compatible->ARM Cortex-M
// Code generation objectives:
//    1. Execution efficiency
//    2. RAM efficiency
// Validation result: Not run
//
#include "minjerk.h"

// Model step function
void minjerkModelClass::step()
{
  real_T DiscreteTimeIntegrator1;
  real_T rtb_Gain;
  real_T rtb_Product;
  real_T rtb_Product1;
  real_T rtb_Product2;
  real_T rtb_Saturation;
  boolean_T rtb_FixPtRelationalOperator;

  // Outputs for Atomic SubSystem: '<Root>/minjerk'
  // Sum: '<S1>/Subtract1' incorporates:
  //   Inport: '<Root>/pfin'
  //   Memory: '<S1>/Memory1'

  rtb_Gain = rtU.pfin - rtY.pref;

  // RelationalOperator: '<S3>/FixPt Relational Operator' incorporates:
  //   Inport: '<Root>/pfin'
  //   UnitDelay: '<S3>/Delay Input1'
  //
  //  Block description for '<S3>/Delay Input1':
  //
  //   Store in Global RAM

  rtb_FixPtRelationalOperator = (rtU.pfin != rtDW.DelayInput1_DSTATE);

  // Outputs for Triggered SubSystem: '<S1>/Sample and Hold' incorporates:
  //   TriggerPort: '<S4>/Trigger'

  if (rtb_FixPtRelationalOperator && (rtPrevZCX.SampleandHold_Trig_ZCE !=
       POS_ZCSIG)) {
    // Inport: '<S4>/In' incorporates:
    //   Abs: '<S1>/Abs'
    //   Inport: '<Root>/vavg'
    //   Product: '<S1>/Divide3'

    rtDW.In = std::abs(rtb_Gain / rtU.vavg);
  }

  rtPrevZCX.SampleandHold_Trig_ZCE = rtb_FixPtRelationalOperator;

  // End of Outputs for SubSystem: '<S1>/Sample and Hold'

  // Product: '<S1>/Product' incorporates:
  //   Constant: '<S1>/Constant'
  //   Logic: '<S1>/NOT'
  //   Memory: '<S1>/Memory'
  //   Sum: '<S1>/Add2'

  rtb_Product = (rtDW.Memory_PreviousInput + 0.001) * static_cast<real_T>
    (!rtb_FixPtRelationalOperator);

  // Sum: '<S1>/Subtract'
  rtb_Saturation = rtDW.In - rtb_Product;

  // RelationalOperator: '<S2>/Compare' incorporates:
  //   Constant: '<S2>/Constant'

  rtb_FixPtRelationalOperator = (rtb_Saturation > 0.0);

  // Gain: '<S1>/Gain'
  rtb_Gain *= 60.0;

  // Saturate: '<S1>/Saturation'
  if (rtb_Saturation <= 0.001) {
    rtb_Saturation = 0.001;
  }

  // End of Saturate: '<S1>/Saturation'

  // Product: '<S1>/Divide2' incorporates:
  //   Gain: '<S1>/Gain1'
  //   Gain: '<S1>/Gain2'
  //   Memory: '<S1>/Memory2'
  //   Memory: '<S1>/Memory3'
  //   Product: '<S1>/Divide'
  //   Product: '<S1>/Divide1'
  //   Product: '<S1>/Product3'
  //   Sum: '<S1>/Add'
  //   Sum: '<S1>/Add1'

  rtb_Gain = ((rtb_Gain * static_cast<real_T>(rtb_FixPtRelationalOperator) /
               rtb_Saturation + -36.0 * rtDW.Memory2_PreviousInput) /
              rtb_Saturation + -9.0 * rtDW.Memory3_PreviousInput) /
    rtb_Saturation;

  // DiscreteIntegrator: '<S1>/Discrete-Time Integrator2'
  rtb_Saturation = 0.0005 * rtb_Gain + rtDW.DiscreteTimeIntegrator2_DSTATE;

  // Product: '<S1>/Product1'
  rtb_Product1 = rtb_Saturation * static_cast<real_T>
    (rtb_FixPtRelationalOperator);

  // DiscreteIntegrator: '<S1>/Discrete-Time Integrator1'
  DiscreteTimeIntegrator1 = 0.0005 * rtb_Product1 +
    rtDW.DiscreteTimeIntegrator1_DSTATE;

  // Product: '<S1>/Product2'
  rtb_Product2 = DiscreteTimeIntegrator1 * static_cast<real_T>
    (rtb_FixPtRelationalOperator);

  // Outport: '<Root>/pref' incorporates:
  //   DiscreteIntegrator: '<S1>/Discrete-Time Integrator'

  rtY.pref = 0.0005 * rtb_Product2 + rtDW.DiscreteTimeIntegrator_DSTATE;

  // Update for UnitDelay: '<S3>/Delay Input1' incorporates:
  //   Inport: '<Root>/pfin'
  //
  //  Block description for '<S3>/Delay Input1':
  //
  //   Store in Global RAM

  rtDW.DelayInput1_DSTATE = rtU.pfin;

  // Update for Memory: '<S1>/Memory'
  rtDW.Memory_PreviousInput = rtb_Product;

  // Update for Memory: '<S1>/Memory2'
  rtDW.Memory2_PreviousInput = rtb_Product2;

  // Update for Memory: '<S1>/Memory3'
  rtDW.Memory3_PreviousInput = rtb_Product1;

  // Update for DiscreteIntegrator: '<S1>/Discrete-Time Integrator2'
  rtDW.DiscreteTimeIntegrator2_DSTATE = 0.0005 * rtb_Gain + rtb_Saturation;

  // Update for DiscreteIntegrator: '<S1>/Discrete-Time Integrator1'
  rtDW.DiscreteTimeIntegrator1_DSTATE = 0.0005 * rtb_Product1 +
    DiscreteTimeIntegrator1;

  // Update for DiscreteIntegrator: '<S1>/Discrete-Time Integrator' incorporates:
  //   Outport: '<Root>/pref'

  rtDW.DiscreteTimeIntegrator_DSTATE = 0.0005 * rtb_Product2 + rtY.pref;

  // End of Outputs for SubSystem: '<Root>/minjerk'

  // Outport: '<Root>/vref'
  rtY.vref = rtb_Product2;

  // Outport: '<Root>/aref'
  rtY.aref = rtb_Product1;

  // Outputs for Atomic SubSystem: '<Root>/minjerk'
  // Outport: '<Root>/EOT' incorporates:
  //   Logic: '<S1>/NOT1'

  rtY.EOT = !rtb_FixPtRelationalOperator;

  // End of Outputs for SubSystem: '<Root>/minjerk'
}

// Model initialize function
void minjerkModelClass::initialize()
{
  rtPrevZCX.SampleandHold_Trig_ZCE = POS_ZCSIG;
}

// Constructor
minjerkModelClass::minjerkModelClass() :
  rtU(),
  rtY(),
  rtDW(),
  rtPrevZCX()
{
  // Currently there is no constructor body generated.
}

// Destructor
minjerkModelClass::~minjerkModelClass()
{
  // Currently there is no destructor body generated.
}

//
// File trailer for generated code.
//
// [EOF]
//
