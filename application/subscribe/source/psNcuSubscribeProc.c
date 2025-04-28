#include "psNcuSubscribeProc.h"
#include "ps_ncu_session.h"
#include "psNcuSubscribeCtxProc.h"
#include "ps_ncu_dataApp.h"
#include "psNcuSubBindAppCtxProc.h"
#include "ps_db_define_ncu.h"
#include "zte_slibc.h"
#include "MemShareCfg.h"
#include "psNcuCtxFunc.h"
#include "psNcuGetCfg.h"
#include "psMcsDebug.h"
#include "McsIPv4Head.h"
#include "McsIPv6Head.h"
#include "psNcuStreamProc.h"
#include "psNcuDataAnalysis.h"
#include "psMcsSubScribeDebug.h"
#include "psNcuReportSBCProc.h"
#include "psNcuReportTimer.h"
#include "psNcuReport.h"
#include "psNcuDataEncode.h"
#include "psNcuApplicationMapCtxProc.h"
#include "ps_ncu_typedef.h"
#include "psNcuHttpLinkProc.h"
#include "ps_db_define_pfu.h"
#include "ncuSCCUAbility.h"
#include "xdb_core_pfu_que.h"
#include "ncuUserGroup.h"

extern WORD32 g_exp_tim;
#define MCS_CHK_55303STAT(EXPR, STATITEM)   {if(unlikely(EXPR)) {NCU_PM_55303_STAT_ADD(STATITEM, 1);}}


