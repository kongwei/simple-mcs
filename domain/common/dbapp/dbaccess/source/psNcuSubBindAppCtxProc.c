/* Started by AICoder, pid:n76d5ca4behd3cb145c60847805fe55d5500e62d */
/******************************************************************************
 * 版权所有 (C)2016 深圳市中兴通讯股份有限公司*
 * 模块名          : MCS
 * 文件名          : psNcuSubBindAppCtxProc.c
 * 相关文件        :
 * 文件实现功能     : SUBBINDAPP上下文增删改查接口
 * 归属团队        : M6
 * 版本           : V7.24.30
-------------------------------------------------------------------------------
* 修改记录:
* 修改日期            版本号           修改人           修改内容
* 2024-09-20    V7.24.30              wya          create
******************************************************************************/
/**************************************************************************
 *                              头文件(满足最小依赖请按照DDD分层架构逐层依赖)
 **************************************************************************/
#include "psNcuSubBindAppCtxProc.h"
#define USE_DM_UPFCOMM_GETALLR_SUBBINDAPP
#include "dbm_lib_upfcomm.h"
#include "r_subbindapp.h"
#include "ps_ncu_typedef.h"
#include "xdb_pfu_dyntbl_acc.h"
#include "xdb_pfu_com.h"
#include "ps_db_define_ncu.h"
#include "psNcuApplicationMapCtxProc.h"
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
T_psNcuAppidRelation*  psQueryAppidRelateCtxBySubAppid(WORD32 dwSubAppid)
{
    DM_PFU_QUERYDYNDATA_BYIDX_REQ tQueryReq = {0}; /* 查询请求 */
    DM_PFU_QUERYDYNDATA_BYIDX_ACK tQueryAck = {0}; /* 查询应答 */

    tQueryReq.hDB         = _NCU_DBHANDLE_COMM; /* 库句柄待定 */
    tQueryReq.hTbl        = DB_HANDLE_R_NCUAPPID_RELATE;
    tQueryReq.dwExpectNum = 1;
    tQueryReq.dataAreaKey.ucKeyFlag = XDB_KEYFLAG_UNIQUE;
    tQueryReq.dataAreaKey.hIdx      = DB_HANDLE_IDX_R_NCU_APPID_RELATE_SUBAPPID;
    tQueryReq.dataAreaKey.ucKeyLen  = sizeof(WORD32);

    CSS_MEMCPY(tQueryReq.dataAreaKey.aucKey,&dwSubAppid,sizeof(WORD32));
    _pDM_PFU_QUERYDYNDATA_BYIDX(&tQueryReq,&tQueryAck);

    if((RC_OK == tQueryAck.retCODE)&&(NULL != tQueryAck.pDataAreaAddr))
    {
        return (T_psNcuAppidRelation *)tQueryAck.pDataAreaAddr;
    }
    return NULL;
}

T_psNcuAppidRelation*  psCrtAppidRelateCtxBySubAppid(WORD32 dwSubAppid)
{
    DM_PFU_ALLOCDYNDATA_BYIDX_REQ tAllocReq = {0};
    DM_PFU_ALLOCDYNDATA_BYIDX_ACK tAllocAck = {0};

    tAllocReq.hDB  = _NCU_DBHANDLE_COMM; /* 库句柄待定 */
    tAllocReq.hTbl = DB_HANDLE_R_NCUAPPID_RELATE;
    tAllocReq.dataAreaKey.ucKeyFlag = XDB_KEYFLAG_UNIQUE;
    tAllocReq.dataAreaKey.hIdx      = DB_HANDLE_IDX_R_NCU_APPID_RELATE_SUBAPPID;
    tAllocReq.dataAreaKey.ucKeyLen  = sizeof(WORD32);

    CSS_MEMCPY(tAllocReq.dataAreaKey.aucKey,&dwSubAppid,sizeof(WORD32));

    _pDM_PFU_ALLOCDYNDATA_BYIDX(&tAllocReq, &tAllocAck);
    if((RC_OK == tAllocAck.retCODE || RC_EXIST == tAllocAck.retCODE) && (NULL != tAllocAck.ptDataAreaAddr))
    {
        T_psNcuAppidRelation  *ptAppidRelateCtx = (T_psNcuAppidRelation *)tAllocAck.ptDataAreaAddr;
        ptAppidRelateCtx->dwCtxId = tAllocAck.dwDataAreaIndex;
        ptAppidRelateCtx->dwSubAppid = dwSubAppid;
        return ptAppidRelateCtx;
    }
    return NULL;
}

