#include "psNcuDataAnalysis.h"
#include "psNcuReport.h"
#include "ps_db_define_ncu.h"
#include "psNcuSubscribeCtxProc.h"
#include "psNcuSubBindAppCtxProc.h"
#include "ps_ncu_stream.h"
#include "ps_ncu_session.h"
#include "psMcsDebug.h"
#include "psNcuSnCtxProc.h"
#include "zte_slibc.h"
#include "McsProtoType.h"
#include "McsHeadCap.h"
#include "psNcuStreamProc.h"
#include "psNcuCtxFunc.h"

#define SETHASTCPSTMFLAG(curStm, bHasTcpStmFlg)\
do{\
    if (curStm->bProType == MCS_TCP_PROTOCOL && curStm->bSpecialStmFlag == 0)\
    {\
        bHasTcpStmFlg = 1;\
    }\
}while(0)

void psNcuUpdStmAnalysisToDataApp(T_psNcuFlowCtx* ptTmpStream, T_psNcuDataAnalysis* ptAnalysisData);
VOID psNcuUpdDataAppForNonTcpStm(T_psNcuDataAnalysis *ptAnalysisData);
extern WORD32 g_ana_tim;
extern WORD32 g_exp_tim;

VOID psNcuInitDataAppAna(T_psNcuDaAppCtx *ptDataAppCtx)
{
    if(NULL == ptDataAppCtx)
    {
        return;
    }
    ptDataAppCtx->dwUlTotalThroughput = 0;
    ptDataAppCtx->dwUlLastThroughput = 0;
    ptDataAppCtx->dwDlTotalThroughput = 0;
    ptDataAppCtx->dwDlLastThroughput = 0;

    ptDataAppCtx->dwUlTotalPktRtt = 0;
    ptDataAppCtx->dwUlLastlPktRtt= 0;
    ptDataAppCtx->dwDlTotalPktRtt= 0;
    ptDataAppCtx->dwDlLastlPktRtt= 0;
    ptDataAppCtx->dwTotalUlRttPktNum = 0;
    ptDataAppCtx->dwLastUlRttPktNum = 0;
    ptDataAppCtx->dwTotalDlRttPktNum = 0;
    ptDataAppCtx->dwLastDlRttPktNum = 0;
    // 丢包率计算
    ptDataAppCtx->dwTotalUlPktLossNum = 0;
    ptDataAppCtx->dwLastUlPktLossNum = 0;
    ptDataAppCtx->dwTotalDlPktLossNum = 0;
    ptDataAppCtx->dwLastDlPktLossNum = 0;
    ptDataAppCtx->dwTotalUlPktNum = 0;
    ptDataAppCtx->dwLastUlPktNum = 0;
    ptDataAppCtx->dwTotalDlPktNum = 0;
    ptDataAppCtx->dwLastDlPktNum = 0;
    
    ptDataAppCtx->bHasPoor = 0; /*是否发生过质差*/
    ptDataAppCtx->bCurQoe = DA_QUALITY_GOOD;  /*当前状态*/
    ptDataAppCtx->bHasPoorNum = 0;
    ptDataAppCtx->bHasGoodNum = 0;
    ptDataAppCtx->bCurDialQoe = 0xFF; /*质差状态*/
    return;
}

