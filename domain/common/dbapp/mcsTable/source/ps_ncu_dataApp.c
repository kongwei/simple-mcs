#include "ps_ncu_dataApp.h"
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
#include "ncu_capacity.h"

void r_ncu_data_app_UIdxMerge(void* pTuple, void* pKey,DWORD dwKeyLen)
{
    lp_r_ncu_dataapp_idx_tuple lpTmp = (lp_r_ncu_dataapp_idx_tuple)pKey;
    lp_r_ncu_dataapp_tuple pAddr = (lp_r_ncu_dataapp_tuple)pTuple;
    lpTmp->ddwUPSeid     = pAddr->ddwUPSeid;
    lpTmp->dwSubAppid    = pAddr->dwSubAppid;
    return;
}

void r_ncu_data_app_UIdxCpy(void* pTuple, void* pKey,DWORD dwKeyLen)
{
    lp_r_ncu_dataapp_idx_tuple lpTmp = (lp_r_ncu_dataapp_idx_tuple)pKey;
    lp_r_ncu_dataapp_tuple pAddr = (lp_r_ncu_dataapp_tuple)pTuple;
    pAddr->ddwUPSeid     = lpTmp->ddwUPSeid;
    pAddr->dwSubAppid    = lpTmp->dwSubAppid;
    return;
}

BOOLEAN r_ncu_data_app_UIdxCmp(void* pTuple, void* pKey,  DWORD dwKeyLen)
{
    lp_r_ncu_dataapp_idx_tuple lpTmp = (lp_r_ncu_dataapp_idx_tuple)pKey;
    lp_r_ncu_dataapp_tuple lpAddr = (lp_r_ncu_dataapp_tuple )pTuple;

    return ((lpTmp->ddwUPSeid == lpAddr->ddwUPSeid) && (lpTmp->dwSubAppid == lpAddr->dwSubAppid));
}

void r_ncu_data_app_NoUIdxMerge(void* pTuple, void* pKey,DWORD dwKeyLen)
{
    lp_r_ncu_dataapp_nonidx_tuple lpTmp  = (lp_r_ncu_dataapp_nonidx_tuple)pKey;
    lp_r_ncu_dataapp_tuple lpAddr        = (lp_r_ncu_dataapp_tuple )pTuple;

    lpTmp->ddwUPSeid    = lpAddr->ddwUPSeid;
    lpTmp->dwAppid      = lpAddr->dwAppid;
    return;
}

void r_ncu_data_app_NoUIdxCpy(void* pTuple, void* pKey,DWORD dwKeyLen)
{
    lp_r_ncu_dataapp_nonidx_tuple lpTmp  = (lp_r_ncu_dataapp_nonidx_tuple)pKey;
    lp_r_ncu_dataapp_tuple lpAddr           = (lp_r_ncu_dataapp_tuple )pTuple;
    lpAddr->ddwUPSeid = lpTmp->ddwUPSeid;
    lpAddr->dwAppid   = lpTmp->dwAppid;
    return;
}

void r_ncu_ana_clockstep_timerid_IdxMerge(void* pTuple, void* pKey,  DWORD dwKeyLen)
{
    lp_r_ncu_dataapp_nonidx_AnaAClockStep_tuple lpTmp  = (lp_r_ncu_dataapp_nonidx_AnaAClockStep_tuple)pKey;
    lp_r_ncu_dataapp_tuple lpAddr           = (lp_r_ncu_dataapp_tuple )pTuple;

    lpTmp->ddwAnaClockStep    = lpAddr->ddwAnaClockStep;
    return;
}

void r_ncu_ana_clockstep_timerid_IdxCpy(void* pTuple, void* pKey,DWORD dwKeyLen)
{
    lp_r_ncu_dataapp_nonidx_AnaAClockStep_tuple lpTmp  =  (lp_r_ncu_dataapp_nonidx_AnaAClockStep_tuple)pKey;
    lp_r_ncu_dataapp_tuple lpAddr           = (lp_r_ncu_dataapp_tuple )pTuple;

    lpAddr->ddwAnaClockStep    = lpTmp->ddwAnaClockStep;
    return;
}
void r_ncu_exp_clockstep_timerid_IdxMerge(void* pTuple, void* pKey,  DWORD dwKeyLen)
{
    lp_r_ncu_dataapp_nonidx_ExpAClockStep_tuple lpTmp  = (lp_r_ncu_dataapp_nonidx_ExpAClockStep_tuple)pKey;
    lp_r_ncu_dataapp_tuple lpAddr           = (lp_r_ncu_dataapp_tuple )pTuple;

    lpTmp->ddwExpClockStep    = lpAddr->ddwExpClockStep;
    return;
}

