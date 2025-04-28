/* Started by AICoder, pid:r0f824bf05m2d0514f670b5e102e2f6e7ff33e80 */
/******************************************************************************
* 版权所有 (C)2016 深圳市中兴通讯股份有限公司*
* 模块名          : NCU
* 文件名          : ApplicationMapCfg.cpp
* 相关文件        :
* 文件实现功能     : applicationmap配置变更
* 归属团队        : M6
* 版本           : V1.0
-------------------------------------------------------------------------------
* 修改记录:
* 修改日期            版本号           修改人           修改内容
* 2024-05-11        V7.24.11.B5        WYA            create
******************************************************************************/

#include "ApplicationMapCfg.h"
#include "r_ncu_applicationmap.h"
#include "dbm_lib_upfcomm.h"
#include "dbmLibComm.h"
#include "ps_ncu_typedef.h"
#include "dbm_lib_upfncu.h"
#include "applicationmap.h"
#include "psNcuApplicationMapCtxProc.h"
void ApplicationMapCfg::powerOnProc()
{
    // TODO: Add your implementation code here.
}

void ApplicationMapCfg::cfgNotifyProc(WORD16 operation, WORD16 msgLen, BYTE *msg)
{
    if (NULL == msg)
    {
        zte_printf_s("\n[ConfigNotify]recv ApplicationMapCfg msg is NULL!\n");
        return;
    }

    switch (operation)
    {
        case _DB_CFGCHG_TUPLE_NOTIFY_INSERT:
        {
            if (msgLen != sizeof(APPLICATIONMAP_TUPLE))
            {
                return ;
            }
            BYTE *ptCtxAddr = NULL;
            LP_APPLICATIONMAP_TUPLE ptuple = (LP_APPLICATIONMAP_TUPLE)msg;
            psNcuCreatAppIDMapCtxByStrAppID(ptuple->application, ptuple->innerappid, &ptCtxAddr);
            break;
        }
        case _DB_CFGCHG_TUPLE_NOTIFY_MODIFY:
        {
            if (msgLen != 2 * sizeof(APPLICATIONMAP_TUPLE))
            {
                return ;
            }
            BYTE *ptCtxAddr = NULL;
            LP_APPLICATIONMAP_TUPLE pOldTuple = (LP_APPLICATIONMAP_TUPLE)msg;
            psNcuDelAppIDMapCtxByStrAppID(pOldTuple->application);

            LP_APPLICATIONMAP_TUPLE pNewTuple = (LP_APPLICATIONMAP_TUPLE)(msg + sizeof(APPLICATIONMAP_TUPLE));
            psNcuCreatAppIDMapCtxByStrAppID(pNewTuple->application, pNewTuple->innerappid, &ptCtxAddr);
            break;
        }
        case _DB_CFGCHG_TUPLE_NOTIFY_DELETE:
        {
            if (msgLen != sizeof(APPLICATIONMAP_TUPLE))
            {
                return ;
            }
            LP_APPLICATIONMAP_TUPLE ptuple = (LP_APPLICATIONMAP_TUPLE)msg;
            psNcuDelAppIDMapCtxByStrAppID(ptuple->application);
            break;
        }
        default:
        {
            zte_printf_s("invalid operation!!!\n");
            break;
        }
    }
}

void ApplicationMapCfg::show()
{
    // TODO: Add your implementation code here.
}

/* Ended by AICoder, pid:r0f824bf05m2d0514f670b5e102e2f6e7ff33e80 */
