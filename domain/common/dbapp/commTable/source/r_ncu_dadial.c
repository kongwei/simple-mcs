
#include "r_ncu_dadial.h"
#include "ps_db_define_ncu.h"
#include "xdb_core_pfu.h"
#include "xdb_pfu_com.h"
#include "xdb_core_pfu_db.h"
#include "xdb_core_pfu_table.h"
#include "xdb_core_uniindex.h"
//#include "xdb_core_pfu_nunidx.h"
#include "xdb_core_pfu_tblimp_api.h"
#include "xdb_pfu_dyntbl_acc.h"
//#include "ps_mcs_define.h"
#include "zte_slibc.h"
#include "psNcuDADialCtxProc.h"
/* Started by AICoder, pid:67f17mb09fbabc614f510829207422363ec73392 */
void r_ncu_dadial_UIdxMerge(void* pTuple, void* pKey, DWORD dwKeyLen)
{
    lp_r_ncu_dadial_idx_tuple lpTmp = (lp_r_ncu_dadial_idx_tuple)pKey;
    lp_r_ncu_dadial_tuple pAddr = (lp_r_ncu_dadial_tuple)pTuple;

    zte_memcpy_s(&(lpTmp->tIMSI), sizeof(T_IMSI), &(pAddr->tIMSI), sizeof(T_IMSI));
    lpTmp->dwSubAppid = pAddr->dwSubAppid;
    return;
}

void r_ncu_dadial_UIdxCpy(void* pTuple, void* pKey, DWORD dwKeyLen)
{
    lp_r_ncu_dadial_idx_tuple lpTmp = (lp_r_ncu_dadial_idx_tuple)pKey;
    lp_r_ncu_dadial_tuple pAddr = (lp_r_ncu_dadial_tuple)pTuple;

    zte_memcpy_s(&(pAddr->tIMSI), sizeof(T_IMSI), &(lpTmp->tIMSI), sizeof(T_IMSI));
    pAddr->dwSubAppid = lpTmp->dwSubAppid;
    return;
}

BOOLEAN r_ncu_dadial_UIdxCmp(void* pTuple, void* pKey, DWORD dwKeyLen)
{
    lp_r_ncu_dadial_idx_tuple lpTmp = (lp_r_ncu_dadial_idx_tuple)pKey;
    lp_r_ncu_dadial_tuple lpAddr = (lp_r_ncu_dadial_tuple)pTuple;

    return ((lpTmp->dwSubAppid == lpAddr->dwSubAppid) &&
            (0 == memcmp(&(lpTmp->tIMSI), &(lpAddr->tIMSI), sizeof(T_IMSI))));
}
/* Ended by AICoder, pid:67f17mb09fbabc614f510829207422363ec73392 */
/* Started by AICoder, pid:903156971bk993514a850b1a809ab610916803ef */
BOOLEAN r_ncu_dadial_ReleaseRsc(LPT_PfuDataBase pPfuDbReg, LPT_PfuTableReg pPfuTableReg, DWORD dwTupleNo)
{
    _DB_STATEMENT_TRUE_RTN_VALUE((NULL == pPfuDbReg), FALSE); 
    LPT_PfuUniIdxReg ptPfuUniIdxReg = NULL;

    BOOLEAN dbRet = FALSE;

    T_psNcuDADial *ptDADialCtx = psMcsGetDADialCtxById(dwTupleNo, pPfuDbReg->hDB);
    _DB_STATEMENT_TRUE_RTN_VALUE((NULL == ptDADialCtx), FALSE);

    ptPfuUniIdxReg = xdb_Pfu_Get_UniIdxReg_ByDbReg(pPfuDbReg, DB_HANDLE_IDX_R_NCU_DADIAL);
    _DB_STATEMENT_TRUE_RTN_VALUE((NULL == ptPfuUniIdxReg), FALSE);
    xdb_pfu_delete_UniIdx_by_tupleNo(pPfuTableReg, ptPfuUniIdxReg, dwTupleNo);

    dbRet = xdb_Delete_Tuple(pPfuTableReg, dwTupleNo);
    _DB_STATEMENT_TRUE_RTN_VALUE((FALSE == dbRet), FALSE);
    return TRUE;
}
/* Ended by AICoder, pid:903156971bk993514a850b1a809ab610916803ef */

