#ifndef _PS_NCU_HTTPLINK_H_
#define _PS_NCU_HTTPLINK_H_

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
#include "HTTP_LB/httplink/http_link_pub.h"

/**************************************************************************
*                             宏定义                                     *
**************************************************************************/
#define ctR_ncu_httplink_CAPACITY     64
#define R_NCU_HTTPLINK               "R_NCU_HTTPLINK"

#ifndef INVALID_COMM_ID
#define INVALID_COMM_ID    0xFFFFFFFF
#endif

#define SCHEMA_HTTP     1
#define SCHEMA_HTTPS    2
/**************************************************************************
*                             全局变量声明                                *
**************************************************************************/


/**************************************************************************
*                             全局函数原型                                *
**************************************************************************/
BOOL create_r_ncu_httplink(DWORD dwDbHandle);
/**************************************************************************
*                             数据结构体定义                                *
**************************************************************************/
typedef struct tagT_psNcuHttpLink
{
    T_IPComm tNwdafIP;
    WORD32   dwClientProfileID;
    WORD16   wNwdafPort;
    BYTE     bSchema;
    BYTE     bRsv;

    WORD32 dwUpdateTimeStamp;          //每次查询更新
    HTTP_REMOTE_STATUS remoteStatus;   //对端状态

    WORD32 dwLinkIndex;
    WORD32 dwLogicNo;
    BYTE   bSequence[HTTP_MAX_SEQUENCE_LEN];
    JID    tHttpLbJid;

    WORD32 dwPollingLogicNo;
    BYTE   bPollingInst;
    BYTE   abRsv[3];

    WORD32 dwLogRptTimeStamp;
}T_psNcuHttpLink;


typedef struct tagR_ncu_httplink_tuple
{
    T_IPComm tNwdafIP;
    WORD32   dwClientProfileID;
    WORD16   wNwdafPort;
    BYTE     bSchema;
} __attribute__ ((packed)) r_ncu_httplink_tuple,  *lp_r_ncu_httplink_tuple;

typedef struct tagR_ncu_httplink_idx_tuple
{
    T_IPComm tNwdafIP;
    WORD32   dwClientProfileID;
    WORD16   wNwdafPort;
    BYTE     bSchema;
} __attribute__ ((packed)) r_ncu_httplink_idx_tuple,  *lp_r_ncu_httplink_idx_tuple;

BOOL create_r_ncu_httplink(DWORD dwDbHandle);
T_psNcuHttpLink* psMcsGetHttpLinkCtxById(WORD32 dwCtxId, WORD32 hDB);
T_psNcuHttpLink* psQueryHttpLinkByUniqueIdx(T_IPComm *tNwdafIP, WORD32 dwClientProfileID, WORD16 wNwdafPort, BYTE bSchema, WORD32 hDB);
T_psNcuHttpLink* psCreateHttpLinkByUniqueIdx(T_IPComm *tNwdafIP, WORD32 dwClientProfileID, WORD16 wNwdafPort, BYTE bSchema, WORD32 hDB);
WORD32 psDelHttpLinkByUniqueIdx(T_IPComm *tNwdafIP, WORD32 dwClientProfileID, WORD16 wNwdafPort, BYTE bSchema, WORD32 hDB);

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
/* The end of _ncu_httplink_H*/
