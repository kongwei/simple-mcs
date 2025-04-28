/* Started by AICoder, pid:hc4b9ba777k732f1494509ebe0d4a1302b74bd5d */
/******************************************************************************
* 模块名          : NCU
* 文件名          : DaDialCfg.cpp
* 文件实现功能     : 数据分析拨测配置
******************************************************************************/
#include "DaDialCfg.h"
#include "ps_ncu_typedef.h"
#include "r_dadialcfg.h"
#include "ps_ncu_dataApp.h"
#include "psNcuApplicationMapCtxProc.h"
extern "C"
{
#include "psNcuDADialCtxProc.h"
#include "psMcsTestSend.h"
}
VOID insertDaDialByImsiSubapp(R_DADIALCFG_TUPLE *ptTuple);
VOID modDaDialByImsiSubapp(R_DADIALCFG_TUPLE *ptTuple);
VOID delDaDialByImsiSubapp(R_DADIALCFG_TUPLE *ptTuple);

void DaDialCfg::cfgNotifyProc(WORD16 operation, WORD16 msgLen, BYTE *msg)
{
    MCS_CHK_VOID(NULL == msg);
    R_DADIALCFG_TUPLE *ptupleNew = NULL;
    switch (operation)
    {
        case _DB_CFGCHG_TUPLE_NOTIFY_INSERT:
        {
            if (sizeof(R_DADIALCFG_TUPLE) != msgLen)
            {
                return ;
            }
            ptupleNew = (R_DADIALCFG_TUPLE *)msg;
            insertDaDialByImsiSubapp(ptupleNew);
            break;
        }
        case _DB_CFGCHG_TUPLE_NOTIFY_MODIFY:
        {
            if(2*sizeof(R_DADIALCFG_TUPLE) != msgLen)
            {
                return ;
            }
            ptupleNew = (R_DADIALCFG_TUPLE *)(msg + sizeof(R_DADIALCFG_TUPLE));
            modDaDialByImsiSubapp(ptupleNew);
            break;
        }
        case _DB_CFGCHG_TUPLE_NOTIFY_DELETE:
        {
            if(sizeof(R_DADIALCFG_TUPLE) != msgLen)
            {
                return;
            }
            ptupleNew = (R_DADIALCFG_TUPLE *)msg;
            delDaDialByImsiSubapp(ptupleNew);
            break;
        }
        default: return;
    }
}

void DaDialCfg::powerOnProc()
{
}

void DaDialCfg::show()
{
}

VOID insertDaDialByImsiSubapp(R_DADIALCFG_TUPLE *ptTuple)
{
    if(NULL == ptTuple)
    {
        return;
    }
    T_IMSI tIMSI = {0};
    if(FALSE == psUpfCommString2tBCD(tIMSI,(BYTE *)ptTuple->imsi,(BYTE)zte_strnlen_s(ptTuple->imsi, MAX_CHAR_IMSI_LEN)))
    {
        return;
    }
    BYTE bDaDialStatus = (QOSQUALITY_NORMAL == ptTuple->qosquality)?DA_QUALITY_GOOD:DA_QUALITY_POOR;
    psNcuAllocDaDialCtxByImsiSubApp(&tIMSI, ptTuple->subapp, bDaDialStatus);
    return;
}

VOID modDaDialByImsiSubapp(R_DADIALCFG_TUPLE *ptTuple)
{
    if(NULL == ptTuple)
    {
        return;
    }
    WORD32 dwSubAppid = 0;
    T_IMSI tIMSI = {0};
    if (FALSE == psNcuGetCfgInnerAppIdByStrAppidmap(ptTuple->subapp, &dwSubAppid))
    {
        return;
    }
    if(FALSE == psUpfCommString2tBCD(tIMSI,(BYTE *)ptTuple->imsi,(BYTE)zte_strnlen_s(ptTuple->imsi, MAX_CHAR_IMSI_LEN)))
    {
        return;
    }
    T_psNcuDADial *ptNcuDADialCtx = psQueryDADialCtxByImsiSubAppid(&tIMSI, dwSubAppid);
    if (NULL == ptNcuDADialCtx)
    {
        return ;
    }
    /* 配置状态变化 更新DaAppCtx拨测状态并上报新状态 */
    if(ptNcuDADialCtx->bQosQuality != ptTuple->qosquality)
    {
        BYTE bDaDialStatus = (QOSQUALITY_NORMAL == ptTuple->qosquality)?DA_QUALITY_GOOD:DA_QUALITY_POOR;
        ptNcuDADialCtx->bQosQuality = bDaDialStatus;
        /* 关联DaAppCtx */
        psNcuDaDialCtxRelateDaAppCtx(ptNcuDADialCtx, UPD_DADIALCTX);
    }
    return;
}

VOID delDaDialByImsiSubapp(R_DADIALCFG_TUPLE *ptTuple)
{
    if(NULL == ptTuple)
    {
        return;
    }
    WORD32 dwSubAppid = 0;
    T_IMSI tIMSI = {0};
    if (FALSE == psNcuGetCfgInnerAppIdByStrAppidmap(ptTuple->subapp, &dwSubAppid))
    {
        return;
    }
    if(FALSE == psUpfCommString2tBCD(tIMSI,(BYTE *)ptTuple->imsi,(BYTE)zte_strnlen_s(ptTuple->imsi, MAX_CHAR_IMSI_LEN)))
    {
        return;
    }
    T_psNcuDADial *ptNcuDADialCtx = psQueryDADialCtxByImsiSubAppid(&tIMSI, dwSubAppid);
    if (NULL == ptNcuDADialCtx)
    {
        return ;
    }
    /* 删除DaAppCtx的关联 */
    psNcuDaDialCtxRelateDaAppCtx(ptNcuDADialCtx,DEL_DADIALCTX);
    psDelDADialCtxByImsiSubAppid(&tIMSI, dwSubAppid);
    return;
}
// end of file
/* Ended by AICoder, pid:hc4b9ba777k732f1494509ebe0d4a1302b74bd5d */
