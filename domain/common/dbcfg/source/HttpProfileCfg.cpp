#include "psMcsGlobalCfg.h"
#include "HttpProfileCfg.h"
#define USE_DM_UPFNCU_GETNC_HTTPPROFILECFG
#include "dbm_lib_upfncu.h"
#include "dbmLibComm.h"
#include "zte_slibc.h"

void NcHttpProfileCfg::powerOnProc()
{
    BOOLEAN bflag = FALSE;
    DM_UPFNCU_GETNC_HTTPPROFILECFG_REQ req = {0};
    DM_UPFNCU_GETNC_HTTPPROFILECFG_ACK ack = {0};

    req.msgType = MSG_CALL;
    req.dwgindex= 1;
    ack.retCODE = RC_DBM_ERROR;

    bflag = DBM_CALL(DM_UPFNCU_GETNC_HTTPPROFILECFG, (LPSTR)&req, (LPSTR)&ack);

    if( RC_DBM_OK != ack.retCODE || (TRUE != bflag))
    {
        g_upfConfig.wClientProfileV4ID         = 0;
        g_upfConfig.wClientProfileV6ID          = 0;
        return;
    }
    g_upfConfig.wClientProfileV4ID         = ack.clientprofileidipv4;
    g_upfConfig.wClientProfileV6ID         = ack.clientprofileidipv6;
}

void NcHttpProfileCfg::cfgNotifyProc(WORD16 operation, WORD16 msgLen, BYTE *msg)
{
    if(NULL == msg)
    {
        return;
    }
    
    NC_HTTPPROFILECFG_TUPLE *ptupleNew = NULL;
    
    switch (operation)
    {
        case _DB_CFGCHG_TUPLE_NOTIFY_INSERT:
        {
            if(sizeof(NC_HTTPPROFILECFG_TUPLE) > msgLen)
            {
                return ;
            }
            ptupleNew = (NC_HTTPPROFILECFG_TUPLE*)msg;
            break;
        }
        case _DB_CFGCHG_TUPLE_NOTIFY_MODIFY:
        {
            if(2*sizeof(NC_HTTPPROFILECFG_TUPLE) > msgLen)
            {
                return ;
            }
            ptupleNew = (NC_HTTPPROFILECFG_TUPLE*)(msg + sizeof(NC_HTTPPROFILECFG_TUPLE));
            break;
        }
        default:
        {
            break;
        }
    }
    
    if( NULL != ptupleNew )
    {
        g_upfConfig.wClientProfileV4ID = ptupleNew->clientprofileidipv4;
        g_upfConfig.wClientProfileV6ID = ptupleNew->clientprofileidipv6;
    }
}


void NcHttpProfileCfg::show()
{
        zte_printf_s("g_upfConfig.wClientProfileV4ID        = %u\n",g_upfConfig.wClientProfileV4ID);
        zte_printf_s("g_upfConfig.wClientProfileV6ID        = %u\n",g_upfConfig.wClientProfileV6ID);
}
