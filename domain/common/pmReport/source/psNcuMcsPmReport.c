
#include "tulip.h"
#include "psNcuMcsPmReport.h"
#include "perfcounterdef.h"
#include "pmm_api.h"
#include "MemShareCfg.h"
#include "psUpfSCTypes.h"
#include "sccu_client.h"
#include "dpathreadpub.h"
#include "ps_ncu_typedef.h"
#include "xdb_pub_pfu.h"
#include "ps_db_define_ncu.h"
#include "psNcuHeartBeatProc.h"


BYTE   g_55300init = 0;
WORD16 g_55300cpuload[MAX_PS_THREAD_INST_NUM][3];
WORD16 g_55301cpuload[3] = {91, 91, 91};

BYTE   g_switch = 1;
WORD32 g_threshold = 30;

#define MAX_CPU_LOAD_RECORD   180
WORD32  g_dwMaxRecord = MAX_CPU_LOAD_RECORD;
WORD64  g_qwCurRecord = 0;
WORD32  g_dwRecord[MAX_PS_THREAD_INST_NUM][MAX_CPU_LOAD_RECORD] ={{0}};
WORD32  g_dwMaxCpuLoadInRecord[MAX_PS_THREAD_INST_NUM] = {0};

BYTE   g_NcuPmPrint = 0;
#define NCU_PM_PRINTF(...)\
{\
    if(g_NcuPmPrint)\
    {\
        zte_printf_s(__VA_ARGS__);\
    }\
}

/*性能统计总入口*/
void psNcuMcsPmmCfgMeasure(VOID)
{
    /*PO 55300/55301性能统计*/
    psVcpuLoadPM();

    /*PO 55302 NCU资源测量*/
    psNcuResourceMeasurement();

    /*PO 55303 NCU控制流测量*/
    psNcuCtrlFlowStatPm();

    /*PO 55304 NCU用户流测量*/
    psNcuUserFlowStatPm();

    return;
}

WORD32 ScGetSelfLogNo(void)
{
    WORD32 dwLogic = SCCUAGetSelfSCLogicNo();
    if(dwLogic == 0xffffffff || dwLogic == 0)
    {
        return INVALID_LOGIC_NO;
    }
    return dwLogic;
}

VOID getThreadName(CHAR *auThreadName, const CHAR *auSC_TypeName, WORD32 dwNCU_ID, BYTE bvCPUNo)
{
    MCS_PCLINT_NULLPTR_RET_VOID(auThreadName);
    MCS_PCLINT_NULLPTR_RET_VOID(auSC_TypeName);

    char    acNCU_ID[12];
    char    acCPUNo[4];
    int     Res = 0;
    strncat (auThreadName,auSC_TypeName,strlen(auSC_TypeName));
    strncat (auThreadName,"_",1);
    Res = sprintf(acNCU_ID, "%d", dwNCU_ID);
    if(Res<0)
    {
        NCU_PM_PRINTF("getThreadName sprintf err \n");
        return;
    }
    strncat (auThreadName,acNCU_ID,strlen(acNCU_ID));
    strncat (auThreadName,"_",1);
    Res = sprintf(acCPUNo, "%d", bvCPUNo);
    if(Res<0)
    {
        NCU_PM_PRINTF("getThreadName sprintf err \n");
        return;
    }
    strncat (auThreadName,acCPUNo,strlen(acCPUNo));
    return;
}

