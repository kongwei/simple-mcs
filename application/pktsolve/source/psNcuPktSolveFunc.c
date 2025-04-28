#include "psNcuPktSolveFunc.h"
#include "psNcuPktParse.h"
#include "psNcuStreamProc.h"
#include "psNcuTcpCalcProc.h"
#include "psNcuDataAnalysis.h"
#include "ps_pub.h"
#include "UpfNcuSynInfo.h"
#include "psNcuSnCtxProc.h"
#include "psNcuSubscribeCtxProc.h"
#include "ps_ncu_session.h"
#include "psNcuReportUDPProc.h"
#include "psNcuReportSBCProc.h"
#include "psMcsDebug.h"
#include "McsIPv4Head.h"
#include "McsIPv6Head.h"
#include "zte_slibc.h"
#include "ps_db_define_ncu.h"
#include "psNcuCtxFunc.h"
#include "psNcuSubBindAppCtxProc.h"
#include "ps_ncu_typedef.h"
#include "psNcuApplicationMapCtxProc.h"
#include "ncuUserGroup.h"
#include "ps_mcs_define.h"
#include "psNcuDADialCtxProc.h"

VOID psNcuFillStrmIndex(T_MediaProcThreadPara* ptMediaProcThreadPara, DB_STRM_INDEX* ptStmIdx);
void psNcuUpdateDAtaAppCtx(T_MediaProcThreadPara* ptMediaProcThreadPara, T_psNcuFlowCtx * ptStmAddr);
void psNcuPktSolveProc(void* ptPktData, WORD16 wLength, T_MediaProcThreadPara *ptMediaProcThreadPara);
void psNcuTrafficReportSolveProc(void* ptTrafficReportData, WORD16 wLength, T_MediaProcThreadPara *ptMediaProcThreadPara);

inline WORD32 psNcuGetCopyPktMaxNum()
{
    WORD32 Mtu = getNcuSoftPara(5035);
    if(Mtu == DEFAULT_MTU || Mtu == 0)
    {
        return MAX_PKTNUM_TONCU;
    }
    return (Mtu - MAX_RAW_LEN)/(MAX_PKTDADA_LEN+PKTINFOHEAD_LEN);
}

inline WORD32 psNcuGetTrafficRptMaxNum()
{
    WORD32 Mtu = getNcuSoftPara(5035);
    if(Mtu == DEFAULT_MTU || Mtu == 0)
    {
        return MAX_TRAFFICREPORTNUM_TONCU;
    }
    return (Mtu - MAX_RAW_LEN)/TRAFFICINFO_LEN;
}
void psNcuMultiPktSolveProc(void* ptPktData, BYTE bPktNum, WORD16 wLength, T_MediaProcThreadPara *ptMediaProcThreadPara)
{
    if(NULL == ptPktData || NULL == ptMediaProcThreadPara || NULL == ptMediaProcThreadPara->ptMcsStatPointer)
    {
        return;
    }
    T_psNcuMcsPerform *ptMcsNcuPerform = (T_psNcuMcsPerform*)ptMediaProcThreadPara->ptMcsStatPointer;
    T_NcuPktCopyInfo *ptNcuSinglePktInfo = (T_NcuPktCopyInfo*)(ptPktData);
    WORD32 i = 0;
    for (i = 0; i < psNcuGetCopyPktMaxNum() && i < bPktNum; i++)
    {
        psNcuPktSolveProc((void *)(ptNcuSinglePktInfo+i), wLength, ptMediaProcThreadPara);
        MCS_LOC_STAT_EX(ptMcsNcuPerform, qwSolveSingleCopyPkt, 1);
    }
}

