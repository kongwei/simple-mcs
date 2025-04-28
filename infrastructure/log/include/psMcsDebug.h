
#ifndef PS_MCS_DEBUG_H_H_
#define PS_MCS_DEBUG_H_H_

#include "tulip.h"
#include "ps_packet.h"
#include "dpa_compat.h"
#include "rte_hexdump.h"
#include "MemShareCfg.h"
#include "UpfNcuSynInfo.h"
#ifdef __cplusplus
extern "C" {
#endif
enum{
    DEBUG_LOW,
    DEBUG_HIG,
    DEBUG_ERR,
};
enum{
    DEBUG_RCV = 1,
    DEBUG_SND = 2,
};
extern WORD32 g_ncu_show;
extern WORD32 g_buff_trace;
extern WORD32 g_job_show;
#define DEBUG_TRACE(level, ...) \
do { \
    if (unlikely(level >= g_ncu_show)) { \
        zte_printf_s("\n[MCS:%-30s:%u] ", __FUNCTION__, __LINE__); \
        zte_printf_s(__VA_ARGS__); \
    } \
} while (0)

#define JOB_TRACE(level, jobtype,...) \
do { \
    if (unlikely(level >= g_job_show)) { \
        zte_printf_s("\n[%s][%-30s:%u] ", #jobtype, __FUNCTION__, __LINE__); \
        zte_printf_s(__VA_ARGS__); \
    } \
} while (0)

#define NEF_TRACE(level, ...)  \
do { \
    if (unlikely(level >= g_job_show)) { \
        zte_printf_s("\n[JOB_TYPE_NCU_NEFENTRY][%-30s:%u] ", __FUNCTION__, __LINE__); \
        zte_printf_s(__VA_ARGS__); \
    } \
} while (0)

#define PKT_TRACE(level, ptPacket) \
do { \
    if (unlikely(level >= g_ncu_show)) { \
        zte_printf_s("\n[MCS:%-30s:%u] ", __FUNCTION__, __LINE__); \
        BYTE               *aucPacketBuf     = NULL; \
        WORD16              wBufferLen       = 0; \
        WORD16              wBufferOffset    = 0; \
        PS_GET_BUF_INFO(ptPacket->pBufferNode, aucPacketBuf, wBufferOffset, wBufferLen); \
        rte_hexdump(stdout,"Exception pkt content",aucPacketBuf + wBufferOffset, wBufferLen); \
    } \
} while (0)

#define BUFF_TRACE(DIR, PTBUFF, WBUFFERLEN) \
do { \
    if (unlikely(DIR & g_buff_trace)) { \
        rte_hexdump(stdout,"pktbuff content",PTBUFF, WBUFFERLEN); \
    } \
} while (0)

T_psNcuMcsPerform *psVpfuMcsGetPerformPtr(WORD32 dwSthrIdx);
static inline T_psNcuMcsPerform* psGetPerform()
{
    if(NULL == g_ptMediaProcThreadPara || NULL == g_ptMediaProcThreadPara->ptMcsStatPointer)
    {
        return NULL;
    }
    return (T_psNcuMcsPerform*)(g_ptMediaProcThreadPara->ptMcsStatPointer);
}
#ifdef __cplusplus
}
#endif
#endif