void psNcuChgForSessionPm(BYTE bOldSubType, BYTE bNewSubType);
WORD32 psNcuAddSessionProc(T_MediaProcThreadPara *ptMediaProcThreadPara, T_NcuSynSessInfo* ptSessionInfo);
void psNcuInitSession(T_psNcuSessionCtx* ptSession, T_NcuSynSessInfo* ptSessionInfo, T_psNcuMcsPerform *ptMcsNcuPerform);
void psNcuLoopSolveAllAppidByUPseid(T_MediaProcThreadPara *ptMediaProcThreadPara, WORD32* appidList, WORD32 appidlen);
WORD32 psNcuUpdScribeInfoToSession(BYTE bSubType, T_psNcuSessionCtx* ptSession, T_NcuSynSubInfo*  ptSubScribeInfo, T_psNcuMcsPerform *ptMcsNcuPerform);
WORD32 psNcuUpdateDataCtxRptByAppid(WORD32 dwAppid, WORD64 ddwUPSeid, BYTE bSoftThreadNo, BYTE bSubType, T_psNcuMcsPerform *ptMcsNcuPerform);
WORD32 psNcuAddSubscribeProc(T_MediaProcThreadPara *ptMediaProcThreadPara, T_NcuSynSubInfo*  ptSubScribeInfo);
WORD32 psNcuCancelSubscribeProc(T_MediaProcThreadPara *ptMediaProcThreadPara);
WORD32 psNcuDelSubScribeDataByAppid(WORD32 dwAppid, WORD64 ddwUPSeid, BYTE bSoftThreadNo, BYTE bSubType, T_psNcuMcsPerform *ptMcsNcuPerform);
WORD32 psNcuUpdSessionProc(T_MediaProcThreadPara *ptMediaProcThreadPara,T_NcuSynSessInfo* ptSessionInfo);
WORD32 psNcuDelSubScribeDataByAppid(WORD32 dwAppid, WORD64 ddwUPSeid, BYTE bSoftThreadNo, BYTE bSubType, T_psNcuMcsPerform *ptMcsNcuPerform);
VOID psNcuDelSessionByGroupNoSub(T_MediaProcThreadPara *ptMediaProcThreadPara, LPT_PfuQueHeadReg pQueHead, WORD32 dwGtpuNo);
WORD32 psDelSessionCtxAndQue(T_psNcuMcsPerform *ptNcuPerform, T_psNcuSessionCtx* ptSession, WORD32 dwhDBByThreadNo);
void psNcu55303StatProc(T_MediaProcThreadPara *ptMediaProcThreadPara, T_NcuSynSubInfoHead* ptSubscribeHead);
WORD32 psNcuSubScribeProc(void* ptSubscribeData, T_MediaProcThreadPara *ptMediaProcThreadPara)
{
    if(NULL == ptSubscribeData || NULL == ptMediaProcThreadPara || NULL == ptMediaProcThreadPara->ptMcsStatPointer)
    {
        return MCS_RET_FAIL;
    }
    T_psNcuMcsPerform *ptMcsNcuPerform = (T_psNcuMcsPerform*)ptMediaProcThreadPara->ptMcsStatPointer;
    T_psNcuMcsPerform *ptNcuPerform = ptMcsNcuPerform;
    T_NcuSynSubInfoHead* ptSubscribeHead = (T_NcuSynSubInfoHead*)ptSubscribeData;
    printT_NcuSynSubInfoHead(ptSubscribeHead);

    WORD32 dwGroupId   = GET_LDU_USERGROUP_FROM_UPSEID(ptSubscribeHead->upseid);
    if(NCUROUP_STATE_MIGOUT == g_ncuGroupState[dwGroupId])
    {
        DEBUG_TRACE(DEBUG_LOW, "recive migout group SubScribe! ddwUPseid=%llu, dwGroupId=%u!\n", ptSubscribeHead->upseid, dwGroupId);
        MCS_LOC_STAT_EX(ptNcuPerform, qwAddSubscribeGroupMigoutErr, 1);
        return MCS_RET_FAIL;
    }
    
    BYTE bType = ptSubscribeHead->bType;
    ptMediaProcThreadPara->ddwUPseid = ptSubscribeHead->upseid;   
    MCS_CHK_STAT(0 == ptSubscribeHead->upseid, qwMsgUpSeidIsZero);
    
    ptMediaProcThreadPara->dwAppid = ptSubscribeHead->appid;
    DEBUG_TRACE(DEBUG_LOW, "type=%u upseid=%llu, dwGroupId=%u, appid=%u\n", bType, ptSubscribeHead->upseid, dwGroupId, ptSubscribeHead->appid);
    
    switch(bType)
    {
        case NCU_ADD_SESSION:
        {
            NCU_PM_55303_STAT_ADD(qwNcuRecvSessionNewMsgNum, 1);
            NCU_PM_55303_STAT_ADD(qwNcuRecvSessionSyncMsgNum, 1);
            MCS_LOC_STAT_EX(ptMcsNcuPerform, qwRcvAddSession, 1);
            DEBUG_TRACE(DEBUG_LOW, "NCU_ADD_SESSION upseid=%llu,appid=%u, sizeof(T_NcuSynSessInfo)=%lu\n",ptSubscribeHead->upseid,ptSubscribeHead->appid, sizeof(T_NcuSynSessInfo));
            T_NcuSynSessInfo* ptSessionInfo = (T_NcuSynSessInfo*)(ptSubscribeHead+1);
            printT_NcuSessionInfo(ptSessionInfo);
            return psNcuAddSessionProc(ptMediaProcThreadPara, ptSessionInfo);
        }
        case NCU_ADD_SUBSCRIPTION:
        {
            psNcu55303StatProc(ptMediaProcThreadPara,ptSubscribeHead);
            DEBUG_TRACE(DEBUG_LOW, "NCU_ADD_SUBSCRIPTION upseid=%llu,appid=%u, sizeof(T_NcuSynSubInfo)=%lu\n",ptSubscribeHead->upseid,ptSubscribeHead->appid, sizeof(T_NcuSynSubInfo));
            T_NcuSynSubInfo*  ptSubScribeInfo = (T_NcuSynSubInfo*)(ptSubscribeHead+1);
            printT_NcuSynSubInfo(ptSubScribeInfo);
            return psNcuAddSubscribeProc(ptMediaProcThreadPara, ptSubScribeInfo);
        }
        case NCU_CANCEL_SUBSCRIPTION:
        {
            switch(ptSubscribeHead->bSubType)
            {
                case PfuQosAna:
                    ptMediaProcThreadPara->bSubType = enumQosAna;
                    NCU_PM_55303_STAT_ADD(qwNcuRecvQoSCancelSubscribeMsgFromN4Num, 1);
                    NCU_PM_55303_STAT_ADD(qwNcuRecvQoSSubscribeMsgFromN4Num, 1);
                    break;
                case PfuExpSpecial:
                    ptMediaProcThreadPara->bSubType = enumQosExpSpecial;
                    NCU_PM_55303_STAT_ADD(qwNcuRecvExpDataCancelSubscribeMsgFromN4Num, 1);
                    NCU_PM_55303_STAT_ADD(qwNcuRecvExpDataSubscribeMsgFromN4Num, 1);
                    break;
                case PfuExpNormal:
                    ptMediaProcThreadPara->bSubType = enumQosExpNormal;
                    NCU_PM_55303_STAT_ADD(qwNcuRecvExpDataCancelSubscribeMsgFromSBI, 1);
                    NCU_PM_55303_STAT_ADD(qwNcuRecvExpDataSubscribeMsgFromSBI, 1);
                    break;
                default:
                    MCS_LOC_STAT_EX(ptMcsNcuPerform, qwCancelSubscribeUnKownType, 1);
                    return MCS_RET_FAIL;
            }
            NCU_PM_55303_STAT_ADD(qwNcuRecvSubscribeCancelMsgNum, 1);
            NCU_PM_55303_STAT_ADD(qwNcuRecvSubscribeMsgNum, 1);
            MCS_LOC_STAT_EX(ptMcsNcuPerform, qwRcvCancelSubscribe, 1);
            DEBUG_TRACE(DEBUG_LOW, "NCU_CANCEL_SUBSCRIPTION upseid=%llu,appid=%u\n",ptSubscribeHead->upseid,ptSubscribeHead->appid);
            return psNcuCancelSubscribeProc(ptMediaProcThreadPara);
        }
        case NCU_MODIFY_SESSION:
        {
            MCS_LOC_STAT_EX(ptMcsNcuPerform, qwRcvModSession, 1);
            NCU_PM_55303_STAT_ADD(qwNcuRecvSessionModifyMsgNum, 1);
            NCU_PM_55303_STAT_ADD(qwNcuRecvSessionSyncMsgNum, 1);
            DEBUG_TRACE(DEBUG_LOW, "NCU_MODIFY_SESSION upseid=%llu,appid=%u\n",ptSubscribeHead->upseid,ptSubscribeHead->appid);
            T_NcuSynSessInfo* ptSessionInfo = (T_NcuSynSessInfo*)(ptSubscribeHead+1);
            return psNcuUpdSessionProc(ptMediaProcThreadPara, ptSessionInfo);
        }
        case NCU_SYNAPPID:
        {
            MCS_LOC_STAT_EX(ptMcsNcuPerform, qwRcvSynAppid, 1);
            break;
            
        }
        case NCU_SYNSUBAPPID:
        {
            MCS_LOC_STAT_EX(ptMcsNcuPerform, qwRcvSynSubappid, 1);
            #ifdef FT_TEST
            T_NcuSynSubAppidInfo* appidrelate =(T_NcuSynSubAppidInfo*)(ptSubscribeHead+1);
            DEBUG_TRACE(DEBUG_LOW, "NCU_SYNSUBAPPID appid = %u, subappid = %u\n", appidrelate->dwAppid, appidrelate->dwSubAppid);
            T_psNcuAppidRelation* ptAppidRelateCtx = psQueryAppidRelateCtxBySubAppid(appidrelate->dwSubAppid);
            if(NULL == ptAppidRelateCtx)
            {
                ptAppidRelateCtx = psCrtAppidRelateCtxBySubAppid(appidrelate->dwSubAppid);
                if(NULL == ptAppidRelateCtx)
                {
                    DEBUG_TRACE(DEBUG_LOW,"psCrtAppidRelateCtxBySubAppid fail\n");
                    return MCS_RET_FAIL;
                }
                psNcuGetDynCtxStrAppIDByInnerId(appidrelate->dwSubAppid, ptAppidRelateCtx->subAppidStr);
                DEBUG_TRACE(DEBUG_LOW,"psCrtAppidRelateCtxBySubAppid succ, subappid=%u\n",appidrelate->dwSubAppid);
            }
            if(ptAppidRelateCtx->dwAppid != appidrelate->dwAppid)
            {
                MCS_LOC_STAT_EX(ptMcsNcuPerform, qwRcvSynSubappidAppidDiff, 1);
                ptAppidRelateCtx->dwAppid = appidrelate->dwAppid;
                psNcuMcsUpdAppidRelateCtxByAppid(appidrelate->dwAppid, ptAppidRelateCtx->dwCtxId);
                DEBUG_TRACE(DEBUG_LOW,"psNcuMcsUpdAppidRelateCtxByAppid , subappid=%u appid=%u\n",appidrelate->dwSubAppid, appidrelate->dwAppid);
            }
            #endif
            break;

        }
        case NCU_DEL_SESSION:
        {
            NCU_PM_55303_STAT_ADD(qwNcuRecvSessionDeleteMsgNum, 1);
            NCU_PM_55303_STAT_ADD(qwNcuRecvSessionSyncMsgNum, 1);
            MCS_LOC_STAT_EX(ptMcsNcuPerform, qwRcvDelSession, 1);
            return psNcuDelSessionProc(ptMediaProcThreadPara);
        }
        case NCU_DEL_STREAM:
        {
            MCS_LOC_STAT_EX(ptMcsNcuPerform, qwRcvDelStream, 1);
            T_NcuSynStmRelInfo* ptStmRel = (T_NcuSynStmRelInfo*)(ptSubscribeHead+1);
            DB_STRM_INDEX  tQuintuple  = {{0},{0},0,0,0,0,0,{0}};
            tQuintuple.ddwVPNInfo      = ptMediaProcThreadPara->ddwUPseid;
            tQuintuple.bProType         = ptStmRel->bProType;
            tQuintuple.wCliPort         = ptStmRel->wCliPort;
            tQuintuple.wSvrPort         = ptStmRel->wSvrPort;
            tQuintuple.bIPType          = ptStmRel->bIPType;
            IPV6_COPY(tQuintuple.tSvrIP,ptStmRel->tSvrIP);
            IPV6_COPY(tQuintuple.tCliIP,ptStmRel->tCliIP);
            if(IPv4_VERSION == tQuintuple.bIPType)
            {
                DEBUG_TRACE(DEBUG_LOW, "ipsrc=%u.%u.%u.%u\n",tQuintuple.tCliIP[0],tQuintuple.tCliIP[1],tQuintuple.tCliIP[2],tQuintuple.tCliIP[3]);
                DEBUG_TRACE(DEBUG_LOW, "ipdst=%u.%u.%u.%u\n",tQuintuple.tSvrIP[0],tQuintuple.tSvrIP[1],tQuintuple.tSvrIP[2],tQuintuple.tSvrIP[3]);
            }
            else if(IPv6_VERSION == tQuintuple.bIPType)
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
            else
            {
                MCS_LOC_STAT_EX(ptMcsNcuPerform, qwRcvDelStmIpTypeErr, 1);
            }
            DEBUG_TRACE(DEBUG_LOW, "iptype=%u, srcport=%u, dstport=%u, pro=%u, upseid=%llu\n",tQuintuple.bIPType, tQuintuple.wCliPort, tQuintuple.wSvrPort, tQuintuple.bProType, tQuintuple.ddwVPNInfo);
    
            T_psNcuFlowCtx *ptStmAddr = (T_psNcuFlowCtx *)psNcuQueryStreamByQuintuple(ptMediaProcThreadPara->dwhDBByThreadNo, (VOID *)&tQuintuple);
            if(NULL == ptStmAddr)
            {
                MCS_LOC_STAT_EX(ptMcsNcuPerform, qwRcvDelStreamNoFind, 1);
                return MCS_RET_FAIL;
            }
            return psDelStmProc(ptMediaProcThreadPara, ptStmAddr); 
        }
        default:
        {
            MCS_LOC_STAT_EX(ptMcsNcuPerform, qwRcvUnknownSub, 1);
            break;
        }
    }
    return MCS_RET_FAIL;
}


