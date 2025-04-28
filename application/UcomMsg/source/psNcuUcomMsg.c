/******************************************************************************
版权所有 (C)2016, 深圳市中兴通讯股份有限公司

模块名          : MCS
文件名          : psNcuUcomMsg.c
相关文件        :
文件实现功能     : Ucom通道报文处理
作者            :
版本            : V1.0
-------------------------------------------------------------------------------
修改记录:
* 修改日期            版本号           修改人              修改内容
* 2024-04-29        V7.24.11         xxx             create
******************************************************************************/
/**************************************************************************
 *                               头文件(满足最小依赖)
 **************************************************************************/
#include "psNcuUcomMsg.h"
#include "MemShareCfg.h"
#include "ps_ncu_typedef.h"
#include "psNcuHeartBeatProc.h"
#include "psMcsDebug.h"
#include "psNcuSubscribeProc.h"
#include "psNcuDADialCtxProc.h"

// 非DDD目录头文件位置

/**************************************************************************
 *                              宏(本源文件使用)
 **************************************************************************/
typedef VOID (*pfType_psNcuUcomMsgProc)(T_MediaProcThreadPara *ptMediaProcThreadPara, BYTE *aucPacketBuf, T_psUpfUcomHead *ptMcsPfuUcomHead);
typedef VOID (*pfType_psNcuUcomMsgProcWithNoUcomHead)(T_MediaProcThreadPara *ptMediaProcThreadPara, BYTE *aucPacketBuf);

typedef struct
{
    WORD32 dwMsgType;
    pfType_psNcuUcomMsgProc pfMsgFunc;
    pfType_psNcuUcomMsgProcWithNoUcomHead pfMsgFuncWithNoUcomHead;
}T_NcuUcomMsgFunc;

/**************************************************************************
 *                              常量(本源文件使用)
 **************************************************************************/

/**************************************************************************
 *                              数据类型(本源文件使用)
 **************************************************************************/

/**************************************************************************
 *                              全局变量(本源文件使用)
 **************************************************************************/

/**************************************************************************
 *                              外部函数原型(评估后慎重添加)
 **************************************************************************/

/**************************************************************************
 *                              局部函数原型(本源文件使用)
 **************************************************************************/
VOID psMcsUcomMng2MediaPostMigOutReqProc(T_MediaProcThreadPara *ptMediaProcThreadPara, BYTE *aucPacketBuf);
VOID psMcsUcomNcu2McsDialCfgChangeProc(T_MediaProcThreadPara *ptMediaProcThreadPara, BYTE *aucPacketBuf, T_psUpfUcomHead *ptNcuUcomHead);
/**************************************************************************
 *                              函数实现(函数布局:总分结构|深度优先)
 **************************************************************************/

static T_NcuUcomMsgFunc tNcuUcomMsgFuncList[]=
{
    {EV_MSG_UCOM_NCU_JOB_TO_MCS_REQ,            psMcsUcomNcu2McsReqProc,   NULL},   
    {EV_MSG_MANAGE_TO_MEDIA_POST_MIG_OUT_REQ,   NULL,                      psMcsUcomMng2MediaPostMigOutReqProc},  
    {EV_MSG_UCOM_NCU_JOB_TO_MCS_DIALCHANGE,     psMcsUcomNcu2McsDialCfgChangeProc,   NULL},
}; 


VOID psNcuUcomMediaProc(T_MediaProcThreadPara *ptMediaProcThreadPara, BYTE *aucPacketBuf, T_psUpfUcomHead *ptNcuUcomHead)
{
    MCS_PCLINT_NULLPTR_RET_VOID(ptMediaProcThreadPara);
    MCS_PCLINT_NULLPTR_RET_VOID(aucPacketBuf);
    MCS_PCLINT_NULLPTR_RET_VOID(ptNcuUcomHead);

    WORD32 dwLoop = 0;
    WORD32 dwListLen = 0;    
    WORD32 dwMsgType = ptNcuUcomHead->dwMsgType;

    dwListLen = sizeof(tNcuUcomMsgFuncList)/sizeof(tNcuUcomMsgFuncList[0]);

    for(dwLoop = 0; dwLoop < dwListLen; dwLoop++)
    {
        if(dwMsgType == tNcuUcomMsgFuncList[dwLoop].dwMsgType)
        {
            if(NULL != tNcuUcomMsgFuncList[dwLoop].pfMsgFunc)
            {
                pfType_psNcuUcomMsgProc pfFunc = NULL;
                pfFunc = tNcuUcomMsgFuncList[dwLoop].pfMsgFunc;
                pfFunc(ptMediaProcThreadPara, aucPacketBuf, ptNcuUcomHead);
            }
            else if(NULL != tNcuUcomMsgFuncList[dwLoop].pfMsgFuncWithNoUcomHead)
            {
                pfType_psNcuUcomMsgProcWithNoUcomHead pfFunc = NULL;
                pfFunc = tNcuUcomMsgFuncList[dwLoop].pfMsgFuncWithNoUcomHead;
                pfFunc(ptMediaProcThreadPara, aucPacketBuf);
            }
            return;
        }
    }

    return;
}


