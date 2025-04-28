/* Started by AICoder, pid:47397x926cg855a14d8a0bd9c01eb0583dc0bbfc */
/******************************************************************************
 * 版权所有 (C)2016 深圳市中兴通讯股份有限公司*
 * 模块名          : MCS
 * 文件名          : psNcuSCCUSCListChgNotify.c
 * 相关文件        :
 * 文件实现功能     : NCU配置变更通知流程
 * 归属团队        : M6
 * 版本           : V7.24.20
-------------------------------------------------------------------------------
* 修改记录:
* 修改日期            版本号           修改人           修改内容
* 2024-04-14        V7.24.20        quan.yanfeng            create
******************************************************************************/
/**************************************************************************
 *                              头文件(满足最小依赖请按照DDD分层架构逐层依赖)
 **************************************************************************/
#include "psNcuSCCUSCListChgNotify.h"
// application    层依赖
#include "upfSc.h"
#include "psUpfSCTypes.h"
#include "psUpfJobTypes.h"
#include "psUpfAbility.h"
#include "ncuSCCUAbility.h"
#include "psMcsDebug.h"
// 非DDD目录依赖

/**************************************************************************
 *                              宏(本源文件使用)
 **************************************************************************/

/**************************************************************************
 *                              常量(本源文件使用)
 **************************************************************************/

/**************************************************************************
 *                              数据类型(本源文件使用)
 **************************************************************************/

/**************************************************************************
 *                              外部函数原型(评估后慎重添加)
 **************************************************************************/

/**************************************************************************
 *                              局部函数原型(本源文件使用)
 **************************************************************************/
void ncuSCCUSCListChgNotifyProc(T_SCCUSCListChgNotifyInfo *pInfo);

/**************************************************************************
 *                              全局变量(本源文件使用)
 **************************************************************************/

/**************************************************************************
 *                              函数实现(函数布局:总分结构|深度优先)
 **************************************************************************/

/*向SCCUServer注册group和sc状态变更回调函数*/
BOOL ncuSCCUInitSCInfo()
{
    if(SCCU_OK != sccuRegSelfSCListChgNotify(ncuSCCUSCListChgNotifyProc))
    {
        return FALSE;
    }
    return TRUE;
}

void ncuSCCUSCListChgNotifyProc(T_SCCUSCListChgNotifyInfo *pInfo)
{
    if(NULL == pInfo)
    {
        return;
    }
    WORD32 dwSelfLogicNo  =  ncuGetScLogicNo();
    if(dwSelfLogicNo != pInfo->SCLogicNo)
    {
        DEBUG_TRACE(DEBUG_LOW,"[ncuSCCUSCListChgNotifyProc]no self LogicNo return! dwSelfLogicNo=%u,pInfo->SCLogicNo=%u  \n", dwSelfLogicNo, pInfo->SCLogicNo);
        return;
    }
    DEBUG_TRACE(DEBUG_LOW, "[ncuSCCUSCListChgNotifyProc]Send msg start! bOpType=%u \n",pInfo->bOpType);
    switch(pInfo->bOpType)
    {
        case SCLIST_CHG_TYPE_UPDATE:
        {
            DWORD dwStatus = pInfo->tNewSCRecord.dwStatus;
            if(SC_STATE_EQUE_BLOCK(dwStatus))
            {
            }
            break;
        }
        default:
            break;
    }

}

/* Ended by AICoder, pid:47397x926cg855a14d8a0bd9c01eb0583dc0bbfc */