WORD32  psDelAppidRelateCtxBySubAppid(WORD32 dwSubAppid)
{
    DM_PFU_RELEASEDYNDATA_BYIDX_REQ tReleaseReq = {0};
    DM_PFU_RELEASEDYNDATA_BYIDX_ACK tReleaseAck = {0};

    tReleaseReq.hDB         = _NCU_DBHANDLE_COMM;
    tReleaseReq.hTbl        = DB_HANDLE_R_NCUAPPID_RELATE;
    tReleaseReq.dataAreaKey.ucKeyFlag = XDB_KEYFLAG_UNIQUE;
    tReleaseReq.dataAreaKey.hIdx      = DB_HANDLE_IDX_R_NCU_APPID_RELATE_SUBAPPID;
    tReleaseReq.dataAreaKey.ucKeyLen  = sizeof(WORD32);

    zte_memcpy_s(&tReleaseReq.dataAreaKey.aucKey,sizeof(WORD32),&dwSubAppid,sizeof(WORD32));
    tReleaseAck.retCODE = RC_ERROR;

    _pDM_PFU_RELEASEDYNDATA_BYIDX(&tReleaseReq, &tReleaseAck);
    if(RC_OK == tReleaseAck.retCODE)
    {
        return MCS_RET_SUCCESS;
    }

    return MCS_RET_FAIL;
}

