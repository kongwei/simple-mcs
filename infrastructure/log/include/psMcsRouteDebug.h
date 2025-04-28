#ifndef PS_MCS_ROUTE_DEBUG_H_
#define PS_MCS_ROUTE_DEBUG_H_

#ifdef __cplusplus
extern "C" {
#endif
#include "tulip.h"
void showV6Route(BYTE thread, WORD32 vpnid, CHAR* dstIP);
void showV4Route(BYTE thread, WORD32 vpnid, CHAR* dstIP);
#ifdef __cplusplus
}
#endif


#endif