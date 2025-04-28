#include "psNcuReport.h"
#include "ps_db_define_ncu.h"
#include "MemShareCfg.h"
#include "psMcsDebug.h"
#include "zte_slibc.h"
#include "psNcuGetCfg.h"
#include "psNcuDataAnalysis.h"
#include "psNcuDAProfileCtxProc.h"
#include "psNcuReportUDPProc.h"
#include "psNcuReportTimer.h"
#include "psNcuReportSBCProc.h"
#include "psNcuReportUDPProc.h"
#include "psNcuMultiReport.h"
#include "psNcuMonitor.h"
#include "ps_ncu_typedef.h"

extern void ncu_c_timer_entry(WORD32 threadID, WORD64 clockStep);

WORD32 g_ana_tim = 5;
WORD32 g_exp_tim = 300;

#define NCU_NO_MULTIRPT(subtype, addnum) ( \
        (((subtype) & enumQosExpNormal) && ((subtype) & enumQosExpSpecial) && ((addnum) == 2))|| \
        ((((subtype) == enumQosExpNormal) || ((subtype) == enumQosExpSpecial)) && (addnum)==1))

void handleDataAnalysisCtxTimer(WORD16 threadID, WORD64 clockStep);
void handleDataAnalysisCtxTimerClockStep(WORD16 threadID, WORD32 dwhIdx, WORD64 clockStep);
void handleExpDataAnalysisCtxForMultiRpt(WORD16 threadID, WORD64 clockStep);

void handleDataAnalysisClock(WORD16 threadID, WORD64 ddwCurrentTick)
{
    WORD64  n = getIncreasedClockStepSize(ddwCurrentTick);
    
    for (;n>0;n--)
    {
        if (getNcuSoftPara(5060) != 0) 
        {        
            ncu_c_timer_entry(threadID,getCurClockStep()+1-n);
        }
        else
        {
            psVpfuMcsDAQualityMonitor(threadID,getCurClockStep()+1-n);
            handleDataAnalysisCtxTimer(threadID,getCurClockStep()+1-n);
        }
    }
    
}

void handleDataAnalysisCtxTimer(WORD16 threadID, WORD64 clockStep)
{
    handleDataAnalysisCtxTimerClockStep(threadID, DB_HANDLE_IDX_R_NCU_DATAAPP_ANA_CLOCK_STEP, clockStep);
    if(likely(0 == g_upfConfig.dataSysCfg.bRptMulSwitch))
    {
        handleDataAnalysisCtxTimerClockStep(threadID, DB_HANDLE_IDX_R_NCU_DATAAPP_EXP_CLOCK_STEP, clockStep);
        return;
    }
    handleExpDataAnalysisCtxForMultiRpt(threadID,clockStep);
}