WORD32 psNcuAddSessionProc(T_MediaProcThreadPara *ptMediaProcThreadPara, T_NcuSynSessInfo* ptSessionInfo)
{
    if(NULL == ptSessionInfo || NULL == ptMediaProcThreadPara || NULL == ptMediaProcThreadPara->ptMcsStatPointer)
    {
        return MCS_RET_FAIL;
    }
    T_psNcuMcsPerform *ptMcsNcuPerform = (T_psNcuMcsPerform*)ptMediaProcThreadPara->ptMcsStatPointer;
    WORD64 ddwUPSeid = ptMediaProcThreadPara->ddwUPseid;
    WORD32 dwhDB     =  ptMediaProcThreadPara->dwhDBByThreadNo;
    
    T_psNcuSessionCtx* ptSession = psQuerySessionByUpseid(ddwUPSeid, dwhDB);
    if(unlikely(NULL != ptSession))
    {
        if(unlikely(ptSession->bNcuToPfuSynSessionctx == 1))
        {
            //如果是NCU给PFU发送同步请求时，建立的上下文
            ptSession->bNcuToPfuSynSessionctx = 0;
            
            ptSession->dwUpdateTimeStamp = ptMediaProcThreadPara->dwPktPowOnSec;
            psVpfuMcsUpdSessionCtxByImsi(ptSessionInfo->bImsi, ptSession->dwSessionCtxId, dwhDB);
            set_last_session(ddwUPSeid,dwhDB);
            
            MCS_LOC_STAT_EX(ptMcsNcuPerform, qwRawSendNcuToPfuSynHasResponse, 1);
        }
        else
        {
            MCS_LOC_STAT_EX(ptMcsNcuPerform, qwAddSessionHasOld, 1);
            ptSession->dwUpdateTimeStamp = ptMediaProcThreadPara->dwPktPowOnSec;
            DEBUG_TRACE(DEBUG_LOW, "GetSession Ok By UPseid=%llu, hDB=%u\n", ddwUPSeid, dwhDB);
        }
    }
    else
    {
        DEBUG_TRACE(DEBUG_LOW, "psCreateSessionByUpseid upseid=%llu\n",ddwUPSeid);
        ptSession = psCreateSessionAndQueByUpseid(ptMcsNcuPerform, ddwUPSeid, dwhDB);
        if(unlikely(NULL == ptSession))
        {
            MCS_LOC_STAT_EX(ptMcsNcuPerform, qwCrtSessionFail, 1);
            DEBUG_TRACE(DEBUG_LOW, "Create T_psNcuSessionCtx Error!\n");
            return MCS_RET_FAIL;
        }
        ptSession->dwCreateTimeStamp = psFtmGetCurStdSec()+0xBC17C200;
        MCS_LOC_STAT_EX(ptMcsNcuPerform, qwCrtSessionSucc, 1);
        NCU_PM_55302_STAT_ADD(qwCurrentSessions,1);
        
        ptSession->dwUpdateTimeStamp = ptMediaProcThreadPara->dwPktPowOnSec;
        psVpfuMcsUpdSessionCtxByImsi(ptSessionInfo->bImsi, ptSession->dwSessionCtxId, dwhDB);
        set_last_session(ddwUPSeid,dwhDB);
    }
    DEBUG_TRACE(DEBUG_LOW, "psNcuInitSession\n");
    psNcuInitSession(ptSession, ptSessionInfo, ptMcsNcuPerform);
    return MCS_RET_SUCCESS;
}

void psNcuInitSession(T_psNcuSessionCtx* ptSession, T_NcuSynSessInfo* ptSessionInfo, T_psNcuMcsPerform *ptMcsNcuPerform)
{
    if(NULL == ptSession || NULL == ptSessionInfo || NULL == ptMcsNcuPerform)
    {
        return;
    }
    
    ptSession->dwSessionID = ptSessionInfo->dwSessionID;
    zte_memcpy_s(ptSession->Apn,64, ptSessionInfo->apnname,64);
    ptSession->bRatType    = ptSessionInfo->bRatType;
    ptSession->bMSIPType   = ptSessionInfo->bMSIPtype;
    zte_memcpy_s(&ptSession->tSNssai, sizeof(T_NcuSnssai), &ptSessionInfo->tSNssai, sizeof(T_NcuSnssai));
    zte_memcpy_s(ptSession->bImsi, IMSI_LEN, ptSessionInfo->bImsi, IMSI_LEN);
    zte_memcpy_s(ptSession->bIsdn, IMSI_LEN, ptSessionInfo->bIsdn, IMSI_LEN);
    zte_memcpy_s(ptSession->tMSIPv4, IPV4_LEN, ptSessionInfo->tMSIPv4, IPV4_LEN);
    zte_memcpy_s(ptSession->tMSIPv6, IPV6_LEN, ptSessionInfo->tMSIPv6, IPV6_LEN);
    if(ptSessionInfo->tUli_Flag)
    {
        zte_memcpy_s(&ptSession->tUserLocation,sizeof(UserLocation), &ptSessionInfo->tUli, sizeof(UserLocation));
        ptSession->bHasUli  = 1;
    }
    else
    {
        MCS_LOC_STAT_EX(ptMcsNcuPerform, qwSessionNotHasUli, 1);
    }
    return;
}