void r_ncu_exp_clockstep_timerid_IdxCpy(void* pTuple, void* pKey,DWORD dwKeyLen)
{
    lp_r_ncu_dataapp_nonidx_ExpAClockStep_tuple lpTmp  =  (lp_r_ncu_dataapp_nonidx_ExpAClockStep_tuple)pKey;
    lp_r_ncu_dataapp_tuple lpAddr           = (lp_r_ncu_dataapp_tuple )pTuple;

    lpAddr->ddwExpClockStep    = lpTmp->ddwExpClockStep;
    return;
}

void r_ncu_ana_mon_clockstep_timerid_IdxMerge(void* pTuple, void* pKey,  DWORD dwKeyLen)
{
    lp_r_ncu_dataapp_nonidx_AnaMonClockStep_tuple lpTmp  = (lp_r_ncu_dataapp_nonidx_AnaMonClockStep_tuple)pKey;
    lp_r_ncu_dataapp_tuple lpAddr           = (lp_r_ncu_dataapp_tuple )pTuple;

    lpTmp->ddwAnaMonClockStep    = lpAddr->ddwAnaMonClockStep;
    return;
}

void r_ncu_ana_mon_clockstep_timerid_IdxCpy(void* pTuple, void* pKey,DWORD dwKeyLen)
{
    lp_r_ncu_dataapp_nonidx_AnaMonClockStep_tuple lpTmp  =  (lp_r_ncu_dataapp_nonidx_AnaMonClockStep_tuple)pKey;
    lp_r_ncu_dataapp_tuple lpAddr           = (lp_r_ncu_dataapp_tuple )pTuple;

    lpAddr->ddwAnaMonClockStep    = lpTmp->ddwAnaMonClockStep;
    return;
}

BOOLEAN r_ncu_data_app_ReleaseRsc(LPT_PfuDataBase pPfuDbReg, LPT_PfuTableReg pPfuTableReg, DWORD dwTupleNo)
{
    _DB_STATEMENT_TRUE_RTN_VALUE((NULL == pPfuDbReg), FALSE);
    LPT_PfuUniIdxReg    ptPfuUniIdxReg = NULL;

    BOOLEAN dbRet = FALSE;
    
    T_psNcuDaAppCtx  *ptDaAppCtx = psMcsGetDaAppCtxById(dwTupleNo, pPfuDbReg->hDB);
    _DB_STATEMENT_TRUE_RTN_VALUE((NULL == ptDaAppCtx), FALSE);

    ptPfuUniIdxReg = xdb_Pfu_Get_UniIdxReg_ByDbReg(pPfuDbReg, DB_HANDLE_IDX_R_NCU_DATAAPP_UPSEID_SUBAPPID);
    _DB_STATEMENT_TRUE_RTN_VALUE((NULL == ptPfuUniIdxReg), FALSE);
    xdb_pfu_delete_UniIdx_by_tupleNo(pPfuTableReg, ptPfuUniIdxReg, dwTupleNo);

    
    LPT_PfuNoUniIdxReg  ptPfuNoUniIdxReg2 = NULL;
    BYTE                aucTupleKey[XDB_IDXKEYLEN_MAX] = {0};
    ptPfuNoUniIdxReg2 = xdb_Pfu_Get_NoUniIdxReg_ByDbReg(pPfuDbReg, DB_HANDLE_IDX_R_NCU_DATAAPP_UPSEID_APPID);
    _DB_STATEMENT_TRUE_RTN_VALUE((NULL == ptPfuNoUniIdxReg2), FALSE);
    xdb_pfu_nunidx_merge_by_tupleno(pPfuTableReg, ptPfuNoUniIdxReg2, aucTupleKey, dwTupleNo);
    xdb_pfu_delete_nunidx(pPfuTableReg, ptPfuNoUniIdxReg2, aucTupleKey, dwTupleNo);

    ptPfuNoUniIdxReg2 = xdb_Pfu_Get_NoUniIdxReg_ByDbReg(pPfuDbReg, DB_HANDLE_IDX_R_NCU_DATAAPP_ANA_CLOCK_STEP);
    _DB_STATEMENT_TRUE_RTN_VALUE((NULL == ptPfuNoUniIdxReg2), FALSE);
    xdb_pfu_nunidx_merge_by_tupleno(pPfuTableReg, ptPfuNoUniIdxReg2, aucTupleKey, dwTupleNo);
    xdb_pfu_delete_nunidx(pPfuTableReg, ptPfuNoUniIdxReg2, aucTupleKey, dwTupleNo);

    ptPfuNoUniIdxReg2 = xdb_Pfu_Get_NoUniIdxReg_ByDbReg(pPfuDbReg, DB_HANDLE_IDX_R_NCU_DATAAPP_EXP_CLOCK_STEP);
    _DB_STATEMENT_TRUE_RTN_VALUE((NULL == ptPfuNoUniIdxReg2), FALSE);
    xdb_pfu_nunidx_merge_by_tupleno(pPfuTableReg, ptPfuNoUniIdxReg2, aucTupleKey, dwTupleNo);
    xdb_pfu_delete_nunidx(pPfuTableReg, ptPfuNoUniIdxReg2, aucTupleKey, dwTupleNo);

    ptPfuNoUniIdxReg2 = xdb_Pfu_Get_NoUniIdxReg_ByDbReg(pPfuDbReg, DB_HANDLE_IDX_R_NCU_DATAAPP_ANA_MON_CLOCK_STEP);
    _DB_STATEMENT_TRUE_RTN_VALUE((NULL == ptPfuNoUniIdxReg2), FALSE);
    xdb_pfu_nunidx_merge_by_tupleno(pPfuTableReg, ptPfuNoUniIdxReg2, aucTupleKey, dwTupleNo);
    xdb_pfu_delete_nunidx(pPfuTableReg, ptPfuNoUniIdxReg2, aucTupleKey, dwTupleNo);

    dbRet = xdb_Delete_Tuple(pPfuTableReg,dwTupleNo);
    _DB_STATEMENT_TRUE_RTN_VALUE((FALSE == dbRet), FALSE);
    return TRUE;
}


