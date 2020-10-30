/*
 * Non-Degree Granting Education License -- for use at non-degree
 * granting, nonprofit, educational organizations only. Not for
 * government, commercial, or other organizational use.
 *
 * File: minjerk.c
 *
 * Code generated for Simulink model 'minjerk'.
 *
 * Model version                  : 2.5
 * Simulink Coder version         : 9.4 (R2020b) 29-Jul-2020
 * C/C++ source code generated on : Thu Oct  8 14:03:34 2020
 *
 * Target selection: ert.tlc
 * Embedded hardware selection: Intel->x86-64 (Windows64)
 * Code generation objectives:
 *    1. Execution efficiency
 *    2. RAM efficiency
 * Validation result: Not run
 */

#include "minjerk.h"

/* Private macros used by the generated code to access rtModel */
#ifndef rtmIsMajorTimeStep
#define rtmIsMajorTimeStep(rtm)        (((rtm)->Timing.simTimeStep) == MAJOR_TIME_STEP)
#endif

#ifndef rtmIsMinorTimeStep
#define rtmIsMinorTimeStep(rtm)        (((rtm)->Timing.simTimeStep) == MINOR_TIME_STEP)
#endif

#ifndef rtmSetTPtr
#define rtmSetTPtr(rtm, val)           ((rtm)->Timing.t = (val))
#endif

/* Block signals and states (default storage) */
DW rtDW;

/* Previous zero-crossings (trigger) states */
PrevZCX rtPrevZCX;

/* External inputs (root inport signals with default storage) */
ExtU rtU;

/* External outputs (root outports fed by signals with default storage) */
ExtY rtY;

/* Real-time model */
static RT_MODEL rtM_;
RT_MODEL *const rtM = &rtM_;