T_psNcuDaAppCtx* psNcuGetDataAppProc(T_MediaProcThreadPara *ptMediaProcThreadPara)
{
    T_psNcuMcsPerform *ptMcsNcuPerform = (T_psNcuMcsPerform*)ptMediaProcThreadPara->ptMcsStatPointer;
    MCS_CHECK_NULLPTR_RET_1ARG(ptMcsNcuPerform, NULL);
    
    T_psNcuDaAppCtx *ptDataAppCtx = NULL;
    WORD64   ddwUPSeid            = ptMediaProcThreadPara->ddwUPseid;
    WORD32   dwSubAppid           = ptMediaProcThreadPara->dwSubAppid;
    WORD32   dwhDB                = ptMediaProcThreadPara->dwhDBByThreadNo;

    DEBUG_TRACE(DEBUG_LOW,"psQueryDaAppCtxByUpseidSubAppid, upseid=%llu, subappid=%u\n", ddwUPSeid, dwSubAppid);
    ptDataAppCtx = psQueryDaAppCtxByUpseidSubAppid(ddwUPSeid, dwSubAppid, dwhDB);
    if(NULL == ptDataAppCtx)
    {
        
        ptDataAppCtx = psCrtDaAppCtxByUpseidSubAppid(ddwUPSeid, dwSubAppid, dwhDB);
        if(NULL == ptDataAppCtx)
        {
            DEBUG_TRACE(DEBUG_LOW,"psCrtDaAppCtxByUpseidSubAppid fail!\n");
            return NULL;
        }
        
        WORD32 dwAppid    = psNcuGetAppidBySubAppid(dwSubAppid);
        DEBUG_TRACE(DEBUG_LOW,"psCrtDaAppCtxByUpseidSubAppid succ, appid=%u!\n",dwAppid);
        if(MCS_RET_FAIL == psNcuMcsUpdDaAppCtxByAppid(ddwUPSeid,dwAppid, ptDataAppCtx->dwID, dwhDB))
        {
            DEBUG_TRACE(DEBUG_LOW,"psNcuMcsUpdDaAppCtxByAppid fail!\n");
        }
        psNcuInitDataAppAna(ptDataAppCtx);
        ptDataAppCtx->dwSubAppid = dwSubAppid;
        ptDataAppCtx->dwAppid = dwAppid;
        T_psNcuDaSubScribeCtx* ptSubScribeCtx = psQuerySubscribeByUpseidAppid(ddwUPSeid, dwAppid, dwhDB);
        if(NULL != ptSubScribeCtx)
        {
            ptDataAppCtx->ptSubScribeCtx = (void*)ptSubScribeCtx;
            ptSubScribeCtx->dwUpdateTimeStamp = ptMediaProcThreadPara->dwPktPowOnSec;
        }
        T_psNcuSessionCtx* ptSessionCtx = psQuerySessionByUpseid(ddwUPSeid,dwhDB);
        if(NULL != ptSessionCtx)
        {
            ptDataAppCtx->ptSessionCtx = (void*)ptSessionCtx;
            ptSessionCtx->dwUpdateTimeStamp = ptMediaProcThreadPara->dwPktPowOnSec;
            
            if(ptSessionCtx->bNcuToPfuSynSessionctx == 0 && ptSubScribeCtx == NULL)
            {
                psNcuToPfuSynSessionCtxReq(ptMediaProcThreadPara);            
                ptSessionCtx->bNcuToPfuSynSessionctx = 1;
                ptSessionCtx->dwUpdateTimeStamp = psFtmGetPowerOnSec();
                MCS_LOC_STAT_EX(ptMcsNcuPerform, qwNcuToPfuSynWhenSubScribeCtxNull, 1);
            }
        }
        
        if(FALSE == psNcuGetSubAppidStr(ptDataAppCtx->subAppidStr,dwSubAppid))
        {
            DEBUG_TRACE(DEBUG_LOW,"psNcuGetSubAppidStr fail!\n");
        }
        XOS_GetCurrentTime(&(ptDataAppCtx->tCrtStdTime));
        ptDataAppCtx->dwUpdateTimeStamp = ptMediaProcThreadPara->dwPktPowOnSec;
        ptDataAppCtx->dwStreamNum = 0;
        ptDataAppCtx->ptFlowCtxHead = NULL;
        psNcuSetDataReportTimer(ptDataAppCtx,ptMediaProcThreadPara);
    }
    else
    {
        DEBUG_TRACE(DEBUG_LOW,"psQueryDaAppCtxByUpseidSubAppid succ\n");
        ptDataAppCtx->dwUpdateTimeStamp = ptMediaProcThreadPara->dwPktPowOnSec;
    }

    return ptDataAppCtx;
}

