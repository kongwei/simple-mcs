#include "psMcsGlobalCfg.h"
#include "DataAnalysSysCfg.h"
#define USE_DM_UPFNCU_GETR_DASYSTEMCFG
#include "dbm_lib_upfncu.h"
#include "dbmLibComm.h"
#include "zte_slibc.h"
extern WORD32 g_exp_tim;
void DataAnalysSysCfg::powerOnProc()
{
    BOOLEAN bflag = FALSE;
    DM_UPFNCU_GETR_DASYSTEMCFG_REQ req = {0};
    DM_UPFNCU_GETR_DASYSTEMCFG_ACK ack = {0};

    req.msgType = MSG_CALL;
    req.dwgindex= 1;
    ack.retCODE = RC_DBM_ERROR;

    bflag = DBM_CALL(DM_UPFNCU_GETR_DASYSTEMCFG, (LPSTR)&req, (LPSTR)&ack);

    if( RC_DBM_OK != ack.retCODE || (TRUE != bflag))
    {
        g_upfConfig.dataSysCfg.bAppexpswitch  = 1;
        g_upfConfig.dataSysCfg.bAnareportintf = 0;
        g_upfConfig.dataSysCfg.bExpreportintf = 1;
        g_upfConfig.dataSysCfg.expreporttime  = 5;
        g_upfConfig.dataSysCfg.bRptMulSwitch  = 0;
    }
    else
    {
        g_upfConfig.dataSysCfg.bAppexpswitch  = ack.appexpswitch;
        g_upfConfig.dataSysCfg.bAnareportintf = ack.anareportintf;
        g_upfConfig.dataSysCfg.bExpreportintf = ack.expreportintf;
        g_upfConfig.dataSysCfg.expreporttime  = ack.expreporttime;
        g_upfConfig.dataSysCfg.bRptMulSwitch  = ack.reportmulswitch;
    }
    g_exp_tim = g_upfConfig.dataSysCfg.expreporttime*60; //临时借用开关。
}

void DataAnalysSysCfg::cfgNotifyProc(WORD16 operation, WORD16 msgLen, BYTE *msg)
{
    if(msg == NULL)
    {
      return;
    }
    
    R_DASYSTEMCFG_TUPLE *ptupleNew = NULL;
    
    switch (operation)
    {
      case _DB_CFGCHG_TUPLE_NOTIFY_INSERT:
        {
            if(sizeof(R_DASYSTEMCFG_TUPLE) > msgLen)
            {
                return ;
            }
            ptupleNew = (R_DASYSTEMCFG_TUPLE*)msg;
            break;
        }
        case _DB_CFGCHG_TUPLE_NOTIFY_MODIFY:
        {
            if(2*sizeof(R_DASYSTEMCFG_TUPLE) > msgLen)
            {
                return ;
            }
            ptupleNew = (R_DASYSTEMCFG_TUPLE*)(msg + sizeof(R_DASYSTEMCFG_TUPLE));
            break;
        }
        default:
        {
            break;
        }
    }
    
    if( NULL != ptupleNew )
    {
        g_upfConfig.dataSysCfg.bAppexpswitch  = ptupleNew->appexpswitch;
        g_upfConfig.dataSysCfg.bAnareportintf = ptupleNew->anareportintf;
        g_upfConfig.dataSysCfg.bExpreportintf = ptupleNew->expreportintf;
        g_upfConfig.dataSysCfg.expreporttime  = ptupleNew->expreporttime;
        g_upfConfig.dataSysCfg.bRptMulSwitch  = ptupleNew->reportmulswitch;
        g_exp_tim = g_upfConfig.dataSysCfg.expreporttime*60; //临时借用开关。
    }
    
}

void DataAnalysSysCfg::show()
{
    zte_printf_s("\n bAppexpswitch            = %u", g_upfConfig.dataSysCfg.bAppexpswitch);
    zte_printf_s("\n bAnareportintf           = %u", g_upfConfig.dataSysCfg.bAnareportintf);
    zte_printf_s("\n bExpreportintf           = %u", g_upfConfig.dataSysCfg.bExpreportintf);
    zte_printf_s("\n expreporttime            = %u", g_upfConfig.dataSysCfg.expreporttime);
    zte_printf_s("\n bRptMulSwitch            = %u", g_upfConfig.dataSysCfg.bRptMulSwitch);
}


