#include "psNcuScanMsgProc.h"

#include "MemShareCfg.h"
#include "psNcuCtxFunc.h"
#include "psMcsGlobalCfg.h"
#include "McsHeadCap.h"
#include "ps_db_define_pfu.h"
#include "ps_ncu_typedef.h"
#include "ps_db_define_ncu.h"
#include "ps_ncu_session.h"
#include "ps_ncu_stream.h"
#include "psNcuSnCtxProc.h"
#include "psMcsDebug.h"
#include "dpathreadpub.h"
#include "psNcuDataAnalysis.h"
#include "psNcuSubscribeProc.h"
#include "psNcuStreamProc.h"
#include "psNcuTcpCalcProc.h"
#include "ncuSCCUAbility.h"
#include "ps_mcs_define.h"
#include "zte_slibc.h"
#include "UPFHelp.h"

enum{
    APPID_RELATE_AGE_NO        = 0,
    APPID_RELATE_AGE_CHECK     = 1,
    APPID_RELATE_AGE_ALWAYS    = 2
};
WORD32 g_set_appid_relate_clear = APPID_RELATE_AGE_CHECK;

/***********************新增上下文扫描说明*******************************
1.在此处添加上下文扫描的注册
2.在配置表ResourceAge-5GMODEL.xls中添加表的老化参数(不加则为默认的换算值)
3.添加吊死检测处理函数
4.公共库的上下文扫面处理需要在psVpfuMcsIsCommDispCtx增加判断
ps:吊死检测入口函数psVpfuMcsScanProc每10ms调用一次
***************************************************************************/

T_McsTableScanTuple g_atNcucanRegTbl[4] =
{
    /*     表句柄                       开关       表名            表扫描参数          扫描处理函数     */
    {DB_HANDLE_R_NCUSESSION,            1,  "R_NCU_SESSION",         {0},   psNcuSessionAgeingFun},
    {DB_HANDLE_R_NCUFLOWCTX,            1,  "R_FLOWCTX",             {0},   psNcuFlowAgeingFun},
    {DB_HANDLE_R_NCU_SN,                1,  "R_NCU_SN",              {0},   psNcuSnAgeingFun}
};

BYTE g_bNumOfScanTask = sizeof(g_atNcucanRegTbl) / sizeof (T_McsTableScanTuple);

T_McsScanMessageFunTuple g_atNcuScanMessageFunTbl[] =
{
    {NCU_NOTIFY_SESSION_RELEASE,    0, psNcuSessionAgeingRel},
    {NCU_NOTIFY_FLOW_RELEASE,       0, psNcuFlowAgeingRel},
    {NCU_NOTIFY_SN_RELEASE,         0, psNcuSnAgeingRel}
};

WORD32 g_NumOfScanMessageFun = (sizeof(g_atNcuScanMessageFunTbl) / sizeof(T_McsScanMessageFunTuple));

VOID psNcuLoopScanMessageProcFun(WORD32 dwMessageType, WORD16 wBufferLen, T_psNcuMediaScanMsg *ptSigHead, T_MediaProcThreadPara *ptMediaProcThreadPara);
WORD32 psNcuScanPktSend(BYTE *pbMsg, WORD32 dwMsgType, WORD32 dwMsgLen, BYTE bMcsThreadNo, BYTE bScanThreadNo);
BOOLEAN psNcuCheckAgeingTimeStamp(WORD32 dwCurtime, WORD32 dwLastUpdStamp, WORD32 dwAgeTime);
VOID psNcuFlowCtxAgeingProc(WORD32 dwScanId,  BYTE bMcsThreadNo, BYTE bScanThreadNo, WORD32 dwAgeTime);
VOID psNcuSessionCtxAgeingProc(WORD32 dwScanId,  BYTE bMcsThreadNo, BYTE bScanThreadNo, WORD32 dwAgeTime);
VOID psNcuSnCtxAgeingProc(WORD32 dwScanId,  BYTE bMcsThreadNo, BYTE bScanThreadNo, WORD32 dwAgeTime);
VOID psNcuGetCtxAgeCfg(WORD32 tabHdl, const CHAR*, WORD32 *ptCtxCap, WORD32 *ptScanNum10ms, WORD32 *ptAgeTime);
VOID ncu_c_scan_entry(WORD32 scan_thread_id);

/**********************************************************************
* 函数名称： psNcuFlowAgeingProc
* 功能描述： Vpfu流上下文老化
* 输入参数： 无
* 输出参数：
* 返 回 值： 无
* 其它说明：
* 修改日期        版本号     修改人           修改内容
* --------------------------------------------------------
*2018.04.24             V1.0      wang.longbiao       Create
***********************************************************************/
VOID psNcuFlowAgeingFun(BYTE bTaskNo, BYTE bMcsThreadNo, BYTE bScanThreadNo)
{
    MCS_CHK_VOID(bTaskNo >= g_bNumOfScanTask);
    T_McsCtxScanPara *ptFlowCtxScanInfo = &(g_atNcucanRegTbl[bTaskNo].tMcsCtxScanInfo[bMcsThreadNo % MAX_PS_THREAD_INST_NUM]);
    WORD32 dwFlowCtxCap = ptFlowCtxScanInfo->dwCtxCap;
    WORD32 dwScanMax    = ptFlowCtxScanInfo->dwScanNum10ms;
    WORD32 dwAgeTime    = ptFlowCtxScanInfo->dwAgeTime;
    WORD32 dwScanId     = 0;
    WORD32 dwScanIndex  = 0;

    for(dwScanIndex = 0; dwScanIndex < dwScanMax; dwScanIndex++)
    {
        dwScanId = ptFlowCtxScanInfo->dwScanIndex++;
        if(dwScanId >= dwFlowCtxCap)
        {
            ptFlowCtxScanInfo->dwScanIndex = 1;   /*记录号从1开始有效*/
            dwScanId = dwFlowCtxCap;   /*保护一下*/
        }
        psNcuFlowCtxAgeingProc(dwScanId, bMcsThreadNo, bScanThreadNo, dwAgeTime);
    }
    return;
}

