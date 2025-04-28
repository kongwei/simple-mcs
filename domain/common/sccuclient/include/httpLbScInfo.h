#ifndef HTTPLB_SCINFO_H
#define HTTPLB_SCINFO_H

#include "tulip.h"
#include "sccu_pub_define.h"

#ifndef INVALID_COMM_ID
#define INVALID_COMM_ID    0xFFFFFFFF
#endif

#ifdef __cplusplus
extern "C"{
#endif

T_ABLInfo getHttpProxyAbility();
WORD32 psGetHttpLbCommidByLogicNo(WORD32 dwLogicNo);
BOOL psRegHttpLbScInfoChg();

#ifdef __cplusplus
}
#endif

#endif
