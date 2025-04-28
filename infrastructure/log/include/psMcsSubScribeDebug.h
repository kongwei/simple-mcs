#ifndef PS_MCS_SUBSCRIBE_DEBUG_H_
#define PS_MCS_SUBSCRIBE_DEBUG_H_

#ifdef __cplusplus
extern "C" {
#endif
#include "tulip.h"
void set_last_session(WORD64 ddwUPSeid, WORD32 dwhDB);
void printT_NcuSynSubInfo(void *ptNcuSynSubInfo);
void printT_NcuSessionInfo(void *info);
void printT_NcuSynSubInfoHead(void *info);
#ifdef __cplusplus
}
#endif
#endif