#ifndef PS_NCU_HTTPLINK_PROC_H_
#define PS_NCU_HTTPLINK_PROC_H_

#include "UpfNcuSynInfo.h"
#include "NcuSynInfo.h"
#include "ps_mcs_define.h"
#include "ps_ncu_session.h"

#ifdef __cplusplus
extern "C"
{
#endif

#include "ps_ncu_httplink.h"
#include "ps_ncu_session.h"

    VOID psNcuResetReportLinkFaultFlag(T_psNcuSessionCtx *ptSession, T_NcuSynSubInfo *ptSubScribeInfo);
    T_psNcuHttpLink *psNcuGetHttpLinkByReportUri(const char *reportUri, WORD32 dwhDB);
    WORD32 psNcuGetHttpLinkInfo(T_MediaProcThreadPara *ptMediaProcThreadPara, T_NcuSynSubInfo *ptSubScribeInfo);
    BOOL psNcuDetectAndGetValidLink(void);
    WORD32 psNcuSendNotifyReqToNwdaf(BYTE *msg, WORD16 msgLen, WORD32 msgID, T_psNcuHttpLink *ptNcuHttpLinkCtx);
    bool psNcuIsHttpLinkFault(T_psNcuHttpLink *ptNcuHttpLinkCtx, T_psNcuSessionCtx *ptSessionCtx);
    bool psNcuIsNeedReportLinkFault(T_psNcuSessionCtx *ptSessionCtx);
    WORD32 psNcuReportLinkFaultToPfu(T_psNcuSessionCtx *ptSessionCtx);
    BOOL psNcuSendHttpLinkDetectMsg(T_MediaProcThreadPara *ptMediaProcThreadPara, T_psNcuHttpLink *ptNcuHttpLinkCtx);
    BOOL psNcuGetHttpLbLogicNoAndInstNo(WORD32 *dwScIndex, BYTE *bInstIndex);

#ifdef __cplusplus
}
#endif
#endif
