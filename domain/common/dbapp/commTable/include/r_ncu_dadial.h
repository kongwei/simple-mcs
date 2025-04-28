/* Started by AICoder, pid:x0c2435a831acbf14ad5080890af5566d9f5d4dd */
#ifndef _R_NCU_DADIAL_H_
#define _R_NCU_DADIAL_H_

#if defined(_ARCHITECTURE_)
#pragma noalign    /* noalign for ic386 */
#elif defined(__BORLANDC__)
#pragma option -a- /* noalign for Borland C */
#elif defined(_MSC_VER)
#pragma pack(1)    /* noalign for Microsoft C */
#elif defined(__WATCOMC__)
#pragma pack(1)    /* noalign for Watcom C */
#elif defined(__DIAB__)
#pragma pack(1)    /* noalign for psosystem C */
#endif

#include "tulip.h"
//#include "upfcommPub.h"
#include "ps_pub.h"
/**************************************************************************
*                             宏定义                                     *
**************************************************************************/
#define ctR_ncu_dadial_CAPACITY     10
#define R_NCU_DADIAL               "R_NCU_DADIAL"

#define QOSQUALITY_NORMAL   0   /*业务正常*/
#define QOSQUALITY_POOR     1   /*业务质差*/
/**************************************************************************
*                             全局变量声明                                *
**************************************************************************/


/**************************************************************************
*                             全局函数原型                                *
**************************************************************************/

/**************************************************************************
*                             数据结构体定义                                *
**************************************************************************/
typedef struct tagT_psNcuDADial
{
    WORD32      dwCtxId;
    WORD32      dwSubAppid;
    T_IMSI      tIMSI;
    BYTE        bQosQuality;
    BYTE        rsv[7];
}T_psNcuDADial;

typedef struct tagR_ncu_dadial_tuple
{
    T_IMSI      tIMSI;
    WORD32      dwSubAppid;
} __attribute__ ((packed)) r_ncu_dadial_tuple,  *lp_r_ncu_dadial_tuple;

typedef struct tagR_ncu_dadial_idx_tuple
{
    T_IMSI      tIMSI;
    WORD32      dwSubAppid;
} __attribute__ ((packed)) r_ncu_dadial_idx_tuple,  *lp_r_ncu_dadial_idx_tuple;

BOOL create_r_ncu_dadial(DWORD dwDbHandle);
T_psNcuDADial* psMcsGetDADialCtxById(WORD32 dwCtxId, WORD32 hDB);

#if defined(_ARCHITECTURE_)
#pragma align      /* align for ic386 */
#elif defined(__BORLANDC__)
#pragma option -a  /* align for Borland C */
#elif defined(_MSC_VER)
#pragma pack()     /* align for Microsoft C */
#elif defined(__WATCOMC__)
#pragma pack()     /* align for Watcom C */
#elif defined(__DIAB__)
#pragma pack()     /* align for psosystem C */
#endif

#endif
/* The end of _ncu_dadial_H*/
/* Ended by AICoder, pid:x0c2435a831acbf14ad5080890af5566d9f5d4dd */