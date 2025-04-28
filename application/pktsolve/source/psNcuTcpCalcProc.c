#include "psNcuTcpCalcProc.h"
#include "McsProtoType.h"
#include "psMcsDebug.h"
#include "psNcuPktParse.h"
#include "psNcuSnCtxProc.h"
#include "MemShareCfg.h"
#include "ps_ncu_typedef.h"
#include "psNcuCtxFunc.h"
#include "ps_db_define_ncu.h"
#include "psNcuScanMsgProc.h"
#include "psNcuGetCfg.h"
/**************************************************************************
 *                              宏(本源文件使用)
 **************************************************************************/
#define MAX_DA_CALC_RTT_ITEM_NUMBER                 100

/**************************************************************************
 *                              常量(本源文件使用)
 **************************************************************************/

/**************************************************************************
 *                              数据类型(本源文件使用)
 **************************************************************************/

/**************************************************************************
 *                              全局变量(本源文件使用)
 **************************************************************************/

/**************************************************************************
 *                              外部函数原型(评估后慎重添加)
 **************************************************************************/
extern WORD32 dwTimeDurationUS;
/**************************************************************************
 *                              局部函数原型(本源文件使用)
 **************************************************************************/
WORD32 delUlSnCtxTailItem(T_psNcuFlowCtx *ptStmAddr, T_psNcuMcsPerform *ptNcuPerform, WORD32 dwhDB);
WORD32 delDlSnCtxTailItem(T_psNcuFlowCtx *ptStmAddr, T_psNcuMcsPerform *ptNcuPerform, WORD32 dwhDB);
VOID psVpfuMcsDACalcUlRTTRelItem(T_psNcuFlowCtx *ptStmCtx, T_psNcuMcsPerform *ptNcuPerform, WORD32 dwAckNum, WORD32 dwhDB, WORD64 ddwCurTics);
VOID psVpfuMcsDACalcDlRTTRelItem(T_psNcuFlowCtx *ptStmCtx, T_psNcuMcsPerform *ptNcuPerform, WORD32 dwAckNum, WORD32 dwhDB, WORD64 ddwCurTics);
VOID psVpfuMcsDaUlRttListCtl(T_psNcuFlowCtx *ptStmAddr, T_psNcuMcsPerform *ptNcuPerform, WORD32 seqNum, WORD32 payLoad, WORD32 dwhDB, WORD64 ddwCurTics);
VOID psVpfuMcsDaDlRttListCtl(T_psNcuFlowCtx *ptStmAddr, T_psNcuMcsPerform *ptNcuPerform, WORD32 seqNum, WORD32 payLoad, WORD32 dwhDB, WORD64 ddwCurTics);
VOID psVpfuMcsDaUlRttListInsert(T_psNcuFlowCtx *ptStmCtx, T_psNcuMcsPerform *ptNcuPerform,  T_NcuMcsSnCtx *ptNewSnCtx);
VOID psVpfuMcsDaDlRttListInsert(T_psNcuFlowCtx *ptStmCtx, T_psNcuMcsPerform *ptNcuPerform, T_NcuMcsSnCtx *ptNewSnCtx);
WORD32 psNcuSnCtxQuickAge(T_psNcuFlowCtx *ptStmCtx, T_psNcuMcsPerform *ptNcuPerform, BYTE bDir);
WORD32 psNcuRmSnCtxList(T_psNcuFlowCtx *ptStmCtx, T_NcuMcsSnCtx *ptCurSnCtx, T_psNcuMcsPerform *ptNcuPerform);
VOID psNcuDaUlRttListDel(T_psNcuFlowCtx *ptNcuFlowCtx, T_NcuMcsSnCtx *ptSnCtx);
VOID psNcuDaDlRttListDel(T_psNcuFlowCtx *ptNcuFlowCtx, T_NcuMcsSnCtx *ptSnCtx);
VOID psNcuModSnHeadTail(T_psNcuFlowCtx *ptStmCtx, T_NcuMcsSnCtx* ptPrev, BYTE bDir);
WORD32 psNcuCalcRttFindSnCtx(T_psNcuFlowCtx *ptStmCtx, WORD32 dwAckNum, BYTE bDir, T_NcuMcsSnCtx **ptTargetSnCtx);
/**************************************************************************
 *                              函数实现(函数布局:总分结构|深度优先)
 **************************************************************************/
VOID psNcuCalcPktLoss(T_psNcuMcsPerform* ptNcuPerform, T_psNcuFlowCtx *ptStmAddr, BYTE bDir)
{
    MCS_CHK_NULL_VOID(ptNcuPerform);
    MCS_CHK_NULL_VOID(ptStmAddr);

    MCS_LOC_STAT_EX(ptNcuPerform, qwCalcPktLoss, 1);
    if (MCS_PKT_DIR_DL == bDir)
    {
        ptStmAddr->dwDlPktLossNum++;
        ptStmAddr->dwTotalDlPktLossNum++;
        MCS_LOC_STAT_EX(ptNcuPerform, qwDlPktLoss, 1);
    }
    else
    {
        ptStmAddr->dwUlPktLossNum++;
        ptStmAddr->dwTotalUlPktLossNum++;
        MCS_LOC_STAT_EX(ptNcuPerform, qwUlPktLoss, 1);
    }

    return ;
}