/* Model step function */
void minjerk_step(void)
{
  real_T DiscreteTimeIntegrator1;
  real_T rtb_Gain;
  real_T rtb_Product;
  real_T rtb_Product1;
  real_T rtb_Product2;
  real_T rtb_Saturation;
  boolean_T rtb_FixPtRelationalOperator;

  /* Outputs for Atomic SubSystem: '<Root>/minjerk' */
  /* Sum: '<S1>/Subtract1' incorporates:
   *  Inport: '<Root>/pfin'
   *  Memory: '<S1>/Memory1'
   */
  rtb_Gain = rtU.pfin - rtY.pref;

  /* RelationalOperator: '<S3>/FixPt Relational Operator' incorporates:
   *  Inport: '<Root>/pfin'
   *  UnitDelay: '<S3>/Delay Input1'
   *
   * Block description for '<S3>/Delay Input1':
   *
   *  Store in Global RAM
   */
  rtb_FixPtRelationalOperator = (rtU.pfin != rtDW.DelayInput1_DSTATE);

  /* Outputs for Triggered SubSystem: '<S1>/Sample and Hold' incorporates:
   *  TriggerPort: '<S4>/Trigger'
   */
  if (rtb_FixPtRelationalOperator && (rtPrevZCX.SampleandHold_Trig_ZCE !=
       POS_ZCSIG)) {
    /* Inport: '<S4>/In' incorporates:
     *  Abs: '<S1>/Abs'
     *  Inport: '<Root>/vavg'
     *  Product: '<S1>/Divide3'
     */
    rtDW.In = fabs(rtb_Gain / rtU.vavg);
  }

  rtPrevZCX.SampleandHold_Trig_ZCE = rtb_FixPtRelationalOperator;

  /* End of Outputs for SubSystem: '<S1>/Sample and Hold' */

  /* Product: '<S1>/Product' incorporates:
   *  Constant: '<S1>/Constant'
   *  Logic: '<S1>/NOT'
   *  Memory: '<S1>/Memory'
   *  Sum: '<S1>/Add2'
   */
  rtb_Product = (rtDW.Memory_PreviousInput + 0.001) * (real_T)
    !rtb_FixPtRelationalOperator;

  /* Sum: '<S1>/Subtract' */
  rtb_Saturation = rtDW.In - rtb_Product;

  /* RelationalOperator: '<S2>/Compare' incorporates:
   *  Constant: '<S2>/Constant'
   */
  rtb_FixPtRelationalOperator = (rtb_Saturation > 0.0);

  /* Gain: '<S1>/Gain' */
  rtb_Gain *= 60.0;

  /* Saturate: '<S1>/Saturation' */
  if (rtb_Saturation <= 0.001) {
    rtb_Saturation = 0.001;
  }

  /* End of Saturate: '<S1>/Saturation' */

  /* Product: '<S1>/Divide2' incorporates:
   *  Gain: '<S1>/Gain1'
   *  Gain: '<S1>/Gain2'
   *  Memory: '<S1>/Memory2'
   *  Memory: '<S1>/Memory3'
   *  Product: '<S1>/Divide'
   *  Product: '<S1>/Divide1'
   *  Product: '<S1>/Product3'
   *  Sum: '<S1>/Add'
   *  Sum: '<S1>/Add1'
   */
  rtb_Gain = ((rtb_Gain * (real_T)rtb_FixPtRelationalOperator / rtb_Saturation +
               -36.0 * rtDW.Memory2_PreviousInput) / rtb_Saturation + -9.0 *
              rtDW.Memory3_PreviousInput) / rtb_Saturation;

  /* DiscreteIntegrator: '<S1>/Discrete-Time Integrator2' */
  rtb_Saturation = 0.0005 * rtb_Gain + rtDW.DiscreteTimeIntegrator2_DSTATE;

  /* Product: '<S1>/Product1' */
  rtb_Product1 = rtb_Saturation * (real_T)rtb_FixPtRelationalOperator;

  /* DiscreteIntegrator: '<S1>/Discrete-Time Integrator1' */
  DiscreteTimeIntegrator1 = 0.0005 * rtb_Product1 +
    rtDW.DiscreteTimeIntegrator1_DSTATE;

  /* Product: '<S1>/Product2' */
  rtb_Product2 = DiscreteTimeIntegrator1 * (real_T)rtb_FixPtRelationalOperator;

  /* Outport: '<Root>/pref' incorporates:
   *  DiscreteIntegrator: '<S1>/Discrete-Time Integrator'
   */
  rtY.pref = 0.0005 * rtb_Product2 + rtDW.DiscreteTimeIntegrator_DSTATE;

  /* Update for UnitDelay: '<S3>/Delay Input1' incorporates:
   *  Inport: '<Root>/pfin'
   *
   * Block description for '<S3>/Delay Input1':
   *
   *  Store in Global RAM
   */
  rtDW.DelayInput1_DSTATE = rtU.pfin;

  /* Update for Memory: '<S1>/Memory' */
  rtDW.Memory_PreviousInput = rtb_Product;

  /* Update for Memory: '<S1>/Memory2' */
  rtDW.Memory2_PreviousInput = rtb_Product2;

  /* Update for Memory: '<S1>/Memory3' */
  rtDW.Memory3_PreviousInput = rtb_Product1;

  /* Update for DiscreteIntegrator: '<S1>/Discrete-Time Integrator2' */
  rtDW.DiscreteTimeIntegrator2_DSTATE = 0.0005 * rtb_Gain + rtb_Saturation;

  /* Update for DiscreteIntegrator: '<S1>/Discrete-Time Integrator1' */
  rtDW.DiscreteTimeIntegrator1_DSTATE = 0.0005 * rtb_Product1 +
    DiscreteTimeIntegrator1;

  /* Update for DiscreteIntegrator: '<S1>/Discrete-Time Integrator' incorporates:
   *  Outport: '<Root>/pref'
   */
  rtDW.DiscreteTimeIntegrator_DSTATE = 0.0005 * rtb_Product2 + rtY.pref;

  /* End of Outputs for SubSystem: '<Root>/minjerk' */

  /* Outport: '<Root>/vref' */
  rtY.vref = rtb_Product2;

  /* Outport: '<Root>/aref' */
  rtY.aref = rtb_Product1;

  /* Outputs for Atomic SubSystem: '<Root>/minjerk' */
  /* Outport: '<Root>/EOT' incorporates:
   *  Logic: '<S1>/NOT1'
   */
  rtY.EOT = !rtb_FixPtRelationalOperator;

  /* End of Outputs for SubSystem: '<Root>/minjerk' */

  /* Update absolute time for base rate */
  /* The "clockTick0" counts the number of times the code of this task has
   * been executed. The absolute time is the multiplication of "clockTick0"
   * and "Timing.stepSize0". Size of "clockTick0" ensures timer will not
   * overflow during the application lifespan selected.
   */
  rtM->Timing.t[0] =
    ((time_T)(++rtM->Timing.clockTick0)) * rtM->Timing.stepSize0;

  {
    /* Update absolute timer for sample time: [0.001s, 0.0s] */
    /* The "clockTick1" counts the number of times the code of this task has
     * been executed. The resolution of this integer timer is 0.001, which is the step size
     * of the task. Size of "clockTick1" ensures timer will not overflow during the
     * application lifespan selected.
     */
    rtM->Timing.clockTick1++;
  }
}

/* Model initialize function */
void minjerk_initialize(void)
{
  /* Registration code */
  {
    /* Setup solver object */
    rtsiSetSimTimeStepPtr(&rtM->solverInfo, &rtM->Timing.simTimeStep);
    rtsiSetTPtr(&rtM->solverInfo, &rtmGetTPtr(rtM));
    rtsiSetStepSizePtr(&rtM->solverInfo, &rtM->Timing.stepSize0);
    rtsiSetErrorStatusPtr(&rtM->solverInfo, (&rtmGetErrorStatus(rtM)));
    rtsiSetRTModelPtr(&rtM->solverInfo, rtM);
  }

  rtsiSetSimTimeStep(&rtM->solverInfo, MAJOR_TIME_STEP);
  rtsiSetSolverName(&rtM->solverInfo,"FixedStepDiscrete");
  rtmSetTPtr(rtM, &rtM->Timing.tArray[0]);
  rtM->Timing.stepSize0 = 0.001;
  rtPrevZCX.SampleandHold_Trig_ZCE = POS_ZCSIG;
}

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