WORD32 psNcuAddStmToDataApp(T_MediaProcThreadPara *ptMediaProcThreadPara, T_psNcuFlowCtx *ptStmAddr, T_psNcuDaAppCtx *ptDataAppCtx)
{
    if(NULL == ptMediaProcThreadPara || NULL == ptStmAddr || NULL == ptDataAppCtx)
    {
        return MCS_RET_FAIL;
    }
    T_psNcuMcsPerform *ptMcsNcuPerform = (T_psNcuMcsPerform*)ptMediaProcThreadPara->ptMcsStatPointer;
    if(NULL == ptMcsNcuPerform)
    {
        return MCS_RET_FAIL;
    }
    if(ptDataAppCtx->dwStreamNum >= 1000)
    {
        MCS_LOC_STAT_EX(ptMcsNcuPerform, qwAddStmToAppOver1000, 1);
        return MCS_RET_FAIL;
    }
    if(NULL != ptStmAddr->ptNcuDaCtx)
    {
        MCS_LOC_STAT_EX(ptMcsNcuPerform, qwAddStmToAppCtxNull, 1);
        return MCS_RET_FAIL;
    }
    DEBUG_TRACE(DEBUG_LOW,"ptDataAppCtx->dwStreamNum =%u\n", ptDataAppCtx->dwStreamNum );
    ptStmAddr->ptNcuDaCtx = (void *)ptDataAppCtx;
    T_psNcuFlowCtx* ptTmpStream = (T_psNcuFlowCtx*)ptDataAppCtx->ptFlowCtxHead;
    if(NULL == ptTmpStream)
    {
        ptDataAppCtx->ptFlowCtxHead = (void*)ptStmAddr;
    }
    else
    {
        WORD32 dwLoop = 0;
        while(NULL != ptTmpStream->ptNcuFlowAppNext)
        {
            dwLoop++;
            if(dwLoop >= 1000)
            {
                return MCS_RET_FAIL;
            }
            ptTmpStream = (T_psNcuFlowCtx*)ptTmpStream->ptNcuFlowAppNext;
        }
        ptTmpStream->ptNcuFlowAppNext = (void *)ptStmAddr;
    }
    ptDataAppCtx->dwStreamNum ++;

    return MCS_RET_SUCCESS;
}

WORD32 psNcuRmStmFromDataApp(T_MediaProcThreadPara *ptMediaProcThreadPara, T_psNcuFlowCtx *ptStmAddr, T_psNcuDaAppCtx *ptDataAppCtx)
{
    if(NULL == ptMediaProcThreadPara || NULL == ptStmAddr || NULL == ptDataAppCtx)
    {
        return MCS_RET_FAIL;
    }
    T_psNcuMcsPerform *ptMcsNcuPerform = (T_psNcuMcsPerform*)ptMediaProcThreadPara->ptMcsStatPointer;
    if(NULL == ptMcsNcuPerform)
    {
        return MCS_RET_FAIL;
    }
    DEBUG_TRACE(DEBUG_LOW,"ptDataAppCtx->dwStreamNum =%u\n", ptDataAppCtx->dwStreamNum );
    T_psNcuFlowCtx* ptTmpStream = (T_psNcuFlowCtx*)ptDataAppCtx->ptFlowCtxHead;
    if(NULL == ptTmpStream || 0 == ptDataAppCtx->dwStreamNum)
    {
        return MCS_RET_SUCCESS;
    }
    if(ptTmpStream == ptStmAddr)
    {
        ptDataAppCtx->ptFlowCtxHead = ptStmAddr->ptNcuFlowAppNext;
        ptDataAppCtx->dwStreamNum --;
        if(NULL == ptDataAppCtx->ptFlowCtxHead || 0 == ptDataAppCtx->dwStreamNum)
        {
            MCS_LOC_STAT_EX(ptMcsNcuPerform, qwDelStmFinalWithDaCtxDel, 1);
            DEBUG_TRACE(DEBUG_LOW,"ptDataAppCtx->ptFlowCtxHead=%p, ptDataAppCtx->dwStreamNum=%u\n", ptDataAppCtx->ptFlowCtxHead,ptDataAppCtx->dwStreamNum);
            psVpfuMcsDelCtxById(ptDataAppCtx->dwID, ptMediaProcThreadPara->dwhDBByThreadNo, DB_HANDLE_R_NCUDATAAPP);
        }
        return MCS_RET_SUCCESS;
    }

    WORD32 dwLoop = 0;
    while(NULL != ptTmpStream->ptNcuFlowAppNext)
    {
        dwLoop ++ ;
        if(dwLoop >= 1000)
        {
            return MCS_RET_FAIL;
        }
        if(ptTmpStream->ptNcuFlowAppNext == ptStmAddr)
        {
            break;
        }
        ptTmpStream = (T_psNcuFlowCtx*)ptTmpStream->ptNcuFlowAppNext;
    }
    ptTmpStream->ptNcuFlowAppNext = ptStmAddr->ptNcuFlowAppNext;
    ptDataAppCtx->dwStreamNum --;
    if(NULL == ptDataAppCtx->ptFlowCtxHead || 0 == ptDataAppCtx->dwStreamNum)
    {
        MCS_LOC_STAT_EX(ptMcsNcuPerform, qwDelStmFinalWithDaCtxDel, 1);
        DEBUG_TRACE(DEBUG_LOW,"ptDataAppCtx->ptFlowCtxHead=%p, ptDataAppCtx->dwStreamNum=%u\n", ptDataAppCtx->ptFlowCtxHead,ptDataAppCtx->dwStreamNum);
        psVpfuMcsDelCtxById(ptDataAppCtx->dwID, ptMediaProcThreadPara->dwhDBByThreadNo, DB_HANDLE_R_NCUDATAAPP);
    }
    return MCS_RET_SUCCESS;
}

