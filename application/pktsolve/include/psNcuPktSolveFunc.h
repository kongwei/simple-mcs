#ifndef PS_NCU_PKT_SOLVE_FUNC_H_
#define PS_NCU_PKT_SOLVE_FUNC_H_

#include "ps_mcs_define.h"
void psNcuMultiPktSolveProc(void* ptPktData, BYTE bPktNum, WORD16 wLength, T_MediaProcThreadPara *ptMediaProcThreadPara);
void psNcuMultiTrafficReportSolveProc(void* ptTrafficReportData, BYTE bPktNum, WORD16 wLength, T_MediaProcThreadPara *ptMediaProcThreadPara);
void psCheckNcuToPfuSynSessionCtx(T_MediaProcThreadPara* ptMediaProcThreadPara);
WORD32 psPktCheckNcuCtxIsNull(T_MediaProcThreadPara* ptMediaProcThreadPara);
WORD32 psPktCheckNcuCtxIsNullInternal(T_MediaProcThreadPara* ptMediaProcThreadPara);
#endif