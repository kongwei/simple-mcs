#ifndef _PS_NCU_SESSION_H_
#define _PS_NCU_SESSION_H_

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
#include "UpfNcuSynInfo.h"
#include "SigTrace.h"
#include "xdb_core_pfu.h"
#include "ps_db_define_pfu.h"

/**************************************************************************
*                             宏定义                                     *
**************************************************************************/
//#define ctR_ncu_SESSION_CAPACITY    200000
#define R_NCU_SESSION               "R_NCU_SESSION"
#define EXPECT_SESSION_NUM (WORD32)10
/**************************************************************************
*                             全局变量声明                                *
**************************************************************************/


/**************************************************************************
*                             全局函数原型                                *
**************************************************************************/

/**************************************************************************
*                             数据结构体定义                                *
**************************************************************************/

typedef struct tagT_psNcuSesionCtx

{
    WORD64 ddwUPSeid;
    WORD32  dwSessionCtxId;
    WORD32  dwSessionID;
    WORD32 dwUpdateTimeStamp;  //PowerOnSec

    WORD32 dwCreateTimeStamp;  //StdSec
    CHAR Apn[64];
    BYTE bImsi[8];
    BYTE bIsdn[8];

    T_NcuSnssai  tSNssai;
    UserLocation  tUserLocation;

    T_IPV6 tMSIPv6;
    T_IPV6 tNwdafIPv6;
    T_IPV4 tNwdafIPv4;
    T_IPV4 tMSIPv4;
    BYTE bRatType;
    BYTE bHasUli;
    BYTE bMSIPType;
    BYTE bNwdafIPType;
    BYTE bSubType;
    BYTE bNcuToPfuSynSessionctx; //Ncu是否向PFU发送过同步请求
    BYTE rcv;
    T_SIGTRACE_MATCHINFO tSigTraceMatchOut;
    CHAR Uri_Exp[AUCREPORTURI_MAXLEN];
    CHAR Uri_Ana[AUCREPORTURI_MAXLEN];
    CHAR bCorrelationId_Exp[AUCCORID_MAXLEN];
    CHAR bCorrelationId_Ana[AUCCORID_MAXLEN];
    BYTE bHasReportLinkFault;
}T_psNcuSessionCtx;

typedef struct tagR_ncu_session_tuple
{
    WORD64      ddwUPSeid;
    T_IMSI      tIMSI;
} __attribute__ ((packed)) r_ncu_session_tuple,  *lp_r_ncu_session_tuple;

typedef struct tagR_ncu_session_idx_tuple
{
    WORD64      ddwUPSeid;
} __attribute__ ((packed)) r_ncu_session_idx_tuple,  *lp_r_ncu_session_idx_tuple;


typedef struct tagR_ncu_session_idx_imsi_tuple
{
    T_IMSI      tIMSI;
} __attribute__ ((packed)) r_ncu_session_idx_imsi_tuple,  *lp_r_ncu_session_idx_imsi_tuple;

#ifdef __cplusplus
extern "C" {
#endif

BOOL create_r_ncu_session(DWORD dwDbHandle);
T_psNcuSessionCtx*  psQuerySessionByUpseid(WORD64 ddwUpseid, WORD32 hDB);
T_psNcuSessionCtx*  psCreateSessionByUpseid(WORD64 ddwUpseid, WORD32 hDB);
WORD32  psDelSessionByUpseid(WORD64 ddwUpseid, WORD32 hDB);
T_psNcuSessionCtx *psMcsGetSessionCtxById(WORD32 dwCtxId, WORD32 hDB);
WORD32 psVpfuMcsUpdSessionCtxByImsi(BYTE* ptIMSI, WORD32 dwCtxId, WORD32 hDB);
WORD32 psVpfuMcsGetSessionCtxNumByIMSI(BYTE *ptIMSI, WORD32 hDB, BYTE *pbQueryAckBuff);
LPT_PfuQueHeadReg _db_getGroupQueHeadAdrr(DWORD dwDbHandle,WORD32 dwGroupId);
BOOLEAN  psGroupQueInsert(DWORD dwDbHandle,WORD32 dwTupleNo, WORD32 dwGroupId, DWORD dwhQueGrpReg);
BOOLEAN psGroupQueDelete(DWORD dwDbHandle,WORD32 dwTupleNo, WORD32 dwGroupId);
WORD32 _db_get_ncusession_capacity();
LPT_PfuQueHeadReg (*psGetPfuQueHeadReg())[MAX_UPF_GROUP_NUM] ;
LPT_PfuDataBase psGetPfuDbReg();

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
