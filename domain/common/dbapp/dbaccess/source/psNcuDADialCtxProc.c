/* Started by AICoder, pid:0e2cfsaa54lf2e714bdf0b41b0dfb375586879e2 */
/******************************************************************************
 * 版权所有 (C)2016 深圳市中兴通讯股份有限公司*
 * 模块名          : MCS
 * 文件名          : psNcuDADialCtxProc.c
 * 相关文件        :
 * 文件实现功能     : DADIAL上下文增删改查接口
 * 归属团队        : M6
 * 版本           : V7.24.40
-------------------------------------------------------------------------------
* 修改记录:
* 修改日期            版本号           修改人           修改内容
* 2024-10-15    V7.24.40              zjw          create
******************************************************************************/
/**************************************************************************
 *                              头文件(满足最小依赖请按照DDD分层架构逐层依赖)
 **************************************************************************/
#include "psNcuDADialCtxProc.h"
#define USE_DM_UPFNCU_GETALLR_DADIALCFG
#include "dbm_lib_upfncu.h"
#include "r_ncu_dadial.h"
#include "r_ncu_daprofile.h"
#include "ps_ncu_typedef.h"
#include "xdb_pfu_dyntbl_acc.h"
#include "xdb_core_pfu.h"
#include "xdb_pfu_com.h"
#include "xdb_core_pfu_db.h"
#include "ps_db_define_ncu.h"
#include "psNcuApplicationMapCtxProc.h"
#include "psMcsDebug.h"
#include "psMcsTestSend.h"
#include "ps_ncu_session.h"
#include "psUpfEvent.h"
#include "psUpfPubSub.h"
#include "psNcuReportTimer.h"
#include "psNcuReportSBCProc.h"

/**************************************************************************
 *                              宏(本源文件使用)
 **************************************************************************/

/**************************************************************************
 *                              常量(本源文件使用)
 **************************************************************************/

/**************************************************************************
 *                              数据类型(本源文件使用)
 **************************************************************************/

/**************************************************************************
 *                              外部函数原型(评估后慎重添加)
 **************************************************************************/

/**************************************************************************
 *                              局部函数原型(本源文件使用)
 **************************************************************************/
VOID psNcuAllocDaDialCtxByImsiSubApp(T_IMSI *tImsi, CHAR *subapp, BYTE bQosQuality);
T_psNcuDADial* psCreateDADialCtxByImsiSubAppid(T_IMSI *tIMSI, WORD32 dwSubAppid);
VOID psNcuDaAppCtxDaDialCtxRelateRpt(T_psNcuDaAppCtx *ptDataAppCtx, BYTE bNewQoe, WORD16 wThreadNo);

/**************************************************************************
 *                              全局变量(本源文件使用)
 **************************************************************************/

/**************************************************************************
 *                              函数实现(函数布局:总分结构|深度优先)
 **************************************************************************/
VOID psNcuGetDADialCtxAllCfg()
{
    DM_UPFNCU_GETALLR_DADIALCFG_REQ req = {0};
    DM_UPFNCU_GETALLR_DADIALCFG_ACK ack = {0};
    BOOLEAN bflag = FALSE;
    WORD16 i = 0;
    WORD32 dwLoop = ctR_ncu_dadial_CAPACITY;

    do
    {
        req.msgType = MSG_CALL;
        req.dwTupleNo = ack.dwTupleNo;
        ack.retCODE = RC_DBM_ERROR;
        bflag = DBM_CALL(DM_UPFNCU_GETALLR_DADIALCFG, (LPSTR)&req, (LPSTR)&ack);
        MCS_CHK_VOID(!bflag);
        MCS_CHK_VOID(RC_DBM_OK != ack.retCODE);

        BOOL result = TRUE;
        for(i = 0; (i < ctR_ncu_dadial_CAPACITY) && (i < ack.dwValidNum); i++)
        {
            dwLoop--;
            T_IMSI  tIMSI = {0};
            result = psUpfCommString2tBCD(tIMSI,(BYTE *)ack.imsi[i],(BYTE)zte_strnlen_s(ack.imsi[i], MAX_CHAR_IMSI_LEN));
            if(FALSE == result)
            {
                DEBUG_TRACE(DEBUG_LOW,"\n[psNcuGetDADialCtxAllCfg] IMSI Convert String to BCD Fail!\n");
                return;
            }
            psNcuAllocDaDialCtxByImsiSubApp(&tIMSI, ack.subapp[i], ack.qosquality[i]);
        }
    } while (!ack.blEnd && dwLoop);

    return;
}
/* Ended by AICoder, pid:0e2cfsaa54lf2e714bdf0b41b0dfb375586879e2 */

