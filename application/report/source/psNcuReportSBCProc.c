#include "psNcuReportSBCProc.h"
#include "MemShareCfg.h"
#include "psUpfCommon.h"
#include "zte_slibc.h"
#include "psUpfEvent.h"
#include "psUpfJobTypes.h"
#include "ncuSCCUAbility.h"
#include "psNcuSubscribeCtxProc.h"
#include "ps_ncu_session.h"
#include "psMcsDebug.h"
#include "ps_mcs_define.h"
#include "r_ncu_daprofile.h"
#include "ps_db_define_ncu.h"
#include "psNcuGetCfg.h"
#include "McsIPv4Head.h"
#include "psNcuCtxFunc.h"
#include "ps_nef_interface.h"
#include "ps_css_interface.h"
#include "dpathreadpub.h"
#include "psNefReportToNwdaf.h"
#include "psNcuSubBindAppCtxProc.h"
#include "McsProtoType.h"
BOOLEAN ncuThrd2NcuNefJob(BYTE *msg, WORD16 dwDataLen, WORD32 dwMsgID);
WORD32 psMcsDafFillUserLocationInfo(QosAnalysisInfo* ptQosRpt, T_psNcuSessionCtx* ptSessionCtx);
WORD32 psMcsFillFlowInfo(QosAnalysisInfo* ptQosRpt, T_psNcuFlowCtx* ptStmCtx, BYTE statusFlg);
WORD32 psVpfuMcsDafFillFlowDescInfo(QosAnalysisInfo* ptQosAnalysisInfo, T_psNcuFlowCtx* ptStmCtx);
WORD32 psMcsAnaReportProc(QosAnalysisInfo* ptQosAnalysisInfo, T_psNcuDaAppCtx * ptDaAppCtx, BYTE* buffer, BYTE statusFlg);
WORD32 psMcsLocationChgReportProc(QosAnalysisInfo* ptQosAnalysisInfo, T_psNcuDaAppCtx * ptDaAppCtx, BYTE* buffer, BYTE statusFlg);
WORD32 psMcsAppExpSpecialReportProc(QosAnalysisInfo* ptQosAnalysisInfo, T_psNcuDaAppCtx * ptDaAppCtx, BYTE* buffer);
void AnaFillsubAppQosReport(QosAnalysisInfo* ptQosAnalysisInfo, T_psNcuDaAppCtx * ptDaAppCtx);
void AnaSavDelStreamToDaApp(T_psNcuDaAppCtx * ptDaAppCtx, T_psNcuFlowCtx *ptStmAddr);