/* 临时设置，后续需要查capacity表*/
WORD32 _db_get_ncu_data_app_capacity()
{
#ifndef FT_TEST
    return _db_get_ncu_capacity_by_tblName(NCCTXNAME_R_NCU_DATA_APP, R_NCU_DATA_APP);
#else
    return 20;
#endif
}
BOOL create_r_ncu_dataApp(DWORD dwDbHandle)
{
    DWORD dwDynTblCapacity = 0;
    LPT_PfuTableReg      pTableReg = NULL;
    LPT_PfuUniIdxReg     pPfuUniIdxReg = NULL;
    LPT_PfuNoUniIdxReg   pPfuNoUniIdxReg    = NULL;

    dwDynTblCapacity = _db_get_ncu_data_app_capacity();
    zte_printf_s("ncu data app capacity: %u\n", dwDynTblCapacity);
    _DB_STATEMENT_TRUE_RTN_VALUE((0 == dwDynTblCapacity), FALSE);

    /* 创建表 */
    XOS_SysLog(LOG_EMERGENCY, "DB Module: create table r_ncu_dataapp start!\n");
    pTableReg = XDB_PFU_CREATE_TABLE(dwDbHandle, DB_HANDLE_R_NCUDATAAPP, R_NCU_DATA_APP, 
                                     dwDynTblCapacity, sizeof(T_psNcuDaAppCtx), 
                                     sizeof(r_ncu_dataapp_tuple));
    if (NULL == pTableReg)
    {
        XOS_SysLog(LOG_EMERGENCY, "[File:%s],[line:%d]DB Module:create table r_ncu_dataApp failed!\n", __FILE__, __LINE__);
        return FALSE;
    }

    /* 创建索引 */
    pPfuUniIdxReg = XDB_PFU_CREATE_UNIIDX(dwDbHandle, DB_HANDLE_IDX_R_NCU_DATAAPP_UPSEID_SUBAPPID, XDB_UNIDX_NORMARL,
                                          dwDynTblCapacity, XDB_MOD_TABLE(dwDynTblCapacity),
                                          sizeof(r_ncu_dataapp_idx_tuple));
    if (NULL == pPfuUniIdxReg)
    {
        XOS_SysLog(LOG_EMERGENCY, "[File:%s],[line:%d]DB Module: create index DB_HANDLE_IDX_R_NCU_SUBSCRIBE_UPSEID_APPID failed!\n", __FILE__, __LINE__);
        return FALSE;
    }
    xdb_pfu_UniIdx_Cpy_Reg(pPfuUniIdxReg, r_ncu_data_app_UIdxCpy);
    xdb_pfu_UniIdx_Merge_Reg(pPfuUniIdxReg, r_ncu_data_app_UIdxMerge);
    xdb_pfu_UniIdx_Cmp_Reg(pPfuUniIdxReg, r_ncu_data_app_UIdxCmp);


    /* 创建非唯一索引 */
    pPfuNoUniIdxReg = XDB_PFU_CREATE_NUNIDX(dwDbHandle, DB_HANDLE_IDX_R_NCU_DATAAPP_UPSEID_APPID,
                                          dwDynTblCapacity, XDB_MOD_TABLE(dwDynTblCapacity),
                                          sizeof(r_ncu_dataapp_nonidx_tuple));
    if (NULL == pPfuNoUniIdxReg)
    {
        XOS_SysLog(LOG_EMERGENCY, "[File:%s],[line:%d]DB Module: create index DB_HANDLE_IDX_R_NCU_DATAAPP_UPSEID_APPID failed!\n", __FILE__, __LINE__);
        return FALSE;
    }
    xdb_pfu_nunidx_cpy_reg(pPfuNoUniIdxReg, r_ncu_data_app_NoUIdxCpy);
    xdb_pfu_nunidx_merge_reg(pPfuNoUniIdxReg, r_ncu_data_app_NoUIdxMerge);

    /* 创建非唯一索引 */
    pPfuNoUniIdxReg = XDB_PFU_CREATE_NUNIDX(dwDbHandle, DB_HANDLE_IDX_R_NCU_DATAAPP_ANA_CLOCK_STEP,
                                          dwDynTblCapacity, XDB_MOD_TABLE(dwDynTblCapacity),
                                          sizeof(r_ncu_dataapp_nonidx_AnaAClockStep_tuple));
    if (NULL == pPfuNoUniIdxReg)
    {
        XOS_SysLog(LOG_EMERGENCY, "[File:%s],[line:%d]DB Module: create index DB_HANDLE_IDX_R_NCU_DATAAPP_ANA_CLOCK_STEP failed!\n", __FILE__, __LINE__);
        return FALSE;
    }
    xdb_pfu_nunidx_cpy_reg(pPfuNoUniIdxReg, r_ncu_ana_clockstep_timerid_IdxCpy);
    xdb_pfu_nunidx_merge_reg(pPfuNoUniIdxReg, r_ncu_ana_clockstep_timerid_IdxMerge);

    /* 创建非唯一索引 */
    pPfuNoUniIdxReg = XDB_PFU_CREATE_NUNIDX(dwDbHandle, DB_HANDLE_IDX_R_NCU_DATAAPP_EXP_CLOCK_STEP,
                                          dwDynTblCapacity, XDB_MOD_TABLE(dwDynTblCapacity),
                                          sizeof(r_ncu_dataapp_nonidx_ExpAClockStep_tuple));
    if (NULL == pPfuNoUniIdxReg)
    {
        XOS_SysLog(LOG_EMERGENCY, "[File:%s],[line:%d]DB Module: create index DB_HANDLE_IDX_R_NCU_DATAAPP_ANA_CLOCK_STEP failed!\n", __FILE__, __LINE__);
        return FALSE;
    }
    xdb_pfu_nunidx_cpy_reg(pPfuNoUniIdxReg, r_ncu_exp_clockstep_timerid_IdxCpy);
    xdb_pfu_nunidx_merge_reg(pPfuNoUniIdxReg, r_ncu_exp_clockstep_timerid_IdxMerge);


    /* 创建非唯一索引 - 质差监控时间片*/
    pPfuNoUniIdxReg = XDB_PFU_CREATE_NUNIDX(dwDbHandle, DB_HANDLE_IDX_R_NCU_DATAAPP_ANA_MON_CLOCK_STEP,
                                          dwDynTblCapacity, XDB_MOD_TABLE(dwDynTblCapacity),
                                          sizeof(r_ncu_dataapp_nonidx_AnaMonClockStep_tuple));
    if (NULL == pPfuNoUniIdxReg)
    {
        XOS_SysLog(LOG_EMERGENCY, "[File:%s],[line:%d]DB Module: create index DB_HANDLE_IDX_R_NCU_DATAAPP_ANA_MON_CLOCK_STEP failed!\n", __FILE__, __LINE__);
        return FALSE;
    }
    xdb_pfu_nunidx_cpy_reg(pPfuNoUniIdxReg, r_ncu_ana_mon_clockstep_timerid_IdxCpy);
    xdb_pfu_nunidx_merge_reg(pPfuNoUniIdxReg, r_ncu_ana_mon_clockstep_timerid_IdxMerge);

    xdb_Set_ReleaseRsc_Overload(pTableReg,r_ncu_data_app_ReleaseRsc);
    XOS_SysLog(LOG_EMERGENCY, "DB Module: create r_ncu_dataapp table and index success!\n");
    return TRUE;
}

