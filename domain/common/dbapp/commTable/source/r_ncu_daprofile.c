/*********************************************************************
* 版权所有(C) 中兴通信股份有限公司. 保留所有权利!
*
* 文件名称： r_dyn_daprofile.c
* 文件标识：
* 内容摘要： 
* 其它说明：
* 当前版本： ZXUN-xGW V7.17.10
* 作    者： 
* 完成日期： 2017-07-19
**********************************************************************/

/**************************************************************************
*                            头文件                                      *
**************************************************************************/
#include "r_ncu_daprofile.h"
#include "zte_slibc.h"
#include "xdb_core_pfu.h"
#include "xdb_core_pfu_db.h"
#include "xdb_core_pfu_table.h"
#include "xdb_core_pfu_tblimp_api.h"
#include "xdb_core_uniindex.h"
#include "xdb_pfu_dyntbl_acc.h"
#include "ps_db_define_ncu.h"
#include "dbmLibComm.h"
#include "r_daprofile.h"

#define USE_DM_ALL
#include "dbm_lib_upfcomm.h"

#include "psNcuDAProfileCtxProc.h"
/**************************************************************************
*                             全局变量                                    *
**************************************************************************/
/**************************************************************************
*                             外部函数声明                                    *
**************************************************************************/
/**************************************************************************
*                     全局函数实现                                      *
**************************************************************************/
void r_dyn_daprofile_application_UIdxMerge(void* pTuple, void* pKey,  DWORD dwKeyLen)
{
    lp_r_dyn_daprofile_idx_application lpTmp = (lp_r_dyn_daprofile_idx_application)pKey;
    lp_r_dyn_daprofile_idx_tuple lpAddr = (lp_r_dyn_daprofile_idx_tuple )pTuple;

    zte_memcpy_s(lpTmp->application, sizeof(lpTmp->application), lpAddr->application, sizeof(lpAddr->application));
    return;
}
DBBOOL r_dyn_daprofile_application_UIdxCmp(void* pTuple, void* pKey,  DWORD dwKeyLen)
{
    lp_r_dyn_daprofile_idx_application lpTmp = (lp_r_dyn_daprofile_idx_application)pKey;
    lp_r_dyn_daprofile_idx_tuple lpAddr = (lp_r_dyn_daprofile_idx_tuple )pTuple;

    return ((0 == memcmp(lpTmp->application, lpAddr->application, sizeof(lpAddr->application))));
}
void r_dyn_daprofile_application_UIdxCpy(void* pTuple, void* pKey,DWORD dwKeyLen)
{
    lp_r_dyn_daprofile_idx_application lpTmp = (lp_r_dyn_daprofile_idx_application)pKey;
    lp_r_dyn_daprofile_idx_tuple lpAddr = (lp_r_dyn_daprofile_idx_tuple )pTuple;

    zte_memcpy_s(lpAddr->application, sizeof(lpAddr->application), lpTmp->application, sizeof(lpTmp->application));
    return;
}
DBBOOL r_dyn_daprofile_ReleaseRsc(LPT_PfuDataBase pPfuDbReg, LPT_PfuTableReg pPfuTableReg, DWORD dwTupleNo)
{
    LPT_PfuUniIdxReg    ptPfuUniIdxReg  = NULL;
    DBBOOL dbRet = FALSE;

    ptPfuUniIdxReg = xdb_Pfu_Get_UniIdxReg_ByDbReg(pPfuDbReg, DB_HANDLE_IDX_R_NCU_DAPROFILE_APPLICATION);
    _DB_STATEMENT_TRUE_RTN_VALUE((NULL == ptPfuUniIdxReg), FALSE);
    xdb_pfu_delete_UniIdx_by_tupleNo(pPfuTableReg, ptPfuUniIdxReg, dwTupleNo);
    dbRet = xdb_Delete_Tuple(pPfuTableReg,dwTupleNo);
    _DB_STATEMENT_TRUE_RTN_VALUE((FALSE == dbRet), FALSE);
    return TRUE;
}
DWORD  _db_get_daprofile_capacity()
{
#ifndef FT_TEST
    return R_DAPROFILE_CAPACITY;
#else
    return 20;
#endif
}

/**********************************************************************
* 函数名称：create_r_dyn_daprofile
* 功能描述：表创建函数
* 输入参数：dwDbHandle, 库句柄
* 输出参数：无
* 返 回 值：TRUE,成功  FALSE,失败
* 其它说明：
* 创建日期       版本号              创建人       修改内容
* -----------------------------------------------
***********************************************************************/
DBBOOL create_r_dyn_daprofile(DWORD dwDbHandle)
{
    DWORD  dwDynTblCapacity = 0;
    LPT_PfuTableReg      pTableReg         = NULL;
    LPT_PfuUniIdxReg     pPfuUniIdxReg     = NULL;

    dwDynTblCapacity = _db_get_daprofile_capacity();

    /* 创建表 */
    XOS_SysLog(LOG_EMERGENCY, "DB Module: create table create_r_dyn_daprofile start!\n");
    pTableReg = XDB_PFU_CREATE_TABLE(dwDbHandle, DB_HANDLE_R_NCUDAPROFILE, R_DYN_DAPROFILE, dwDynTblCapacity, sizeof(T_VpfuDAProfileCtx),sizeof(r_dyn_daprofile_idx_tuple));
    if (NULL == pTableReg)
    {
        XOS_SysLog(LOG_EMERGENCY, "[File:%s],[line:%d]DB Module:create table create_r_dyn_daprofile failed!\n", __FILE__, __LINE__);
        return FALSE;
    }
    /* 创建唯一索引 */
    pPfuUniIdxReg = XDB_PFU_CREATE_UNIIDX(dwDbHandle, DB_HANDLE_IDX_R_NCU_DAPROFILE_APPLICATION, XDB_UNIDX_NORMARL,
                                          dwDynTblCapacity, XDB_MOD_TABLE(dwDynTblCapacity),
                                          sizeof(r_dyn_daprofile_idx_application));
    if (NULL == pPfuUniIdxReg)
    {
        XOS_SysLog(LOG_EMERGENCY, "[File:%s],[line:%d]DB Module: create index DB_HANDLE_IDX_R_NCU_DAPROFILE_APPLICATION failed!\n", __FILE__, __LINE__);
        return FALSE;
    }
    xdb_pfu_UniIdx_Cpy_Reg(pPfuUniIdxReg,   r_dyn_daprofile_application_UIdxCpy);
    xdb_pfu_UniIdx_Merge_Reg(pPfuUniIdxReg, r_dyn_daprofile_application_UIdxMerge);
    xdb_pfu_UniIdx_Cmp_Reg(pPfuUniIdxReg,   r_dyn_daprofile_application_UIdxCmp);
    xdb_Set_ReleaseRsc_Overload(pTableReg,r_dyn_daprofile_ReleaseRsc);
    XOS_SysLog(LOG_EMERGENCY, "DB Module: create r_dyn_daprofile table and index success!\n");

    getDAProfileAllCfg();

    return TRUE;
}