void handleExpDataAnalysisCtxForMultiRpt(WORD16 threadID, WORD64 clockStep)
{
    T_psMultiGroupData* ptGroupData = &(g_tMultiGroup[threadID%MEDIA_THRD_NUM][0]);
    zte_memset_s(ptGroupData,sizeof(T_psMultiGroupData)*GROUP_RPT_MAX_NUM, 0, sizeof(T_psMultiGroupData)*GROUP_RPT_MAX_NUM);

    WORD32 dwhDB  = _NCU_GET_DBHANDLE(threadID);
    WORD32 dwhIdx = DB_HANDLE_IDX_R_NCU_DATAAPP_EXP_CLOCK_STEP;
    MCS_DM_QUERYDYNDATA_ACK *ptMcsDynCtxNoUniqAck = &g_ptVpfuShMemVar->tGwGloData.atMcsDynCtxNoUniqAck[threadID % MEDIA_THRD_NUM];
    WORD32 dwReportNum = psVpfuMcsGetAllDaAppCtxByClockStep(clockStep, dwhDB, dwhIdx, (void*)ptMcsDynCtxNoUniqAck);
    WORD32 dwIndex = 0;
    WORD32 dwCtxId = 0;
    BYTE   bAddResult = 0;
    T_psNcuDaAppCtx * ptDaAppCtx = NULL;
    for(dwIndex=0;dwIndex < MIN(EXPECT_DATA_APP_NUM,dwReportNum);dwIndex++)
    {
        dwCtxId = ptMcsDynCtxNoUniqAck->adwDataArea[dwIndex];
        ptDaAppCtx = psMcsGetDaAppCtxById(dwCtxId, dwhDB);
        MCS_CHK_CONTINUE(NULL == ptDaAppCtx);
        psNcuUpdAllStmDataToDataApp(ptDaAppCtx,FALSE, DA_QOS_EXP);
        BYTE bSubType = getDaTypeFromDataAppCtx(ptDaAppCtx);
        bAddResult = 0;
        if((bSubType & enumQosExpNormal) && (MCS_RET_FAIL ==psCheckExpNormalAddAppCtxToGroup(ptDaAppCtx,ptGroupData, threadID)))
        {
            psNcuReportUDPToNwdafProc(ptDaAppCtx, threadID, RPT_TYPE_LOOP, enumQosExpNormal);
            bAddResult += 1;
        }
        if((bSubType & enumQosExpSpecial) && (MCS_RET_FAIL == psCheckExpSpecialAddAppCtxToGroup(ptDaAppCtx,ptGroupData, threadID)))
        {
            psNcuReportUDPToNwdafProc(ptDaAppCtx, threadID, RPT_TYPE_LOOP, enumQosExpSpecial);
            bAddResult += 1;
        }
        if((bSubType & enumQosAna) && 0 != psGetAppExpSwitchForAnaUser())
        {
            psNcuReportToUpmProc(ptDaAppCtx,NULL, threadID, RPT_TYPE_LOOP);
        }
        if(NCU_NO_MULTIRPT(bSubType, bAddResult))
        {
            psNcuRestoreDataAppAfterRpt(ptDaAppCtx, threadID);
        }
        psNcuMcsResetDaAppCtxByClockStep(clockStep+ ptDaAppCtx->ddwExpClockStep, DB_HANDLE_IDX_R_NCU_DATAAPP_EXP_CLOCK_STEP, dwCtxId, dwhDB);
    }


    getNormalExpMultiRptInfo(threadID, &ptGroupData[GROUP_EXP_NORMAL]);
    psNcuShowMultiRptData();
    psNcuReportMultiProc(ptGroupData,threadID);
    psNcuReSetDataInfo(ptGroupData,threadID);
}

void handleDataAnalysisCtxTimerClockStep(WORD16 threadID, WORD32 dwhIdx, WORD64 clockStep)
{
    WORD32 dwhDB = _NCU_GET_DBHANDLE(threadID);
    MCS_DM_QUERYDYNDATA_ACK *ptMcsDynCtxNoUniqAck = &g_ptVpfuShMemVar->tGwGloData.atMcsDynCtxNoUniqAck[threadID % MEDIA_THRD_NUM];
    WORD32 dwReportNum = psVpfuMcsGetAllDaAppCtxByClockStep(clockStep, dwhDB, dwhIdx, (void*)ptMcsDynCtxNoUniqAck);
    WORD32 dwIndex = 0;
    WORD32 dwCtxId = 0;
    T_psNcuDaAppCtx * ptDaAppCtx = NULL;
    T_VpfuDAProfileCtx *ptDAProfileTuple = NULL;
    BYTE bCruStatus = 0;
    for(dwIndex=0;dwIndex <EXPECT_DATA_APP_NUM && dwIndex<dwReportNum;dwIndex++)
    {
        dwCtxId = ptMcsDynCtxNoUniqAck->adwDataArea[dwIndex];
        ptDaAppCtx = psMcsGetDaAppCtxById(dwCtxId, dwhDB);
        if(NULL == ptDaAppCtx)
        {
            DEBUG_TRACE(DEBUG_LOW,"psMcsGetDaAppCtxById fail\n");
            continue;
        }
        BYTE bSubType = getDaTypeFromDataAppCtx(ptDaAppCtx);
        DEBUG_TRACE(DEBUG_LOW,"handleDataAnalysisCtxTimerClockStep\n");
        if(dwhIdx == DB_HANDLE_IDX_R_NCU_DATAAPP_EXP_CLOCK_STEP)
        {
            psNcuReportBySubType((BYTE)threadID,ptDaAppCtx,NULL, bSubType, RPT_TYPE_LOOP);
            psNcuMcsResetDaAppCtxByClockStep(clockStep+ ptDaAppCtx->ddwExpClockStep, DB_HANDLE_IDX_R_NCU_DATAAPP_EXP_CLOCK_STEP, dwCtxId, dwhDB);
        }
        else if(dwhIdx == DB_HANDLE_IDX_R_NCU_DATAAPP_ANA_CLOCK_STEP)
        {
            if(1 == ptDaAppCtx->bHasPoor)
            {
                ptDAProfileTuple = (T_VpfuDAProfileCtx *)ptDaAppCtx->ptDAProfileTuple;
                MCS_CHK_CONTINUE(NULL == ptDAProfileTuple);

                bCruStatus = (0 == ptDaAppCtx->bIsDialFlg)?ptDaAppCtx->bCurQoe:ptDaAppCtx->bCurDialQoe;
                if(DA_QUALITY_POOR == bCruStatus)
                {
                    ptDaAppCtx->ddwAnaRepClockStep = milliSecToClockStep(ptDAProfileTuple->reportpoortime *1000);
                }
                else if(DA_QUALITY_GOOD == bCruStatus)
                {
                    ptDaAppCtx->ddwAnaRepClockStep = milliSecToClockStep(ptDAProfileTuple->reportgoodtime *1000);
                }
            }
            psNcuReportBySubType((BYTE)threadID,ptDaAppCtx,NULL, bSubType, RPT_TYPE_ANA);   
            psNcuMcsResetDaAppCtxByClockStep(clockStep + ptDaAppCtx->ddwAnaRepClockStep, DB_HANDLE_IDX_R_NCU_DATAAPP_ANA_CLOCK_STEP, dwCtxId, dwhDB);
        }
    }
    MCS_CHK_VOID_LIKELY(dwReportNum < EXPECT_DATA_APP_NUM);

    dwReportNum = psVpfuMcsGetAllDaAppCtxByClockStep(clockStep, dwhDB, dwhIdx, (void*)ptMcsDynCtxNoUniqAck);
    for(dwIndex=0;dwIndex <EXPECT_DATA_APP_NUM && dwIndex<dwReportNum;dwIndex++)
    {
        dwCtxId = ptMcsDynCtxNoUniqAck->adwDataArea[dwIndex];
        psNcuMcsResetDaAppCtxByClockStep(getCurClockStep()+1, dwhIdx, dwCtxId, dwhDB);
    }
}


