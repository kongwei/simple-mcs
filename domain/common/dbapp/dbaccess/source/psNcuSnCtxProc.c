/* Started by AICoder, pid:q68bfu24d7m466514dc90a045050f59ea80152ad */
/******************************************************************************
 * 版权所有 (C)2016 深圳市中兴通讯股份有限公司*
 * 模块名          : MCS
 * 文件名          : psNcuSnCtxProc.c
 * 相关文件        :
 * 文件实现功能     : SnCtx增删改查接口
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
#include "psNcuSnCtxProc.h"
#include "psMcsDebug.h"
#include "ps_mcs_define.h"
#include "zte_slibc.h"
#include "MemShareCfg.h"
#include "psNcuCtxFunc.h"
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
extern VOID psVpfuMcsShowNcuMcsSnCtxInfo(T_NcuMcsSnCtx *ptNcuMcsSnCtx);

/**************************************************************************
 *                              局部函数原型(本源文件使用)
 **************************************************************************/

/**************************************************************************
 *                              全局变量(本源文件使用)
 **************************************************************************/
WORD32 g_psNcuPrtSn = 0;
/**************************************************************************
 *                              函数实现(函数布局:总分结构|深度优先)
 **************************************************************************/
T_NcuMcsSnCtx* psCrtSnCtxByFlowIdSeqDir(WORD32 dwFlowId, WORD32 dwSeqNum, BYTE bDir, WORD32 hDB)
{
    DM_PFU_ALLOCDYNDATA_BYIDX_REQ tAllocReq = {0};
    DM_PFU_ALLOCDYNDATA_BYIDX_ACK tAllocAck = {0};

    tAllocReq.hDB  = hDB;
    tAllocReq.hTbl = DB_HANDLE_R_NCU_SN;
    tAllocReq.dataAreaKey.ucKeyFlag = XDB_KEYFLAG_UNIQUE;
    tAllocReq.dataAreaKey.hIdx      = DB_HANDLE_IDX_R_NCU_SN_FLOWID_SN_DIR;
    tAllocReq.dataAreaKey.ucKeyLen  = sizeof(WORD32) + sizeof(WORD32) + sizeof(BYTE);

    zte_memcpy_s(tAllocReq.dataAreaKey.aucKey,sizeof(WORD32),&dwFlowId,sizeof(WORD32));
    zte_memcpy_s(&tAllocReq.dataAreaKey.aucKey[sizeof(WORD32)],sizeof(WORD32),&dwSeqNum,sizeof(WORD32));
    zte_memcpy_s(&tAllocReq.dataAreaKey.aucKey[sizeof(WORD32)+sizeof(WORD32)],sizeof(BYTE),&bDir,sizeof(BYTE));

    _pDM_PFU_ALLOCDYNDATA_BYIDX(&tAllocReq, &tAllocAck);
    if((RC_OK == tAllocAck.retCODE || RC_EXIST == tAllocAck.retCODE) && (NULL != tAllocAck.ptDataAreaAddr))
    {
        T_NcuMcsSnCtx *ptNcuMcsSnCtx = (T_NcuMcsSnCtx *)tAllocAck.ptDataAreaAddr;
        ptNcuMcsSnCtx->dwCtxID       = tAllocAck.dwDataAreaIndex;
        ptNcuMcsSnCtx->dwFlowId      = dwFlowId;
        ptNcuMcsSnCtx->dwAckNum      = dwSeqNum; //seqnum + payload
        ptNcuMcsSnCtx->bDir          = bDir;
        ptNcuMcsSnCtx->dwCreateTimeStamp    = psFtmGetPowerOnSec();
        return ptNcuMcsSnCtx;
    }

    return NULL;
}

