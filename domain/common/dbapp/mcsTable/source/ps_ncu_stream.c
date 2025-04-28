/*********************************************************************
* 版权所有(C) 中兴通信股份有限公司. 保留所有权利!
*
* 文件名称： r_flowctx.c
* 文件标识：
* 内容摘要： 流上下文表
* 其它说明：
* 当前版本： ZXUN-xGW V4.13.20
* 作    者： zjc
* 完成日期： 2013-07-03
**********************************************************************/

/**************************************************************************
*                            头文件                                      *
**************************************************************************/

// #include "xdb_pub_pfu.h"
#include "xdb_core_pfu.h"
#include "xdb_core_pfu_db.h"
#include "xdb_core_uniindex.h"
#include "xdb_core_pfu_table.h"
#include "xdb_core_pfu_tblimp_api.h"
#include "ps_db_define_ncu.h"
#include "ps_ncu_stream.h"
#include "ps_mcs_define.h"
#include "ncu_capacity.h"

/**************************************************************************
*                             全局变量                                    *
**************************************************************************/
T_PfuTblIdxReg       g_pFlowReg[XDB_NCU_DB_NUM] = {0};

/**************************************************************************
*                             外部函数声明                                    *
**************************************************************************/


/**************************************************************************
*                     全局函数实现                                      *
**************************************************************************/
void r_flowctx_UIdxMerge(void* pTuple, void* pKey,  DWORD dwKeyLen)
{
    BYTE* pbTuple=(BYTE *)pTuple;
    BYTE* pbKey=(BYTE *)pKey;
    *(WORD64 *)&pbKey[0] = *(WORD64 *)&pbTuple[0];
    *(WORD64 *)&pbKey[8] = *(WORD64 *)&pbTuple[8];
    *(WORD64 *)&pbKey[16] = *(WORD64 *)&pbTuple[16];
    *(WORD64 *)&pbKey[24] = *(WORD64 *)&pbTuple[24];
    *(WORD64 *)&pbKey[32] = *(WORD64 *)&pbTuple[32];
    *(WORD32 *)&pbKey[40] = *(WORD32 *)&pbTuple[40];
    *(WORD16 *)&pbKey[44] = *(WORD16 *)&pbTuple[44];
    return;
}

BOOLEAN r_flowctx_UIdxCmp(void* pTuple, void* pKey,  DWORD dwKeyLen)
{
    BYTE* pbTuple = (BYTE*)pTuple;
    BYTE* pbKey = (BYTE*)pKey;
    return ((*(WORD64 *)pbKey == *(WORD64 *)pbTuple)
        && (*(WORD64 *)&pbKey[8] == *(WORD64 *)&pbTuple[8])
        && (*(WORD64 *)&pbKey[16] == *(WORD64 *)&pbTuple[16])
        && (*(WORD64 *)&pbKey[24] == *(WORD64 *)&pbTuple[24])
        && (*(WORD64 *)&pbKey[32] == *(WORD64 *)&pbTuple[32])
        && (*(WORD32 *)&pbKey[40] == *(WORD32 *)&pbTuple[40])
        && (*(WORD16 *)&pbKey[44] == *(WORD16 *)&pbTuple[44]));
}

void r_flowctx_UIdxCpy(void* pTuple, void* pKey,DWORD dwKeyLen)
{
	BYTE* pbTuple=(BYTE *)pTuple;
	BYTE* pbKey=(BYTE *)pKey;

    *(WORD64 *)&pbTuple[0] = *(WORD64 *)&pbKey[0];
    *(WORD64 *)&pbTuple[8] = *(WORD64 *)&pbKey[8];
    *(WORD64 *)&pbTuple[16] = *(WORD64 *)&pbKey[16];
    *(WORD64 *)&pbTuple[24] = *(WORD64 *)&pbKey[24];
    *(WORD64 *)&pbTuple[32] = *(WORD64 *)&pbKey[32];
    *(WORD32 *)&pbTuple[40] = *(WORD32 *)&pbKey[40];
    *(WORD16 *)&pbTuple[44] = *(WORD16 *)&pbKey[44];
    return;
}

