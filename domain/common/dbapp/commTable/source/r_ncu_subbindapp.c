#include "r_ncu_subbindapp.h"
#include "ps_db_define_ncu.h"
#include "xdb_core_pfu.h"
#include "xdb_pfu_com.h"
#include "xdb_core_pfu_db.h"
#include "xdb_core_pfu_table.h"
#include "xdb_core_uniindex.h"
#include "xdb_core_pfu_nunidx.h"
#include "xdb_core_pfu_tblimp_api.h"
#include "ps_mcs_define.h"
#include "zte_slibc.h"
#include "ncu_capacity.h"
#include "r_ncu_applicationmap.h"
#include "psMcsDebug.h"
#include "r_subbindapp.h"
VOID psNcuGetSubBindAppAllCfg();
T_psNcuAppidRelation *psMcsGetAppidRelateCtxById(WORD32 dwCtxId, WORD32 hDB);
void r_ncu_appid_relation_UIdxMerge(void* pTuple, void* pKey,DWORD dwKeyLen)
{
    lp_r_ncu_appid_relation_idx_tuple lpTmp = (lp_r_ncu_appid_relation_idx_tuple)pKey;
    lp_r_ncu_appid_relation_tuple pAddr = (lp_r_ncu_appid_relation_tuple)pTuple;
    lpTmp->dwSubAppid    = pAddr->dwSubAppid;
    return;
}

void r_ncu_appid_relation_UIdxCpy(void* pTuple, void* pKey,DWORD dwKeyLen)
{
    lp_r_ncu_appid_relation_idx_tuple lpTmp = (lp_r_ncu_appid_relation_idx_tuple)pKey;
    lp_r_ncu_appid_relation_tuple pAddr = (lp_r_ncu_appid_relation_tuple)pTuple;
    pAddr->dwSubAppid    = lpTmp->dwSubAppid;
    return;
}

BOOLEAN r_ncu_appid_relation_UIdxCmp(void* pTuple, void* pKey,  DWORD dwKeyLen)
{
    lp_r_ncu_appid_relation_idx_tuple lpTmp = (lp_r_ncu_appid_relation_idx_tuple)pKey;
    lp_r_ncu_appid_relation_tuple lpAddr = (lp_r_ncu_appid_relation_tuple )pTuple;

    return (lpTmp->dwSubAppid == lpAddr->dwSubAppid);
}

void r_ncu_appid_relation_NoUIdxMerge(void* pTuple, void* pKey,DWORD dwKeyLen)
{
    lp_r_ncu_appid_relation_nonidx_tuple lpTmp  = (lp_r_ncu_appid_relation_nonidx_tuple)pKey;
    lp_r_ncu_appid_relation_tuple lpAddr        = (lp_r_ncu_appid_relation_tuple )pTuple;

    lpTmp->dwAppid      = lpAddr->dwAppid;
    return;
}

void r_ncu_appid_relation_NoUIdxCpy(void* pTuple, void* pKey,DWORD dwKeyLen)
{
    lp_r_ncu_appid_relation_nonidx_tuple lpTmp  = (lp_r_ncu_appid_relation_nonidx_tuple)pKey;
    lp_r_ncu_appid_relation_tuple lpAddr           = (lp_r_ncu_appid_relation_tuple )pTuple;
    lpAddr->dwAppid   = lpTmp->dwAppid;
    return;
}

BOOLEAN r_ncu_appid_relate_ReleaseRsc(LPT_PfuDataBase pPfuDbReg, LPT_PfuTableReg pPfuTableReg, DWORD dwTupleNo)
{
    _DB_STATEMENT_TRUE_RTN_VALUE((NULL == pPfuDbReg), FALSE);
    LPT_PfuUniIdxReg    ptPfuUniIdxReg = NULL;

    BOOLEAN dbRet = FALSE;
    
    T_psNcuAppidRelation  *ptAppidRelateCtx = psMcsGetAppidRelateCtxById(dwTupleNo, pPfuDbReg->hDB);
    _DB_STATEMENT_TRUE_RTN_VALUE((NULL == ptAppidRelateCtx), FALSE);

    ptPfuUniIdxReg = xdb_Pfu_Get_UniIdxReg_ByDbReg(pPfuDbReg, DB_HANDLE_IDX_R_NCU_APPID_RELATE_SUBAPPID);
    _DB_STATEMENT_TRUE_RTN_VALUE((NULL == ptPfuUniIdxReg), FALSE);
    xdb_pfu_delete_UniIdx_by_tupleNo(pPfuTableReg, ptPfuUniIdxReg, dwTupleNo);

    
    LPT_PfuNoUniIdxReg  ptPfuNoUniIdxReg2 = NULL;
    BYTE                aucTupleKey[XDB_IDXKEYLEN_MAX] = {0};
    ptPfuNoUniIdxReg2 = xdb_Pfu_Get_NoUniIdxReg_ByDbReg(pPfuDbReg, DB_HANDLE_IDX_R_NCU_APPID_RELATE_APPID);
    _DB_STATEMENT_TRUE_RTN_VALUE((NULL == ptPfuNoUniIdxReg2), FALSE);
    xdb_pfu_nunidx_merge_by_tupleno(pPfuTableReg, ptPfuNoUniIdxReg2, aucTupleKey, dwTupleNo);
    xdb_pfu_delete_nunidx(pPfuTableReg, ptPfuNoUniIdxReg2, aucTupleKey, dwTupleNo);

    dbRet = xdb_Delete_Tuple(pPfuTableReg,dwTupleNo);
    _DB_STATEMENT_TRUE_RTN_VALUE((FALSE == dbRet), FALSE);
    return TRUE;
}


