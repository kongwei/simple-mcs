#ifndef PS_NCU_MULTI_REPORT_H_
#define PS_NCU_MULTI_REPORT_H_

#include "tulip.h"
#include "psUpfCommon.h"
#include "ps_ncu_dataApp.h"
#ifdef __cplusplus
extern "C" {
#endif

#define GROUP_EXP_NORMAL 0
#define GROUP_RPT_MAX_NUM 32
#define GROUP_APP_MAX_NUM 10
typedef struct{
    T_psNcuDaAppCtx* ptAppCtx[GROUP_APP_MAX_NUM];
    T_IPV6 tNwdafIP;
    WORD16 wMtu;
    WORD16 rsv;
    BYTE   bTrigger;
    BYTE   bIPType;
    BYTE   bAppCtxNum;
    BYTE   bIsValid;
}T_psMultiGroupData;
extern T_psMultiGroupData g_tMultiGroup[MEDIA_THRD_NUM][GROUP_RPT_MAX_NUM];
T_psMultiGroupData* psGetMultiGroup(WORD16 threadID, WORD16 groupID);

WORD32 psCheckExpNormalAddAppCtxToGroup(T_psNcuDaAppCtx * ptDaAppCtx, T_psMultiGroupData* ptGroupData, WORD16 threadID);
WORD32 psCheckExpSpecialAddAppCtxToGroup(T_psNcuDaAppCtx * ptDaAppCtx, T_psMultiGroupData* ptGroupData, WORD16 threadID);
void getNormalExpMultiRptInfo(WORD16 threadID, T_psMultiGroupData* ptGroupData);
void psNcuReportMultiProc(T_psMultiGroupData* ptGroupData, WORD16 threadID);
void psNcuReSetDataInfo(T_psMultiGroupData* ptGroupData, WORD16 threadID);
void psNcuShowMultiRptData();
WORD32 psCSSGSUMediaSendOutRust(PS_PACKET *ptPkt);
#ifdef __cplusplus
}
#endif
#endif