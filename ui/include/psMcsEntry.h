#ifndef PS_MCS_ENTRY_H_
#define PS_MCS_ENTRY_H_

#include "ps_packet.h"
#include "ps_mcs_define.h"
void psVpfuMcsIPPktRcv(PS_PACKET *ptPacket);
VOID  psVpfuNcuUcomMsgProc(T_MediaProcThreadPara *ptMediaProcThreadPara);
void psNcuSolveOtherPkt(PS_PACKET *ptPacket, T_MediaProcThreadPara *ptMediaProcThreadPara);
#endif