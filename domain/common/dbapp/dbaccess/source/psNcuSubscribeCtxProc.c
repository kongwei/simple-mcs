/* Started by AICoder, pid:a61c03af54jeda8145620ae950bb369ce6e16951 */
/******************************************************************************
 * 版权所有 (C)2016 深圳市中兴通讯股份有限公司*
 * 模块名          : MCS
 * 文件名          : psNcuSubscribeCtxProc.c
 * 相关文件        :
 * 文件实现功能     : SubscribeCtx增删改查接口
 * 归属团队        : MD6
 * 版本           : V7.24.30
-------------------------------------------------------------------------------
* 修改记录:
* 修改日期            版本号           修改人           修改内容
  * 2024-09-20    V7.24.30              wya          create
******************************************************************************/
/**************************************************************************
 *                              头文件(满足最小依赖请按照DDD分层架构逐层依赖)
 **************************************************************************/
#include "psNcuSubscribeCtxProc.h"
#include "psNcuApplicationMapCtxProc.h"
#include "psMcsDebug.h"
#include "ps_db_define_ncu.h"
#include "xdb_pfu_dyntbl_acc.h"
#include "xdb_pfu_com.h"
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

/**************************************************************************
 *                              全局变量(本源文件使用)
 **************************************************************************/

/**************************************************************************
 *                              函数实现(函数布局:总分结构|深度优先)
 **************************************************************************/
T_psNcuDaSubScribeCtx *psMcsGetsubscribeCtxById(WORD32 dwCtxId, WORD32 hDB)
{
    DM_PFU_QUERYDYNDATA_BYTUPLENO_REQ tQueryReq = {0}; /* 查询请求 */
    DM_PFU_QUERYDYNDATA_BYTUPLENO_ACK tQueryAck = {0}; /* 查询应答 */
    _DB_STATEMENT_TRUE_RTN_VALUE((0 == dwCtxId), NULL);
    tQueryReq.hDB               = hDB; /* 库句柄待定 */
    tQueryReq.hTbl              = DB_HANDLE_R_NCUSUBCRIBE;
    tQueryReq.dwDataAreaIndex   = dwCtxId;
    _pDM_PFU_QUERYDYNDATA_BYTUPLENO(&tQueryReq, &tQueryAck);
    _DB_STATEMENT_TRUE_RTN_VALUE((RC_OK != tQueryAck.retCODE), NULL);
    return (T_psNcuDaSubScribeCtx*)tQueryAck.ptDataAreaAddr;/*返回数据区指针 */
}

T_psNcuDaSubScribeCtx*  psQuerySubscribeByUpseidAppid(WORD64 ddwUPSeid, WORD32 dwAppid, WORD32 hDB)
{
    DM_PFU_QUERYDYNDATA_BYIDX_REQ tQueryReq = {0}; /* 查询请求 */
    DM_PFU_QUERYDYNDATA_BYIDX_ACK tQueryAck = {0}; /* 查询应答 */

    tQueryReq.hDB         = hDB; /* 库句柄待定 */
    tQueryReq.hTbl        = DB_HANDLE_R_NCUSUBCRIBE;
    tQueryReq.dwExpectNum = 1;
    tQueryReq.dataAreaKey.ucKeyFlag = XDB_KEYFLAG_UNIQUE;
    tQueryReq.dataAreaKey.hIdx      = DB_HANDLE_IDX_R_NCU_SUBSCRIBE_UPSEID_APPID;
    tQueryReq.dataAreaKey.ucKeyLen  = sizeof(WORD64)+sizeof(WORD32);

    CSS_MEMCPY(tQueryReq.dataAreaKey.aucKey,&ddwUPSeid,sizeof(WORD64));
    CSS_MEMCPY(((BYTE*)tQueryReq.dataAreaKey.aucKey)+sizeof(WORD64),&dwAppid,sizeof(WORD32));
    _pDM_PFU_QUERYDYNDATA_BYIDX(&tQueryReq,&tQueryAck);

    if((RC_OK == tQueryAck.retCODE)&&(NULL != tQueryAck.pDataAreaAddr))
    {
        return (T_psNcuDaSubScribeCtx *)tQueryAck.pDataAreaAddr;
    }
    return NULL;
}
T_psNcuDaSubScribeCtx*  psCreatesubscribeByUpseidAppid(WORD64 ddwUPSeid, WORD32 dwAppid, WORD32 hDB)
{
    DM_PFU_ALLOCDYNDATA_BYIDX_REQ tAllocReq = {0};
    DM_PFU_ALLOCDYNDATA_BYIDX_ACK tAllocAck = {0};

    tAllocReq.hDB  = hDB; /* 库句柄待定 */
    tAllocReq.hTbl = DB_HANDLE_R_NCUSUBCRIBE;
    tAllocReq.dataAreaKey.ucKeyFlag = XDB_KEYFLAG_UNIQUE;
    tAllocReq.dataAreaKey.hIdx      = DB_HANDLE_IDX_R_NCU_SUBSCRIBE_UPSEID_APPID;
    tAllocReq.dataAreaKey.ucKeyLen  = sizeof(WORD64)+sizeof(WORD32);

    CSS_MEMCPY(tAllocReq.dataAreaKey.aucKey,&ddwUPSeid,sizeof(WORD64));
    CSS_MEMCPY(((BYTE*)tAllocReq.dataAreaKey.aucKey)+sizeof(WORD64),&dwAppid,sizeof(WORD32));

    _pDM_PFU_ALLOCDYNDATA_BYIDX(&tAllocReq, &tAllocAck);
    if((RC_OK == tAllocAck.retCODE || RC_EXIST == tAllocAck.retCODE) && (NULL != tAllocAck.ptDataAreaAddr))
    {
        T_psNcuDaSubScribeCtx  *ptMcsNcusubscribeCtx = (T_psNcuDaSubScribeCtx *)tAllocAck.ptDataAreaAddr;
        ptMcsNcusubscribeCtx->dwSubscribeCtxId = tAllocAck.dwDataAreaIndex;
        ptMcsNcusubscribeCtx->ddwUPSeid = ddwUPSeid;
        ptMcsNcusubscribeCtx->dwAppId = dwAppid;
        psNcuGetDynCtxStrAppIDByInnerId(dwAppid, ptMcsNcusubscribeCtx->appidstr);
        DEBUG_TRACE(DEBUG_LOW,"\n[Mcs]psNcuGetDynCtxStrAppIDByInnerId: query  strAppID = %s,dwAppid = %u\n",ptMcsNcusubscribeCtx->appidstr,dwAppid);
        return ptMcsNcusubscribeCtx;
    }
    return NULL;
}
WORD32  psDelSubscribeByUpseidAppid(WORD64 ddwUPSeid,WORD32 dwAppid, WORD32 hDB)
{
    DM_PFU_RELEASEDYNDATA_BYIDX_REQ tReleaseReq = {0};
    DM_PFU_RELEASEDYNDATA_BYIDX_ACK tReleaseAck = {0};

    tReleaseReq.hDB         = hDB;
    tReleaseReq.hTbl        = DB_HANDLE_R_NCUSUBCRIBE;
    tReleaseReq.dataAreaKey.ucKeyFlag = XDB_KEYFLAG_UNIQUE;
    tReleaseReq.dataAreaKey.hIdx      = DB_HANDLE_IDX_R_NCU_SUBSCRIBE_UPSEID_APPID;
    tReleaseReq.dataAreaKey.ucKeyLen  = sizeof(WORD64)+sizeof(WORD32);

    zte_memcpy_s(&tReleaseReq.dataAreaKey.aucKey,sizeof(WORD64),&ddwUPSeid,sizeof(WORD64));
    zte_memcpy_s(((BYTE*)tReleaseReq.dataAreaKey.aucKey)+sizeof(WORD64),sizeof(WORD32),&dwAppid,sizeof(WORD32));

    tReleaseAck.retCODE = RC_ERROR;

    _pDM_PFU_RELEASEDYNDATA_BYIDX(&tReleaseReq, &tReleaseAck);
    if(RC_OK == tReleaseAck.retCODE)
    {
        return MCS_RET_SUCCESS;
    }

    return MCS_RET_FAIL;
}

