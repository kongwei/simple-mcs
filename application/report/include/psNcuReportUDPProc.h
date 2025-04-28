#ifndef PS_NCU_REPORT_UDP_PROC_H_
#define PS_NCU_REPORT_UDP_PROC_H_

#include "ps_ncu_dataApp.h"
#include "ps_ncu_session.h"
void psNcuReportUDPToNwdafProc(T_psNcuDaAppCtx * ptDaAppCtx, WORD16 threadID, BYTE bRptType,BYTE trigger);
WORD32 getExpSpecialNcuIp(T_psNcuSessionCtx* ptSessionCtx, BYTE* ptUpfIP, BYTE* tNwdafIP, BYTE* pbRptIpType);
#endif