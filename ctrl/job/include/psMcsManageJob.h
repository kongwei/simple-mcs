/******************************************************************************
版权所有 (C)2003, 深圳市中兴通讯股份有限公司

模块名          : MCS
文件名          : psVpfuMcsManageJob.h
相关文件        :
文件实现功能    : 媒体面管理Job头文件
作者            :
版本            : V1.0
-------------------------------------------------------------------------------
修改记录:
日  期      版本        修改人      修改内容
2012.01.20  V1.0                    新建
******************************************************************************/
/*lint -save -e10*/
/*lint -save -e537*/

#ifndef _PS_VPFU_MCS_MANAGEJOB_H_
#define _PS_VPFU_MCS_MANAGEJOB_H_

/**************************************************************************
 *                            其它条件编译选项                            *
***************************************************************************/
#ifdef __cplusplus  
extern "C" {
#endif


/**************************************************************************
 *                           标准、非标准头文件                           *
***************************************************************************/
#include "tulip.h"
/**************************************************************************
 *                                  常量                                  *
 **************************************************************************/


/**************************************************************************
 *                                 宏定义                                 *
 **************************************************************************/
#define MCS_MANAGEJOB_UPDATE_SN_NOAGAIN         0
/* 定时器消息 */
#define EV_TIMER_TRIGGER_MAXGROUPID             EV_TIMER_2


/* 定时器参数 */
#define TIMER_NCU_NWDAF_HEARTBEAT_CHACK                  TIMER_NO_3
#define TIMER_NCU_TO_NCU_SYN_NWDAFIPLIST                 TIMER_NO_4

/* 定时器消息 */
#define EV_TIMER_NCU_NWDAF_HEARTBEAT_CHACK               EV_TIMER_3
#define EV_TIMER_NCU_TO_NCU_SYN_NWDAFIPLIST              EV_TIMER_4

#define EV_TIMER_GET_LICENSE_CHECK                       EV_TIMER_19
#define TIMER_GET_LICENSE_CHECK                          TIMER_NO_19

/**************************************************************************
 *                                数据类型                                *
 **************************************************************************/

typedef struct tagT_psManageJobPriData
{
    DWORD dwJobPowerOnReqTimerId;
    DWORD dw1000MsTimerId; /* 1000ms 基本定时器 */
    DWORD dwReportPmTimerId;
    DWORD dwUpdateSNTimerId;
    DWORD dwIPSegRspTimerId;
    DWORD dwSmartcfgRspTimerId;
    DWORD dwGTPNodeListTimerId;
    DWORD dwFWBuildTimerId;
    DWORD dwPFURegisterTimerId;
    DWORD dwUpmClientInitTimerId;
    DWORD dwRewriteTimerId;

    DWORD dwWholeRouteSynTimes;
    DWORD dwWholeRouteSynSeqNo;
    DWORD dwGtpIpIsPub;
    DWORD dwRewriteResendTimerId;

    DWORD dwRewriteV6TimerId;
    DWORD dwRewriteV6ResendTimerId;

    DWORD dwLastFramedRouteChkTupleNo;
    DWORD dwLastStaticRouteChkTupleNo;
    DWORD dwLastSpecifyDispChkTupleNo;
    DWORD dwPfuMigInTCETraceTimerId;

    DWORD dwAlgScanTimerId;

    /*lint -e754*/
    BYTE abRsv[3];
    /*lint +e754*/
    BYTE bUpdateSNThrdNum; /* 收到更新消息后, 当前已经更新SN的现场个数 */
    DWORD dwUpdateSNAgain; /* 更新所有线程SN后, 重新再更新一遍 */

    BYTE   abMsgSendBuffer[36000];
	DWORD dwLastFramedIpv6RouteChkTupleNo;

    BYTE  bTempAssignFlag;                  /* 温度初始化赋值标志位 */
    BYTE  bTemperatureAlarmFlag;            /* 告警标志位      0-低温告警      1-高温告警*/
    DWORD dwTemperatureHighCount;           /* 高温告警计数 */  
    DWORD dwTemperatureLowCount;            /* 低温告警计数 */  
    DWORD dwTemperatureHighRstCount;        /* 高温告警恢复计数 */
    DWORD dwTemperatureLowRstCount;         /* 低温告警恢复计数 */

    WORD32 dwLinkCheckTimerId;
    WORD32 dwLinkCheckTime;
} T_psManageJobPriData;
/**************************************************************************
 *                              全局变量声明                              *
 **************************************************************************/


/**************************************************************************
 *                              全局函数声明                              *
 **************************************************************************/


/************************************************************************/
/*                       函数声明                                       */
/************************************************************************/
VOID psVpfuMcsManageJobEntry(WORD16 wState, WORD32 dwMsgId, VOID *pMsgBody, VOID *pPData, BOOLEAN bSame);

#ifdef __cplusplus
}
#endif

#endif