VOID psNcuFlowCtxAgeingProc(WORD32 dwScanId, BYTE bMcsThreadNo, BYTE bScanThreadNo, WORD32 dwAgeTime)
{
    T_psNcuMcsPerform *ptNcuMcsPerform = psVpfuMcsGetPerformPtr(bScanThreadNo);
    MCS_CHK_NULL_VOID(ptNcuMcsPerform);
    WORD32  hDB  = _NCU_GET_DBHANDLE(bMcsThreadNo);
    T_psNcuFlowCtx *ptNcuFlowCtx = (T_psNcuFlowCtx *)psVpfuMcsGetCtxById(dwScanId, hDB, DB_HANDLE_R_NCUFLOWCTX);
    MCS_CHK_NULL_VOID(ptNcuFlowCtx);
    WORD32 dwCurtime = psFtmGetPowerOnSec();
    WORD32 dwLastUpdStamp = ptNcuFlowCtx->dwUpdateTimeStamp;
    if(FALSE == psNcuCheckAgeingTimeStamp(dwCurtime, dwLastUpdStamp, dwAgeTime))
    {
        return;
    }
    T_psMcsCtxRelReport tMcsFlowCtxScan = {0};
    tMcsFlowCtxScan.dwMcsCtxId          = dwScanId;
    tMcsFlowCtxScan.dwStampInMcsCtx     = dwLastUpdStamp;
    if(SUCCESS == psNcuScanPktSend((BYTE *)&tMcsFlowCtxScan, NCU_NOTIFY_FLOW_RELEASE, sizeof(tMcsFlowCtxScan),
                                            bMcsThreadNo, bScanThreadNo))
    {
        MCS_LOC_STAT_EX(ptNcuMcsPerform, qwFlowAgeingRelCtx, 1);
    }
}

BOOLEAN psNcuCheckAgeingTimeStamp(WORD32 dwCurtime, WORD32 dwLastUpdStamp, WORD32 dwAgeTime)
{
    if (dwCurtime < dwLastUpdStamp)
    {
        return TRUE;
    }
    WORD64 ddwDeleteTime = (WORD64)dwLastUpdStamp + (WORD64)dwAgeTime;
    if ((WORD64)dwCurtime >= ddwDeleteTime)
    {
        return TRUE;
    }
    return FALSE;
}

/**********************************************************************
* 函数名称：psVpfuMcsGetCtxScanNum
* 功能描述：以会话表容量为基准, 按比例获取10ms扫描数量
* 输入参数：dwCtxCap：需扫描的上下文容量
*           dwMcsThreadNo: 所在线程
*           dwPorportion: 扫描比例, 1/100为单位, 100即为1:1
* 输出参数： 无
* 返 回 值： 扫描数量
* 其它说明： 无
* 修改日期        版本号        修改人        修改内容
   2018.04.11              v1.0               wang.longbiao            创建
***********************************************************************/
WORD32 MAX_SCAN_STEP = 20;
INLINE WORD32 psNcuGetCtxScanNum(WORD32 dwCtxCap,BYTE bMcsSthNo)
{
    WORD32 dwSessCtxCap = g_atNcucanRegTbl[0].tMcsCtxScanInfo[bMcsSthNo % MAX_PS_THREAD_INST_NUM].dwCtxCap;       /*0号的位置放的是会话表*/
    WORD16 wSessScanMaxNum = g_atNcucanRegTbl[0].tMcsCtxScanInfo[bMcsSthNo % MAX_PS_THREAD_INST_NUM].dwScanNum10ms; /*0号的位置放的是会话表*/
    WORD32 dwScanNum10ms = 0;
    WORD32 dwPorportion  = 100; /* 默认1:1 */

    if(0 != dwSessCtxCap)
    {
        dwPorportion = (dwCtxCap * 100)/dwSessCtxCap; /* 百分比为单位 */
    }
    dwScanNum10ms = (dwPorportion * wSessScanMaxNum)/100;
    if((0 == dwScanNum10ms) && (0 != wSessScanMaxNum))
    {
        /* 容量过小的情况，最少扫描1个 */
        dwScanNum10ms = 1;
    }
    dwScanNum10ms = MIN(dwCtxCap, dwScanNum10ms);
    dwScanNum10ms = MIN(MAX_SCAN_STEP, dwScanNum10ms);

    return dwScanNum10ms;
}

/**********************************************************************
* 函数名称： psVpfuMcsUseDefaultScanPara
* 功能描述： 如果从配置中找不到动态表的扫描参数，则采用默认值扫描
* 输入参数： 无
* 输出参数： 无
* 返 回 值： 无
* 其它说明： 此函数控制核和媒体核均可使用
* 修改日期        版本号     修改人           修改内容
* --------------------------------------------------------
* 2018.12.14                V1.0            wang.longbiao           创建
***********************************************************************/
VOID psNcuUseDefaultScanPara(T_McsTableScanTuple *ptMcsCtxScanTuple,BYTE   bMcsSthNo)
{
    MCS_CHK_NULL_VOID(ptMcsCtxScanTuple);
    bMcsSthNo = bMcsSthNo % MAX_PS_THREAD_INST_NUM;
    WORD32 dwCtxCap = ptMcsCtxScanTuple->tMcsCtxScanInfo[bMcsSthNo].dwCtxCap;

    ptMcsCtxScanTuple->tMcsCtxScanInfo[bMcsSthNo].dwAgeTime = 0;
    if(DB_HANDLE_R_NCUSESSION == ptMcsCtxScanTuple->dwMcsCtxType)
    {
        ptMcsCtxScanTuple->tMcsCtxScanInfo[bMcsSthNo].dwScanNum10ms = 10;
    }
    else
    {
        ptMcsCtxScanTuple->tMcsCtxScanInfo[bMcsSthNo].dwScanNum10ms = psNcuGetCtxScanNum(dwCtxCap,bMcsSthNo);
    }
    return;
}

//     //DB_CTX_PFU_R_DISP_TEID