VOID psNcuCalcPktBandWith(T_MediaProcThreadPara *ptMediaProcThreadPara, T_psNcuFlowCtx *ptStmAddr)
{
    MCS_CHK_NULL_VOID(ptMediaProcThreadPara);
    MCS_CHK_NULL_VOID(ptMediaProcThreadPara->ptMcsStatPointer);
    MCS_CHK_NULL_VOID(ptStmAddr);

    T_psNcuMcsPerform* ptNcuPerform = (T_psNcuMcsPerform*)ptMediaProcThreadPara->ptMcsStatPointer;
    MCS_LOC_STAT_EX(ptNcuPerform, qwCalcBandWith, 1);

    T_psNcuPktDesc *ptNcuPktDesc = &ptMediaProcThreadPara->tPktDesc;
    WORD16 wPktLen = ptNcuPktDesc->wPktLen;
    if (MCS_PKT_DIR_DL == ptNcuPktDesc->bDir)
    {
        ptStmAddr->dwTotalDlPktBytes += wPktLen;
        ptStmAddr->dwDlPktBytes += wPktLen;
        ptStmAddr->dwTotalDlPktNum ++;
        ptStmAddr->dwDlPktNum ++;
        MCS_LOC_STAT_EX(ptNcuPerform, qwRcvCopyPktDlNum, 1);
        MCS_LOC_STAT_EX(ptNcuPerform, qwRcvCopyPktDlBytes, wPktLen);
    }
    else
    {
        ptStmAddr->dwTotalUlPktBytes += wPktLen;
        ptStmAddr->dwUlPktBytes += wPktLen;
        ptStmAddr->dwTotalUlPktNum ++;
        ptStmAddr->dwUlPktNum ++;
        MCS_LOC_STAT_EX(ptNcuPerform, qwRcvCopyPktUlNum, 1);
        MCS_LOC_STAT_EX(ptNcuPerform, qwRcvCopyPktUlBytes, wPktLen);
    }
}

VOID psNcuCalcPktBandWithByTrafficInfo(T_MediaProcThreadPara *ptMediaProcThreadPara, T_psNcuFlowCtx *ptStmAddr, VOID* ptTrafficData)
{
    MCS_CHK_NULL_VOID(ptMediaProcThreadPara);
    MCS_CHK_NULL_VOID(ptMediaProcThreadPara->ptMcsStatPointer);
    MCS_CHK_NULL_VOID(ptStmAddr);
    MCS_CHK_NULL_VOID(ptTrafficData);

    T_psNcuMcsPerform* ptNcuPerform = (T_psNcuMcsPerform*)ptMediaProcThreadPara->ptMcsStatPointer;
    MCS_LOC_STAT_EX(ptNcuPerform, qwCalcBandWithByTrafficInfo, 1);

   T_NcuTrafficReportInfo *ptTrafficReportInfo = (T_NcuTrafficReportInfo*)ptTrafficData;
    WORD32 dwTrafficLength = ptTrafficReportInfo->dwTrafficLength;
    WORD32 dwReportPktNum = ptTrafficReportInfo->dwReportPktNum;
    NCU_PM_55304_STAT_ADD(qwNcuRcvTrafficRptMsgs, 1);
    
    if (MCS_PKT_DIR_DL == ptTrafficReportInfo->bDir)
    {
        ptStmAddr->dwTotalDlPktBytes += dwTrafficLength;
        ptStmAddr->dwDlPktBytes += dwTrafficLength;
        ptStmAddr->dwTotalDlPktNum += dwReportPktNum;
        ptStmAddr->dwDlPktNum += dwReportPktNum;
        NCU_PM_55304_STAT_ADD(qwNcuRcvDlTrafficRptPktsNum, dwReportPktNum);
        NCU_PM_55304_STAT_ADD(qwNcuRcvDlTrafficRptPktsBytes, dwTrafficLength);
        MCS_LOC_STAT_EX(ptNcuPerform, qwRcvTrafficRptDlNum, dwReportPktNum);
        MCS_LOC_STAT_EX(ptNcuPerform, qwRcvTrafficRptDlBytes, dwTrafficLength);
    }
    else
    {
        ptStmAddr->dwTotalUlPktBytes += dwTrafficLength;
        ptStmAddr->dwUlPktBytes += dwTrafficLength;
        ptStmAddr->dwTotalUlPktNum += dwReportPktNum;
        ptStmAddr->dwUlPktNum += dwReportPktNum;
        NCU_PM_55304_STAT_ADD(qwNcuRcvUlTrafficRptPktsNum, dwReportPktNum);
        NCU_PM_55304_STAT_ADD(qwNcuRcvUlTrafficRptPktsBytes, dwTrafficLength);
        MCS_LOC_STAT_EX(ptNcuPerform, qwRcvTrafficRptUlNum, dwReportPktNum);
        MCS_LOC_STAT_EX(ptNcuPerform, qwRcvTrafficRptUlBytes, dwTrafficLength);
    }
}