WORD32 psNcuAddSubscribeProc(T_MediaProcThreadPara *ptMediaProcThreadPara, T_NcuSynSubInfo*  ptSubScribeInfo)
{
    if( NULL == ptSubScribeInfo || NULL == ptMediaProcThreadPara || NULL == ptMediaProcThreadPara->ptMcsStatPointer)
    {
        return MCS_RET_FAIL;
    }
    T_psNcuMcsPerform *ptMcsNcuPerform = (T_psNcuMcsPerform*)ptMediaProcThreadPara->ptMcsStatPointer;
    WORD64 ddwUPSeid = ptMediaProcThreadPara->ddwUPseid;
    WORD32 dwhDB     =  ptMediaProcThreadPara->dwhDBByThreadNo;
    BYTE   bSubType  = ptMediaProcThreadPara->bSubType;

    T_psNcuSessionCtx* ptSession = psQuerySessionByUpseid(ddwUPSeid, dwhDB);
    if(unlikely(NULL != ptSession))
    {
        ptSession->dwUpdateTimeStamp = ptMediaProcThreadPara->dwPktPowOnSec;
        DEBUG_TRACE(DEBUG_LOW, "GetSession Ok By UPseid=%llu, hDB=%u\n", ddwUPSeid, dwhDB);
    }
    else
    {
        DEBUG_TRACE(DEBUG_LOW, "GetSession not find upseid=%llu\n",ddwUPSeid);
        MCS_LOC_STAT_EX(ptMcsNcuPerform, qwAddSubscribeNoSession, 1);
        return MCS_RET_FAIL;
    }
    WORD32 dwIndex = 0;
    
    DEBUG_TRACE(DEBUG_LOW, "ptSubScribeInfo wAppidNum=%u ", ptSubScribeInfo->wAppidNum);
    if(0 == ptSubScribeInfo->wAppidNum)
    {
        MCS_LOC_STAT_EX(ptMcsNcuPerform, qwAddSubscribeAppidNone, 1);
        return MCS_RET_FAIL;
    }
    if(MCS_RET_FAIL == psNcuUpdScribeInfoToSession(bSubType, ptSession, ptSubScribeInfo, ptMcsNcuPerform))
    {
        return MCS_RET_FAIL;
    }
    
    psNcuLoopSolveAllAppidByUPseid(ptMediaProcThreadPara,ptSubScribeInfo->appid, MIN(ptSubScribeInfo->wAppidNum, SUBINDI_MAXIELIST));
    T_psNcuDaSubScribeCtx* ptSubScribeCtx = NULL;
    for(dwIndex=0;dwIndex< SUBINDI_MAXIELIST && dwIndex < ptSubScribeInfo->wAppidNum; dwIndex++)
    {
        WORD32 dwAppid  = ptSubScribeInfo->appid[dwIndex];
        if(0 == dwAppid)
        {
            continue;
        }
        DEBUG_TRACE(DEBUG_LOW, "Add subcribe appid = %u, subtype=%u(1=ana,4=n4exp,8=normalexp)\n", dwAppid, bSubType);
        ptSubScribeCtx = psCreatesubscribeByUpseidAppid(ddwUPSeid, dwAppid,dwhDB);
        if(unlikely(NULL == ptSubScribeCtx))
        {
            MCS_LOC_STAT_EX(ptMcsNcuPerform, qwCrtSubscribeCtxFail, 1);
            DEBUG_TRACE(DEBUG_LOW,"Create T_psNcuDaSubScribeCtx Error!\n");
            continue;
        }
        if(0 != ptSubScribeCtx->bSubType)
        {
            MCS_LOC_STAT_EX(ptMcsNcuPerform, qwCrtSubscribeExistDiffSub, 1);
        }
        MCS_LOC_STAT_EX(ptMcsNcuPerform, qwCrtSubscribe, 1);
        DEBUG_TRACE(DEBUG_LOW,"Create T_psNcuDaSubScribeCtx succ\n");
        ptSubScribeCtx->dwCreateTimeStamp = psFtmGetCurStdSec()+0xBC17C200;
        ptSubScribeCtx->dwUpdateTimeStamp = ptMediaProcThreadPara->dwPktPowOnSec;
        ptSubScribeCtx->bSubType |= bSubType;
        psVpfuMcsUpdSubScribeCtxByUPseid(ddwUPSeid, ptSubScribeCtx->dwSubscribeCtxId, dwhDB);
        DEBUG_TRACE(DEBUG_LOW,"ptSubScribeCtx->bSubType=%u(1=ana,4=n4exp,8=normalexp)\n",ptSubScribeCtx->bSubType);
    }
    
    psNcuGetHttpLinkInfo(ptMediaProcThreadPara, ptSubScribeInfo);
    return MCS_RET_SUCCESS;
}
WORD32 psAppidSolveCallBack_JudgeInList(WORD32 curappid, WORD32* appidlist, WORD32 appidlen)
{
    if(NULL == appidlist || 0 == appidlen)
    {
        return MCS_RET_FAIL;
    }
    WORD32 dwIndex = 0;
    for(dwIndex=0;dwIndex<appidlen;dwIndex++)
    {
        if(curappid == appidlist[dwIndex])
        {
            return dwIndex;
        }
    }
    return MCS_RET_FAIL;

}
void psNcuLoopSolveAllAppidByUPseid(T_MediaProcThreadPara *ptMediaProcThreadPara, WORD32* appidList, WORD32 appidlen)
{
    if(NULL == ptMediaProcThreadPara || NULL ==ptMediaProcThreadPara->ptMcsStatPointer || 0 == appidlen)
    {
        return;
    }
    T_psNcuMcsPerform *ptMcsNcuPerform = (T_psNcuMcsPerform*)ptMediaProcThreadPara->ptMcsStatPointer;
    WORD64 ddwUPSeid                   = ptMediaProcThreadPara->ddwUPseid;
    WORD32 dwhDB                       =  ptMediaProcThreadPara->dwhDBByThreadNo;
    BYTE   bSubType                    = ptMediaProcThreadPara->bSubType;
    BYTE  bSoftThreadNo                = ptMediaProcThreadPara->bThreadNo % MEDIA_THRD_NUM;
    WORD32 dwNeedDelAppidList[255]     = {0};
    WORD32 dwNeedUpdAppidList[255]     = {0};
    WORD32 dwNeedDelAppidNum           = 0;
    WORD32 dwNeedUpdAppidNum           = 0;


    MCS_DM_QUERYDYNDATA_ACK *ptMcsDynCtxNoUniqAck = &g_ptVpfuShMemVar->tGwGloData.atMcsDynCtxNoUniqAck[bSoftThreadNo];
    WORD32 allAppidNum = psVpfuMcsGetAllSubScribeCtxByUPseid(ddwUPSeid, dwhDB, (BYTE*)ptMcsDynCtxNoUniqAck);
    WORD32 dwCtxId     = 0;
    WORD32 dwPositionInList  = 0;
    WORD32 dwIndex     = 0;
    if(0 == allAppidNum)
    {
        return;
    }
    MCS_LOC_STAT_EX(ptMcsNcuPerform, qwUpdSubScribeAppidList, 1);
    for(dwIndex=0;dwIndex< EXPECT_SUBSCRIBE_NUM && dwIndex < allAppidNum; dwIndex++)
    {
        dwCtxId = ptMcsDynCtxNoUniqAck->adwDataArea[dwIndex];
        T_psNcuDaSubScribeCtx  *ptSubScribeCtx = psMcsGetsubscribeCtxById(dwCtxId, dwhDB);
        if(NULL == ptSubScribeCtx)
        {
            continue;
        }
        DEBUG_TRACE(DEBUG_LOW, "ptSubScribeCtx->bSubType=%u, msg bSubType=%u(1=ana,4=n4exp,8=normalexp)\n",ptSubScribeCtx->bSubType, bSubType);
        dwPositionInList = psAppidSolveCallBack_JudgeInList(ptSubScribeCtx->dwAppId, appidList, appidlen);
        if(MCS_RET_FAIL != dwPositionInList) //不是index,是索引编号
        {
            dwPositionInList = dwPositionInList%255;
            DEBUG_TRACE(DEBUG_LOW, "Add subcribe appid has exitst [appid=%u]\n", appidList[dwPositionInList]);
            if((0 == (ptSubScribeCtx->bSubType & bSubType)) && (dwNeedUpdAppidNum < 255))
            {
                MCS_LOC_STAT_EX(ptMcsNcuPerform, qwSameAppidSupportMore, 1);
                dwNeedUpdAppidList[dwNeedUpdAppidNum++] = appidList[dwPositionInList]; //已经存在appid上下文，则更新其能力，同时将消息中appidList对应位置置零不需要创建。
            }
            else
            {
                MCS_LOC_STAT_EX(ptMcsNcuPerform, qwSameAppidSameAbility, 1);
            }
            appidList[dwPositionInList] = 0; 
            ptSubScribeCtx->dwUpdateTimeStamp = ptMediaProcThreadPara->dwPktPowOnSec;
            continue;
        }

        //会话中的appid不在此次订阅列表中，则appid可能存在于其他订阅，或者需要删除。不能更新时间。
        DEBUG_TRACE(DEBUG_LOW, "ptSubScribeCtx->bSubType=%u, cursubtype=%u(1=ana,4=n4exp,8=normalexp)\n",ptSubScribeCtx->bSubType, bSubType);
        ptSubScribeCtx->bSubType &= (~bSubType);  //取消
        DEBUG_TRACE(DEBUG_LOW, "ptSubScribeCtx->bSubType=%u\n(1=ana,4=n4exp,8=normalexp)",ptSubScribeCtx->bSubType);
        MCS_LOC_STAT_EX(ptMcsNcuPerform, qwUpdAppidNoFindNeedRemove, 1);
        if(0 != ptSubScribeCtx->bSubType)
        {
            MCS_LOC_STAT_EX(ptMcsNcuPerform, qwUpdAppidNoFindUsedOther, 1);
            DEBUG_TRACE(DEBUG_LOW, "Add subcribe appid has exitst [appid=%u]\n",ptSubScribeCtx->dwAppId);
            continue;
        }
        if(dwNeedDelAppidNum >=255)
        {
            MCS_LOC_STAT_EX(ptMcsNcuPerform, qweRemoveAppidCtxMore255, 1);
            continue;
        }
        DEBUG_TRACE(DEBUG_LOW, "Add subcribe appid need del [appid=%u]\n",ptSubScribeCtx->dwAppId);
        MCS_LOC_STAT_EX(ptMcsNcuPerform, qwUpdSubScribeRemoveAppid, 1);
        dwNeedDelAppidList[dwNeedDelAppidNum++] = ptSubScribeCtx->dwAppId;
        
    }
    for(dwIndex=0;dwIndex<255&& dwIndex<dwNeedDelAppidNum;dwIndex++)
    {
        psNcuDelSubScribeDataByAppid(dwNeedDelAppidList[dwIndex],ddwUPSeid,bSoftThreadNo,bSubType,ptMcsNcuPerform); //由于内部也用了ptMcsDynCtxNoUniqAck，不能放循环里。
    }
    for(dwIndex=0;dwIndex<255&& dwIndex<dwNeedUpdAppidNum;dwIndex++)
    {
        psNcuUpdateDataCtxRptByAppid(dwNeedUpdAppidList[dwIndex], ddwUPSeid, bSoftThreadNo, bSubType, ptMcsNcuPerform);
    }
    
    return;
}

WORD32 psNcuUpdateDataCtxRptByAppid(WORD32 dwAppid, WORD64 ddwUPSeid, BYTE bSoftThreadNo, BYTE bSubType, T_psNcuMcsPerform *ptMcsNcuPerform)
{
    if(NULL == ptMcsNcuPerform)
    {
        return MCS_RET_FAIL;
    }
    WORD32 dwhDB = _NCU_GET_DBHANDLE(bSoftThreadNo);
    T_psNcuDaSubScribeCtx* ptSubScribeCtx = psQuerySubscribeByUpseidAppid(ddwUPSeid, dwAppid, dwhDB);
    if(NULL == ptSubScribeCtx)
    {
        return MCS_RET_FAIL;
    }
    MCS_DM_QUERYDYNDATA_ACK *ptMcsDynCtxNoUniqAck = &g_ptVpfuShMemVar->tGwGloData.atMcsDynCtxNoUniqAck[bSoftThreadNo%MEDIA_THRD_NUM];
    WORD32 dwAppctxNum = psVpfuMcsGetAllDaAppCtxByAppid(ddwUPSeid,dwAppid,dwhDB, (BYTE*)ptMcsDynCtxNoUniqAck);
    WORD32 dwIndex = 0;
    WORD32 dwCtxId   = 0;
    WORD32 dwCurClockStep = getCurSetTimerClockStep();
    
    for(;dwIndex<EXPECT_DATA_APP_NUM && dwIndex < dwAppctxNum; dwIndex++)
    {
        dwCtxId = ptMcsDynCtxNoUniqAck->adwDataArea[dwIndex];
        T_psNcuDaAppCtx* ptDaAppCtx = psMcsGetDaAppCtxById(dwCtxId, dwhDB);
        if(NULL == ptDaAppCtx)
        {
            continue;
        }
        if(bSubType == enumQosAna && (!(ptSubScribeCtx->bSubType & enumQosAna)))
        { //原来没有质差现在有质差
            DEBUG_TRACE(DEBUG_LOW, "old appid=%u no has QosAna but add QosAna", dwAppid);
            MCS_LOC_STAT_EX(ptMcsNcuPerform, qwSubScribeAppAddQosAna, 1);
            MCS_LOC_STAT_EX(ptMcsNcuPerform, qwQosAnaTimerInit, 1);
            psSetAnaTimerInit(ptDaAppCtx,dwCurClockStep,dwhDB);
            continue;
        }
        if(bSubType != enumQosAna && (ptSubScribeCtx->bSubType == enumQosAna))
        {//原来只有质差现在有了体验,不需要更新时间索引，默认会启用exp时间索引。
            DEBUG_TRACE(DEBUG_LOW, "old appid=%u only has QosAna but add exp", dwAppid);
            MCS_LOC_STAT_EX(ptMcsNcuPerform, qwSubScribeAppAddExp, 1);
        }
    }
    ptSubScribeCtx->bSubType |= bSubType; 
    return MCS_RET_SUCCESS;
}