VOID psVcpuLoadPMByThreadPara(T_PSThreadInstAffinityPara *ptPSThreadInstAffinityPara)
{
    BYTE   bInstNum    = 0;
    BYTE   bvCPUIndex  = 0;  /*容器内dpdk编号*/
    BYTE   bvCPUNo     = 0;  /*虚机内编号*/
    BYTE   bSthr       = 0;
    WORD32 dwMemHandle = 0;
    WORD32 dwIndex     = 0;
    WORD32 dwNCU_ID    = 0;
    WORD64 qwPmDlt     = 0;

    MCS_PCLINT_NULLPTR_RET_VOID(ptPSThreadInstAffinityPara);

    bInstNum = ptPSThreadInstAffinityPara->bInstNum;
    if(0 == g_55300init)
    {
        for(dwIndex = 0; (dwIndex < bInstNum)&&(dwIndex < MAX_PS_THREAD_INST_NUM); dwIndex++)
        {
            g_55300cpuload[dwIndex][0] = 91;
            g_55300cpuload[dwIndex][1] = 91;
            g_55300cpuload[dwIndex][2] = 91;

        }
        g_55300init = 1;
    }
    dwNCU_ID = ScGetSelfLogNo();
    if (INVALID_LOGIC_NO == dwNCU_ID)
    {
        NCU_PM_PRINTF("get selfLogicNo err\n");
        return;
    }

    for(dwIndex = 0; (dwIndex < bInstNum)&&(dwIndex < MAX_PS_THREAD_INST_NUM); dwIndex++)
    {
        CHAR  auThreadName[64] = {0};
        BYTE * abObj = NULL;
        bvCPUIndex = ptPSThreadInstAffinityPara->atThreadParaList[dwIndex].bvCPUIndex;
        bvCPUNo    = ptPSThreadInstAffinityPara->atThreadParaList[dwIndex].bvCPUNo;
        bSthr      = ptPSThreadInstAffinityPara->atThreadParaList[dwIndex].bSoftThreadNo % MEDIA_THRD_NUM;
        getThreadName(auThreadName, SC_NAME_NEAR_COMPUTING, dwNCU_ID, bvCPUNo);
        abObj = (BYTE*)auThreadName;

        if(TRUE == ptPSThreadInstAffinityPara->atThreadParaList[dwIndex].bNeedSleep)
        {
            qwPmDlt = XOS_GetSingleCpuRate(bvCPUNo);
        }
        else
        {
            qwPmDlt = dpaGetcpuloadWithPerf(bvCPUIndex);
        }
        NCU_PM_PRINTF("auThreadName:%s;  bvCPUIndex:%d ,bvCPUNo:%d,bNeedSleep:%d,qwPmDlt:%llu \n",auThreadName,bvCPUIndex,bvCPUNo,
        ptPSThreadInstAffinityPara->atThreadParaList[dwIndex].bNeedSleep, qwPmDlt);

        WORD64 qwPmDltReal = qwPmDlt;
        if (1 == g_switch)
        {
            WORD16 qwCpuAver = 0;
            WORD16 qwThreshold = 0;
            qwCpuAver = (g_55300cpuload[bvCPUIndex%MAX_PS_THREAD_INST_NUM][0] + g_55300cpuload[bvCPUIndex%MAX_PS_THREAD_INST_NUM][1] + g_55300cpuload[bvCPUIndex%MAX_PS_THREAD_INST_NUM][2])/3;
            qwThreshold = qwCpuAver + g_threshold;
            if (qwPmDlt > qwThreshold && qwThreshold > 0)
            {
                qwPmDlt = qwCpuAver;
            }
        }
        NCU_PM_PRINTF("auThreadName:%s;  cpuload:%llu \n",auThreadName, qwPmDlt);

        /*55300*/
        dwMemHandle = MeasureGetMemHandle(C55300011);
        MeasureAddSafe(C55300011, abObj, qwPmDlt, dwMemHandle);
        MeasureAddSafe(C55300012, abObj, qwPmDlt, dwMemHandle);
        MeasureAddSafe(C55300013, abObj, qwPmDlt, dwMemHandle);

        g_55300cpuload[bvCPUIndex%MAX_PS_THREAD_INST_NUM][0] = g_55300cpuload[bvCPUIndex%MAX_PS_THREAD_INST_NUM][1];
        g_55300cpuload[bvCPUIndex%MAX_PS_THREAD_INST_NUM][1] = g_55300cpuload[bvCPUIndex%MAX_PS_THREAD_INST_NUM][2];
        g_55300cpuload[bvCPUIndex%MAX_PS_THREAD_INST_NUM][2] = qwPmDltReal;
    }

    return;
}

VOID psMcsVcpuLoadPM(T_PSThreadInstAffinityPara *ptPSThreadInstAffinityPara)
{
    BYTE * abObj            = NULL;
    BYTE   bvCPUNo          = 0;
    BYTE   dwInstNum        = 0;
    BYTE   bvCPUIndex       = 0;
    WORD32 dwIndex          = 0;
    WORD32 dwNCU_ID         = 0;
    WORD32 dwMemHandle      = 0;
    WORD32 dwTotalRecordNum = 0;
    WORD64 qwPmDlt          = 0;
    WORD64 qwCpuLoad        = 0;
    WORD64 qwCpuLoadAll     = 0;

    MCS_PCLINT_NULLPTR_RET_VOID(ptPSThreadInstAffinityPara);
    if (g_dwMaxRecord > MAX_CPU_LOAD_RECORD) 
    {
        g_dwMaxRecord = MAX_CPU_LOAD_RECORD;
    }
    dwTotalRecordNum = MIN(g_dwMaxRecord,g_qwCurRecord);
    dwInstNum = ptPSThreadInstAffinityPara->bInstNum;
    if (0 == dwInstNum)
    {
        return;
    }
    for(dwIndex = 0; (dwIndex < dwInstNum)&&(dwIndex < MAX_PS_THREAD_INST_NUM); dwIndex++)
    {
        bvCPUNo    = ptPSThreadInstAffinityPara->atThreadParaList[dwIndex].bvCPUNo;/*虚机内编号*/
        bvCPUIndex = ptPSThreadInstAffinityPara->atThreadParaList[dwIndex].bvCPUIndex;/*容器内dpdk编号*/
        /*非独占核,使用平台接口*/
        if (TRUE == ptPSThreadInstAffinityPara->atThreadParaList[dwIndex].bNeedSleep)
        {
            qwCpuLoad = XOS_GetSingleCpuRate(bvCPUNo);
        }
        else
        {
            /*独占核,使用dpa接口*/
            qwCpuLoad = pdaGetcpuload(bvCPUIndex);
        }

        qwCpuLoadAll += qwCpuLoad;
        g_dwRecord[dwIndex][g_qwCurRecord%g_dwMaxRecord] =  qwCpuLoad;
        WORD32 dwTmpRecord =0;
        g_dwMaxCpuLoadInRecord[dwIndex] = 0;
        for(; dwTmpRecord < dwTotalRecordNum; dwTmpRecord++)
        {
            if (g_dwMaxCpuLoadInRecord[dwIndex] < g_dwRecord[dwIndex][dwTmpRecord])
            {
                g_dwMaxCpuLoadInRecord[dwIndex] =  g_dwRecord[dwIndex][dwTmpRecord];
            }
        }
    }
    g_qwCurRecord ++;
    /*55301*/
    dwNCU_ID = ScGetSelfLogNo();
    if (INVALID_LOGIC_NO == dwNCU_ID)
    {
        NCU_PM_PRINTF("get selfLogicNo err\n");
        return;
    }
    abObj = (BYTE*)(&dwNCU_ID);
    dwMemHandle = MeasureGetMemHandle(C55301011);
    qwPmDlt = qwCpuLoadAll/dwInstNum;
    WORD64 qwPmDltReal = qwPmDlt;

    if (1 == g_switch)
    {
        WORD16 wCpuAver = 0;
        WORD16 wThreshold = 0;
        wCpuAver = (g_55301cpuload[0] + g_55301cpuload[1] + g_55301cpuload[2])/3;
        wThreshold = wCpuAver + g_threshold;
        if (qwPmDlt > wThreshold && wThreshold > 0)
        {
            qwPmDlt = wCpuAver;
        }
    }
    MeasureAddSafe(C55301011, abObj, qwPmDlt, dwMemHandle);
    MeasureAddSafe(C55301012, abObj, qwPmDlt, dwMemHandle);

    g_55301cpuload[0] = g_55301cpuload[1];
    g_55301cpuload[1] = g_55301cpuload[2];
    g_55301cpuload[2] = qwPmDltReal;

    return;
}

