#ifndef PS_NCU_ROUTE_PROC_H_
#define PS_NCU_ROUTE_PROC_H_
#include "ps_packet.h"
WORD32 psNcuUlIPV4GiRouteGet(PS_PACKET* pkt, T_IPV4 ptNwdafIP);
WORD32 psNcuUlIPV6GiRouteGet(PS_PACKET* pkt, T_IPV6 ptNwdafIP);
WORD16 psNcuGetMtu(T_IPV6 tDstIP, BYTE iptype, WORD16 threadID);
#endif
