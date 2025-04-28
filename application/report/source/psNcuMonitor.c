#include "ps_mcs_define.h"
#include "psNcuDataAnalysis.h"
#include "psMcsDebug.h"
#include "r_ncu_daprofile.h"
#include "xdb_pfu_com.h"
#include "ps_db_define_ncu.h"
#include "psNcuReportTimer.h"
#include "psNcuReportSBCProc.h"
#include "ps_ncu_session.h"
#include "ps_ncu_typedef.h"

inline WORD32 getStmValue(WORD32* value)
{
    if (NULL == value)
    {
        return 0;
    }
    return *value;
}

WORD32 psNcuGetStreamRttByDir(T_psNcuFlowCtx *ptTmpStream, BYTE bDir)
{
    if (NULL == ptTmpStream)
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
        return (ptTmpStream->dwTotalDlRttPktNum - ptTmpStream->dwLastDlRttPktNum) != 0 ? (ptTmpStream->dwTotalDlPktRtt - ptTmpStream->dwLastDlPktRtt) / (ptTmpStream->dwTotalDlRttPktNum - ptTmpStream->dwLastDlRttPktNum) : 0;
    }
    return (ptTmpStream->dwTotalUlRttPktNum - ptTmpStream->dwLastUlRttPktNum) != 0 ? (ptTmpStream->dwTotalUlPktRtt - ptTmpStream->dwLastUlPktRtt) / (ptTmpStream->dwTotalUlRttPktNum - ptTmpStream->dwLastUlRttPktNum) : 0;
}