BYTE getDaTypeFromDataAppCtx(T_psNcuDaAppCtx *ptDataAppCtx)
{
    BYTE bSubType = 0;

    if(NULL == ptDataAppCtx || NULL == ptDataAppCtx->ptSubScribeCtx)
    {
        return 0;
    }
    T_psNcuDaSubScribeCtx* ptSubScribeCtx = ptDataAppCtx->ptSubScribeCtx;
    bSubType = ptSubScribeCtx->bSubType;
    return bSubType;


    return 0;
}

WORD32 psNcuUpdAllStmDataToDataApp(T_psNcuDaAppCtx *ptDataAppCtx, BYTE bDelSteam, BYTE triggerType)
{
    
    if(NULL == g_ptMediaProcThreadPara|| NULL == ptDataAppCtx)
    {
        return MCS_RET_FAIL;
    }
    DEBUG_TRACE(DEBUG_LOW,"ptDataAppCtx->dwStreamNum =%u\n", ptDataAppCtx->dwStreamNum );

    T_psNcuFlowCtx* ptTmpStream = (T_psNcuFlowCtx*)ptDataAppCtx->ptFlowCtxHead;
    WORD32 dwLoop = 0;
    BYTE   bHasTcpStmFlg = 0;
    while(NULL != ptTmpStream)
    {
        dwLoop++;
        if(dwLoop >= 1000)
        {
            break;
        }
        T_psNcuFlowCtx* curStm = ptTmpStream;
        ptTmpStream =  (T_psNcuFlowCtx*)curStm->ptNcuFlowAppNext;
        psNcuUpdStmAnalysisToDataApp(curStm, &ptDataAppCtx->Analysis);

        SETHASTCPSTMFLAG(curStm, bHasTcpStmFlg);

        if(TRUE == bDelSteam)
        {
            psNcuMcsRelStmByID(curStm->dwFlowID, g_ptMediaProcThreadPara->dwhDBByThreadNo);
            ptDataAppCtx->dwStreamNum --;
            psDelAllSnCtxByFlowId(curStm->dwFlowID, g_ptMediaProcThreadPara->dwhDBByThreadNo);
        }
    }
    if (!bHasTcpStmFlg)
    {
        psNcuUpdDataAppForNonTcpStm(&ptDataAppCtx->Analysis);
    }
    return MCS_RET_SUCCESS;
}

WORD32 psNcuRestoreDataAppAfterRpt(T_psNcuDaAppCtx *ptDataAppCtx, WORD16 wThreadID)
{
    if(NULL == ptDataAppCtx)
    {
        return MCS_RET_FAIL;
    }
    T_psNcuDataAnalysis* ptAnalysisData  = &ptDataAppCtx->Analysis;
    zte_memset_s(ptAnalysisData, sizeof(T_psNcuDataAnalysis), 0, sizeof(T_psNcuDataAnalysis));
    return MCS_RET_SUCCESS;
}

inline WORD32 getAndClrValue(WORD32* value)
{
    if (NULL == value)
    {
        return 0;
    }
    WORD32 ret = *value;
    *value = 0;
    return ret;
}

