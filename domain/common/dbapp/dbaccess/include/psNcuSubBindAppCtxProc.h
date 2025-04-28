/* Started by AICoder, pid:fcdfch447dgd8c314246099df0dc464bbf656e79 */
/******************************************************************************
 * 版权所有 (C)2016, 深圳市中兴通讯股份有限公司
 * 模块名          : MCS
 * 文件名          : psNcuSubBindAppCtxProc.h
 * 相关文件        :
 * 文件实现功能     : SUBBINDAPP上下文增删改查接口
 * 归属团队        : M6
 * 版本           : V7.24.30
 -------------------------------------------------------------------------------
 * 修改记录:
 * 修改日期           版本号           修改人           修改内容
 * 2024-09-20    V7.24.30              wya          create
 ******************************************************************************/
#ifndef _PS_NCU_SUBBINDAPP_CTX_PROC_
#define _PS_NCU_SUBBINDAPP_CTX_PROC_

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************
 *                              头文件(满足自包含，满足最小依赖)
 **************************************************************************/
#include "r_ncu_subbindapp.h"
/**************************************************************************
 *                              宏(对外提供)
 **************************************************************************/

/**************************************************************************
 *                              数据类型(对外提供)
 **************************************************************************/

/**************************************************************************
 *                              接口(对外提供，最小可见)
 **************************************************************************/
T_psNcuAppidRelation*  psQueryAppidRelateCtxBySubAppid(WORD32 dwSubAppid);
T_psNcuAppidRelation*  psCrtAppidRelateCtxBySubAppid(WORD32 dwSubAppid);
WORD32 psNcuMcsUpdAppidRelateCtxByAppid(WORD32 dwAppid,WORD32 dwCtxId);
WORD32  psDelAppidRelateCtxBySubAppid(WORD32 dwSubAppid);
WORD32 psVpfuMcsGetAllAppidRelateCtxByAppid(WORD32 dwAppid, BYTE *pbQueryAckBuff);
WORD32 psNcuGetAppidBySubAppid(WORD32 dwSubAppid);
BOOLEAN psNcuGetSubAppidStr(CHAR* ptStr, WORD32 dwSubAppid);
VOID allocSubBindAppBySubApp(CHAR *subapp, CHAR *application);
/**************************************************************************
 *                              全局变量声明(对外提供，最小可见)
 **************************************************************************/

#ifdef __cplusplus
}
#endif

#endif // _PS_NCU_SUBBINDAPP_CTX_PROC_
/* Ended by AICoder, pid:fcdfch447dgd8c314246099df0dc464bbf656e79 */
