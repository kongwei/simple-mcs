/* Started by AICoder, pid:z1fedrb880r6e5c1440a0ac10081f3635231eb3c */
/******************************************************************************
 * 版权所有 (C)2016, 深圳市中兴通讯股份有限公司
 * 模块名          : MCS
 * 文件名          : psNcuSubscribeCtxProc.h
 * 相关文件        :
 * 文件实现功能     : SubscribeCtx增删改查接口
 * 归属团队        : M6
 * 版本           : V7.24.30
 -------------------------------------------------------------------------------
 * 修改记录:
 * 修改日期           版本号           修改人           修改内容
  * 2024-09-20    V7.24.30              wya          create
 ******************************************************************************/
#ifndef _PS_NCU_SUBSCRIBE_CTX_PROC_
#define _PS_NCU_SUBSCRIBE_CTX_PROC_

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************
 *                              头文件(满足自包含，满足最小依赖)
 **************************************************************************/
#include "ps_ncu_subscribe.h"
/**************************************************************************
 *                              宏(对外提供)
 **************************************************************************/

/**************************************************************************
 *                              数据类型(对外提供)
 **************************************************************************/

/**************************************************************************
 *                              接口(对外提供，最小可见)
 **************************************************************************/
T_psNcuDaSubScribeCtx *psMcsGetsubscribeCtxById(WORD32 dwCtxId, WORD32 hDB);
T_psNcuDaSubScribeCtx*  psQuerySubscribeByUpseidAppid(WORD64 ddwUPSeid, WORD32 dwAppid, WORD32 hDB);
T_psNcuDaSubScribeCtx*  psCreatesubscribeByUpseidAppid(WORD64 ddwUPSeid, WORD32 dwAppid, WORD32 hDB);
inline WORD32 psVpfuMcsUpdSubScribeCtxByUPseid(WORD64 ddwUPSeid,WORD32 dwSessCtxId,WORD32 hDB);
WORD32  psDelSubscribeByUpseidAppid(WORD64 ddwUPSeid,WORD32 dwAppid, WORD32 hDB);
WORD32 psVpfuMcsGetAllSubScribeCtxByUPseid(WORD64 ddwUPSeid,WORD32 hDB,BYTE *pbQueryAckBuff);
/**************************************************************************
 *                              全局变量声明(对外提供，最小可见)
 **************************************************************************/

#ifdef __cplusplus
}
#endif

#endif // _PS_NCU_SUBSCRIBE_CTX_PROC_
/* Ended by AICoder, pid:z1fedrb880r6e5c1440a0ac10081f3635231eb3c */