// 汇总所有流下的流量
WORD32 psNcuGetAnaSumByAllStm(T_psNcuDaAppCtx *ptDataAppCtx, WORD32 dwIntervalTime)
{
    _DB_STATEMENT_TRUE_RTN_VALUE((NULL == ptDataAppCtx)||(0==dwIntervalTime), FALSE);
    DEBUG_TRACE(DEBUG_LOW,"ptDataAppCtx->dwStreamNum =%u\n", ptDataAppCtx->dwStreamNum );
    T_psNcuFlowCtx* ptTmpStream = (T_psNcuFlowCtx*)ptDataAppCtx->ptFlowCtxHead;
    WORD32 dwLoop = 0;
    T_psNcuFlowCtx* curStm = NULL;
    while(NULL != ptTmpStream)
    {
        dwLoop++;
        if(dwLoop >= 1000)
        {
            break;
        }
        if(ptDataAppCtx != ptTmpStream->ptNcuDaCtx)
        {
            DEBUG_TRACE(DEBUG_LOW,"psNcuGetAnaSumByAllStm get wrong stream!!!\n");
            continue;
        }
        curStm = ptTmpStream;
        // 带宽的计算
        ptDataAppCtx->dwUlTotalThroughput += getStmValue(&ptTmpStream->dwTotalUlPktBytes); 
        ptDataAppCtx->dwDlTotalThroughput += getStmValue(&ptTmpStream->dwTotalDlPktBytes); 
        ptTmpStream->bandwidthUl = (getStmValue(&ptTmpStream->dwTotalUlPktBytes) - getStmValue(&ptTmpStream->dwLastUlPktBytes))*8/(dwIntervalTime);
        ptTmpStream->bandwidthDl = (getStmValue(&ptTmpStream->dwTotalDlPktBytes) - getStmValue(&ptTmpStream->dwLastDlPktBytes))*8/(dwIntervalTime);
        ptTmpStream->dwLastUlPktBytes = ptTmpStream->dwTotalUlPktBytes;
        ptTmpStream->dwLastDlPktBytes = ptTmpStream->dwTotalDlPktBytes;
        // 时延的计算
        ptDataAppCtx->dwUlTotalPktRtt += getStmValue(&ptTmpStream->dwTotalUlPktRtt);
        ptDataAppCtx->dwDlTotalPktRtt += getStmValue(&ptTmpStream->dwTotalDlPktRtt);
        ptDataAppCtx->dwTotalUlRttPktNum += getStmValue(&ptTmpStream->dwTotalUlRttPktNum);
        ptDataAppCtx->dwTotalDlRttPktNum += getStmValue(&ptTmpStream->dwTotalDlRttPktNum);
        ptTmpStream->dwUlRtt = psNcuGetStreamRttByDir(ptTmpStream, MCS_PKT_DIR_UL);
        ptTmpStream->dwDlRtt = psNcuGetStreamRttByDir(ptTmpStream, MCS_PKT_DIR_DL);
        ptTmpStream->dwLastUlPktRtt = ptTmpStream->dwTotalUlPktRtt;
        ptTmpStream->dwLastDlPktRtt = ptTmpStream->dwTotalDlPktRtt;
        ptTmpStream->dwLastUlRttPktNum = ptTmpStream->dwTotalUlRttPktNum;
        ptTmpStream->dwLastDlRttPktNum = ptTmpStream->dwTotalDlRttPktNum;
        // 丢包率的计算
        ptDataAppCtx->dwTotalUlPktLossNum += getStmValue(&ptTmpStream->dwTotalUlPktLossNum);
        ptDataAppCtx->dwTotalDlPktLossNum += getStmValue(&ptTmpStream->dwTotalDlPktLossNum);
        ptDataAppCtx->dwTotalUlPktNum += getStmValue(&ptTmpStream->dwTotalUlPktNum);
        ptDataAppCtx->dwTotalDlPktNum += getStmValue(&ptTmpStream->dwTotalDlPktNum);
        // 单位时间丢包个数与总包数计算，上报需要
        ptTmpStream->dwUlUnitPktLossNum =  ptTmpStream->dwTotalUlPktLossNum - ptTmpStream->dwLastUlPktLossNum;
        ptTmpStream->dwDlUnitPktLossNum =  ptTmpStream->dwTotalDlPktLossNum - ptTmpStream->dwLastDlPktLossNum;
        ptTmpStream->dwTotalUlUnitPktNum = ptTmpStream->dwTotalUlPktNum - ptTmpStream->dwLastUlPktNum;
        ptTmpStream->dwTotalDlUnitPktNum = ptTmpStream->dwTotalDlPktNum - ptTmpStream->dwLastDlPktNum;
        // 丢包率计算
        ptTmpStream->dwUlPktLossRate = (ptTmpStream->dwTotalUlUnitPktNum) != 0 ? (ptTmpStream->dwUlUnitPktLossNum)*10000 / (ptTmpStream->dwTotalUlUnitPktNum) : 0;
        ptTmpStream->dwDlPktLossRate = (ptTmpStream->dwTotalDlUnitPktNum) != 0 ? (ptTmpStream->dwDlUnitPktLossNum)*10000 / (ptTmpStream->dwTotalDlUnitPktNum) : 0;
        
        ptTmpStream->dwLastUlPktLossNum = ptTmpStream->dwTotalUlPktLossNum;
        ptTmpStream->dwLastDlPktLossNum = ptTmpStream->dwTotalDlPktLossNum;
        ptTmpStream->dwLastUlPktNum = ptTmpStream->dwTotalUlPktNum;
        ptTmpStream->dwLastDlPktNum = ptTmpStream->dwTotalDlPktNum;

        ptTmpStream =  (T_psNcuFlowCtx*)curStm->ptNcuFlowAppNext;
    }
    ptDataAppCtx->dwUlTotalThroughput += ptDataAppCtx->dwDelTotalUlPktBytes;
    ptDataAppCtx->dwDlTotalThroughput += ptDataAppCtx->dwDelTotalDlPktBytes;

    ptDataAppCtx->dwUlTotalPktRtt    += ptDataAppCtx->dwDelTotalUlPktRtt;
    ptDataAppCtx->dwDlTotalPktRtt    += ptDataAppCtx->dwDelTotalDlPktRtt;
    ptDataAppCtx->dwTotalUlRttPktNum += ptDataAppCtx->dwDelTotalUlRttPktNum;
    ptDataAppCtx->dwTotalDlRttPktNum += ptDataAppCtx->dwDelTotalDlRttPktNum;

    ptDataAppCtx->dwTotalUlPktLossNum += ptDataAppCtx->dwDelTotalUlPktLossNum;
    ptDataAppCtx->dwTotalDlPktLossNum += ptDataAppCtx->dwDelTotalDlPktLossNum;
    ptDataAppCtx->dwTotalUlPktNum     += ptDataAppCtx->dwDelTotalUlPktNum;
    ptDataAppCtx->dwTotalDlPktNum     += ptDataAppCtx->dwDelTotalDlPktNum;
    return MCS_RET_SUCCESS;
}


