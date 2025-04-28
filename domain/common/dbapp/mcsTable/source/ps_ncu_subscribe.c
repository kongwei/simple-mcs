#include "ps_ncu_subscribe.h"
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
#include "ps_ncu_subscribe.h"
#include "psNcuSubscribeCtxProc.h"

void r_ncu_subscribe_UIdxMerge(void* pTuple, void* pKey,DWORD dwKeyLen)
{
    lp_r_ncu_subscribe_idx_upseid_appid_tuple lpTmp = (lp_r_ncu_subscribe_idx_upseid_appid_tuple)pKey;
    lp_r_ncu_subscribe_tuple pAddr = (lp_r_ncu_subscribe_tuple)pTuple;
    lpTmp->ddwUPSeid = pAddr->ddwUPSeid;
    lpTmp->dwAppid    = pAddr->dwAppid;
    return;
}

void r_ncu_subscribe_UIdxCpy(void* pTuple, void* pKey,DWORD dwKeyLen)
{
    lp_r_ncu_subscribe_idx_upseid_appid_tuple lpTmp = (lp_r_ncu_subscribe_idx_upseid_appid_tuple)pKey;
    lp_r_ncu_subscribe_tuple pAddr = (lp_r_ncu_subscribe_tuple)pTuple;
    pAddr->ddwUPSeid = lpTmp->ddwUPSeid;
    pAddr->dwAppid    = lpTmp->dwAppid;
    return;
}

BOOLEAN r_ncu_subscribe_UIdxCmp(void* pTuple, void* pKey,  DWORD dwKeyLen)
{
    lp_r_ncu_subscribe_idx_upseid_appid_tuple lpTmp = (lp_r_ncu_subscribe_idx_upseid_appid_tuple)pKey;
    lp_r_ncu_subscribe_tuple lpAddr = (lp_r_ncu_subscribe_tuple )pTuple;

    return ((lpTmp->ddwUPSeid == lpAddr->ddwUPSeid) && (lpTmp->dwAppid == lpAddr->dwAppid));
}

void r_ncu_subscribe_upseid_NoUIdxMerge(void* pTuple, void* pKey,DWORD dwKeyLen)
{
    lp_r_ncu_subscribe_idx_upseid_tuple lpTmp  = (lp_r_ncu_subscribe_idx_upseid_tuple)pKey;
    lp_r_ncu_subscribe_tuple lpAddr           = (lp_r_ncu_subscribe_tuple )pTuple;

    lpTmp->ddwUPSeid = lpAddr->ddwUPSeid;
    return;
}

void r_ncu_subscribe_upseid_NoUIdxCpy(void* pTuple, void* pKey,DWORD dwKeyLen)
{
    lp_r_ncu_subscribe_idx_upseid_tuple lpTmp  = (lp_r_ncu_subscribe_idx_upseid_tuple)pKey;
    lp_r_ncu_subscribe_tuple lpAddr           = (lp_r_ncu_subscribe_tuple )pTuple;
    lpAddr->ddwUPSeid = lpTmp->ddwUPSeid;
    return;
}

BOOLEAN r_ncu_subscribe_ReleaseRsc(LPT_PfuDataBase pPfuDbReg, LPT_PfuTableReg pPfuTableReg, DWORD dwTupleNo)
{
    _DB_STATEMENT_TRUE_RTN_VALUE((NULL == pPfuDbReg), FALSE);
    LPT_PfuUniIdxReg    ptPfuUniIdxReg = NULL;

    BOOLEAN dbRet = FALSE;
    
    T_psNcuDaSubScribeCtx  *ptSubScribeCtx = NULL;

    ptSubScribeCtx = psMcsGetsubscribeCtxById(dwTupleNo, pPfuDbReg->hDB);
    _DB_STATEMENT_TRUE_RTN_VALUE((NULL == ptSubScribeCtx), FALSE);

    ptPfuUniIdxReg = xdb_Pfu_Get_UniIdxReg_ByDbReg(pPfuDbReg, DB_HANDLE_IDX_R_NCU_SUBSCRIBE_UPSEID_APPID);
    _DB_STATEMENT_TRUE_RTN_VALUE((NULL == ptPfuUniIdxReg), FALSE);
    xdb_pfu_delete_UniIdx_by_tupleNo(pPfuTableReg, ptPfuUniIdxReg, dwTupleNo);

    
    LPT_PfuNoUniIdxReg  ptPfuNoUniIdxReg2 = NULL;
    BYTE                aucTupleKey[XDB_IDXKEYLEN_MAX] = {0};
    ptPfuNoUniIdxReg2 = xdb_Pfu_Get_NoUniIdxReg_ByDbReg(pPfuDbReg, DB_HANDLE_IDX_R_NCU_SUBSCRIBE_UPSEID);
    _DB_STATEMENT_TRUE_RTN_VALUE((NULL == ptPfuNoUniIdxReg2), FALSE);
    xdb_pfu_nunidx_merge_by_tupleno(pPfuTableReg, ptPfuNoUniIdxReg2, aucTupleKey, dwTupleNo);
    xdb_pfu_delete_nunidx(pPfuTableReg, ptPfuNoUniIdxReg2, aucTupleKey, dwTupleNo);
    
    dbRet = xdb_Delete_Tuple(pPfuTableReg,dwTupleNo);
    _DB_STATEMENT_TRUE_RTN_VALUE((FALSE == dbRet), FALSE);
    return TRUE;
}


