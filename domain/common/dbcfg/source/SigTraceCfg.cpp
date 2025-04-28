#include "SigTraceCfg.h"
#include "ps_p4.h"
#include "NcuSigTrace.h"

#include "psMcsGlobalCfg.h"
#define USE_DM_UPF_GETR_SIGTRACECFG
#include "dbm_lib_upf.h"
#include "dbmLibComm.h"

T_UpfSigTraceCfg g_sigtracecfg;


void SigTraceCfg::powerOnProc()
{
    BOOLEAN bflag = FALSE;
    DM_UPF_GETR_SIGTRACECFG_REQ req = {0};
    DM_UPF_GETR_SIGTRACECFG_ACK ack = {0};

    req.msgType = MSG_CALL;
    req.dwgindex= 1;
    ack.retCODE = RC_DBM_ERROR;

    bflag = DBM_CALL(DM_UPF_GETR_SIGTRACECFG, (LPSTR)&req, (LPSTR)&ack);

    if( RC_DBM_OK != ack.retCODE || (TRUE != bflag))
    {
        g_sigtracecfg.bSigStopOnOverLoad             = 1;
        g_sigtracecfg.bSigRecoveryThreshold          = 55;
        g_sigtracecfg.bSigStopThreshold              = 80;
        g_sigtracecfg.bSigOverTimeThreshold_Hours    = 24;
        g_sigtracecfg.WildTraceMaxNum                = 200;
        g_sigtracecfg.MaxReportPPS                   = 2000;
    }
    else
    {
        g_sigtracecfg.bSigStopOnOverLoad              = ack.stoponoverload;
        g_sigtracecfg.bSigRecoveryThreshold           = ack.recoverythreshold;
        g_sigtracecfg.bSigStopThreshold               = ack.stopthreshold;
        g_sigtracecfg.bSigOverTimeThreshold_Hours     = ack.overtimethreshold;
        g_sigtracecfg.WildTraceMaxNum                 = ack.wildtracemaxnum;
        g_sigtracecfg.MaxReportPPS                    = ack.maxreportpps;
    }
    GetSigTraceCfg(ack,bflag);
}

void SigTraceCfg::cfgNotifyProc(WORD16 operation, WORD16 msgLen, BYTE *msg)
{
    if(msg == NULL)
    {
      return;
    }
    
    R_SIGTRACECFG_TUPLE *ptupleNew = NULL;
    
    switch (operation)
    {
      case _DB_CFGCHG_TUPLE_NOTIFY_INSERT:
        {
            if(sizeof(R_SIGTRACECFG_TUPLE) > msgLen)
            {
                return ;
            }
            ptupleNew = (R_SIGTRACECFG_TUPLE*)msg;
            break;
        }
        case _DB_CFGCHG_TUPLE_NOTIFY_MODIFY:
        {
            if(2*sizeof(R_SIGTRACECFG_TUPLE) > msgLen)
            {
                return ;
            }
            ptupleNew = (R_SIGTRACECFG_TUPLE*)(msg + sizeof(R_SIGTRACECFG_TUPLE));
            break;
        }
        default:
        {
            break;
        }
    }
    
    if( NULL != ptupleNew )
    {
        g_sigtracecfg.bSigStopOnOverLoad            = ptupleNew->stoponoverload;
        g_sigtracecfg.bSigRecoveryThreshold         = ptupleNew->recoverythreshold;
        g_sigtracecfg.bSigStopThreshold             = ptupleNew->stopthreshold;
        g_sigtracecfg.bSigOverTimeThreshold_Hours   = ptupleNew->overtimethreshold;
        g_sigtracecfg.WildTraceMaxNum               = ptupleNew->wildtracemaxnum;
        g_sigtracecfg.MaxReportPPS                  = ptupleNew->maxreportpps;
        
    }
    GetSigTraceCfgNotify(ptupleNew);
}

void SigTraceCfg::show()
{
    printf("\n bSigStopOnOverLoad            = %u", g_sigtracecfg.bSigStopOnOverLoad);
    printf("\n bSigRecoveryThreshold         = %u", g_sigtracecfg.bSigRecoveryThreshold);
    printf("\n bSigStopThreshold             = %u", g_sigtracecfg.bSigStopThreshold);
    printf("\n bSigOverTimeThreshold_Hours   = %u", g_sigtracecfg.bSigOverTimeThreshold_Hours);
    printf("\n WildTraceMaxNum               = %u", g_sigtracecfg.WildTraceMaxNum);
    printf("\n MaxReportPPS                  = %u", g_sigtracecfg.MaxReportPPS);
}