/* Started by AICoder, pid:0ec40u92d0mc4a31456d09ac3035544020383690 */
WORD32 _db_get_ncu_dadial_capacity()
{
#ifndef FT_TEST
    return ctR_ncu_dadial_CAPACITY;
#else
    return 10;
#endif
}

BOOL create_r_ncu_dadial(DWORD dwDbHandle)
{
    DWORD dwDynTblCapacity = 0;
    LPT_PfuTableReg      pTableReg = NULL;
    LPT_PfuUniIdxReg     pPfuUniIdxReg = NULL;

    dwDynTblCapacity = _db_get_ncu_dadial_capacity();
    zte_printf_s("ncu dadial capacity: %u\n", dwDynTblCapacity);
    _DB_STATEMENT_TRUE_RTN_VALUE((0 == dwDynTblCapacity), FALSE);

    /* 创建表 */
    XOS_SysLog(LOG_EMERGENCY, "DB Module: create table r_ncu_dadial start!\n");
    pTableReg = XDB_PFU_CREATE_TABLE(dwDbHandle, DB_HANDLE_R_NCU_DADIAL, R_NCU_DADIAL,
                                     dwDynTblCapacity, sizeof(T_psNcuDADial),
                                     sizeof(r_ncu_dadial_tuple));
    if (NULL == pTableReg)
    {
        XOS_SysLog(LOG_EMERGENCY, "[File:%s],[line:%d]DB Module:create table r_ncu_dadial failed!\n", __FILE__, __LINE__);
        return FALSE;
    }

    /* 创建索引 */
    pPfuUniIdxReg = XDB_PFU_CREATE_UNIIDX(dwDbHandle, DB_HANDLE_IDX_R_NCU_DADIAL, XDB_UNIDX_NORMARL,
                                          dwDynTblCapacity, XDB_MOD_TABLE(dwDynTblCapacity),
                                          sizeof(r_ncu_dadial_idx_tuple));
    if (NULL == pPfuUniIdxReg)
    {
        XOS_SysLog(LOG_EMERGENCY, "[File:%s],[line:%d]DB Module: create index DB_HANDLE_IDX_R_NCU_DADIAL failed!\n", __FILE__, __LINE__);
        return FALSE;
    }
    xdb_pfu_UniIdx_Cpy_Reg(pPfuUniIdxReg, r_ncu_dadial_UIdxCpy);
    xdb_pfu_UniIdx_Merge_Reg(pPfuUniIdxReg, r_ncu_dadial_UIdxMerge);
    xdb_pfu_UniIdx_Cmp_Reg(pPfuUniIdxReg, r_ncu_dadial_UIdxCmp);

    xdb_Set_ReleaseRsc_Overload(pTableReg, r_ncu_dadial_ReleaseRsc);
    XOS_SysLog(LOG_EMERGENCY, "DB Module: create r_ncu_dadial table and index success!\n");

    psNcuGetDADialCtxAllCfg();
    return TRUE;
}
/* Ended by AICoder, pid:0ec40u92d0mc4a31456d09ac3035544020383690 */

/* Started by AICoder, pid:v86cft9235g667814d51093c10c1bf151d024464 */
T_psNcuDADial* psMcsGetDADialCtxById(WORD32 dwCtxId, WORD32 hDB)
{
    DM_PFU_QUERYDYNDATA_BYTUPLENO_REQ tQueryReq = {0}; /* 查询请求 */
    DM_PFU_QUERYDYNDATA_BYTUPLENO_ACK tQueryAck = {0}; /* 查询应答 */
    _DB_STATEMENT_TRUE_RTN_VALUE((0 == dwCtxId), NULL);
    tQueryReq.hDB               = _NCU_DBHANDLE_COMM; /* 库句柄待定 */
    tQueryReq.hTbl              = DB_HANDLE_R_NCU_DADIAL;
    tQueryReq.dwDataAreaIndex   = dwCtxId;
    _pDM_PFU_QUERYDYNDATA_BYTUPLENO(&tQueryReq, &tQueryAck);
    _DB_STATEMENT_TRUE_RTN_VALUE((RC_OK != tQueryAck.retCODE), NULL);
    return (T_psNcuDADial*)tQueryAck.ptDataAreaAddr;/*返回数据区指针 */
}
/* Ended by AICoder, pid:v86cft9235g667814d51093c10c1bf151d024464 */