VOID psVcpuLoadPmForScan(T_PSThreadInstAffinityPara *ptPSThreadInstAffinityPara)
{    
    WORD32 dwMemHandle = 0;
    WORD64 qwPmDlt = 0;
    BYTE bInstNum = 0;
    WORD32 dwSelfLogicNo = 0;
    WORD32 dwIndex = 0;

    dwSelfLogicNo = ScGetSelfLogNo();
    if (INVALID_LOGIC_NO == dwSelfLogicNo)
    {
        NCU_PM_PRINTF("get selfLogicNo err\n");
        return;
    }
    MCS_PCLINT_NULLPTR_RET_VOID(ptPSThreadInstAffinityPara);

    bInstNum = ptPSThreadInstAffinityPara->bInstNum;
    for(dwIndex = 0; (dwIndex < bInstNum)&&(dwIndex < MAX_PS_THREAD_INST_NUM); dwIndex++)
    {

        BYTE * abObj = NULL;
        CHAR  auThreadName[64] = {0};
        
        /*bvCPUIndex=0，scan线程处理时，把所有共享核一起上报*/
        if(TRUE == ptPSThreadInstAffinityPara->atThreadParaList[dwIndex].bNeedSleep)
        {
            WORD32 dwShareCpuNum = psGetShareCpuNum();
            WORD32 dwShareIndex = 0;
            BYTE   bScanShareFlag = 0;/*判断scan线程是否是共享核的flag,1为共享核，0为独占核*/
            WORD32 dwScanVcpuNo = ptPSThreadInstAffinityPara->atThreadParaList[dwIndex].bvCPUNo;
            NCU_PM_PRINTF("get dwScanVcpuNo !! dwScanVcpuNo  %d  !!!!!\n",dwScanVcpuNo);
            NCU_PM_PRINTF("get dwShareCpuNum !! dwShareCpuNum  %d  !!!!!\n",dwShareCpuNum);
            
            for(dwShareIndex = 0;dwShareIndex < dwShareCpuNum && dwShareIndex < MAX_PS_THREAD_INST_NUM;dwShareIndex++)
            {
                WORD32 dwShareVcpuNo = psGetShareCpuByIndex(dwShareIndex);
                NCU_PM_PRINTF("get ShareVcpuNo !! dwShareVcpuNo  %d  !!!!!\n",dwShareVcpuNo);
                _mcs_if(dwShareVcpuNo == dwScanVcpuNo)
                {
                    bScanShareFlag = 1;
                }
                qwPmDlt = XOS_GetSingleCpuRate(dwShareVcpuNo);
             

                getThreadName(auThreadName, SC_NAME_NEAR_COMPUTING, dwSelfLogicNo, dwShareVcpuNo);
                abObj = (BYTE*)auThreadName;
                NCU_PM_PRINTF("auThreadName:%s;  cpuload:%llu \n",auThreadName, qwPmDlt);
                /*55300*/
                dwMemHandle = MeasureGetMemHandle(C55300011);
                MeasureAddSafe(C55300011, abObj, qwPmDlt, dwMemHandle);
                MeasureAddSafe(C55300012, abObj, qwPmDlt, dwMemHandle);
                MeasureAddSafe(C55300013, abObj, qwPmDlt, dwMemHandle);
                memset(auThreadName, 0, 64);

            }
            _mcs_if(0 == bScanShareFlag && 0 != dwScanVcpuNo)/*scan非共享核单独上报，一般为测试场景*/
            {    
                qwPmDlt =(BYTE)pdaGetcpuload(ptPSThreadInstAffinityPara->atThreadParaList[dwIndex].bvCPUIndex) ;/*scan核为共享核，使用这个接口*/

                getThreadName(auThreadName, SC_NAME_NEAR_COMPUTING, dwSelfLogicNo, dwScanVcpuNo);
                abObj = (BYTE*)auThreadName;
                NCU_PM_PRINTF("auThreadName:%s;  cpuload:%llu \n",auThreadName, qwPmDlt);
                /*55300*/
                dwMemHandle = MeasureGetMemHandle(C55300011);
                MeasureAddSafe(C55300011, abObj, qwPmDlt, dwMemHandle);
                MeasureAddSafe(C55300012, abObj, qwPmDlt, dwMemHandle);
                MeasureAddSafe(C55300013, abObj, qwPmDlt, dwMemHandle);
                memset(auThreadName, 0, 64);

            }

        }
    }
}