VOID psNcuAllocDaDialCtxByImsiSubApp(T_IMSI *tImsi, CHAR *subapp, BYTE bQosQuality)
{
    if (NULL == tImsi || NULL == subapp)
    {
        return ;
    }

    WORD32 dwSubAppid = 0;
    if (FALSE == psNcuGetCfgInnerAppIdByStrAppidmap(subapp, &dwSubAppid))
    {
        DEBUG_TRACE(DEBUG_LOW,"\n[Mcs]psNcuAllocDaDialCtxByImsiSubApp: query strSubApp = %s,dwSubAppid = %u fail!\n",subapp,dwSubAppid);
        return;
    }
    T_psNcuDADial *ptNcuDADialCtx = psCreateDADialCtxByImsiSubAppid(tImsi, dwSubAppid);
    if (NULL == ptNcuDADialCtx)
    {
        DEBUG_TRACE(DEBUG_LOW,"\n[Mcs]psNcuAllocDaDialCtxByImsiSubApp: CreateDADialCtx fail!\n");
        return ;
    }
    ptNcuDADialCtx->bQosQuality = bQosQuality;

    /* 关联DaAppCtx */
    psNcuDaDialCtxRelateDaAppCtx(ptNcuDADialCtx, ADD_DADIALCTX);
    return;
}

/* Started by AICoder, pid:v5252xa79794099146e7097520d275882f17b793 */ 
T_psNcuDADial* psQueryDADialCtxByImsiSubAppid(T_IMSI *tIMSI, WORD32 dwSubAppid)
{
    _DB_STATEMENT_TRUE_RTN_VALUE((NULL == tIMSI), NULL);

    DM_PFU_QUERYDYNDATA_BYIDX_REQ tQueryReq = {0}; /* 查询请求 */
    DM_PFU_QUERYDYNDATA_BYIDX_ACK tQueryAck = {0}; /* 查询应答 */

    r_ncu_dadial_idx_tuple *ptIndex = NULL;

    tQueryReq.hDB         = _NCU_DBHANDLE_COMM; /* 库句柄待定 */
    tQueryReq.hTbl        = DB_HANDLE_R_NCU_DADIAL;
    tQueryReq.dwExpectNum = 1;
    tQueryReq.dataAreaKey.ucKeyFlag = XDB_KEYFLAG_UNIQUE;
    tQueryReq.dataAreaKey.hIdx      = DB_HANDLE_IDX_R_NCU_DADIAL;
    tQueryReq.dataAreaKey.ucKeyLen  = sizeof(r_ncu_dadial_idx_tuple);
    ptIndex = (r_ncu_dadial_idx_tuple *)tQueryReq.dataAreaKey.aucKey;
    ptIndex->dwSubAppid = dwSubAppid;
    zte_memcpy_s(&(ptIndex->tIMSI), sizeof(T_IMSI), tIMSI, sizeof(T_IMSI));

    _pDM_PFU_QUERYDYNDATA_BYIDX(&tQueryReq, &tQueryAck);

    if ((RC_OK == tQueryAck.retCODE) && (NULL != tQueryAck.pDataAreaAddr))
    {
        return (T_psNcuDADial *)tQueryAck.pDataAreaAddr;
    }
    return NULL;
}