WORD32 psNcuMcsUpdAppidRelateCtxByAppid(WORD32 dwAppid,WORD32 dwCtxId)
{
    DM_PFU_UPDATEDYNDATA_REQ tReq = {0};
    DM_PFU_UPDATEDYNDATA_ACK tAck = {0};

    tReq.hDB = _NCU_DBHANDLE_COMM;
    tReq.hTbl= DB_HANDLE_R_NCUAPPID_RELATE;
    tReq.ucUpdateMethodFlag = 0;
    tReq.dwDataAreaIndex = dwCtxId;
    tReq.dataAreaKey.ucKeyLen  = sizeof(WORD32);
    tReq.dataAreaKey.ucKeyFlag = XDB_KEYFLAG_NOUNIQUE;
    tReq.dataAreaKey.hIdx = DB_HANDLE_IDX_R_NCU_APPID_RELATE_APPID;

    zte_memcpy_s(&tReq.dataAreaKey.aucKey,sizeof(WORD32),&dwAppid,sizeof(WORD32));
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

WORD32 psVpfuMcsGetAllAppidRelateCtxByAppid(WORD32 dwAppid, BYTE *pbQueryAckBuff)
{
    DM_PFU_QUERYDYNDATA_BYIDX_REQ  tQueryReq = {0}; /* 查询请求 */
    DM_PFU_QUERYDYNDATA_BYIDX_ACK *ptQueryAck = (DM_PFU_QUERYDYNDATA_BYIDX_ACK *)pbQueryAckBuff; /* 查询应答 */
    WORD32 dwCtxNum = 0;

    tQueryReq.hDB         = _NCU_DBHANDLE_COMM; /* 库句柄待定 */
    tQueryReq.hTbl        = DB_HANDLE_R_NCUAPPID_RELATE;
    tQueryReq.dwExpectNum = EXPECT_DATA_APP_NUM;
    tQueryReq.dataAreaKey.ucKeyFlag = XDB_KEYFLAG_NOUNIQUE;
    tQueryReq.dataAreaKey.hIdx      = DB_HANDLE_IDX_R_NCU_APPID_RELATE_APPID;
    tQueryReq.dataAreaKey.ucKeyLen  = sizeof(WORD32);

    zte_memcpy_s(&tQueryReq.dataAreaKey.aucKey,sizeof(WORD32),&dwAppid,sizeof(WORD32));
    _pDM_PFU_QUERYDYNDATA_BYIDX(&tQueryReq,ptQueryAck);

    if(RC_OK == ptQueryAck->retCODE)
    {
        dwCtxNum = ptQueryAck->dwDataAreaNum;
        return dwCtxNum;/* 上下文个数 */
    }
    else
    {
         return dwCtxNum;
    }
}

WORD32 psNcuGetAppidBySubAppid(WORD32 dwSubAppid)
{
    T_psNcuAppidRelation* ptAppidRelateCtx = psQueryAppidRelateCtxBySubAppid(dwSubAppid);
    if(NULL == ptAppidRelateCtx)
    {
        return INVALIDINNERAPPID;
    }
    #ifndef FT_TEST
    WORD32 dwAppid = psNcuGetDynCtxInnerAppIDByStrAppID(ptAppidRelateCtx->appidStr);
    ptAppidRelateCtx->dwAppid = dwAppid;
    #endif
    return ptAppidRelateCtx->dwAppid;
}

BOOLEAN psNcuGetSubAppidStr(CHAR* ptStr, WORD32 dwSubAppid)
{
    if(NULL == ptStr)
    {
        return FALSE;
    }
    T_psNcuAppidRelation* ptAppidRelateCtx = psQueryAppidRelateCtxBySubAppid(dwSubAppid);
    if(NULL == ptAppidRelateCtx)
    {
        return FALSE;
    }
    zte_memcpy_s(ptStr, LEN_APPLICATIONMAP_APPLICATION_MAX+1, ptAppidRelateCtx->subAppidStr, LEN_APPLICATIONMAP_APPLICATION_MAX+1);
    return TRUE;
}

VOID allocSubBindAppBySubApp(CHAR *subapp, CHAR *application)
{
    if (NULL == subapp || NULL == application) 
    {
        return ;
    }
    WORD32 dwSubAppid = 0;
    WORD32    dwAppid = 0;

    if (FALSE == psNcuGetCfgInnerAppIdByStrAppidmap(subapp, &dwSubAppid))
    {
        return;
    }
    T_psNcuAppidRelation *ptNcuAppidRelt = psCrtAppidRelateCtxBySubAppid(dwSubAppid);
    if (NULL == ptNcuAppidRelt)
    {
        return ;
    }
    zte_memcpy_s(ptNcuAppidRelt->subAppidStr, sizeof(ptNcuAppidRelt->subAppidStr), subapp, sizeof(ptNcuAppidRelt->subAppidStr));
    zte_memcpy_s(ptNcuAppidRelt->appidStr, sizeof(ptNcuAppidRelt->appidStr), application, sizeof(ptNcuAppidRelt->appidStr));
    if (FALSE == psNcuGetCfgInnerAppIdByStrAppidmap(application, &dwAppid))
    {
        return;
    }
    ptNcuAppidRelt->dwAppid = dwAppid;
    psNcuMcsUpdAppidRelateCtxByAppid(dwAppid, ptNcuAppidRelt->dwCtxId);
}

VOID psNcuGetSubBindAppAllCfg()
{
    DM_UPFCOMM_GETALLR_SUBBINDAPP_REQ    req = {0};
    DM_UPFCOMM_GETALLR_SUBBINDAPP_ACK    ack = {0};
    BOOLEAN                            bflag = FALSE;
    WORD16                                 i = 0;
    WORD32                            dwLoop = R_SUBBINDAPP_CAPACITY;
    do
    {
        req.msgType = MSG_CALL;
        req.dwTupleNo = ack.dwTupleNo;
        ack.retCODE = RC_DBM_ERROR;
        bflag = DBM_CALL(DM_UPFCOMM_GETALLR_SUBBINDAPP, (LPSTR)&req, (LPSTR)&ack);
        MCS_CHK_VOID(!bflag);
        MCS_CHK_VOID(RC_DBM_OK != ack.retCODE);

        for(i = 0; (i < UPFNCU_GETALL_MAX) && (i < ack.dwValidNum); i++)
        {
            dwLoop--;
            allocSubBindAppBySubApp(ack.subapp[i], ack.application[i]);
        }
    }while(!ack.blEnd && dwLoop);
    return;
}

T_psNcuAppidRelation *psMcsGetAppidRelateCtxById(WORD32 dwCtxId, WORD32 hDB)
{
    DM_PFU_QUERYDYNDATA_BYTUPLENO_REQ tQueryReq = {0}; /* 查询请求 */
    DM_PFU_QUERYDYNDATA_BYTUPLENO_ACK tQueryAck = {0}; /* 查询应答 */
    _DB_STATEMENT_TRUE_RTN_VALUE((0 == dwCtxId), NULL);
    tQueryReq.hDB               = _NCU_DBHANDLE_COMM; /* 库句柄待定 */
    tQueryReq.hTbl              = DB_HANDLE_R_NCUAPPID_RELATE;
    tQueryReq.dwDataAreaIndex   = dwCtxId;
    _pDM_PFU_QUERYDYNDATA_BYTUPLENO(&tQueryReq, &tQueryAck);
    _DB_STATEMENT_TRUE_RTN_VALUE((RC_OK != tQueryAck.retCODE), NULL);
    return (T_psNcuAppidRelation*)tQueryAck.ptDataAreaAddr;/*返回数据区指针 */
}

VOID showSubBindApp(CHAR *subapp)
{
    if (NULL == subapp)
    {
        return ;
    }

    WORD32 dwSubAppid = 0;
    if (FALSE == psNcuGetCfgInnerAppIdByStrAppidmap(subapp, &dwSubAppid))
    {
        return;
    }
    T_psNcuAppidRelation *ptNcuAppidRelt = psQueryAppidRelateCtxBySubAppid(dwSubAppid);
    if (NULL == ptNcuAppidRelt)
    {
        return ;
    }
    zte_printf_s("dwCtxId     = %u\n", ptNcuAppidRelt->dwCtxId);
    zte_printf_s("dwSubAppid  = %u\n", ptNcuAppidRelt->dwSubAppid);
    zte_printf_s("dwAppid     = %u\n", ptNcuAppidRelt->dwAppid);
    zte_printf_s("subAppidStr = %s\n", ptNcuAppidRelt->subAppidStr);
    zte_printf_s("appidStr    = %s\n", ptNcuAppidRelt->appidStr);
}
/* Ended by AICoder, pid:n76d5ca4behd3cb145c60847805fe55d5500e62d */