VOID psVcpuLoadPM()
{
    MCS_PCLINT_NULLPTR_RET_VOID(g_ptVpfuShMemVar);
    T_PSThreadInstAffinityPara *ptPSThreadInstAffinityPara = NULL;
    T_PSThreadInstAffinityPara  tScanAffinityPara = {0};

    ptPSThreadInstAffinityPara = &(g_ptVpfuShMemVar->tVpfuMcsThreadPara);
    psVcpuLoadPMByThreadPara(ptPSThreadInstAffinityPara);    //55300
    psMcsVcpuLoadPM(ptPSThreadInstAffinityPara);             //55301

    ptPSThreadInstAffinityPara = &(g_ptVpfuShMemVar->tVpfuRcvThreadPara);
    psVcpuLoadPMByThreadPara(ptPSThreadInstAffinityPara);

    if (THDM_FAIL != psGetMediaThreadInfo(THDM_MEDIA_TYPE_PFU_RSC_SCAN,&tScanAffinityPara))
    {
        ptPSThreadInstAffinityPara = &tScanAffinityPara;
        psVcpuLoadPmForScan(ptPSThreadInstAffinityPara);
    }

    return;
}

WORD64 getNcuMediaSessionNum(void)
{
    BYTE    bInstNum;
    BYTE    bSoftThreadNo   = 0;
    DWORD   dwDBHandle;
    WORD32  dwRet           = 0;
    WORD64  qwSessionNumAll = 0;
    LPT_PfuTableReg            pTblReg = NULL;
    T_PSThreadInstAffinityPara tInstPara = {0};

    dwRet = psGetMediaThreadInfo(THDM_MEDIA_TYPE_PFU_MEDIA_PROC, &tInstPara);
    if ((THDM_SUCCESS != dwRet) || (tInstPara.bInstNum > MAX_PS_THREAD_INST_NUM))
    {
        return 0;
    }
    for(bInstNum = 0; bInstNum < tInstPara.bInstNum; bInstNum++)
    {
        bSoftThreadNo = tInstPara.atThreadParaList[bInstNum].bSoftThreadNo;
        dwDBHandle    = _PFU_GET_DBHANDLE(bSoftThreadNo);
        pTblReg       = xdb_Pfu_Get_TableReg(dwDBHandle, DB_HANDLE_R_NCUSESSION);
        _DB_STATEMENT_TRUE_CONTINUE(NULL == pTblReg);
        qwSessionNumAll += pTblReg->dwTupleNum;
    }
    NCU_PM_PRINTF("qwSessionNumAll: %llu \n", qwSessionNumAll);

    return qwSessionNumAll;
}

VOID psNcuResourceMeasurement()
{
    MCS_PCLINT_NULLPTR_RET_VOID(g_ptVpfuShMemVar);

    WORD32 dwID        = 0;
    WORD64 qwPmDlt     = 0;
    WORD32 dwMemHandle = 0;

    T_NcuSessionStat *ptNcuSessionStat        = NULL;
    T_NcuSessionStat  tNcuSessionStatStat     = {0};

    for(dwID = 0; dwID < MEDIA_THRD_NUM; dwID++)
    {
        ptNcuSessionStat = &(g_ptVpfuShMemVar->tNcuSessionStat[dwID]);

        tNcuSessionStatStat.qwQualityAssuranceSessions           += ptNcuSessionStat->qwQualityAssuranceSessions;
        tNcuSessionStatStat.qwKeyUserExperienceDataSessions      += ptNcuSessionStat->qwKeyUserExperienceDataSessions;
        tNcuSessionStatStat.qwRegularUserExperienceDataSessions  += ptNcuSessionStat->qwRegularUserExperienceDataSessions;
    }

    dwMemHandle = MeasureGetMemHandle(C55302011);
    qwPmDlt     = getNcuMediaSessionNum();
    MeasureAddSafe(C55302011,NULL,qwPmDlt,dwMemHandle);
    MeasureAddSafe(C55302012,NULL,qwPmDlt,dwMemHandle);
    MeasureAddSafe(C55302013,NULL,qwPmDlt,dwMemHandle);

    qwPmDlt = tNcuSessionStatStat.qwQualityAssuranceSessions;
    MeasureAddSafe(C55302014,NULL,qwPmDlt,dwMemHandle);

    qwPmDlt = tNcuSessionStatStat.qwKeyUserExperienceDataSessions;
    MeasureAddSafe(C55302015,NULL,qwPmDlt,dwMemHandle);

    qwPmDlt = tNcuSessionStatStat.qwRegularUserExperienceDataSessions;
    MeasureAddSafe(C55302016,NULL,qwPmDlt,dwMemHandle);

    return;
}