void psNcuPktSolveProc(void* ptPktData, WORD16 wLength, T_MediaProcThreadPara *ptMediaProcThreadPara)
{
    if(NULL == ptPktData || NULL == ptMediaProcThreadPara || NULL == ptMediaProcThreadPara->ptMcsStatPointer)
    {
        return;
    }
    T_psNcuMcsPerform *ptMcsNcuPerform = (T_psNcuMcsPerform*)ptMediaProcThreadPara->ptMcsStatPointer;
    T_NcuPktInfoHead        *ptPktHead = (T_NcuPktInfoHead*)ptPktData;
    DEBUG_TRACE(DEBUG_LOW, "ddwUPseid=%llu, dwSubAppid=%u, iptype=%u, l34len=%u,dir=%u!\n", ptPktHead->upseid, ptPktHead->subappid, ptPktHead->bIpType,ptPktHead->wL34Length, ptPktHead->bDir);
    
    WORD32 dwGroupId   = GET_LDU_USERGROUP_FROM_UPSEID(ptPktHead->upseid);
    if(NCUROUP_STATE_MIGOUT == g_ncuGroupState[dwGroupId])
    {
        DEBUG_TRACE(DEBUG_LOW, "recive migout group pkt! ddwUPseid=%llu, dwGroupId=%u!\n", ptPktHead->upseid, dwGroupId);
        MCS_LOC_STAT_EX(ptMcsNcuPerform, qwRcvCopyPktGroupMigoutErr, 1);
        return;
    }

    ptMediaProcThreadPara->ddwUPseid = ptPktHead->upseid;
    ptMediaProcThreadPara->dwSubAppid = ptPktHead->subappid;
    BYTE bDelStm     = ptPktHead->bDelStream;
    DB_STRM_INDEX  tQuintuple  = {{0},{0},0,0,0,0,0,{0}};

    if(ptPktHead->bDir == MCS_PKT_DIR_DL)
    {
        NCU_PM_55304_STAT_ADD(qwNcuRcvDlCpyPktBytes, ptPktHead->wL34Length);
    }
    else
    {
        NCU_PM_55304_STAT_ADD(qwNcuRcvUlCpyPktBytes, ptPktHead->wL34Length);
    }

    if (MCS_RET_FAIL == psNcuParsePktInfo(ptMediaProcThreadPara, ptPktData))
    {
        MCS_LOC_STAT_EX(ptMcsNcuPerform, qwRcvCopyPktParseErr, 1);
        DEBUG_TRACE(DEBUG_ERR,"Parse pkt ipinfo fail!\n");
        return ;
    }

    psNcuFillStrmIndex(ptMediaProcThreadPara, &tQuintuple);
    MCS_CHK_VOID(MCS_RET_FAIL == psPktCheckNcuCtxIsNullInternal(ptMediaProcThreadPara));

    T_psNcuFlowCtx *ptStmAddr = psNcuQryStreamProc(ptMediaProcThreadPara, &tQuintuple);
    if(NULL == ptStmAddr)
    {
        MCS_CHK_VOID(MCS_RET_FAIL == psPktCheckNcuCtxIsNull(ptMediaProcThreadPara));
        ptStmAddr = psNcuCrtStreamProc(ptMediaProcThreadPara, &tQuintuple);
        if(NULL == ptStmAddr)
        {
            DEBUG_TRACE(DEBUG_LOW,"stream crt fail\n");
            MCS_LOC_STAT_EX(ptMcsNcuPerform, qwCrtStreamFail, 1);
            return;
        }
        DEBUG_TRACE(DEBUG_LOW, "stream crt succ!\n");
        MCS_LOC_STAT_EX(ptMcsNcuPerform, qwCrtStreamSucc, 1);
    }
    else
    {
        ptStmAddr->dwUpdateTimeStamp = ptMediaProcThreadPara->dwPktPowOnSec;
        MCS_LOC_STAT_EX(ptMcsNcuPerform, qwQryStreamSucc, 1);
        DEBUG_TRACE(DEBUG_LOW,"stream query succ!\n");
    }
    /* 流计算暂时写死，需要按算法计算 */
    if(1 == ptPktHead->bSlowStmFlag)
    {
        ptStmAddr->b5QI = ptPktHead->b5QI;
    }
    ptStmAddr->dwPktNum++;
    ptStmAddr->dwPktBytes += ptPktHead->wL34Length;
    ptStmAddr->dwSubAppId = ptMediaProcThreadPara->dwSubAppid;
    /* 流计算暂时写死，需要按算法计算 */

    /* 带宽计算 */
    psNcuCalcPktBandWith(ptMediaProcThreadPara, ptStmAddr);
    /* 时延计算 && 丢包计算 */
    psNcuCalcPktRtt(ptMediaProcThreadPara, ptStmAddr, ptPktData);

    T_psNcuDaAppCtx *ptDataAppCtx = (T_psNcuDaAppCtx*)ptStmAddr->ptNcuDaCtx;
    if(NULL != ptDataAppCtx)
    {
        if(ptDataAppCtx->dwSubAppid != ptStmAddr->dwSubAppId)
        {
            //应该不会存在。
            MCS_LOC_STAT_EX(ptMcsNcuPerform, qwDaAppCtxSubAppidDiffFromStm, 1);
            DEBUG_TRACE(DEBUG_LOW,"NULL != ptDataAppCtx && ptDataAppCtx->dwSubAppid != ptStmAddr->dwSubAppId\n");
            
            psNcuRmStmFromDataApp(ptMediaProcThreadPara, ptStmAddr, ptDataAppCtx);
            ptDataAppCtx = NULL;
            ptStmAddr->dwSubAppId = 0;
            ptStmAddr->ptNcuDaCtx = NULL;

            psCheckNcuToPfuSynSessionCtx(ptMediaProcThreadPara);
        }
        else
        {
            ptDataAppCtx->dwUpdateTimeStamp = ptMediaProcThreadPara->dwPktPowOnSec;
            T_psNcuDaSubScribeCtx* ptSubScribeCtx = (T_psNcuDaSubScribeCtx*)ptDataAppCtx->ptSubScribeCtx;
            if(NULL == ptSubScribeCtx)
            {
                MCS_LOC_STAT_EX(ptMcsNcuPerform, qwDaAppNoSubScribeCtx, 1);
            }
            else
            {
                ptSubScribeCtx->dwUpdateTimeStamp = ptMediaProcThreadPara->dwPktPowOnSec;
            }
            T_psNcuSessionCtx* ptSessionCtx = (T_psNcuSessionCtx*)ptDataAppCtx->ptSessionCtx;
            if(NULL == ptSessionCtx)
            {
                MCS_LOC_STAT_EX(ptMcsNcuPerform, qwDaAppNoSessionCtx, 1);
            }
            else
            {
                ptSessionCtx->dwUpdateTimeStamp = ptMediaProcThreadPara->dwPktPowOnSec;
            }
        }
    }
    if(NULL == ptDataAppCtx)
    {
        DEBUG_TRACE(DEBUG_LOW,"psNcuAddStmToDataApp\n");
        ptDataAppCtx = psNcuGetDataAppProc(ptMediaProcThreadPara);
        if(NULL == ptDataAppCtx)
        {
            MCS_LOC_STAT_EX(ptMcsNcuPerform, qwStreamGetDataAppFail, 1);
            DEBUG_TRACE(DEBUG_LOW,"psNcuGetDataAppProc get NULL\n");
        }
        psNcuAddStmToDataApp(ptMediaProcThreadPara, ptStmAddr, ptDataAppCtx);
        /* 关联拨测上下文 */
        psNcuDaAppCtxRelateDaDialCtx(ptDataAppCtx, ptMediaProcThreadPara->bThreadNo);
        return;
    }

        //上报能力，其实可以挪到业务识别的时候更新，目前放在去订阅的时候更新所有已经创建的数据分析上下文

    if(bDelStm)
    {
        MCS_LOC_STAT_EX(ptMcsNcuPerform, qwStreamDelByPktCpy, 1);
        psDelStmProc(ptMediaProcThreadPara,ptStmAddr);
    }
    
    return;
}

