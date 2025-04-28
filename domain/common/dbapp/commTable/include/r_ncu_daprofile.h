/*********************************************************************
* 版权所有(C) 中兴通信股份有限公司. 保留所有权利!
*
* 文件名称： r_dyn_daprofile.h
* 文件标识：
* 内容摘要： DAProfile上下文
* 其它说明：
* 当前版本： ZXUN-xGW V7.18.10
* 作    者： 
* 完成日期： 
**********************************************************************/
#ifndef _R_NCU_DAPROFILE_H
#define _R_NCU_DAPROFILE_H


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
*                             宏定义                                     *
**************************************************************************/
#include "xdb_pfu_com.h"
// #define USE_DM_ALL
// #include "dbm_lib_upf.h"
#include "upfcommPub.h"

#define ctR_DYN_DAPROFILE_CAPACITY    (1024)
#define ciR_DYN_DAPROFILE_MOD         (DWORD)((ctR_DYN_DAPROFILE_CAPACITY)/4*2+9)    /*可能需要删除*/
/* 表名 */
#define R_DYN_DAPROFILE               "R_DYN_DAPROFILE"
#define INVALIDINNERAPPID            ((WORD32)(-1))                                    /*可能需要删除*/

/**************************************************************************
*                             全局变量声明                                *
**************************************************************************/


/**************************************************************************
*                             全局函数原型                                *
**************************************************************************/
DBBOOL create_r_dyn_daprofile(DWORD dwDbHandle);

/**************************************************************************
*                             数据结构体定义                                *
**************************************************************************/
typedef struct
{
   WORD32  dwRvs;
   CHAR    application[LEN_R_DAPROFILE_APPLICATION_MAX + 1];
} __attribute__ ((packed)) r_dyn_daprofile_idx_tuple,  *lp_r_dyn_daprofile_idx_tuple;
typedef struct
{
   CHAR    application[LEN_R_DAPROFILE_APPLICATION_MAX + 1];
} __attribute__ ((packed)) r_dyn_daprofile_idx_application,  *lp_r_dyn_daprofile_idx_application;

typedef struct
{
    BYTE  ulbwswitch;
    BYTE  dlbwswitch;
    BYTE  andelayswitch;
    BYTE  dndelayswitch;
    BYTE  ulplrswitch;
    BYTE  dlplrswitch;
    BYTE  checkpoortimes;
    BYTE  checkgoodtimes;
    BYTE  reportevent;
    BYTE  bRvs[7];
    WORD32  ulbwthreshold;   
    WORD32  dlbwthreshold;
    WORD32  andelaythreshold;
    WORD32  dndelaythreshold;
    WORD32  ulplrthreshold;
    WORD32  dlplrthreshold;
    WORD32  detecttime;
    WORD32  reportpoortime;
    WORD32  reportgoodtime;
    WORD32  bwzerothreshold;
    CHAR    application[LEN_R_DAPROFILE_APPLICATION_MAX + 1];
} __attribute__ ((packed)) T_VpfuDAProfileCtx;

DBBOOL queryDAProfileCtx(lp_r_dyn_daprofile_idx_application idx,T_VpfuDAProfileCtx **ptDAProfileTuple);
T_VpfuDAProfileCtx* allocDAProfileCtx(lp_r_dyn_daprofile_idx_application idx, WORD32 *pdwDAProfileCtxId);
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
/* The end of _R_DYN_DAPROFILE_H*/