BOOLEAN r_flowctx_ReleaseRsc(LPT_PfuDataBase pPfuDbReg, LPT_PfuTableReg pPfuTableReg, DWORD dwTupleNo)
{
    LPT_PfuUniIdxReg            ptPfuUniIdxReg = NULL;
    BOOLEAN                      dbRet = FALSE;

    ptPfuUniIdxReg = xdb_Pfu_Get_UniIdxReg_ByDbReg(pPfuDbReg, DB_HANDLE_IDX_R_NCU_FLOWCTX);
    _DB_STATEMENT_TRUE_RTN_VALUE((NULL == ptPfuUniIdxReg), FALSE);
    xdb_pfu_delete_UniIdx_by_tupleNo(pPfuTableReg, ptPfuUniIdxReg, dwTupleNo);

    dbRet = xdb_Delete_Tuple(pPfuTableReg,dwTupleNo);
    _DB_STATEMENT_TRUE_RTN_VALUE((FALSE == dbRet), FALSE);
    return TRUE;
}

DWORD  _db_get_flowctx_capacity()
{
#ifndef FT_TEST
    return _db_get_ncu_capacity_by_tblName(NCCTXNAME_R_FLOWCTX, R_FLOWCTX);
#else
    return 20;
#endif
}

/**********************************************************************
* 函数名称：create_r_flowctx
* 功能描述：表创建函数
* 输入参数：dwDbHandle, 库句柄
* 输出参数：无
* 返 回 值：TRUE,成功  FALSE,失败
* 其它说明：
* 创建日期       版本号              创建人       修改内容
* -----------------------------------------------
* 2013-07-03     ZXUN-xGW V4.13.20      zjc             创建
***********************************************************************/
BOOLEAN create_r_flowctx(DWORD dwDbHandle)
{
    WORD   wLen = 0;
    DWORD dwDynTblCapacity = 0;
    LPT_PfuTableReg      pTableReg = NULL;
    LPT_PfuUniIdxReg     pPfuUniIdxReg = NULL;
    WORD32               dwVCpuNo = 0xff;

    dwDynTblCapacity = _db_get_flowctx_capacity();
    _DB_STATEMENT_TRUE_RTN_VALUE((0 == dwDynTblCapacity), FALSE);

    /* 获取业务上下文长度 */
    wLen = sizeof(T_psNcuFlowCtx);

    /* 创建表 */
    XOS_SysLog(LOG_EMERGENCY, "DB Module: create table r_flowctx start!\n");
    pTableReg = XDB_PFU_CREATE_TABLE(dwDbHandle, DB_HANDLE_R_NCUFLOWCTX, R_FLOWCTX, dwDynTblCapacity, wLen,sizeof(r_flowctx_tuple));//lint !e734
    if (NULL == pTableReg)
    {
        XOS_SysLog(LOG_EMERGENCY, "[File:%s],[line:%d]DB Module:create table r_flowctx failed!\n", __FILE__, __LINE__);
        return FALSE;
    }
    /* 创建索引 */
    pPfuUniIdxReg = XDB_PFU_CREATE_UNIIDX(dwDbHandle, DB_HANDLE_IDX_R_NCU_FLOWCTX, XDB_UNIDX_NORMARL,
                                          dwDynTblCapacity, XDB_MOD_TABLE(dwDynTblCapacity),
                                          sizeof(r_flowctx_idx_tuple));//lint !e734
    if (NULL == pPfuUniIdxReg)
    {
        XOS_SysLog(LOG_EMERGENCY, "[File:%s],[line:%d]DB Module: create index DB_HANDLE_IDX_R_NCU_FLOWCTX failed!\n", __FILE__, __LINE__);
        return FALSE;
    }
    xdb_pfu_UniIdx_Cpy_Reg(pPfuUniIdxReg, r_flowctx_UIdxCpy);
    xdb_pfu_UniIdx_Merge_Reg(pPfuUniIdxReg, r_flowctx_UIdxMerge);
    xdb_pfu_UniIdx_Cmp_Reg(pPfuUniIdxReg, r_flowctx_UIdxCmp);
   

    dwVCpuNo = _NCU_GET_CPUNO(dwDbHandle);

    if (dwVCpuNo > CSS_X86_MAX_THREAD_NUM)
    {

        return FALSE;
    }

    g_pFlowReg[dwVCpuNo].pPfuTblReg = pTableReg;
    g_pFlowReg[dwVCpuNo].pPfuIdxReg = pPfuUniIdxReg;

    xdb_Set_ReleaseRsc_Overload(pTableReg,r_flowctx_ReleaseRsc);
    XOS_SysLog(LOG_EMERGENCY, "DB Module: create r_flowctx table and index success!\n");
    return TRUE;
}