VOID psNcuCalcPktRtt(T_MediaProcThreadPara *ptMediaProcThreadPara, T_psNcuFlowCtx *ptStmAddr, VOID* ptPktData)
{
    MCS_CHK_NULL_VOID(ptStmAddr);
    MCS_CHK_NULL_VOID(ptPktData);
    MCS_CHK_NULL_VOID(ptMediaProcThreadPara);
    MCS_CHK_NULL_VOID(ptMediaProcThreadPara->ptMcsStatPointer);


    T_psNcuMcsPerform* ptNcuPerform = (T_psNcuMcsPerform*)ptMediaProcThreadPara->ptMcsStatPointer;
    if (MCS_TCP_PROTOCOL != ptStmAddr->bProType)
    {
        DEBUG_TRACE(DEBUG_HIG, "func:%s, not tcp pkt,cur pro = %u.\n", __func__, ptStmAddr->bProType);
        MCS_LOC_STAT_EX(ptNcuPerform, qwNotTcpPkt, 1);
        return ;
    }

    MCS_LOC_STAT_EX(ptNcuPerform, qwCalcPktRtt, 1);
    WORD32 hDB = ptMediaProcThreadPara->dwhDBByThreadNo ;
    T_NcuPktInfoHead* ptPktHead = (T_NcuPktInfoHead*)ptPktData;
    T_psNcuPktDesc *ptPktDesc = &ptMediaProcThreadPara->tPktDesc;

    WORD32 dwSeqNum = ptPktDesc->dwSeqNumber;
    WORD32 dwAckNum = ptPktDesc->dwAckNumber;
    BYTE   bDir    = ptPktDesc->bDir;
    BYTE   bAckDir = ptPktDesc->bDir ^ MCS_PKT_DIR_DL;
    WORD32 dwFlowId  = ptStmAddr->dwFlowID;
    WORD16 dwPayLoad = ptPktDesc->wPayloadSize;
    WORD64 ddwCurTics = ptPktHead->dwTimestamp;
    DEBUG_TRACE(DEBUG_LOW, "dwFlowId = %u, pktDir = %u, bAckDir = %u, dwSeqNum = %u, dwAckNum = %u, dwPayLoad = %u, ddwCurTics = %llu.\n",
                        dwFlowId, ptPktHead->bDir, bAckDir, dwSeqNum, dwAckNum, dwPayLoad, ddwCurTics);

    if (MCS_PKT_DIR_DL == ptPktHead->bDir)
    {
        ptStmAddr->dwTotalDlTcpPktNum ++;
        ptStmAddr->dwDlTcpPktNum ++;
        MCS_LOC_STAT_EX(ptNcuPerform, qwRcvDlTcpPktNum, 1);
        if (ptStmAddr->wSnUlNum)
        {
            psVpfuMcsDACalcUlRTTRelItem(ptStmAddr, ptNcuPerform, dwAckNum, hDB, ddwCurTics);
        }
    }
    else
    {
        ptStmAddr->dwTotalUlTcpPktNum ++;
        ptStmAddr->dwUlTcpPktNum ++;
        MCS_LOC_STAT_EX(ptNcuPerform, qwRcvUlTcpPktNum, 1);
        if (ptStmAddr->wSnDlNum)
        {
            psVpfuMcsDACalcDlRTTRelItem(ptStmAddr, ptNcuPerform, dwAckNum, hDB, ddwCurTics);
        }
    }

    if (unlikely(psNcuIsPktHasSynAckFlg(ptPktDesc)))
    {
        MCS_LOC_STAT_EX(ptNcuPerform, qwRcvSynFinRstPkt, 1);
        if (dwPayLoad > 0)
        {
            MCS_LOC_STAT_EX(ptNcuPerform, qwRcvSynFinRstPktWithPayLoad, 1);
        }
        return ;
    }

    /* payload > 0 add item */
    if  (dwPayLoad > 0)
    {
        /* search ptNcuSnCtx table, if match not create new item, not update timestamp */
        T_NcuMcsSnCtx *ptNcuSnCtx = psQrySnCtxByFlowIdSeqDir(dwFlowId, dwSeqNum + dwPayLoad, bDir, hDB);
        if (unlikely(NULL != ptNcuSnCtx))
        {
            MCS_LOC_STAT_EX(ptNcuPerform, qwDARetransPktSnCtxExist, 1);

            /* 重传报文当做丢包处理 */
            psNcuCalcPktLoss(ptNcuPerform, ptStmAddr, bDir);
            return;
        }
        if (bDir == MCS_PKT_DIR_DL)
        {
            /* download rtt list action */
            psVpfuMcsDaDlRttListCtl(ptStmAddr, ptNcuPerform, dwSeqNum, dwPayLoad, hDB, ddwCurTics);
        }
        else
        {
            /* upload rtt list action */
            psVpfuMcsDaUlRttListCtl(ptStmAddr, ptNcuPerform, dwSeqNum, dwPayLoad, hDB, ddwCurTics);
        }
    }
    return;
}

VOID psVpfuMcsDaUlRttListCtl(T_psNcuFlowCtx *ptStmAddr, T_psNcuMcsPerform *ptNcuPerform, WORD32 seqNum, WORD32 payLoad, WORD32 dwhDB, WORD64 ddwCurTics)
{
    MCS_CHK_NULL_VOID(ptStmAddr);
    MCS_CHK_NULL_VOID(ptNcuPerform);

    WORD32 ulNum  = ptStmAddr->wSnUlNum;
    /* upload snctx item num overload, release tail item */
    if (ulNum >= MAX_DA_CALC_RTT_ITEM_NUMBER)
    {
        if(MCS_RET_SUCCESS != delUlSnCtxTailItem(ptStmAddr, ptNcuPerform, dwhDB))
        {
            return;
        }
    }

    WORD32 flowID = ptStmAddr->dwFlowID;
    /* create new snctx item for curr pkt */
    T_NcuMcsSnCtx* snCtx = psGetSnCtxByFlowIdSeqDir(flowID, seqNum + payLoad, MCS_PKT_DIR_UL, dwhDB);
    if (NULL == snCtx)
    {
        MCS_LOC_STAT_EX(ptNcuPerform, qwDACreateSnCtxUlFail, 1);
        return;
    }

    MCS_LOC_STAT_EX(ptNcuPerform, qwDACreateSnCtxUlSucc, 1);
    /* init snctx elements */
    snCtx->ddwTimeStamp = ddwCurTics;
    snCtx->dwSeqNum     = seqNum;
    snCtx->wPayLoad     = payLoad;
    snCtx->bDir         = MCS_PKT_DIR_UL;
    DEBUG_TRACE(DEBUG_HIG, "ddwTimeStamp:%llu, flowID:%u, seqNum:%u, payLoad:%u, dir:%u", snCtx->ddwTimeStamp, flowID, seqNum, payLoad, MCS_PKT_DIR_UL);
    /* new element insert to upload deque */
    psVpfuMcsDaUlRttListInsert(ptStmAddr, ptNcuPerform, snCtx);
    return;
}

