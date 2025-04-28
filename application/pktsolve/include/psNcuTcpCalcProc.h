#ifndef PS_NCU_TCP_CALC_PROC_H_
#define PS_NCU_TCP_CALC_PROC_H_

#include "ps_ncu_stream.h"
#include "ps_mcs_define.h"
#include "psNcuSnCtxProc.h"
VOID psNcuCalcPktBandWith(T_MediaProcThreadPara *ptMediaProcThreadPara, T_psNcuFlowCtx *ptStmAddr);
VOID psNcuCalcPktBandWithByTrafficInfo(T_MediaProcThreadPara *ptMediaProcThreadPara, T_psNcuFlowCtx *ptStmAddr, VOID* ptTrafficData);
VOID psNcuCalcPktRtt(T_MediaProcThreadPara *ptMediaProcThreadPara, T_psNcuFlowCtx *ptStmAddr, VOID* ptPktData);
WORD32 psNcuDaRttListDel(T_MediaProcThreadPara *ptMediaProcThreadPara, T_NcuMcsSnCtx *ptSnCtx);
#endif