void psNcuReportToUpmProc(T_psNcuDaAppCtx * ptDaAppCtx, T_psNcuFlowCtx *ptStmAddr, WORD16 threadID, BYTE bRptType)
{
    if(NULL == ptDaAppCtx || NULL == ptDaAppCtx->ptSubScribeCtx || NULL == ptDaAppCtx->ptSessionCtx)
    {
        return;
    }
    if(NULL == g_ptMediaProcThreadPara)
    {
        return ;
    }
    T_psNcuMcsPerform *ptNcuPerform = (T_psNcuMcsPerform*)g_ptMediaProcThreadPara->ptMcsStatPointer;
    if(NULL == ptNcuPerform)
    {
        return;
    }
    if(NULL == ptDaAppCtx->ptFlowCtxHead || 0 == ptDaAppCtx->dwStreamNum )
    {
        MCS_LOC_STAT_EX(ptNcuPerform, qwNcuReportToUpmWithFlowZero, 1);
        DEBUG_TRACE(DEBUG_LOW,"ptDaAppCtx->ptFlowCtxHead=%p, ptDaAppCtx->dwStreamNum=%u\n", ptDaAppCtx->ptFlowCtxHead,ptDaAppCtx->dwStreamNum);
        return;
    }
    T_psNcuDaSubScribeCtx* ptSubScribeCtx = ptDaAppCtx->ptSubScribeCtx;
    T_psNcuSessionCtx* ptSessionCtx = ptDaAppCtx->ptSessionCtx;
    T_NwdafInfoToUPM* buffer = (T_NwdafInfoToUPM*)&g_ptVpfuShMemVar->aucInnerAckMsg[threadID%MEDIA_THRD_NUM][0];
    zte_memset_s(buffer, sizeof(T_NwdafInfoToUPM), 0, sizeof(T_NwdafInfoToUPM));
    zte_memcpy_s(buffer->aucCorid,AUCCORID_MAXLEN,ptSessionCtx->bCorrelationId_Ana,AUCCORID_MAXLEN);
    zte_memcpy_s(buffer->aucReportUri,AUCREPORTURI_MAXLEN,ptSessionCtx->Uri_Ana,AUCREPORTURI_MAXLEN);
    QosAnalysisInfo* ptQosRpt = &(buffer->tQosAnalysisInfo);
    zte_memcpy_s(ptQosRpt->appId, APP_ID_LEN, ptSubScribeCtx->appidstr, MIN(APP_ID_LEN,MAX_APPID_LEN));
    zte_memcpy_s(ptQosRpt->subAppId, APP_ID_LEN, ptDaAppCtx->subAppidStr, MIN(APP_ID_LEN,MAX_APPID_LEN));
    switch(bRptType)
    {
        case RPT_TYPE_LOOP:
            MCS_LOC_STAT_EX(ptNcuPerform, qwReportToUpmByTypeLoop, 1);
            ptQosRpt->subAppStatus_Flag = 1;
            ptQosRpt->subAppStatus = DAF_RUNNING;
            psMcsAppExpSpecialReportProc(ptQosRpt,ptDaAppCtx, (BYTE*)buffer); //只携带subAppQosMonReports
            DEBUG_TRACE(DEBUG_LOW, "RPT_TYPE_LOOP");
            break;
        case RPT_TYPE_STREAM_DEL:
            DEBUG_TRACE(DEBUG_LOW, "RPT_TYPE_STREAM_DEL");
            ptQosRpt->subAppStatus_Flag = 1;
            ptQosRpt->subAppStatus = DAF_RUNNING;
            AnaSavDelStreamToDaApp(ptDaAppCtx,ptStmAddr);
            if(ptDaAppCtx->dwStreamNum == 1)
            {
                MCS_LOC_STAT_EX(ptNcuPerform, qwReportToUpmLastFlow, 1);
                ptQosRpt->subAppStatus = TERMINATED;
            }
            if(psGetAppExpSwitchForAnaUser())
            {
                MCS_LOC_STAT_EX(ptNcuPerform, qwReportToUpmAppExpByStreamDel, 1);
                psMcsAppExpSpecialReportProc(ptQosRpt,ptDaAppCtx, (BYTE*)buffer); //只携带subAppQosMonReports，重点业务体验上报
                zte_memset_s(ptQosRpt,sizeof(QosAnalysisInfo),0,sizeof(QosAnalysisInfo));
                zte_memcpy_s(ptQosRpt->appId, APP_ID_LEN, ptSubScribeCtx->appidstr, MIN(APP_ID_LEN,MAX_APPID_LEN));
                zte_memcpy_s(ptQosRpt->subAppId, APP_ID_LEN, ptDaAppCtx->subAppidStr, MIN(APP_ID_LEN,MAX_APPID_LEN));
            }
            if(1 == ptDaAppCtx->bHasRptPoor)
            {
                ptQosRpt->subAppStatus_Flag = 1;
                ptQosRpt->subAppStatus = DAF_RUNNING;
                if(ptDaAppCtx->dwStreamNum == 1)
                {
                    ptQosRpt->subAppStatus = TERMINATED;
                }
                psMcsFillFlowInfo(ptQosRpt,ptStmAddr, TRUE);
                AnaFillsubAppQosReport(ptQosRpt, ptDaAppCtx);
                MCS_LOC_STAT_EX(ptNcuPerform, qwReportToUpmByStreamDel, 1);
                NCU_PM_55303_STAT_ADD(qwNcuReportQoSNumViaSBI, 1);
                handleDataAnalyticsNcuToUpmReq((BYTE*)buffer, ptDaAppCtx);
            }
            break;
        case RPT_TYPE_ANA:
            ptQosRpt->subAppStatus_Flag = 1;
            ptQosRpt->subAppStatus = DAF_RUNNING;
            MCS_LOC_STAT_EX(ptNcuPerform, qwReportToUpmByTypeAna, 1);
            psMcsAnaReportProc(ptQosRpt,ptDaAppCtx, (BYTE*)buffer, FALSE);
            DEBUG_TRACE(DEBUG_LOW, "RPT_TYPE_ANA");
            break;
        case RPT_TYPE_LOCATION_CHG:
            MCS_LOC_STAT_EX(ptNcuPerform, qwReportToUpmByLocationChg, 1);
            psMcsLocationChgReportProc(ptQosRpt,ptDaAppCtx, (BYTE*)buffer, FALSE);
            DEBUG_TRACE(DEBUG_LOW, "RPT_TYPE_LOCATION_CHG");
            break;
        default:
            DEBUG_TRACE(DEBUG_HIG, "unsupport rpttype for sbc, rpttype=%u\n", bRptType);
            return;
    }

}
void AnaSavDelStreamToDaApp(T_psNcuDaAppCtx * ptDaAppCtx, T_psNcuFlowCtx *ptStmAddr)
{
    if(NULL == ptDaAppCtx || NULL == ptStmAddr)
    {
        return;
    }
    DEBUG_TRACE(DEBUG_LOW, "AnaSavDelStreamToDaApp enter !!\n");
    ptDaAppCtx->dwDelTotalUlPktNum     += ptStmAddr->dwTotalUlPktNum;
    ptDaAppCtx->dwDelTotalDlPktNum     += ptStmAddr->dwTotalDlPktNum;
    ptDaAppCtx->dwDelTotalUlPktBytes   += ptStmAddr->dwTotalUlPktBytes;
    ptDaAppCtx->dwDelTotalDlPktBytes   += ptStmAddr->dwTotalDlPktBytes;
    ptDaAppCtx->dwDelTotalUlPktLossNum += ptStmAddr->dwTotalUlPktLossNum;
    ptDaAppCtx->dwDelTotalDlPktLossNum += ptStmAddr->dwTotalDlPktLossNum;
    ptDaAppCtx->dwDelTotalUlPktRtt     += ptStmAddr->dwTotalUlPktRtt;
    ptDaAppCtx->dwDelTotalDlPktRtt     += ptStmAddr->dwTotalDlPktRtt;
    ptDaAppCtx->dwDelTotalUlRttPktNum  += ptStmAddr->dwTotalUlRttPktNum;
    ptDaAppCtx->dwDelTotalDlRttPktNum  += ptStmAddr->dwTotalDlRttPktNum;
    return;
}
WORD32 psMcsLocationChgReportProc(QosAnalysisInfo* ptQosAnalysisInfo, T_psNcuDaAppCtx * ptDaAppCtx, BYTE* buffer, BYTE statusFlg)
{
    DEBUG_TRACE(DEBUG_LOW, "psMcsLocationChgReportProc enter !!\n");
    if(NULL == ptQosAnalysisInfo || NULL == ptDaAppCtx)
    {
        return MCS_RET_FAIL;
    }
    WORD32 dwRet = MCS_RET_SUCCESS;
    T_psNcuSessionCtx* ptSessionCtx = ptDaAppCtx->ptSessionCtx;
    if(NULL == ptSessionCtx)
    {
        return MCS_RET_FAIL;
    }
    psMcsDafFillUserLocationInfo(ptQosAnalysisInfo, ptSessionCtx);
    NCU_PM_55303_STAT_ADD(qwNcuReportQoSNumViaSBI, 1);
    dwRet |= handleDataAnalyticsNcuToUpmReq((BYTE*)buffer, ptDaAppCtx);
    DEBUG_TRACE(DEBUG_LOW,"\n[psMcsLocationChgReportProc]pfu send data analytics req to upm!,dwRet = %u\n",dwRet);
    return dwRet;
}