T_psNcuDADial* psCreateDADialCtxByImsiSubAppid(T_IMSI *tIMSI, WORD32 dwSubAppid)
{
    _DB_STATEMENT_TRUE_RTN_VALUE((NULL == tIMSI), NULL);

    DM_PFU_ALLOCDYNDATA_BYIDX_REQ tAllocReq = {0};
    DM_PFU_ALLOCDYNDATA_BYIDX_ACK tAllocAck = {0};

    r_ncu_dadial_idx_tuple *ptIndex = NULL;

    tAllocReq.hDB  = _NCU_DBHANDLE_COMM; /* 库句柄待定 */
    tAllocReq.hTbl = DB_HANDLE_R_NCU_DADIAL;
    tAllocReq.dataAreaKey.ucKeyFlag = XDB_KEYFLAG_UNIQUE;
    tAllocReq.dataAreaKey.hIdx      = DB_HANDLE_IDX_R_NCU_DADIAL;
    tAllocReq.dataAreaKey.ucKeyLen  = sizeof(r_ncu_dadial_idx_tuple);
    ptIndex = (r_ncu_dadial_idx_tuple *)tAllocReq.dataAreaKey.aucKey;
    ptIndex->dwSubAppid = dwSubAppid;
    zte_memcpy_s(&(ptIndex->tIMSI), sizeof(T_IMSI), tIMSI, sizeof(T_IMSI));

    _pDM_PFU_ALLOCDYNDATA_BYIDX(&tAllocReq, &tAllocAck);
    if ((RC_OK == tAllocAck.retCODE || RC_EXIST == tAllocAck.retCODE) && (NULL != tAllocAck.ptDataAreaAddr))
    {
        T_psNcuDADial *ptNcuDADialCtx = (T_psNcuDADial *)tAllocAck.ptDataAreaAddr;
        ptNcuDADialCtx->dwCtxId = tAllocAck.dwDataAreaIndex;
        zte_memcpy_s(&(ptNcuDADialCtx->tIMSI), sizeof(T_IMSI), tIMSI, sizeof(T_IMSI));
        ptNcuDADialCtx->dwSubAppid  = dwSubAppid;
        return ptNcuDADialCtx;
    }
    return NULL;
}

WORD32 psDelDADialCtxByImsiSubAppid(T_IMSI *tIMSI, WORD32 dwSubAppid)
{
    _DB_STATEMENT_TRUE_RTN_VALUE((NULL == tIMSI), MCS_RET_FAIL);

    DM_PFU_RELEASEDYNDATA_BYIDX_REQ tReleaseReq = {0};
    DM_PFU_RELEASEDYNDATA_BYIDX_ACK tReleaseAck = {0};

    r_ncu_dadial_idx_tuple *ptIndex = NULL;

    tReleaseReq.hDB         = _NCU_DBHANDLE_COMM;
    tReleaseReq.hTbl        = DB_HANDLE_R_NCU_DADIAL;
    tReleaseReq.dataAreaKey.ucKeyFlag = XDB_KEYFLAG_UNIQUE;
    tReleaseReq.dataAreaKey.hIdx      = DB_HANDLE_IDX_R_NCU_DADIAL;
    tReleaseReq.dataAreaKey.ucKeyLen  = sizeof(r_ncu_dadial_idx_tuple);
    ptIndex = (r_ncu_dadial_idx_tuple *)tReleaseReq.dataAreaKey.aucKey;
    ptIndex->dwSubAppid = dwSubAppid;
    zte_memcpy_s(&(ptIndex->tIMSI), sizeof(T_IMSI), tIMSI, sizeof(T_IMSI));

    tReleaseAck.retCODE = RC_ERROR;

    _pDM_PFU_RELEASEDYNDATA_BYIDX(&tReleaseReq, &tReleaseAck);
    if (RC_OK == tReleaseAck.retCODE)
    {
        return MCS_RET_SUCCESS;
    }

    return MCS_RET_FAIL;
}
/* Ended by AICoder, pid:v5252xa79794099146e7097520d275882f17b793 */ 

