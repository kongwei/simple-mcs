/**********************************************************************
 *  ModuleName:     Stream Pub
 *  File Name:      ps_mcs_trace.h
 *  Description:    primary include file of stream pub module
 *  History:
 *  Date            Version     Modifier        Activies
 *  -------------------------------------------------------------------
 *  2024.04.12      1.0         wjm          create
 *********************************************************************/
#ifndef PS_MCS_TRACE_H
#define PS_MCS_TRACE_H

#ifdef __cplusplus
extern "C" {
#endif
/**************************************************************************
 *                              头文件(满足自包含)
 **************************************************************************/

//#include "r_sx_node.h"
#include "SigTrace.h"
#include "psMcsGlobalCfg.h"

/**************************************************************************
*                              宏(对外提供)
***************************************************************************/



#define M_OVERLOAD  1
#define M_NORMAL    0
#define SigTrace_NCU(ptSessionCtx, event, msg, msglen) \
do {\
    if (likely(0 == *pSigTraceTaskNum))  break;\
    if (unlikely(NULL == ptSessionCtx))  break;\
    T_SIGTRACE_MATCHLIST tmplist = {0};\
    T_SIGTRACE_MATCHINFO *pinfo = &ptSessionCtx->tSigTraceMatchOut;\
    if (unlikely(!SigTrace_PreMatch(&ptSessionCtx->tSigTraceMatchOut))) {\
        if(likely(!is_all_zero(ptSessionCtx->bImsi, 8))) {\
            tmplist.imsi=(T_IMSI *)(ptSessionCtx->bImsi);\
        }\
        if(likely(!is_all_zero(ptSessionCtx->bIsdn, 8))) {\
            tmplist.isdn=(T_MSISDN *)(ptSessionCtx->bIsdn);\
        }\
        tmplist.msgType=SIGTRACE_NCU_SESSION;\
        SigTrace_Match(&tmplist, &ptSessionCtx->tSigTraceMatchOut); \
        if (0 == pinfo->traceid[0]) break;} \
    else { \
        if (0 == pinfo->traceid[0]) break; \
        tmplist.msgType=SIGTRACE_NCU_SESSION;\
        if(likely(!is_all_zero(ptSessionCtx->bImsi, 8))) {\
            tmplist.imsi=(T_IMSI *)(ptSessionCtx->bImsi);\
        }\
        if(likely(!is_all_zero(ptSessionCtx->bIsdn, 8))) {\
            tmplist.isdn=(T_MSISDN *)(ptSessionCtx->bIsdn);\
        }\
    } \
    if(TRUE != Ncu_SigTrace_FlowCtrl()) break; \
    if(MCS_RET_FAIL == psNcuSigtraceOverLoadProc())break; \
    zte_memcpy_s(tmplist.upfName, LEN_R_UPASSOCADDR_UPNAME_MAX+1, g_upfConfig.daUpfIpCfg.upfName, LEN_R_UPASSOCADDR_UPNAME_MAX+1);  \
    SigTrace_NCU_Report(pinfo, &tmplist, SIGTRACE_INTERFACE_NNWDAF_UDP, SIGTRACE_DIRECT_SEND, event, msg, msglen);\
} while (0)
/* SBI上报发消息给UPM再转到HTTPLB之前，先匹配一次，不进行跟踪 */
#define SigTrace_NCU_SBI(ptSessionCtx, msgdata, msglen) \
do {\
    if (likely(0 == *pSigTraceTaskNum))  break;\
    if (unlikely(NULL == ptSessionCtx))  break;\
    T_SIGTRACE_MATCHLIST tmplist = {0};\
    T_SIGTRACE_MATCHINFO *pinfo = &ptSessionCtx->tSigTraceMatchOut;\
    if (unlikely(!SigTrace_PreMatch(&ptSessionCtx->tSigTraceMatchOut))) {\
        tmplist.imsi=(T_IMSI *)(ptSessionCtx->bImsi);\
        tmplist.isdn=(T_MSISDN *)(ptSessionCtx->bIsdn);\
        tmplist.msgType=SIGTRACE_NCU_SESSION;\
        SigTrace_Match(&tmplist, &ptSessionCtx->tSigTraceMatchOut); \
        if (0 == pinfo->traceid[0]) break;} \
} while (0)

/* SBI上报从HTTPLB回来后再做预匹配成功，在job上再查会话进行跟踪,如果预匹配不成功，也不需要进行匹配了 */
#define SigTrace_NCU_SBI_Job(ptSessionCtx, msgdata, msglen, dwMsgID, bDirect) \
do {\
    if (likely(0 == *pSigTraceTaskNum))  break;\
    if (unlikely(NULL == ptSessionCtx))  break;\
    T_SIGTRACE_MATCHLIST tmplist = {0};\
    T_SIGTRACE_MATCHINFO *pinfo = &ptSessionCtx->tSigTraceMatchOut;\
    if (unlikely(!SigTrace_PreMatch(&ptSessionCtx->tSigTraceMatchOut))) {\
         break;} \
    else { \
        if (0 == pinfo->traceid[0]) break; \
        tmplist.msgType=SIGTRACE_NCU_SESSION;\
        if(likely(!is_all_zero(ptSessionCtx->bImsi, 8))) {\
            tmplist.imsi=(T_IMSI *)(ptSessionCtx->bImsi);\
        }\
        if(likely(!is_all_zero(ptSessionCtx->bIsdn, 8))) {\
            tmplist.isdn=(T_MSISDN *)(ptSessionCtx->bIsdn);\
        }\
    } \
    if(TRUE != NcuJob_SigTrace_FlowCtrl()) break; \
    if(MCS_RET_FAIL == psNcuJobSigtraceOverLoadProc())break; \
    zte_memcpy_s(tmplist.upfName, LEN_R_UPASSOCADDR_UPNAME_MAX+1, g_upfConfig.daUpfIpCfg.upfName, LEN_R_UPASSOCADDR_UPNAME_MAX+1);  \
    T_NrfTraceMsg tNrfTraceMsg = {0};\
    tNrfTraceMsg.direct = bDirect;\
    tNrfTraceMsg.msgId = dwMsgID;\
    NcuSbiSigTrace(msgdata, msglen, &tNrfTraceMsg);\
    SigTrace_Proc_Sbi_Report(pinfo, &tmplist, &tNrfTraceMsg);\
} while (0)



#define SigTrace_NCU_Node(direct, event, msg, msglen) \
do {\
    if (likely(0 == *pSigTraceTaskNum)) break;\
    T_SIGTRACE_MATCHLIST tmplist = {0};\
    T_SIGTRACE_MATCHINFO tmpinfo = {0};\
    tmplist.msgType=SIGTRACE_NCU_NODE;\
    SigTrace_Match(&tmplist, &tmpinfo);\
    if(TRUE != NcuJob_SigTrace_FlowCtrl()) break; \
    if(MCS_RET_FAIL == psNcuJobSigtraceOverLoadProc())break; \
    zte_memcpy_s(tmplist.upfName, LEN_R_UPASSOCADDR_UPNAME_MAX+1, g_upfConfig.daUpfIpCfg.upfName, LEN_R_UPASSOCADDR_UPNAME_MAX+1);  \
    SigTrace_NCU_Report(&tmpinfo, &tmplist, SIGTRACE_INTERFACE_NNWDAF_UDP, direct, event, msg, msglen);\
} while (0)

/**************失败观察 start***************/
#define FailTrace_NCU(ptSessionCtx, event, failcode, msg, msglen, pFormat, ...) \
do { \
    if (likely(0 == *pFailTraceTaskNum)) break; \
    if(MCS_RET_FAIL == psNcuFailtraceOverLoadProc())break; \
    /* 简单限速(实现放trace下) */ \
    T_SIGTRACE_MATCHLIST tFailmatchlist = {0}; \
    if(likely(NULL != ptSessionCtx))\
    {\
        if(likely(!is_all_zero(ptSessionCtx->bImsi, 8))) {\
            tFailmatchlist.imsi=(T_IMSI *)(ptSessionCtx->bImsi);\
        }\
        if(likely(!is_all_zero(ptSessionCtx->bIsdn, 8))) {\
            tFailmatchlist.isdn=(T_MSISDN *)(ptSessionCtx->bIsdn);\
        }\
    }\
    FailTrace_NCU_Proc(&tFailmatchlist, FO_INTERFACE_NUPF, event, failcode, msg, msglen, pFormat, ##__VA_ARGS__); \
} while (0)


/**************失败观察 end***************/

/**************************************************************************
 *                              数据类型(对外提供)
 **************************************************************************/
typedef struct
{
    BYTE     bState;            /*  负荷状态机，0是normal，1是overload */
    BYTE     bResv[7];
}T_pfuTraceInfo;

extern WORD32  *pDataTraceTaskNum;


/**************************************************************************
 *                              接口(对外提供，最小可见)
 **************************************************************************/
WORD32 psNcuSigtraceOverLoadProc();
BOOLEAN Ncu_SigTrace_FlowCtrl();
WORD32 psNcuJobSigtraceOverLoadProc();
BOOLEAN NcuJob_SigTrace_FlowCtrl();
BOOL is_all_zero(BYTE array[], BYTE length);

#ifdef __cplusplus
}
#endif
#endif