WORD32 psNcuUpdScribeInfoToSession(BYTE bSubType, T_psNcuSessionCtx* ptSession, T_NcuSynSubInfo*  ptSubScribeInfo, T_psNcuMcsPerform *ptMcsNcuPerform)
{
    if(NULL == ptSession || NULL == ptSubScribeInfo || NULL == ptMcsNcuPerform)
    {
        return MCS_RET_FAIL;
    }
    BYTE bOldSubType = ptSession->bSubType;
    ptSession->bSubType |= bSubType;  //会话是什么类型的,每次订阅进行更新
    psNcuChgForSessionPm(bOldSubType, ptSession->bSubType);
    if(bSubType == enumQosExpNormal)
    {
         MCS_LOC_STAT_EX(ptMcsNcuPerform, qwAddSubscribeExpNormal, 1);
         return MCS_RET_SUCCESS;
    }

    if(bSubType == enumQosAna)
    {
        MCS_LOC_STAT_EX(ptMcsNcuPerform, qwAddSubscribeQosAna, 1);
        zte_memcpy_s(ptSession->bCorrelationId_Ana, AUCCORID_MAXLEN, ptSubScribeInfo->aucCorid,AUCCORID_MAXLEN);
        psNcuResetReportLinkFaultFlag(ptSession, ptSubScribeInfo);
        zte_memcpy_s(ptSession->Uri_Ana, AUCREPORTURI_MAXLEN, ptSubScribeInfo->aucReportUri,AUCREPORTURI_MAXLEN);    
    }
    else if(bSubType == enumQosExpSpecial)
    {
        MCS_LOC_STAT_EX(ptMcsNcuPerform, qwAddSubscribeExpSpecial, 1);
        zte_memcpy_s(ptSession->bCorrelationId_Exp, AUCCORID_MAXLEN, ptSubScribeInfo->aucCorid,AUCCORID_MAXLEN);
        zte_memcpy_s(ptSession->Uri_Exp, AUCREPORTURI_MAXLEN, ptSubScribeInfo->aucReportUri,AUCREPORTURI_MAXLEN);    
    }
    else
    {
        MCS_LOC_STAT_EX(ptMcsNcuPerform, qwAddSubscribeUnKownType, 1);
        return MCS_RET_FAIL;
    }
    MCS_LOC_STAT_EX(ptMcsNcuPerform, qwAddN4SubscribeNeedNwdafIP, 1);
    DEBUG_TRACE(DEBUG_LOW, "ptSubScribeInfo->bIplistNum = %u\n", ptSubScribeInfo->bIplistNum);
    if(ptSubScribeInfo->bIplistNum == 0)
    {
        DEBUG_TRACE(DEBUG_LOW,"get SubScribeInfo->bIplistNum=0\n");
        MCS_LOC_STAT_EX(ptMcsNcuPerform, qwAddN4SubscribeNoFindIP, 1);
    }
    T_IPV6 tUpfIP = {0};
    if((TRUE == psCheckIPValid(ptSubScribeInfo->NwdafIPv6_N4, IPv6_VERSION)) && (MCS_RET_SUCCESS == getNcuIP(tUpfIP, IPv6_VERSION)))
    {
        IPV6_COPY(ptSession->tNwdafIPv6, ptSubScribeInfo->NwdafIPv6_N4);
        ptSession->bNwdafIPType |= IPCOMM_TYPE_IPV6;
        MCS_LOC_STAT_EX(ptMcsNcuPerform, qwSubscribeGetNwdafV6, 1);
        DEBUG_TRACE(DEBUG_LOW,"getNcuIP ipv6= %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x,"
            "nwdafipv6= %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x\n",
            tUpfIP[0],tUpfIP[1],tUpfIP[2],tUpfIP[3],
            tUpfIP[4],tUpfIP[5],tUpfIP[6],tUpfIP[7],
            tUpfIP[8],tUpfIP[9],tUpfIP[10],tUpfIP[11],
            tUpfIP[12],tUpfIP[13],tUpfIP[14],tUpfIP[15],
            ptSession->tNwdafIPv6[0],ptSession->tNwdafIPv6[1],ptSession->tNwdafIPv6[2],ptSession->tNwdafIPv6[3],
            ptSession->tNwdafIPv6[4],ptSession->tNwdafIPv6[5],ptSession->tNwdafIPv6[6],ptSession->tNwdafIPv6[7],
            ptSession->tNwdafIPv6[8],ptSession->tNwdafIPv6[9],ptSession->tNwdafIPv6[10],ptSession->tNwdafIPv6[11],
            ptSession->tNwdafIPv6[12],ptSession->tNwdafIPv6[13],ptSession->tNwdafIPv6[14],ptSession->tNwdafIPv6[15]);
    }
    if((TRUE == psCheckIPValid(ptSubScribeInfo->NwdafIPv4_N4, IPv4_VERSION)) && (MCS_RET_SUCCESS == getNcuIP(tUpfIP, IPv4_VERSION)))
    {
        IPV4_COPY(ptSession->tNwdafIPv4, ptSubScribeInfo->NwdafIPv4_N4);
        ptSession->bNwdafIPType |= IPCOMM_TYPE_IPV4;
        MCS_LOC_STAT_EX(ptMcsNcuPerform, qwSubscribeGetNwdafV4, 1);
        DEBUG_TRACE(DEBUG_LOW,"getNcuIP ipv4, ncuip=%u.%u.%u.%u, nwdafipv4=%u.%u.%u.%u\n",
                tUpfIP[0],tUpfIP[1],tUpfIP[2],tUpfIP[3],
                ptSession->tNwdafIPv4[0],ptSession->tNwdafIPv4[1],ptSession->tNwdafIPv4[2],ptSession->tNwdafIPv4[3]);
    }
    return MCS_RET_SUCCESS;
}

WORD32 psNcuCancelSubscribeProc(T_MediaProcThreadPara *ptMediaProcThreadPara)
{
    if(NULL == ptMediaProcThreadPara || NULL == ptMediaProcThreadPara->ptMcsStatPointer)
    {
        return MCS_RET_FAIL;
    }
    T_psNcuMcsPerform *ptMcsNcuPerform = (T_psNcuMcsPerform*)ptMediaProcThreadPara->ptMcsStatPointer;
    WORD64 ddwUPSeid     = ptMediaProcThreadPara->ddwUPseid;
    WORD32 dwhDB         = ptMediaProcThreadPara->dwhDBByThreadNo;
    WORD32 dwAppid       = ptMediaProcThreadPara->dwAppid;
    WORD32 dwIndex       = 0;
    WORD32 dwCtxNum      = 0;
    WORD32 dwCtxId       = 0;
    BYTE   bSoftThreadNo = ptMediaProcThreadPara->bThreadNo % MEDIA_THRD_NUM;
    BYTE   bSubType      = ptMediaProcThreadPara->bSubType;

    MCS_DM_QUERYDYNDATA_ACK *ptMcsDynCtxNoUniqAck = &g_ptVpfuShMemVar->tGwGloData.atMcsDynCtxNoUniqAck[bSoftThreadNo];
    psNcuDelSubScribeDataByAppid(dwAppid,ddwUPSeid,bSoftThreadNo,bSubType,ptMcsNcuPerform);
    T_psNcuSessionCtx* ptSession = psQuerySessionByUpseid(ddwUPSeid, dwhDB);
    if(unlikely(NULL != ptSession))
    {
        ptSession->dwUpdateTimeStamp = ptMediaProcThreadPara->dwPktPowOnSec;
        DEBUG_TRACE(DEBUG_LOW, "GetSession Ok By UPseid=%llu, hDB=%u\n", ddwUPSeid, dwhDB);
    }
    else
    {
        MCS_LOC_STAT_EX(ptMcsNcuPerform, qwCancelSubscribeNoSession, 1);
        return MCS_RET_FAIL;
    }
    
    dwCtxNum = psVpfuMcsGetAllSubScribeCtxByUPseid(ddwUPSeid, dwhDB, (BYTE*)ptMcsDynCtxNoUniqAck);
    BYTE bOldSubType = ptSession->bSubType;
    if(0 == dwCtxNum)
    {
        MCS_LOC_STAT_EX(ptMcsNcuPerform, qwCancelAllSubThenDelSession, 1);
        if (MCS_RET_SUCCESS == psDelSessionCtxAndQue(ptMcsNcuPerform, ptSession, dwhDB))
        {
            NCU_PM_55302_STAT_REMOVE(qwCurrentSessions, 1);
            psNcuChgForSessionPm(bOldSubType, 0);
            MCS_LOC_STAT_EX(ptMcsNcuPerform, qwDelSessionSucc, 1);
            return MCS_RET_SUCCESS;
        }
        MCS_LOC_STAT_EX(ptMcsNcuPerform, qwDelSessionFail, 1);
        return MCS_RET_FAIL;
    }
    ptSession->bSubType = 0;
    for(dwIndex=0;dwIndex<EXPECT_DATA_APP_NUM&& dwIndex<dwCtxNum; dwIndex++)
    {
        dwCtxId = ptMcsDynCtxNoUniqAck->adwDataArea[dwIndex];
        T_psNcuDaSubScribeCtx  *ptSubScribeCtx = psMcsGetsubscribeCtxById(dwCtxId, dwhDB);
        if(NULL == ptSubScribeCtx)
        {
            continue;
        }
        ptSession->bSubType |= ptSubScribeCtx->bSubType; //重新识别会话能力角色
    }
    psNcuChgForSessionPm(bOldSubType,  ptSession->bSubType);
    return MCS_RET_SUCCESS;
    
    
}