VOID psNcuDaDialCtxRelateDaAppCtx(T_psNcuDADial *ptNcuDADialCtx, BYTE bOperation)
{
    MCS_CHK_NULL_VOID(ptNcuDADialCtx);

    T_psNcuSessionCtx * ptVpfuSesionNode = NULL;
    T_psNcuDaAppCtx *ptDataAppCtx = NULL;
    WORD32 dwIdx    = 0;
    WORD32 ctxnum   = 0;
    WORD32 i        = 0;
    WORD32 ctxid    = 0;
    WORD32 dwhDB    = 0;
    WORD32 dwMsgId  = EV_MSG_UCOM_NCU_JOB_TO_MCS_DIALCHANGE;
    T_psNcuDialCfgChangeInfo tNcuDialCfgChangeInfo= {0};
    /* 无法获取具体的线程号，使用额外MEDIA_THRD_NUM数据区 */
    MCS_DM_QUERYDYNDATA_ACK *ack = &g_ptVpfuShMemVar->tGwGloData.atMcsDynCtxNoUniqAck[MEDIA_THRD_NUM];
    for(dwIdx = 0; dwIdx < XDB_PFU_DB_NUM; dwIdx++)
    {
        MCS_CHK_CONTINUE(NULL == g_tPfuDbSysLocal[dwIdx].pDataBaseMemAddr);
        DEBUG_TRACE(DEBUG_LOW,"\n[psNcuDaDialCtxRelateDaAppCtx] g_tPfuDbSysLocal[dwIdx].pDataBaseMemAddr is not NULL!\n");

        /* 获取session ctx */
        dwhDB = g_tPfuDbSysLocal[dwIdx].hDB;
        ctxnum = psVpfuMcsGetSessionCtxNumByIMSI(ptNcuDADialCtx->tIMSI, dwhDB, (BYTE*)ack);
        DEBUG_TRACE(DEBUG_LOW,"[psNcuDaDialCtxRelateDaAppCtx]get session ctxnum is %d, hdb: %d\n",ctxnum, dwhDB);
        MCS_CHK_CONTINUE(0 == ctxnum);

        for (i = 0; i < MIN(ctxnum, MCS_DYNCTX_NOUNIQ_EXPECTNUM); i++)
        {
            ctxid = ack->adwDataArea[i];
            ptVpfuSesionNode = (T_psNcuSessionCtx *)psMcsGetSessionCtxById(ctxid, dwhDB);
            MCS_CHK_CONTINUE(NULL == ptVpfuSesionNode);
            /* 获取DaApp ctx */
            ptDataAppCtx = psQueryDaAppCtxByUpseidSubAppid(ptVpfuSesionNode->ddwUPSeid, ptNcuDADialCtx->dwSubAppid, dwhDB);
            MCS_CHK_CONTINUE(NULL == ptDataAppCtx);
            WORD16 wThreadNo = dwIdx % MEDIA_THRD_NUM;
            /* 从job发送 Ucom消息到媒体面线程 */
            tNcuDialCfgChangeInfo.bOperation      = bOperation;
            tNcuDialCfgChangeInfo.bDialQosQuality = ptNcuDADialCtx->bQosQuality;
            tNcuDialCfgChangeInfo.dwSubAppid      = ptNcuDADialCtx->dwSubAppid;
            tNcuDialCfgChangeInfo.ddwUpseid       = ptVpfuSesionNode->ddwUPSeid;

            if(MCS_RET_SUCCESS != upfSendMsgToMcsByUcom((BYTE *)&tNcuDialCfgChangeInfo,dwMsgId, sizeof(T_psNcuDialCfgChangeInfo) , wThreadNo))
            {
                DEBUG_TRACE(DEBUG_LOW, "psNcuDaDialCtxRelateDaAppCtx: NCU SendMsgToMcsByUcom fail!\n");
                return ;
            }
            DEBUG_TRACE(DEBUG_LOW, "psNcuDaDialCtxRelateDaAppCtx: NCU SendMsgToMcsByUcom SUCC!\n");
        }
    }
    return ;
}

