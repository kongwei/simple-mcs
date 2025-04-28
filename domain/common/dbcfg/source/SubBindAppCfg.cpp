/* Started by AICoder, pid:k13bd6500dxe72d14161080af017c63921249c75 */
/******************************************************************************
* 版权所有 (C)2016 深圳市中兴通讯股份有限公司*
* 模块名          : MCS
* 文件名          : SubBindAppCfg.cpp
* 相关文件        :
* 文件实现功能     : 数据分析子应用绑定应用配置
* 归属团队        : M6
* 版本           : V7.24.30
-------------------------------------------------------------------------------
* 修改记录:
* 修改日期            版本号           修改人           修改内容
* 2024-03-14        V7.24.30           wya            create
******************************************************************************/
#include "SubBindAppCfg.h"
#include "r_subbindapp.h"
#include "dbm_lib_upfcomm.h"
#include "r_ncu_subbindapp.h"
#include "psNcuSubBindAppCtxProc.h"
#include "ps_ncu_typedef.h"
#include "r_ncu_applicationmap.h"
#include "psNcuApplicationMapCtxProc.h"
VOID insertSubBindAppBySubApp(R_SUBBINDAPP_TUPLE *ptTuple);
VOID modSubBindAppBySubApp(R_SUBBINDAPP_TUPLE *ptTuple);
VOID delSubBindAppBySubApp(R_SUBBINDAPP_TUPLE *ptTuple);

void SubBindAppCfg::cfgNotifyProc(WORD16 operation, WORD16 msgLen, BYTE *msg)
{
    MCS_CHK_VOID(NULL == msg);

    R_SUBBINDAPP_TUPLE            *ptupleNew = NULL;
    switch (operation)
    {
        case _DB_CFGCHG_TUPLE_NOTIFY_INSERT:
        {
            if (sizeof(R_SUBBINDAPP_TUPLE) != msgLen)
            {
                return ;
            }
            ptupleNew = (R_SUBBINDAPP_TUPLE *)msg;
            insertSubBindAppBySubApp(ptupleNew);
            break;
        }
        case _DB_CFGCHG_TUPLE_NOTIFY_MODIFY:
        {
            if (2*sizeof(R_SUBBINDAPP_TUPLE) != msgLen)
            {
                return ;
            }
            ptupleNew = (R_SUBBINDAPP_TUPLE *)(msg + sizeof(R_SUBBINDAPP_TUPLE));
            modSubBindAppBySubApp(ptupleNew);
            break;
        }
        case _DB_CFGCHG_TUPLE_NOTIFY_DELETE:
        {
            if (sizeof(R_SUBBINDAPP_TUPLE) != msgLen)
            {
                return ;
            }
            ptupleNew = (R_SUBBINDAPP_TUPLE *)msg;
            delSubBindAppBySubApp(ptupleNew);
            break;
        }
        default:
        {
            break;
        }
    }
}

void SubBindAppCfg::powerOnProc()
{
}

void SubBindAppCfg::show()
{
}

VOID insertSubBindAppBySubApp(R_SUBBINDAPP_TUPLE *ptTuple)
{
    if (NULL == ptTuple) 
    {
        return ;
    }
    allocSubBindAppBySubApp(ptTuple->subapp, ptTuple->application);
}

/* Started by AICoder, pid:81dc87b5ealed3e14fee0a812061f92dd356abe6 */
VOID modSubBindAppBySubApp(R_SUBBINDAPP_TUPLE *ptTuple)
{
    if (NULL == ptTuple) 
    {
        return ;
    }
    WORD32 dwSubAppid = 0;
    WORD32    dwAppid = 0;

    if (FALSE == psNcuGetCfgInnerAppIdByStrAppidmap(ptTuple->subapp, &dwSubAppid))
    {
        return;
    }
    T_psNcuAppidRelation *ptNcuAppidRelt = psQueryAppidRelateCtxBySubAppid(dwSubAppid);
    if (NULL == ptNcuAppidRelt)
    {
        return ;
    }
    zte_memcpy_s(ptNcuAppidRelt->appidStr, sizeof(ptNcuAppidRelt->appidStr), ptTuple->application, sizeof(ptNcuAppidRelt->appidStr));
    if (FALSE == psNcuGetCfgInnerAppIdByStrAppidmap(ptTuple->application, &dwAppid))
    {
        return;
    }
    ptNcuAppidRelt->dwAppid = dwAppid;
    psNcuMcsUpdAppidRelateCtxByAppid(dwAppid, ptNcuAppidRelt->dwCtxId);
}
/* Ended by AICoder, pid:81dc87b5ealed3e14fee0a812061f92dd356abe6 */

VOID delSubBindAppBySubApp(R_SUBBINDAPP_TUPLE *ptTuple)
{
    if (NULL == ptTuple) 
    {
        return ;
    }
    WORD32 dwSubAppid = 0;

    if (FALSE == psNcuGetCfgInnerAppIdByStrAppidmap(ptTuple->subapp, &dwSubAppid))
    {
        return;
    }
    psDelAppidRelateCtxBySubAppid(dwSubAppid);
}
// end of file
/* Ended by AICoder, pid:k13bd6500dxe72d14161080af017c63921249c75 */
