#ifndef PS_NCU_REPORT_H_
#define PS_NCU_REPORT_H_

#include "tulip.h"
#include "ps_mcs_define.h"
#include "ps_ncu_dataApp.h"
#include "ps_ncu_stream.h"
#ifdef __cplusplus
extern "C" {
#endif
void handleDataAnalysisClock(WORD16 threadID, WORD64 ddwCurrentTick);
void psNcuSetDataReportTimer(T_psNcuDaAppCtx* ptDataAppCtx, T_MediaProcThreadPara *ptMediaProcThreadPara);
void psSetAnaTimerInit(T_psNcuDaAppCtx* ptDataAppCtx, WORD64 dwCurClockStep, WORD32 dwhDB);
void psNcuReportBySubType(BYTE bThreadNo, T_psNcuDaAppCtx *ptDataAppCtx, T_psNcuFlowCtx *ptStmAddr,BYTE bSubType, BYTE bRptType);
#ifdef __cplusplus
}
#endif
#endif