void psNcuSetDataReportTimer(T_psNcuDaAppCtx* ptDataAppCtx, T_MediaProcThreadPara *ptMediaProcThreadPara)
{
    T_psNcuMcsPerform *ptNcuPerform = psGetPerform();
    if(NULL == ptDataAppCtx || NULL == ptMediaProcThreadPara || NULL == ptNcuPerform)
    {
        DEBUG_TRACE(DEBUG_LOW,"psNcuSetDataReportTimer ptDataAppCtx=%p\n", ptDataAppCtx);
        return;
    }
    WORD64 dwCurClockStep = getCurSetTimerClockStep();
    WORD32 dwhDB   = ptMediaProcThreadPara->dwhDBByThreadNo;
    DEBUG_TRACE(DEBUG_LOW,"psNcuSetDataReportTimer init\n");
    BYTE  bSubType = getDaTypeFromDataAppCtx(ptDataAppCtx);
    if(bSubType & enumQosAna)
    {
        MCS_LOC_STAT_EX(ptNcuPerform, qwQosAnaTimerInit, 1);
        psSetAnaTimerInit(ptDataAppCtx,dwCurClockStep,dwhDB);
        MCS_LOC_STAT_EX(ptNcuPerform, qwQosAnaTimerForExpInit, 1);
    }
    if(bSubType & enumQosExpSpecial || bSubType & enumQosExpNormal)
    {
        MCS_LOC_STAT_EX(ptNcuPerform, qwQosExpTimerInit, 1);
        
    }
    ptDataAppCtx->ddwExpClockStep = milliSecToClockStep(psGetAppExpRptTimerForAna()*1000);
    WORD64 ddwClockStepExp = dwCurClockStep +ptDataAppCtx->ddwExpClockStep;
    DEBUG_TRACE(DEBUG_LOW,"psNcuSetDataReportTimer DA_QOS_EXP,ddwClockStepExp=%llu\n",ddwClockStepExp);
    psNcuMcsUpdDaAppCtxByClockStep(ddwClockStepExp, DB_HANDLE_IDX_R_NCU_DATAAPP_EXP_CLOCK_STEP,ptDataAppCtx->dwID, dwhDB, enumUpdateIndex);

}
void psSetAnaTimerInit(T_psNcuDaAppCtx* ptDataAppCtx, WORD64 dwCurClockStep, WORD32 dwhDB)
{
    T_psNcuMcsPerform *ptNcuPerform = psGetPerform();
    if(NULL == ptDataAppCtx || NULL == ptNcuPerform)
    {
        return;
    }
    WORD32 dwCtxId = ptDataAppCtx->dwID;

    T_VpfuDAProfileCtx   *ptDAProfileTuple  = NULL;
    if(TRUE != queryDAProfileCtx((lp_r_dyn_daprofile_idx_application)ptDataAppCtx->subAppidStr,&ptDAProfileTuple))
    {
        MCS_LOC_STAT_EX(ptNcuPerform, qwQueryDAProfileCtxFail, 1);
        DEBUG_TRACE(DEBUG_LOW,"psNcuSetDataReportTimer get ptDAProfileTuple fail!\n");
        return;
    }
    ptDataAppCtx->ptDAProfileTuple = ptDAProfileTuple;
    ptDataAppCtx->ddwAnaMonClockStep = milliSecToClockStep(ptDAProfileTuple->detecttime*1000);
    WORD64 ddwClockStepAnaMon = dwCurClockStep + ptDataAppCtx->ddwAnaMonClockStep;
    DEBUG_TRACE(DEBUG_LOW,"psNcuSetDataReportTimer DA_QOS_ANA,ddwClockStepAnaMon=%llu\n",ddwClockStepAnaMon);
    MCS_LOC_STAT_EX(ptNcuPerform, qwQosAnaMonTimerInit, 1);
    psNcuMcsUpdDaAppCtxByClockStep(ddwClockStepAnaMon, DB_HANDLE_IDX_R_NCU_DATAAPP_ANA_MON_CLOCK_STEP,dwCtxId, dwhDB, enumUpdateIndex);

}