/*lint -e701 */
DWORD _xdb_pfu_hash_idx_flowctx(T_PfuUniIdxReg *pIdxReg, void* pInputKey)
{
    lp_r_flowctx_idx_tuple pKey = (lp_r_flowctx_idx_tuple)pInputKey;
    return (((WORD32*)pKey->tCliIP)[0]
          + ((WORD32*)pKey->tCliIP)[1]
          + ((WORD32*)pKey->tCliIP)[2]
          + ((WORD32*)pKey->tCliIP)[3]
          + ((WORD32*)pKey->tSvrIP)[0]
          + ((WORD32*)pKey->tSvrIP)[1]
          + ((WORD32*)pKey->tSvrIP)[2]
          + ((WORD32*)pKey->tSvrIP)[3]
          + ((pKey->wCliPort<<16)|pKey->wSvrPort)
          + (WORD32)pKey->ddwVPNInfo
          + (WORD32)(pKey->ddwVPNInfo >> 32)
          + ((pKey->bIPType<<16)|pKey->bProType));
}
/*lint +e701 */

#define PFU_QUERY_BULK_QUE_FLOW()\
{\
    dwLoop++;\
    dwTupleNoTmp = dwTupleNoNext;\
    if(0 == dwTupleNoTmp)\
    {\
        return dwTupleNoTmp;\
    }\
    if (unlikely(dwLoop > pTableReg->dwCapacity))\
    {\
        return INVAILD_TUPLE_32F;\
    }\
    pCtxAddr = xdb_Get_Tuple_Addr(pTableReg, dwTupleNoTmp);\
    bID = xdb_pfu_Idx_Cmp(pIdxReg, pCtxAddr, pKey);\
    if (bID)\
    {\
        return dwTupleNoTmp;\
    }\
    dwTupleNoNext = _dbTbl_QueGetNext((LP_DB_TBL_QUEINFO_T)pIdxReg->pBulkMemAddr, pTableReg->dwCapacity, dwTupleNoTmp);\
}\

DWORD xdb_pfu_query_bulk_que_flow(LP_DB_TBL_QUEHEAD_T ptQueHead, T_PfuTableReg *pTableReg,
                                  T_PfuUniIdxReg *pIdxReg,VOID *pKey)
{
    DWORD    dwTupleNoTmp , dwTupleNoNext , dwLoop = 0;
    VOID     *pCtxAddr ;
    BOOLEAN   bID;

    dwTupleNoNext = ptQueHead->tQueHead.dwNext;
    PFU_QUERY_BULK_QUE_FLOW();
    PFU_QUERY_BULK_QUE_FLOW();
    PFU_QUERY_BULK_QUE_FLOW();
    PFU_QUERY_BULK_QUE_FLOW();
    PFU_QUERY_BULK_QUE_FLOW();
    do
    {
        dwLoop++;
        dwTupleNoTmp = dwTupleNoNext;
        if(0 == dwTupleNoTmp)
        {
            return 0;
        }
        if(dwLoop > pTableReg->dwCapacity)
        {
            return INVAILD_TUPLE_32F;
        }

        pCtxAddr = xdb_Get_Tuple_Addr(pTableReg, dwTupleNoTmp);
        bID = xdb_pfu_Idx_Cmp(pIdxReg, pCtxAddr, pKey);
        if (bID)
        {
            return dwTupleNoTmp;
        }

        dwTupleNoNext = _dbTbl_QueGetNext((LP_DB_TBL_QUEINFO_T)pIdxReg->pBulkMemAddr, pTableReg->dwCapacity, dwTupleNoTmp);
    } while(1); /*(!bID);Expression 'bID' used in the condition always yields the same result, and causes an unreachable code*/

    return dwTupleNoTmp;
}

BYTE *psNcuQueryStreamByQuintuple(WORD32 hDB, VOID *key)
{
    if(NULL == key)
    {
        return NULL;
    }
    DWORD                       dwTempTupleNo;
    LPT_PfuTableReg             pTempTblRegAddr = NULL;
    LPT_PfuUniIdxReg            pUniIdxReg = NULL;
    LPT_PfuUniIdxHashItem       pHashItem = NULL;
    WORD32                      dwVCpuNo = XDB_NCU_DB_NUM;
    BYTE                        *pTupleAddr = NULL;

    dwVCpuNo        = _NCU_GET_CPUNO(hDB);
    if(dwVCpuNo >= XDB_NCU_DB_NUM)
    {
        return NULL;
    }
    pTempTblRegAddr = g_pFlowReg[dwVCpuNo].pPfuTblReg;
    pUniIdxReg      = g_pFlowReg[dwVCpuNo].pPfuIdxReg;
    if(NULL == pUniIdxReg)
    {
        return NULL;
    }
    pHashItem = (LPT_PfuUniIdxHashItem)_xdb_pfu_unidx_hashitem(pUniIdxReg, key);
    if(NULL == pHashItem)
    {
        return NULL;
    }
    dwTempTupleNo = xdb_pfu_query_bulk_que_flow(&pHashItem->tHash, pTempTblRegAddr, pUniIdxReg,key);
    if((INVAILD_TUPLE_32F == dwTempTupleNo) || (0 == dwTempTupleNo))
    {
        return NULL;
    }
    pTupleAddr = (BYTE *)xdb_Get_Tuple_Head_Addr(pTempTblRegAddr, dwTempTupleNo);
    return pTupleAddr+pTempTblRegAddr->wDataFieldOffset;
}


