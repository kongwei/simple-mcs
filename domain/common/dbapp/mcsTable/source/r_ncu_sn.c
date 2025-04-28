/*********************************************************************
* 版权所有(C) 中兴通信股份有限公司. 保留所有权利!
*
* 文件名称： r_ncu_sn.c
* 文件标识：
* 内容摘要： NCU上SN上下文
* 其它说明：
* 当前版本： ZXUN-CSN
* 作    者：
* 完成日期： 2024-04-03
**********************************************************************/

/**************************************************************************
*                            头文件                                      *
**************************************************************************/
#include "r_ncu_sn.h"
#include "ps_db_define_ncu.h"
#include "xdb_core_pfu.h"
#include "xdb_core_pfu_db.h"
#include "xdb_core_pfu_table.h"
#include "xdb_core_uniindex.h"
#include "xdb_core_pfu_nunidx.h"
#include "xdb_core_pfu_tblimp_api.h"
#include "xdb_pfu_dyntbl_acc.h"
#include "ps_mcs_define.h"
#include "zte_slibc.h"
#include "MemShareCfg.h"
#include "psNcuCtxFunc.h"
#include "ncu_capacity.h"
#include "psMcsDebug.h"
/**************************************************************************
*                             全局变量                                    *
**************************************************************************/

/**************************************************************************
*                          外部函数声明                                    *
**************************************************************************/

/**************************************************************************
 *                              局部函数原型(本源文件使用)
 **************************************************************************/
T_NcuMcsSnCtx* psQryNcuMcsSnCtxById(WORD32 dwCtxId, WORD32 hDB);
/**************************************************************************
*                       函数实现                                         *
**************************************************************************/
void r_ncu_sn_UIdxMerge(void* pTuple, void* pKey, DWORD dwKeyLen)
{
    lp_r_ncu_sn_idx_flowsndir lpTmp = (lp_r_ncu_sn_idx_flowsndir)pKey;
    lp_r_ncu_sn_idx_tuple    lpAddr = (lp_r_ncu_sn_idx_tuple )pTuple;

    lpTmp->dwFlowId = lpAddr->dwFlowId;
    lpTmp->dwSeqNum = lpAddr->dwSeqNum;
    lpTmp->bDir     = lpAddr->bDir;
    return;
}
DBBOOL r_ncu_sn_UIdxCmp(void* pTuple, void* pKey, DWORD dwKeyLen)
{
    lp_r_ncu_sn_idx_flowsndir lpTmp = (lp_r_ncu_sn_idx_flowsndir)pKey;
    lp_r_ncu_sn_idx_tuple    lpAddr = (lp_r_ncu_sn_idx_tuple )pTuple;

    return ((lpTmp->dwFlowId  == lpAddr->dwFlowId)
        && (lpTmp->dwSeqNum  == lpAddr->dwSeqNum)
        && (lpTmp->bDir  == lpAddr->bDir));
}
void r_ncu_sn_UIdxCpy(void* pTuple, void* pKey, DWORD dwKeyLen)
{
    lp_r_ncu_sn_idx_flowsndir lpTmp = (lp_r_ncu_sn_idx_flowsndir)pKey;
    lp_r_ncu_sn_idx_tuple    lpAddr = (lp_r_ncu_sn_idx_tuple )pTuple;

    lpAddr->dwFlowId = lpTmp->dwFlowId;
    lpAddr->dwSeqNum = lpTmp->dwSeqNum;
    lpAddr->bDir     = lpTmp->bDir;
    return;
}
void r_ncu_sn_Flowid_IdxMerge(void* pTuple, void* pKey, DWORD dwKeyLen)
{
    lp_r_ncu_sn_nonidx_flowid lpTmp = (lp_r_ncu_sn_nonidx_flowid)pKey;
    lp_r_ncu_sn_idx_tuple    lpAddr = (lp_r_ncu_sn_idx_tuple )pTuple;

    lpTmp->dwFlowId = lpAddr->dwFlowId;
    return;
}
void r_ncu_sn_Flowid_IdxCpy(void* pTuple, void* pKey, DWORD dwKeyLen)
{
    lp_r_ncu_sn_nonidx_flowid lpTmp = (lp_r_ncu_sn_nonidx_flowid)pKey;
    lp_r_ncu_sn_idx_tuple    lpAddr = (lp_r_ncu_sn_idx_tuple )pTuple;

    lpAddr->dwFlowId = lpTmp->dwFlowId;
    return;
}