WORD32 delUlSnCtxTailItem(T_psNcuFlowCtx *ptStmAddr, T_psNcuMcsPerform *ptNcuPerform, WORD32 dwhDB)
{
    MCS_CHK_RET(ptStmAddr, MCS_RET_FAIL);
    MCS_CHK_RET(ptNcuPerform, MCS_RET_FAIL);

    T_NcuMcsSnCtx *snCtxTail = ptStmAddr->ptSnUlTail;
    T_NcuMcsSnCtx *snCtxTailPrev = snCtxTail->ptSnPrev;
    WORD32 snCtxID = snCtxTail->dwCtxID;

    /* use CtxID release tail item */
    if (MCS_RET_SUCCESS != psVpfuMcsDelCtxById(snCtxID, dwhDB, DB_HANDLE_R_NCU_SN))
    {
        MCS_LOC_STAT_EX(ptNcuPerform, qwDAReleaseSnCtxUlTailFail, 1);
        return MCS_RET_FAIL;
    }
    ptStmAddr->ptSnUlTail = snCtxTailPrev;
    snCtxTailPrev->ptSnNext = NULL;
    ptStmAddr->wSnUlNum--;
    MCS_LOC_STAT_EX(ptNcuPerform, qwDAReleaseSnCtxUlTailSucc, 1);

    return MCS_RET_SUCCESS;
}

VOID psVpfuMcsDaDlRttListCtl(T_psNcuFlowCtx *ptStmAddr, T_psNcuMcsPerform *ptNcuPerform, WORD32 seqNum, WORD32 payLoad, WORD32 dwhDB, WORD64 ddwCurTics)
{
    MCS_CHK_NULL_VOID(ptStmAddr);
    MCS_CHK_NULL_VOID(ptNcuPerform);
    WORD32 ulNum = ptStmAddr->wSnDlNum;

    /* download snctx item num overload, release tail item */
    if (unlikely(ulNum >= MAX_DA_CALC_RTT_ITEM_NUMBER))
    {
        if(unlikely(MCS_RET_SUCCESS != delDlSnCtxTailItem(ptStmAddr, ptNcuPerform, dwhDB)))
        {
            return;
        }
    }

    WORD32 flowID = ptStmAddr->dwFlowID;
    /* create new snctx item for curr pkt */
    T_NcuMcsSnCtx* snCtx = psGetSnCtxByFlowIdSeqDir(flowID, seqNum + payLoad, MCS_PKT_DIR_DL, dwhDB);
    if (unlikely(NULL == snCtx))
    {
        MCS_LOC_STAT_EX(ptNcuPerform, qwDACreateSnCtxDlFail, 1);
        return;
    }
    MCS_LOC_STAT_EX(ptNcuPerform, qwDACreateSnCtxDlSucc, 1);

    /* init snctx elements */
    snCtx->ddwTimeStamp = ddwCurTics;
    snCtx->dwSeqNum     = seqNum;
    snCtx->wPayLoad     = payLoad;
    DEBUG_TRACE(DEBUG_HIG, "ddwTimeStamp:%llu, flowID:%u, seqNum:%u, payLoad:%u, dir:%u", snCtx->ddwTimeStamp, flowID, seqNum, payLoad, MCS_PKT_DIR_DL);
    /* new element insert to download deque */
    psVpfuMcsDaDlRttListInsert(ptStmAddr, ptNcuPerform, snCtx);
    return;
}

WORD32 delDlSnCtxTailItem(T_psNcuFlowCtx *ptStmAddr, T_psNcuMcsPerform *ptNcuPerform, WORD32 dwhDB)
{
    MCS_CHK_RET(ptStmAddr, MCS_RET_FAIL);
    MCS_CHK_RET(ptNcuPerform, MCS_RET_FAIL);

    T_NcuMcsSnCtx *snCtxTail = ptStmAddr->ptSnDlTail;
    T_NcuMcsSnCtx *snCtxTailPrev = snCtxTail->ptSnPrev;
    WORD32 snCtxID = snCtxTail->dwCtxID;

    /* use CtxID release tail item */
    if (MCS_RET_SUCCESS != psVpfuMcsDelCtxById(snCtxID, dwhDB, DB_HANDLE_R_NCU_SN))
    {
        MCS_LOC_STAT_EX(ptNcuPerform, qwDAReleaseSnCtxDlTailFail, 1);
        return MCS_RET_FAIL;
    }
    ptStmAddr->ptSnDlTail = snCtxTailPrev;
    snCtxTailPrev->ptSnNext = NULL;
    ptStmAddr->wSnDlNum--;
    MCS_LOC_STAT_EX(ptNcuPerform, qwDAReleaseSnCtxDlTailSucc, 1);

    return MCS_RET_SUCCESS;
}

