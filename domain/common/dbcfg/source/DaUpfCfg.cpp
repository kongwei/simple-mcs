#include "psMcsGlobalCfg.h"
#include "DaUpfCfg.h"
#define USE_DM_UPFNCU_GETR_DAUPFCFG
#include "dbm_lib_upfncu.h"
#define USE_DM_UPF_SHARE_GETR_NETWORKINSTANCE_BY_NETWORKINSTANCE
#include "dbm_lib_upf_share.h"
#include "dbmLibComm.h"
#include "zte_slibc.h"
#include "ps_ncu_typedef.h"
#include "psUpfEvent.h"
#include "psUpfJobTypes.h"
#include "ncuSCCUAbility.h"
#include "psUpfCommon.h"
#include "psNcuCtrlStatInterface.h"

#ifdef __cplusplus
extern "C" {
#endif
#include "dpathreadpub.h" 
#ifdef __cplusplus
}
#endif


WORD32 psDpaGetNcuIpAddr();
VOID psNcuGetVrfByNetworkinstance(CHAR* ptnetworkinstance, WORD16* pwVpnidv4, WORD16* pwVpnidv6);
VOID  psNcuUpfIpChange(R_DAUPFCFG_TUPLE* ptupleNew, R_DAUPFCFG_TUPLE* ptupleOld);



VOID DaUpfIpCfg::powerOnProc()
{
    BOOLEAN bflag = FALSE;
    DM_UPFNCU_GETR_DAUPFCFG_REQ req = {0};
    DM_UPFNCU_GETR_DAUPFCFG_ACK ack = {0};

    req.msgType = MSG_CALL;
    req.dwgindex= 1;
    ack.retCODE = RC_DBM_ERROR;

    bflag = DBM_CALL(DM_UPFNCU_GETR_DAUPFCFG, (LPSTR)&req, (LPSTR)&ack);

    if( RC_DBM_OK != ack.retCODE || (TRUE != bflag))
    {
        zte_strncpy_s(g_upfConfig.daUpfIpCfg.upfIpv4,         LEN_R_DAUPFCFG_UPFIPV4_MAX+1,        "1.2.3.4",        LEN_R_DAUPFCFG_UPFIPV4_MAX);
        zte_strncpy_s(g_upfConfig.daUpfIpCfg.upfIpv6,         LEN_R_DAUPFCFG_UPFIPV6_MAX+1,        "1:2:3::4",        LEN_R_DAUPFCFG_UPFIPV6_MAX);
        g_upfConfig.daUpfIpCfg.bPreferAddrType = 1;
        g_upfConfig.daUpfIpCfg.wVrfV4 = 0;
        g_upfConfig.daUpfIpCfg.wVrfV6 = 0;
        psDpaGetNcuIpAddr();
        return;
    }
    zte_strncpy_s(g_upfConfig.daUpfIpCfg.upfIpv4,         LEN_R_DAUPFCFG_UPFIPV4_MAX+1,        ack.upfipv4,        LEN_R_DAUPFCFG_UPFIPV4_MAX);
    zte_strncpy_s(g_upfConfig.daUpfIpCfg.upfIpv6,         LEN_R_DAUPFCFG_UPFIPV6_MAX+1,        ack.upfipv6,        LEN_R_DAUPFCFG_UPFIPV6_MAX);
    g_upfConfig.daUpfIpCfg.bPreferAddrType = ack.preferaddr;
    psDpaGetNcuIpAddr();
    psNcuGetVrfByNetworkinstance(ack.networkinstance, &g_upfConfig.daUpfIpCfg.wVrfV4, &g_upfConfig.daUpfIpCfg.wVrfV6);
}

void DaUpfIpCfg::cfgNotifyProc(WORD16 operation, WORD16 msgLen, BYTE *msg)
{
    if(NULL == msg)
    {
        return;
    }
    
    R_DAUPFCFG_TUPLE *ptupleNew = NULL;
    R_DAUPFCFG_TUPLE *ptupleOld = NULL;
    
    switch (operation)
    {
        case _DB_CFGCHG_TUPLE_NOTIFY_INSERT:
        {
            if(sizeof(R_DAUPFCFG_TUPLE) > msgLen)
            {
                return ;
            }
            ptupleNew = (R_DAUPFCFG_TUPLE*)msg;
            break;
        }
        case _DB_CFGCHG_TUPLE_NOTIFY_MODIFY:
        {
            if(2*sizeof(R_DAUPFCFG_TUPLE) > msgLen)
            {
                return ;
            }
            ptupleOld = (R_DAUPFCFG_TUPLE*)(msg);
            ptupleNew = (R_DAUPFCFG_TUPLE*)(msg + sizeof(R_DAUPFCFG_TUPLE));
            break;
        }
        default:
        {
            break;
        }
    }
    
    if( NULL != ptupleNew )
    {
        zte_strncpy_s(g_upfConfig.daUpfIpCfg.upfIpv4,         LEN_R_DAUPFCFG_UPFIPV4_MAX+1,        ptupleNew->upfipv4,        LEN_R_DAUPFCFG_UPFIPV4_MAX);
        zte_strncpy_s(g_upfConfig.daUpfIpCfg.upfIpv6,         LEN_R_DAUPFCFG_UPFIPV6_MAX+1,        ptupleNew->upfipv6,        LEN_R_DAUPFCFG_UPFIPV6_MAX);
        g_upfConfig.daUpfIpCfg.bPreferAddrType = ptupleNew->preferaddr;
        psDpaGetNcuIpAddr();
        psNcuGetVrfByNetworkinstance(ptupleNew->networkinstance, &g_upfConfig.daUpfIpCfg.wVrfV4, &g_upfConfig.daUpfIpCfg.wVrfV6);
        if(TRUE == ncuIsSelfGroup(0))
        {
              psNcuUpfIpChange(ptupleNew, ptupleOld);
        }
    }
}

