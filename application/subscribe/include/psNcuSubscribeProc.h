#ifndef PS_NCU_SUBSCRIBE_PROC_H_
#define PS_NCU_SUBSCRIBE_PROC_H_

#include "UpfNcuSynInfo.h"
#include "ps_mcs_define.h"
#include "MemShareCfg.h"
#include "ps_ncu_session.h"

WORD32 psNcuSubScribeProc(void* ptSubscribeData, T_MediaProcThreadPara *ptMediaProcThreadPara);
WORD32 psNcuDelSessionProc(T_MediaProcThreadPara *ptMediaProcThreadPara);
VOID psNcuDelSessionByGroupNo(T_MediaProcThreadPara *ptMediaProcThreadPara, T_psMcsSessDelByGroupArea *ptMsgBody);
#endif