VOID psVpfuMcsDACalcUlRTTRelItem(T_psNcuFlowCtx *ptStmCtx, T_psNcuMcsPerform *ptNcuPerform, WORD32 dwAckNum, WORD32 dwhDB, WORD64 ddwCurTics)
{
    MCS_PCLINT_NULLPTR_RET_2ARG_VOID(ptStmCtx, ptNcuPerform);
    MCS_PCLINT_NULLPTR_RET_2ARG_VOID(ptStmCtx->ptSnUlTail, ptStmCtx->ptSnUlHead);

    WORD32 dwRttDuration = dwTimeDurationUS;
    MCS_CHK_VOID(dwRttDuration == 0);

    T_NcuMcsSnCtx *targetSnCtx = NULL;
    _mcs_if (MCS_RET_FAIL == psNcuCalcRttFindSnCtx(ptStmCtx, dwAckNum, MCS_PKT_DIR_UL, &targetSnCtx) || NULL == targetSnCtx)
    {
        MCS_LOC_STAT_EX(ptNcuPerform, qwDAExecCalcUlRttNotMatch, 1);
        return ;
    }
    MCS_LOC_STAT_EX(ptNcuPerform, qwDAExecCalcUlRttFindSn, 1);

    WORD32 rtt;
    WORD32 snCtxID;
    T_NcuMcsSnCtx *snCtxPrevTmp = targetSnCtx->ptSnPrev;
    T_NcuMcsSnCtx *snCtxNextTmp = ptStmCtx->ptSnUlTail;
    T_NcuMcsSnCtx *snCtxTmp = ptStmCtx->ptSnUlTail;
    WORD64 curTicks = ddwCurTics;
    WORD32 loop = MIN(ptStmCtx->wSnUlNum, MAX_DA_CALC_RTT_ITEM_NUMBER);
    while(snCtxNextTmp != NULL && snCtxNextTmp != snCtxPrevTmp && loop)
    {
        rtt = curTicks > snCtxNextTmp->ddwTimeStamp ? (curTicks - snCtxNextTmp->ddwTimeStamp) / dwRttDuration /1000 : 0; /* ms */
        ptStmCtx->dwUlPktRtt += rtt;
        ptStmCtx->dwTotalUlPktRtt += rtt;
        ptStmCtx->dwUlRttPktNum++;
        ptStmCtx->dwTotalUlRttPktNum++;

        /* release acked item */
        snCtxID = snCtxNextTmp->dwCtxID;
        snCtxTmp = snCtxNextTmp;
        snCtxNextTmp = snCtxNextTmp->ptSnPrev;
        DEBUG_TRACE(DEBUG_HIG, "\n[Mcs]DACalcRTT Ul Del Ctx rtt = %u UlPktNum:%u UlRtt:%u snCtxID:%u\n",
                    rtt, ptStmCtx->dwUlRttPktNum, ptStmCtx->dwTotalUlPktRtt, snCtxID);

        if(MCS_RET_SUCCESS != psVpfuMcsDelCtxById(snCtxID, dwhDB, DB_HANDLE_R_NCU_SN))
        {
            ptStmCtx->ptSnUlTail = snCtxTmp;
            snCtxTmp->ptSnNext = NULL;
            return;
        }
        ptStmCtx->wSnUlNum--;
        loop--;
        MCS_LOC_STAT_EX(ptNcuPerform, qwDAExecCalcUlRtt, 1);
    }

    if(ptStmCtx->wSnUlNum == 0)
    {
        ptStmCtx->ptSnUlHead = NULL;
        ptStmCtx->ptSnUlTail = NULL;
    }
    else
    {
        ptStmCtx->ptSnUlTail = snCtxNextTmp;
        if(snCtxNextTmp != NULL)
            snCtxNextTmp->ptSnNext = NULL;
    }
    return;
}

VOID psVpfuMcsDACalcDlRTTRelItem(T_psNcuFlowCtx *ptStmCtx, T_psNcuMcsPerform *ptNcuPerform, WORD32 dwAckNum, WORD32 dwhDB, WORD64 ddwCurTics)
{
    MCS_PCLINT_NULLPTR_RET_2ARG_VOID(ptStmCtx, ptNcuPerform);
    MCS_PCLINT_NULLPTR_RET_2ARG_VOID(ptStmCtx->ptSnDlTail, ptStmCtx->ptSnDlHead);

    WORD32 dwRttDuration = dwTimeDurationUS;
    MCS_CHK_VOID(dwRttDuration == 0);

    T_NcuMcsSnCtx *targetSnCtx = NULL;

    _mcs_if (MCS_RET_FAIL == psNcuCalcRttFindSnCtx(ptStmCtx, dwAckNum, MCS_PKT_DIR_DL, &targetSnCtx) || NULL == targetSnCtx)
    {
        MCS_LOC_STAT_EX(ptNcuPerform, qwDAExecCalcDlRttNotMatch, 1);
        return ;
    }
    MCS_LOC_STAT_EX(ptNcuPerform, qwDAExecCalcDlRttFindSn, 1);

    WORD32 rtt;
    WORD32 snCtxID;
    T_NcuMcsSnCtx *snCtxPrevTmp = targetSnCtx->ptSnPrev;
    T_NcuMcsSnCtx *snCtxNextTmp = ptStmCtx->ptSnDlTail;
    T_NcuMcsSnCtx *snCtxTmp     = ptStmCtx->ptSnDlTail;

    WORD32 loop = MIN(ptStmCtx->wSnDlNum, MAX_DA_CALC_RTT_ITEM_NUMBER);
    WORD64 curTicks = ddwCurTics;
    while(snCtxNextTmp != NULL && snCtxNextTmp != snCtxPrevTmp && loop)
    {
        rtt = curTicks > snCtxNextTmp->ddwTimeStamp ? (curTicks - snCtxNextTmp->ddwTimeStamp) / dwRttDuration /1000 : 0; /* ms */
        ptStmCtx->dwDlPktRtt += rtt;
        ptStmCtx->dwTotalDlPktRtt += rtt;
        ptStmCtx->dwDlRttPktNum++;
        ptStmCtx->dwTotalDlRttPktNum++;

        /* release acked item */
        snCtxID = snCtxNextTmp->dwCtxID;
        snCtxTmp = snCtxNextTmp;
        snCtxNextTmp = snCtxNextTmp->ptSnPrev;
        DEBUG_TRACE(DEBUG_HIG, "\n[Mcs]DACalcRTT Dl Del Ctx rtt = %u DlPktNum:%u DlRtt:%u snCtxID:%u\n", 
                    rtt, ptStmCtx->dwDlRttPktNum, ptStmCtx->dwTotalDlPktRtt, snCtxID);

        if(MCS_RET_SUCCESS != psVpfuMcsDelCtxById(snCtxID, dwhDB, DB_HANDLE_R_NCU_SN))
        {
            ptStmCtx->ptSnDlTail = snCtxTmp;
            snCtxTmp->ptSnNext = NULL;
            return;
        }
        ptStmCtx->wSnDlNum--;
        MCS_LOC_STAT_EX(ptNcuPerform, qwDAExecCalcDlRtt, 1);
        loop--;
    }

    if(ptStmCtx->wSnDlNum == 0)
    {
        ptStmCtx->ptSnDlHead = NULL;
        ptStmCtx->ptSnDlTail = NULL;
    }
    else
    {
        ptStmCtx->ptSnDlTail = snCtxNextTmp;
        if(snCtxNextTmp != NULL)
            snCtxNextTmp->ptSnNext = NULL;
    }

    return;
}