VOID psNcuCtrlFlowStatPm()
{
    MCS_PCLINT_NULLPTR_RET_VOID(g_ptVpfuShMemVar);

    WORD32 dwID        = 0;
    WORD64 qwPmDlt     = 0;
    WORD32 dwMemHandle = 0;

    T_NcuCtrlFlowStatPm *ptNcuCtrlFlow        = NULL;
    T_NcuCtrlFlowStatPm *ptNcuCtrlFlowHistory = NULL;
    T_NcuCtrlFlowStatPm  tNcuCtrlFlowStat     = {0};

    ptNcuCtrlFlowHistory     = &(g_ptVpfuShMemVar->tNcuCtrlFlow[MEDIA_THRD_NUM]);

    for(dwID = 0; dwID < MEDIA_THRD_NUM; dwID++)
    {
        ptNcuCtrlFlow = &(g_ptVpfuShMemVar->tNcuCtrlFlow[dwID]);

        tNcuCtrlFlowStat.qwNcuRecvSessionSyncMsgNum                  += ptNcuCtrlFlow->qwNcuRecvSessionSyncMsgNum;
        tNcuCtrlFlowStat.qwNcuRecvSessionNewMsgNum                   += ptNcuCtrlFlow->qwNcuRecvSessionNewMsgNum;
        tNcuCtrlFlowStat.qwNcuRecvSessionModifyMsgNum                += ptNcuCtrlFlow->qwNcuRecvSessionModifyMsgNum;
        tNcuCtrlFlowStat.qwNcuRecvSessionDeleteMsgNum                += ptNcuCtrlFlow->qwNcuRecvSessionDeleteMsgNum;
        tNcuCtrlFlowStat.qwNcuRecvSubscribeMsgNum                    += ptNcuCtrlFlow->qwNcuRecvSubscribeMsgNum;
        tNcuCtrlFlowStat.qwNcuRecvSubscribeNewMsgNum                 += ptNcuCtrlFlow->qwNcuRecvSubscribeNewMsgNum;
        tNcuCtrlFlowStat.qwNcuRecvSubscribeModifyMsgNum              += ptNcuCtrlFlow->qwNcuRecvSubscribeModifyMsgNum;
        tNcuCtrlFlowStat.qwNcuRecvSubscribeCancelMsgNum              += ptNcuCtrlFlow->qwNcuRecvSubscribeCancelMsgNum;
        tNcuCtrlFlowStat.qwNcuRecvQoSSubscribeMsgFromN4Num           += ptNcuCtrlFlow->qwNcuRecvQoSSubscribeMsgFromN4Num;
        tNcuCtrlFlowStat.qwNcuRecvQoSNewSubscribeMsgFromN4Num        += ptNcuCtrlFlow->qwNcuRecvQoSNewSubscribeMsgFromN4Num;
        tNcuCtrlFlowStat.qwNcuRecvQoSModifySubscribeMsgFromN4Num     += ptNcuCtrlFlow->qwNcuRecvQoSModifySubscribeMsgFromN4Num;
        tNcuCtrlFlowStat.qwNcuRecvQoSCancelSubscribeMsgFromN4Num     += ptNcuCtrlFlow->qwNcuRecvQoSCancelSubscribeMsgFromN4Num;
        tNcuCtrlFlowStat.qwNcuRecvExpDataSubscribeMsgFromN4Num       += ptNcuCtrlFlow->qwNcuRecvExpDataSubscribeMsgFromN4Num;
        tNcuCtrlFlowStat.qwNcuRecvExpDataNewSubscribeMsgFromN4Num    += ptNcuCtrlFlow->qwNcuRecvExpDataNewSubscribeMsgFromN4Num;
        tNcuCtrlFlowStat.qwNcuRecvExpDataModifySubscribeMsgFromN4Num += ptNcuCtrlFlow->qwNcuRecvExpDataModifySubscribeMsgFromN4Num;
        tNcuCtrlFlowStat.qwNcuRecvExpDataCancelSubscribeMsgFromN4Num += ptNcuCtrlFlow->qwNcuRecvExpDataCancelSubscribeMsgFromN4Num;
        tNcuCtrlFlowStat.qwNcuRecvExpDataSubscribeMsgFromSBI         += ptNcuCtrlFlow->qwNcuRecvExpDataSubscribeMsgFromSBI;
        tNcuCtrlFlowStat.qwNcuRecvExpDataNewSubscribeMsgFromSBI      += ptNcuCtrlFlow->qwNcuRecvExpDataNewSubscribeMsgFromSBI;
        tNcuCtrlFlowStat.qwNcuRecvExpDataModifySubscribeMsgFromSBI   += ptNcuCtrlFlow->qwNcuRecvExpDataModifySubscribeMsgFromSBI;
        tNcuCtrlFlowStat.qwNcuRecvExpDataCancelSubscribeMsgFromSBI   += ptNcuCtrlFlow->qwNcuRecvExpDataCancelSubscribeMsgFromSBI;
        tNcuCtrlFlowStat.qwNcuReportQoSNumViaSBI                     += ptNcuCtrlFlow->qwNcuReportQoSNumViaSBI;
        tNcuCtrlFlowStat.qwNcuReportKeyExpDataNumViaUDP              += ptNcuCtrlFlow->qwNcuReportKeyExpDataNumViaUDP;
        tNcuCtrlFlowStat.qwNcuReportOrdinaryExpDataNumViaUDP         += ptNcuCtrlFlow->qwNcuReportOrdinaryExpDataNumViaUDP;
        tNcuCtrlFlowStat.qwContextPullRequestCount                   += ptNcuCtrlFlow->qwContextPullRequestCount;
        tNcuCtrlFlowStat.qwNcuReportKeyExpDataNumViaSBI              += ptNcuCtrlFlow->qwNcuReportKeyExpDataNumViaSBI;
    }

    dwMemHandle = MeasureGetMemHandle(C55303011);
    qwPmDlt = tNcuCtrlFlowStat.qwNcuRecvSessionSyncMsgNum - ptNcuCtrlFlowHistory->qwNcuRecvSessionSyncMsgNum;
    MeasureAddSafe(C55303011, NULL, qwPmDlt, dwMemHandle);
    
    qwPmDlt = tNcuCtrlFlowStat.qwNcuRecvSessionNewMsgNum - ptNcuCtrlFlowHistory->qwNcuRecvSessionNewMsgNum;
    MeasureAddSafe(C55303012, NULL, qwPmDlt, dwMemHandle);
    
    qwPmDlt = tNcuCtrlFlowStat.qwNcuRecvSessionModifyMsgNum - ptNcuCtrlFlowHistory->qwNcuRecvSessionModifyMsgNum;
    MeasureAddSafe(C55303013, NULL, qwPmDlt, dwMemHandle);
    
    qwPmDlt = tNcuCtrlFlowStat.qwNcuRecvSessionDeleteMsgNum - ptNcuCtrlFlowHistory->qwNcuRecvSessionDeleteMsgNum;
    MeasureAddSafe(C55303014, NULL, qwPmDlt, dwMemHandle);
    
    qwPmDlt = tNcuCtrlFlowStat.qwNcuRecvSubscribeMsgNum - ptNcuCtrlFlowHistory->qwNcuRecvSubscribeMsgNum;
    MeasureAddSafe(C55303015, NULL, qwPmDlt, dwMemHandle);
    
    qwPmDlt = tNcuCtrlFlowStat.qwNcuRecvSubscribeNewMsgNum - ptNcuCtrlFlowHistory->qwNcuRecvSubscribeNewMsgNum;
    MeasureAddSafe(C55303016, NULL, qwPmDlt, dwMemHandle);
    
    qwPmDlt = tNcuCtrlFlowStat.qwNcuRecvSubscribeModifyMsgNum - ptNcuCtrlFlowHistory->qwNcuRecvSubscribeModifyMsgNum;
    MeasureAddSafe(C55303017, NULL, qwPmDlt, dwMemHandle);
    
    qwPmDlt = tNcuCtrlFlowStat.qwNcuRecvSubscribeCancelMsgNum - ptNcuCtrlFlowHistory->qwNcuRecvSubscribeCancelMsgNum;
    MeasureAddSafe(C55303018, NULL, qwPmDlt, dwMemHandle);
    
    qwPmDlt = tNcuCtrlFlowStat.qwNcuRecvQoSSubscribeMsgFromN4Num - ptNcuCtrlFlowHistory->qwNcuRecvQoSSubscribeMsgFromN4Num;
    MeasureAddSafe(C55303019, NULL, qwPmDlt, dwMemHandle);
    
    qwPmDlt = tNcuCtrlFlowStat.qwNcuRecvQoSNewSubscribeMsgFromN4Num - ptNcuCtrlFlowHistory->qwNcuRecvQoSNewSubscribeMsgFromN4Num;
    MeasureAddSafe(C55303020, NULL, qwPmDlt, dwMemHandle);
    
    qwPmDlt = tNcuCtrlFlowStat.qwNcuRecvQoSModifySubscribeMsgFromN4Num - ptNcuCtrlFlowHistory->qwNcuRecvQoSModifySubscribeMsgFromN4Num;
    MeasureAddSafe(C55303021, NULL, qwPmDlt, dwMemHandle);
    
    qwPmDlt = tNcuCtrlFlowStat.qwNcuRecvQoSCancelSubscribeMsgFromN4Num - ptNcuCtrlFlowHistory->qwNcuRecvQoSCancelSubscribeMsgFromN4Num;
    MeasureAddSafe(C55303022, NULL, qwPmDlt, dwMemHandle);
    
    qwPmDlt = tNcuCtrlFlowStat.qwNcuRecvExpDataSubscribeMsgFromN4Num - ptNcuCtrlFlowHistory->qwNcuRecvExpDataSubscribeMsgFromN4Num;
    MeasureAddSafe(C55303023, NULL, qwPmDlt, dwMemHandle);
    
    qwPmDlt = tNcuCtrlFlowStat.qwNcuRecvExpDataNewSubscribeMsgFromN4Num - ptNcuCtrlFlowHistory->qwNcuRecvExpDataNewSubscribeMsgFromN4Num;
    MeasureAddSafe(C55303024, NULL, qwPmDlt, dwMemHandle);
    
    qwPmDlt = tNcuCtrlFlowStat.qwNcuRecvExpDataModifySubscribeMsgFromN4Num - ptNcuCtrlFlowHistory->qwNcuRecvExpDataModifySubscribeMsgFromN4Num;
    MeasureAddSafe(C55303025, NULL, qwPmDlt, dwMemHandle);
    
    qwPmDlt = tNcuCtrlFlowStat.qwNcuRecvExpDataCancelSubscribeMsgFromN4Num - ptNcuCtrlFlowHistory->qwNcuRecvExpDataCancelSubscribeMsgFromN4Num;
    MeasureAddSafe(C55303026, NULL, qwPmDlt, dwMemHandle);
    
    qwPmDlt = tNcuCtrlFlowStat.qwNcuRecvExpDataSubscribeMsgFromSBI - ptNcuCtrlFlowHistory->qwNcuRecvExpDataSubscribeMsgFromSBI;
    MeasureAddSafe(C55303027, NULL, qwPmDlt, dwMemHandle);
    
    qwPmDlt = tNcuCtrlFlowStat.qwNcuRecvExpDataNewSubscribeMsgFromSBI - ptNcuCtrlFlowHistory->qwNcuRecvExpDataNewSubscribeMsgFromSBI;
    MeasureAddSafe(C55303028, NULL, qwPmDlt, dwMemHandle);
    
    qwPmDlt = tNcuCtrlFlowStat.qwNcuRecvExpDataModifySubscribeMsgFromSBI - ptNcuCtrlFlowHistory->qwNcuRecvExpDataModifySubscribeMsgFromSBI;
    MeasureAddSafe(C55303029, NULL, qwPmDlt, dwMemHandle);
    
    qwPmDlt = tNcuCtrlFlowStat.qwNcuRecvExpDataCancelSubscribeMsgFromSBI - ptNcuCtrlFlowHistory->qwNcuRecvExpDataCancelSubscribeMsgFromSBI;
    MeasureAddSafe(C55303030, NULL, qwPmDlt, dwMemHandle);
    
    qwPmDlt = tNcuCtrlFlowStat.qwNcuReportQoSNumViaSBI - ptNcuCtrlFlowHistory->qwNcuReportQoSNumViaSBI;
    MeasureAddSafe(C55303031, NULL, qwPmDlt, dwMemHandle);
    
    qwPmDlt = tNcuCtrlFlowStat.qwNcuReportKeyExpDataNumViaUDP - ptNcuCtrlFlowHistory->qwNcuReportKeyExpDataNumViaUDP;
    MeasureAddSafe(C55303032, NULL, qwPmDlt, dwMemHandle);
    
    qwPmDlt = tNcuCtrlFlowStat.qwNcuReportOrdinaryExpDataNumViaUDP - ptNcuCtrlFlowHistory->qwNcuReportOrdinaryExpDataNumViaUDP;
    MeasureAddSafe(C55303033, NULL, qwPmDlt, dwMemHandle);

    qwPmDlt = tNcuCtrlFlowStat.qwContextPullRequestCount - ptNcuCtrlFlowHistory->qwContextPullRequestCount;
    MeasureAddSafe(C55303034, NULL, qwPmDlt, dwMemHandle);

    qwPmDlt = tNcuCtrlFlowStat.qwNcuReportKeyExpDataNumViaSBI - ptNcuCtrlFlowHistory->qwNcuReportKeyExpDataNumViaSBI;
    MeasureAddSafe(C55303035, NULL, qwPmDlt, dwMemHandle);

    zte_memcpy_s(ptNcuCtrlFlowHistory, sizeof(T_NcuCtrlFlowStatPm), &tNcuCtrlFlowStat, sizeof(T_NcuCtrlFlowStatPm));

    return;
}