void psNcuMultiTrafficReportSolveProc(void* ptTrafficReportData, BYTE bPktNum, WORD16 wLength, T_MediaProcThreadPara *ptMediaProcThreadPara)
{
    MCS_PCLINT_NULLPTR_RET_3ARG_VOID(ptTrafficReportData, ptMediaProcThreadPara, ptMediaProcThreadPara->ptMcsStatPointer);
    T_psNcuMcsPerform *ptMcsNcuPerform = (T_psNcuMcsPerform*)ptMediaProcThreadPara->ptMcsStatPointer;

    T_NcuTrafficReportInfo *ptNcuSingleTrafficRpt = (T_NcuTrafficReportInfo*)(ptTrafficReportData);
    WORD32 i = 0;
    for (i = 0; i < psNcuGetTrafficRptMaxNum() && i < bPktNum; i++)
    {
        psNcuTrafficReportSolveProc((void *)(ptNcuSingleTrafficRpt+i), wLength, ptMediaProcThreadPara);
        MCS_LOC_STAT_EX(ptMcsNcuPerform, qwSolveSingleTrafficReport, 1);
    }
}

void psNcuTrafficReportSolveProc(void* ptTrafficReportData, WORD16 wLength, T_MediaProcThreadPara *ptMediaProcThreadPara)
{
    MCS_PCLINT_NULLPTR_RET_3ARG_VOID(ptTrafficReportData, ptMediaProcThreadPara, ptMediaProcThreadPara->ptMcsStatPointer);
    
    T_psNcuMcsPerform *ptNcuPerform = (T_psNcuMcsPerform*)ptMediaProcThreadPara->ptMcsStatPointer;
    T_NcuTrafficReportInfo *ptTrafficReportInfo = (T_NcuTrafficReportInfo*)ptTrafficReportData;
    ptTrafficReportInfo = (T_NcuTrafficReportInfo*)ptTrafficReportData;
    ptMediaProcThreadPara->ddwUPseid = ptTrafficReportInfo->ddwUpSeid;
    ptMediaProcThreadPara->dwSubAppid = ptTrafficReportInfo->dwSubAppid;
    //校验会话和订阅上下文是否存在
    DEBUG_TRACE(DEBUG_LOW, "ddwUPseid=%llu, dwSubAppid=%u, dwhDBB=%u\n",ptTrafficReportInfo->ddwUpSeid,ptTrafficReportInfo->dwSubAppid, ptMediaProcThreadPara->dwhDBByThreadNo);
    MCS_CHK_VOID(MCS_RET_FAIL == psPktCheckNcuCtxIsNullInternal(ptMediaProcThreadPara));

    BYTE bIPType     = ptTrafficReportInfo->bIPType;
    WORD32 dwReportPktNum = ptTrafficReportInfo->dwReportPktNum;
    WORD32 dwTrafficLength = ptTrafficReportInfo->dwTrafficLength;
    //提取五元组
    MCS_CHK_VOID_STAT((bIPType != IPv4_VERSION) && (bIPType != IPv6_VERSION), qwTrafficReportIpTypeError);
    DB_STRM_INDEX  tQuintuple  = {{0},{0},0,0,0,0,0,{0}};
    tQuintuple.ddwVPNInfo      = ptMediaProcThreadPara->ddwUPseid;
    tQuintuple.bIPType         = bIPType;
    tQuintuple.bProType         = ptTrafficReportInfo->bProType;
    tQuintuple.wCliPort         = ptTrafficReportInfo->wCliPort;
    tQuintuple.wSvrPort         = ptTrafficReportInfo->wSvrPort;
    IPV6_COPY(tQuintuple.tSvrIP,ptTrafficReportInfo->tSvrIP);
    IPV6_COPY(tQuintuple.tCliIP,ptTrafficReportInfo->tCliIP);
    DEBUG_TRACE(DEBUG_LOW, "bIPType=%u, bProType=%u, wCliPort=%u, wSvrPort=%u\n",bIPType,ptTrafficReportInfo->bProType,ptTrafficReportInfo->wCliPort,ptTrafficReportInfo->wSvrPort);
    if(IPv4_VERSION == bIPType)
    {
        DEBUG_TRACE(DEBUG_LOW, "ipsrc=%u.%u.%u.%u\n",tQuintuple.tCliIP[0],tQuintuple.tCliIP[1],tQuintuple.tCliIP[2],tQuintuple.tCliIP[3]);
        DEBUG_TRACE(DEBUG_LOW, "ipdst=%u.%u.%u.%u\n",tQuintuple.tSvrIP[0],tQuintuple.tSvrIP[1],tQuintuple.tSvrIP[2],tQuintuple.tSvrIP[3]);
    }
    else if(IPv6_VERSION == bIPType)
    {
        DEBUG_TRACE(DEBUG_LOW,"ipsrc ipv6= %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x\n"
                    "ipdst ipv6= %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x\n",
                    tQuintuple.tCliIP[0],tQuintuple.tCliIP[1],tQuintuple.tCliIP[2],tQuintuple.tCliIP[3],
                    tQuintuple.tCliIP[4],tQuintuple.tCliIP[5],tQuintuple.tCliIP[6],tQuintuple.tCliIP[7],
                    tQuintuple.tCliIP[8],tQuintuple.tCliIP[9],tQuintuple.tCliIP[10],tQuintuple.tCliIP[11],
                    tQuintuple.tCliIP[12],tQuintuple.tCliIP[13],tQuintuple.tCliIP[14],tQuintuple.tCliIP[15],
                    tQuintuple.tSvrIP[0],tQuintuple.tSvrIP[1],tQuintuple.tSvrIP[2],tQuintuple.tSvrIP[3],
                    tQuintuple.tSvrIP[4],tQuintuple.tSvrIP[5],tQuintuple.tSvrIP[6],tQuintuple.tSvrIP[7],
                    tQuintuple.tSvrIP[8],tQuintuple.tSvrIP[9],tQuintuple.tSvrIP[10],tQuintuple.tSvrIP[11],
                    tQuintuple.tSvrIP[12],tQuintuple.tSvrIP[13],tQuintuple.tSvrIP[14],tQuintuple.tSvrIP[15]);
    }

    T_psNcuFlowCtx *ptStmAddr = psNcuQryOrCrtStreamProc(ptMediaProcThreadPara, &tQuintuple);
    if(NULL == ptStmAddr)
    {
        DEBUG_TRACE(DEBUG_ERR, "stream qry or crt fail!\n");
        MCS_LOC_STAT_EX(ptNcuPerform, qwQryOrCrtStreamFail, 1);
        return;
    }
    MCS_LOC_STAT_EX(ptNcuPerform, qwQryOrCrtStreamSucc, 1);
    DEBUG_TRACE(DEBUG_LOW, "dwReportPktNum=%u, dwTrafficLength=%u\n",dwReportPktNum,dwTrafficLength);
    ptStmAddr->dwPktNum += dwReportPktNum;
    ptStmAddr->dwPktBytes += dwTrafficLength;
    ptStmAddr->dwSubAppId = ptMediaProcThreadPara->dwSubAppid;

    ptStmAddr->b5QI = ptTrafficReportInfo->b5QI;
    

    if(ptTrafficReportInfo->bSpecialStmFlag)
    {
        ptStmAddr->bSpecialStmFlag = 1;
    }

    /* 带宽计算 */
    psNcuCalcPktBandWithByTrafficInfo(ptMediaProcThreadPara, ptStmAddr, ptTrafficReportInfo);
    psNcuUpdateDAtaAppCtx(ptMediaProcThreadPara, ptStmAddr);

    if(ptTrafficReportInfo->bDelStream)
    {
        MCS_LOC_STAT_EX(ptNcuPerform, qwStreamDelByTrafficRpt, 1);
        psDelStmProc(ptMediaProcThreadPara,ptStmAddr);
    }
    return;
}

