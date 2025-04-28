#include "ResourceAgeCfg.h"
#include "ps_p4.h"

#include "psMcsGlobalCfg.h"
#define USE_DM_UPFNCU_GETALLR_NC_RESOURCEAGE
#include "dbm_lib_upfncu.h"
#include "dbmLibComm.h"
#include "zte_slibc.h"
void setResourceAgeCfgSN(CHAR* tblName, WORD32 dwAgeTime, BYTE bScanStep);

void ResourceAgeCfg::powerOnProc()
{
    BOOLEAN bflag = FALSE;
    DM_UPFNCU_GETALLR_NC_RESOURCEAGE_REQ req = {0};
    DM_UPFNCU_GETALLR_NC_RESOURCEAGE_ACK ack = {0};
    WORD32 ResourseNum = 0;

    req.msgType = MSG_CALL;
    req.dwTupleNo = 0;
    ack.retCODE = RC_DBM_ERROR;

    WORD32 cycleTime = R_NC_RESOURCEAGE_CAPACITY/UPFNCU_GETALL_MAX;
    WORD32 j = 0;
    zte_memset_s(&(g_upfConfig.resourceAgeCfg[0]), sizeof(T_ResourceAgeCfg)*R_NC_RESOURCEAGE_CAPACITY, 0 ,sizeof(T_ResourceAgeCfg)*R_NC_RESOURCEAGE_CAPACITY);
    for(WORD32 i = 0;i <= cycleTime; ++i)
    {
        bflag = (DBBOOL)DBM_CALL(DM_UPFNCU_GETALLR_NC_RESOURCEAGE, (LPSTR)&req, (LPSTR)&ack);
        if(!(TRUE == bflag && RC_OK == ack.retCODE))
        {
            setResourceAgeCfgSN("R_NCU_SN", 3, 30);
            return ;
        }

        for(j = 0; j < MIN(ack.dwValidNum, UPFNCU_GETALL_MAX); ++j)
        {
            if(ResourseNum >= R_NC_RESOURCEAGE_CAPACITY)
            {
                break;
            }
            zte_strncpy_s(g_upfConfig.resourceAgeCfg[ResourseNum].bCtxname, LEN_R_NC_RESOURCEAGE_CTXNAME_MAX, ack.ctxname[j], LEN_R_NC_RESOURCEAGE_CTXNAME_MAX);
            g_upfConfig.resourceAgeCfg[ResourseNum].dwAgetime  =  ack.agetime[j];
            g_upfConfig.resourceAgeCfg[ResourseNum].bScanstep =  ack.scanstep[j];
            setResourceAgeCfgSN(ack.ctxname[j], ack.agetime[j], ack.scanstep[j]);
            ResourseNum++;
        }
        req.dwTupleNo = ack.dwTupleNo;
        if (ack.blEnd) break;
    }
    g_upfConfig.bResourceAgeCfgAgeTableNums = ResourseNum;
}

void ResourceAgeCfg::cfgNotifyProc(WORD16 operation, WORD16 msgLen, BYTE *msg)
{
    if(msg == NULL)
    {
      return;
    }
    
    R_NC_RESOURCEAGE_TUPLE *ptupleNew = NULL;
    
    switch (operation)
    {
      case _DB_CFGCHG_TUPLE_NOTIFY_INSERT:
        {
            if(sizeof(R_NC_RESOURCEAGE_TUPLE) > msgLen)
            {
                return ;
            }
            ptupleNew = (R_NC_RESOURCEAGE_TUPLE*)msg;
            break;
        }
        case _DB_CFGCHG_TUPLE_NOTIFY_MODIFY:
        {
            if(2*sizeof(R_NC_RESOURCEAGE_TUPLE) > msgLen)
            {
                return ;
            }
            ptupleNew = (R_NC_RESOURCEAGE_TUPLE*)(msg + sizeof(R_NC_RESOURCEAGE_TUPLE));
            break;
        }
        default:
        {
            break;
        }
    }
    
    if( NULL != ptupleNew )
    {
        for(WORD32 i = 0; i<R_RESOURCEAGE_CAPACITY; i++)
        {
            if(0 == strncmp(g_upfConfig.resourceAgeCfg[i].bCtxname, ptupleNew->ctxname, LEN_R_NC_RESOURCEAGE_CTXNAME_MAX+1))
            {
                zte_strncpy_s(g_upfConfig.resourceAgeCfg[i].bCtxname, LEN_R_NC_RESOURCEAGE_CTXNAME_MAX, ptupleNew->ctxname, LEN_R_NC_RESOURCEAGE_CTXNAME_MAX);
                g_upfConfig.resourceAgeCfg[i].dwAgetime  =  ptupleNew->agetime;
                g_upfConfig.resourceAgeCfg[i].bScanstep  =  ptupleNew->scanstep;
                break;
            }
        }
        setResourceAgeCfgSN(ptupleNew->ctxname, ptupleNew->agetime, ptupleNew->scanstep);
        g_upfConfig.bResourceAgeCfgAgeParaChanged = 1;
        
    }

}

void ResourceAgeCfg::show()
{
    for(WORD32 i=0; i<R_NC_RESOURCEAGE_CAPACITY; i++)
    {
        if('\0' == g_upfConfig.resourceAgeCfg[i].bCtxname[0])
        {
            break;
        }
        zte_printf_s("\n bCtxname[%d]             = %s", i, g_upfConfig.resourceAgeCfg[i].bCtxname);
        zte_printf_s("\n dwAgetime[%d]            = %d", i, g_upfConfig.resourceAgeCfg[i].dwAgetime);
        zte_printf_s("\n bScanstep[%d]            = %d", i, g_upfConfig.resourceAgeCfg[i].bScanstep);
    }
    zte_printf_s("\n snctx dwAgetime           = %d", g_upfConfig.tSnAge.dwAgetime);
    zte_printf_s("\n snctx bScanstep           = %d", g_upfConfig.tSnAge.bScanstep);
}

void setResourceAgeCfgSN(CHAR* tblName, WORD32 dwAgeTime, BYTE bScanStep)
{
    if (NULL == tblName)
    {
        return ;
    }

    if (0 != strncmp(tblName, "R_NCU_SN", LEN_R_RESOURCEAGE_CTXNAME_MAX+1))
    {
        return ;
    }
    g_upfConfig.tSnAge.dwAgetime = dwAgeTime;
    g_upfConfig.tSnAge.bScanstep = bScanStep;
}