T_NcuMcsSnCtx* psQrySnCtxByFlowIdSeqDir(WORD32 dwFlowId, WORD32 dwSeqNum, BYTE bDir, WORD32 hDB)
{

    DM_PFU_QUERYDYNDATA_BYIDX_REQ tQueryReq = {0};
    DM_PFU_QUERYDYNDATA_BYIDX_ACK tQueryAck = {0};

    tQueryReq.hDB         = hDB;
    tQueryReq.hTbl        = DB_HANDLE_R_NCU_SN;
    tQueryReq.dwExpectNum = 1;
    tQueryReq.dataAreaKey.ucKeyFlag = XDB_KEYFLAG_UNIQUE;
    tQueryReq.dataAreaKey.hIdx      = DB_HANDLE_IDX_R_NCU_SN_FLOWID_SN_DIR;
    tQueryReq.dataAreaKey.ucKeyLen  = sizeof(WORD32) + sizeof(WORD32) + sizeof(BYTE);
    
    zte_memcpy_s(tQueryReq.dataAreaKey.aucKey,sizeof(WORD32),&dwFlowId,sizeof(WORD32));
    zte_memcpy_s(&tQueryReq.dataAreaKey.aucKey[sizeof(WORD32)],sizeof(WORD32),&dwSeqNum,sizeof(WORD32));
    zte_memcpy_s(&tQueryReq.dataAreaKey.aucKey[sizeof(WORD32)+sizeof(WORD32)],sizeof(BYTE),&bDir,sizeof(BYTE));
    
    _pDM_PFU_QUERYDYNDATA_BYIDX(&tQueryReq,&tQueryAck);

    if((RC_OK == tQueryAck.retCODE)&&(NULL != tQueryAck.pDataAreaAddr))
    {
        if (g_psNcuPrtSn)
        {
            psVpfuMcsShowNcuMcsSnCtxInfo((T_NcuMcsSnCtx*)tQueryAck.pDataAreaAddr);
        }
        return (T_NcuMcsSnCtx*)tQueryAck.pDataAreaAddr;
    }
    return NULL;
}

T_NcuMcsSnCtx* psQryNcuMcsSnCtxById(WORD32 dwCtxId, WORD32 hDB)
{
    DM_PFU_QUERYDYNDATA_BYTUPLENO_REQ tQueryReq = {0};
    DM_PFU_QUERYDYNDATA_BYTUPLENO_ACK tQueryAck = {0};
    _DB_STATEMENT_TRUE_RTN_VALUE((0 == dwCtxId), NULL);
    tQueryReq.hDB               = hDB;
    tQueryReq.hTbl              = DB_HANDLE_R_NCU_SN;
    tQueryReq.dwDataAreaIndex   = dwCtxId;
    _pDM_PFU_QUERYDYNDATA_BYTUPLENO(&tQueryReq, &tQueryAck);
    _DB_STATEMENT_TRUE_RTN_VALUE((RC_OK != tQueryAck.retCODE), NULL);
    return (T_NcuMcsSnCtx*)tQueryAck.ptDataAreaAddr;
}

WORD32 psUpdSnCtxByFlowId(WORD32 dwFlowId,WORD32 dwCtxID,WORD32 hDB)
{
    DM_PFU_UPDATEDYNDATA_REQ tReq = {0};
    DM_PFU_UPDATEDYNDATA_ACK tAck = {0};

    tReq.hDB    = hDB;
    tReq.hTbl   = DB_HANDLE_R_NCU_SN;
    tReq.ucUpdateMethodFlag    = 1;
    tReq.dwDataAreaIndex       = dwCtxID;
    tReq.dataAreaKey.ucKeyLen  = sizeof(WORD32);
    tReq.dataAreaKey.ucKeyFlag = XDB_KEYFLAG_NOUNIQUE;
    tReq.dataAreaKey.hIdx      = DB_HANDLE_IDX_R_NCU_SN_FLOWID;

    *((WORD32 *)&tReq.dataAreaKey.aucKey[0]) = dwFlowId;

    _pDM_PFU_UPDATEDYNDATA_BYNOUNIIDX(&tReq, &tAck);
    if (RC_OK != tAck.retCODE)
    {
        return MCS_RET_FAIL;
    }

    return MCS_RET_SUCCESS;
}