void psNcuUpdateDAtaAppCtx(T_MediaProcThreadPara* ptMediaProcThreadPara, T_psNcuFlowCtx * ptStmAddr)
{
    MCS_PCLINT_NULLPTR_RET_3ARG_VOID(ptStmAddr, ptMediaProcThreadPara, ptMediaProcThreadPara->ptMcsStatPointer);
    T_psNcuMcsPerform *ptMcsNcuPerform = (T_psNcuMcsPerform*)ptMediaProcThreadPara->ptMcsStatPointer;
    T_psNcuDaAppCtx *ptDataAppCtx = (T_psNcuDaAppCtx*)ptStmAddr->ptNcuDaCtx;
    if(NULL != ptDataAppCtx)
    {
        if(ptDataAppCtx->dwSubAppid != ptStmAddr->dwSubAppId)
        {
            //应该不会存在。
            MCS_LOC_STAT_EX(ptMcsNcuPerform, qwTRDaAppCtxSubAppidDiffFromStm, 1);
            DEBUG_TRACE(DEBUG_LOW,"NULL != ptDataAppCtx && ptDataAppCtx->dwSubAppid != ptStmAddr->dwSubAppId\n");
            
            psNcuRmStmFromDataApp(ptMediaProcThreadPara, ptStmAddr, ptDataAppCtx);
            ptDataAppCtx = NULL;
            ptStmAddr->dwSubAppId = 0;
            ptStmAddr->ptNcuDaCtx = NULL;

            psCheckNcuToPfuSynSessionCtx(ptMediaProcThreadPara);
        }
        else
        {
            ptDataAppCtx->dwUpdateTimeStamp = ptMediaProcThreadPara->dwPktPowOnSec;
            T_psNcuDaSubScribeCtx* ptSubScribeCtx = (T_psNcuDaSubScribeCtx*)ptDataAppCtx->ptSubScribeCtx;
            if(NULL == ptSubScribeCtx)
            {
                MCS_LOC_STAT_EX(ptMcsNcuPerform, qwTRDaAppNoSubScribeCtx, 1);
            }
            else
            {
                ptSubScribeCtx->dwUpdateTimeStamp = ptMediaProcThreadPara->dwPktPowOnSec;
            }
            T_psNcuSessionCtx* ptSessionCtx = (T_psNcuSessionCtx*)ptDataAppCtx->ptSessionCtx;
            if(NULL == ptSessionCtx)
            {
                MCS_LOC_STAT_EX(ptMcsNcuPerform, qwTRDaAppNoSessionCtx, 1);
            }
            else
            {
                ptSessionCtx->dwUpdateTimeStamp = ptMediaProcThreadPara->dwPktPowOnSec;
            }
        }
    }
    if(NULL == ptDataAppCtx)
    {
        DEBUG_TRACE(DEBUG_LOW,"psNcuAddStmToDataApp\n");
        ptDataAppCtx = psNcuGetDataAppProc(ptMediaProcThreadPara);
        if(NULL == ptDataAppCtx)
        {
            MCS_LOC_STAT_EX(ptMcsNcuPerform, qwTRStreamGetDataAppFail, 1);
            DEBUG_TRACE(DEBUG_LOW,"psNcuGetDataAppProc get NULL\n");
        }
        psNcuAddStmToDataApp(ptMediaProcThreadPara, ptStmAddr, ptDataAppCtx);
        return;
    }
    return;
}