/* 临时设置，后续需要查capacity表*/
WORD32 _db_get_ncusubscribe_capacity()
{
#ifndef FT_TEST
    return _db_get_ncu_capacity_by_tblName(NCCTXNAME_R_NCU_SUBSCRIBE, R_NCU_SUBSCRIBE);
#else
    return 20;
#endif
}
BOOL create_r_ncu_subscribe(DWORD dwDbHandle)
{
    DWORD dwDynTblCapacity = 0;
    LPT_PfuTableReg      pTableReg = NULL;
    LPT_PfuUniIdxReg     pPfuUniIdxReg = NULL;
    LPT_PfuNoUniIdxReg   pPfuNoUniIdxReg    = NULL;

    dwDynTblCapacity = _db_get_ncusubscribe_capacity();
    zte_printf_s("ncu subscribe capacity: %u\n", dwDynTblCapacity);
    _DB_STATEMENT_TRUE_RTN_VALUE((0 == dwDynTblCapacity), FALSE);

    /* 创建表 */
    XOS_SysLog(LOG_EMERGENCY, "DB Module: create table r_pfu_subscribe start!\n");
    pTableReg = XDB_PFU_CREATE_TABLE(dwDbHandle, DB_HANDLE_R_NCUSUBCRIBE, R_NCU_SUBSCRIBE, 
                                     dwDynTblCapacity, sizeof(T_psNcuDaSubScribeCtx), 
                                     sizeof(r_ncu_subscribe_tuple));
    if (NULL == pTableReg)
    {
        XOS_SysLog(LOG_EMERGENCY, "[File:%s],[line:%d]DB Module:create table r_ncu_subscribe failed!\n", __FILE__, __LINE__);
        return FALSE;
    }

    /* 创建索引 */
    pPfuUniIdxReg = XDB_PFU_CREATE_UNIIDX(dwDbHandle, DB_HANDLE_IDX_R_NCU_SUBSCRIBE_UPSEID_APPID, XDB_UNIDX_NORMARL,
                                          dwDynTblCapacity, XDB_MOD_TABLE(dwDynTblCapacity),
                                          sizeof(r_ncu_subscribe_idx_upseid_appid_tuple));
    if (NULL == pPfuUniIdxReg)
    {
        XOS_SysLog(LOG_EMERGENCY, "[File:%s],[line:%d]DB Module: create index DB_HANDLE_IDX_R_NCU_SUBSCRIBE_UPSEID_APPID failed!\n", __FILE__, __LINE__);
        return FALSE;
    }
    xdb_pfu_UniIdx_Cpy_Reg(pPfuUniIdxReg, r_ncu_subscribe_UIdxCpy);
    xdb_pfu_UniIdx_Merge_Reg(pPfuUniIdxReg, r_ncu_subscribe_UIdxMerge);
    xdb_pfu_UniIdx_Cmp_Reg(pPfuUniIdxReg, r_ncu_subscribe_UIdxCmp);


    /* 创建非唯一索引*/
    pPfuNoUniIdxReg = XDB_PFU_CREATE_NUNIDX(dwDbHandle, DB_HANDLE_IDX_R_NCU_SUBSCRIBE_UPSEID,
                                          dwDynTblCapacity, XDB_MOD_TABLE(dwDynTblCapacity),
                                          sizeof(r_ncu_subscribe_idx_upseid_tuple));
    if (NULL == pPfuNoUniIdxReg)
    {
        XOS_SysLog(LOG_EMERGENCY, "[File:%s],[line:%d]DB Module: create index DB_HANDLE_IDX_R_PFU_subscribe_MSISDN failed!\n", __FILE__, __LINE__);
        return FALSE;
    }
    xdb_pfu_nunidx_cpy_reg(pPfuNoUniIdxReg, r_ncu_subscribe_upseid_NoUIdxCpy);
    xdb_pfu_nunidx_merge_reg(pPfuNoUniIdxReg, r_ncu_subscribe_upseid_NoUIdxMerge);
    
    xdb_Set_ReleaseRsc_Overload(pTableReg,r_ncu_subscribe_ReleaseRsc);
    XOS_SysLog(LOG_EMERGENCY, "DB Module: create r_ncu_subscribe table and index success!\n");

    return TRUE;
}