T_NcuMcsSnCtx* psGetSnCtxByFlowIdSeqDir(WORD32 dwFlowId, WORD32 dwSeqNum, BYTE bDir, WORD32 hDB)
{
    DEBUG_TRACE(DEBUG_LOW, "dwFlowId = %u, dwSeqNum = %u, bDir = %u, hDB = %x", dwFlowId, dwSeqNum, bDir, hDB);
    T_NcuMcsSnCtx* ptNcuSnCtx = psQrySnCtxByFlowIdSeqDir(dwFlowId, dwSeqNum, bDir, hDB);
    if (NULL != ptNcuSnCtx)
    {
        return ptNcuSnCtx;
    }

    ptNcuSnCtx = psCrtSnCtxByFlowIdSeqDir(dwFlowId, dwSeqNum, bDir, hDB);
    if(NULL != ptNcuSnCtx)
    {
        psUpdSnCtxByFlowId(dwFlowId, ptNcuSnCtx->dwCtxID, hDB);
    }
    return ptNcuSnCtx;
}

WORD32 psGetAllSnCtxByFlowId(WORD32 dwFlowId, WORD32 hDB, BYTE *pbQueryAckBuff)
{
    DM_PFU_QUERYDYNDATA_BYIDX_REQ tQueryReq   = {0};
    LP_DM_PFU_QUERYDYNDATA_BYIDX_ACK ptQueryAck = (LP_DM_PFU_QUERYDYNDATA_BYIDX_ACK)pbQueryAckBuff; /* 查询应答 */
    WORD32 dwDaDelayCtxNum = 0;

    tQueryReq.hDB         = hDB;
    tQueryReq.hTbl        = DB_HANDLE_R_NCU_SN;
    tQueryReq.dwExpectNum = MCS_DYNCTX_NOUNIQ_EXPECTNUM;
    tQueryReq.dataAreaKey.ucKeyFlag = XDB_KEYFLAG_NOUNIQUE;
    tQueryReq.dataAreaKey.hIdx      = DB_HANDLE_IDX_R_NCU_SN_FLOWID;
    tQueryReq.dataAreaKey.ucKeyLen  = sizeof(WORD32);

    CSS_MEMCPY(tQueryReq.dataAreaKey.aucKey,&dwFlowId,sizeof(WORD32));

    ptQueryAck->retCODE = RC_ERROR;
    ptQueryAck->dwDataAreaNum = 0;

    _pDM_PFU_QUERYDYNDATA_BYIDX(&tQueryReq,ptQueryAck);

    if(RC_OK == ptQueryAck->retCODE)
    {
        dwDaDelayCtxNum = ptQueryAck->dwDataAreaNum;
    }
    return dwDaDelayCtxNum;
}

WORD32 psDelAllSnCtxByFlowId(WORD32 dwFlowId, WORD32 hDB)
{
    WORD16 bSoftThreadNo = _NCU_GET_CPUNO(hDB);
    if (bSoftThreadNo >= MEDIA_THRD_NUM+1)
    {
        return MCS_RET_FAIL;
    }
    MCS_DM_QUERYDYNDATA_ACK *ptMcsDynCtxNoUniqAck = &g_ptVpfuShMemVar->tGwGloData.atMcsDynCtxNoUniqAck[bSoftThreadNo];

    WORD32 dwAppctxNum = psGetAllSnCtxByFlowId(dwFlowId, hDB, (BYTE*)ptMcsDynCtxNoUniqAck);
    WORD32     dwCtxId = 0;
    WORD32           i = 0;

    for(; i < MCS_DYNCTX_NOUNIQ_EXPECTNUM && i < dwAppctxNum; i++)
    {
        dwCtxId = ptMcsDynCtxNoUniqAck->adwDataArea[i];
        psVpfuMcsDelCtxById(dwCtxId, hDB, DB_HANDLE_R_NCU_SN);
    }
    return MCS_RET_SUCCESS;
}
/* Ended by AICoder, pid:q68bfu24d7m466514dc90a045050f59ea80152ad */