void psNcuReportBySubType(BYTE bThreadNo, T_psNcuDaAppCtx *ptDataAppCtx, T_psNcuFlowCtx *ptStmAddr,BYTE bSubType, BYTE bRptType)
{
    T_psNcuMcsPerform* ptNcuPerform = psGetPerform();
    if(NULL == ptDataAppCtx || NULL == ptNcuPerform)  //ptStmAddr可以为空
    {
        return;
    }
    if((bRptType == RPT_TYPE_LOOP || bRptType == RPT_TYPE_STREAM_DEL))
    {
        psNcuUpdAllStmDataToDataApp(ptDataAppCtx,FALSE, DA_QOS_EXP);
    }
    if(bSubType & enumQosExpNormal)
    {
        MCS_LOC_STAT_EX(ptNcuPerform, qwQosExpNormalReport, 1);
        DEBUG_TRACE(DEBUG_LOW,"enumQosExpNormal!\n");
        psNcuReportUDPToNwdafProc(ptDataAppCtx, bThreadNo, bRptType, enumQosExpNormal);
    } 
    if(bSubType & enumQosExpSpecial)
    {
        DEBUG_TRACE(DEBUG_LOW,"enumQosExpSpecial!\n");
        MCS_LOC_STAT_EX(ptNcuPerform, qwQosExpSpecialReport, 1);
        psNcuReportUDPToNwdafProc(ptDataAppCtx, bThreadNo, bRptType, enumQosExpSpecial);
    }
    
    if(bSubType & enumQosAna)
    {
        switch(bRptType)
        {
            case RPT_TYPE_LOOP:
                if(psGetAppExpSwitchForAnaUser())
                {
                    MCS_LOC_STAT_EX(ptNcuPerform, qwQosAnaAppExpReport, 1);
                    DEBUG_TRACE(DEBUG_LOW,"qwQosAnaAppExpReport loop!\n");
                    psNcuReportToUpmProc(ptDataAppCtx,ptStmAddr, bThreadNo, bRptType);
                }
                break;
            default:
                MCS_LOC_STAT_EX(ptNcuPerform, qwQosAnaReport, 1);
                DEBUG_TRACE(DEBUG_LOW,"enumQosAna!\n");
                psNcuReportToUpmProc(ptDataAppCtx,ptStmAddr, bThreadNo, bRptType);
                break;
        }
    } 
    if((bRptType == RPT_TYPE_LOOP|| bRptType == RPT_TYPE_STREAM_DEL))
    {
        psNcuRestoreDataAppAfterRpt(ptDataAppCtx, bThreadNo);
    }
    
}