VOID psNcuDialCfgChangeProc(T_psNcuDialCfgChangeInfo *ptNcuDialCfgChangeInfo, WORD16 wThreadNo)
{
    MCS_CHK_NULL_VOID(ptNcuDialCfgChangeInfo);
    WORD32 dwhDB = _NCU_GET_DBHANDLE(wThreadNo);
    T_psNcuDaAppCtx *ptDataAppCtx = psQueryDaAppCtxByUpseidSubAppid(ptNcuDialCfgChangeInfo->ddwUpseid, ptNcuDialCfgChangeInfo->dwSubAppid, dwhDB);
    MCS_CHK_NULL_VOID(ptDataAppCtx);

    BYTE  bLastQoe  = 0;
    BYTE  bNewQoe   = 0;
    ptDataAppCtx->bIsDialFlg  = 1;
    if(ADD_DADIALCTX == ptNcuDialCfgChangeInfo->bOperation)
    {
        bLastQoe = ptDataAppCtx->bCurQoe;
        bNewQoe  = ptNcuDialCfgChangeInfo->bDialQosQuality;
        ptDataAppCtx->bCurDialQoe = ptNcuDialCfgChangeInfo->bDialQosQuality;
    }
    else if(UPD_DADIALCTX == ptNcuDialCfgChangeInfo->bOperation)
    {
        bLastQoe = ptDataAppCtx->bCurDialQoe;
        bNewQoe  = ptNcuDialCfgChangeInfo->bDialQosQuality;
        ptDataAppCtx->bCurDialQoe = ptNcuDialCfgChangeInfo->bDialQosQuality;
    }
    else
    {
        bLastQoe = ptDataAppCtx->bCurDialQoe;
        bNewQoe  = ptDataAppCtx->bCurQoe;
        ptDataAppCtx->bCurDialQoe = 0xFF;
        ptDataAppCtx->bIsDialFlg  = 0;
    }
    DEBUG_TRACE(DEBUG_LOW,"[psNcuDialCfgChangeProc]DialQoe is %d, CurQoe: %d\n",ptDataAppCtx->bCurDialQoe, ptDataAppCtx->bCurQoe);
    /* 质差拨测状态和分析上下文状态不一致 触发上报 */
    if(bLastQoe != bNewQoe)
    {
        psNcuDaAppCtxDaDialCtxRelateRpt(ptDataAppCtx, bNewQoe, wThreadNo);
    }
    return;
}

VOID psNcuDaAppCtxRelateDaDialCtx(T_psNcuDaAppCtx *ptDataAppCtx, BYTE bThreadNo)
{
    MCS_CHK_NULL_VOID(ptDataAppCtx);
    MCS_CHK_NULL_VOID(ptDataAppCtx->ptSessionCtx);

    T_psNcuSessionCtx* ptNcuSessionCtx = (T_psNcuSessionCtx*)ptDataAppCtx->ptSessionCtx;
    T_psNcuDADial *ptNcuDADialCtx = psQueryDADialCtxByImsiSubAppid((T_IMSI *)ptNcuSessionCtx->bImsi, ptDataAppCtx->dwSubAppid);
    if (NULL == ptNcuDADialCtx)
    {
        return ;
    }
    ptDataAppCtx->bIsDialFlg  = 1;
    ptDataAppCtx->bCurDialQoe = ptNcuDADialCtx->bQosQuality;

    /* 拨测状态与当前状态不一致 则触发上报 */
    if(ptDataAppCtx->bCurQoe != ptDataAppCtx->bCurDialQoe)
    {
        WORD16 wThreadNo = bThreadNo % MEDIA_THRD_NUM;
        psNcuDaAppCtxDaDialCtxRelateRpt(ptDataAppCtx, ptDataAppCtx->bCurDialQoe, wThreadNo);
    }
    return;
}

