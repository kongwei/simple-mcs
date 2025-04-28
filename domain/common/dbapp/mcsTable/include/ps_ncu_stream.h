#ifndef _PS_NCU_STREAM_H_
#define _PS_NCU_STREAM_H_
#include "tulip.h"
#include "NcuSynInfo.h"
#include "UpfNcuSynInfo.h"
#include "xdb_pfu_dyntbl_acc.h"
#include "xdb_core_pfu.h"
#include "xdb_core_pfu_que_nolock.h"

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

#define R_FLOWCTX               "R_FLOWCTX"

typedef struct tagT_psNcuFlowCtx
{
    WORD32  dwSessionID;
    WORD32  dwUpdateTimeStamp;  //PowerOnSec
    WORD32  dwCreateTimeStamp;  //StdSec
    WORD32  dwTunnelID;        /*记录TEID或GREKEY*/
    
    WORD32  dwFlowID;
   /*五元组信息*/
    BYTE     bIPType;        /*MCS_IP_TECH_IPV4 or MCS_IP_TECH_IPV6*/
    BYTE     bProType;
    BYTE     ucULInnerDscp;
    BYTE     ucULOuterDscp;

    WORD64 ddwUPSeid;
    /* 关联session表 */
    VOID *ptNcuSesionCtx;
    VOID *ptNcuFlowAppNext; /** 同一个业务下的流 */
    WORD32 dwSubAppId;
    WORD32 dwAppId; 

    VOID *ptNcuDaCtx;

    WORD32 dwPktNum;
    WORD32 dwPktBytes;
    /* 更新到appCtx后清0 */
    WORD32 dwUlPktNum;
    WORD32 dwDlPktNum;
    WORD32 dwUlPktBytes;
    WORD32 dwDlPktBytes;
    WORD32 dwUlTcpPktNum;
    WORD32 dwDlTcpPktNum;
    WORD32 dwUlPktLossNum;
    WORD32 dwDlPktLossNum;
    WORD32 dwUlDisorderNum;
    WORD32 dwDlDisorderNum;
    WORD32 dwUlPktRtt;
    WORD32 dwDlPktRtt;
    WORD32 dwUlRttPktNum; /* 只用来计算RTT */
    WORD32 dwDlRttPktNum; /* 只用来计算RTT */

    /* 不清0 */
    WORD32 dwTotalUlPktNum;
    WORD32 dwTotalDlPktNum;
    WORD32 dwTotalUlPktBytes;
    WORD32 dwTotalDlPktBytes;
    WORD32 dwTotalUlTcpPktNum;
    WORD32 dwTotalDlTcpPktNum;
    WORD32 dwTotalUlPktLossNum;
    WORD32 dwTotalDlPktLossNum;
    WORD32 dwTotalDlDisorderNum;
    WORD32 dwTotalUlDisorderNum;
    WORD32 dwTotalUlPktRtt;
    WORD32 dwTotalDlPktRtt;
    WORD32 dwTotalUlRttPktNum;
    WORD32 dwTotalDlRttPktNum;

    /* 质差分析需要 */
    WORD32 dwLastUlPktBytes;
    WORD32 dwLastDlPktBytes;
    WORD32 bandwidthUl;
    WORD32 bandwidthDl;

    WORD32 dwLastUlPktRtt;
    WORD32 dwLastDlPktRtt;
    WORD32 dwLastUlRttPktNum;
    WORD32 dwLastDlRttPktNum;
    WORD32 dwUlRtt;
    WORD32 dwDlRtt;

    WORD32 dwLastUlPktLossNum;
    WORD32 dwLastDlPktLossNum;
    WORD32 dwLastUlPktNum;
    WORD32 dwLastDlPktNum;
    WORD16 dwUlPktLossRate;
    WORD16 dwDlPktLossRate;
    WORD32 dwUlUnitPktLossNum;
    WORD32 dwDlUnitPktLossNum;
    WORD32 dwTotalUlUnitPktNum;
    WORD32 dwTotalDlUnitPktNum;

    WORD32 dwhDBByThreadNo;
    WORD32 dwRptTimeStamp;

    WORD32 dwUlExpectSn;
    WORD32 dwDlExpectSn;

    /*上下文中挂的流的质差保障计算时延的双向链表*/
    VOID    *ptSnDlHead;   /* 质差计算时延上下文下行头指针 */
    VOID    *ptSnDlTail;   /* 质差计算时延上下文下行尾指针 */
    VOID    *ptSnUlHead;   /* 质差计算时延上下文上行头指针 */
    VOID    *ptSnUlTail;   /* 质差计算时延上下文上行尾指针 */
    WORD16  wSnDlNum;      /* 质差计算时延上下文下行记录数 */
    WORD16  wSnUlNum;      /* 质差计算时延上下文上行记录数 */
    BYTE    b5QI;
    BYTE    bSpecialStmFlag;
    BYTE    rsv[2];

    WORD32 dwPktLossNum[DA_QOS_MAX];
    WORD32 dwPktPrevNum[DA_QOS_MAX];
    WORD32 dwPktPrevBytes[DA_QOS_MAX];
    WORD32 dwPktPrevRtt[DA_QOS_MAX];
    WORD32 dwPktPreLossNum[DA_QOS_MAX];
}T_psNcuFlowCtx;

typedef struct tagR_flowctx_tuple
{
   T_IPV6    tCliIP;
   T_IPV6    tSvrIP;
   WORD64    ddwVPNInfo;
   WORD      wCliPort;
   WORD      wSvrPort;
   BYTE      bProType;
   BYTE      bIPType;    /* 合一结构，4:IPv4  6:IPv6 7:MAC*/
} __attribute__ ((packed)) r_flowctx_tuple,  *lp_r_flowctx_tuple;

typedef struct tagR_flowctx_idx_tuple
{
   T_IPV6    tCliIP;
   T_IPV6    tSvrIP;
   WORD64    ddwVPNInfo;
   WORD      wCliPort;
   WORD      wSvrPort;
   BYTE      bProType;
   BYTE      bIPType;    /* 合一结构，4:IPv4  6:IPv6 */
} __attribute__ ((packed)) r_flowctx_idx_tuple,  *lp_r_flowctx_idx_tuple;

BOOLEAN create_r_flowctx(DWORD dwDbHandle);
T_psNcuFlowCtx* psNcuMcsCreatStmByIndex(DB_STRM_INDEX* ptQuintuple,WORD32 hDB);
BYTE *psNcuQueryStreamByQuintuple(WORD32 hDB, VOID *key);
T_psNcuFlowCtx* psNcuMcsGetStmByID(WORD32 dwFlowID,WORD32 hDB);
WORD32 psNcuMcsRelStmByID(WORD32 dwFlowID,WORD32 hDB);
DWORD xdb_pfu_query_bulk_que_flow(LP_DB_TBL_QUEHEAD_T ptQueHead, T_PfuTableReg *pTableReg,
                                  T_PfuUniIdxReg *pIdxReg,VOID *pKey);
DWORD _db_get_flowctx_capacity();
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