VOID psNcuUserFlowStatPm()
{
    MCS_PCLINT_NULLPTR_RET_VOID(g_ptVpfuShMemVar);

    WORD32 dwID        = 0;
    WORD64 qwPmDlt     = 0;
    WORD32 dwMemHandle = 0;

    T_NcuUserFlowStatPm *ptNcuUserFlow        = NULL;
    T_NcuUserFlowStatPm *ptNcuUserFlowHistory = NULL;
    T_NcuUserFlowStatPm  tNcuUserFlowStat     = {0};

    ptNcuUserFlowHistory     = &(g_ptVpfuShMemVar->tNcuUserFlow[MEDIA_THRD_NUM]);

    for(dwID = 0; dwID < MEDIA_THRD_NUM; dwID++)
    {
        ptNcuUserFlow = &(g_ptVpfuShMemVar->tNcuUserFlow[dwID]);

        tNcuUserFlowStat.qwNcuRcvCpyPktMsgs     += ptNcuUserFlow->qwNcuRcvCpyPktMsgs;
        tNcuUserFlowStat.qwNcuRcvUlCpyPkts      += ptNcuUserFlow->qwNcuRcvUlCpyPkts;
        tNcuUserFlowStat.qwNcuRcvDlCpyPkts      += ptNcuUserFlow->qwNcuRcvDlCpyPkts;
        tNcuUserFlowStat.qwNcuRcvUlCpyPktBytes  += ptNcuUserFlow->qwNcuRcvUlCpyPktBytes;
        tNcuUserFlowStat.qwNcuRcvDlCpyPktBytes  += ptNcuUserFlow->qwNcuRcvDlCpyPktBytes;
        tNcuUserFlowStat.qwNcuRcvTrafficRptMsgs          += ptNcuUserFlow->qwNcuRcvTrafficRptMsgs;
        tNcuUserFlowStat.qwNcuRcvUlTrafficRptPktsNum     += ptNcuUserFlow->qwNcuRcvUlTrafficRptPktsNum;
        tNcuUserFlowStat.qwNcuRcvDlTrafficRptPktsNum     += ptNcuUserFlow->qwNcuRcvDlTrafficRptPktsNum;
        tNcuUserFlowStat.qwNcuRcvUlTrafficRptPktsBytes   += ptNcuUserFlow->qwNcuRcvUlTrafficRptPktsBytes;
        tNcuUserFlowStat.qwNcuRcvDlTrafficRptPktsBytes   += ptNcuUserFlow->qwNcuRcvDlTrafficRptPktsBytes;
    }

    dwMemHandle = MeasureGetMemHandle(C55304011);
    qwPmDlt = tNcuUserFlowStat.qwNcuRcvCpyPktMsgs - ptNcuUserFlowHistory->qwNcuRcvCpyPktMsgs;
    MeasureAddSafe(C55304011,NULL,qwPmDlt,dwMemHandle);

    qwPmDlt = tNcuUserFlowStat.qwNcuRcvUlCpyPkts - ptNcuUserFlowHistory->qwNcuRcvUlCpyPkts;
    MeasureAddSafe(C55304012,NULL,qwPmDlt,dwMemHandle);

    qwPmDlt = tNcuUserFlowStat.qwNcuRcvDlCpyPkts - ptNcuUserFlowHistory->qwNcuRcvDlCpyPkts;
    MeasureAddSafe(C55304013,NULL,qwPmDlt,dwMemHandle);

    qwPmDlt = tNcuUserFlowStat.qwNcuRcvUlCpyPktBytes - ptNcuUserFlowHistory->qwNcuRcvUlCpyPktBytes;
    MeasureAddSafe(C55304014,NULL,qwPmDlt,dwMemHandle);

    qwPmDlt = tNcuUserFlowStat.qwNcuRcvDlCpyPktBytes - ptNcuUserFlowHistory->qwNcuRcvDlCpyPktBytes;
    MeasureAddSafe(C55304015,NULL,qwPmDlt,dwMemHandle);

    qwPmDlt = tNcuUserFlowStat.qwNcuRcvTrafficRptMsgs - ptNcuUserFlowHistory->qwNcuRcvTrafficRptMsgs;
    MeasureAddSafe(C55304016,NULL,qwPmDlt,dwMemHandle);

    qwPmDlt = tNcuUserFlowStat.qwNcuRcvUlTrafficRptPktsNum - ptNcuUserFlowHistory->qwNcuRcvUlTrafficRptPktsNum;
    MeasureAddSafe(C55304017,NULL,qwPmDlt,dwMemHandle);

    qwPmDlt = tNcuUserFlowStat.qwNcuRcvDlTrafficRptPktsNum - ptNcuUserFlowHistory->qwNcuRcvDlTrafficRptPktsNum;
    MeasureAddSafe(C55304018,NULL,qwPmDlt,dwMemHandle);

    qwPmDlt = tNcuUserFlowStat.qwNcuRcvUlTrafficRptPktsBytes - ptNcuUserFlowHistory->qwNcuRcvUlTrafficRptPktsBytes;
    MeasureAddSafe(C55304019,NULL,qwPmDlt,dwMemHandle);

    qwPmDlt = tNcuUserFlowStat.qwNcuRcvDlTrafficRptPktsBytes - ptNcuUserFlowHistory->qwNcuRcvDlTrafficRptPktsBytes;
    MeasureAddSafe(C55304020,NULL,qwPmDlt,dwMemHandle);

    zte_memcpy_s(ptNcuUserFlowHistory, sizeof(T_NcuUserFlowStatPm), &tNcuUserFlowStat, sizeof(T_NcuUserFlowStatPm));
    return;
}
