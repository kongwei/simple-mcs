#include "ps_ncu_httplink.h"
#include "ps_db_define_ncu.h"
#include "xdb_core_pfu.h"
#include "xdb_pfu_com.h"
#include "xdb_core_pfu_db.h"
#include "xdb_core_pfu_table.h"
#include "xdb_core_uniindex.h"
#include "xdb_core_pfu_nunidx.h"
#include "xdb_core_pfu_tblimp_api.h"
#include "xdb_pfu_dyntbl_acc.h"
#include "ps_mcs_define.h"
#include "zte_slibc.h"
#include "psMcsDebug.h"

/* Started by AICoder, pid:q2a2d53d41gec9e14e610b22004d124d0557b55a */
void r_ncu_httplink_UIdxMerge(void* pTuple, void* pKey, DWORD dwKeyLen)
{
    _DB_STATEMENT_TRUE_LOG_RTN_NONE((NULL == pTuple), LOG_EMERGENCY);
    _DB_STATEMENT_TRUE_LOG_RTN_NONE((NULL == pKey), LOG_EMERGENCY);

    lp_r_ncu_httplink_idx_tuple lpTmp = (lp_r_ncu_httplink_idx_tuple)pKey;
    lp_r_ncu_httplink_tuple pAddr = (lp_r_ncu_httplink_tuple)pTuple;
    zte_memcpy_s(&(lpTmp->tNwdafIP), sizeof(T_IPComm), &(pAddr->tNwdafIP), sizeof(T_IPComm));
    lpTmp->dwClientProfileID = pAddr->dwClientProfileID;
    lpTmp->wNwdafPort = pAddr->wNwdafPort;
    lpTmp->bSchema    = pAddr->bSchema;
    return;
}

void r_ncu_httplink_UIdxCpy(void* pTuple, void* pKey, DWORD dwKeyLen)
{
    _DB_STATEMENT_TRUE_LOG_RTN_NONE((NULL == pTuple), LOG_EMERGENCY);
    _DB_STATEMENT_TRUE_LOG_RTN_NONE((NULL == pKey), LOG_EMERGENCY);

    lp_r_ncu_httplink_idx_tuple lpTmp = (lp_r_ncu_httplink_idx_tuple)pKey;
    lp_r_ncu_httplink_tuple pAddr = (lp_r_ncu_httplink_tuple)pTuple;
    zte_memcpy_s(&(pAddr->tNwdafIP), sizeof(T_IPComm), &(lpTmp->tNwdafIP), sizeof(T_IPComm));
    pAddr->dwClientProfileID = lpTmp->dwClientProfileID;
    pAddr->wNwdafPort = lpTmp->wNwdafPort;
    pAddr->bSchema    = lpTmp->bSchema;
    return;
}

BOOLEAN r_ncu_httplink_UIdxCmp(void* pTuple, void* pKey, DWORD dwKeyLen)
{
    _DB_STATEMENT_TRUE_RTN_VALUE((NULL == pTuple), FALSE);
    _DB_STATEMENT_TRUE_RTN_VALUE((NULL == pKey), FALSE);

    lp_r_ncu_httplink_idx_tuple lpTmp = (lp_r_ncu_httplink_idx_tuple)pKey;
    lp_r_ncu_httplink_tuple lpAddr = (lp_r_ncu_httplink_tuple)pTuple;

    return ((lpTmp->dwClientProfileID == lpAddr->dwClientProfileID) &&
            (lpTmp->wNwdafPort == lpAddr->wNwdafPort) &&
            (lpTmp->bSchema == lpAddr->bSchema) &&
            (0 == memcmp(&(lpTmp->tNwdafIP), &(lpAddr->tNwdafIP), sizeof(T_IPComm))));
}
/* Ended by AICoder, pid:q2a2d53d41gec9e14e610b22004d124d0557b55a */

