#ifndef _PS_NCU_DATA_APP_H_
#define _PS_NCU_DATA_APP_H_

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
#include "NcuSynInfo.h"
#include "UpfNcuSynInfo.h"
#include "upfcommPub.h"
/**************************************************************************
*                             宏定义                                     *
**************************************************************************/
//#define ctR_ncu_SESSION_CAPACITY    200000
#define R_NCU_DATA_APP               "R_NCU_DATA_APP"
#define EXPECT_DATA_APP_NUM          (WORD32)1000

// 质差判断需要的宏定义
#define FLOAT_ZERO                                  1e-6  
#define DA_QUALITY_POOR                             1  //质差
#define DA_QUALITY_GOOD                             5  //质优
/**************************************************************************
*                             全局变量声明                                *
**************************************************************************/


/**************************************************************************
*                             全局函数原型                                *
**************************************************************************/

/**************************************************************************
*                             数据结构体定义                                *
**************************************************************************/
enum ValueOfUpdateIndexMethod   
{
    enumUpdateIndex     = 1,
    enumDeleteIndex     = 2
};

typedef struct tagT_psNcuDataAnalysis
{
    WORD32 dwUlThroughput;
    WORD32 dwDlThroughput;
    WORD32 dwUlPktNum;
    WORD32 dwDlPktNum;

    WORD32 dwUlRtt;
    WORD32 dwDlRtt;
    WORD32 dwUlRttPktNum;
    WORD32 dwDlRttPktNum;


    WORD32 dwUlTcpPktNum;
    WORD32 dwDlTcpPktNum;
    WORD32 dwUlLossNum;
    WORD32 dwDlLossNum;

    WORD32 dwUlAvgBps;
    WORD32 dwDlAvgBps;

    WORD32 dwUlAvgRtt;
    WORD32 dwDlAvgRtt;

    WORD32 dwUlAvgLossRate;
    WORD32 dwDlAvgLossRate;
}T_psNcuDataAnalysis;

typedef struct tagT_psNcuDaAppCtx

{
    WORD64 ddwUPSeid;
    WORD32  dwID;
    WORD32 dwSubAppid;
    WORD32 dwAppid;
    WORD32 dwUpdateTimeStamp;  //PowerOnSec
    T_Time tCrtStdTime; 

    WORD64 ddwAnaRepClockStep; //质差上报步长
    WORD64 ddwAnaMonClockStep; //质差监控步长
    WORD64 ddwExpClockStep;
    CHAR  subAppidStr[LEN_APPLICATIONMAP_APPLICATION_MAX+1];
    WORD32 dwStreamNum;

    void* ptSubScribeCtx;
    void* ptSessionCtx;
    void* ptFlowCtxHead;
    void* ptDAProfileTuple;
    T_psNcuDataAnalysis Analysis; //QosExp

    // 以下为质差分析需要的
    // 带宽计算
    WORD64 dwUlTotalThroughput;
    WORD64 dwUlLastThroughput;
    WORD64 dwDlTotalThroughput;
    WORD64 dwDlLastThroughput;
    WORD32  dwDlUnitBw;
    WORD32  dwUlUnitBw;
    // 时延RTT计算
    WORD32 dwUlTotalPktRtt;
    WORD32 dwUlLastlPktRtt;
    WORD32 dwDlTotalPktRtt;
    WORD32 dwDlLastlPktRtt;
    WORD32 dwTotalUlRttPktNum;
    WORD32 dwLastUlRttPktNum;
    WORD32 dwTotalDlRttPktNum;
    WORD32 dwLastDlRttPktNum;
    WORD32 dwUlUnitRtt;
    WORD32 dwDlUnitRtt;
    // 丢包率计算
    WORD32 dwTotalUlPktLossNum;
    WORD32 dwLastUlPktLossNum;
    WORD32 dwTotalDlPktLossNum;
    WORD32 dwLastDlPktLossNum;
    WORD32 dwTotalUlPktNum;
    WORD32 dwLastUlPktNum;
    WORD32 dwTotalDlPktNum;
    WORD32 dwLastDlPktNum;
    WORD32 dwUlUnitPktLossNum;
    WORD32 dwDlUnitPktLossNum;
    WORD32 dwUlUnitPktNum;
    WORD32 dwDlUnitPktNum;

    // 已删流数据保存
    WORD32 dwDelTotalUlPktNum;
    WORD32 dwDelTotalDlPktNum;
    WORD64 dwDelTotalUlPktBytes;
    WORD64 dwDelTotalDlPktBytes;
    WORD32 dwDelTotalUlPktLossNum;
    WORD32 dwDelTotalDlPktLossNum;
    WORD32 dwDelTotalUlPktRtt;
    WORD32 dwDelTotalDlPktRtt;
    WORD32 dwDelTotalUlRttPktNum;
    WORD32 dwDelTotalDlRttPktNum;

    BYTE   bHasPoor; /*是否发生过质差*/
    BYTE   bCurQoe;  /*当前状态*/
    BYTE   bHasPoorNum;
    BYTE   bHasGoodNum;
    BYTE   bIsDialFlg;   /* 是否处于质差拨测状态 */
    BYTE   bCurDialQoe;  /*当前配置的拨测状态*/
    BYTE   bHasRptPoor;  /* 是否上报过质差 */
    BYTE   bRsv[1];
}T_psNcuDaAppCtx;

