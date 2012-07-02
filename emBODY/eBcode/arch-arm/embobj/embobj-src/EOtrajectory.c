
/* @file       EOtrajectory.c
    @brief      This file implements internal implementation of a motor minimum jerk trajectory generator.
    @author     alessandro.scalzo@iit.it
    @date       27/03/2012
**/


// --------------------------------------------------------------------------------------------------------------------
// - external dependencies
// --------------------------------------------------------------------------------------------------------------------

#include "stdlib.h"
#include "EoCommon.h"

#include "EOtheMemoryPool.h"
#include "EOtheErrorManager.h"
#include "EOVtheSystem.h"

extern const float   EMS_FFREQUENCY;
extern const float   EMS_PERIOD;

// --------------------------------------------------------------------------------------------------------------------
// - declaration of extern public interface
// --------------------------------------------------------------------------------------------------------------------

#include "EOtrajectory.h"


// --------------------------------------------------------------------------------------------------------------------
// - declaration of extern hidden interface 
// --------------------------------------------------------------------------------------------------------------------

#include "EOtrajectory_hid.h" 


// --------------------------------------------------------------------------------------------------------------------
// - #define with internal scope
// --------------------------------------------------------------------------------------------------------------------

#define LIMIT(min, x, max) if (x < (min)) x = (min); else if (x > (max)) x = (max); 

// --------------------------------------------------------------------------------------------------------------------
// - definition (and initialisation) of extern variables, but better using _get(), _set() 
// --------------------------------------------------------------------------------------------------------------------
// empty-section

// --------------------------------------------------------------------------------------------------------------------
// - typedef with internal scope
// --------------------------------------------------------------------------------------------------------------------
// empty-section


// --------------------------------------------------------------------------------------------------------------------
// - declaration of static functions
// --------------------------------------------------------------------------------------------------------------------
// empty-section


// --------------------------------------------------------------------------------------------------------------------
// - definition (and initialisation) of static variables
// --------------------------------------------------------------------------------------------------------------------

static const char s_eobj_ownname[] = "EOtrajectory";

// --------------------------------------------------------------------------------------------------------------------
// - definition of extern public functions
// --------------------------------------------------------------------------------------------------------------------

extern EOtrajectory* eo_trajectory_New(void) 
{
    EOtrajectory *o = eo_mempool_GetMemory(eo_mempool_GetHandle(), eo_mempool_align_32bit, sizeof(EOtrajectory), 1);

    if (o)
    {
        o->Pn = 0.0f;
        o->Vn = 0.0f;
        o->An = 0.0f;

        o->Pf = 0.0f;
        o->Vf = 0.0f;

        o->Ky = 0.0f;
        o->Yn = 0.0f;
        o->Xn = 0.0f;

        o->pos_steps_to_end = 0;
        o->vel_steps_to_end = 0;

        o->pos_min =    0;
        o->pos_max = 4095;
        o->vel_max = 2048;
        o->avg_vel = 1024;

        o->boost = eobool_false;
        o->boostVf = 0.0f;
        o->boostPn = 0.0f;
        o->boostVn = 0.0f;
        o->boostAn = 0.0f;
        o->boostKy = 0.0f;
        o->boostYn = 0.0f;
        o->boost_steps_to_end = 0;
    }

    return o;
}

extern void eo_trajectory_SetLimits(EOtrajectory *o, int32_t pos_min, int32_t pos_max, int32_t vel_max)
{
    o->pos_min = pos_min;
    o->pos_max = pos_max;
    o->vel_max = vel_max;
}

extern void eo_trajectory_Init(EOtrajectory *o, int32_t p0, int32_t v0, int32_t a0)
{
    o->Pn = p0;
    o->Vn = v0;
    o->An = a0;
}

extern void eo_trajectory_SetPosReference(EOtrajectory *o, int32_t p1, int32_t avg_vel)
{
    o->vel_steps_to_end = 0;
    o->Vf = 0.0f;

    if (!avg_vel) avg_vel = 1024;

    o->avg_vel = avg_vel;

    o->Pf = (float)p1;

    LIMIT(o->pos_min, o->Pf, o->pos_max)

    float D = o->Pf-o->Pn;

    float Va = (float)avg_vel;

    if ((D<0.0f) ^ (Va<0.0f)) Va = -Va;

    float T = D/Va;

    o->pos_steps_to_end = (int32_t)(T*EMS_FFREQUENCY);
    
    float lbyT = 1.0f/T;
    float PbyT = EMS_PERIOD*lbyT;

    float DbyT2 =    Va*lbyT;
    float VbyT  = o->Vn*lbyT; 

    float A3 = ( 120.0f*DbyT2-60.0f*VbyT-10.0f*o->An)*PbyT*PbyT*PbyT;
    float A2 = (-180.0f*DbyT2+96.0f*VbyT+18.0f*o->An)*PbyT*PbyT;

    o->Ky = 6.0f*A3;
    o->Yn = 2.0f*A2+o->Ky;
    o->Xn = (60.0f*DbyT2-36.0f*VbyT-9.0f*o->An)*PbyT+A2+A3;
}

extern void eo_trajectory_SetVelReference(EOtrajectory *o, int32_t v1, int32_t avg_acc)
{
    o->pos_steps_to_end = 0;

    if (!avg_acc) avg_acc = 1024;

    o->Vf = (float)v1;

    LIMIT(-o->vel_max, o->Vf, o->vel_max)

    float D = o->Vf-o->Vn;
   
    float Aa = (float)avg_acc;

    if ((D<0.0f) ^ (Aa<0.0f)) Aa = -Aa;

    float T = D/Aa;   
   
    o->vel_steps_to_end = (int32_t)(T*EMS_FFREQUENCY);

    float PbyT = EMS_PERIOD/T;

    o->Ky = (-12.0f*Aa+6.0f*o->An)*PbyT*PbyT;
    o->Yn = (  6.0f*Aa-4.0f*o->An)*PbyT+0.5f*o->Ky;
}

