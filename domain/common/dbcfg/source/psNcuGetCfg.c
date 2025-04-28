#include "psNcuGetCfg.h"
#include "UpfNcuSynInfo.h"
#include "ps_mcs_define.h"
#include "ps_pub.h"
#include "psMcsGlobalCfg.h"
#include "McsIPv4Head.h"
#include "McsIPv6Head.h"
BYTE tNwdafIpType = 4;
T_IPV6 tNwdafIPv6 = { 101,102,103,104 };
T_IPV4 tNwdafIPv4 = {10,225,41,230};

WORD32 g_vrf = 0;
WORD16 getNwdafPort()
{
    return g_upfConfig.daSyslinkCfg.udpport;
}


WORD32 getNwdafIpv4(T_IPV4 ipv4)
{
    IPV4_COPY(ipv4, tNwdafIPv4);
    return MCS_RET_SUCCESS;
}

WORD32 getNwdafIpv6(T_IPV6 ipv6)
{
    IPV6_COPY(ipv6, tNwdafIPv6);
    return MCS_RET_SUCCESS;
}
WORD32 getNcuIP(T_IPV6 ipv6, BYTE iptype)
{
    if(iptype == IPv4_VERSION && inet_pton(AF_INET, g_upfConfig.daUpfIpCfg.upfIpv4, ipv6))
    {
        return MCS_RET_SUCCESS;
    }
    else if(iptype == IPv6_VERSION && inet_pton(AF_INET6, g_upfConfig.daUpfIpCfg.upfIpv6, ipv6))
    {
        return MCS_RET_SUCCESS;
    }
    return MCS_RET_FAIL;


}
/* Started by AICoder, pid:m4ee7qeb16h84ba149d90b074005db19d310af22 */
WORD16 psGetVrfV4()
{
    return g_upfConfig.daUpfIpCfg.wVrfV4;
}
/* Ended by AICoder, pid:m4ee7qeb16h84ba149d90b074005db19d310af22 */

WORD16 psGetVrfV6()
{
    return g_upfConfig.daUpfIpCfg.wVrfV6;
}

BYTE psGetAppRptMulSwitch()
{
    return g_upfConfig.dataSysCfg.bRptMulSwitch;
}
WORD32 psGetAppExpSwitchForAnaUser()
{
    return g_upfConfig.dataSysCfg.bAppexpswitch;
}
WORD32 psGetAppExpRptTimerForAna()
{
    return g_upfConfig.dataSysCfg.expreporttime * 60; //单位s
}

WORD16 psGetClientProfileidByUri(const char* uri)
{
    if(NULL == uri)
    {
        return 0;
    }
    BYTE bFoundv6 = TRUE;
    WORD32 len = zte_strnlen_s(uri, AUCREPORTURI_MAXLEN);
    WORD32 i = 0;
    for(i=0;i<len;i++)
    {
        if(uri[i] == '.')
        {
            bFoundv6 = FALSE;
            break;
        }
    }
    if(TRUE == bFoundv6)
    {
        return  g_upfConfig.wClientProfileV6ID;
    }
    return g_upfConfig.wClientProfileV4ID;
}

/* Started by AICoder, pid:1e11bf8890fd92714e6f0b67d08e4f2e3de43f52 */
BYTE psGetDaHttpCheckSwitch()
{
    return g_upfConfig.daHttpCfg.bCheckSwitch;
}

BYTE psGetDaHttpCheckFailurePolicy()
{
    return g_upfConfig.daHttpCfg.bCheckFailurePolicy;
}

BYTE psGetDaHttpReportPolicy()
{
    return g_upfConfig.daHttpCfg.bReportPolicy;
}

WORD32 psGetDaHttpCheckTime()
{
    return g_upfConfig.daHttpCfg.dwCheckTime;
}

WORD32 psGetDaHttpAgingTime()
{
    return g_upfConfig.daHttpCfg.dwAgingTime;
}
/* Ended by AICoder, pid:1e11bf8890fd92714e6f0b67d08e4f2e3de43f52 */

#define SHOW_IP_ADDR(IP_NAME) \
do \
{ \
zte_printf_s("ip addr = "); \
WORD32 i = 0; \
for (i = 0; i < sizeof(IP_NAME); i++) \
{ \
    zte_printf_s("%u ", IP_NAME[i]); \
} \
zte_printf_s("\n"); \
}while (0)

void showNcuIp()
{
    zte_printf_s("upf cfg ipv4 = %s\n", g_upfConfig.daUpfIpCfg.upfIpv4);
    zte_printf_s("upf cfg ipv6 = %s\n", g_upfConfig.daUpfIpCfg.upfIpv6);
    zte_printf_s("upf cfg vrfv4 = %u\n", g_upfConfig.daUpfIpCfg.wVrfV4);
    zte_printf_s("upf cfg vrfv6 = %u\n", g_upfConfig.daUpfIpCfg.wVrfV6);

}

void setNcuIP(char* ipv4, char* ipv6)
{
    if(NULL != ipv4)
    {
        zte_memset_s(g_upfConfig.daUpfIpCfg.upfIpv4, LEN_R_DAUPFCFG_UPFIPV4_MAX,0,LEN_R_DAUPFCFG_UPFIPV4_MAX);
        zte_memcpy_s(g_upfConfig.daUpfIpCfg.upfIpv4, LEN_R_DAUPFCFG_UPFIPV4_MAX,ipv4,zte_strnlen_s(ipv4,LEN_R_DAUPFCFG_UPFIPV4_MAX));
    }
    if(NULL != ipv6)
    {
        zte_memset_s(g_upfConfig.daUpfIpCfg.upfIpv6, LEN_R_DAUPFCFG_UPFIPV6_MAX,0,LEN_R_DAUPFCFG_UPFIPV6_MAX);
        zte_memcpy_s(g_upfConfig.daUpfIpCfg.upfIpv6, LEN_R_DAUPFCFG_UPFIPV6_MAX,ipv6,zte_strnlen_s(ipv6,LEN_R_DAUPFCFG_UPFIPV6_MAX));
    }
}

WORD32 psNcuGetSnCtxAgeTime()
{
    return g_upfConfig.tSnAge.dwAgetime;
}

void setNcuPreferAddrType(BYTE v)
{
    g_upfConfig.daUpfIpCfg.bPreferAddrType = v;
}
/* ******************************************************* */
/* ********************以下接口为Rust提供********************/
/* ******************************************************* */

WORD32 ncuGetSoftPara(WORD32 id)
{
    return getNcuSoftPara(id);
}

BYTE ncuGetPreferAddrType()
{
    return g_upfConfig.daUpfIpCfg.bPreferAddrType;
}

WORD16 ncuGetClientProfileV4ID(){
    return g_upfConfig.wClientProfileV4ID;
}

WORD16 ncuGetClientProfileV6ID(){
    return g_upfConfig.wClientProfileV6ID;
}

BYTE ncuEchoAckPolicyAckSub()
{
    return NCACKPOLICY_ACKSUB == g_upfConfig.daSyslinkCfg.echoackpolicy;
}

BYTE ncuGetResourceAgeCfgChangedFlag()
{
    return g_upfConfig.bResourceAgeCfgAgeParaChanged;
}

VOID ncuClearResourceAgeCfgChangedFlag()
{
    g_upfConfig.bResourceAgeCfgAgeParaChanged = 0;
}
