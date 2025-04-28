#ifndef PS_NCU_REPORT_SBC_PROC_H_
#define PS_NCU_REPORT_SBC_PROC_H_

#include "ps_ncu_dataApp.h"
#include "ps_ncu_stream.h"
void psNcuReportToUpmProc(T_psNcuDaAppCtx * ptDaAppCtx, T_psNcuFlowCtx *ptStmAddr, WORD16 threadID, BYTE bRptType);

#endif