void psNcuClrDataAppTotaldata(T_psNcuDaAppCtx * ptDataAppCtx)
{
    if(NULL == ptDataAppCtx )
    {
        DEBUG_TRACE(DEBUG_LOW,"psNcuClrDataAppTotaldata input NULL\n");
        return;
    }
    ptDataAppCtx->dwUlTotalThroughput = 0;
    ptDataAppCtx->dwDlTotalThroughput = 0;
    ptDataAppCtx->dwUlTotalPktRtt = 0;
    ptDataAppCtx->dwDlTotalPktRtt = 0;
    ptDataAppCtx->dwTotalUlRttPktNum = 0;
    ptDataAppCtx->dwTotalDlRttPktNum = 0;
    ptDataAppCtx->dwTotalUlPktLossNum = 0;
    ptDataAppCtx->dwTotalDlPktLossNum = 0;
    ptDataAppCtx->dwTotalUlPktNum  = 0;
    ptDataAppCtx->dwTotalDlPktNum  = 0;
    // 清空当前粒度 删流的信息
    ptDataAppCtx->dwDelTotalUlPktNum     =0;
    ptDataAppCtx->dwDelTotalDlPktNum     =0;
    ptDataAppCtx->dwDelTotalUlPktBytes   =0;
    ptDataAppCtx->dwDelTotalDlPktBytes   =0;
    ptDataAppCtx->dwDelTotalUlPktLossNum =0;
    ptDataAppCtx->dwDelTotalDlPktLossNum =0;
    ptDataAppCtx->dwDelTotalUlPktRtt     =0;
    ptDataAppCtx->dwDelTotalDlPktRtt     =0;
    ptDataAppCtx->dwDelTotalUlRttPktNum  =0;
    ptDataAppCtx->dwDelTotalDlRttPktNum  =0;
    return;
}

void psNcuGetDataAppLastdata(T_psNcuDaAppCtx * ptDataAppCtx)
{
    if(NULL == ptDataAppCtx )
    {
        DEBUG_TRACE(DEBUG_LOW,"psNcuGetDataAppLastdata input NULL\n");
        return;
    }
    ptDataAppCtx->dwUlLastThroughput = ptDataAppCtx->dwUlTotalThroughput - ptDataAppCtx->dwDelTotalUlPktBytes;
    ptDataAppCtx->dwDlLastThroughput = ptDataAppCtx->dwDlTotalThroughput - ptDataAppCtx->dwDelTotalDlPktBytes ;

    ptDataAppCtx->dwUlLastlPktRtt = ptDataAppCtx->dwUlTotalPktRtt - ptDataAppCtx->dwDelTotalUlPktRtt;
    ptDataAppCtx->dwDlLastlPktRtt = ptDataAppCtx->dwDlTotalPktRtt - ptDataAppCtx->dwDelTotalDlPktRtt;
    ptDataAppCtx->dwLastUlRttPktNum = ptDataAppCtx->dwTotalUlRttPktNum - ptDataAppCtx->dwDelTotalUlRttPktNum;
    ptDataAppCtx->dwLastDlRttPktNum = ptDataAppCtx->dwTotalDlRttPktNum - ptDataAppCtx->dwDelTotalDlRttPktNum;

    ptDataAppCtx->dwLastUlPktLossNum = ptDataAppCtx->dwTotalUlPktLossNum - ptDataAppCtx->dwDelTotalUlPktLossNum;
    ptDataAppCtx->dwLastDlPktLossNum = ptDataAppCtx->dwTotalDlPktLossNum - ptDataAppCtx->dwDelTotalDlPktLossNum;
    ptDataAppCtx->dwLastUlPktNum = ptDataAppCtx->dwTotalUlPktNum - ptDataAppCtx->dwDelTotalUlPktNum;
    ptDataAppCtx->dwLastDlPktNum = ptDataAppCtx->dwTotalDlPktNum - ptDataAppCtx->dwDelTotalDlPktNum;
    return;
}