/* Started by AICoder, pid:ha0132ce9bp8cb21403f09e940f4c026abe40c4b */
BOOLEAN r_ncu_httplink_ReleaseRsc(LPT_PfuDataBase pPfuDbReg, LPT_PfuTableReg pPfuTableReg, DWORD dwTupleNo)
{
    _DB_STATEMENT_TRUE_RTN_VALUE((NULL == pPfuDbReg), FALSE);
    LPT_PfuUniIdxReg ptPfuUniIdxReg = NULL;

    BOOLEAN dbRet = FALSE;

    T_psNcuHttpLink *ptHttpLinkCtx = psMcsGetHttpLinkCtxById(dwTupleNo, pPfuDbReg->hDB);
    _DB_STATEMENT_TRUE_RTN_VALUE((NULL == ptHttpLinkCtx), FALSE);

    ptPfuUniIdxReg = xdb_Pfu_Get_UniIdxReg_ByDbReg(pPfuDbReg, DB_HANDLE_IDX_R_NCU_HTTPLINK);
    _DB_STATEMENT_TRUE_RTN_VALUE((NULL == ptPfuUniIdxReg), FALSE);
    xdb_pfu_delete_UniIdx_by_tupleNo(pPfuTableReg, ptPfuUniIdxReg, dwTupleNo);

    dbRet = xdb_Delete_Tuple(pPfuTableReg, dwTupleNo);
    _DB_STATEMENT_TRUE_RTN_VALUE((FALSE == dbRet), FALSE);
    return TRUE;
}
/* Ended by AICoder, pid:ha0132ce9bp8cb21403f09e940f4c026abe40c4b */

/* Started by AICoder, pid:e2d81tf4d6gad401482109e20021fd4eb0028a44 */
BOOL create_r_ncu_httplink(DWORD dwDbHandle)
{
    DWORD dwDynTblCapacity = 0;
    LPT_PfuTableReg      pTableReg = NULL;
    LPT_PfuUniIdxReg     pPfuUniIdxReg = NULL;

#ifndef FT_TEST
    dwDynTblCapacity = ctR_ncu_httplink_CAPACITY;
#else
    dwDynTblCapacity = 20;
#endif
    zte_printf_s("ncu httplink capacity: %u\n", dwDynTblCapacity);

    /* 创建表 */
    XOS_SysLog(LOG_EMERGENCY, "DB Module: create table r_ncu_httplink start!\n");
    pTableReg = XDB_PFU_CREATE_TABLE(dwDbHandle, DB_HANDLE_R_NCU_HTTPLINK, R_NCU_HTTPLINK,
                                     dwDynTblCapacity, sizeof(T_psNcuHttpLink),
                                     sizeof(r_ncu_httplink_tuple));
    if (NULL == pTableReg)
    {
        XOS_SysLog(LOG_EMERGENCY, "[File:%s],[line:%d]DB Module:create table r_ncu_httplink failed!\n", __FILE__, __LINE__);
        return FALSE;
    }

    /* 创建索引 */
    pPfuUniIdxReg = XDB_PFU_CREATE_UNIIDX(dwDbHandle, DB_HANDLE_IDX_R_NCU_HTTPLINK, XDB_UNIDX_NORMARL,
                                          dwDynTblCapacity, XDB_MOD_TABLE(dwDynTblCapacity),
                                          sizeof(r_ncu_httplink_idx_tuple));
    if (NULL == pPfuUniIdxReg)
    {
        XOS_SysLog(LOG_EMERGENCY, "[File:%s],[line:%d]DB Module: create index DB_HANDLE_IDX_R_NCU_HTTPLINK failed!\n", __FILE__, __LINE__);
        return FALSE;
    }
    xdb_pfu_UniIdx_Cpy_Reg(pPfuUniIdxReg, r_ncu_httplink_UIdxCpy);
    xdb_pfu_UniIdx_Merge_Reg(pPfuUniIdxReg, r_ncu_httplink_UIdxMerge);
    xdb_pfu_UniIdx_Cmp_Reg(pPfuUniIdxReg, r_ncu_httplink_UIdxCmp);

    xdb_Set_ReleaseRsc_Overload(pTableReg, r_ncu_httplink_ReleaseRsc);
    XOS_SysLog(LOG_EMERGENCY, "DB Module: create r_ncu_httplink table and index success!\n");

    return TRUE;
}
/* Ended by AICoder, pid:e2d81tf4d6gad401482109e20021fd4eb0028a44 */

