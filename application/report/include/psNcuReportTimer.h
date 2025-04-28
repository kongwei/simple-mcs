#ifndef PS_NCU_REPORT_TIMER_H_
#define PS_NCU_REPORT_TIMER_H_

#ifdef __cplusplus
extern "C" {
#endif
#include "tulip.h"
void psNcuDataReportTimerInit();
__inline WORD64 getIncreasedClockStepSize(WORD64 ddwCurrentTick);
#define CLOCK_STEP_POWER    (4)  /* 精确定时器步长：一秒的2^x分之一 */
#define CLOCK_STEP_LEN      (1<<CLOCK_STEP_POWER)
    WORD64 getCurSetTimerClockStep();
WORD64 getCurClockStep();



static __inline WORD32 milliSecToClockStep(WORD32 milliSec)
{
    WORD32  val = (WORD32)(((WORD64)milliSec<<CLOCK_STEP_POWER)/1000);
       
    return (0 == val ? 1 : val);
}

static __inline WORD64 secToClockStep(WORD32 sec)
{
    WORD64  val = (WORD32)((WORD64)sec<<CLOCK_STEP_POWER);

    return (0 == val ? 1 : val);
}

#ifdef __cplusplus
}
#endif
#endif