BOOL IsScanNecessay(WORD32 TblHandle, BYTE bMcsThreadNo)
{
    WORD32  hDB;

    // else
        hDB     = _NCU_GET_DBHANDLE(bMcsThreadNo);

    WORD32  capacity = 0;
    WORD32  tuplenum = 0;
    if (SUCCESS == psVpfuMcsGetCapacity(TblHandle, hDB, &capacity, &tuplenum))
    {
        if (0 == tuplenum)
        {
            return FALSE;
        }
    }

    return TRUE;
}

VOID psNcuInitOneScanTaskPara(T_McsTableScanTuple *ptMcsCtxScanTuple)
{
    MCS_CHK_NULL_VOID(ptMcsCtxScanTuple);
    T_ResourceAgeCfg *ptAgeParaCfg = NULL;
    WORD32 dwhDB      = 0;
    WORD32 dwCtxCap   = 0;
    WORD32 dwRetValue = 0;
    WORD32 dwTupleNum = 0;
    WORD32 dwLoopcfg  = 0;
    BYTE   bSthIndex  = 0;
    BYTE   bMcsSthNo  = 0;
    BYTE   bInstNum   = 0;
    BYTE   ucFindFlg  = 0;

    /*获取表容量*/
    // else
        bMcsSthNo = g_ptVpfuShMemVar->tVpfuMcsThreadPara.atThreadParaList[0].bSoftThreadNo;
        dwhDB = _NCU_GET_DBHANDLE(bMcsSthNo);

    dwRetValue = psVpfuMcsGetCapacity(ptMcsCtxScanTuple->dwMcsCtxType, dwhDB, &dwCtxCap, &dwTupleNum);
    MCS_CHK_VOID(ERROR_MEDIA == dwRetValue);

    /*按媒体面线程逐个获取扫描参数*/
    bInstNum = g_ptVpfuShMemVar->tVpfuMcsThreadPara.bInstNum;
    for(bSthIndex = 0; bSthIndex < bInstNum && bSthIndex < MAX_PS_THREAD_INST_NUM; bSthIndex++)
    {
        ucFindFlg = 0;
        bMcsSthNo = g_ptVpfuShMemVar->tVpfuMcsThreadPara.atThreadParaList[bSthIndex].bSoftThreadNo % MAX_PS_THREAD_INST_NUM;

        ptMcsCtxScanTuple->tMcsCtxScanInfo[bMcsSthNo].dwScanIndex = 1;
        ptMcsCtxScanTuple->tMcsCtxScanInfo[bMcsSthNo].dwCtxCap    = dwCtxCap;

        dwTupleNum = MIN(g_upfConfig.bResourceAgeCfgAgeTableNums, R_RESOURCEAGE_CAPACITY);
        for(dwLoopcfg = 0; dwLoopcfg < dwTupleNum; dwLoopcfg++)
        {
            ptAgeParaCfg = &(g_upfConfig.resourceAgeCfg[dwLoopcfg]);
            if(0  == memcmp(ptMcsCtxScanTuple->aTableName,ptAgeParaCfg->bCtxname,LEN_R_RESOURCEAGE_CTXNAME_MAX))
            {
                ptMcsCtxScanTuple->tMcsCtxScanInfo[bMcsSthNo].dwScanNum10ms = (WORD32)ptAgeParaCfg->bScanstep;
                ptMcsCtxScanTuple->tMcsCtxScanInfo[bMcsSthNo].dwAgeTime     = ptAgeParaCfg->dwAgetime;
                ucFindFlg = 1;
                break;
            }
        }

        /*没有在OAM配置中查找到,使用默认扫描步长*/
        if(0 == ucFindFlg)
        {
            psNcuUseDefaultScanPara(ptMcsCtxScanTuple,bMcsSthNo);
        }

        /*注意:公共线程的上下文也是按照媒体面线程号扫的,但是扫的时候只扫1次*/
        if(_NCU_DBHANDLE_COMM == dwhDB)
        {
            break;
        }
    }

    return;
}

/**********************************************************************
* 函数名称： psNcuInitAgeingScan
* 功能描述： 初始化吊死扫描参数(表容量/步长等)
* 输入参数： 无
* 输出参数： 无
* 返 回 值： 无
* 其它说明： 此函数控制核和媒体核均可使用
* 修改日期        版本号     修改人           修改内容
* --------------------------------------------------------
* 2024.04.11                V1.0            m6             创建
***********************************************************************/
VOID psNcuInitAgeingScan()
{
     MCS_CHK_VOID(1 == g_upfConfig.bResourceAgeCfgScanSuccFlg);

    T_McsTableScanTuple *ptMcsCtxScanTuple = NULL;
    BYTE bTaskIndex = 0;

    /*线程有没有准备好*/
    if(0 == psVpfuGetThreadParaFlag())
    {
        return;
    }

    /*逐一对每张表进行扫描信息初始化*/
    for(bTaskIndex = 0; bTaskIndex< g_bNumOfScanTask; bTaskIndex++)
    {
        ptMcsCtxScanTuple = &g_atNcucanRegTbl[bTaskIndex];

        psNcuInitOneScanTaskPara(ptMcsCtxScanTuple);
    }

    g_upfConfig.bResourceAgeCfgScanSuccFlg = 1;
    return;
}

VOID psNcuScanParaChangeJudge()
{
    if(unlikely(1 == g_upfConfig.bResourceAgeCfgAgeParaChanged))
    {
        g_upfConfig.bResourceAgeCfgScanSuccFlg = 0;
        psNcuInitAgeingScan();
        g_upfConfig.bResourceAgeCfgAgeParaChanged = 0;
    }
    return;
}