VOID psMcsUcomNcu2McsReqProc(T_MediaProcThreadPara *ptMediaProcThreadPara, BYTE *aucPacketBuf, T_psUpfUcomHead *ptNcuUcomHead)
{
    MCS_PCLINT_NULLPTR_RET_VOID(ptMediaProcThreadPara);
    MCS_PCLINT_NULLPTR_RET_VOID(aucPacketBuf);
    MCS_PCLINT_NULLPTR_RET_VOID(ptNcuUcomHead);
    T_psNcuMcsPerform *ptNcuPerform = (T_psNcuMcsPerform *)ptMediaProcThreadPara->ptMcsStatPointer;
    MCS_PCLINT_NULLPTR_RET_VOID(ptNcuPerform);
    
    T_psNcuHeartHead *ptNcuHeartHead = (T_psNcuHeartHead *)aucPacketBuf;
    DEBUG_TRACE(DEBUG_LOW, "psMcsUcomNcu2McsReqProc: enter!\n");
    if(EV_MSG_UCOM_NCU_JOB_TO_MCS_REQ == ptNcuUcomHead->dwMsgType)
    {
        /* 主用链路请求发送 */
        DEBUG_TRACE(DEBUG_LOW, "psMcsUcomNcu2McsReqProc: Ucom Ncu to Mcs EV_MSG_UCOM_NCU_JOB_TO_MCS_REQ!\n");
        MCS_LOC_STAT_EX(ptNcuPerform, qwRcvUcomMsgHeartBeatCheckReq, 1);
        psNcuUcomSendHeartBeatCheckReq(ptMediaProcThreadPara,ptNcuHeartHead);
    }
    return;
}

/* Started by AICoder, pid:qccb9z98b4x429f145fc0ba63031d2177604eadb */
VOID psMcsUcomNcu2McsDialCfgChangeProc(T_MediaProcThreadPara *ptMediaProcThreadPara, BYTE *aucPacketBuf, T_psUpfUcomHead *ptNcuUcomHead)
{
    MCS_PCLINT_NULLPTR_RET_3ARG_VOID(ptMediaProcThreadPara, aucPacketBuf, ptNcuUcomHead); 
    DEBUG_TRACE(DEBUG_LOW, "psMcsUcomNcu2McsDialCfgChangeProc: enter!\n");
    T_psNcuMcsPerform *ptNcuPerform = (T_psNcuMcsPerform*)ptMediaProcThreadPara->ptMcsStatPointer;
    MCS_PCLINT_NULLPTR_RET_VOID(ptNcuPerform);

    MCS_LOC_STAT_EX(ptNcuPerform, qwRcvUcomMsgDialStatusChangeReq, 1);
    T_psNcuDialCfgChangeInfo *ptNcuDialCfgChangeInfo = (T_psNcuDialCfgChangeInfo *)aucPacketBuf;
    /* 拨测配置状态变更 触发上报 */
    WORD16 dwThreadNo = ptMediaProcThreadPara->bThreadNo;
    psNcuDialCfgChangeProc(ptNcuDialCfgChangeInfo, dwThreadNo);
    return;
}
/* Ended by AICoder, pid:qccb9z98b4x429f145fc0ba63031d2177604eadb */

VOID psMcsUcomMng2MediaPostMigOutReqProc(T_MediaProcThreadPara *ptMediaProcThreadPara, BYTE *aucPacketBuf)
{
    MCS_PCLINT_NULLPTR_RET_2ARG_VOID(ptMediaProcThreadPara, aucPacketBuf); 
    T_psNcuMcsPerform *ptNcuPerform = (T_psNcuMcsPerform*)ptMediaProcThreadPara->ptMcsStatPointer;
    MCS_PCLINT_NULLPTR_RET_VOID(ptNcuPerform);

    MCS_LOC_STAT_EX(ptNcuPerform, qwRcvUcomMsgSessDelByGroup, 1);
    T_psMcsSessDelByGroupArea *ptMsgBody = (T_psMcsSessDelByGroupArea *)aucPacketBuf;
    psNcuDelSessionByGroupNo(ptMediaProcThreadPara, ptMsgBody);
    
    return;
}

//end of file