T_psNcuDaAppCtx *psMcsGetDaAppCtxById(WORD32 dwCtxId, WORD32 hDB)
{
    DM_PFU_QUERYDYNDATA_BYTUPLENO_REQ tQueryReq = {0}; /* 查询请求 */
    DM_PFU_QUERYDYNDATA_BYTUPLENO_ACK tQueryAck = {0}; /* 查询应答 */
    _DB_STATEMENT_TRUE_RTN_VALUE((0 == dwCtxId), NULL);
    tQueryReq.hDB               = hDB; /* 库句柄待定 */
    tQueryReq.hTbl              = DB_HANDLE_R_NCUDATAAPP;
    tQueryReq.dwDataAreaIndex   = dwCtxId;
    _pDM_PFU_QUERYDYNDATA_BYTUPLENO(&tQueryReq, &tQueryAck);
    _DB_STATEMENT_TRUE_RTN_VALUE((RC_OK != tQueryAck.retCODE), NULL);
    return (T_psNcuDaAppCtx*)tQueryAck.ptDataAreaAddr;/*返回数据区指针 */
}

T_psNcuDaAppCtx*  psQueryDaAppCtxByUpseidSubAppid(WORD64 ddwUPSeid, WORD32 dwSubAppid, WORD32 hDB)
{
    DM_PFU_QUERYDYNDATA_BYIDX_REQ tQueryReq = {0}; /* 查询请求 */
    DM_PFU_QUERYDYNDATA_BYIDX_ACK tQueryAck = {0}; /* 查询应答 */

    tQueryReq.hDB         = hDB; /* 库句柄待定 */
    tQueryReq.hTbl        = DB_HANDLE_R_NCUDATAAPP;
    tQueryReq.dwExpectNum = 1;
    tQueryReq.dataAreaKey.ucKeyFlag = XDB_KEYFLAG_UNIQUE;
    tQueryReq.dataAreaKey.hIdx      = DB_HANDLE_IDX_R_NCU_DATAAPP_UPSEID_SUBAPPID;
    tQueryReq.dataAreaKey.ucKeyLen  = sizeof(WORD64)+sizeof(WORD32);

    CSS_MEMCPY(tQueryReq.dataAreaKey.aucKey,&ddwUPSeid,sizeof(WORD64));
    CSS_MEMCPY(((BYTE*)tQueryReq.dataAreaKey.aucKey)+sizeof(WORD64),&dwSubAppid,sizeof(WORD32));
    _pDM_PFU_QUERYDYNDATA_BYIDX(&tQueryReq,&tQueryAck);

    if((RC_OK == tQueryAck.retCODE)&&(NULL != tQueryAck.pDataAreaAddr))
    {
        return (T_psNcuDaAppCtx *)tQueryAck.pDataAreaAddr;
    }
    return NULL;
}
T_psNcuDaAppCtx*  psCrtDaAppCtxByUpseidSubAppid(WORD64 ddwUPSeid, WORD32 dwSubAppid, WORD32 hDB)
{
    DM_PFU_ALLOCDYNDATA_BYIDX_REQ tAllocReq = {0};
    DM_PFU_ALLOCDYNDATA_BYIDX_ACK tAllocAck = {0};

    tAllocReq.hDB  = hDB; /* 库句柄待定 */
    tAllocReq.hTbl = DB_HANDLE_R_NCUDATAAPP;
    tAllocReq.dataAreaKey.ucKeyFlag = XDB_KEYFLAG_UNIQUE;
    tAllocReq.dataAreaKey.hIdx      = DB_HANDLE_IDX_R_NCU_DATAAPP_UPSEID_SUBAPPID;
    tAllocReq.dataAreaKey.ucKeyLen  = sizeof(WORD64)+sizeof(WORD32);

    CSS_MEMCPY(tAllocReq.dataAreaKey.aucKey,&ddwUPSeid,sizeof(WORD64));
    CSS_MEMCPY(((BYTE*)tAllocReq.dataAreaKey.aucKey)+sizeof(WORD64),&dwSubAppid,sizeof(WORD32));

    _pDM_PFU_ALLOCDYNDATA_BYIDX(&tAllocReq, &tAllocAck);
    if((RC_OK == tAllocAck.retCODE || RC_EXIST == tAllocAck.retCODE) && (NULL != tAllocAck.ptDataAreaAddr))
    {
        T_psNcuDaAppCtx  *ptDaAppCtx = (T_psNcuDaAppCtx *)tAllocAck.ptDataAreaAddr;
        ptDaAppCtx->dwID = tAllocAck.dwDataAreaIndex;
        ptDaAppCtx->ddwUPSeid = ddwUPSeid;
        return ptDaAppCtx;
    }
    return NULL;
}
WORD32  psDelDaAppCtxByUpseidSubAppid(WORD64 ddwUPSeid,WORD32 dwSubAppid, WORD32 hDB)
{
    DM_PFU_RELEASEDYNDATA_BYIDX_REQ tReleaseReq = {0};
    DM_PFU_RELEASEDYNDATA_BYIDX_ACK tReleaseAck = {0};

    tReleaseReq.hDB         = hDB;
    tReleaseReq.hTbl        = DB_HANDLE_R_NCUDATAAPP;
    tReleaseReq.dataAreaKey.ucKeyFlag = XDB_KEYFLAG_UNIQUE;
    tReleaseReq.dataAreaKey.hIdx      = DB_HANDLE_IDX_R_NCU_DATAAPP_UPSEID_SUBAPPID;
    tReleaseReq.dataAreaKey.ucKeyLen  = sizeof(WORD64)+sizeof(WORD32);

    zte_memcpy_s(&tReleaseReq.dataAreaKey.aucKey,sizeof(WORD64),&ddwUPSeid,sizeof(WORD64));
    zte_memcpy_s(((BYTE*)tReleaseReq.dataAreaKey.aucKey)+sizeof(WORD64),sizeof(WORD32),&dwSubAppid,sizeof(WORD32));

    tReleaseAck.retCODE = RC_ERROR;

    _pDM_PFU_RELEASEDYNDATA_BYIDX(&tReleaseReq, &tReleaseAck);
    if(RC_OK == tReleaseAck.retCODE)
    {
        return MCS_RET_SUCCESS;
    }

    return MCS_RET_FAIL;
}

