#include "psMcsGlobalCfg.h"
#include "DaSysLinkCfg.h"
#define USE_DM_UPFNCU_GETR_DALINKCFG
#include "dbm_lib_upfncu.h"
#include "dbmLibComm.h"
#include "zte_slibc.h"
#include "psUpfEvent.h"
#include "psUpfJobTypes.h"
#include "ncuSCCUAbility.h"
#include "psUpfCommon.h"

void DaSysLinkCfg::powerOnProc()
{
    BOOLEAN bflag = FALSE;
    DM_UPFNCU_GETR_DALINKCFG_REQ req = {0};
    DM_UPFNCU_GETR_DALINKCFG_ACK ack = {0};

    req.msgType = MSG_CALL;
    req.dwgindex= 1;
    ack.retCODE = RC_DBM_ERROR;

    bflag = DBM_CALL(DM_UPFNCU_GETR_DALINKCFG, (LPSTR)&req, (LPSTR)&ack);

    if( RC_DBM_OK != ack.retCODE || (TRUE != bflag))
    {
        g_upfConfig.daSyslinkCfg.echoswitch         = ack.echoswitch;
        g_upfConfig.daSyslinkCfg.echotime           = ack.echotime;
        g_upfConfig.daSyslinkCfg.echofailuretimes   = ack.echofailuretimes;
        g_upfConfig.daSyslinkCfg.echofailurepolicy  = ack.echofailurepolicy;
        g_upfConfig.daSyslinkCfg.echoackpolicy      = ack.echoackpolicy;
        g_upfConfig.daSyslinkCfg.times              = ack.times;
        g_upfConfig.daSyslinkCfg.udpport            = 15000;
        return;
    }
    g_upfConfig.daSyslinkCfg.echoswitch         = ack.echoswitch;
    g_upfConfig.daSyslinkCfg.echotime           = ack.echotime;
    g_upfConfig.daSyslinkCfg.echofailuretimes   = ack.echofailuretimes;
    g_upfConfig.daSyslinkCfg.echofailurepolicy  = ack.echofailurepolicy;
    g_upfConfig.daSyslinkCfg.echoackpolicy      = ack.echoackpolicy;
    g_upfConfig.daSyslinkCfg.times              = ack.times;
    g_upfConfig.daSyslinkCfg.udpport            = ack.udpport;
}



void DaSysLinkCfg::cfgNotifyProc(WORD16 operation, WORD16 msgLen, BYTE *msg)
{
    if(NULL == msg)
    {
        return;
    }
    
    R_DALINKCFG_TUPLE *ptupleNew = NULL;
    
    switch (operation)
    {
        case _DB_CFGCHG_TUPLE_NOTIFY_INSERT:
        {
            if(sizeof(R_DALINKCFG_TUPLE) > msgLen)
            {
                return ;
            }
            ptupleNew = (R_DALINKCFG_TUPLE*)msg;
            break;
        }
        case _DB_CFGCHG_TUPLE_NOTIFY_MODIFY:
        {
            if(2*sizeof(R_DALINKCFG_TUPLE) > msgLen)
            {
                return ;
            }
            ptupleNew = (R_DALINKCFG_TUPLE*)(msg + sizeof(R_DALINKCFG_TUPLE));
            break;
        }
        default:
        {
            break;
        }
    }
    
    if( NULL != ptupleNew )
    {
        g_upfConfig.daSyslinkCfg.echoswitch         = ptupleNew->echoswitch;
        g_upfConfig.daSyslinkCfg.echotime           = ptupleNew->echotime;
        g_upfConfig.daSyslinkCfg.echofailuretimes   = ptupleNew->echofailuretimes;
        g_upfConfig.daSyslinkCfg.echofailurepolicy  = ptupleNew->echofailurepolicy;
        g_upfConfig.daSyslinkCfg.echoackpolicy      = ptupleNew->echoackpolicy;
        g_upfConfig.daSyslinkCfg.times              = ptupleNew->times;
        g_upfConfig.daSyslinkCfg.udpport            = ptupleNew->udpport;

        if(TRUE == ncuIsSelfGroup(0))
        {
            ncuSendMsgToUpm((BYTE *)&g_upfConfig.daSyslinkCfg, sizeof(T_DaSynLinkCfg),EV_MSG_NWDAF_NCU_TO_UPM_NWDAFINFO, JOB_TYPE_MCS_MANAGE);
        }
    }
}


void DaSysLinkCfg::show()
{
        zte_printf_s("g_upfConfig.daSyslinkCfg.echoswitch         = %u\n",g_upfConfig.daSyslinkCfg.echoswitch);
        zte_printf_s("g_upfConfig.daSyslinkCfg.echotime           = %u\n",g_upfConfig.daSyslinkCfg.echotime);
        zte_printf_s("g_upfConfig.daSyslinkCfg.echofailuretimes   = %u\n",g_upfConfig.daSyslinkCfg.echofailuretimes);
        zte_printf_s("g_upfConfig.daSyslinkCfg.echofailurepolicy  = %u\n",g_upfConfig.daSyslinkCfg.echofailurepolicy);
        zte_printf_s("g_upfConfig.daSyslinkCfg.echoackpolicy      = %u\n",g_upfConfig.daSyslinkCfg.echoackpolicy);
        zte_printf_s("g_upfConfig.daSyslinkCfg.times              = %u\n",g_upfConfig.daSyslinkCfg.times);
        zte_printf_s("g_upfConfig.daSyslinkCfg.udpport            = %u\n",g_upfConfig.daSyslinkCfg.udpport);
}