VOID psNcuScanCtxOnSwitch(WORD32 dwThreadNo, WORD32 dwScanSwitch)
{
    BYTE bInstNum = g_ptVpfuShMemVar->tVpfuMcsThreadPara.bInstNum;    
    BYTE bMcsThr = 0xff;
    BYTE bSthIndex = 0;
    BYTE bTaskIndex = 0;
    
    for(bTaskIndex = 0; bTaskIndex < g_bNumOfScanTask; bTaskIndex++)
    {        
        MCS_CHK_CONTINUE(NULL == g_atNcucanRegTbl[bTaskIndex].pScanFun);
        MCS_CHK_CONTINUE(dwScanSwitch > g_atNcucanRegTbl[bTaskIndex].dwMcsCtxSwitch);
            
        /*扫描媒体面线程*/
        for(bSthIndex = 0; bSthIndex < bInstNum && bSthIndex < MAX_PS_THREAD_INST_NUM; bSthIndex++)
        {            
            /*获取媒体面线程*/
            bMcsThr = g_ptVpfuShMemVar->tVpfuMcsThreadPara.atThreadParaList[bSthIndex].bSoftThreadNo;

            /*调用注册的吊死检测函数*/
            if (IsScanNecessay(g_atNcucanRegTbl[bTaskIndex].dwMcsCtxType, bMcsThr))
            {
                g_atNcucanRegTbl[bTaskIndex].pScanFun(bTaskIndex, bMcsThr, dwThreadNo);
            }
        }
    }
}
/**********************************************************************
* 函数名称： psVpfuMcsScanProc
* 功能描述： 提供给vpfu扫描线程的mcs上下文扫描回调
* 输入参数： dwThreadNo:扫描线程软件线程号
* 输出参数：
* 返 回 值： 无
* 其它说明：
* 修改日期        版本号     修改人           修改内容
* --------------------------------------------------------
*2018.04.27                 V1.0              wang.longbiao       Create
***********************************************************************/
VOID psNcuScanProc(WORD32 dwThreadNo)
{
 
    if (getNcuSoftPara(5060) != 0)
    {
        ncu_c_scan_entry(dwThreadNo);
        return;
    }

    /*动态表配置的步长或老化时间发生变化*/
    psNcuScanParaChangeJudge();

    /* 老化扫描上下文 */
    psNcuScanCtxOnSwitch(dwThreadNo, MCS_AGING_SCAN_ON_NORMAL);

    return;
}

VOID psNcuLoopScanPktProc(T_MediaProcThreadPara *ptMediaProcThreadPara)
{
    MCS_CHK_NULL_VOID(ptMediaProcThreadPara);
    PS_PACKET *ptPacket = ptMediaProcThreadPara->ptPacket;
    MCS_CHK_NULL_VOID(ptPacket);
    T_psNcuMcsPerform *ptNcuMcsPerform = (T_psNcuMcsPerform *)ptMediaProcThreadPara->ptMcsStatPointer;/*按线程统计*/
    MCS_CHK_NULL_VOID(ptNcuMcsPerform);
    T_psNcuMediaScanMsg *ptSigHead = NULL;

    BYTE *aucPacketBuf    = NULL; /* 当前mbuf实际提供的内存 */
    WORD16 wBufferLen     = 0;    /* 当前mbuf实际提供的长度 */
    WORD16 wBufferOffset  = 0;    /* 当前mbuf偏移 */

    if (unlikely(NULL == ptPacket->pBufferNode))
    {
        ptNcuMcsPerform->qwPktInfoNull++;
        return ;
    }

    /* 对内性能统计 */
    MCS_LOC_STAT_EX(ptNcuMcsPerform, qwRcvLoopScanPkt, 1);
    PS_GET_BUF_INFO(ptPacket->pBufferNode, aucPacketBuf, wBufferOffset, wBufferLen);
    ptSigHead     =  (T_psNcuMediaScanMsg*)(aucPacketBuf+wBufferOffset);
    DEBUG_TRACE(DEBUG_HIG,
                "\n[Mcs] psNcuLoopScanPktProc :ptSigHead->dwMessageType(0x%08x)  \n ",
                ptSigHead->dwMessageType);

    psNcuLoopScanMessageProcFun(ptSigHead->dwMessageType, wBufferLen, ptSigHead, ptMediaProcThreadPara);
    return ;
}

/* 根据消息类型调用其对应的消息处理函数 */
VOID psNcuLoopScanMessageProcFun(WORD32 dwMessageType, WORD16 wBufferLen, T_psNcuMediaScanMsg *ptSigHead, T_MediaProcThreadPara *ptMediaProcThreadPara)
{
    WORD32 cyl;

    for (cyl = 0; cyl < g_NumOfScanMessageFun; cyl++) {
        if (g_atNcuScanMessageFunTbl[cyl].dwMessageType == dwMessageType) {
            g_atNcuScanMessageFunTbl[cyl].pScanMessageFun(wBufferLen, ptSigHead, ptMediaProcThreadPara);
            break;
        }
    }

    return;
}

WORD32 psNcuScanPktSend(BYTE *pbMsg, WORD32 dwMsgType, WORD32 dwMsgLen, BYTE bMcsThreadNo, BYTE bScanThreadNo)
{
    PS_PACKET        *ptNewPkt       = NULL;
    T_psNcuMediaScanMsg *ptMediaScanMsg = NULL;
    BYTE   *pbPacketBuf   = NULL; /* 当前mbuf实际提供的内存 */
    WORD16  wBufferLen    = 0;    /* 当前mbuf实际提供的长度 */
    WORD16  wBufferOffset = 0;    /* 当前mbuf偏移 */
    WORD32  dwCSSRet      = 0;

    ptNewPkt =  psCssDesBufAlloc(bScanThreadNo);
    if(unlikely(NULL == ptNewPkt))
    {
        return ERROR_MEDIA;
    }
    PACKET_PROCESS_START(ptNewPkt, PKTCHK_MCS_SCAN);//PKTCHK_MCS_PROC
    if(unlikely(NULL == ptNewPkt->pBufferNode))
    {
        psCSSPktLinkBuffDesFree(ptNewPkt, PKTCHK_MCS_SCAN);
        return ERROR_MEDIA;
    }
    ptNewPkt->bLBKey = ptNewPkt->bSthr;

    PS_GET_BUF_INFO(ptNewPkt->pBufferNode, pbPacketBuf, wBufferOffset, wBufferLen);

    /* GENERAL头数据 */
    ptMediaScanMsg = (T_psNcuMediaScanMsg *)(pbPacketBuf + wBufferOffset);
    ptMediaScanMsg->dwMessageType = dwMsgType;
    ptMediaScanMsg->wMessageLen = (WORD16)dwMsgLen;

    /* 消息净荷数据 */
    pbPacketBuf = (BYTE *)ptMediaScanMsg + sizeof(T_psNcuMediaScanMsg);
    CSS_MEMCPY(pbPacketBuf, pbMsg, dwMsgLen);

    wBufferLen   = (WORD16)(sizeof(T_psNcuMediaScanMsg) + dwMsgLen);
    PS_SET_BUF_INFO(ptNewPkt, ptNewPkt->pBufferNode, wBufferOffset, wBufferLen);

    /*目前都是cpu内发送*/
    ptNewPkt->wPktAttr = CSS_FWD_INNER_LOOPSCAN;

    PACKET_PROCESS_END(ptNewPkt, PKTCHK_MCS_SCAN);
    dwCSSRet = dpaMediaLocalFwd(ptNewPkt, (WORD32)bMcsThreadNo);
    if (0 == dwCSSRet) /*成功*/
    {
        /*统计待添加*/
        return SUCCESS;
    }
    else /*失败*/
    {
        /*统计待添加*/
        return ERROR_MEDIA;
    }
}

