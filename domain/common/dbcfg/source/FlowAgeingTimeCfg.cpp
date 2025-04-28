#include "FlowAgeingTimeCfg.h"
#include "ps_p4.h"

#include "psMcsGlobalCfg.h"
#define USE_DM_UPFNCU_GETNC_FLOWAGEINGTIME
#include "dbm_lib_upfncu.h"
#include "dbmLibComm.h"
#include "zte_slibc.h"



void FlowAgeingTimeCfg::powerOnProc()
{
    BOOLEAN bflag = FALSE;
    DM_UPFNCU_GETNC_FLOWAGEINGTIME_REQ req = {0};
    DM_UPFNCU_GETNC_FLOWAGEINGTIME_ACK ack = {0};

    req.msgType = MSG_CALL;
    req.dwgindex= 1;
    ack.retCODE = RC_DBM_ERROR;

    bflag = DBM_CALL(DM_UPFNCU_GETNC_FLOWAGEINGTIME, (LPSTR)&req, (LPSTR)&ack);

    if( RC_DBM_OK != ack.retCODE || (TRUE != bflag))
    {
        g_upfConfig.flowAgeingTimeCfg.longtime      = 7200;
        g_upfConfig.flowAgeingTimeCfg.shorttime     = 120;
    }
    else
    {
        g_upfConfig.flowAgeingTimeCfg.longtime      = ack.longtime;
        g_upfConfig.flowAgeingTimeCfg.shorttime     = ack.shorttime;
    }
}

void FlowAgeingTimeCfg::cfgNotifyProc(WORD16 operation, WORD16 msgLen, BYTE *msg)
{
    if(msg == NULL)
    {
      return;
    }
    
    NC_FLOWAGEINGTIME_TUPLE *ptupleNew = NULL;
    
    switch (operation)
    {
      case _DB_CFGCHG_TUPLE_NOTIFY_INSERT:
        {
            if(sizeof(NC_FLOWAGEINGTIME_TUPLE) > msgLen)
            {
                return ;
            }
            ptupleNew = (NC_FLOWAGEINGTIME_TUPLE*)msg;
            break;
        }
        case _DB_CFGCHG_TUPLE_NOTIFY_MODIFY:
        {
            if(2*sizeof(NC_FLOWAGEINGTIME_TUPLE) > msgLen)
            {
                return ;
            }
            ptupleNew = (NC_FLOWAGEINGTIME_TUPLE*)(msg + sizeof(NC_FLOWAGEINGTIME_TUPLE));
            break;
        }
        default:
        {
            break;
        }
    }
    
    if( NULL != ptupleNew )
    {
        g_upfConfig.flowAgeingTimeCfg.longtime      = ptupleNew->longtime;
        g_upfConfig.flowAgeingTimeCfg.shorttime     = ptupleNew->shorttime;    
    }

}

void FlowAgeingTimeCfg::show()
{
    zte_printf_s("\n longtime              = %u", g_upfConfig.flowAgeingTimeCfg.longtime);
    zte_printf_s("\n shorttime             = %u", g_upfConfig.flowAgeingTimeCfg.shorttime);
}


