/******************************************************************************
版权所有 (C)2016, 深圳市中兴通讯股份有限公司

模块名          : MCS
文件名          : psNcuSigTrace.c
相关文件         :
文件实现功能       : NCU信令跟踪
作者           : wjm
版本           : V1.0
-------------------------------------------------------------------------------
修改记录:
日  期                 版本             修改人             修改内容
* 2024-04-22         V7.23.20         M7             modify
******************************************************************************/
/**************************************************************************
 *                               头文件(满足最小依赖)
 **************************************************************************/
#include "ps_mcs_trace.h"
#include "psNcuSigTrace.h"
#include "MemShareCfg.h"
#include "ps_p4.h"
#include "psMcsGlobalCfg.h"
#include "ps_mcs_define.h"
#include "ps_ncu_typedef.h"
//#include "HTTP_LB/nghttp2codec/HTTPCodec.h"
#include "psMcsDebug.h"
#include "psNefReportToNwdaf.h"
#include "ncuSCCUAbility.h"

/**************************************************************************
 *                              宏(本源文件使用)
 **************************************************************************/

#define SIG_FLOWCTRL_GRAN       1000           /* ms,信令跟踪流控粒度 */

/**************************************************************************
 *                              常量(本源文件使用)
 **************************************************************************/


/**************************************************************************
 *                              数据类型(本源文件使用)
 **************************************************************************/



/**************************************************************************
 *                              全局变量(本源文件使用)
 **************************************************************************/
extern T_UpfSigTraceCfg g_sigtracecfg;
extern __thread T_MediaProcThreadPara* g_ptMediaProcThreadPara;
T_pfuTraceInfo sigTrace = {0};
extern WORD64 g_w64McsMaxCpuLoad;
WORD32 g_MasterSCNum   = 0;

/**************************************************************************
 *                              外部函数原型(评估后慎重添加)
 **************************************************************************/


/**************************************************************************
 *                              局部函数原型(本源文件使用)
 **************************************************************************/
#define _VGW_CSS_SIMU_GNU_
static inline void psGwMcsAtomicAdd64Stat(atomic_t_64 *ptAtomic, WORD32 dwNum)
{
    /* MOS_POINTER_CHECK (ptAtomic); */
    #ifndef  _VGW_CSS_SIMU_GNU_
    atomic_add_unsigned_64((int64_t)dwNum, (rte_atomic64_t *)ptAtomic);
    #else
    atomic_add_64(dwNum,ptAtomic);
    #endif
    return;
}


/**************************************************************************
 *                              函数实现(函数布局:总分结构|深度优先)
 **************************************************************************/

WORD32 psNcuSigtraceOverLoadProc()
{
    T_MediaProcThreadPara *ptMediaProcThreadPara = g_ptMediaProcThreadPara;
    MCS_PCLINT_NULLPTR_RET_1ARG(ptMediaProcThreadPara, FALSE);
    T_psNcuMcsPerform *ptMcsNcuPerform = (T_psNcuMcsPerform*)ptMediaProcThreadPara->ptMcsStatPointer;
    MCS_PCLINT_NULLPTR_RET_1ARG(ptMcsNcuPerform, FALSE);
    MCS_CHK_RET(0 == g_sigtracecfg.bSigStopOnOverLoad, MCS_RET_SUCCESS);

    WORD64 dwMaxCpuload = g_w64McsMaxCpuLoad;
    if(M_OVERLOAD == sigTrace.bState)
    {
        if(dwMaxCpuload < g_sigtracecfg.bSigRecoveryThreshold) 
        {
            sigTrace.bState = M_NORMAL;
        }
        else
        {
            MCS_LOC_STAT_EX(ptMcsNcuPerform, qwSigOverLoadDrop, 1);
            return MCS_RET_FAIL;
        }
    }
    else 
    {
        if(dwMaxCpuload > g_sigtracecfg.bSigStopThreshold) 
        {
            sigTrace.bState = M_OVERLOAD;
            MCS_LOC_STAT_EX(ptMcsNcuPerform, qwSigOverLoadDrop, 1);
            return MCS_RET_FAIL;
        }
    }
    return MCS_RET_SUCCESS;
}