WORD32 g_dwSnMax = 20;
/* Started by AICoder, pid:258cfof426r7fdd144c00aa0e039674afe69b9ff */
WORD32 psNcuCalcRttFindSnCtx(T_psNcuFlowCtx *ptStmCtx, WORD32 dwAckNum, BYTE bDir, T_NcuMcsSnCtx **ptTargetSnCtx)
{
    MCS_CHK_NULL_RET(ptStmCtx, MCS_RET_FAIL);
    MCS_CHK_NULL_RET(ptTargetSnCtx, MCS_RET_FAIL);

    WORD32 dwSnNum = 0;
    T_NcuMcsSnCtx *snCtxHead = NULL;
    T_NcuMcsSnCtx *snCtxTail = NULL;

    if (MCS_PKT_DIR_DL == bDir)
    {
        dwSnNum = ptStmCtx->wSnDlNum;
        snCtxHead = ptStmCtx->ptSnDlHead;
        snCtxTail = ptStmCtx->ptSnDlTail;
    }
    else
    {
        dwSnNum = ptStmCtx->wSnUlNum;
        snCtxHead = ptStmCtx->ptSnUlHead;
        snCtxTail = ptStmCtx->ptSnUlTail;
    }

    MCS_CHK_NULL_RET(snCtxHead, MCS_RET_FAIL);
    MCS_CHK_NULL_RET(snCtxTail, MCS_RET_FAIL);
    MCS_CHK_RET(dwAckNum > snCtxHead->dwAckNum || dwAckNum < snCtxTail->dwAckNum, MCS_RET_FAIL);

    if (dwSnNum > g_dwSnMax)
    {
        *ptTargetSnCtx = psQrySnCtxByFlowIdSeqDir(ptStmCtx->dwFlowID, dwAckNum, bDir, ptStmCtx->dwhDBByThreadNo);
        return MCS_RET_SUCCESS;
    }

    T_NcuMcsSnCtx *ptNextSnCtx = snCtxHead;
    while (ptNextSnCtx != NULL && ptNextSnCtx->dwAckNum >= dwAckNum)
    {
        if (ptNextSnCtx->dwAckNum == dwAckNum)
        {
            *ptTargetSnCtx = ptNextSnCtx;
            return MCS_RET_SUCCESS;
        }
        ptNextSnCtx = ptNextSnCtx->ptSnNext;
    }

    return MCS_RET_FAIL;
}
/* Ended by AICoder, pid:258cfof426r7fdd144c00aa0e039674afe69b9ff */

VOID psVpfuMcsDaUlRttListInsert(T_psNcuFlowCtx *ptStmCtx, T_psNcuMcsPerform *ptNcuPerform,  T_NcuMcsSnCtx *ptNewSnCtx)
{
    MCS_CHK_NULL_VOID(ptNcuPerform);
    MCS_CHK_NULL_VOID(ptStmCtx);
    MCS_CHK_NULL_VOID(ptNewSnCtx);
    psNcuSnCtxQuickAge(ptStmCtx, ptNcuPerform, ptNewSnCtx->bDir);
    T_NcuMcsSnCtx *ptHead = ptStmCtx->ptSnUlHead;
    T_NcuMcsSnCtx *ptTail = ptStmCtx->ptSnUlTail;
    if ((NULL != ptHead) && (NULL != ptTail))
    {
        if (ptNewSnCtx->dwSeqNum > ptHead->dwSeqNum) /* new element insert at head */
        {
            ptNewSnCtx->ptSnPrev = NULL;
            ptNewSnCtx->ptSnNext = ptHead;
            ptHead->ptSnPrev = ptNewSnCtx;
            ptStmCtx->ptSnUlHead = ptNewSnCtx;
            MCS_LOC_STAT_EX(ptNcuPerform, qwDAUlRttListInsertHead, 1);
            DEBUG_TRACE(DEBUG_HIG, "\n[Mcs]DACalcRTT head ptHead->dwSeqNum = %u, ptNewSnCtx->dwSeqNum:%u\n",
                        ptHead->dwSeqNum, ptNewSnCtx->dwSeqNum);
        }
        else
        {
            /* 重传和乱序当做丢包 */
            psNcuCalcPktLoss(ptNcuPerform, ptStmCtx, ptNewSnCtx->bDir);
            MCS_CHK_NULL_VOID(g_ptMediaProcThreadPara);
            psVpfuMcsDelCtxById(ptNewSnCtx->dwCtxID, g_ptMediaProcThreadPara->dwhDBByThreadNo, DB_HANDLE_R_NCU_SN);
            MCS_LOC_STAT_EX(ptNcuPerform, qwDAUlRttListNoNeedInsert, 1);
            return ;
        }
        ptStmCtx->wSnUlNum++;
        return;
    }

    /* when ptHead and ptTail is none , head and tail both point to new element */
    ptStmCtx->ptSnUlHead = ptNewSnCtx;
    ptStmCtx->ptSnUlTail = ptNewSnCtx;
    ptNewSnCtx->ptSnNext = NULL;
    ptNewSnCtx->ptSnPrev = NULL;
    ptStmCtx->wSnUlNum++;
    MCS_LOC_STAT_EX(ptNcuPerform, qwDAUlRttListInsertFirstElem, 1);
    return;
}

