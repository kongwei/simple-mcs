/* Started by AICoder, pid:naaeah759a3334014305080610307d552230b580 */
/******************************************************************************
 * 版权所有 (C)2016, 深圳市中兴通讯股份有限公司
 * 模块名          : ncu
 * 文件名          : r_ncu_applicationmap.h
 * 相关文件        :
 * 文件实现功能     : appid映射字符串
 * 归属团队        : M6
 * 版本           : V1.0
------------------------------------------------------------------------------
 * 修改记录:
 * 修改日期           版本号           修改人           修改内容
 * 2024-05-11        V7.24.11.B5       WYA            create
 ******************************************************************************/
#include "r_ncu_applicationmap.h"
#include "applicationmap.h"
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
#include "ps_ncu_typedef.h"
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
WORD32 psNcuGetApplicationMapForCfg();
/**************************************************************************
 *                              全局变量(本源文件使用)
 **************************************************************************/

/**************************************************************************
 *                              函数实现(函数布局:总分结构|深度优先)
 **************************************************************************/
void r_ncu_appidmap_name_UIdxMerge(void* pTuple, void* pKey, DWORD dwKeyLen)
{
    lp_r_ncu_appidmap_name_idx_tuple lpTmp  = (lp_r_ncu_appidmap_name_idx_tuple)pKey;
    lp_r_ncu_appidmap_tuple          lpAddr = (lp_r_ncu_appidmap_tuple )pTuple;

    zte_memcpy_s(lpTmp->application, sizeof(lpTmp->application), lpAddr->application, sizeof(lpAddr->application));
    return;
}

DBBOOL r_ncu_appidmap_name_UIdxCmp(void* pTuple, void* pKey,  DWORD dwKeyLen)
{
    lp_r_ncu_appidmap_name_idx_tuple lpTmp  = (lp_r_ncu_appidmap_name_idx_tuple)pKey;
    lp_r_ncu_appidmap_tuple          lpAddr = (lp_r_ncu_appidmap_tuple )pTuple;

    return ((0 == memcmp(lpTmp->application, lpAddr->application, sizeof(lpAddr->application))));
}

void r_ncu_appidmap_name_UIdxCpy(void* pTuple, void* pKey,DWORD dwKeyLen)
{
    lp_r_ncu_appidmap_name_idx_tuple lpTmp  = (lp_r_ncu_appidmap_name_idx_tuple)pKey;
    lp_r_ncu_appidmap_tuple          lpAddr = (lp_r_ncu_appidmap_tuple )pTuple;

    zte_memcpy_s(lpAddr->application, sizeof(lpAddr->application), lpTmp->application, sizeof(lpTmp->application));
    return;
}

void r_ncu_appidmap_innerid_UIdxMerge(void* pTuple, void* pKey,  DWORD dwKeyLen)
{
    lp_r_ncu_appidmap_innerid_idx_tuple lpTmp  = (lp_r_ncu_appidmap_innerid_idx_tuple)pKey;
    lp_r_ncu_appidmap_tuple             lpAddr = (lp_r_ncu_appidmap_tuple )pTuple;

    lpTmp->innerappid = lpAddr->innerappid;
    return;
}

DBBOOL r_ncu_appidmap_innerid_UIdxCmp(void* pTuple, void* pKey,  DWORD dwKeyLen)
{
    lp_r_ncu_appidmap_innerid_idx_tuple lpTmp  = (lp_r_ncu_appidmap_innerid_idx_tuple)pKey;
    lp_r_ncu_appidmap_tuple             lpAddr = (lp_r_ncu_appidmap_tuple )pTuple;

    return (lpTmp->innerappid == lpAddr->innerappid);
}

void r_ncu_appidmap_innerid_UIdxCpy(void* pTuple, void* pKey,DWORD dwKeyLen)
{
    lp_r_ncu_appidmap_innerid_idx_tuple lpTmp  = (lp_r_ncu_appidmap_innerid_idx_tuple)pKey;
    lp_r_ncu_appidmap_tuple             lpAddr = (lp_r_ncu_appidmap_tuple )pTuple;

    lpAddr->innerappid = lpTmp->innerappid;
    return;
}

DBBOOL r_ncu_appidmap_ReleaseRsc(LPT_PfuDataBase pPfuDbReg, LPT_PfuTableReg pPfuTableReg, DWORD dwTupleNo)
{
    LPT_PfuUniIdxReg    ptPfuUniIdxReg  = NULL;
    DBBOOL dbRet = FALSE;

    ptPfuUniIdxReg = xdb_Pfu_Get_UniIdxReg_ByDbReg(pPfuDbReg, DB_HANDLE_IDX_R_NCU_APPIDMAP_NAME);
    MCS_CHK_RET((NULL == ptPfuUniIdxReg), FALSE);
    xdb_pfu_delete_UniIdx_by_tupleNo(pPfuTableReg, ptPfuUniIdxReg, dwTupleNo);

    ptPfuUniIdxReg = xdb_Pfu_Get_UniIdxReg_ByDbReg(pPfuDbReg, DB_HANDLE_IDX_R_NCU_APPIDMAP_INNERID);
    MCS_CHK_RET((NULL == ptPfuUniIdxReg), FALSE);
    xdb_pfu_delete_UniIdx_by_tupleNo(pPfuTableReg, ptPfuUniIdxReg, dwTupleNo);

    dbRet = xdb_Delete_Tuple(pPfuTableReg,dwTupleNo);
    MCS_CHK_RET((FALSE == dbRet), FALSE);
    return TRUE;
}