T_psNcuFlowCtx* psNcuMcsCreatStmByIndex(DB_STRM_INDEX* ptQuintuple,WORD32 hDB)
{
    if (NULL == ptQuintuple)
    {
        return NULL;
    }
    DM_PFU_ALLOCDYNDATA_BYIDX_REQ tAllocReq = {0};
    DM_PFU_ALLOCDYNDATA_BYIDX_ACK tAllocAck = {0};
    tAllocAck.retCODE = RC_ERROR;
    T_psNcuFlowCtx            *ptMcsNcuStmCtx = NULL;

    tAllocReq.hDB                   = hDB; /* 库句柄待定 */
    tAllocReq.hTbl                  = DB_HANDLE_R_NCUFLOWCTX;
    tAllocReq.dataAreaKey.ucKeyFlag = XDB_KEYFLAG_UNIQUE;
    tAllocReq.dataAreaKey.hIdx      = DB_HANDLE_IDX_R_NCU_FLOWCTX;
    tAllocReq.dataAreaKey.ucKeyLen  = sizeof(DB_STRM_INDEX) - DB_STRM_INDEX_RSV_LENGTH;
    CSS_MEMCPY(tAllocReq.dataAreaKey.aucKey,ptQuintuple,tAllocReq.dataAreaKey.ucKeyLen);
    _pDM_PFU_ALLOCDYNDATA_BYIDX(&tAllocReq, &tAllocAck);
    if(NULL == tAllocAck.ptDataAreaAddr)
    {
        return NULL;
    }
    if((RC_OK != tAllocAck.retCODE) && (RC_EXIST != tAllocAck.retCODE))
    {
        return NULL;
    }

    ptMcsNcuStmCtx = (T_psNcuFlowCtx *)tAllocAck.ptDataAreaAddr;
    ptMcsNcuStmCtx->dwFlowID     = tAllocAck.dwDataAreaIndex;
    ptMcsNcuStmCtx->bIPType      = ptQuintuple->bIPType;
    ptMcsNcuStmCtx->bProType     = ptQuintuple->bProType;
    return ptMcsNcuStmCtx;
}


T_psNcuFlowCtx* psNcuMcsGetStmByID(WORD32 dwFlowID,WORD32 hDB)
{
    DM_PFU_QUERYDYNDATA_BYTUPLENO_REQ tQueryReq = {0}; /* 查询请求 */
    DM_PFU_QUERYDYNDATA_BYTUPLENO_ACK tQueryAck = {0}; /* 查询应答 */
    /* 先根据dwLocalTEIDU查询转发表 */

    tQueryReq.hDB             = hDB;  /* 库句柄待定 */
    tQueryReq.hTbl            = DB_HANDLE_R_NCUFLOWCTX;
    tQueryReq.dwDataAreaIndex = dwFlowID;

    _pDM_PFU_QUERYDYNDATA_BYTUPLENO(&tQueryReq,  &tQueryAck);
    if(RC_OK != tQueryAck.retCODE || NULL == tQueryAck.ptDataAreaAddr)
    {
        return NULL;
    }
    return (T_psNcuFlowCtx *)tQueryAck.ptDataAreaAddr;/*返回转发表数据区指针 */
}


WORD32  psNcuMcsRelStmByID(WORD32 dwFlowID,WORD32 hDB)
{
    DM_PFU_RELEASEDYNDATA_BYTUPLENO_REQ tRelReq = {0};
    DM_PFU_RELEASEDYNDATA_BYTUPLENO_ACK tRelAck = {0};

    tRelReq.hDB = hDB; /* 库句柄待定 */
    tRelReq.hTbl= DB_HANDLE_R_NCUFLOWCTX;
    tRelReq.dwDataAreaIndex = dwFlowID;
    tRelAck.retCODE = RC_ERROR;
    _pDM_PFU_RELEASEDYNDATA_BYTUPLENO(&tRelReq,&tRelAck);
    if(RC_OK != tRelAck.retCODE)
    {
        return MCS_RET_FAIL;
    }
    return MCS_RET_SUCCESS;
}

// end of file