VOID psNcuSessionAgeingFun(BYTE bTaskNo, BYTE bMcsThreadNo, BYTE bScanThreadNo)
{
    MCS_CHK_VOID(bTaskNo >= g_bNumOfScanTask);
    T_McsCtxScanPara *ptSessionCtxScanInfo = &(g_atNcucanRegTbl[bTaskNo].tMcsCtxScanInfo[bMcsThreadNo % MAX_PS_THREAD_INST_NUM]);
    WORD32 dwSessionCtxCap = ptSessionCtxScanInfo->dwCtxCap;
    WORD32 dwScanMax    = ptSessionCtxScanInfo->dwScanNum10ms;
    WORD32 dwAgeTime    = ptSessionCtxScanInfo->dwAgeTime;
    WORD32 dwScanId     = 0;
    WORD32 dwScanIndex  = 0;

    for(dwScanIndex = 0; dwScanIndex < dwScanMax; dwScanIndex++)
    {
        dwScanId = ptSessionCtxScanInfo->dwScanIndex++;
        if(dwScanId >= dwSessionCtxCap)
        {
            ptSessionCtxScanInfo->dwScanIndex = 1;   /*记录号从1开始有效*/
            dwScanId = dwSessionCtxCap;   /*保护一下*/
        }
        psNcuSessionCtxAgeingProc(dwScanId, bMcsThreadNo, bScanThreadNo, dwAgeTime);
    }
    return;
}

VOID psNcuSessionCtxAgeingProc(WORD32 dwScanId, BYTE bMcsThreadNo, BYTE bScanThreadNo, WORD32 dwAgeTime)
{
    T_psNcuMcsPerform *ptNcuMcsPerform = psVpfuMcsGetPerformPtr(bScanThreadNo);
    MCS_CHK_NULL_VOID(ptNcuMcsPerform);
    WORD32  hDB  = _NCU_GET_DBHANDLE(bMcsThreadNo);
    T_psNcuSessionCtx *ptNcuSessionCtx = (T_psNcuSessionCtx *)psVpfuMcsGetCtxById(dwScanId, hDB, DB_HANDLE_R_NCUSESSION);
    MCS_CHK_NULL_VOID(ptNcuSessionCtx);

    BYTE bNcuToPfuSynSessionctx = ptNcuSessionCtx->bNcuToPfuSynSessionctx;
    if(bNcuToPfuSynSessionctx == 1)
    {
        dwAgeTime = getNcuSoftPara(5010);
    }
    WORD32 dwCurtime = psFtmGetPowerOnSec();
    WORD32 dwLastUpdStamp = ptNcuSessionCtx->dwUpdateTimeStamp;
    if(FALSE == psNcuCheckAgeingTimeStamp(dwCurtime, dwLastUpdStamp, dwAgeTime))
    {
        return;
    }

    T_psMcsCtxRelReport tMcsSessionCtxScan = {0};
    tMcsSessionCtxScan.dwMcsCtxId          = dwScanId;
    tMcsSessionCtxScan.dwStampInMcsCtx     = ptNcuSessionCtx->dwUpdateTimeStamp;
    if(SUCCESS == psNcuScanPktSend((BYTE *)&tMcsSessionCtxScan, NCU_NOTIFY_SESSION_RELEASE, sizeof(tMcsSessionCtxScan),
                                            bMcsThreadNo, bScanThreadNo))
    {   
        if(bNcuToPfuSynSessionctx == 1)
        {
            MCS_LOC_STAT_EX(ptNcuMcsPerform, qwSessionAgeingRelCtxNcuToPfuSyn, 1);
            return;
        }
        MCS_LOC_STAT_EX(ptNcuMcsPerform, qwSessionAgeingRelCtx, 1);
        return;
    }
                                            
    return;
}

VOID psNcuSessionAgeingRel(WORD16 wBufferLen, T_psNcuMediaScanMsg *ptSigHead, T_MediaProcThreadPara *ptMediaProcThreadPara)
{
    MCS_CHK_NULL_VOID(ptMediaProcThreadPara);
    MCS_CHK_NULL_VOID(ptSigHead);

    T_psMcsCtxRelReport *ptMcsPFUCtxScan    = NULL;
    T_psNcuSessionCtx   *ptNcuSessionCtx    = NULL;
    T_psNcuMcsPerform   *ptNcuPerform       = (T_psNcuMcsPerform *)ptMediaProcThreadPara->ptMcsStatPointer;/*按线程统计*/
    WORD32  hDB  = ptMediaProcThreadPara->dwhDBByThreadNo;

    if (unlikely(wBufferLen < (sizeof(T_psNcuMediaScanMsg) + sizeof(T_psMcsCtxRelReport))))
    {
        return;
    }

    ptMcsPFUCtxScan = (T_psMcsCtxRelReport *)((BYTE *)ptSigHead + sizeof(T_psNcuMediaScanMsg));
    ptNcuSessionCtx = (T_psNcuSessionCtx*)psVpfuMcsGetCtxById(ptMcsPFUCtxScan->dwMcsCtxId, hDB, DB_HANDLE_R_NCUSESSION);
    if (NULL == ptNcuSessionCtx)
    {
        MCS_LOC_STAT_EX(ptNcuPerform, qwSessionAgeingQySessionErr, 1);
        return;
    }
    if (unlikely(ptMcsPFUCtxScan->dwStampInMcsCtx != ptNcuSessionCtx->dwUpdateTimeStamp))
    {
        MCS_LOC_STAT_EX(ptNcuPerform, qwSessionAgeingTimeUpded, 1);
        return;
    }

    MCS_LOC_STAT_EX(ptNcuPerform, qwSessionAgeingRelSessionCtx, 1);
    ptMediaProcThreadPara->ddwUPseid = ptNcuSessionCtx->ddwUPSeid;
    WORD32 dwReturnValue = psNcuDelSessionProc(ptMediaProcThreadPara);
    MCS_CHK_VOID_STAT(MCS_RET_SUCCESS != dwReturnValue,qwSessionCtxAgeingRelErr);

    return;
}

