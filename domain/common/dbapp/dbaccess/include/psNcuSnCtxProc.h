/* Started by AICoder, pid:cc6a1y099fl1ae714e3b0b115070da67bfa1ce2e */
/******************************************************************************
 * 版权所有 (C)2016, 深圳市中兴通讯股份有限公司
 * 模块名          : MCS
 * 文件名          : psNcuSnCtxProc.h
 * 相关文件        :
 * 文件实现功能     : SnCtx增删改查接口
 * 归属团队        : MD6
 * 版本           : V7.24.30
-------------------------------------------------------------------------------
* 修改记录:
* 修改日期            版本号           修改人           修改内容
  * 2024-09-20    V7.24.30              wya          create
 ******************************************************************************/
#ifndef _PS_NCU_SN_CTX_PROC_
#define _PS_NCU_SN_CTX_PROC_

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************
 *                              头文件(满足自包含，满足最小依赖)
 **************************************************************************/
#include "r_ncu_sn.h"
/**************************************************************************
 *                              宏(对外提供)
 **************************************************************************/

/**************************************************************************
 *                              数据类型(对外提供)
 **************************************************************************/

/**************************************************************************
 *                              接口(对外提供，最小可见)
 **************************************************************************/
T_NcuMcsSnCtx* psCrtSnCtxByFlowIdSeqDir(WORD32 dwFlowId, WORD32 dwSeqNum, BYTE bDir, WORD32 hDB);
T_NcuMcsSnCtx* psQrySnCtxByFlowIdSeqDir(WORD32 dwFlowId, WORD32 dwSeqNum, BYTE bDir, WORD32 hDB);
WORD32 psUpdSnCtxByFlowId(WORD32 dwFlowId,WORD32 dwCtxID,WORD32 hDB);
WORD32 psDelAllSnCtxByFlowId(WORD32 dwFlowId, WORD32 hDB);
WORD32 psGetAllSnCtxByFlowId(WORD32 dwFlowId, WORD32 hDB, BYTE *pbQueryAckBuff);
T_NcuMcsSnCtx* psGetSnCtxByFlowIdSeqDir(WORD32 dwFlowId, WORD32 dwSeqNum, BYTE bDir, WORD32 hDB);
/**************************************************************************
 *                              全局变量声明(对外提供，最小可见)
 **************************************************************************/

#ifdef __cplusplus
}
#endif

#endif // _PS_NCU_SN_CTX_PROC_
/* Ended by AICoder, pid:cc6a1y099fl1ae714e3b0b115070da67bfa1ce2e */