typedef struct tagR_ncu_dataapp_tuple
{
    WORD64      ddwUPSeid;
    WORD32      dwSubAppid;
    WORD32      dwAppid;
    WORD64      ddwAnaClockStep;
    WORD64      ddwExpClockStep;
    WORD64      ddwAnaMonClockStep;
} __attribute__ ((packed)) r_ncu_dataapp_tuple,  *lp_r_ncu_dataapp_tuple;

typedef struct tagR_ncu_dataapp_idx_tuple
{
    WORD64      ddwUPSeid;
    WORD32      dwSubAppid;
} __attribute__ ((packed)) r_ncu_dataapp_idx_tuple,  *lp_r_ncu_dataapp_idx_tuple;

typedef struct tagR_ncu_dataapp_nonidx_tuple
{
    WORD64      ddwUPSeid;
    WORD32      dwAppid;
} __attribute__ ((packed)) r_ncu_dataapp_nonidx_tuple,  *lp_r_ncu_dataapp_nonidx_tuple;

typedef struct tagR_ncu_dataapp_nonidx_AnaClockStep_tuple
{
   WORD64      ddwAnaClockStep;
} __attribute__ ((packed)) r_ncu_dataapp_nonidx_AnaAClockStep_tuple,  *lp_r_ncu_dataapp_nonidx_AnaAClockStep_tuple;

typedef struct tagR_ncu_dataapp_nonidx_AnaMonClockStep_tuple
{
   WORD64      ddwAnaMonClockStep;
} __attribute__ ((packed)) r_ncu_dataapp_nonidx_AnaMonClockStep_tuple,  *lp_r_ncu_dataapp_nonidx_AnaMonClockStep_tuple;

typedef struct tagR_ncu_dataapp_nonidx_ExpClockStep_tuple
{
   WORD64      ddwExpClockStep;
} __attribute__ ((packed)) r_ncu_dataapp_nonidx_ExpAClockStep_tuple,  *lp_r_ncu_dataapp_nonidx_ExpAClockStep_tuple;

BOOL create_r_ncu_dataApp(DWORD dwDbHandle);

T_psNcuDaAppCtx *psMcsGetDaAppCtxById(WORD32 dwCtxId, WORD32 hDB);
T_psNcuDaAppCtx*  psQueryDaAppCtxByUpseidSubAppid(WORD64 ddwUPSeid, WORD32 dwSubAppid, WORD32 hDB);
T_psNcuDaAppCtx*  psCrtDaAppCtxByUpseidSubAppid(WORD64 ddwUPSeid, WORD32 dwSubAppid, WORD32 hDB);
inline WORD32 psNcuMcsUpdDaAppCtxByAppid(WORD64 ddwUPSeid,WORD32 dwAppid,WORD32 dwCtxId,WORD32 hDB);
WORD32  psDelDaAppCtxByUpseidSubAppid(WORD64 ddwUPSeid,WORD32 dwSubAppid, WORD32 hDB);
WORD32 psVpfuMcsGetAllDaAppCtxByAppid(WORD64 ddwUPSeid,WORD32 dwAppid,WORD32 hDB,BYTE *pbQueryAckBuff);
inline WORD32 psNcuMcsUpdDaAppCtxByClockStep(WORD64 ddwClockStep, WORD32 hIdx, WORD32 dwCtxId,WORD32 hDB,BYTE method);
inline void psNcuMcsResetDaAppCtxByClockStep(WORD64 ddwClockStep, WORD32 hIdx, WORD32 dwCtxId,WORD32 hDB);
WORD32 psVpfuMcsGetAllDaAppCtxByClockStep(WORD64 ddwClockStep,WORD32 hDB, WORD32 hIdx, BYTE *pbQueryAckBuff);
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
