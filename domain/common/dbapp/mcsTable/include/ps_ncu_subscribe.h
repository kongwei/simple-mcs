#ifndef _PS_NCU_SUBSCRIBE_H_
#define _PS_NCU_SUBSCRIBE_H_

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
#include "ps_pub.h"
#include "upfcommPub.h"
/**************************************************************************
*                             宏定义                                     *
**************************************************************************/
//#define ctR_ncu_subscribe_CAPACITY    200000
#define R_NCU_SUBSCRIBE               "R_NCU_SUBSCRIBE"
#define EXPECT_SUBSCRIBE_NUM (WORD32)100
/**************************************************************************
*                             全局变量声明                                *
**************************************************************************/


/**************************************************************************
*                             全局函数原型                                *
**************************************************************************/
BOOL create_r_ncu_subscribe(DWORD dwDbHandle);
/**************************************************************************
*                             数据结构体定义                                *
**************************************************************************/
typedef struct tagT_psNcuDaSubscribeCtx

{
    WORD32 dwSubscribeCtxId;
    WORD32 dwAppId;
    WORD64 ddwUPSeid;
    WORD32 dwUpdateTimeStamp;  //PowerOnSec  
    WORD32 dwCreateTimeStamp;  //StdSec
    CHAR appidstr[LEN_APPLICATIONMAP_APPLICATION_MAX+1];
    BYTE bSubType;  /**                  0b00000001 -- N4 质差订阅
                                           0b00000010 -- N4 体验数据订阅
                                           0b00000100 -- 服务化 体验数据订阅
                     */
}T_psNcuDaSubScribeCtx;

typedef struct tagR_ncu_subscribe_tuple
{
    WORD64      ddwUPSeid;
    WORD32      dwAppid;
} __attribute__ ((packed)) r_ncu_subscribe_tuple,  *lp_r_ncu_subscribe_tuple;

typedef struct tagR_ncu_subscribe_idx_tuple
{
    WORD64      ddwUPSeid;
} __attribute__ ((packed)) r_ncu_subscribe_idx_upseid_tuple,  *lp_r_ncu_subscribe_idx_upseid_tuple;


typedef struct tagR_ncu_subscribe_idx_upseid_appid_tuple
{
    WORD64      ddwUPSeid;
    WORD32      dwAppid;
} __attribute__ ((packed)) r_ncu_subscribe_idx_upseid_appid_tuple,  *lp_r_ncu_subscribe_idx_upseid_appid_tuple;


BOOL create_r_ncu_subscribe(DWORD dwDbHandle);

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
/* The end of _ncu_subscribe_H*/