WORD32 psNcuGetQosAnaRttByDir(T_psNcuDaAppCtx *ptDataAppCtx, BYTE bDir)
{
    if (NULL == ptDataAppCtx)
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
        return (ptDataAppCtx->dwTotalDlRttPktNum - ptDataAppCtx->dwLastDlRttPktNum) != 0 ? (ptDataAppCtx->dwDlTotalPktRtt - ptDataAppCtx->dwDlLastlPktRtt) / (ptDataAppCtx->dwTotalDlRttPktNum - ptDataAppCtx->dwLastDlRttPktNum) : 0;
    }
    return (ptDataAppCtx->dwTotalUlRttPktNum - ptDataAppCtx->dwLastUlRttPktNum) != 0 ? (ptDataAppCtx->dwUlTotalPktRtt - ptDataAppCtx->dwUlLastlPktRtt) / (ptDataAppCtx->dwTotalUlRttPktNum - ptDataAppCtx->dwLastUlRttPktNum) : 0;
}

// 判断当前业务是否质差
DBBOOL psVpfuMcsDAPoorQuality(T_psNcuDaAppCtx *ptDataAppCtx, T_psNcuMcsPerform *ptNcuPerform)
{   
    if(NULL == ptDataAppCtx || NULL == ptNcuPerform )
    {
        DEBUG_TRACE(DEBUG_LOW,"psVpfuMcsDAPoorQuality input NULL\n");
        return FALSE;
    }
    T_VpfuDAProfileCtx *ptDAProfileTuple = NULL;
    ptDAProfileTuple = (T_VpfuDAProfileCtx *)ptDataAppCtx->ptDAProfileTuple;
    _DB_STATEMENT_TRUE_RTN_VALUE((NULL == ptDAProfileTuple), FALSE);
    
    DBBOOL bflag = FALSE;
    WORD32 dwulbw = 0;
    WORD32 dwdlbw = 0;
    WORD64 dwUlThroughput = 0;
    WORD64 dwDlThroughput = 0;
    WORD32 dwIntervalTime = ptDAProfileTuple->detecttime;
    _DB_STATEMENT_TRUE_RTN_VALUE((0 == dwIntervalTime), FALSE);

    psNcuGetAnaSumByAllStm(ptDataAppCtx, dwIntervalTime);
    dwUlThroughput = ptDataAppCtx->dwUlTotalThroughput - ptDataAppCtx->dwUlLastThroughput;
    dwDlThroughput = ptDataAppCtx->dwDlTotalThroughput - ptDataAppCtx->dwDlLastThroughput;
    

    if(dwIntervalTime)
    {
        dwulbw = dwUlThroughput*8/(dwIntervalTime);
        dwdlbw = dwDlThroughput*8/(dwIntervalTime);
    }
    WORD32 dwUlRtt = psNcuGetQosAnaRttByDir(ptDataAppCtx, MCS_PKT_DIR_UL);
    WORD32 dwDlRtt = psNcuGetQosAnaRttByDir(ptDataAppCtx, MCS_PKT_DIR_DL);
    ptDataAppCtx->dwUlUnitBw = dwulbw;
    ptDataAppCtx->dwDlUnitBw = dwdlbw;
    ptDataAppCtx->dwUlUnitRtt = dwUlRtt;
    ptDataAppCtx->dwDlUnitRtt = dwDlRtt;

    ptDataAppCtx->dwUlUnitPktLossNum = ptDataAppCtx->dwTotalUlPktLossNum - ptDataAppCtx->dwLastUlPktLossNum;
    ptDataAppCtx->dwDlUnitPktLossNum = ptDataAppCtx->dwTotalDlPktLossNum - ptDataAppCtx->dwLastDlPktLossNum;
    ptDataAppCtx->dwUlUnitPktNum = ptDataAppCtx->dwTotalUlPktNum - ptDataAppCtx->dwLastUlPktNum;
    ptDataAppCtx->dwDlUnitPktNum = ptDataAppCtx->dwTotalDlPktNum - ptDataAppCtx->dwLastDlPktNum;
    WORD16 wUlPktLossRate = (WORD16)(ptDataAppCtx->dwUlUnitPktNum != 0 ? (ptDataAppCtx->dwUlUnitPktLossNum)*10000 / (ptDataAppCtx->dwUlUnitPktNum) : 0);
    WORD16 wDlPktLossRate = (WORD16)((ptDataAppCtx->dwDlUnitPktNum) != 0 ? (ptDataAppCtx->dwDlUnitPktLossNum) *10000/ (ptDataAppCtx->dwDlUnitPktNum) : 0);

    DEBUG_TRACE(DEBUG_LOW, "\n-------------------------------------------------\n");
    DEBUG_TRACE(DEBUG_LOW, "\n[Mcs] dwbw Print \n ulTotal = %llu, ulLast = %llu, dlTotal = %llu, dlLast = %llu,\n",
                ptDataAppCtx->dwUlTotalThroughput, ptDataAppCtx->dwUlTotalThroughput, ptDataAppCtx->dwDlTotalThroughput, ptDataAppCtx->dwDlLastThroughput);
    
    DEBUG_TRACE(DEBUG_LOW, "\n[Mcs] dwulRTT Print \n ulPktNumRttTotal = %u, ulPktNumLast = %u, ulPktRTTTotal = %u, ulPktRttLast = %u,\n",
                ptDataAppCtx->dwTotalUlRttPktNum, ptDataAppCtx->dwLastUlRttPktNum, ptDataAppCtx->dwUlTotalPktRtt, ptDataAppCtx->dwUlLastlPktRtt);

    DEBUG_TRACE(DEBUG_LOW, "\n[Mcs] dwdlRTT Print \n dlPktNumRttTotal = %u, dlPktNumLast = %u, dlPktRTTTotal = %u, dlPktRttLast = %u,\n",
                ptDataAppCtx->dwTotalDlRttPktNum, ptDataAppCtx->dwLastDlRttPktNum, ptDataAppCtx->dwDlTotalPktRtt, ptDataAppCtx->dwDlLastlPktRtt);

    DEBUG_TRACE(DEBUG_LOW, "\n[Mcs] dwulLossRate Print \n ulPktNumTotal = %u, ulPktNumLast = %u, ulPktLossTotal = %u, ulPktLossLast = %u,\n",
                ptDataAppCtx->dwTotalUlPktNum, ptDataAppCtx->dwLastUlPktNum, ptDataAppCtx->dwTotalUlPktLossNum, ptDataAppCtx->dwLastUlPktLossNum);

    DEBUG_TRACE(DEBUG_LOW, "\n[Mcs] dwdlLossRate Print \n dlPktNumTotal = %u, dlPktNumLast = %u, dlPktRTTTotal = %u, dlPktRttLast = %u,\n",
                ptDataAppCtx->dwTotalDlPktNum, ptDataAppCtx->dwLastDlPktNum, ptDataAppCtx->dwTotalDlPktLossNum, ptDataAppCtx->dwLastDlPktLossNum);

    DEBUG_TRACE(DEBUG_LOW, "\n-------------------------------------------------\n");
    DEBUG_TRACE(DEBUG_LOW, "\n[Mcs] ptDataAppCtx->upseid = 0x%llx\n, dwulbw = %u,dwdlbw = %u,ulbwthreshold = %u,dlbwthreshold = %u\n",
                ptDataAppCtx->ddwUPSeid, dwulbw, dwdlbw, ptDAProfileTuple->ulbwthreshold, ptDAProfileTuple->dlbwthreshold);

    DEBUG_TRACE(DEBUG_LOW, "\n[Mcs] dwUlRtt = %u, dwDlRtt = %u, ulRttThreshold = %u, dlRttThreshold = %u\n",
                dwUlRtt, dwDlRtt, ptDAProfileTuple->andelaythreshold, ptDAProfileTuple->dndelaythreshold);
    
    DEBUG_TRACE(DEBUG_LOW, "\n[Mcs] dwUlLossRate = 0x%u, dwDlLossRate = %u, ulplrthreshold = %u, dlplrthreshold = %u\n",
                wUlPktLossRate, wDlPktLossRate, ptDAProfileTuple->ulplrthreshold, ptDAProfileTuple->dlplrthreshold);
    psNcuGetDataAppLastdata(ptDataAppCtx);
    psNcuClrDataAppTotaldata(ptDataAppCtx);
    if (ptDAProfileTuple->ulbwswitch  && dwulbw < ptDAProfileTuple->ulbwthreshold*1024)
    {
        if(DA_QUALITY_POOR==ptDataAppCtx->bCurQoe || dwulbw >= ptDAProfileTuple->bwzerothreshold*1024)
        {
            DEBUG_TRACE(DEBUG_LOW, "\ndwulbw poor bflag is true\n");
            MCS_LOC_STAT_EX(ptNcuPerform, qwQuePoorByUlbw, 1);    
            bflag = TRUE;
        }  
    }
    if(ptDAProfileTuple->dlbwswitch  && dwdlbw < ptDAProfileTuple->dlbwthreshold*1024)
    {
        if(DA_QUALITY_POOR==ptDataAppCtx->bCurQoe || dwdlbw >= ptDAProfileTuple->bwzerothreshold*1024)
        {
            MCS_LOC_STAT_EX(ptNcuPerform, qwQuePoorByDlbw, 1);
            DEBUG_TRACE(DEBUG_LOW,"\ndwdlbw poor bflag is true\n");
            bflag = TRUE;
        }  
    }
    if(ptDAProfileTuple->andelayswitch && dwUlRtt > 0 && dwUlRtt> ptDAProfileTuple->andelaythreshold)
    {
        MCS_LOC_STAT_EX(ptNcuPerform, qwQuePoorByAnDelay, 1);
        DEBUG_TRACE(DEBUG_LOW,"\ndwUlRtt poor bflag is true\n");
        bflag = TRUE;
    }
    if(ptDAProfileTuple->dndelayswitch && dwDlRtt > 0 && dwDlRtt> ptDAProfileTuple->dndelaythreshold)
    {
        MCS_LOC_STAT_EX(ptNcuPerform, qwQuePoorByDnDelay, 1);
        DEBUG_TRACE(DEBUG_LOW,"\ndwDlRtt poor bflag is true\n");
        bflag = TRUE;
    }
    if(ptDAProfileTuple->ulplrswitch && wUlPktLossRate > 0 && wUlPktLossRate> ptDAProfileTuple->ulplrthreshold)
    {
        MCS_LOC_STAT_EX(ptNcuPerform, qwQuePoorByUlPlr, 1);
        DEBUG_TRACE(DEBUG_LOW,"\nwUlPktLossRate poor bflag is true\n");
        bflag = TRUE;
    }
    if(ptDAProfileTuple->dlplrswitch && wDlPktLossRate > 0 && wDlPktLossRate> ptDAProfileTuple->dlplrthreshold)
    {
        MCS_LOC_STAT_EX(ptNcuPerform, qwQuePoorByDlPlr, 1);
        DEBUG_TRACE(DEBUG_LOW,"\nDlPktLossRate poor bflag is true\n");
        bflag = TRUE;
    }
    return bflag;
}