VOID psVpfuMcsDaDlRttListInsert(T_psNcuFlowCtx *ptStmCtx, T_psNcuMcsPerform *ptNcuPerform, T_NcuMcsSnCtx *ptNewSnCtx)
{
    MCS_CHK_NULL_VOID(ptNcuPerform);
    MCS_CHK_NULL_VOID(ptStmCtx);
    MCS_CHK_NULL_VOID(ptNewSnCtx);
    psNcuSnCtxQuickAge(ptStmCtx, ptNcuPerform, ptNewSnCtx->bDir);
    T_NcuMcsSnCtx *ptHead = ptStmCtx->ptSnDlHead;
    T_NcuMcsSnCtx *ptTail = ptStmCtx->ptSnDlTail;

    if ((NULL != ptHead) && (NULL != ptTail))
    {
        if (ptNewSnCtx->dwSeqNum > ptHead->dwSeqNum) /* new element insert at head */
        {
            ptNewSnCtx->ptSnPrev = NULL;
            ptNewSnCtx->ptSnNext = ptHead;
            ptHead->ptSnPrev = ptNewSnCtx;
            ptStmCtx->ptSnDlHead = ptNewSnCtx;
            MCS_LOC_STAT_EX(ptNcuPerform, qwDADlRttListInsertHead, 1);
            DEBUG_TRACE(DEBUG_HIG, "\n[Mcs]DACalcRTT head ptHead->dwSeqNum = %u, ptNewSnCtx->dwSeqNum:%u\n",
                        ptHead->dwSeqNum, ptNewSnCtx->dwSeqNum);
        }
        else
        {
            /* 重传和乱序当做丢包 */
            psNcuCalcPktLoss(ptNcuPerform, ptStmCtx, ptNewSnCtx->bDir);
            MCS_CHK_NULL_VOID(g_ptMediaProcThreadPara);
            psVpfuMcsDelCtxById(ptNewSnCtx->dwCtxID, g_ptMediaProcThreadPara->dwhDBByThreadNo, DB_HANDLE_R_NCU_SN);
            MCS_LOC_STAT_EX(ptNcuPerform, qwDADlRttListNoNeedInsert, 1);
            return ;
        }
        ptStmCtx->wSnDlNum++;
        return;
    }

    /* when ptHead and ptTail is none , head and tail both point to new element */
    ptStmCtx->ptSnDlHead = ptNewSnCtx;
    ptStmCtx->ptSnDlTail = ptNewSnCtx;
    ptNewSnCtx->ptSnNext = NULL;
    ptNewSnCtx->ptSnPrev = NULL;
    ptStmCtx->wSnDlNum++;
    MCS_LOC_STAT_EX(ptNcuPerform, qwDADlRttListInsertFirstElem, 1);
    return;
}

WORD32 psNcuSnCtxQuickAge(T_psNcuFlowCtx *ptStmCtx, T_psNcuMcsPerform *ptNcuPerform, BYTE bDir)
{
    MCS_CHK_NULL_RET(ptNcuPerform, MCS_RET_FAIL);
    MCS_CHK_NULL_RET(ptStmCtx, MCS_RET_FAIL);

    WORD32 dwSnCtxAgeTime = psNcuGetSnCtxAgeTime();
    _mcs_if (0 == dwSnCtxAgeTime)
    {
        MCS_LOC_STAT_EX(ptNcuPerform, qwSnAgeTimeZero, 1);
        return MCS_RET_FAIL;
    }

    T_NcuMcsSnCtx *ptPrevTmp = MCS_PKT_DIR_UL == bDir ? ptStmCtx->ptSnUlTail : ptStmCtx->ptSnDlTail;
    if (NULL == ptPrevTmp)
    {
        return MCS_RET_SUCCESS;
    }

    WORD32 dwCurPowerOn = psFtmGetPowerOnSec();

    T_NcuMcsSnCtx* ptPrev = NULL;
    WORD32 Loop = MAX_DA_CALC_RTT_ITEM_NUMBER;
    while(NULL != ptPrevTmp && Loop)
    {
        DEBUG_TRACE(DEBUG_HIG, "\n dwCurPowerOn= %u, ptHead->dwCreateTimeStamp:%u, dwSnCtxAgeTime:%u\n",
                        dwCurPowerOn, ptPrevTmp->dwCreateTimeStamp, dwSnCtxAgeTime);
        if (dwCurPowerOn <= ptPrevTmp->dwCreateTimeStamp + dwSnCtxAgeTime)
        {
            MCS_LOC_STAT_EX(ptNcuPerform, qwSnNotReachAgeTime, 1);
            return MCS_RET_SUCCESS;
        }
        ptPrev = ptPrevTmp->ptSnPrev;
        psNcuRmSnCtxList(ptStmCtx, ptPrevTmp, ptNcuPerform);
        psNcuModSnHeadTail(ptStmCtx, ptPrev, bDir);
        if (ptPrev)
        {
            ptPrev->ptSnNext = NULL;
        }
        ptPrevTmp = ptPrev;
        Loop--;
    }
    return MCS_RET_SUCCESS;
}

