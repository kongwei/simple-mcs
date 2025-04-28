/* Started by AICoder, pid:8864d0b6f8d873b1486c080fe04cfd4766135f1e */
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
#ifndef _PS_NCU_APPLICATIONMAP_
#define _PS_NCU_APPLICATIONMAP_

#if defined(_ARCHITECTURE_)
#pragma noalign    /* noalign for ic386 */
#elif defined(__BORLANDC__)
#pragma option -a- /* noalign for Borland C */
#elif defined(_MSC_VER)
#pragma pack(1)    /* noalign for Microsoft C */
#elif defined(__WATCOMC__)
#pragma pack(1)    /* noalign for Watcom C */
#elif defined(__DIAB)
#pragma pack(1)    /* noalign for psosystem C */
#endif
/**************************************************************************
 *                              头文件(满足自包含，满足最小依赖)
 **************************************************************************/
#include "upfcommPub.h"
/**************************************************************************
 *                              宏(对外提供)
 **************************************************************************/
#define R_NCU_APPIDMAP               "R_NCU_APPIDMAP"
#define INVALIDINNERAPPID            ((WORD32)(-1))
/**************************************************************************
 *                              数据类型(对外提供)
 **************************************************************************/
typedef struct tagr_ncu_appidmap_tuple
{
    WORD32  innerappid;
    CHAR    application[LEN_APPLICATIONMAP_APPLICATION_MAX + 1];
} __attribute__ ((packed)) r_ncu_appidmap_tuple,  *lp_r_ncu_appidmap_tuple;

typedef struct tagr_ncu_appidmap_name_idx_tuple
{
    CHAR    application[LEN_APPLICATIONMAP_APPLICATION_MAX + 1];
} __attribute__ ((packed)) r_ncu_appidmap_name_idx_tuple,  *lp_r_ncu_appidmap_name_idx_tuple;

typedef struct tagr_ncu_appidmap_innerid_idx_tuple
{
    WORD32  innerappid;
} __attribute__ ((packed)) r_ncu_appidmap_innerid_idx_tuple,  *lp_r_ncu_appidmap_innerid_idx_tuple;
/**************************************************************************
 *                              接口(对外提供，最小可见)
 **************************************************************************/
DBBOOL create_r_ncu_appidmap(WORD32 dwDbHandle);
/**************************************************************************
 *                              全局变量声明(对外提供，最小可见)
 **************************************************************************/

#if defined(_ARCHITECTURE_)
#pragma align      /* align for ic386 */
#elif defined(__BORLANDC__)
#pragma option -a  /* align for Borland C */
#elif defined(_MSC_VER)
#pragma pack()     /* align for Microsoft C */
#elif defined(__WATCOMC__)
#pragma pack()     /* align for Watcom C */
#elif defined(__DIAB)
#pragma pack()     /* align for psosystem C */
#endif

#endif
// end of file
/* Ended by AICoder, pid:8864d0b6f8d873b1486c080fe04cfd4766135f1e */