/*信令跟踪流控*/
BOOLEAN Ncu_SigTrace_FlowCtrl()
{
    T_MediaProcThreadPara *ptMediaProcThreadPara = g_ptMediaProcThreadPara;
    MCS_PCLINT_NULLPTR_RET_1ARG(ptMediaProcThreadPara, FALSE);
    T_psNcuMcsPerform *ptMcsNcuPerform = (T_psNcuMcsPerform*)ptMediaProcThreadPara->ptMcsStatPointer;
    MCS_PCLINT_NULLPTR_RET_1ARG(ptMcsNcuPerform, FALSE);
    WORD32 wSigTraceRate;
    WORD64 dwCurTime                       = 0;
    WORD64 *pdwBttmTime                    = NULL;
    WORD64 *pqwRcvSTrcPktTotalNum          = NULL;
    if(0 == g_MasterSCNum)
    {
        g_MasterSCNum=ncuGetMasterScNum();
    }
    WORD32 dwMasterSCNum = g_MasterSCNum;
    _mcs_if(0 != dwMasterSCNum)
    {
        wSigTraceRate = g_sigtracecfg.MaxReportPPS / dwMasterSCNum; 

        pqwRcvSTrcPktTotalNum = &g_ptVpfuShMemVar->tShMemSigTrcTb.qwSigTrcTotalNum;
        pdwBttmTime = &g_ptVpfuShMemVar->tShMemSigTrcTb.dwSigTrcBttmTime;
        if (0 == *pdwBttmTime)
        {
            *pdwBttmTime = psFtmGetLocalPowerOnMilliSec(); //psVpfuMcsGetStdUs(0);
        }
        dwCurTime = psFtmGetLocalPowerOnMilliSec();//psVpfuMcsGetStdUs(0);
        psGwMcsAtomicAdd64Stat((atomic_t_64 *)pqwRcvSTrcPktTotalNum, 1);
        if (*pqwRcvSTrcPktTotalNum >= wSigTraceRate)
        {
            if (dwCurTime - *pdwBttmTime >= (SIG_FLOWCTRL_GRAN))
            {
                *pqwRcvSTrcPktTotalNum = 0;
                *pdwBttmTime = psFtmGetLocalPowerOnMilliSec();
            }
            else
            {
                MCS_LOC_STAT_EX(ptMcsNcuPerform, qwOutLimDropSigNum, 1);
                return FALSE;
            }
        }
    }

    return TRUE;
}

WORD32 psNcuJobSigtraceOverLoadProc()
{
    MCS_CHK_RET(0 == g_sigtracecfg.bSigStopOnOverLoad, MCS_RET_SUCCESS);

    WORD64 dwMaxCpuload = g_w64McsMaxCpuLoad;
    if(M_OVERLOAD == sigTrace.bState)
    {
        if(dwMaxCpuload < g_sigtracecfg.bSigRecoveryThreshold) 
        {
            sigTrace.bState = M_NORMAL;
        }
        else
        {
            DEBUG_TRACE(DEBUG_LOW, "qwSigOverLoadDrop\n");
            return MCS_RET_FAIL;
        }
    }
    else 
    {
        if(dwMaxCpuload > g_sigtracecfg.bSigStopThreshold) 
        {
            sigTrace.bState = M_OVERLOAD;
            DEBUG_TRACE(DEBUG_LOW, "qwSigOverLoadDrop\n");
            return MCS_RET_FAIL;
        }
    }
    return MCS_RET_SUCCESS;
}


/*信令跟踪流控*/
BOOLEAN NcuJob_SigTrace_FlowCtrl()
{
    WORD32 wSigTraceRate;
    WORD64 dwCurTime                       = 0;
    WORD64 *pdwBttmTime                    = NULL;
    WORD64 *pqwRcvSTrcPktTotalNum          = NULL;
    if(0 == g_MasterSCNum)
    {
        g_MasterSCNum=ncuGetMasterScNum();
    }
    WORD32 dwMasterSCNum = g_MasterSCNum;
    _mcs_if(0 != dwMasterSCNum)
    {
        wSigTraceRate = g_sigtracecfg.MaxReportPPS / dwMasterSCNum;

        pqwRcvSTrcPktTotalNum = &g_ptVpfuShMemVar->tShMemSigTrcTb.qwSigTrcTotalNum;
        pdwBttmTime = &g_ptVpfuShMemVar->tShMemSigTrcTb.dwSigTrcBttmTime;
        if (0 == *pdwBttmTime)
        {
            *pdwBttmTime = psFtmGetLocalPowerOnMilliSec(); //psVpfuMcsGetStdUs(0);
        }
        dwCurTime = psFtmGetLocalPowerOnMilliSec();//psVpfuMcsGetStdUs(0);
        psGwMcsAtomicAdd64Stat((atomic_t_64 *)pqwRcvSTrcPktTotalNum, 1);
        if (*pqwRcvSTrcPktTotalNum >= wSigTraceRate)
        {
            if (dwCurTime - *pdwBttmTime >= (SIG_FLOWCTRL_GRAN))
            {
                *pqwRcvSTrcPktTotalNum = 0;
                *pdwBttmTime = psFtmGetLocalPowerOnMilliSec();
            }
            else
            {
                DEBUG_TRACE(DEBUG_LOW, "qwOutLimDropSigNum\n");
                return FALSE;
            }
        }
    }
    return TRUE;
}

BOOL is_all_zero(BYTE array[], BYTE length)
{
    BYTE i = 0;
    for (i = 0; i < length; i++)
    {
        if (array[i] != 0) {
            return FALSE; 
        }
    }
    return TRUE;
}

/* 信令跟踪，c for rust适配接口*/
VOID sigtrace_udp_report(T_psNcuSessionCtx* ptSessionCtx, BYTE event,const BYTE *msg, WORD16 msglen)
{
    SigTrace_NCU(ptSessionCtx, event, msg, msglen);
}

/* 信令跟踪，c for rust适配接口*/
VOID sigtrace_sbi_report(T_psNcuSessionCtx* ptSessionCtx)
{
    //该接口仅做预匹配, msglen没有使用
    SigTrace_NCU_SBI(ptSessionCtx, NULL, 0);
}

/* 信令跟踪，c for rust适配接口*/
VOID sigtrace_node_report(BYTE direct, BYTE event, const BYTE *msg, WORD16 msglen)
{
    SigTrace_NCU_Node(direct, event, msg, msglen);
}