#include "rte_config.h"
#include "rte_cycles.h"
#include "psNcuReportTimer.h"
#include "rte_cycles.h"
#include "UPFHelp.h"
#include "tulip_oss.h"
#include "ps_css_interface.h"
#include "UPFHelp.h"
#include "zte_slibc.h"

__thread WORD64 gLastClockStepTsc = 0;
__thread WORD64 gCurClockStep = 0;
__thread WORD64 gCurSetTimerClockStep = 0;
__thread WORD64 gTscHz = 0;
WORD64 MaxIncreasedClockStepSize = 0;
WORD32 g_ClockStep = CLOCK_STEP_POWER;

void psNcuDataReportTimerInit()
{
    gLastClockStepTsc = rte_rdtsc();
    gTscHz            = rte_get_tsc_hz();
}

UPF_HELP_REG("ncu", 
"set clock step size",
void psNcuStubSetClockStepSize(BYTE num))
{
    if(num < 1 || num > 15)
    {
        g_ClockStep = CLOCK_STEP_POWER;
    }
    else
    {
        g_ClockStep = num;
    }
}

WORD64 getCurSetTimerClockStep()
{
    return gCurSetTimerClockStep;
}

WORD64 getCurClockStep() 
{
    return gCurClockStep;
}


__inline WORD64 getIncreasedClockStepSize(WORD64 ddwCurrentTick)
{
    WORD64 StepHZ = gTscHz>>g_ClockStep;
    WORD64 CurTsc = ddwCurrentTick;
    
    if (CurTsc < gLastClockStepTsc) return 0;
    if(0 == StepHZ)
    {
        return 0;
    }
    
    WORD64 IncreasedClockStepSize = (CurTsc-gLastClockStepTsc)/StepHZ;
    
    if (IncreasedClockStepSize > 0)
    {
        gCurClockStep           += IncreasedClockStepSize;
        gCurSetTimerClockStep   = gCurClockStep;
        gLastClockStepTsc       += StepHZ*IncreasedClockStepSize; /* 此处不可直接赋值CurTsc，否则丢粒度 */

        if (IncreasedClockStepSize > MaxIncreasedClockStepSize) 
        {
            MaxIncreasedClockStepSize = IncreasedClockStepSize;
        }
    }

    return IncreasedClockStepSize;
}


void dataReportTimerSHow(void)
{ 
    zte_printf_s("\n---show urr time para Begin---");
    zte_printf_s("\n rte_get_tsc_hz()             = %llu", (WORD64)rte_get_tsc_hz());
    zte_printf_s("\n rte_rdtsc()                  = %llu", (WORD64)rte_rdtsc());
    zte_printf_s("\n gLastClockStepTsc            = %llu", gLastClockStepTsc);
    zte_printf_s("\n gCurClockStep                = %llu", gCurClockStep);
    zte_printf_s("\n gCurSetTimerClockStep        = %llu", gCurSetTimerClockStep);
    zte_printf_s("\n gTscHz                       = %llu", gTscHz);
    zte_printf_s("\n MaxStepSize                  = %llu", MaxIncreasedClockStepSize);
    
    zte_printf_s("\n XOS_GetCurStdSecs()          = %u", XOS_GetCurStdSecs());
    zte_printf_s("\n psFtmGetCurStdSec()          = %u", psFtmGetCurStdSec());
    zte_printf_s("\n XOS_GetSecFromPowerOn()      = %u", XOS_GetSecFromPowerOn());
    zte_printf_s("\n XOS_GetSysRunSecFromPowerOn()= %u", XOS_GetSysRunSecFromPowerOn());
    zte_printf_s("\n psFtmGetCurStdMilliSec()     = %llu", psFtmGetCurStdMilliSec());
    zte_printf_s("\n XOS_GetMilliSecFromPowerOn() = %llu", (WORD64)XOS_GetMilliSecFromPowerOn());
    
    zte_printf_s("\n---end---\n");
} 