void psCheckNcuToPfuSynSessionCtx(T_MediaProcThreadPara* ptMediaProcThreadPara)
{
    MCS_CHECK_NULLPTR_RET_VOID(ptMediaProcThreadPara);
    T_psNcuMcsPerform *ptMcsNcuPerform = (T_psNcuMcsPerform*)ptMediaProcThreadPara->ptMcsStatPointer;
    MCS_CHECK_NULLPTR_RET_VOID(ptMcsNcuPerform);
    WORD64 ddwUPSeid = ptMediaProcThreadPara->ddwUPseid;
    WORD32   dwhDB = ptMediaProcThreadPara->dwhDBByThreadNo;
    
    T_psNcuSessionCtx* ptSessionCtx = psQuerySessionByUpseid(ddwUPSeid,dwhDB);
    if(NULL == ptSessionCtx)
    {
        return;
    }
    if(ptSessionCtx->bNcuToPfuSynSessionctx == 1)
    {
        return;
    }
    psNcuToPfuSynSessionCtxReq(ptMediaProcThreadPara);            
    ptSessionCtx->bNcuToPfuSynSessionctx = 1;
    ptSessionCtx->dwUpdateTimeStamp = psFtmGetPowerOnSec();
    MCS_LOC_STAT_EX(ptMcsNcuPerform, qwNcuToPfuSynWhenStmSubAppidOld, 1);
    
    return;
}

