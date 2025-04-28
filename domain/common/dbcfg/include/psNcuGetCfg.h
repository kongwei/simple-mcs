#ifndef PS_NCU_GET_CFG_H_
#define PS_NCU_GET_CFG_H_

#include "tulip.h"
#include "ps_typedef.h"
#include "psMcsGlobalCfg.h"

#ifdef __cplusplus
extern "C" {
#endif

#define UPF_IP_PREFER_ADDR_TYPE_IPV4  0
#define UPF_IP_PREFER_ADDR_TYPE_IPV6  1

WORD32 getSubScribeType();
void setNcuIP(char* ipv4, char* ipv6);
WORD32 getDataType();
WORD16 getNwdafPort();
WORD32 getNwdafIpv4(T_IPV4 ipv4);
WORD32 getNwdafIpv6(T_IPV6 ipv6);
WORD32 getNcuIP(T_IPV6 ipv6, BYTE iptype);
WORD16 psGetVrfV4();
WORD16 psGetVrfV6();
WORD32 psGetAppExpSwitchForAnaUser();
WORD32 psGetAppExpRptTimerForAna();
WORD16 psGetClientProfileidByUri(const char* uri);
WORD32 psNcuGetSnCtxAgeTime();
/* ******************************************************* */
/* ********************以下接口为Rust提供********************/
/* ******************************************************* */
WORD32 ncuGetSoftPara(WORD32 id);
BYTE ncuGetPreferAddrType();
BYTE psGetAppRptMulSwitch();
BYTE psGetDaHttpCheckSwitch();
BYTE psGetDaHttpCheckFailurePolicy();
BYTE psGetDaHttpReportPolicy();
WORD32 psGetDaHttpCheckTime();
WORD32 psGetDaHttpAgingTime();
WORD16 ncuGetClientProfileV4ID();
WORD16 ncuGetClientProfileV6ID();
BYTE ncuEchoAckPolicyAckSub();
BYTE ncuGetResourceAgeCfgChangedFlag();
VOID ncuClearResourceAgeCfgChangedFlag();
#ifdef __cplusplus
}
#endif
#endif
