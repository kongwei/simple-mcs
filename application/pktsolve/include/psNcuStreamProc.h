#ifndef PS_NCU_STREAM_PROC_H_
#define PS_NCU_STREAM_PROC_H_

#include "ps_ncu_stream.h"
#include "xdb_pfu_dyntbl_acc.h"
#include "ps_mcs_define.h"
#include "MemShareCfg.h"
#include "ps_ncu_session.h"

/*
typedef enum
{
   NCU_TO_PFU_SYN_SESSION_CTX_REQ = 1, 
} NCU_TO_PFU_INFO_TYPE;
*/
/*
typedef struct tagT_psNcuToPfuInfo
{
    BYTE     bVersion;
    BYTE     bType;
    BYTE     bRsv[6];
    WORD64  ddwUPSeid;
}T_psNcuToPfuInfo;
*/
#define INVALID_NCU_COMMID   0xFFFFFFFF

T_psNcuFlowCtx* psNcuQryStreamProc(T_MediaProcThreadPara *ptMediaProcThreadPara,DB_STRM_INDEX* ptStmIdx);
T_psNcuFlowCtx* psNcuCrtStreamProc(T_MediaProcThreadPara *ptMediaProcThreadPara,DB_STRM_INDEX* ptStmIdx);
T_psNcuFlowCtx* psNcuQryOrCrtStreamProc(T_MediaProcThreadPara *ptMediaProcThreadPara,DB_STRM_INDEX* ptStmIdx);
WORD32 psDelStmProc(T_MediaProcThreadPara* ptMediaProcThreadPara,T_psNcuFlowCtx *ptStmAddr);
WORD32 psNcuToPfuSynSessionCtxReq(T_MediaProcThreadPara* ptMediaProcThreadPara);
VOID psNcuToPfuSynSessionCtxProc(T_MediaProcThreadPara* ptMediaProcThreadPara);
T_psNcuSessionCtx* psCreateSessionAndQueByUpseid(T_psNcuMcsPerform *ptNcuPerform, WORD64 ddwUPSeid, WORD32 dwhDBByThreadNo);
#endif