WORD32 psNcuDelSubScribeDataByAppid(WORD32 dwAppid, WORD64 ddwUPSeid, BYTE bSoftThreadNo, BYTE bSubType, T_psNcuMcsPerform *ptMcsNcuPerform)
{
    if(NULL == ptMcsNcuPerform)
    {
        return MCS_RET_FAIL;
    }
    WORD32 dwhDB = _NCU_GET_DBHANDLE(bSoftThreadNo);
    T_psNcuDaSubScribeCtx* ptSubScribeCtx = psQuerySubscribeByUpseidAppid(ddwUPSeid, dwAppid, dwhDB);
    if(NULL == ptSubScribeCtx)
    {
        return MCS_RET_FAIL;
    }
    MCS_DM_QUERYDYNDATA_ACK *ptMcsDynCtxNoUniqAck = &g_ptVpfuShMemVar->tGwGloData.atMcsDynCtxNoUniqAck[bSoftThreadNo%MEDIA_THRD_NUM];
    WORD32 dwAppctxNum = psVpfuMcsGetAllDaAppCtxByAppid(ddwUPSeid,dwAppid,dwhDB, (BYTE*)ptMcsDynCtxNoUniqAck);
    WORD32 dwIndex = 0;
    WORD32 dwCtxId   = 0;
    
    ptSubScribeCtx->bSubType &= (~bSubType);  //清除能力
    for(;dwIndex<EXPECT_DATA_APP_NUM && dwIndex < dwAppctxNum; dwIndex++)
    {
        dwCtxId = ptMcsDynCtxNoUniqAck->adwDataArea[dwIndex];
        T_psNcuDaAppCtx* ptDaAppCtx = psMcsGetDaAppCtxById(dwCtxId, dwhDB);
        if(NULL == ptDaAppCtx)
        {
            continue;
        }
        if(0 == ptSubScribeCtx->bSubType)
        {
            MCS_LOC_STAT_EX(ptMcsNcuPerform, qwDelDataAnalysisCtx, 1);
            psNcuUpdAllStmDataToDataApp(ptDaAppCtx, TRUE, 0xff); //不需要，后面就删除DaAppCtx了
            psVpfuMcsDelCtxById(dwCtxId, dwhDB, DB_HANDLE_R_NCUDATAAPP);
        }
        else
        {
            if(bSubType == enumQosAna)
            {  //取消质差，则删除质差定时索引
                MCS_LOC_STAT_EX(ptMcsNcuPerform, qwCancelDataAnalysisAnaRpt, 1);
                psNcuMcsUpdDaAppCtxByClockStep(0, DB_HANDLE_IDX_R_NCU_DATAAPP_ANA_MON_CLOCK_STEP,dwCtxId, dwhDB, enumDeleteIndex);
                psNcuMcsUpdDaAppCtxByClockStep(0, DB_HANDLE_IDX_R_NCU_DATAAPP_ANA_CLOCK_STEP,dwCtxId, dwhDB, enumDeleteIndex);
            }
            else if(ptSubScribeCtx->bSubType == enumQosAna)
            { 
                MCS_LOC_STAT_EX(ptMcsNcuPerform, qwCancelDataAnalysisExpRpt, 1);
            }
        }
    }
    if(0 != ptSubScribeCtx->bSubType)
    {
        MCS_LOC_STAT_EX(ptMcsNcuPerform, qwCancelSubScribeButStillUsed, 1);
        return MCS_RET_SUCCESS;
    }
    if(MCS_RET_SUCCESS == psDelSubscribeByUpseidAppid(ddwUPSeid, dwAppid, dwhDB))
    {
        MCS_LOC_STAT_EX(ptMcsNcuPerform, qwDelSubscribeSucc, 1);
        DEBUG_TRACE(DEBUG_LOW,"del success subscribe ctx!\n");
        return MCS_RET_SUCCESS;
    }
    MCS_LOC_STAT_EX(ptMcsNcuPerform, qwDelSubscribeFail, 1);
    DEBUG_TRACE(DEBUG_LOW,"no find subscribe ctx!\n");
    return MCS_RET_FAIL;
}

WORD32 psNcuDelSessionProc(T_MediaProcThreadPara *ptMediaProcThreadPara)
{
    if(NULL == ptMediaProcThreadPara || NULL == ptMediaProcThreadPara->ptMcsStatPointer)
    {
        return MCS_RET_FAIL;
    }
    T_psNcuMcsPerform *ptMcsNcuPerform = (T_psNcuMcsPerform*)ptMediaProcThreadPara->ptMcsStatPointer;
    WORD64 ddwUPSeid = ptMediaProcThreadPara->ddwUPseid;
    WORD32 dwhDB     = ptMediaProcThreadPara->dwhDBByThreadNo;
    BYTE  bSoftThreadNo = ptMediaProcThreadPara->bThreadNo % MEDIA_THRD_NUM;
    DEBUG_TRACE(DEBUG_LOW,"psNcuDelSessionProc upseid=%llu, threadno=%u\n",ddwUPSeid, bSoftThreadNo);
    T_psNcuSessionCtx* ptSession = psQuerySessionByUpseid(ddwUPSeid, dwhDB);
    if(NULL == ptSession)
    {
        DEBUG_TRACE(DEBUG_LOW, "GetSession fail By UPseid=%llu, hDB=%u\n", ddwUPSeid, dwhDB);
        return MCS_RET_FAIL;
    }
    WORD32 dwIndex   = 0;
    WORD32 dwCtxId   = 0;
    MCS_DM_QUERYDYNDATA_ACK *ptMcsDynCtxNoUniqAck = &g_ptVpfuShMemVar->tGwGloData.atMcsDynCtxNoUniqAck[bSoftThreadNo];
    WORD32 dwSubscribeNum = psVpfuMcsGetAllSubScribeCtxByUPseid(ddwUPSeid, dwhDB, (BYTE*)ptMcsDynCtxNoUniqAck);
    WORD32 dwAppidList[EXPECT_SUBSCRIBE_NUM] = {0};
    WORD32 num =0;
    DEBUG_TRACE(DEBUG_LOW,"ppsVpfuMcsGetAllSubScribeCtxByUPseid num=%u\n",dwSubscribeNum);
    for(dwIndex=0; dwIndex <EXPECT_SUBSCRIBE_NUM && dwIndex<dwSubscribeNum; dwIndex++)
    {
        dwCtxId = ptMcsDynCtxNoUniqAck->adwDataArea[dwIndex];
        T_psNcuDaSubScribeCtx  *ptSubScribeCtx = psMcsGetsubscribeCtxById(dwCtxId, dwhDB);
        if(NULL != ptSubScribeCtx)
        {
            dwAppidList[num] = ptSubScribeCtx->dwAppId;
            num ++;
        }
        if(MCS_RET_FAIL == psVpfuMcsDelCtxById(dwCtxId, dwhDB, DB_HANDLE_R_NCUSUBCRIBE))
        {
            DEBUG_TRACE(DEBUG_LOW, "SubScribe ctx del fail!\n");
            MCS_LOC_STAT_EX(ptMcsNcuPerform, qwDelSubscribeFail, 1);
        }
        else
        {
            MCS_LOC_STAT_EX(ptMcsNcuPerform, qwDelSubscribeSucc, 1);
        }
    }
    for(dwIndex=0;dwIndex<EXPECT_SUBSCRIBE_NUM && dwIndex<num;dwIndex++)
    {
        WORD32 dwAppctxNum = psVpfuMcsGetAllDaAppCtxByAppid(ddwUPSeid,dwAppidList[dwIndex],dwhDB, (BYTE*)ptMcsDynCtxNoUniqAck);
        DEBUG_TRACE(DEBUG_LOW,"psVpfuMcsGetAllDaAppCtxByAppid appid=%u, dwAppctxNum=%u\n",dwAppidList[dwIndex],dwAppctxNum);
        WORD32 j = 0;
        for(;j<EXPECT_DATA_APP_NUM && j < dwAppctxNum; j++)
        {
            dwCtxId = ptMcsDynCtxNoUniqAck->adwDataArea[j];
            psNcuUpdAllStmDataToDataApp(psMcsGetDaAppCtxById(dwCtxId, dwhDB), TRUE, 0xff);
            if (MCS_RET_FAIL == psVpfuMcsDelCtxById(dwCtxId, dwhDB, DB_HANDLE_R_NCUDATAAPP))
            {
                DEBUG_TRACE(DEBUG_LOW, "DataApp ctx del fail!\n");
                MCS_LOC_STAT_EX(ptMcsNcuPerform, qwDelDataAppFail, 1);
            }
            else
            {
                MCS_LOC_STAT_EX(ptMcsNcuPerform, qwDelDataAppSucc, 1);
            }
        }
    }

    WORD32 ret = psDelSessionCtxAndQue(ptMcsNcuPerform, ptSession, dwhDB);
    if (MCS_RET_SUCCESS == ret)
    {
        NCU_PM_55302_STAT_REMOVE(qwCurrentSessions, 1);
        psNcuChgForSessionPm(ptSession->bSubType, 0);
        MCS_LOC_STAT_EX(ptMcsNcuPerform, qwDelSessionSucc, 1);
    }
    else
    {
        MCS_LOC_STAT_EX(ptMcsNcuPerform, qwDelSessionFail, 1);
    }
    return ret;
}

