#ifndef _R_NCU_SUBBINDAPP_H_
#define _R_NCU_SUBBINDAPP_H_

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
#include "upfcommPub.h"
/**************************************************************************
*                             宏定义                                     *
**************************************************************************/
//#define ctR_ncu_SESSION_CAPACITY    200000
#define R_NCU_SUBBINDAPP               "R_NCU_SUBBINDAPP"
#define EXPECT_DATA_APP_NUM          (WORD32)1000
/**************************************************************************
*                             全局变量声明                                *
**************************************************************************/


/**************************************************************************
*                             全局函数原型                                *
**************************************************************************/

/**************************************************************************
*                             数据结构体定义                                *
**************************************************************************/

typedef struct tagT_psNcuAppidRelation
{
    WORD32 dwCtxId;
    WORD32 dwAppid;
    WORD32 dwSubAppid;
    WORD32 rsv;
    CHAR   subAppidStr[LEN_APPLICATIONMAP_APPLICATION_MAX+1];
    CHAR   appidStr[LEN_APPLICATIONMAP_APPLICATION_MAX+1];
}T_psNcuAppidRelation;

typedef struct tagR_ncu_appid_relation_tuple
{
    WORD32      dwSubAppid;
    WORD32      dwAppid;
} __attribute__ ((packed)) r_ncu_appid_relation_tuple,  *lp_r_ncu_appid_relation_tuple;

typedef struct tagR_ncu_appid_relation_idx_tuple
{
    WORD32      dwSubAppid;
} __attribute__ ((packed)) r_ncu_appid_relation_idx_tuple,  *lp_r_ncu_appid_relation_idx_tuple;

typedef struct tagR_ncu_appid_relation_nonidx_tuple
{
    WORD32      dwAppid;
} __attribute__ ((packed)) r_ncu_appid_relation_nonidx_tuple,  *lp_r_ncu_appid_relation_nonidx_tuple;

#ifdef __cplusplus
extern "C" {
#endif

BOOL create_r_ncu_appid_relation(DWORD dwDbHandle);

#ifdef __cplusplus
}
#endif
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
/* The end of _ncu_SESSION_H*/