VOID psNcuFlowAgeingRel(WORD16 wBufferLen, T_psNcuMediaScanMsg *ptSigHead, T_MediaProcThreadPara *ptMediaProcThreadPara)
{
    MCS_CHK_NULL_VOID(ptMediaProcThreadPara);
    MCS_CHK_NULL_VOID(ptSigHead);

    T_psMcsCtxRelReport *ptMcsPFUCtxScan    = NULL;
    T_psNcuFlowCtx      *ptNcuFlowCtx       = NULL;
    T_psNcuMcsPerform   *ptNcuPerform       = (T_psNcuMcsPerform *)ptMediaProcThreadPara->ptMcsStatPointer;/*按线程统计*/
    WORD32  hDB  = ptMediaProcThreadPara->dwhDBByThreadNo;

    if (unlikely(wBufferLen < (sizeof(T_psNcuMediaScanMsg) + sizeof(T_psMcsCtxRelReport))))
    {
        return;
    }

    ptMcsPFUCtxScan = (T_psMcsCtxRelReport *)((BYTE *)ptSigHead + sizeof(T_psNcuMediaScanMsg));
    ptNcuFlowCtx = (T_psNcuFlowCtx*)psVpfuMcsGetCtxById(ptMcsPFUCtxScan->dwMcsCtxId, hDB, DB_HANDLE_R_NCUFLOWCTX);
    if (NULL == ptNcuFlowCtx)
    {
        MCS_LOC_STAT_EX(ptNcuPerform, qwFlowAgeingQyFlowErr, 1);
        return;
    }
    if (unlikely(ptMcsPFUCtxScan->dwStampInMcsCtx != ptNcuFlowCtx->dwUpdateTimeStamp))
    {
        MCS_LOC_STAT_EX(ptNcuPerform, qwFlowAgeingTimeUpded, 1);
        return;
    }

    MCS_LOC_STAT_EX(ptNcuPerform, qwFlowAgeingRelFlowCtx, 1);
    WORD32 dwReturnValue = psDelStmProc(ptMediaProcThreadPara, ptNcuFlowCtx);
    MCS_CHK_VOID_STAT(MCS_RET_SUCCESS != dwReturnValue, qwFlowCtxAgeingRelErr);
    return;
}

VOID psNcuSnAgeingFun(BYTE bTaskNo, BYTE bMcsThreadNo, BYTE bScanThreadNo)
{
    MCS_CHK_VOID(bTaskNo >= g_bNumOfScanTask);
    T_McsCtxScanPara *ptSnCtxScanInfo = &(g_atNcucanRegTbl[bTaskNo].tMcsCtxScanInfo[bMcsThreadNo % MAX_PS_THREAD_INST_NUM]);
    WORD32 dwSnCtxCap   = ptSnCtxScanInfo->dwCtxCap;
    WORD32 dwScanMax    = ptSnCtxScanInfo->dwScanNum10ms;
    WORD32 dwAgeTime    = ptSnCtxScanInfo->dwAgeTime;
    WORD32 dwScanId     = 0;
    WORD32 dwScanIndex  = 0;

    for(dwScanIndex = 0; dwScanIndex < dwScanMax; dwScanIndex++)
    {
        dwScanId = ptSnCtxScanInfo->dwScanIndex++;
        if(dwScanId >= dwSnCtxCap)
        {
            ptSnCtxScanInfo->dwScanIndex = 1;   /*记录号从1开始有效*/
            dwScanId = dwSnCtxCap;   /*保护一下*/
        }
        psNcuSnCtxAgeingProc(dwScanId, bMcsThreadNo, bScanThreadNo, dwAgeTime);
    }
    return;
}

VOID psNcuSnCtxAgeingProc(WORD32 dwScanId, BYTE bMcsThreadNo, BYTE bScanThreadNo, WORD32 dwAgeTime)
{
    T_psNcuMcsPerform *ptNcuMcsPerform = psVpfuMcsGetPerformPtr(bScanThreadNo);
    MCS_CHK_NULL_VOID(ptNcuMcsPerform);
    WORD32  hDB  = _NCU_GET_DBHANDLE(bMcsThreadNo);
    T_NcuMcsSnCtx *ptNcuSnCtx = (T_NcuMcsSnCtx *)psVpfuMcsGetCtxById(dwScanId, hDB, DB_HANDLE_R_NCU_SN);
    MCS_CHK_NULL_VOID(ptNcuSnCtx);
    WORD32 dwCurtime = psFtmGetPowerOnSec();
    WORD32 dwLastUpdStamp = ptNcuSnCtx->dwCreateTimeStamp;
    if(FALSE == psNcuCheckAgeingTimeStamp(dwCurtime, dwLastUpdStamp, dwAgeTime))
    {
        return;
    }
    T_psMcsCtxRelReport tMcsSnCtxScan = {0};
    tMcsSnCtxScan.dwMcsCtxId          = dwScanId;
    tMcsSnCtxScan.dwStampInMcsCtx     = dwLastUpdStamp;
    if(SUCCESS == psNcuScanPktSend((BYTE *)&tMcsSnCtxScan, NCU_NOTIFY_SN_RELEASE, sizeof(tMcsSnCtxScan),
                                            bMcsThreadNo, bScanThreadNo))
    {
        MCS_LOC_STAT_EX(ptNcuMcsPerform, qwSnAgeingRelCtx, 1);
    }
}