T_psNcuHttpLink* psMcsGetHttpLinkCtxById(WORD32 dwCtxId, WORD32 hDB)
{
    DM_PFU_QUERYDYNDATA_BYTUPLENO_REQ tQueryReq = {0}; /* 查询请求 */
    DM_PFU_QUERYDYNDATA_BYTUPLENO_ACK tQueryAck = {0}; /* 查询应答 */
    _DB_STATEMENT_TRUE_RTN_VALUE((0 == dwCtxId), NULL);
    tQueryReq.hDB               = hDB; /* 库句柄待定 */
    tQueryReq.hTbl              = DB_HANDLE_R_NCU_HTTPLINK;
    tQueryReq.dwDataAreaIndex   = dwCtxId;
    _pDM_PFU_QUERYDYNDATA_BYTUPLENO(&tQueryReq, &tQueryAck);
    _DB_STATEMENT_TRUE_RTN_VALUE((RC_OK != tQueryAck.retCODE), NULL);
    return (T_psNcuHttpLink*)tQueryAck.ptDataAreaAddr;/*返回数据区指针 */
}

T_psNcuHttpLink* psQueryHttpLinkByUniqueIdx(T_IPComm *tNwdafIP, WORD32 dwClientProfileID, WORD16 wNwdafPort, BYTE bSchema, WORD32 hDB)
{
    _DB_STATEMENT_TRUE_RTN_VALUE((NULL == tNwdafIP), NULL);

    DM_PFU_QUERYDYNDATA_BYIDX_REQ tQueryReq = {0}; /* 查询请求 */
    DM_PFU_QUERYDYNDATA_BYIDX_ACK tQueryAck = {0}; /* 查询应答 */

    r_ncu_httplink_idx_tuple *ptIndex = NULL;

    tQueryReq.hDB         = hDB; /* 库句柄待定 */
    tQueryReq.hTbl        = DB_HANDLE_R_NCU_HTTPLINK;
    tQueryReq.dwExpectNum = 1;
    tQueryReq.dataAreaKey.ucKeyFlag = XDB_KEYFLAG_UNIQUE;
    tQueryReq.dataAreaKey.hIdx      = DB_HANDLE_IDX_R_NCU_HTTPLINK;
    tQueryReq.dataAreaKey.ucKeyLen  = sizeof(r_ncu_httplink_idx_tuple);
    ptIndex = (r_ncu_httplink_idx_tuple *)tQueryReq.dataAreaKey.aucKey;
    ptIndex->dwClientProfileID = dwClientProfileID;
    ptIndex->wNwdafPort = wNwdafPort;
    ptIndex->bSchema    = bSchema;
    zte_memcpy_s(&(ptIndex->tNwdafIP), sizeof(T_IPComm), tNwdafIP, sizeof(T_IPComm));

    _pDM_PFU_QUERYDYNDATA_BYIDX(&tQueryReq, &tQueryAck);

    if((RC_OK == tQueryAck.retCODE) && (NULL != tQueryAck.pDataAreaAddr))
    {
        return (T_psNcuHttpLink *)tQueryAck.pDataAreaAddr;
    }
    return NULL;
}