WORD32 psMcsAppExpSpecialReportProc(QosAnalysisInfo* ptQosAnalysisInfo, T_psNcuDaAppCtx * ptDaAppCtx, BYTE* buffer)
{
    DEBUG_TRACE(DEBUG_LOW, "psMcsAppExpSpecialReportProc enter !!\n");
    if(NULL == ptQosAnalysisInfo || NULL == ptDaAppCtx)
    {
        return MCS_RET_FAIL;
    }
    ptQosAnalysisInfo->infoIndicate  = QOSINFOTYPE_EXP;
    ptQosAnalysisInfo->subAppQosQuality_Flag = 1;
    ptQosAnalysisInfo->subAppQosQuality = (0 == ptDaAppCtx->bIsDialFlg)?ptDaAppCtx->bCurQoe:ptDaAppCtx->bCurDialQoe;    // 如果处于拨测态,则上报配置的拨测状态;
    psMcsDafFillUserLocationInfo(ptQosAnalysisInfo, (T_psNcuSessionCtx*)(ptDaAppCtx->ptSessionCtx));
    T_psNcuDataAnalysis* ptAnalysis = &ptDaAppCtx->Analysis;
    ptQosAnalysisInfo->subAppQosMonReport_Flag = 1;
    ptQosAnalysisInfo->subAppQosMonReport.bandwidthUl_Flag = 1;
    ptQosAnalysisInfo->subAppQosMonReport.bandwidthUl = ptAnalysis->dwUlThroughput;
    ptQosAnalysisInfo->subAppQosMonReport.bandwidthDl_Flag = 1;
    ptQosAnalysisInfo->subAppQosMonReport.bandwidthDl = ptAnalysis->dwDlThroughput;
    // RTT
    ptQosAnalysisInfo->subAppQosMonReport.delayAn_Flag = 1;
    ptQosAnalysisInfo->subAppQosMonReport.delayAn = ptAnalysis->dwUlAvgRtt;
    ptQosAnalysisInfo->subAppQosMonReport.delayDn_Flag = 1;
    ptQosAnalysisInfo->subAppQosMonReport.delayDn = ptAnalysis->dwDlAvgRtt;
    // 丢包率
    ptQosAnalysisInfo->subAppQosMonReport.lostpacketsUl_Flag = 1;
    ptQosAnalysisInfo->subAppQosMonReport.lostpacketsUl = ptAnalysis->dwUlLossNum;
    ptQosAnalysisInfo->subAppQosMonReport.totalpacketsUl_Flag = 1;
    ptQosAnalysisInfo->subAppQosMonReport.totalpacketsUl = ptAnalysis->dwUlTcpPktNum;
    ptQosAnalysisInfo->subAppQosMonReport.lostpacketsDl_Flag = 1;
    ptQosAnalysisInfo->subAppQosMonReport.lostpacketsDl = ptAnalysis->dwDlLossNum;
    ptQosAnalysisInfo->subAppQosMonReport.totalpacketsDl_Flag = 1;
    ptQosAnalysisInfo->subAppQosMonReport.totalpacketsDl = ptAnalysis->dwDlTcpPktNum;
    
    NCU_PM_55303_STAT_ADD(qwNcuReportKeyExpDataNumViaSBI, 1);
    return handleDataAnalyticsNcuToUpmReq((BYTE*)buffer, ptDaAppCtx);
    
}
void psNcuAnaSubFillWithTCP(T_psNcuDaAppCtx *ptDataAppCtx, QosAnalysisInfo* ptQosAnalysisInfo)
{
    
    if(NULL == ptDataAppCtx || NULL == ptQosAnalysisInfo)
    {
        return;
    }
    DEBUG_TRACE(DEBUG_LOW, "psNcuAnaSubFillWithTCP enter !!\n");
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

        if (curStm->bProType == MCS_TCP_PROTOCOL && curStm->bSpecialStmFlag == 0)
        {
            bHasTcpStmFlg = 1;
        }
    }
    if (!bHasTcpStmFlg)
    {
        ptQosAnalysisInfo->subAppQosMonReport.delayAn    = (WORD32)-1;
        ptQosAnalysisInfo->subAppQosMonReport.delayDn     = (WORD32)-1;
        ptQosAnalysisInfo->subAppQosMonReport.lostpacketsUl  = (WORD32)-1;
        ptQosAnalysisInfo->subAppQosMonReport.totalpacketsUl  = (WORD32)-1;
        ptQosAnalysisInfo->subAppQosMonReport.lostpacketsDl   = (WORD32)-1;
        ptQosAnalysisInfo->subAppQosMonReport.totalpacketsDl   = (WORD32)-1;
    }
    return;
}

