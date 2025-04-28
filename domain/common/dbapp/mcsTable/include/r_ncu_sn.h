#ifndef _PS_R_NCU_SN_H_
#define _PS_R_NCU_SN_H_

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

#include "tulip.h"
#include "xdb_pfu_com.h"
/**************************************************************************
*                             宏定义                                     *
**************************************************************************/
#define R_NCU_SN               "R_NCU_SN"
/**************************************************************************
*                             全局变量声明                                *
**************************************************************************/


/**************************************************************************
*                             全局函数原型                                *
**************************************************************************/

/**************************************************************************
*                             数据结构体定义                                *
**************************************************************************/
typedef struct tagT_NcuMcsSnCtx
{
    WORD64 ddwTimeStamp;
    WORD32 dwFlowId;
    WORD32 dwSeqNum;
    WORD32 dwCtxID;
    WORD16 wPayLoad;
    BYTE   bDir;
    BYTE   bRvs;
    WORD32 dwCreateTimeStamp;
    WORD32 dwAckNum;

    VOID *ptSnNext;
    VOID *ptSnPrev;
}T_NcuMcsSnCtx;

typedef struct tagR_ncu_sn_idx_tuple
{
    WORD32 dwFlowId;
    WORD32 dwSeqNum;
    BYTE   bDir;
} __attribute__ ((packed)) r_ncu_sn_idx_tuple,  *lp_r_ncu_sn_idx_tuple;

typedef struct tagR_ncu_sn_idx_flowsndir
{
    WORD32 dwFlowId;
    WORD32 dwSeqNum;
    BYTE   bDir;
} __attribute__ ((packed)) r_ncu_sn_idx_flowsndir,  *lp_r_ncu_sn_idx_flowsndir;

typedef struct tagR_ncu_sn_nonidx_flowid
{
    WORD32 dwFlowId;
} __attribute__ ((packed)) r_ncu_sn_nonidx_flowid,  *lp_r_ncu_sn_nonidx_flowid;

DBBOOL create_r_ncu_sn(DWORD dwDbHandle);
DWORD  _db_get_ncu_sn_capacity();

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