DBBOOL r_ncu_sn_ReleaseRsc(LPT_PfuDataBase pPfuDbReg, LPT_PfuTableReg pPfuTableReg, DWORD dwTupleNo)
{
    _DB_STATEMENT_TRUE_RTN_VALUE((NULL == pPfuDbReg), FALSE);

    T_NcuMcsSnCtx  *ptSnCtx = psQryNcuMcsSnCtxById(dwTupleNo, pPfuDbReg->hDB);
    _DB_STATEMENT_TRUE_RTN_VALUE((NULL == ptSnCtx), FALSE);

    LPT_PfuUniIdxReg    ptPfuUniIdxReg  = NULL;
    ptPfuUniIdxReg = xdb_Pfu_Get_UniIdxReg_ByDbReg(pPfuDbReg, DB_HANDLE_IDX_R_NCU_SN_FLOWID_SN_DIR);
    _DB_STATEMENT_TRUE_RTN_VALUE((NULL == ptPfuUniIdxReg), FALSE);
    xdb_pfu_delete_UniIdx_by_tupleNo(pPfuTableReg, ptPfuUniIdxReg, dwTupleNo);
    
    LPT_PfuNoUniIdxReg  pPfuNoUniIdxReg = NULL;
    BYTE aucTupleKey[XDB_IDXKEYLEN_MAX]  = {0};
    pPfuNoUniIdxReg = xdb_Pfu_Get_NoUniIdxReg_ByDbReg(pPfuDbReg, DB_HANDLE_IDX_R_NCU_SN_FLOWID);
    _DB_STATEMENT_TRUE_RTN_VALUE((NULL == pPfuNoUniIdxReg), FALSE);
    xdb_pfu_nunidx_merge_by_tupleno(pPfuTableReg, pPfuNoUniIdxReg, aucTupleKey, dwTupleNo);
    xdb_pfu_delete_nunidx(pPfuTableReg, pPfuNoUniIdxReg, aucTupleKey, dwTupleNo);

    DBBOOL dbRet = FALSE;
    dbRet = xdb_Delete_Tuple(pPfuTableReg,dwTupleNo);
    _DB_STATEMENT_TRUE_RTN_VALUE((FALSE == dbRet), FALSE);
    return TRUE;
}

/* 临时设置，后续需要查capacity表*/
DWORD  _db_get_ncu_sn_capacity()
{
#ifndef FT_TEST
    return _db_get_ncu_capacity_by_tblName(NCCTXNAME_R_NCU_SN, R_NCU_SN);
#else
    return 20;
#endif
}
/**********************************************************************
* 函数名称：create_r_ncu_sn
* 功能描述：表创建函数
* 输入参数：dwDbHandle, 库句柄
* 输出参数：无
* 返 回 值：TRUE,成功  FALSE,失败
* 其它说明：
* 创建日期       版本号              创建人       修改内容
* -----------------------------------------------
***********************************************************************/
DBBOOL create_r_ncu_sn(DWORD dwDbHandle)
{
    DWORD  dwDynTblCapacity = 0;
    LPT_PfuTableReg      pTableReg         = NULL;
    LPT_PfuUniIdxReg     pPfuUniIdxReg     = NULL;
    LPT_PfuNoUniIdxReg  pPfuNoUniIdxReg    = NULL;

    dwDynTblCapacity = _db_get_ncu_sn_capacity();

    /* 创建表 */
    XOS_SysLog(LOG_EMERGENCY, "DB Module: create table create_r_ncu_sn start!\n");
    pTableReg = XDB_PFU_CREATE_TABLE(dwDbHandle, DB_HANDLE_R_NCU_SN, R_NCU_SN,
                                    dwDynTblCapacity,
                                    sizeof(T_NcuMcsSnCtx),
                                    sizeof(r_ncu_sn_idx_tuple));

    if (NULL == pTableReg)
    {
        XOS_SysLog(LOG_EMERGENCY, "[File:%s],[line:%d]DB Module:create table create_r_ncu_sn failed!\n", __FILE__, __LINE__);
        return FALSE;
    }
    
    /* 创建唯一索引 */
    pPfuUniIdxReg = XDB_PFU_CREATE_UNIIDX(dwDbHandle, DB_HANDLE_IDX_R_NCU_SN_FLOWID_SN_DIR, XDB_UNIDX_NORMARL,
                                        dwDynTblCapacity, XDB_MOD_TABLE(dwDynTblCapacity),
                                        sizeof(r_ncu_sn_idx_flowsndir));
    if (NULL == pPfuUniIdxReg)
    {
        XOS_SysLog(LOG_EMERGENCY, "[File:%s],[line:%d]DB Module: create index DB_HANDLE_IDX_R_NCU_SN_FLOWID_SN_DIR failed!\n", __FILE__, __LINE__);
        return FALSE;
    }
    xdb_pfu_UniIdx_Cpy_Reg(pPfuUniIdxReg,   r_ncu_sn_UIdxCpy);
    xdb_pfu_UniIdx_Merge_Reg(pPfuUniIdxReg, r_ncu_sn_UIdxMerge);
    xdb_pfu_UniIdx_Cmp_Reg(pPfuUniIdxReg,   r_ncu_sn_UIdxCmp);

    /* 创建非唯一索引 flowid */
    pPfuNoUniIdxReg = XDB_PFU_CREATE_NUNIDX(dwDbHandle, DB_HANDLE_IDX_R_NCU_SN_FLOWID, dwDynTblCapacity,
                   XDB_MOD_TABLE(dwDynTblCapacity), sizeof(r_ncu_sn_nonidx_flowid)); //lint !e734

    if (NULL == pPfuNoUniIdxReg)
    {
        XOS_SysLog(LOG_EMERGENCY, "[File:r_qosmonitor.c],[line:%d]DB Module: create index DB_HANDLE_IDX_R_NCU_SN_FLOWID failed!\n", __LINE__);
        return FALSE;
    }

    xdb_pfu_nunidx_cpy_reg(pPfuNoUniIdxReg, r_ncu_sn_Flowid_IdxCpy);
    xdb_pfu_nunidx_merge_reg(pPfuNoUniIdxReg, r_ncu_sn_Flowid_IdxMerge);
    
    xdb_Set_ReleaseRsc_Overload(pTableReg,r_ncu_sn_ReleaseRsc);
    XOS_SysLog(LOG_EMERGENCY, "DB Module: create r_ncu_sn table and index success!\n");

    return TRUE;
}