void AnaFillsubAppQosReport(QosAnalysisInfo* ptQosAnalysisInfo, T_psNcuDaAppCtx * ptDaAppCtx)
{
    if(NULL == ptQosAnalysisInfo || NULL == ptDaAppCtx)
    {
        return;
    }
    DEBUG_TRACE(DEBUG_LOW, "AnaFillsubAppQosReport enter !!\n");
    ptQosAnalysisInfo->infoIndicate = QOSINFOTYPE_ANA;
    ptQosAnalysisInfo->subAppQosQuality_Flag = 1;

    BYTE bRptQosQuality = (0 == ptDaAppCtx->bIsDialFlg)?ptDaAppCtx->bCurQoe:ptDaAppCtx->bCurDialQoe;    // 如果处于拨测态,则上报配置的拨测状态
    ptQosAnalysisInfo->subAppQosQuality = bRptQosQuality;
    if(DA_QUALITY_POOR == bRptQosQuality)
    {
        ptDaAppCtx->bHasRptPoor = 1; //上报过质差
    }
    // 填写subAppQosReport
    ptQosAnalysisInfo->subAppQosMonReport_Flag               = 1;
    ptQosAnalysisInfo->subAppQosMonReport.delayAn_Flag       = 1;
    ptQosAnalysisInfo->subAppQosMonReport.delayAn            = ptDaAppCtx->dwUlUnitRtt;
    ptQosAnalysisInfo->subAppQosMonReport.delayDn_Flag       = 1;
    ptQosAnalysisInfo->subAppQosMonReport.delayDn            = ptDaAppCtx->dwDlUnitRtt;
    ptQosAnalysisInfo->subAppQosMonReport.bandwidthUl_Flag   = 1;
    ptQosAnalysisInfo->subAppQosMonReport.bandwidthUl        = ptDaAppCtx->dwUlUnitBw;
    ptQosAnalysisInfo->subAppQosMonReport.bandwidthDl_Flag   = 1;
    ptQosAnalysisInfo->subAppQosMonReport.bandwidthDl        = ptDaAppCtx->dwDlUnitBw;
    ptQosAnalysisInfo->subAppQosMonReport.lostpacketsUl_Flag  = 1;
    ptQosAnalysisInfo->subAppQosMonReport.lostpacketsUl       = ptDaAppCtx->dwUlUnitPktLossNum;
    ptQosAnalysisInfo->subAppQosMonReport.totalpacketsUl_Flag = 1;
    ptQosAnalysisInfo->subAppQosMonReport.totalpacketsUl      = ptDaAppCtx->dwUlUnitPktNum;
    ptQosAnalysisInfo->subAppQosMonReport.lostpacketsDl_Flag  = 1;
    ptQosAnalysisInfo->subAppQosMonReport.lostpacketsDl       = ptDaAppCtx->dwDlUnitPktLossNum;
    ptQosAnalysisInfo->subAppQosMonReport.totalpacketsDl_Flag = 1;
    ptQosAnalysisInfo->subAppQosMonReport.totalpacketsDl      = ptDaAppCtx->dwDlUnitPktNum;

    // 判断是否全是非TCP报文
    psNcuAnaSubFillWithTCP(ptDaAppCtx, ptQosAnalysisInfo);

	T_psNcuMcsPerform *ptNcuPerform = psGetPerform();
    if(NULL== ptNcuPerform)
    {
        DEBUG_TRACE(DEBUG_LOW,"AnaFillsubAppQosReport get ptNcuPerform fail!!\n");
        return;
    }
    MCS_LOC_STAT_EX(ptNcuPerform, qwFillSubAppQosRep, 1);
    return;
}
void AnaClrReportFlow(QosAnalysisInfo* ptQosAnalysisInfo)
{
    DEBUG_TRACE(DEBUG_LOW, "AnaClrReportFlow enter !!\n");
    if(NULL == ptQosAnalysisInfo)
    {
        return;
    }
    ptQosAnalysisInfo->flowQosMonReports_Flag = 0;
    ptQosAnalysisInfo->num_FlowQosMonReports = 0;
    zte_memset_s(ptQosAnalysisInfo->flowQosMonReports,sizeof(QosMonitoringReport_Nnwdaf)*QOSANA_NUM,0,sizeof(QosMonitoringReport_Nnwdaf)*QOSANA_NUM);

    ptQosAnalysisInfo->flowQosDescs_Flag = 0;
    ptQosAnalysisInfo->num_FlowQosDescs = 0;
    zte_memset_s(ptQosAnalysisInfo->flowQosDescs,sizeof(FlowDescription)*QOSANA_NUM,0,sizeof(FlowDescription)*QOSANA_NUM);
   
    ptQosAnalysisInfo->flowQosQualities_Flag = 0;
    ptQosAnalysisInfo->num_FlowQosQualities = 0;
    zte_memset_s(ptQosAnalysisInfo->flowQosQualities,sizeof(QualityStatus)*QOSANA_NUM,0,sizeof(QualityStatus)*QOSANA_NUM);

    ptQosAnalysisInfo->n5qi_Flag = 0;
    ptQosAnalysisInfo->num_N5qi = 0;
    zte_memset_s(ptQosAnalysisInfo->n5qi,sizeof(_5Qi)*QOSANA_NUM,0,sizeof(_5Qi)*QOSANA_NUM);

    ptQosAnalysisInfo->flowStatus_Flag = 0;
    ptQosAnalysisInfo->num_FlowStatus = 0;
    zte_memset_s(ptQosAnalysisInfo->flowStatus,sizeof(RunningStatus)*QOSANA_NUM,0,sizeof(RunningStatus)*QOSANA_NUM);
    return;
}