inline WORD32 psNcuMcsUpdDaAppCtxByAppid(WORD64 ddwUPSeid,WORD32 dwAppid, WORD32 dwCtxId,WORD32 hDB)
{
    DM_PFU_UPDATEDYNDATA_REQ tReq = {0};
    DM_PFU_UPDATEDYNDATA_ACK tAck = {0};

    tReq.hDB = hDB;
    tReq.hTbl= DB_HANDLE_R_NCUDATAAPP;
    tReq.ucUpdateMethodFlag = 0;
    tReq.dwDataAreaIndex = dwCtxId;
    tReq.dataAreaKey.ucKeyLen  = sizeof(WORD64)+sizeof(WORD32);
    tReq.dataAreaKey.ucKeyFlag = XDB_KEYFLAG_NOUNIQUE;
    tReq.dataAreaKey.hIdx = DB_HANDLE_IDX_R_NCU_DATAAPP_UPSEID_APPID;

    zte_memcpy_s(&tReq.dataAreaKey.aucKey,sizeof(WORD64),&ddwUPSeid,sizeof(WORD64));
    zte_memcpy_s(((BYTE*)tReq.dataAreaKey.aucKey)+sizeof(WORD64),sizeof(WORD32),&dwAppid,sizeof(WORD32));

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


inline WORD32 psNcuMcsUpdDaAppCtxByClockStep(WORD64 ddwClockStep, WORD32 hIdx,WORD32 dwCtxId,WORD32 hDB, BYTE method)
{
    DM_PFU_UPDATEDYNDATA_REQ tReq = {0};
    DM_PFU_UPDATEDYNDATA_ACK tAck = {0};

    tReq.hDB = hDB;
    tReq.hTbl= DB_HANDLE_R_NCUDATAAPP;
    tReq.ucUpdateMethodFlag = method;
    tReq.dwDataAreaIndex = dwCtxId;
    tReq.dataAreaKey.ucKeyLen  = sizeof(WORD64);
    tReq.dataAreaKey.ucKeyFlag = XDB_KEYFLAG_NOUNIQUE;
    tReq.dataAreaKey.hIdx = hIdx;

    zte_memcpy_s(&tReq.dataAreaKey.aucKey,sizeof(WORD64),&ddwClockStep,sizeof(WORD64));

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


WORD32 psVpfuMcsGetAllDaAppCtxByAppid(WORD64 ddwUPSeid,WORD32 dwAppid,WORD32 hDB,BYTE *pbQueryAckBuff)
{
    DM_PFU_QUERYDYNDATA_BYIDX_REQ  tQueryReq = {0}; /* 查询请求 */
    DM_PFU_QUERYDYNDATA_BYIDX_ACK *ptQueryAck = (DM_PFU_QUERYDYNDATA_BYIDX_ACK *)pbQueryAckBuff; /* 查询应答 */
    WORD32 dwCtxNum = 0;

    tQueryReq.hDB         = hDB; /* 库句柄待定 */
    tQueryReq.hTbl        = DB_HANDLE_R_NCUDATAAPP;
    tQueryReq.dwExpectNum = EXPECT_DATA_APP_NUM;
    tQueryReq.dataAreaKey.ucKeyFlag = XDB_KEYFLAG_NOUNIQUE;
    tQueryReq.dataAreaKey.hIdx      = DB_HANDLE_IDX_R_NCU_DATAAPP_UPSEID_APPID;
    tQueryReq.dataAreaKey.ucKeyLen  = sizeof(WORD64) + sizeof(WORD32);

    zte_memcpy_s(&tQueryReq.dataAreaKey.aucKey,sizeof(WORD64),&ddwUPSeid,sizeof(WORD64));
    zte_memcpy_s(((BYTE*)tQueryReq.dataAreaKey.aucKey)+sizeof(WORD64),sizeof(WORD32),&dwAppid,sizeof(WORD32));
    _pDM_PFU_QUERYDYNDATA_BYIDX(&tQueryReq,ptQueryAck);

    if(RC_OK == ptQueryAck->retCODE)
    {
        dwCtxNum = ptQueryAck->dwDataAreaNum;
        return dwCtxNum;/* 上下文个数 */
    }
    return dwCtxNum;
}

inline void psNcuMcsResetDaAppCtxByClockStep(WORD64 ddwNewClockStep, WORD32 dwhIdx,WORD32 dwCtxId,WORD32 dwhDB)
{
    psNcuMcsUpdDaAppCtxByClockStep(0, dwhIdx,dwCtxId, dwhDB, enumDeleteIndex);
    psNcuMcsUpdDaAppCtxByClockStep(ddwNewClockStep, dwhIdx,dwCtxId, dwhDB, enumUpdateIndex);
}
WORD32 psVpfuMcsGetAllDaAppCtxByClockStep(WORD64 ddwClockStep,WORD32 hDB, WORD32 hIdx, BYTE *pbQueryAckBuff)
{
    DM_PFU_QUERYDYNDATA_BYIDX_REQ  tQueryReq = {0}; /* 查询请求 */
    DM_PFU_QUERYDYNDATA_BYIDX_ACK *ptQueryAck = (DM_PFU_QUERYDYNDATA_BYIDX_ACK *)pbQueryAckBuff; /* 查询应答 */
    WORD32 dwCtxNum = 0;

    tQueryReq.hDB         = hDB; /* 库句柄待定 */
    tQueryReq.hTbl        = DB_HANDLE_R_NCUDATAAPP;
    tQueryReq.dwExpectNum = EXPECT_DATA_APP_NUM;
    tQueryReq.dataAreaKey.ucKeyFlag = XDB_KEYFLAG_NOUNIQUE;
    tQueryReq.dataAreaKey.hIdx      = hIdx;
    tQueryReq.dataAreaKey.ucKeyLen  = sizeof(WORD64);

    zte_memcpy_s(&tQueryReq.dataAreaKey.aucKey,sizeof(WORD64),&ddwClockStep,sizeof(WORD64));
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