WORD32 psNcuRmSnCtxList(T_psNcuFlowCtx *ptStmCtx, T_NcuMcsSnCtx *ptCurSnCtx, T_psNcuMcsPerform *ptNcuPerform)
{
    MCS_CHK_NULL_RET(ptCurSnCtx, MCS_RET_FAIL);
    MCS_CHK_NULL_RET(ptStmCtx, MCS_RET_FAIL);
    MCS_CHK_NULL_RET(ptNcuPerform, MCS_RET_FAIL);
    MCS_CHK_NULL_RET(g_ptMediaProcThreadPara, MCS_RET_FAIL);
    WORD32 hDB = g_ptMediaProcThreadPara->dwhDBByThreadNo;
    BYTE  bDir = ptCurSnCtx->bDir;
    if (MCS_PKT_DIR_DL == bDir)
    {
        MCS_LOC_STAT_EX(ptNcuPerform, qwDlSnCtxQuickAgeing, 1);
        if (ptStmCtx->wSnDlNum)
        {
            ptStmCtx->wSnDlNum--;
        }
    }
    else
    {
        MCS_LOC_STAT_EX(ptNcuPerform, qwUlSnCtxQuickAgeing, 1);
        if (ptStmCtx->wSnUlNum)
        {
            ptStmCtx->wSnUlNum--;
        }
    }
    psVpfuMcsDelCtxById(ptCurSnCtx->dwCtxID, hDB, DB_HANDLE_R_NCU_SN);
    return MCS_RET_SUCCESS;
}

VOID psNcuModSnHeadTail(T_psNcuFlowCtx *ptStmCtx, T_NcuMcsSnCtx* ptPrev, BYTE bDir)
{
    MCS_CHK_NULL_VOID(ptStmCtx);
    if (MCS_PKT_DIR_DL == bDir)
    {
        if (ptStmCtx->wSnDlNum == 0)
        {
            ptStmCtx->ptSnDlTail = NULL;
            ptStmCtx->ptSnDlHead = NULL;
        }
        else
        {
            ptStmCtx->ptSnDlTail = ptPrev;
        }
    }
    else
    {
        if (ptStmCtx->wSnUlNum == 0)
        {
            ptStmCtx->ptSnUlTail = NULL;
            ptStmCtx->ptSnUlHead = NULL;
        }
        else
        {
            ptStmCtx->ptSnUlTail = ptPrev;
        }
    }
}

WORD32 psNcuDaRttListDel(T_MediaProcThreadPara *ptMediaProcThreadPara, T_NcuMcsSnCtx *ptSnCtx)
{
    MCS_CHK_NULL_RET(ptSnCtx, MCS_RET_FAIL);
    T_psNcuFlowCtx *ptNcuFlowCtx = NULL;
    WORD32  hDB  = ptMediaProcThreadPara->dwhDBByThreadNo;
    T_NcuMcsSnCtx *ptNext = ptSnCtx->ptSnNext;
    T_NcuMcsSnCtx *ptPrev = ptSnCtx->ptSnPrev;
    ptNcuFlowCtx = psNcuMcsGetStmByID(ptSnCtx->dwFlowId, hDB);
    if(NULL != ptNcuFlowCtx)
    {
        if(MCS_PKT_DIR_UL == ptSnCtx->bDir)
        {
            psNcuDaUlRttListDel(ptNcuFlowCtx, ptSnCtx);
        }
        else
        {
            psNcuDaDlRttListDel(ptNcuFlowCtx, ptSnCtx);
        }
    }
    if (NULL != ptNext)
    {
        ptNext->ptSnPrev = ptPrev;
    }
    if (NULL != ptPrev)
    {
        ptPrev->ptSnNext = ptNext;
    }
    psVpfuMcsDelCtxById(ptSnCtx->dwCtxID, hDB, DB_HANDLE_R_NCU_SN);

    return MCS_RET_SUCCESS;
}

VOID psNcuDaUlRttListDel(T_psNcuFlowCtx *ptNcuFlowCtx, T_NcuMcsSnCtx *ptSnCtx)
{
    MCS_CHK_NULL_VOID(ptNcuFlowCtx);
    MCS_CHK_NULL_VOID(ptSnCtx);
    T_NcuMcsSnCtx *ptNext = ptSnCtx->ptSnNext;
    T_NcuMcsSnCtx *ptPrev = ptSnCtx->ptSnPrev;
    if (NULL == ptNext)
    {
        ptNcuFlowCtx->ptSnUlTail = ptPrev;
    }
    if (NULL == ptPrev)
    {
        ptNcuFlowCtx->ptSnUlHead = ptNext;
    }
    if (0 < ptNcuFlowCtx->wSnUlNum)
    {
        ptNcuFlowCtx->wSnUlNum--;
    }

    return;
}

VOID psNcuDaDlRttListDel(T_psNcuFlowCtx *ptNcuFlowCtx, T_NcuMcsSnCtx *ptSnCtx)
{
    MCS_CHK_NULL_VOID(ptNcuFlowCtx);
    MCS_CHK_NULL_VOID(ptSnCtx);
    T_NcuMcsSnCtx *ptNext = ptSnCtx->ptSnNext;
    T_NcuMcsSnCtx *ptPrev = ptSnCtx->ptSnPrev;
    if (NULL == ptNext)
    {
        ptNcuFlowCtx->ptSnDlTail = ptPrev;
    }
    if (NULL == ptPrev)
    {
        ptNcuFlowCtx->ptSnDlHead = ptNext;
    }
    if (0 < ptNcuFlowCtx->wSnDlNum)
    {
        ptNcuFlowCtx->wSnDlNum--;
    }

    return;
}