WORD32 psMcsAnaReportProc(QosAnalysisInfo* ptQosAnalysisInfo, T_psNcuDaAppCtx * ptDaAppCtx, BYTE* buffer, BYTE statusFlg)
{
    DEBUG_TRACE(DEBUG_LOW, "psMcsAnaReportProc enter !!\n");
    if(NULL == ptQosAnalysisInfo || NULL == ptDaAppCtx)
    {
        return MCS_RET_FAIL;
    }
    // ana按流上报，最多一次上报5个流信息
    BYTE bflowrepoNum = 0;
    WORD32 dwRet = MCS_RET_SUCCESS;
    T_psNcuFlowCtx* ptTmpStream = (T_psNcuFlowCtx*)ptDaAppCtx->ptFlowCtxHead;
    T_psNcuFlowCtx* curStm = NULL;
    while(NULL != ptTmpStream)
    {
        if(ptDaAppCtx != ptTmpStream->ptNcuDaCtx)
        {
            DEBUG_TRACE(DEBUG_LOW,"psMcsAnaReportProc get wrong stream!!!\n");
            continue;
        }
        curStm = ptTmpStream;
        ptTmpStream =  (T_psNcuFlowCtx*)curStm->ptNcuFlowAppNext;

        bflowrepoNum++;
        psMcsFillFlowInfo(ptQosAnalysisInfo,curStm,statusFlg);

        if(0 == bflowrepoNum%5)
        {
            DEBUG_TRACE(DEBUG_LOW, "\n[Mcs]psMcsAnaReportProc ptQosAnalysisInfo->appId is %s\n",ptQosAnalysisInfo->appId);
            AnaFillsubAppQosReport(ptQosAnalysisInfo, ptDaAppCtx);
            dwRet |= handleDataAnalyticsNcuToUpmReq((BYTE*)buffer, ptDaAppCtx);
            NCU_PM_55303_STAT_ADD(qwNcuReportQoSNumViaSBI, 1);
            AnaClrReportFlow(ptQosAnalysisInfo);
        }
        if((NULL == ptTmpStream) && (0 != bflowrepoNum%5))
        {
            DEBUG_TRACE(DEBUG_LOW,"\n[Mcs]psMcsAnaReportProc report last patch stream ptQosAnalysisInfo->appId is %s\n",ptQosAnalysisInfo->appId);
            AnaFillsubAppQosReport(ptQosAnalysisInfo, ptDaAppCtx);
            dwRet |= handleDataAnalyticsNcuToUpmReq((BYTE*)buffer, ptDaAppCtx);
            NCU_PM_55303_STAT_ADD(qwNcuReportQoSNumViaSBI, 1);
        }
    }
    DEBUG_TRACE(DEBUG_LOW,"\n[psMcsAnaReportProc]pfu send data analytics req to upm!,dwRet = %u\n",dwRet);
    return dwRet;
}
/*
BOOLEAN ncuThrd2NcuNefJob(BYTE *msg, WORD16 dwDataLen, WORD32 dwMsgID)
{
    WORD wSelfThreadNo = (WORD)psCSSGetThrdNo();
    DEBUG_TRACE(DEBUG_HIG, "msgID[%u] msgLen[%u] dwThr[%d]!\n",
                  dwMsgID, dwDataLen, wSelfThreadNo);
    PS_PACKET* ptPacket = psCSSGenPkt(msg, dwDataLen, wSelfThreadNo);
    if (unlikely(NULL == ptPacket))
    {
        DEBUG_TRACE(DEBUG_HIG, "psCSSGenPkt Failed, msgID[%u] msgLen[%u] dwThr[%d]!\n",
                  dwMsgID, dwDataLen, wSelfThreadNo);
        return FALSE;
    }

    if(0 != dpaUcomUpSend1(JOB_TYPE_NCU_NEFENTRY, dwMsgID, ptPacket))
    {
        DEBUG_TRACE(DEBUG_HIG, "dpaUcomUpSend1 JOB_TYPE_NCU_NEFENTRY fail!\n");
        return FALSE;
    }
    return TRUE;
}
*/