void DaUpfIpCfg::show()
{
    zte_printf_s("\n upfName            = %s", g_upfConfig.daUpfIpCfg.upfName);
    zte_printf_s("\n upfIpv4            = %s", g_upfConfig.daUpfIpCfg.upfIpv4);
    zte_printf_s("\n upfIpv6            = %s", g_upfConfig.daUpfIpCfg.upfIpv6);
    zte_printf_s("\n preferaddr         = %d", g_upfConfig.daUpfIpCfg.bPreferAddrType);
    zte_printf_s("\n wVrfV4             = %u", g_upfConfig.daUpfIpCfg.wVrfV4);
    zte_printf_s("\n wVrfV6             = %u", g_upfConfig.daUpfIpCfg.wVrfV6);
}


WORD32 psDpaGetNcuIpAddr()
{
    T_IPComm tV4ip = {0};
    T_IPComm tV6ip = {0};
    T_GtpServiceIp config;
    zte_memset_s(&config,sizeof(T_GtpServiceIp),0,sizeof(T_GtpServiceIp));

    if(1 == inet_pton(AF_INET,  g_upfConfig.daUpfIpCfg.upfIpv4, tV4ip.abAddress))
    {
         tV4ip.bType = IPCOMM_TYPE_IPV4;
         config.bV4NWDAFAddrNum = 1;
         config.dwNWDAFAddr[0] = *(WORD32*)tV4ip.abAddress;
    }
    if(1 == inet_pton(AF_INET6, g_upfConfig.daUpfIpCfg.upfIpv6, tV6ip.abAddress))
    {
        tV6ip.bType = IPCOMM_TYPE_IPV6;
        config.bV6NWDAFAddrNum = 1;
        config.tV6NWDAFAddr[0].bType = IPCOMM_TYPE_IPV6;
        zte_memcpy_s(config.tV6NWDAFAddr[0].abAddress,IPCOMM_ADDR_LEN,tV6ip.abAddress,IPCOMM_ADDR_LEN);
    }
    dpaSetGtpserviceAddr(&config);
    return 0;
}

VOID psNcuGetVrfByNetworkinstance(CHAR* ptnetworkinstance, WORD16* pwVpnidv4, WORD16* pwVpnidv6)
{
    MCS_CHK_NULL_VOID(ptnetworkinstance);
    MCS_CHK_NULL_VOID(pwVpnidv4);
    MCS_CHK_NULL_VOID(pwVpnidv6);

    BOOLEAN bflag = FALSE;
    DM_UPF_SHARE_GETR_NETWORKINSTANCE_BY_NETWORKINSTANCE_REQ req = {0};
    DM_UPF_SHARE_GETR_NETWORKINSTANCE_BY_NETWORKINSTANCE_ACK ack = {0};
    req.msgType = MSG_CALL;
    zte_strncpy_s(req.networkinstance, LEN_R_NETWORKINSTANCE_NETWORKINSTANCE_MAX + 1, ptnetworkinstance, LEN_R_NETWORKINSTANCE_NETWORKINSTANCE_MAX);
    ack.retCODE = RC_DBM_ERROR;

    bflag = DBM_CALL(DM_UPF_SHARE_GETR_NETWORKINSTANCE_BY_NETWORKINSTANCE, (LPSTR)&req, (LPSTR)&ack);
    MCS_CHK_VOID(TRUE != bflag);
    MCS_CHK_VOID(RC_DBM_OK != ack.retCODE);
    *pwVpnidv4    = ack.vpnid;
    *pwVpnidv6    = ack.vpnidv6;
}

VOID  psNcuUpfIpChange(R_DAUPFCFG_TUPLE* ptupleNew, R_DAUPFCFG_TUPLE* ptupleOld)
{
    if(NULL == ptupleNew || NULL == ptupleOld)
    return;
    
    WORD32  dwRet  = 0;
    
    if(memcmp(ptupleNew->upfipv4, ptupleOld->upfipv4, LEN_R_DAUPFCFG_UPFIPV4_MAX) || memcmp(ptupleNew->upfipv6, ptupleOld->upfipv6, LEN_R_DAUPFCFG_UPFIPV6_MAX))
    {
        dwRet = ncuSendMsgToUpm((BYTE *)&g_upfConfig.daUpfIpCfg, sizeof(T_DaUpfIpCfg),EV_MSG_NWDAF_NCU_TO_UPM_NWDAFINFO, JOB_TYPE_MCS_MANAGE);
        if(UPF_RET_SUCCESS != dwRet) 
        {
            JOB_LOC_STAT_ADDONE(qwDaUpfIpCfgSendToUpmGetSubFail);
            return;
        }
        JOB_LOC_STAT_ADDONE(qwDaUpfIpCfgSendToUpmGetSubSucc);
    }
    return;
}