DWORD  _db_get_appidmap_capacity()
{
#ifndef FT_TEST
    return APPLICATIONMAP_CAPACITY;
#else
    return 20;
#endif
}

/**********************************************************************
* 函数名称：create_r_ncu_appidmap
* 功能描述：表创建函数
* 输入参数：dwDbHandle, 库句柄
* 输出参数：无
* 返 回 值：TRUE,成功  FALSE,失败
* 其它说明：
* 创建日期       版本号              创建人       修改内容
* -----------------------------------------------
***********************************************************************/
DBBOOL create_r_ncu_appidmap(DWORD dwDbHandle)
{
    DWORD  dwDynTblCapacity = 0;
    LPT_PfuTableReg      pTableReg         = NULL;
    LPT_PfuUniIdxReg     pPfuUniIdxReg     = NULL;

    dwDynTblCapacity = _db_get_appidmap_capacity();

    /* 创建表 */
    XOS_SysLog(LOG_EMERGENCY, "DB Module: create table create_r_ncu_appidmap start!\n");
    pTableReg = XDB_PFU_CREATE_TABLE(dwDbHandle, DB_HANDLE_R_NCU_APPIDMAP, R_NCU_APPIDMAP, dwDynTblCapacity, 0, sizeof(r_ncu_appidmap_tuple));//lint !e734

	if (NULL == pTableReg)
    {
        XOS_SysLog(LOG_EMERGENCY, "[File:%s],[line:%d]DB Module:create table create_r_ncu_appidmap failed!\n", __FILE__, __LINE__);
        return FALSE;
    }

    /* 创建唯一索引application */
    pPfuUniIdxReg = XDB_PFU_CREATE_UNIIDX(dwDbHandle, DB_HANDLE_IDX_R_NCU_APPIDMAP_NAME, XDB_UNIDX_NORMARL,
                                            dwDynTblCapacity, XDB_MOD_TABLE(dwDynTblCapacity),
                                            sizeof(r_ncu_appidmap_name_idx_tuple));//lint !e734
    if (NULL == pPfuUniIdxReg)
    {
        XOS_SysLog(LOG_EMERGENCY, "[File:%s],[line:%d]DB Module: create index DB_HANDLE_IDX_R_DYN_APPIDMAP_NAME failed!\n", __FILE__, __LINE__);
        return FALSE;
    }
    xdb_pfu_UniIdx_Cpy_Reg(pPfuUniIdxReg,   r_ncu_appidmap_name_UIdxCpy);
    xdb_pfu_UniIdx_Merge_Reg(pPfuUniIdxReg, r_ncu_appidmap_name_UIdxMerge);
    xdb_pfu_UniIdx_Cmp_Reg(pPfuUniIdxReg,   r_ncu_appidmap_name_UIdxCmp);

    /* 创建唯一索引innerappid */
    pPfuUniIdxReg = XDB_PFU_CREATE_UNIIDX(dwDbHandle, DB_HANDLE_IDX_R_NCU_APPIDMAP_INNERID, XDB_UNIDX_NORMARL,
                                            dwDynTblCapacity, XDB_MOD_TABLE(dwDynTblCapacity),
                                            sizeof(r_ncu_appidmap_innerid_idx_tuple));//lint !e734
    if (NULL == pPfuUniIdxReg)
    {
        XOS_SysLog(LOG_EMERGENCY, "[File:%s],[line:%d]DB Module: create index DB_HANDLE_IDX_R_DYN_APPIDMAP_INNERID failed!\n", __FILE__, __LINE__);
        return FALSE;
    }
    xdb_pfu_UniIdx_Cpy_Reg(pPfuUniIdxReg,   r_ncu_appidmap_innerid_UIdxCpy);
    xdb_pfu_UniIdx_Merge_Reg(pPfuUniIdxReg, r_ncu_appidmap_innerid_UIdxMerge);
    xdb_pfu_UniIdx_Cmp_Reg(pPfuUniIdxReg,   r_ncu_appidmap_innerid_UIdxCmp);

    xdb_Set_ReleaseRsc_Overload(pTableReg,r_ncu_appidmap_ReleaseRsc);
    XOS_SysLog(LOG_EMERGENCY, "DB Module: create r_ncu_appidmap table and index success!\n");

    psNcuGetApplicationMapForCfg();

    return TRUE;
}

/* Ended by AICoder, pid:naaeah759a3334014305080610307d552230b580 */