/* 临时设置，后续需要查capacity表*/
WORD32 _db_get_ncu_appid_relate_capacity()
{
#ifndef FT_TEST
    return R_SUBBINDAPP_CAPACITY;
#else
    return 20;
#endif
}

BOOL create_r_ncu_appid_relation(DWORD dwDbHandle)
{
    DWORD dwDynTblCapacity = 0;
    LPT_PfuTableReg      pTableReg = NULL;
    LPT_PfuUniIdxReg     pPfuUniIdxReg = NULL;
    LPT_PfuNoUniIdxReg   pPfuNoUniIdxReg    = NULL;

    dwDynTblCapacity = _db_get_ncu_appid_relate_capacity();
    zte_printf_s("ncu data app capacity: %u\n", dwDynTblCapacity);
    _DB_STATEMENT_TRUE_RTN_VALUE((0 == dwDynTblCapacity), FALSE);

    /* 创建表 */
    XOS_SysLog(LOG_EMERGENCY, "DB Module: create table r_pfu_subscribe start!\n");
    pTableReg = XDB_PFU_CREATE_TABLE(dwDbHandle, DB_HANDLE_R_NCUAPPID_RELATE, R_NCU_SUBBINDAPP, 
                                     dwDynTblCapacity, sizeof(T_psNcuAppidRelation), 
                                     sizeof(r_ncu_appid_relation_tuple));
    if (NULL == pTableReg)
    {
        XOS_SysLog(LOG_EMERGENCY, "[File:%s],[line:%d]DB Module:create table r_ncu_dataApp failed!\n", __FILE__, __LINE__);
        return FALSE;
    }

    /* 创建索引 */
    pPfuUniIdxReg = XDB_PFU_CREATE_UNIIDX(dwDbHandle, DB_HANDLE_IDX_R_NCU_APPID_RELATE_SUBAPPID, XDB_UNIDX_NORMARL,
                                          dwDynTblCapacity, XDB_MOD_TABLE(dwDynTblCapacity),
                                          sizeof(r_ncu_appid_relation_idx_tuple));
    if (NULL == pPfuUniIdxReg)
    {
        XOS_SysLog(LOG_EMERGENCY, "[File:%s],[line:%d]DB Module: create index DB_HANDLE_IDX_R_NCU_APPID_RELATE_SUBAPPID failed!\n", __FILE__, __LINE__);
        return FALSE;
    }
    xdb_pfu_UniIdx_Cpy_Reg(pPfuUniIdxReg, r_ncu_appid_relation_UIdxCpy);
    xdb_pfu_UniIdx_Merge_Reg(pPfuUniIdxReg, r_ncu_appid_relation_UIdxMerge);
    xdb_pfu_UniIdx_Cmp_Reg(pPfuUniIdxReg, r_ncu_appid_relation_UIdxCmp);


    /* 创建非唯一索引 */
    pPfuNoUniIdxReg = XDB_PFU_CREATE_NUNIDX(dwDbHandle, DB_HANDLE_IDX_R_NCU_APPID_RELATE_APPID,
                                          dwDynTblCapacity, XDB_MOD_TABLE(dwDynTblCapacity),
                                          sizeof(r_ncu_appid_relation_nonidx_tuple));
    if (NULL == pPfuNoUniIdxReg)
    {
        XOS_SysLog(LOG_EMERGENCY, "[File:%s],[line:%d]DB Module: create index DB_HANDLE_IDX_R_NCU_APPID_RELATE_APPID failed!\n", __FILE__, __LINE__);
        return FALSE;
    }
    xdb_pfu_nunidx_cpy_reg(pPfuNoUniIdxReg, r_ncu_appid_relation_NoUIdxCpy);
    xdb_pfu_nunidx_merge_reg(pPfuNoUniIdxReg, r_ncu_appid_relation_NoUIdxMerge);

    xdb_Set_ReleaseRsc_Overload(pTableReg,r_ncu_appid_relate_ReleaseRsc);
    XOS_SysLog(LOG_EMERGENCY, "DB Module: create create_r_ncu_appid_relation table and index success!\n");

    psNcuGetSubBindAppAllCfg();
    return TRUE;
}
