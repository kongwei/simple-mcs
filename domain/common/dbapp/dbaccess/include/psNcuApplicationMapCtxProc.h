/* Started by AICoder, pid:ra929m94bfy7844142800908c009ac6065b1253f */
/******************************************************************************
 * 版权所有 (C)2016, 深圳市中兴通讯股份有限公司
 * 模块名          : MCS
 * 文件名          : psNcuApplicationMapCtxProc.h
 * 相关文件        :
 * 文件实现功能     : APPLICATIONMAP上下文增删改查接口
 * 归属团队        : M6
 * 版本           : V7.24.20
 -------------------------------------------------------------------------------
 * 修改记录:
 * 修改日期           版本号           修改人           修改内容
 * 2024-09-20    V7.24.30              wya          create
 ******************************************************************************/
#ifndef _PS_NCU_APPLICATIONMAP_CTX_PROC_
#define _PS_NCU_APPLICATIONMAP_CTX_PROC_

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************
 *                              头文件(满足自包含，满足最小依赖)
 **************************************************************************/
#include "r_ncu_applicationmap.h"
/**************************************************************************
 *                              宏(对外提供)
 **************************************************************************/


/**************************************************************************
 *                              数据类型(对外提供)
 **************************************************************************/



/**************************************************************************
 *                              接口(对外提供，最小可见)
 **************************************************************************/
DBBOOL psNcuGetCfgStrAppidMapByInnerId(WORD32 dwInnerAppId, CHAR *strAppID);
WORD32 psNcuGetDynCtxInnerAppIDByStrAppID(CHAR *aucStrAppID);
DBBOOL psNcuGetDynCtxStrAppIDByInnerId(WORD32 dwInnerAppID, CHAR *strAppID);
WORD32 psNcuCreatAppIDMapCtxByStrAppID(CHAR *aucStrAppID, WORD32 dwInnerID, BYTE **ptCtxAddr);
VOID psNcuDelAppIDMapCtxByStrAppID(CHAR *aucStrAppID);
void psNcuUpdateAppIDMapByInnerId(WORD32 dwTupleNo, WORD32 dwInnerID);
DBBOOL psNcuGetCfgInnerAppIdByStrAppidmap(CHAR *strAppID,WORD32 *pdwInnerAppId);
WORD32 psNcuGetApplicationMapForCfg();

/**************************************************************************
 *                              全局变量声明(对外提供，最小可见)
 **************************************************************************/

#ifdef __cplusplus
}
#endif

#endif
/* Ended by AICoder, pid:ra929m94bfy7844142800908c009ac6065b1253f */
