/* Started by AICoder, pid:2881f488bde3f1f1428d09dd40f6674413d60e50 */
/******************************************************************************
 * 版权所有 (C)2016, 深圳市中兴通讯股份有限公司
 * 模块名          : MCS
 * 文件名          : psNcuDADialCtxProc.h
 * 相关文件        :
 * 文件实现功能     : DADIAL上下文增删改查接口
 * 归属团队        : M6
 * 版本           : V7.24.40
 -------------------------------------------------------------------------------
 * 修改记录:
 * 修改日期           版本号           修改人           修改内容
 * 2024-10-15     V7.24.40              zjw          create
 ******************************************************************************/
#ifndef _PS_NCU_DADIAL_CTX_PROC_
#define _PS_NCU_DADIAL_CTX_PROC_

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************
 *                              头文件(满足自包含，满足最小依赖)
 **************************************************************************/
#include "r_ncu_dadial.h"
#include "ps_ncu_dataApp.h"
/**************************************************************************
 *                              宏(对外提供)
 **************************************************************************/
#define MAX_CHAR_IMSI_LEN  (WORD32)16 /* IMSI的最大长度 */
#define ADD_DADIALCTX  (WORD32)0 /* 新增DaDial上下文 */
#define DEL_DADIALCTX  (WORD32)1 /* 删除DaDial上下文 */
#define UPD_DADIALCTX  (WORD32)2 /* 删除DaDial上下文 */

/**************************************************************************
 *                              数据类型(对外提供)
 **************************************************************************/
typedef struct tagT_psDialCfgChangeInfo
{
    BYTE   bOperation;
    BYTE   bDialQosQuality;
    WORD32 dwSubAppid;
    WORD64 ddwUpseid;
}T_psNcuDialCfgChangeInfo;

/**************************************************************************
 *                              接口(对外提供，最小可见)
 **************************************************************************/

/**************************************************************************
 *                              全局变量声明(对外提供，最小可见)
 **************************************************************************/
VOID psNcuGetDADialCtxAllCfg();
VOID psNcuAllocDaDialCtxByImsiSubApp(T_IMSI *tImsi, CHAR *subapp, BYTE bQosQuality);
T_psNcuDADial* psQueryDADialCtxByImsiSubAppid(T_IMSI *tIMSI, WORD32 dwSubAppid);
WORD32 psDelDADialCtxByImsiSubAppid(T_IMSI *tIMSI, WORD32 dwSubAppid);
VOID psNcuDaDialCtxRelateDaAppCtx(T_psNcuDADial *ptNcuDADialCtx, BYTE bOperation);
VOID psNcuDaAppCtxRelateDaDialCtx(T_psNcuDaAppCtx *ptDataAppCtx, BYTE bThreadNo);
VOID psNcuDialCfgChangeProc(T_psNcuDialCfgChangeInfo *ptNcuDialCfgChangeInfo, WORD16 wThreadNo);
#ifdef __cplusplus
}
#endif

#endif // _PS_NCU_DADIAL_CTX_PROC_
/* Ended by AICoder, pid:2881f488bde3f1f1428d09dd40f6674413d60e50 */