extern void eo_trajectory_AddVelReference(EOtrajectory *o, int32_t v1, int32_t avg_acc)
{
    o->boost = eobool_true;

    if (!avg_acc)
    {
        o->boost_steps_to_end = 0;
        o->boostVf = o->boostVn;
        return;
    }

    o->boostVf = (float)v1;

    LIMIT(-o->vel_max, o->boostVf, o->vel_max)

    float D = o->boostVf-o->boostVn;
   
    float Aa = (float)avg_acc;

    if ((D<0.0f) ^ (Aa<0.0f)) Aa = -Aa;

    float T = D/Aa;   
   
    o->boost_steps_to_end = (int32_t)(T*EMS_FFREQUENCY);

    float PbyT = EMS_PERIOD/T;

    o->boostKy = (-12.0f*Aa+6.0f*o->boostAn)*PbyT*PbyT;
    o->boostYn = (  6.0f*Aa-4.0f*o->boostAn)*PbyT+0.5f*o->boostKy;
}

extern void eo_trajectory_StopBoost(EOtrajectory *o)
{
    o->boost = eobool_false;

    o->Pn += o->boostPn;
    o->Vn += o->boostVn;
    o->An += o->boostAn;

    o->boostPn = 0.0f;
    o->boostVn = 0.0f;
    o->boostVf = 0.0f;
    o->boostAn = 0.0f;
}

extern void eo_trajectory_TimeoutVelReference(EOtrajectory *o)
{
    if (o->boost)
    {
        eo_trajectory_StopBoost(o);
        eo_trajectory_SetPosReference(o, (int32_t)o->Pf, (int32_t)o->avg_vel);
    }
}

extern void eo_trajectory_Stop(EOtrajectory *o)
{
    o->pos_steps_to_end = 0;
    o->vel_steps_to_end = 0;

    o->Pf = o->Pn;
    o->Vf = 0.0f;
}

extern eObool_t eo_trajectory_IsDone(EOtrajectory* o)
{
    return !o->vel_steps_to_end && !o->pos_steps_to_end;
}

extern int32_t eo_trajectory_GetPos(EOtrajectory* o)
{
    return (int32_t)o->Pn;
}

extern int32_t eo_trajectory_GetVel(EOtrajectory* o)
{
    return (int32_t)o->Vn;
}

extern int32_t eo_trajectory_GetAcc(EOtrajectory* o)
{
    return (int32_t)o->An;
}

extern int8_t eo_trajectory_Step(EOtrajectory* o, float *p, float *v)
{
    if (o->pos_steps_to_end)
    {
        --o->pos_steps_to_end;

        o->An += o->Xn;
        o->Xn += o->Yn;
        o->Yn += o->Ky;
        
        o->Vn += EMS_PERIOD*o->An;
        o->Pn += EMS_PERIOD*o->Vn;   
    }
    else if (o->vel_steps_to_end)
    {
        --o->vel_steps_to_end;

        o->An += o->Yn;
        o->Yn += o->Ky;

        o->Vn += EMS_PERIOD*o->An;
        o->Pn += EMS_PERIOD*o->Vn;

        o->Pf = o->Pn; 
    }
    else
    {
        if (o->Vf != 0.0f) o->Pf += EMS_PERIOD*o->Vf;

        o->Vn = o->Vf;        
        o->Pn = o->Pf;
        o->An = 0.0f;
    }

    int8_t limit = 0;

    if (o->Pn < o->pos_min)
    {
        o->pos_steps_to_end = 0;
        o->vel_steps_to_end = 0;

        o->Pf = o->Pn = o->pos_min;
        o->Vf = o->Vn = o->An = 0.0f;

        limit = -1;
    }

    if (o->Pn > o->pos_max)
    {
        o->pos_steps_to_end = 0;
        o->vel_steps_to_end = 0;

        o->Pf = o->Pn = o->pos_max;
        o->Vf = o->Vn = o->An = 0.0f;

        limit =  1;
    }
    
    if (o->boost)
    {
        if (o->boost_steps_to_end)
        {
            --o->boost_steps_to_end;

            o->boostAn += o->boostYn;
            o->boostYn += o->boostKy;

            o->boostVn += EMS_PERIOD*o->boostAn;
            o->boostPn += EMS_PERIOD*o->boostVn;
        }
        else
        {
            if (o->boostVf != 0.0f) o->boostPn += EMS_PERIOD*o->boostVf;
        
            o->boostVn = o->boostVf;
            o->boostAn = 0.0f;        
        }

        //LIMIT((int32_t)o->pos_min, *p, (int32_t)o->pos_max)
    }

    *p = (int32_t)(o->Pn+o->boostPn);
    *v = (int32_t)(o->Vn+o->boostVn);
   
    return limit;
}


// --------------------------------------------------------------------------------------------------------------------
// - definition of extern hidden functions 
// --------------------------------------------------------------------------------------------------------------------
// empty-section


// --------------------------------------------------------------------------------------------------------------------
// - definition of static functions 
// --------------------------------------------------------------------------------------------------------------------
// empty section


// --------------------------------------------------------------------------------------------------------------------
// - end-of-file (leave a blank line after)
// --------------------------------------------------------------------------------------------------------------------