WORD32 g_ChkNcuCtxNullInternal = 10;
WORD32 psPktCheckNcuCtxIsNullInternal(T_MediaProcThreadPara* ptMediaProcThreadPara)
{
    WORD32 dwInterval = g_ChkNcuCtxNullInternal;
    static __thread WORD32 s_dwTimes = 0;
    if (0 != dwInterval && s_dwTimes++ % dwInterval)
    {
        return MCS_RET_SUCCESS;
    }
    return psPktCheckNcuCtxIsNull(ptMediaProcThreadPara);
}
WORD32 psPktCheckNcuCtxIsNull(T_MediaProcThreadPara* ptMediaProcThreadPara)
{
    MCS_CHECK_NULLPTR_RET_1ARG(ptMediaProcThreadPara, MCS_RET_FAIL);
    T_psNcuMcsPerform *ptMcsNcuPerform = (T_psNcuMcsPerform*)ptMediaProcThreadPara->ptMcsStatPointer;
    MCS_CHECK_NULLPTR_RET_1ARG(ptMcsNcuPerform, MCS_RET_FAIL);

    /*检查会话上下文*/
    WORD64 ddwUPSeid = ptMediaProcThreadPara->ddwUPseid;
    WORD32   dwhDB = ptMediaProcThreadPara->dwhDBByThreadNo;
    T_psNcuSessionCtx* ptSessionCtx = psQuerySessionByUpseid(ddwUPSeid,dwhDB);
    if(NULL == ptSessionCtx)
    {
        MCS_LOC_STAT_EX(ptMcsNcuPerform, qwPktCheckCtxSessionCtxFail, 1);
        psNcuToPfuSynSessionCtxProc(ptMediaProcThreadPara);
        return MCS_RET_FAIL;
    }
    if(ptSessionCtx->bNcuToPfuSynSessionctx == 1)
    {
        MCS_LOC_STAT_EX(ptMcsNcuPerform, qwPktWaitRcvPfuSynResponse, 1);
        return MCS_RET_FAIL;
    }

    /*检查订阅上下文*/
    WORD32   dwSubAppid           = ptMediaProcThreadPara->dwSubAppid;
    WORD32   dwAppid    = psNcuGetAppidBySubAppid(dwSubAppid);
    if(dwAppid == INVALIDINNERAPPID)
    {
        MCS_LOC_STAT_EX(ptMcsNcuPerform, qwPktCheckCtxGetAppidFail, 1);
        psNcuToPfuSynSessionCtxReq(ptMediaProcThreadPara);
        ptSessionCtx->bNcuToPfuSynSessionctx = 1;
        ptSessionCtx->dwUpdateTimeStamp = psFtmGetPowerOnSec();
        return MCS_RET_FAIL;
    }
    
    T_psNcuDaSubScribeCtx* ptSubScribeCtx = psQuerySubscribeByUpseidAppid(ddwUPSeid, dwAppid, dwhDB);
    if(NULL == ptSubScribeCtx)
    {
        MCS_LOC_STAT_EX(ptMcsNcuPerform, qwPktCheckCtxQuerySubscribeFail, 1);
        psNcuToPfuSynSessionCtxReq(ptMediaProcThreadPara);
        ptSessionCtx->bNcuToPfuSynSessionctx = 1;
        ptSessionCtx->dwUpdateTimeStamp = psFtmGetPowerOnSec();
        return MCS_RET_FAIL;
    }
    
    return MCS_RET_SUCCESS;
}