WORD32 psMcsFillFlowInfo(QosAnalysisInfo* ptQosAnalysisInfo, T_psNcuFlowCtx* ptStmCtx, BYTE statusFlg)
{
    DEBUG_TRACE(DEBUG_LOW, "\n psMcsFillFlowInfo enter !\n");
    if(NULL == ptQosAnalysisInfo || NULL == ptStmCtx)
    {
        return MCS_RET_FAIL;
    }
    T_psNcuDaAppCtx * ptDaAppCtx = ptStmCtx->ptNcuDaCtx;
    if(NULL == ptDaAppCtx)
    {
        return MCS_RET_FAIL;
    }
    ptQosAnalysisInfo->flowStatus_Flag = 1;
    _DB_STATEMENT_TRUE_RTN_VALUE((ptQosAnalysisInfo->num_FlowStatus >= QOSANA_NUM), MCS_RET_FAIL);
    if(statusFlg)
    {
        ptQosAnalysisInfo->flowStatus[ptQosAnalysisInfo->num_FlowStatus] = TERMINATED;
    }
    else
    {
        ptQosAnalysisInfo->flowStatus[ptQosAnalysisInfo->num_FlowStatus] = DAF_RUNNING;
    }
    ptQosAnalysisInfo->num_FlowStatus++;
    ptQosAnalysisInfo->n5qi_Flag = 1;
    _DB_STATEMENT_TRUE_RTN_VALUE((ptQosAnalysisInfo->num_N5qi >= QOSANA_NUM), MCS_RET_FAIL);
    ptQosAnalysisInfo->n5qi[ptQosAnalysisInfo->num_N5qi] = ptStmCtx->b5QI;
    ptQosAnalysisInfo->num_N5qi++;
    DEBUG_TRACE(DEBUG_LOW, "\n[Mcs]psMcsFillFlowInfo ,b5QI is %u\n",ptStmCtx->b5QI);
    
    ptQosAnalysisInfo->flowQosQualities_Flag = 1;
    _DB_STATEMENT_TRUE_RTN_VALUE((ptQosAnalysisInfo->num_FlowQosQualities >= QOSANA_NUM), MCS_RET_FAIL);
    ptQosAnalysisInfo->flowQosQualities[ptQosAnalysisInfo->num_FlowQosQualities] = (0 == ptDaAppCtx->bIsDialFlg)?ptDaAppCtx->bCurQoe:ptDaAppCtx->bCurDialQoe;    // 如果处于拨测态,则上报配置的拨测状态
    ptQosAnalysisInfo->num_FlowQosQualities++;
    WORD32 bandwidthUl = ptStmCtx->bandwidthUl;
    WORD32 bandwidthDl = ptStmCtx->bandwidthDl;
    DEBUG_TRACE(DEBUG_LOW, "\n[Mcs]psMcsFillFlowInfo ,bandwidthUl is %u, bandwidthDl is %u\n",bandwidthUl,bandwidthDl);
    DEBUG_TRACE(DEBUG_LOW, "\n[Mcs]psMcsFillFlowInfo ,delayAn is %u, delayDn is %u\n",ptStmCtx->dwUlRtt,ptStmCtx->dwDlRtt);
    DEBUG_TRACE(DEBUG_LOW, "\n[Mcs]psMcsFillFlowInfo ,packetLossRateUl is %u, packetLossRateUl is %u\n",ptStmCtx->dwUlPktLossRate,ptStmCtx->dwDlPktLossRate);

    ptQosAnalysisInfo->flowQosMonReports_Flag = 1;
    _DB_STATEMENT_TRUE_RTN_VALUE((ptQosAnalysisInfo->num_FlowQosMonReports >= QOSANA_NUM), MCS_RET_FAIL);
    ptQosAnalysisInfo->flowQosMonReports[ptQosAnalysisInfo->num_FlowQosMonReports].bandwidthUl_Flag = 1;
    ptQosAnalysisInfo->flowQosMonReports[ptQosAnalysisInfo->num_FlowQosMonReports].bandwidthUl = bandwidthUl;
    ptQosAnalysisInfo->flowQosMonReports[ptQosAnalysisInfo->num_FlowQosMonReports].bandwidthDl_Flag = 1;
    ptQosAnalysisInfo->flowQosMonReports[ptQosAnalysisInfo->num_FlowQosMonReports].bandwidthDl = bandwidthDl;
    // RTT
    ptQosAnalysisInfo->flowQosMonReports[ptQosAnalysisInfo->num_FlowQosMonReports].delayAn_Flag = 1;
    ptQosAnalysisInfo->flowQosMonReports[ptQosAnalysisInfo->num_FlowQosMonReports].delayAn = ptStmCtx->dwUlRtt;
    ptQosAnalysisInfo->flowQosMonReports[ptQosAnalysisInfo->num_FlowQosMonReports].delayDn_Flag = 1;
    ptQosAnalysisInfo->flowQosMonReports[ptQosAnalysisInfo->num_FlowQosMonReports].delayDn = ptStmCtx->dwDlRtt;
    // 丢包率
    ptQosAnalysisInfo->flowQosMonReports[ptQosAnalysisInfo->num_FlowQosMonReports].lostpacketsUl_Flag = 1;
    ptQosAnalysisInfo->flowQosMonReports[ptQosAnalysisInfo->num_FlowQosMonReports].lostpacketsUl = ptStmCtx->dwUlUnitPktLossNum;
    ptQosAnalysisInfo->flowQosMonReports[ptQosAnalysisInfo->num_FlowQosMonReports].totalpacketsUl_Flag = 1;
    ptQosAnalysisInfo->flowQosMonReports[ptQosAnalysisInfo->num_FlowQosMonReports].totalpacketsUl = ptStmCtx->dwTotalUlUnitPktNum;
    ptQosAnalysisInfo->flowQosMonReports[ptQosAnalysisInfo->num_FlowQosMonReports].lostpacketsDl_Flag = 1;
    ptQosAnalysisInfo->flowQosMonReports[ptQosAnalysisInfo->num_FlowQosMonReports].lostpacketsDl = ptStmCtx->dwDlUnitPktLossNum;
    ptQosAnalysisInfo->flowQosMonReports[ptQosAnalysisInfo->num_FlowQosMonReports].totalpacketsDl_Flag = 1;
    ptQosAnalysisInfo->flowQosMonReports[ptQosAnalysisInfo->num_FlowQosMonReports].totalpacketsDl = ptStmCtx->dwTotalDlUnitPktNum;
    
    if(ptStmCtx->bProType != MCS_TCP_PROTOCOL || ptStmCtx->bSpecialStmFlag)
    {
        ptQosAnalysisInfo->flowQosMonReports[ptQosAnalysisInfo->num_FlowQosMonReports].delayAn = (WORD32)-1;
        ptQosAnalysisInfo->flowQosMonReports[ptQosAnalysisInfo->num_FlowQosMonReports].delayDn = (WORD32)-1;
        // 丢包率
        ptQosAnalysisInfo->flowQosMonReports[ptQosAnalysisInfo->num_FlowQosMonReports].lostpacketsUl = (WORD32)-1;
        ptQosAnalysisInfo->flowQosMonReports[ptQosAnalysisInfo->num_FlowQosMonReports].totalpacketsUl = (WORD32)-1;
        ptQosAnalysisInfo->flowQosMonReports[ptQosAnalysisInfo->num_FlowQosMonReports].lostpacketsDl = (WORD32)-1;
        ptQosAnalysisInfo->flowQosMonReports[ptQosAnalysisInfo->num_FlowQosMonReports].totalpacketsDl = (WORD32)-1;
     
    }
    ptQosAnalysisInfo->num_FlowQosMonReports++;

    psVpfuMcsDafFillFlowDescInfo(ptQosAnalysisInfo, ptStmCtx);
    psMcsDafFillUserLocationInfo(ptQosAnalysisInfo, (T_psNcuSessionCtx*)ptStmCtx->ptNcuSesionCtx);

    return MCS_RET_SUCCESS;

}