void psNcuUserLocationChangeProc(T_psNcuSessionCtx* ptSession, T_MediaProcThreadPara *ptMediaProcThreadPara)
{
    if(NULL == ptSession || NULL == ptMediaProcThreadPara) 
    {
        return;
    }
    T_psNcuMcsPerform *ptMcsNcuPerform = (T_psNcuMcsPerform*)ptMediaProcThreadPara->ptMcsStatPointer;
    if(NULL == ptMcsNcuPerform)
    {
        return;
    }
    WORD64 ddwUPSeid = ptMediaProcThreadPara->ddwUPseid;
    WORD32 dwhDB     = ptMediaProcThreadPara->dwhDBByThreadNo;
    BYTE  bSoftThreadNo = ptMediaProcThreadPara->bThreadNo % MEDIA_THRD_NUM;
    MCS_DM_QUERYDYNDATA_ACK *ptMcsDynCtxNoUniqAck = &g_ptVpfuShMemVar->tGwGloData.atMcsDynCtxNoUniqAck[bSoftThreadNo];
    WORD32 dwSubscribeNum = psVpfuMcsGetAllSubScribeCtxByUPseid(ddwUPSeid, dwhDB, (BYTE*)ptMcsDynCtxNoUniqAck);
    WORD32 dwAppidList[EXPECT_SUBSCRIBE_NUM] = {0};
    WORD32 num =0;
    WORD32 dwIndex   = 0;
    WORD32 dwCtxId   = 0;
    DEBUG_TRACE(DEBUG_LOW,"psNcuUserLocationChangeProc get SubscribeNum=%u\n",dwSubscribeNum);
    for(dwIndex=0; dwIndex <EXPECT_SUBSCRIBE_NUM && dwIndex<dwSubscribeNum && num < EXPECT_SUBSCRIBE_NUM; dwIndex++)
    {
        dwCtxId = ptMcsDynCtxNoUniqAck->adwDataArea[dwIndex];
        T_psNcuDaSubScribeCtx  *ptSubScribeCtx = psMcsGetsubscribeCtxById(dwCtxId, dwhDB);
        if(NULL != ptSubScribeCtx)
        {
            dwAppidList[num] = ptSubScribeCtx->dwAppId;
            num ++;
        }
    }
    for (dwIndex = 0;dwIndex < EXPECT_SUBSCRIBE_NUM && dwIndex < num;dwIndex++)
    {
        WORD32 dwAppctxNum = psVpfuMcsGetAllDaAppCtxByAppid(ddwUPSeid, dwAppidList[dwIndex], dwhDB, (BYTE*)ptMcsDynCtxNoUniqAck);
        DEBUG_TRACE(DEBUG_LOW, "psNcuUserLocationChangeProc appid=%u, dwAppctxNum=%u\n", dwAppidList[dwIndex], dwAppctxNum);
        WORD32 j = 0;
        for (;j < EXPECT_DATA_APP_NUM && j < dwAppctxNum; j++)
        {
            dwCtxId = ptMcsDynCtxNoUniqAck->adwDataArea[j];
            T_psNcuDaAppCtx* ptNcuDaAppCtx = psMcsGetDaAppCtxById(dwCtxId, dwhDB);
            if (NULL == ptNcuDaAppCtx)
            {
                DEBUG_TRACE(DEBUG_LOW, "psNcuUserLocationChangeProc get appctx Que is good ,appid=%u, dwCtxId=%u\n", dwAppidList[dwIndex], dwCtxId);
                continue;
            }
            if (!ptNcuDaAppCtx->bHasPoor)
            {
                DEBUG_TRACE(DEBUG_LOW, "psNcuUserLocationChangeProc get appctx Que is not haspoor ,appid=%u, dwCtxId=%u\n", dwAppidList[dwIndex], dwCtxId);
                continue;
            }
            MCS_LOC_STAT_EX(ptMcsNcuPerform, qwUsrLocationCfgRptHasPoor, 1);
            // 上报接口，只上报位置上报
            psNcuReportToUpmProc(ptNcuDaAppCtx, NULL, ptMediaProcThreadPara->bThreadNo, RPT_TYPE_LOCATION_CHG);

        }
    }

}


WORD32 psNcuUpdSessionProc(T_MediaProcThreadPara *ptMediaProcThreadPara,T_NcuSynSessInfo* ptSessionInfo)
{
    if(NULL == ptMediaProcThreadPara || NULL == ptSessionInfo || NULL == ptMediaProcThreadPara->ptMcsStatPointer)
    {
        return MCS_RET_FAIL;
    }
    T_psNcuMcsPerform *ptMcsNcuPerform = (T_psNcuMcsPerform*)ptMediaProcThreadPara->ptMcsStatPointer;
    WORD64 ddwUPSeid = ptMediaProcThreadPara->ddwUPseid;
    WORD32 dwhDB     = ptMediaProcThreadPara->dwhDBByThreadNo;
    BYTE   bIsUliChg = FALSE;

    T_psNcuSessionCtx* ptSession = psQuerySessionByUpseid(ddwUPSeid, dwhDB);
    if(unlikely(NULL == ptSession))
    {
        DEBUG_TRACE(DEBUG_LOW,"GetSession fail By UPseid=%llu, hDB=%u\n", ddwUPSeid, dwhDB);
        return MCS_RET_FAIL;
    }
    if(1 == ptSession->bHasUli && 0 != memcmp(&ptSession->tUserLocation, &ptSessionInfo->tUli,sizeof(ptSessionInfo->tUli)))
    {
        MCS_LOC_STAT_EX(ptMcsNcuPerform, qwRcvModSessionUpdUli, 1);
        bIsUliChg = TRUE;
    }
    psNcuInitSession(ptSession, ptSessionInfo, ptMcsNcuPerform);
    if(1 == ptSession->bHasUli && TRUE == bIsUliChg)
    {
        DEBUG_TRACE(DEBUG_LOW,"MODSession has locationChanged !\n");
        psNcuUserLocationChangeProc(ptSession, ptMediaProcThreadPara);
    }
    return MCS_RET_SUCCESS;
}