// 判断当前是否质差且状态变化
DBBOOL psVpfuMcsDAIsStatusChanged(T_psNcuDaAppCtx *ptDataAppCtx, BYTE *ptCurQoe)
{
    DEBUG_TRACE(DEBUG_LOW, "psVpfuMcsDAIsStatusChanged enter!!\n");
    T_psNcuMcsPerform *ptNcuPerform = psGetPerform();
    if(NULL == ptNcuPerform)
    {
        DEBUG_TRACE(DEBUG_LOW,"psVpfuMcsDAIsStatusChanged get ptNcuPerform fail!!\n");
        return FALSE;
    }
    _DB_STATEMENT_TRUE_RTN_VALUE(NULL == ptDataAppCtx,FALSE);
    _DB_STATEMENT_TRUE_RTN_VALUE(NULL == ptCurQoe,FALSE);

    T_VpfuDAProfileCtx *ptDAProfileTuple = NULL;
    ptDAProfileTuple = (T_VpfuDAProfileCtx *)ptDataAppCtx->ptDAProfileTuple;
    _DB_STATEMENT_TRUE_RTN_VALUE(NULL == ptDAProfileTuple,FALSE);
    DBBOOL bflag = FALSE;
    if(psVpfuMcsDAPoorQuality(ptDataAppCtx, ptNcuPerform))
    {
        if(0 == ptDataAppCtx->bHasPoor || (1 == ptDataAppCtx->bHasPoor && DA_QUALITY_GOOD == ptDataAppCtx->bCurQoe))
        {
            ptDataAppCtx->bHasPoorNum++;
            DEBUG_TRACE(DEBUG_LOW,"\npsVpfuMcsDAIsStatusChanged bHasPoorNum is %u, checkpoortimes is %u\n",ptDataAppCtx->bHasPoorNum, ptDAProfileTuple->checkpoortimes);
            if(ptDataAppCtx->bHasPoorNum >= ptDAProfileTuple->checkpoortimes)
            {   
                DEBUG_TRACE(DEBUG_LOW,"\npsVpfuMcsDAIsStatusChanged GOOD TO POOR bHasPoorNum is %u, checkpoortimes is %u\n",ptDataAppCtx->bHasPoorNum, ptDAProfileTuple->checkpoortimes);
                bflag = TRUE;
                ptDataAppCtx->bHasPoorNum = 0;
                ptDataAppCtx->bHasPoor = 1;
                ptDataAppCtx->bCurQoe = DA_QUALITY_POOR;
                MCS_LOC_STAT_EX(ptNcuPerform, qwChkCurQuePoor, 1);
            }
            else
            {
                ptDataAppCtx->bCurQoe = DA_QUALITY_GOOD;
            }
        }
        else
        {
            ptDataAppCtx->bHasGoodNum = 0;
        }
    }
    else
    {
        if(1 == ptDataAppCtx->bHasPoor && DA_QUALITY_POOR == ptDataAppCtx->bCurQoe)
        {
            ptDataAppCtx->bHasGoodNum++;
            DEBUG_TRACE(DEBUG_LOW,"\npsVpfuMcsDAIsStatusChanged  bHasGoodNum is %u, checkgoodtimes is %u\n",ptDataAppCtx->bHasGoodNum, ptDAProfileTuple->checkgoodtimes);
            if(ptDataAppCtx->bHasGoodNum >= ptDAProfileTuple->checkgoodtimes)
            {
                DEBUG_TRACE(DEBUG_LOW,"\npsVpfuMcsDAIsStatusChanged POOR TO GOOD bHasGoodNum is %u, checkgoodtimes is %u\n",ptDataAppCtx->bHasGoodNum, ptDAProfileTuple->checkgoodtimes);
                bflag = TRUE;
                ptDataAppCtx->bHasGoodNum = 0;
                ptDataAppCtx->bCurQoe = DA_QUALITY_GOOD;
                MCS_LOC_STAT_EX(ptNcuPerform, qwChkCurQueGood, 1);
            }
        }
        else
        {
            ptDataAppCtx->bHasPoorNum = 0;
        }
    }
    *ptCurQoe = ptDataAppCtx->bCurQoe;
    return bflag; 
}