T_psNcuHttpLink* psCreateHttpLinkByUniqueIdx(T_IPComm *tNwdafIP, WORD32 dwClientProfileID, WORD16 wNwdafPort, BYTE bSchema, WORD32 hDB)
{
    _DB_STATEMENT_TRUE_RTN_VALUE((NULL == tNwdafIP), NULL);

    DM_PFU_ALLOCDYNDATA_BYIDX_REQ tAllocReq = {0};
    DM_PFU_ALLOCDYNDATA_BYIDX_ACK tAllocAck = {0};

    r_ncu_httplink_idx_tuple *ptIndex = NULL;

    tAllocReq.hDB  = hDB; /* 库句柄待定 */
    tAllocReq.hTbl = DB_HANDLE_R_NCU_HTTPLINK;
    tAllocReq.dataAreaKey.ucKeyFlag = XDB_KEYFLAG_UNIQUE;
    tAllocReq.dataAreaKey.hIdx      = DB_HANDLE_IDX_R_NCU_HTTPLINK;
    tAllocReq.dataAreaKey.ucKeyLen  = sizeof(r_ncu_httplink_idx_tuple);
    ptIndex = (r_ncu_httplink_idx_tuple *)tAllocReq.dataAreaKey.aucKey;
    ptIndex->dwClientProfileID = dwClientProfileID;
    ptIndex->wNwdafPort = wNwdafPort;
    ptIndex->bSchema = bSchema;
    zte_memcpy_s(&(ptIndex->tNwdafIP), sizeof(T_IPComm), tNwdafIP, sizeof(T_IPComm));

    _pDM_PFU_ALLOCDYNDATA_BYIDX(&tAllocReq, &tAllocAck);
    if((RC_OK == tAllocAck.retCODE || RC_EXIST == tAllocAck.retCODE) && (NULL != tAllocAck.ptDataAreaAddr))
    {
        T_psNcuHttpLink *ptNcuHttpLinkCtx = (T_psNcuHttpLink *)tAllocAck.ptDataAreaAddr;

        zte_memcpy_s(&(ptNcuHttpLinkCtx->tNwdafIP), sizeof(T_IPComm), tNwdafIP, sizeof(T_IPComm));
        ptNcuHttpLinkCtx->wNwdafPort = wNwdafPort;
        ptNcuHttpLinkCtx->dwClientProfileID = dwClientProfileID;
        ptNcuHttpLinkCtx->bSchema = bSchema;
        ptNcuHttpLinkCtx->remoteStatus = HTTP_REMOTE_VALID;
        ptNcuHttpLinkCtx->tHttpLbJid.dwJno = 0;
        ptNcuHttpLinkCtx->tHttpLbJid.dwComId = INVALID_COMM_ID;
        ptNcuHttpLinkCtx->dwUpdateTimeStamp = psFtmGetPowerOnSec();
        return ptNcuHttpLinkCtx;
    }
    return NULL;
}

WORD32 psDelHttpLinkByUniqueIdx(T_IPComm *tNwdafIP, WORD32 dwClientProfileID, WORD16 wNwdafPort, BYTE bSchema, WORD32 hDB)
{
    _DB_STATEMENT_TRUE_RTN_VALUE((NULL == tNwdafIP), MCS_RET_FAIL);

    DM_PFU_RELEASEDYNDATA_BYIDX_REQ tReleaseReq = {0};
    DM_PFU_RELEASEDYNDATA_BYIDX_ACK tReleaseAck = {0};

    r_ncu_httplink_idx_tuple *ptIndex = NULL;

    tReleaseReq.hDB         = hDB;
    tReleaseReq.hTbl        = DB_HANDLE_R_NCU_HTTPLINK;
    tReleaseReq.dataAreaKey.ucKeyFlag = XDB_KEYFLAG_UNIQUE;
    tReleaseReq.dataAreaKey.hIdx      = DB_HANDLE_IDX_R_NCU_HTTPLINK;
    tReleaseReq.dataAreaKey.ucKeyLen  = sizeof(r_ncu_httplink_idx_tuple);
    ptIndex = (r_ncu_httplink_idx_tuple *)tReleaseReq.dataAreaKey.aucKey;
    ptIndex->dwClientProfileID = dwClientProfileID;
    ptIndex->wNwdafPort = wNwdafPort;
    ptIndex->bSchema = bSchema;
    zte_memcpy_s(&(ptIndex->tNwdafIP), sizeof(T_IPComm), tNwdafIP, sizeof(T_IPComm));

    tReleaseAck.retCODE = RC_ERROR;

    _pDM_PFU_RELEASEDYNDATA_BYIDX(&tReleaseReq, &tReleaseAck);
    if(RC_OK == tReleaseAck.retCODE)
    {
        return MCS_RET_SUCCESS;
    }

    return MCS_RET_FAIL;
}
