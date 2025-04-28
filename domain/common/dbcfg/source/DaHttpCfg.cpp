#include "psMcsGlobalCfg.h"
#include "DaHttpCfg.h"
#define USE_DM_UPFNCU_GETR_DAHTTPCFG
#include "dbm_lib_upfncu.h"
#include "dbmLibComm.h"
#include "zte_slibc.h"

void DaHttpCfg::powerOnProc()
{
    BOOLEAN bflag = FALSE;
    DM_UPFNCU_GETR_DAHTTPCFG_REQ req = {0};
    DM_UPFNCU_GETR_DAHTTPCFG_ACK ack = {0};

    req.msgType = MSG_CALL;
    req.dwgindex= 1;
    ack.retCODE = RC_DBM_ERROR;
    bflag = DBM_CALL(DM_UPFNCU_GETR_DAHTTPCFG, (LPSTR)&req, (LPSTR)&ack);
    if( RC_DBM_OK != ack.retCODE || (TRUE != bflag))
    {
        g_upfConfig.daHttpCfg.bCheckSwitch         = 1;
        g_upfConfig.daHttpCfg.bCheckFailurePolicy  = 1;
        g_upfConfig.daHttpCfg.bReportPolicy        = 2;
        g_upfConfig.daHttpCfg.dwCheckTime          = 5;
        g_upfConfig.daHttpCfg.dwAgingTime          = 30;
        return;
    }

    g_upfConfig.daHttpCfg.bCheckSwitch         = ack.checkswitch;
    g_upfConfig.daHttpCfg.bCheckFailurePolicy  = ack.checkfailurepolicy;
    g_upfConfig.daHttpCfg.bReportPolicy        = ack.reportpolicy;
    g_upfConfig.daHttpCfg.dwCheckTime          = ack.checktime;
    g_upfConfig.daHttpCfg.dwAgingTime          = ack.agingtime;
    return;
}

/* Started by AICoder, pid:t52651e3dambb4b142810ac5108123302be2142a */
void DaHttpCfg::cfgNotifyProc(WORD16 operation, WORD16 msgLen, BYTE *msg)
{
    if(NULL == msg)
    {
        return;
    }

    R_DAHTTPCFG_TUPLE *ptupleNew = NULL;
    switch (operation)
    {
        case _DB_CFGCHG_TUPLE_NOTIFY_INSERT:
        {
            if(sizeof(R_DAHTTPCFG_TUPLE) > msgLen)
            {
                return ;
            }
            ptupleNew = (R_DAHTTPCFG_TUPLE*)msg;
            break;
        }
        case _DB_CFGCHG_TUPLE_NOTIFY_MODIFY:
        {
            if(2*sizeof(R_DAHTTPCFG_TUPLE) > msgLen)
            {
                return ;
            }
            ptupleNew = (R_DAHTTPCFG_TUPLE*)(msg + sizeof(R_DAHTTPCFG_TUPLE));
            break;
        }
        default:
        {
            break;
        }
    }

    if( NULL != ptupleNew )
    {
        g_upfConfig.daHttpCfg.bCheckSwitch         = ptupleNew->checkswitch;
        g_upfConfig.daHttpCfg.bCheckFailurePolicy  = ptupleNew->checkfailurepolicy;
        g_upfConfig.daHttpCfg.bReportPolicy        = ptupleNew->reportpolicy;
        g_upfConfig.daHttpCfg.dwCheckTime          = ptupleNew->checktime;
        g_upfConfig.daHttpCfg.dwAgingTime          = ptupleNew->agingtime;
    }

    return;
}
/* Ended by AICoder, pid:t52651e3dambb4b142810ac5108123302be2142a */

void DaHttpCfg::show()
{
    zte_printf_s("g_upfConfig.daHttpCfg.bCheckSwitch         = %u\n", g_upfConfig.daHttpCfg.bCheckSwitch);
    zte_printf_s("g_upfConfig.daHttpCfg.bCheckFailurePolicy  = %u\n", g_upfConfig.daHttpCfg.bCheckFailurePolicy);
    zte_printf_s("g_upfConfig.daHttpCfg.bReportPolicy        = %u\n", g_upfConfig.daHttpCfg.bReportPolicy);
    zte_printf_s("g_upfConfig.daHttpCfg.dwCheckTime          = %u\n", g_upfConfig.daHttpCfg.dwCheckTime);
    zte_printf_s("g_upfConfig.daHttpCfg.dwAgingTime          = %u\n", g_upfConfig.daHttpCfg.dwAgingTime);
    return;
}