// 监控开始入口函数
VOID psVpfuMcsDAQualityMonitor(WORD16 threadID, WORD64 clockStep)
{
    WORD32 dwhDB = _NCU_GET_DBHANDLE(threadID);
    MCS_DM_QUERYDYNDATA_ACK *ptMcsDynCtxNoUniqAck = &g_ptVpfuShMemVar->tGwGloData.atMcsDynCtxNoUniqAck[threadID];
    WORD32 dwReportNum = psVpfuMcsGetAllDaAppCtxByClockStep(clockStep, dwhDB, DB_HANDLE_IDX_R_NCU_DATAAPP_ANA_MON_CLOCK_STEP, (void*)ptMcsDynCtxNoUniqAck);
    WORD32 dwIndex = 0;
    WORD32 dwCtxId = 0;
    T_psNcuDaAppCtx * ptDataAppCtx = NULL;
    T_VpfuDAProfileCtx *ptDAProfileTuple = NULL;
    WORD32 reporttime = 0;
    BYTE bCurQoe = 0;
    T_psNcuMcsPerform *ptNcuPerform = psGetPerform();
    if(NULL== ptNcuPerform)
    {
        DEBUG_TRACE(DEBUG_LOW,"psVpfuMcsDAQualityMonitor get ptNcuPerform fail!!\n");
        return;
    }

    for(dwIndex=0;dwIndex <EXPECT_DATA_APP_NUM && dwIndex<dwReportNum;dwIndex++)
    {
        DEBUG_TRACE(DEBUG_LOW, "psVpfuMcsDAQualityMonitor get dwReportNum is %d!!,CURindex is %d\n", dwReportNum, dwIndex);
        dwCtxId = ptMcsDynCtxNoUniqAck->adwDataArea[dwIndex];
        ptDataAppCtx = psMcsGetDaAppCtxById(dwCtxId, dwhDB);
        MCS_CHK_CONTINUE(NULL == ptDataAppCtx);
        DEBUG_TRACE(DEBUG_ERR,"psVpfuMcsDAQualityMonitor get ptDataAppCtx By CtxId Succ!!\n");

        ptDAProfileTuple = (T_VpfuDAProfileCtx *)ptDataAppCtx->ptDAProfileTuple;
        _DB_STATEMENT_TRUE_CONTINUE(NULL == ptDAProfileTuple);
        ptDataAppCtx->ddwAnaMonClockStep = milliSecToClockStep(ptDAProfileTuple->detecttime*1000);

        if(psVpfuMcsDAIsStatusChanged(ptDataAppCtx,&bCurQoe) && ((!ptDataAppCtx->bIsDialFlg)))
        {
            if(DA_QUALITY_POOR == bCurQoe)
            {
                reporttime = ptDAProfileTuple->reportpoortime;
                ptDataAppCtx->ddwAnaRepClockStep = milliSecToClockStep(reporttime*1000);
                // 此处增加上报接口
                psNcuReportToUpmProc(ptDataAppCtx, NULL, threadID, RPT_TYPE_ANA);
                MCS_LOC_STAT_EX(ptNcuPerform, qwReportToUpmByMonitorPoor, 1);
                DEBUG_TRACE(DEBUG_LOW,"psVpfuMcsDAQualityMonitor ddwReportTime is %u, timestamp is %llu\n",reporttime, clockStep);
                psNcuMcsUpdDaAppCtxByClockStep(clockStep + ptDataAppCtx->ddwAnaRepClockStep, DB_HANDLE_IDX_R_NCU_DATAAPP_ANA_CLOCK_STEP,dwCtxId, dwhDB, enumUpdateIndex);
            }
            else if(DA_QUALITY_GOOD == bCurQoe)
            {
                // 质优上报一次 后续不上报
                psNcuReportToUpmProc(ptDataAppCtx, NULL, threadID, RPT_TYPE_ANA);
                MCS_LOC_STAT_EX(ptNcuPerform, qwReportToUpmByMonitorGood, 1);
                psNcuMcsUpdDaAppCtxByClockStep(0, DB_HANDLE_IDX_R_NCU_DATAAPP_ANA_CLOCK_STEP, dwCtxId, dwhDB, enumDeleteIndex);
            }
        }
        psNcuMcsUpdDaAppCtxByClockStep(clockStep + ptDataAppCtx->ddwAnaMonClockStep, DB_HANDLE_IDX_R_NCU_DATAAPP_ANA_MON_CLOCK_STEP,dwCtxId, dwhDB, enumUpdateIndex);
    }
    MCS_CHK_VOID_LIKELY(dwReportNum < EXPECT_DATA_APP_NUM);

    dwReportNum = psVpfuMcsGetAllDaAppCtxByClockStep(clockStep, dwhDB, DB_HANDLE_IDX_R_NCU_DATAAPP_ANA_MON_CLOCK_STEP, (void*)ptMcsDynCtxNoUniqAck);
    for(dwIndex=0;dwIndex <EXPECT_DATA_APP_NUM && dwIndex<dwReportNum;dwIndex++)
    {
        dwCtxId = ptMcsDynCtxNoUniqAck->adwDataArea[dwIndex];
        psNcuMcsUpdDaAppCtxByClockStep(getCurClockStep()+1, DB_HANDLE_IDX_R_NCU_DATAAPP_ANA_MON_CLOCK_STEP,dwCtxId, dwhDB, enumUpdateIndex);
    }
    return;
}