VOID psNcuFillStrmIndex(T_MediaProcThreadPara* ptMediaProcThreadPara, DB_STRM_INDEX* ptStmIdx)
{
    if (NULL == ptMediaProcThreadPara || NULL == ptStmIdx) 
    {
        return ;
    }

    T_psNcuPktDesc *ptPktDesc = &ptMediaProcThreadPara->tPktDesc;
    ptStmIdx->ddwVPNInfo      = ptMediaProcThreadPara->ddwUPseid;
    ptStmIdx->bIPType         = ptPktDesc->bIpType;
    ptStmIdx->bProType        = ptPktDesc->bPro;
    if (MCS_PKT_DIR_DL == ptPktDesc->bDir)
    {
        ptStmIdx->wCliPort = ptPktDesc->wDstPort;
        ptStmIdx->wSvrPort = ptPktDesc->wSrcPort;
        zte_memcpy_s(ptStmIdx->tCliIP, IPV6_LEN, ptPktDesc->tDstIp, IPV6_LEN);
        zte_memcpy_s(ptStmIdx->tSvrIP, IPV6_LEN, ptPktDesc->tSrcIp, IPV6_LEN);
        NCU_PM_55304_STAT_ADD(qwNcuRcvDlCpyPkts, 1);
    }
    else
    {
        ptStmIdx->wCliPort = ptPktDesc->wSrcPort;
        ptStmIdx->wSvrPort = ptPktDesc->wDstPort;
        zte_memcpy_s(ptStmIdx->tCliIP, IPV6_LEN, ptPktDesc->tSrcIp, IPV6_LEN);
        zte_memcpy_s(ptStmIdx->tSvrIP, IPV6_LEN, ptPktDesc->tDstIp, IPV6_LEN);
        NCU_PM_55304_STAT_ADD(qwNcuRcvUlCpyPkts, 1);
    }

    if(IPv4_VERSION == ptPktDesc->bIpType)
    {
        DEBUG_TRACE(DEBUG_LOW, "ipsrc=%u.%u.%u.%u\n", MCS_LOG_IPV4_FIELD(ptStmIdx->tCliIP));
        DEBUG_TRACE(DEBUG_LOW, "ipdst=%u.%u.%u.%u\n", MCS_LOG_IPV4_FIELD(ptStmIdx->tSvrIP));
    }
    else
    {
        DEBUG_TRACE(DEBUG_LOW,"ipsrc ipv6= %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x\n"
            "ipdst ipv6= %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x\n",
            MCS_LOG_IPV6_FIELD(ptStmIdx->tCliIP), MCS_LOG_IPV6_FIELD(ptStmIdx->tSvrIP));
    }
    DEBUG_TRACE(DEBUG_LOW, "iptype=%u, srcport=%u, dstport=%u, pro=%u, upseid=%llu\n", ptStmIdx->bIPType, ptStmIdx->wCliPort, ptStmIdx->wSvrPort, ptStmIdx->bProType, ptStmIdx->ddwVPNInfo);
    return ;
}