VOID psNcuDaAppCtxDaDialCtxRelateRpt(T_psNcuDaAppCtx *ptDataAppCtx, BYTE bNewQoe, WORD16 wThreadNo)
{
    WORD64 ddwNextClockStep = 0;
    WORD32 reporttime     = 0;
    WORD64 dwCurClockStep = getCurSetTimerClockStep();
    WORD32 dwhDB = _NCU_GET_DBHANDLE(wThreadNo);
    DEBUG_TRACE(DEBUG_LOW,"[psNcuDaAppCtxDaDialCtxRelateRpt]dwCurClockStep is %llu\n",dwCurClockStep);
    DEBUG_TRACE(DEBUG_LOW,"[psNcuDaAppCtxDaDialCtxRelateRpt]wThreadNo is %u,dwhDB is %u\n",wThreadNo, dwhDB);

    if(DA_QUALITY_POOR == bNewQoe)
    {
        DEBUG_TRACE(DEBUG_LOW,"psNcuDaAppCtxDaDialCtxRelateRpt newQoe is Poor!!\n");
        ptDataAppCtx->bHasPoor = 1;
        T_VpfuDAProfileCtx *ptDAProfileTuple = (T_VpfuDAProfileCtx *)ptDataAppCtx->ptDAProfileTuple;
        MCS_CHK_NULL_VOID(ptDAProfileTuple);
        reporttime = ptDAProfileTuple->reportpoortime;
        DEBUG_TRACE(DEBUG_LOW,"[psNcuDaAppCtxDaDialCtxRelateRpt]reportpoortime is %u\n",ptDAProfileTuple->reportpoortime);
        ptDataAppCtx->ddwAnaRepClockStep = milliSecToClockStep(reporttime*1000);
        ddwNextClockStep = dwCurClockStep + ptDataAppCtx->ddwAnaRepClockStep;
        DEBUG_TRACE(DEBUG_LOW,"[psNcuDaAppCtxDaDialCtxRelateRpt]ddwAnaRepClockStep is %llu, ddwNextClockStep is %llu\n",ptDataAppCtx->ddwAnaRepClockStep,ddwNextClockStep);
        psNcuReportToUpmProc(ptDataAppCtx, NULL, wThreadNo, RPT_TYPE_ANA);
        psNcuMcsUpdDaAppCtxByClockStep(ddwNextClockStep, DB_HANDLE_IDX_R_NCU_DATAAPP_ANA_CLOCK_STEP, ptDataAppCtx->dwID, dwhDB, enumUpdateIndex);
    }
    else 
    {
        DEBUG_TRACE(DEBUG_LOW,"psNcuDaAppCtxDaDialCtxRelateRpt newQoe is Good!!\n");
        ptDataAppCtx->bHasPoorNum = 0;
        psNcuReportToUpmProc(ptDataAppCtx, NULL, wThreadNo, RPT_TYPE_ANA);
        psNcuMcsUpdDaAppCtxByClockStep(0, DB_HANDLE_IDX_R_NCU_DATAAPP_ANA_CLOCK_STEP, ptDataAppCtx->dwID, dwhDB, enumDeleteIndex);
    }
    return;
}

VOID showDADialCtx(CHAR *imsi, CHAR *subapp)
{
    if (NULL == imsi ||NULL == subapp)
    {
        return ;
    }

    T_IMSI  tIMSI = {0};
    BOOL result = psUpfCommString2tBCD(tIMSI,(BYTE *)imsi,(BYTE)zte_strnlen_s(imsi, MAX_CHAR_IMSI_LEN));
    if(FALSE == result)
    {
        return;
    }

    WORD32 dwSubAppid = 0;
    if (FALSE == psNcuGetCfgInnerAppIdByStrAppidmap(subapp, &dwSubAppid))
    {
        return;
    }
    T_psNcuDADial *ptNcuDADialCtx = psQueryDADialCtxByImsiSubAppid(&tIMSI, dwSubAppid);
    if (NULL == ptNcuDADialCtx)
    {
        return ;
    }
    zte_printf_s("dwCtxId      = %u\n", ptNcuDADialCtx->dwCtxId);
    zte_printf_s("bQosQuality  = %u\n", ptNcuDADialCtx->bQosQuality);
    return;
}