VOID psNcuSnAgeingRel(WORD16 wBufferLen, T_psNcuMediaScanMsg *ptSigHead, T_MediaProcThreadPara *ptMediaProcThreadPara)
{
    MCS_CHK_NULL_VOID(ptMediaProcThreadPara);
    MCS_CHK_NULL_VOID(ptSigHead);

    T_psMcsCtxRelReport *ptMcsPFUCtxScan    = NULL;
    T_NcuMcsSnCtx       *ptNcuSnCtx         = NULL;
    T_psNcuMcsPerform   *ptNcuPerform       = (T_psNcuMcsPerform *)ptMediaProcThreadPara->ptMcsStatPointer;/*按线程统计*/
    WORD32  hDB  = ptMediaProcThreadPara->dwhDBByThreadNo;

    if (unlikely(wBufferLen < (sizeof(T_psNcuMediaScanMsg) + sizeof(T_psMcsCtxRelReport))))
    {
        return;
    }

    ptMcsPFUCtxScan = (T_psMcsCtxRelReport *)((BYTE *)ptSigHead + sizeof(T_psNcuMediaScanMsg));
    ptNcuSnCtx = (T_NcuMcsSnCtx*)psVpfuMcsGetCtxById(ptMcsPFUCtxScan->dwMcsCtxId, hDB, DB_HANDLE_R_NCU_SN);
    if (NULL == ptNcuSnCtx)
    {
        MCS_LOC_STAT_EX(ptNcuPerform, qwSnAgeingQySnErr, 1);
        return;
    }
    if (unlikely(ptMcsPFUCtxScan->dwStampInMcsCtx != ptNcuSnCtx->dwCreateTimeStamp))
    {
        MCS_LOC_STAT_EX(ptNcuPerform, qwSnAgeingTimeUpded, 1);
        return;
    }

    MCS_LOC_STAT_EX(ptNcuPerform, qwSnAgeingRelSnCtx, 1);
    WORD32 dwReturnValue = psNcuDaRttListDel(ptMediaProcThreadPara, ptNcuSnCtx);
    MCS_CHK_VOID_STAT(MCS_RET_SUCCESS != dwReturnValue,qwSnCtxAgeingRelErr);

    return;
}

VOID showNcuScanInfo()
{
    WORD32  i = 0;
    BYTE    bSthIndex  = 0;
    BYTE    bInstNum = g_ptVpfuShMemVar->tVpfuMcsThreadPara.bInstNum;
    for(; i < g_bNumOfScanTask; i++)
    {
        zte_printf_s("%s:\n", g_atNcucanRegTbl[i].aTableName);
        for (bSthIndex = 0; bSthIndex < bInstNum && bSthIndex < MAX_PS_THREAD_INST_NUM; bSthIndex++)
        {
            zte_printf_s("thread[%u] dwCtxCap[%u] dwAgeTime[%u] dwScanIndex[%u] dwScanNum10ms[%u]\n", bSthIndex,
            g_atNcucanRegTbl[i].tMcsCtxScanInfo[bSthIndex].dwCtxCap, g_atNcucanRegTbl[i].tMcsCtxScanInfo[bSthIndex].dwAgeTime,
            g_atNcucanRegTbl[i].tMcsCtxScanInfo[bSthIndex].dwScanIndex, g_atNcucanRegTbl[i].tMcsCtxScanInfo[bSthIndex].dwScanNum10ms);
        }
    }
}

void set_appid_relate_age(BYTE flg)
{
    g_set_appid_relate_clear = flg;
    if(flg > APPID_RELATE_AGE_ALWAYS)
    {
        g_set_appid_relate_clear = APPID_RELATE_AGE_ALWAYS;
    }
}

WORD32 psNcuGetCtxAgeTime(const CHAR* pCtxname)
{
    if (NULL == pCtxname)
    {
        return 0;
    }

    WORD32 i = 0;
    for (; i < R_RESOURCEAGE_CAPACITY; i++)
    {
        T_ResourceAgeCfg* ptResourceAgeCfg = &g_upfConfig.resourceAgeCfg[i];
        if (strncmp(g_upfConfig.resourceAgeCfg[i].bCtxname, pCtxname, LEN_R_RESOURCEAGE_CTXNAME_MAX+1))
        {
            continue;
        }
        return ptResourceAgeCfg->dwAgetime;
    }
    return 0;
}

UPF_HELP_REG("ncu",
             "alloc full sessions",
             VOID psNcuDbgAllocFullSessions())
{
    BYTE bInstNum = g_ptVpfuShMemVar->tVpfuMcsThreadPara.bInstNum;
    BYTE bMcsThr = 0xff;
    BYTE bSthIndex = 0;
    for(bSthIndex = 0; bSthIndex < bInstNum && bSthIndex < MAX_PS_THREAD_INST_NUM; bSthIndex++)
    {
        /*获取媒体面线程*/
        bMcsThr = g_ptVpfuShMemVar->tVpfuMcsThreadPara.atThreadParaList[bSthIndex].bSoftThreadNo;
        WORD32 hDB = _NCU_GET_DBHANDLE(bMcsThr);
        WORD64 ddwUpseid = 0;
        WORD32 i = 0;
        WORD32 cap = _db_get_ncusession_capacity();
        T_psNcuSessionCtx *ptNcuSessionCtx = NULL;
        for(; i < cap; i++)
        {
            ddwUpseid++;
            ptNcuSessionCtx = psCreateSessionByUpseid(ddwUpseid, hDB);
            MCS_CHK_CONTINUE(NULL == ptNcuSessionCtx);
            ptNcuSessionCtx->dwUpdateTimeStamp = psFtmGetPowerOnSec();
        }
    }
}

