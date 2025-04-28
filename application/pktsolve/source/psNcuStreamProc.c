#include "psNcuStreamProc.h"
#include "ps_ncu_session.h"
#include "psNcuDataAnalysis.h"
#include "psNcuReport.h"
#include "psMcsDebug.h"
#include "psNcuCtxFunc.h"
#include "ps_db_define_ncu.h"
#include "psNcuSnCtxProc.h"
#include "ps_db_define_pfu.h"
#include "ncuSCCUAbility.h"
#include "dpathreadpub.h"
#include "UpfNcuSynInfo.h"
#include "McsHeadCap.h"
#include "ps_ncu_typedef.h"
#include "pfuUserGroup.h"
#include "psNcuPktSolveFunc.h"

T_psNcuFlowCtx* psNcuQryStreamProc(T_MediaProcThreadPara *ptMediaProcThreadPara,DB_STRM_INDEX* ptStmIdx)
{
    if(NULL == ptMediaProcThreadPara || NULL == ptStmIdx || NULL == ptMediaProcThreadPara->ptMcsStatPointer)
    {
        return NULL;
    }
     T_psNcuMcsPerform *ptMcsNcuPerform = (T_psNcuMcsPerform*)ptMediaProcThreadPara->ptMcsStatPointer;
    T_psNcuFlowCtx *ptStmAddr = (T_psNcuFlowCtx *)psNcuQueryStreamByQuintuple(ptMediaProcThreadPara->dwhDBByThreadNo, (VOID *)ptStmIdx);
    if(NULL != ptStmAddr)
    {
        
        if(ptStmAddr->ddwUPSeid != ptMediaProcThreadPara->ddwUPseid || NULL == ptStmAddr->ptNcuSesionCtx)
        {
            MCS_LOC_STAT_EX(ptMcsNcuPerform, qwQueryStreamSessionOrUPSeidErr, 1);
            DEBUG_TRACE(DEBUG_LOW,"ptStmAddr->ddwUPSeid != ptMediaProcThreadPara->ddwUPseid || NULL == ptStmAddr->ptNcuSesionCtx\n");
            //如果流上upseid和报文携带的不一致，需要删流推流量，走新建流程
            psNcuRmStmFromDataApp(ptMediaProcThreadPara, ptStmAddr, ptStmAddr->ptNcuDaCtx);
            psNcuMcsRelStmByID(ptStmAddr->dwFlowID, ptMediaProcThreadPara->dwhDBByThreadNo);
            psDelAllSnCtxByFlowId(ptStmAddr->dwFlowID, ptMediaProcThreadPara->dwhDBByThreadNo);
            return NULL;
        }
        ptStmAddr->dwUpdateTimeStamp = ptMediaProcThreadPara->dwPktPowOnSec;
        if(ptStmAddr->dwSubAppId != ptMediaProcThreadPara->dwSubAppid)
        {
            MCS_LOC_STAT_EX(ptMcsNcuPerform, qwQueryStreamSubAppidDiff, 1);
            DEBUG_TRACE(DEBUG_LOW, "ptStmAddr->dwSubAppId != ptMediaProcThreadPara->dwSubAppid\n");
            //流的业务特征发生变化，需要将其流量推送一把，然后将其从NcuDaCtx中摘除，然后查询或新建新的subAppid的NcuDaCtx上下文
            psNcuRmStmFromDataApp(ptMediaProcThreadPara, ptStmAddr, ptStmAddr->ptNcuDaCtx);
            ptStmAddr->dwSubAppId = 0;
            ptStmAddr->ptNcuDaCtx = NULL;
        }
    }
    return ptStmAddr;
}
T_psNcuFlowCtx* psNcuCrtStreamProc(T_MediaProcThreadPara *ptMediaProcThreadPara,DB_STRM_INDEX* ptStmIdx)
{
    if(NULL == ptMediaProcThreadPara || NULL == ptStmIdx)
    {
        return NULL;
    }
    T_psNcuFlowCtx* ptStmAddr = psNcuMcsCreatStmByIndex(ptStmIdx, ptMediaProcThreadPara->dwhDBByThreadNo);
    if(NULL == ptStmAddr)
    {
        return NULL;
    }
    
    ptStmAddr->dwCreateTimeStamp = psFtmGetCurStdSec()+0xBC17C200;
    ptStmAddr->dwUpdateTimeStamp = ptMediaProcThreadPara->dwPktPowOnSec;
    ptStmAddr->ddwUPSeid         = ptMediaProcThreadPara->ddwUPseid;
    ptStmAddr->dwhDBByThreadNo   = ptMediaProcThreadPara->dwhDBByThreadNo;
    ptStmAddr->dwRptTimeStamp    = psFtmGetPowerOnSec();
    T_psNcuSessionCtx* ptSessionCtx = psQuerySessionByUpseid(ptMediaProcThreadPara->ddwUPseid, ptMediaProcThreadPara->dwhDBByThreadNo);
    if(NULL == ptSessionCtx)
    {
        psNcuMcsRelStmByID(ptStmAddr->dwFlowID, ptMediaProcThreadPara->dwhDBByThreadNo);
        psDelAllSnCtxByFlowId(ptStmAddr->dwFlowID, ptMediaProcThreadPara->dwhDBByThreadNo);
        return NULL;
    }
    ptStmAddr->ptNcuSesionCtx = (VOID*)ptSessionCtx;
    ptStmAddr->dwSessionID   = ptSessionCtx->dwSessionID;
    ptStmAddr->dwSubAppId = 0;
    ptStmAddr->ptNcuDaCtx = NULL;
    return ptStmAddr;
}
/* Started by AICoder, pid:d00124db40iacd614154080c9067823c98a2c7c7 */
T_psNcuFlowCtx* psNcuQryOrCrtStreamProc(T_MediaProcThreadPara *ptMediaProcThreadPara,DB_STRM_INDEX* ptStmIdx)
{
    MCS_CHECK_NULLPTR_RET_3ARG(ptMediaProcThreadPara, ptStmIdx, ptMediaProcThreadPara->ptMcsStatPointer, NULL);
    T_psNcuMcsPerform *ptMcsNcuPerform = (T_psNcuMcsPerform*)ptMediaProcThreadPara->ptMcsStatPointer;
    T_psNcuFlowCtx *ptStmAddr = psNcuQryStreamProc(ptMediaProcThreadPara, ptStmIdx);
    if(NULL == ptStmAddr)
    {
        MCS_CHK_RET(MCS_RET_FAIL == psPktCheckNcuCtxIsNull(ptMediaProcThreadPara), NULL);

        ptStmAddr = psNcuCrtStreamProc(ptMediaProcThreadPara, ptStmIdx);
        if(NULL == ptStmAddr)
        {
            DEBUG_TRACE(DEBUG_LOW,"stream crt fail\n");
            MCS_LOC_STAT_EX(ptMcsNcuPerform, qwCrtStreamFail, 1);
            return NULL;
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

    return ptStmAddr;
}
/* Ended by AICoder, pid:d00124db40iacd614154080c9067823c98a2c7c7 */
WORD32 psDelStmProc(T_MediaProcThreadPara* ptMediaProcThreadPara,T_psNcuFlowCtx *ptStmAddr)
{
    if(NULL == ptMediaProcThreadPara || NULL == ptStmAddr || NULL == ptMediaProcThreadPara->ptMcsStatPointer)
    {
        return MCS_RET_FAIL;
    }
    T_psNcuMcsPerform *ptMcsNcuPerform = (T_psNcuMcsPerform*)ptMediaProcThreadPara->ptMcsStatPointer;
    if(NULL == ptStmAddr->ptNcuDaCtx)
    {
        MCS_LOC_STAT_EX(ptMcsNcuPerform, qwDelStmWithDaCtxNULL, 1);
        DEBUG_TRACE(DEBUG_LOW," ptStmAddr->ptNcuDaCtx=NULL\n");
        psNcuMcsRelStmByID(ptStmAddr->dwFlowID, ptMediaProcThreadPara->dwhDBByThreadNo);
        psDelAllSnCtxByFlowId(ptStmAddr->dwFlowID, ptMediaProcThreadPara->dwhDBByThreadNo);
        return MCS_RET_FAIL;
    }
    T_psNcuDaAppCtx *ptDataAppCtx = (T_psNcuDaAppCtx*)ptStmAddr->ptNcuDaCtx;
    BYTE bSubType = getDaTypeFromDataAppCtx(ptDataAppCtx);
    DEBUG_TRACE(DEBUG_LOW,"bDelStm for psNcuReportToNwdafProc,ptDataAppCtx=%p, ptSubScribeCtx=%p, bSubType=%u\n",ptDataAppCtx,ptDataAppCtx->ptSubScribeCtx, bSubType);
    psNcuReportBySubType(ptMediaProcThreadPara->bThreadNo, ptDataAppCtx, ptStmAddr, bSubType, RPT_TYPE_STREAM_DEL);
    
    psNcuRmStmFromDataApp(ptMediaProcThreadPara, ptStmAddr, ptDataAppCtx);
    psNcuMcsRelStmByID(ptStmAddr->dwFlowID, ptMediaProcThreadPara->dwhDBByThreadNo);
    psDelAllSnCtxByFlowId(ptStmAddr->dwFlowID, ptMediaProcThreadPara->dwhDBByThreadNo);
    return MCS_RET_SUCCESS;
}

WORD32 psNcuToPfuSynSessionCtxReq(T_MediaProcThreadPara* ptMediaProcThreadPara)
{
    MCS_CHECK_NULLPTR_RET_1ARG(ptMediaProcThreadPara, MCS_RET_FAIL);
    T_psNcuMcsPerform *ptMcsNcuPerform = (T_psNcuMcsPerform*)ptMediaProcThreadPara->ptMcsStatPointer;
    MCS_CHECK_NULLPTR_RET_1ARG(ptMcsNcuPerform, MCS_RET_FAIL);

    BYTE ncuThreadNo = ptMediaProcThreadPara->bThreadNo;
    WORD64 ddwUPSeid = ptMediaProcThreadPara->ddwUPseid;
    
    WORD32 dwGroupId   = GET_USERGROUP_FROM_UPSEID(ddwUPSeid);
    WORD32 threadNoPfu    = GET_THREAD_FROM_UPSEID(ddwUPSeid);
    WORD32 dwPfuCommId = ncuGetPfuCommidByGroup(dwGroupId);
    if(INVALID_NCU_COMMID == dwPfuCommId)
    {
        MCS_LOC_STAT_EX(ptMcsNcuPerform, qwPfuGetGroupCommIdFail, 1);
        return MCS_RET_FAIL;
    }

    BYTE  *aucPacketBuf     = NULL; /* 当前mbuf实际提供的内存 */
    WORD16 wBufferLen       = 0;    /* 当前mbuf实际提供的长度 */
    WORD16 wBufferOffset    = 0;    /* 当前mbuf偏移 */

    PS_PACKET *ptPktNew = psCssDesBufAlloc(ncuThreadNo);
    if(NULL == ptPktNew)
    {
        MCS_LOC_STAT_EX(ptMcsNcuPerform, qwAllocNcuToPfuSynPktFail, 1);
        return MCS_RET_FAIL;
    }
    if(NULL == ptPktNew->pBufferNode)
    {
        psCSSPktLinkDesFree(ptPktNew);
        MCS_LOC_STAT_EX(ptMcsNcuPerform, qwAllocNcuToPfuSynPktBufferFail, 1);
        return MCS_RET_FAIL;
    }
    PACKET_PROCESS_START(ptPktNew, PKTCHK_MCS_PROC);
    
    PS_GET_BUF_INFO(ptPktNew->pBufferNode, aucPacketBuf, wBufferOffset, wBufferLen);

    T_psNcuToPfuInfo *ptNcuToPfuSynSessionCtx  = NULL;
    ptNcuToPfuSynSessionCtx = (T_psNcuToPfuInfo *)(aucPacketBuf + wBufferOffset);
    ptNcuToPfuSynSessionCtx->bType = NCU_TO_PFU_SYN_SESSION_CTX_REQ;
    ptNcuToPfuSynSessionCtx->ddwUPSeid = ddwUPSeid;

    wBufferLen += sizeof(T_psNcuToPfuInfo);
    PS_SET_BUF_INFO(ptPktNew, ptPktNew->pBufferNode, wBufferOffset, wBufferLen);

    RawInnerParam tRawInnerParam       = {0};
    tRawInnerParam.dwFlag              = RAWFLAG_TYPE_INNERMEDIA;
    tRawInnerParam.wAttrib             = CSS_FWD_NCU_TO_PFU_INFO; 
    tRawInnerParam.dwDispIndex        = threadNoPfu;
    ptPktNew->bLBKey        = ptPktNew->bSthr;
    
    PACKET_PROCESS_END(ptPktNew, PKTCHK_MCS_PROC);
    /* Raw通道互转到对应PFU */
    if(MCS_RET_SUCCESS != dpaFastSendToNFF(&tRawInnerParam, ptPktNew, dwPfuCommId, (WORD32)ptPktNew->bSthr))
    {
        DEBUG_TRACE(DEBUG_LOW, "sendSynReq->dpaSendToNFF fail qwNcuToPfuSynSessionCtxReq !!!\n");
        MCS_LOC_STAT_EX(ptMcsNcuPerform, qwRawSendNcuToPfuSynFail, 1);
        return MCS_RET_FAIL;
    }
    DEBUG_TRACE(DEBUG_LOW, "sendSynReq->dpaSendToNFF sucess qwNcuToPfuSynSessionCtxReq !!!\n");
    MCS_LOC_STAT_EX(ptMcsNcuPerform, qwRawSendNcuToPfuSynSucess, 1);
    NCU_PM_55303_STAT_ADD(qwContextPullRequestCount, 1);
    return MCS_RET_SUCCESS;
}

VOID psNcuToPfuSynSessionCtxProc(T_MediaProcThreadPara* ptMediaProcThreadPara)
{
    MCS_CHECK_NULLPTR_RET_VOID(ptMediaProcThreadPara);
    T_psNcuMcsPerform *ptMcsNcuPerform = (T_psNcuMcsPerform*)ptMediaProcThreadPara->ptMcsStatPointer;
    MCS_CHECK_NULLPTR_RET_VOID(ptMcsNcuPerform);
    
    WORD32 ret = psNcuToPfuSynSessionCtxReq(ptMediaProcThreadPara);
    if(MCS_RET_FAIL == ret)
    {
        return;
    }
    
    T_psNcuSessionCtx* ptSession = psCreateSessionAndQueByUpseid(ptMcsNcuPerform, ptMediaProcThreadPara->ddwUPseid, ptMediaProcThreadPara->dwhDBByThreadNo);
    if(unlikely(NULL == ptSession))
    {
        MCS_LOC_STAT_EX(ptMcsNcuPerform, qwCrtSessionNcuToPfuSynFail, 1);
        DEBUG_TRACE(DEBUG_LOW, "Create T_psNcuSessionCtx NcuToPfuSyn Error!\n");
    }
    else
    {
        NCU_PM_55302_STAT_ADD(qwCurrentSessions, 1);
        MCS_LOC_STAT_EX(ptMcsNcuPerform, qwCrtSessionNcuToPfuSynSucc, 1);
        ptSession->bNcuToPfuSynSessionctx = 1;
        ptSession->dwCreateTimeStamp = psFtmGetCurStdSec()+0xBC17C200;
        ptSession->dwUpdateTimeStamp = psFtmGetPowerOnSec();
    }
    return;
}

T_psNcuSessionCtx*  psCreateSessionAndQueByUpseid(T_psNcuMcsPerform *ptNcuPerform, WORD64 ddwUPSeid, WORD32 dwhDBByThreadNo)
{
    MCS_PCLINT_NULLPTR_RET_1ARG(ptNcuPerform, NULL);
    T_psNcuSessionCtx* ptSession = psCreateSessionByUpseid(ddwUPSeid, dwhDBByThreadNo);
    MCS_PCLINT_NULLPTR_RET_1ARG(ptSession, NULL);
    
    WORD32 dwLduGroupId = GET_LDU_USERGROUP_FROM_UPSEID(ddwUPSeid);
    BOOLEAN boolRet = psGroupQueInsert(dwhDBByThreadNo, ptSession->dwSessionCtxId, dwLduGroupId, DB_HANDLE_QUE_GRP_R_PFU_SESSION_GROUP);
    if(TRUE != boolRet)
    {
        MCS_LOC_STAT_EX(ptNcuPerform, qwInsertGroupQueFail, 1);
    }

    return ptSession;
}