void psNcuChgForSessionPm(BYTE bOldSubType, BYTE bNewSubType)
{
    if((0 == (bOldSubType & enumQosAna)) && (bNewSubType & enumQosAna)) //之前不含ana现在包含ana
    {
         NCU_PM_55302_STAT_ADD(qwQualityAssuranceSessions,1);
    }
    if((0 == (bNewSubType & enumQosAna)) && (bOldSubType & enumQosAna)) //之前含ana现在不含ana
    {
         NCU_PM_55302_STAT_REMOVE(qwQualityAssuranceSessions,1);
    }
    if((0 == (bOldSubType & enumQosExpSpecial)) && (bNewSubType & enumQosExpSpecial)) //之前不含expspecial现在包含expspecial
    {
         NCU_PM_55302_STAT_ADD(qwKeyUserExperienceDataSessions,1);
    }
    if((0 == (bNewSubType & enumQosExpSpecial)) && (bOldSubType & enumQosExpSpecial)) //之前含expspecial现在不含expspecial
    {
         NCU_PM_55302_STAT_REMOVE(qwKeyUserExperienceDataSessions,1);
    }
    if((0 == (bOldSubType & enumQosExpNormal)) && (bNewSubType & enumQosExpNormal)) //之前不含expnomal现在包含expnomal
    {
         NCU_PM_55302_STAT_ADD(qwRegularUserExperienceDataSessions,1);
    }
    if((0 == (bNewSubType & enumQosExpNormal)) && (bOldSubType & enumQosExpNormal)) //之前含expnomal现在不含expnomal
    {
         NCU_PM_55302_STAT_REMOVE(qwRegularUserExperienceDataSessions,1);
    }
    return;

}


WORD32 psDelSessionCtxAndQue(T_psNcuMcsPerform *ptNcuPerform, T_psNcuSessionCtx* ptSession, WORD32 dwhDBByThreadNo)
{
    MCS_PCLINT_NULLPTR_RET_2ARG(ptNcuPerform, ptSession, MCS_RET_FAIL); 
    
    WORD32 dwLduGroupId = GET_LDU_USERGROUP_FROM_UPSEID(ptSession->ddwUPSeid);
    BOOLEAN boolRet = psGroupQueDelete(dwhDBByThreadNo, ptSession->dwSessionCtxId, dwLduGroupId);
    if(TRUE != boolRet)
    {
        MCS_LOC_STAT_EX(ptNcuPerform, qwDelGroupQueFail, 1);
    }
    WORD32 ret = psVpfuMcsDelCtxById(ptSession->dwSessionCtxId, dwhDBByThreadNo, DB_HANDLE_R_NCUSESSION);

    return ret;
}

/* Started by AICoder, pid:ta7e3v178072f02141d008b3002e1e4124f5ea03 */
VOID psNcuDelSessionByGroupNo(T_MediaProcThreadPara *ptMediaProcThreadPara, T_psMcsSessDelByGroupArea *ptMsgBody)
{
    MCS_PCLINT_NULLPTR_RET_2ARG_VOID(ptMediaProcThreadPara, ptMsgBody);
    T_psNcuMcsPerform *ptNcuPerform = (T_psNcuMcsPerform*)ptMediaProcThreadPara->ptMcsStatPointer;
    MCS_PCLINT_NULLPTR_RET_VOID(ptNcuPerform); 
    
    WORD32 dwhDBByThreadNo  = ptMediaProcThreadPara->dwhDBByThreadNo; 
    WORD32 dwGtpuNo    = ptMsgBody->dwGroupId; 
    MCS_CHK_VOID_STAT(ncuIsSelfGroup(dwGtpuNo), qwDelSessionByGroupFailIsSelfGroup);
    LPT_PfuQueHeadReg pQueHead = _db_getGroupQueHeadAdrr(dwhDBByThreadNo, dwGtpuNo);
    MCS_PCLINT_NULLPTR_RET_VOID(pQueHead);

    WORD32 dwLoop = 0;
    WORD32 queLength = _xdb_pfu_QueGetLength(pQueHead);
    for(; dwLoop < UPF_MAX_SESSDEL_NUM && dwLoop < queLength; dwLoop++)
    {
        psNcuDelSessionByGroupNoSub(ptMediaProcThreadPara, pQueHead, dwGtpuNo);
    }
}

VOID psNcuDelSessionByGroupNoSub(T_MediaProcThreadPara *ptMediaProcThreadPara, LPT_PfuQueHeadReg pQueHead, WORD32 dwGtpuNo)
{
    MCS_PCLINT_NULLPTR_RET_2ARG_VOID(ptMediaProcThreadPara, pQueHead); 
    T_psNcuMcsPerform *ptNcuPerform = (T_psNcuMcsPerform*)ptMediaProcThreadPara->ptMcsStatPointer;

    WORD32 dwTupleNo = _xdb_pfu_QueGetHead(pQueHead);
    T_psNcuSessionCtx *ptNcuSessionCtx = psVpfuMcsGetCtxById(dwTupleNo, ptMediaProcThreadPara->dwhDBByThreadNo, DB_HANDLE_R_NCUSESSION);
    
    if(NULL == ptNcuSessionCtx)
    {
        psGroupQueDelete(ptMediaProcThreadPara->dwhDBByThreadNo, dwTupleNo, dwGtpuNo);
        MCS_LOC_STAT_EX(ptNcuPerform, qwDelSessionByGroupGetSessCtxFail, 1);
        return;
    }
    
    ptMediaProcThreadPara->ddwUPseid = ptNcuSessionCtx->ddwUPSeid;
    WORD32 dwReturnValue = psNcuDelSessionProc(ptMediaProcThreadPara);

    if(MCS_RET_SUCCESS != dwReturnValue)
    {
        MCS_LOC_STAT_EX(ptNcuPerform, qwDelSessionByGroupNumFail, 1);
    }
    else
    {
        MCS_LOC_STAT_EX(ptNcuPerform, qwDelSessionByGroupNumSucc, 1);
    }
}
/* Ended by AICoder, pid:ta7e3v178072f02141d008b3002e1e4124f5ea03 */
void psNcu55303StatProc(T_MediaProcThreadPara *ptMediaProcThreadPara, T_NcuSynSubInfoHead* ptSubscribeHead)
{
    MCS_PCLINT_NULLPTR_RET_3ARG_VOID(ptMediaProcThreadPara,ptMediaProcThreadPara->ptMcsStatPointer,ptSubscribeHead);
    T_psNcuMcsPerform *ptNcuPerform = (T_psNcuMcsPerform*)ptMediaProcThreadPara->ptMcsStatPointer;
    switch(ptSubscribeHead->bSubType)
    {
        case PfuQosAna:
            ptMediaProcThreadPara->bSubType = enumQosAna;
            NCU_PM_55303_STAT_ADD(qwNcuRecvQoSSubscribeMsgFromN4Num, 1);
            MCS_CHK_55303STAT(!ptSubscribeHead->bSubUpdateFlag, qwNcuRecvQoSNewSubscribeMsgFromN4Num);
            MCS_CHK_55303STAT(ptSubscribeHead->bSubUpdateFlag, qwNcuRecvQoSModifySubscribeMsgFromN4Num);
            break;
        case PfuExpSpecial:
            ptMediaProcThreadPara->bSubType = enumQosExpSpecial;
            NCU_PM_55303_STAT_ADD(qwNcuRecvExpDataSubscribeMsgFromN4Num, 1);
            MCS_CHK_55303STAT(!ptSubscribeHead->bSubUpdateFlag, qwNcuRecvExpDataNewSubscribeMsgFromN4Num);
            MCS_CHK_55303STAT(ptSubscribeHead->bSubUpdateFlag, qwNcuRecvExpDataModifySubscribeMsgFromN4Num);
            break;
        case PfuExpNormal:
            ptMediaProcThreadPara->bSubType = enumQosExpNormal;
            NCU_PM_55303_STAT_ADD(qwNcuRecvExpDataNewSubscribeMsgFromSBI, 1);
            NCU_PM_55303_STAT_ADD(qwNcuRecvExpDataSubscribeMsgFromSBI, 1);
            break;
        default:
            MCS_LOC_STAT_EX(ptNcuPerform, qwAddSubscribeUnKownType, 1);
            return;
    }
    MCS_CHK_55303STAT(!ptSubscribeHead->bSubUpdateFlag, qwNcuRecvSubscribeNewMsgNum);
    MCS_CHK_55303STAT(ptSubscribeHead->bSubUpdateFlag, qwNcuRecvSubscribeModifyMsgNum);
    NCU_PM_55303_STAT_ADD(qwNcuRecvSubscribeMsgNum, 1);
    MCS_CHK_STAT(!ptSubscribeHead->bSubUpdateFlag, qwRcvAddSubscribe);
    MCS_CHK_STAT(ptSubscribeHead->bSubUpdateFlag, qwRcvUpdateSubscribe);
    return;
}