UPF_HELP_REG("ncu",
             "alloc full flows",
             VOID psNcuDbgAllocFullFlows())
{
    BYTE bInstNum = g_ptVpfuShMemVar->tVpfuMcsThreadPara.bInstNum;
    BYTE bMcsThr = 0xff;
    BYTE bSthIndex = 0;
    for(bSthIndex = 0; bSthIndex < bInstNum && bSthIndex < MAX_PS_THREAD_INST_NUM; bSthIndex++)
    {
        /*获取媒体面线程*/
        bMcsThr = g_ptVpfuShMemVar->tVpfuMcsThreadPara.atThreadParaList[bSthIndex].bSoftThreadNo;
        WORD32 hDB = _NCU_GET_DBHANDLE(bMcsThr);
        DB_STRM_INDEX index = {0};
        WORD32 i = 0;
        WORD32 cap = _db_get_flowctx_capacity();
        T_psNcuFlowCtx *ptNcuFlowCtx = NULL;
        for(; i < cap; i++)
        {
            index.wSvrPort++;
            ptNcuFlowCtx = psNcuMcsCreatStmByIndex(&index, hDB);
            MCS_CHK_CONTINUE(NULL == ptNcuFlowCtx);
            ptNcuFlowCtx->dwUpdateTimeStamp = psFtmGetPowerOnSec();
        }
    }
}

UPF_HELP_REG("ncu",
             "alloc full sns",
             VOID psNcuDbgAllocFullSns())
{
    BYTE bInstNum = g_ptVpfuShMemVar->tVpfuMcsThreadPara.bInstNum;
    BYTE bMcsThr = 0xff;
    BYTE bSthIndex = 0;
    for(bSthIndex = 0; bSthIndex < bInstNum && bSthIndex < MAX_PS_THREAD_INST_NUM; bSthIndex++)
    {
        /*获取媒体面线程*/
        bMcsThr = g_ptVpfuShMemVar->tVpfuMcsThreadPara.atThreadParaList[bSthIndex].bSoftThreadNo;
        WORD32 hDB = _NCU_GET_DBHANDLE(bMcsThr);
        WORD32 dwSeqNum = 1;
        for(; dwSeqNum <= _db_get_ncu_sn_capacity(); dwSeqNum++)
        {
            psCrtSnCtxByFlowIdSeqDir(1, dwSeqNum, MCS_PKT_DIR_UL, hDB);;
        }
    }
}

//Get aging config for Rust 
VOID psNcuGetSessionAgeCfg(WORD32 *ptCtxCap, WORD32 *ptScanNum10ms, WORD32 *ptAgeTime)
{
    MCS_CHK_NULL_VOID(ptCtxCap);
    MCS_CHK_NULL_VOID(ptScanNum10ms);
    MCS_CHK_NULL_VOID(ptAgeTime);

    psNcuGetCtxAgeCfg(DB_HANDLE_R_NCUSESSION, "R_NCU_SESSION", ptCtxCap, ptScanNum10ms, ptAgeTime);
}

/* Started by AICoder, pid:gc45bef55ax381614f1e0b5650e5371131e91fd5 */
VOID psNcuGetFlowCtxAgeCfg(WORD32 *ptCtxCap, WORD32 *ptScanNum10ms, WORD32 *ptAgeTime)
{
    MCS_CHK_NULL_VOID(ptCtxCap);
    MCS_CHK_NULL_VOID(ptScanNum10ms);
    MCS_CHK_NULL_VOID(ptAgeTime);

    psNcuGetCtxAgeCfg(DB_HANDLE_R_NCUFLOWCTX, "R_FLOWCTX", ptCtxCap, ptScanNum10ms, ptAgeTime);
}

VOID psNcuGetSnCtxAgeCfg(WORD32 *ptCtxCap, WORD32 *ptScanNum10ms, WORD32 *ptAgeTime)
{
    MCS_CHK_NULL_VOID(ptCtxCap);
    MCS_CHK_NULL_VOID(ptScanNum10ms);
    MCS_CHK_NULL_VOID(ptAgeTime);

    psNcuGetCtxAgeCfg(DB_HANDLE_R_NCU_SN, "R_NCU_SN", ptCtxCap, ptScanNum10ms, ptAgeTime);
}
/* Ended by AICoder, pid:gc45bef55ax381614f1e0b5650e5371131e91fd5 */

VOID psNcuGetCtxAgeCfg(WORD32 tabHdl, const CHAR *ptTableName, WORD32 *ptCtxCap, WORD32 *ptScanNum10ms, WORD32 *ptAgeTime)
{
    MCS_CHK_NULL_VOID(ptCtxCap);
    MCS_CHK_NULL_VOID(ptScanNum10ms);
    MCS_CHK_NULL_VOID(ptTableName);
    MCS_CHK_NULL_VOID(ptAgeTime);

    WORD32 dwhDB      = 0;
    WORD32 dwRetValue = 0;
    WORD32 dwTupleNum = 0;
    BYTE   bMcsSthNo  = 0;

    bMcsSthNo = g_ptVpfuShMemVar->tVpfuMcsThreadPara.atThreadParaList[0].bSoftThreadNo;
    dwhDB = _NCU_GET_DBHANDLE(bMcsSthNo);

    *ptCtxCap = 0;
    dwRetValue = psVpfuMcsGetCapacity(tabHdl, dwhDB, ptCtxCap, &dwTupleNum);
    MCS_CHK_VOID(ERROR_MEDIA == dwRetValue);
    
    WORD32 dwLoopcfg  = 0;
    T_ResourceAgeCfg *ptAgeParaCfg = NULL;

     dwTupleNum = MIN(g_upfConfig.bResourceAgeCfgAgeTableNums, R_RESOURCEAGE_CAPACITY);
    *ptAgeTime = 0;
    *ptScanNum10ms = 10;

    int tblNameLen = zte_strnlen_s(ptTableName, LEN_R_RESOURCEAGE_CTXNAME_MAX);
    for(dwLoopcfg = 0; dwLoopcfg < dwTupleNum; dwLoopcfg++)
    {
        ptAgeParaCfg = &(g_upfConfig.resourceAgeCfg[dwLoopcfg]);
        if(0  == memcmp(ptTableName,ptAgeParaCfg->bCtxname,tblNameLen))
        {
            *ptScanNum10ms = (WORD32)ptAgeParaCfg->bScanstep;
            *ptAgeTime = ptAgeParaCfg->dwAgetime;
            break;
        }
    }
}