inline WORD32 psVpfuMcsUpdSubScribeCtxByUPseid(WORD64 ddwUPSeid,WORD32 dwCtxId,WORD32 hDB)
{
    DM_PFU_UPDATEDYNDATA_REQ tReq = {0};
    DM_PFU_UPDATEDYNDATA_ACK tAck = {0};

    tReq.hDB = hDB;
    tReq.hTbl= DB_HANDLE_R_NCUSUBCRIBE;
    tReq.ucUpdateMethodFlag = 0;
    tReq.dwDataAreaIndex = dwCtxId;
    tReq.dataAreaKey.ucKeyLen  = sizeof(WORD64);
    tReq.dataAreaKey.ucKeyFlag = XDB_KEYFLAG_NOUNIQUE;
    tReq.dataAreaKey.hIdx = DB_HANDLE_IDX_R_NCU_SUBSCRIBE_UPSEID;

    CSS_MEMCPY(tReq.dataAreaKey.aucKey,&ddwUPSeid,sizeof(WORD64));

    _pDM_PFU_UPDATEDYNDATA_BYNOUNIIDX(&tReq, &tAck);
    if (RC_OK != tAck.retCODE)
    {
        if(RC_EXIST ==tAck.retCODE)
        {
            return MCS_RET_EXIST;
        }
        return MCS_RET_FAIL;
    }
    return MCS_RET_SUCCESS;
}


WORD32 psVpfuMcsGetAllSubScribeCtxByUPseid(WORD64 ddwUPSeid,WORD32 hDB,BYTE *pbQueryAckBuff)
{
    DM_PFU_QUERYDYNDATA_BYIDX_REQ  tQueryReq = {0}; /* 查询请求 */
    DM_PFU_QUERYDYNDATA_BYIDX_ACK *ptQueryAck = (DM_PFU_QUERYDYNDATA_BYIDX_ACK *)pbQueryAckBuff; /* 查询应答 */
    WORD32 dwCtxNum = 0;

    tQueryReq.hDB         = hDB; /* 库句柄待定 */
    tQueryReq.hTbl        = DB_HANDLE_R_NCUSUBCRIBE;
    tQueryReq.dwExpectNum = EXPECT_SUBSCRIBE_NUM;
    tQueryReq.dataAreaKey.ucKeyFlag = XDB_KEYFLAG_NOUNIQUE;
    tQueryReq.dataAreaKey.hIdx      = DB_HANDLE_IDX_R_NCU_SUBSCRIBE_UPSEID;
    tQueryReq.dataAreaKey.ucKeyLen  = sizeof(WORD64);

    CSS_MEMCPY(tQueryReq.dataAreaKey.aucKey,&ddwUPSeid,sizeof(WORD64));
    _pDM_PFU_QUERYDYNDATA_BYIDX(&tQueryReq,ptQueryAck);

    if(RC_OK == ptQueryAck->retCODE)
    {
        dwCtxNum = ptQueryAck->dwDataAreaNum;
        return dwCtxNum;/* 返回会话上下文个数 */
    }
    else
    {
         return dwCtxNum;
    }
}

/* Ended by AICoder, pid:a61c03af54jeda8145620ae950bb369ce6e16951 */
