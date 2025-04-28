#ifndef PS_NCU_DATA_ANALYSIS_H_
#define PS_NCU_DATA_ANALYSIS_H_
#include "ps_mcs_define.h"
#include "ps_ncu_dataApp.h"
#include "ps_ncu_stream.h"
T_psNcuDaAppCtx* psNcuGetDataAppProc(T_MediaProcThreadPara *ptMediaProcThreadPara);
WORD32 psNcuAddStmToDataApp(T_MediaProcThreadPara *ptMediaProcThreadPara, T_psNcuFlowCtx *ptStmAddr, T_psNcuDaAppCtx *ptDataAppCtx);
WORD32 psNcuRmStmFromDataApp(T_MediaProcThreadPara *ptMediaProcThreadPara, T_psNcuFlowCtx *ptStmAddr, T_psNcuDaAppCtx *ptDataAppCtx);
WORD32 psNcuUpdAllStmDataToDataApp(T_psNcuDaAppCtx *ptDataAppCtx, BYTE bDelSteam, BYTE triggerType);
BYTE getDaTypeFromDataAppCtx(T_psNcuDaAppCtx *ptDataAppCtx);
WORD32 psNcuRestoreDataAppAfterRpt(T_psNcuDaAppCtx *ptDataAppCtx, WORD16 wThreadID);
#endif