WORD32 psNcuGetRttByDir(T_psNcuDataAnalysis *ptAnalysisData, BYTE bDir)
{
    if (NULL == ptAnalysisData)
    {
        return 0;
    }
    BYTE bTargetDir = bDir;
    if (!getNcuSoftPara(5044))
    {
        bTargetDir = bDir ^ MCS_PKT_DIR_DL;
    }
    if (MCS_PKT_DIR_DL == bTargetDir)
    {
        return ptAnalysisData->dwDlRttPktNum !=0 ? ptAnalysisData->dwDlRtt/ptAnalysisData->dwDlRttPktNum : 0;
    }
    return ptAnalysisData->dwUlRttPktNum !=0 ? ptAnalysisData->dwUlRtt/ptAnalysisData->dwUlRttPktNum : 0;
}

void psNcuUpdStmAnalysisToDataApp(T_psNcuFlowCtx* ptTmpStream, T_psNcuDataAnalysis *ptAnalysisData)
{
    if(NULL == ptTmpStream || NULL == ptAnalysisData )
    {
        return;
    }
    WORD32 dwCurTimeStamp = psFtmGetPowerOnSec();
    WORD32 timeInternal   = dwCurTimeStamp > ptTmpStream->dwRptTimeStamp ? dwCurTimeStamp - ptTmpStream->dwRptTimeStamp : 1;
    ptTmpStream->dwRptTimeStamp = dwCurTimeStamp;

    ptAnalysisData->dwUlThroughput += getAndClrValue(&ptTmpStream->dwUlPktBytes)*8/timeInternal; //bps
    ptAnalysisData->dwDlThroughput += getAndClrValue(&ptTmpStream->dwDlPktBytes)*8/timeInternal; //bps
    ptAnalysisData->dwUlPktNum += getAndClrValue(&ptTmpStream->dwUlPktNum);
    ptAnalysisData->dwDlPktNum += getAndClrValue(&ptTmpStream->dwDlPktNum);
    ptAnalysisData->dwUlRtt += getAndClrValue(&ptTmpStream->dwUlPktRtt);
    ptAnalysisData->dwDlRtt += getAndClrValue(&ptTmpStream->dwDlPktRtt);
    ptAnalysisData->dwUlRttPktNum += getAndClrValue(&ptTmpStream->dwUlRttPktNum);
    ptAnalysisData->dwDlRttPktNum += getAndClrValue(&ptTmpStream->dwDlRttPktNum);
    ptAnalysisData->dwUlTcpPktNum += getAndClrValue(&ptTmpStream->dwUlTcpPktNum);
    ptAnalysisData->dwDlTcpPktNum += getAndClrValue(&ptTmpStream->dwDlTcpPktNum);
    ptAnalysisData->dwUlLossNum += getAndClrValue(&ptTmpStream->dwUlPktLossNum);
    ptAnalysisData->dwDlLossNum += getAndClrValue(&ptTmpStream->dwDlPktLossNum);
    ptAnalysisData->dwUlAvgBps = 1000;
    ptAnalysisData->dwDlAvgBps = 1000;
    ptAnalysisData->dwUlAvgRtt = psNcuGetRttByDir(ptAnalysisData, MCS_PKT_DIR_UL);
    ptAnalysisData->dwDlAvgRtt = psNcuGetRttByDir(ptAnalysisData, MCS_PKT_DIR_DL);
}

VOID psNcuUpdDataAppForNonTcpStm(T_psNcuDataAnalysis *ptAnalysisData)
{
    if (NULL == ptAnalysisData)
    {
        return ;
    }

    DEBUG_TRACE(DEBUG_LOW,"dataApp associate non Tcp Stm\n");

    ptAnalysisData->dwUlAvgRtt    = (WORD32)-1;
    ptAnalysisData->dwDlAvgRtt    = (WORD32)-1;
    ptAnalysisData->dwUlTcpPktNum = (WORD32)-1;
    ptAnalysisData->dwDlTcpPktNum = (WORD32)-1;
    ptAnalysisData->dwUlLossNum   = (WORD32)-1;
    ptAnalysisData->dwDlLossNum   = (WORD32)-1;
}