WORD32 psVpfuMcsDafFillFlowDescInfo(QosAnalysisInfo* ptQosAnalysisInfo, T_psNcuFlowCtx* ptStmCtx)
{    
    DEBUG_TRACE(DEBUG_LOW, "\n psVpfuMcsDafFillFlowDescInfo enter !\n");
    lp_r_flowctx_idx_tuple ptQuintuple = (lp_r_flowctx_idx_tuple)psVpfuMcsGetCtxIdxById(ptStmCtx->dwFlowID, ptStmCtx->dwhDBByThreadNo, DB_HANDLE_R_NCUFLOWCTX);

    if(NULL == ptQuintuple)
    {
        DEBUG_TRACE(DEBUG_HIG,  "\n[Mcs]psVpfuMcsDafFillFlowDescInfo ptQuintuple is NULL\n");
        return MCS_RET_FAIL;
    }
    ptQosAnalysisInfo->flowQosDescs_Flag = 1;
    _DB_STATEMENT_TRUE_RTN_VALUE((ptQosAnalysisInfo->num_FlowQosDescs >= QOSANA_NUM), MCS_RET_FAIL);
    FlowDescription *ptFlowDesc = &ptQosAnalysisInfo->flowQosDescs[ptQosAnalysisInfo->num_FlowQosDescs];
    CHAR proto[4] = {0};
    CHAR pdnip[40] = {0};
    CHAR ueip[40] = {0};
    CHAR mask[5] = {0};
    CHAR pdnport[7] = {0};
    CHAR ueport[7] = {0};
    if(0 == ptQuintuple->bProType)
    {
        zte_snprintf_s(proto,sizeof(proto),"ip");
    }
    else
    {
        zte_snprintf_s(proto,sizeof(proto),"%u",ptQuintuple->bProType);
    }
    if(IPv4_VERSION == ptQuintuple->bIPType)
    {
        zte_snprintf_s(pdnip,sizeof(pdnip),"%u.%u.%u.%u", ptQuintuple->tSvrIP[0],ptQuintuple->tSvrIP[1],ptQuintuple->tSvrIP[2],ptQuintuple->tSvrIP[3]);
        zte_snprintf_s(ueip,sizeof(pdnip),"%u.%u.%u.%u", ptQuintuple->tCliIP[0],ptQuintuple->tCliIP[1],ptQuintuple->tCliIP[2],ptQuintuple->tCliIP[3]);
        zte_snprintf_s(mask,sizeof(mask),"/%u",32);
    }
    else
    {
        zte_snprintf_s(pdnip,sizeof(pdnip),"%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x",
                    ptQuintuple->tSvrIP[0],ptQuintuple->tSvrIP[1],ptQuintuple->tSvrIP[2],ptQuintuple->tSvrIP[3],
                    ptQuintuple->tSvrIP[4],ptQuintuple->tSvrIP[5],ptQuintuple->tSvrIP[6],ptQuintuple->tSvrIP[7],
                    ptQuintuple->tSvrIP[8],ptQuintuple->tSvrIP[9],ptQuintuple->tSvrIP[10],ptQuintuple->tSvrIP[11],
                    ptQuintuple->tSvrIP[12],ptQuintuple->tSvrIP[13],ptQuintuple->tSvrIP[14],ptQuintuple->tSvrIP[15]);
        zte_snprintf_s(ueip,sizeof(ueip),"%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x",
                    ptQuintuple->tCliIP[0],ptQuintuple->tCliIP[1],ptQuintuple->tCliIP[2],ptQuintuple->tCliIP[3],
                    ptQuintuple->tCliIP[4],ptQuintuple->tCliIP[5],ptQuintuple->tCliIP[6],ptQuintuple->tCliIP[7],
                    ptQuintuple->tCliIP[8],ptQuintuple->tCliIP[9],ptQuintuple->tCliIP[10],ptQuintuple->tCliIP[11],
                    ptQuintuple->tCliIP[12],ptQuintuple->tCliIP[13],ptQuintuple->tCliIP[14],ptQuintuple->tCliIP[15]);
        zte_snprintf_s(mask,sizeof(mask),"/%u",128);
    }
    
    
    if(0 == ptQuintuple->wSvrPort)
    {
        zte_snprintf_s(pdnport,sizeof(pdnport)," ");
    }
    else
    {
        zte_snprintf_s(pdnport,sizeof(pdnport)," %u",ptQuintuple->wSvrPort);
    }
    if(0 == ptQuintuple->wCliPort)
    {
        zte_snprintf_s(ueport,sizeof(ueport)," ");
    }
    else
    {
        zte_snprintf_s(ueport,sizeof(ueport)," %u",ptQuintuple->wCliPort);
    }
    
    DEBUG_TRACE(DEBUG_LOW, "permit out %s from %s%s%s to %s%s%s !\n",proto,pdnip,mask,pdnport,ueip,mask,ueport);
    zte_snprintf_s((CHAR *)ptFlowDesc,FLOW_DESCRIPTION_LEN,"permit out %s from %s%s%s to %s%s%s",proto,pdnip,mask,pdnport,ueip,mask,ueport);
    ptQosAnalysisInfo->num_FlowQosDescs++;
    return MCS_RET_SUCCESS;
}

WORD32 psMcsDafFillUserLocationInfo(QosAnalysisInfo* ptQosRpt, T_psNcuSessionCtx* ptSessionCtx)
{
    DEBUG_TRACE(DEBUG_LOW, "\n sMcsDafFillUserLocationInfo enter !\n");
    if(NULL == ptQosRpt|| NULL == ptSessionCtx)
    {
        return MCS_RET_FAIL;
    }
    if(0 == ptSessionCtx->bHasUli)
    {
        DEBUG_TRACE(DEBUG_ERR, "ptSessionCtx no find uli");
        return MCS_RET_FAIL;
    }
    ptQosRpt->userLocationInfo_Flag = 1;
    DEBUG_TRACE(DEBUG_LOW, "psMcsDafFillUserLocationInfo sizeof(Userlocation) is %lu !\n", sizeof(ptSessionCtx->tUserLocation));
    zte_memcpy_s(&(ptQosRpt->userLocationInfo),sizeof(ptQosRpt->userLocationInfo), &(ptSessionCtx->tUserLocation), sizeof(UserLocation));
    return MCS_RET_SUCCESS;
            
}