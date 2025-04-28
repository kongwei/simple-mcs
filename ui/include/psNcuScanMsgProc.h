#ifndef PS_NCU_SCAN_MSG_PROC_H_
#define PS_NCU_SCAN_MSG_PROC_H_

#include "tulip.h"
#include "ps_mcs_define.h"
#include "ps_packet.h"

#ifndef MAX_PS_THREAD_INST_NUM
#define MAX_PS_THREAD_INST_NUM 100
#endif
/*扫描老化相关*/
typedef struct
{
    WORD32 dwCtxCap;           /*上下文容量*/
    WORD32 dwAgeTime;          /*老化时长ms*/
    WORD32 dwScanIndex;        /*扫描序列号*/
    WORD32 dwScanNum10ms;      /*扫描步长,10ms扫描上下文个数*/
}T_McsCtxScanPara;

typedef VOID (*PsMcsTableScanFun)(BYTE, BYTE, BYTE);

#define MCS_AGING_SCAN_CLOSE  0
#define MCS_AGING_SCAN_ON_NORMAL  1
#define MCS_AGING_SCAN_ON_MIGRATE 2

#define MCS_NCU_TO_PFU_SYN_SESSION_CTX_AGE_TIME  (WORD32)30   /*NCU同步请求建立的SessionCtx，保留时间为30s*/
typedef struct tagMcsTableScanTuple
{
    WORD32            dwMcsCtxType;      /*上下文句柄*/
    WORD32            dwMcsCtxSwitch;    /*开关控制:1普通打开,2迁移时也打开,0关闭*/
    CHAR              aTableName[32];    /*表名*/
    T_McsCtxScanPara  tMcsCtxScanInfo[MAX_PS_THREAD_INST_NUM];   /*扫描参数*/
    PsMcsTableScanFun pScanFun;          /*注册吊死检测函数*/
} T_McsTableScanTuple;

typedef VOID (*PsMcsScanMessageFun)(WORD16 wBufferLen, T_psNcuMediaScanMsg *ptSigHead, T_MediaProcThreadPara *ptMediaProcThreadPara);
typedef struct tagMcsScanMessageFunTuple
{
    WORD32 dwMessageType;                /* 消息类型 */
    WORD32 rsv;
    PsMcsScanMessageFun pScanMessageFun; /* 消息类型对应的处理函数 */
} T_McsScanMessageFunTuple;

typedef struct
{
    WORD32   dwMcsCtxId;       /*上下文ID*/
    WORD32   dwStampInMcsCtx;  /*上下文里记录的时间戳，用于接收线程比对 */
    BYTE     bFwdScanType;     /*转发表扫描类型，0:正常的老化扫描;1:快速扫描;  也用于SEQNUM老化场景：1:无会话；2:重插队列失败；3：重传间隔到达*/
    BYTE     bRsv[3];
    WORD32   dwCurCtxCap;
}T_psMcsCtxRelReport;

VOID psNcuInitAgeingScan();
VOID psNcuFlowAgeingFun(BYTE bTaskNo, BYTE bMcsThreadNo, BYTE bScanThreadNo);
VOID psNcuScanProc(WORD32 dwThreadNo);
VOID psNcuLoopScanPktProc(T_MediaProcThreadPara *ptMediaProcThreadPara);
VOID psNcuFlowAgeingRel(WORD16 wBufferLen, T_psNcuMediaScanMsg *ptSigHead, T_MediaProcThreadPara *ptMediaProcThreadPara);
VOID psNcuSessionAgeingFun(BYTE bTaskNo, BYTE bMcsThreadNo, BYTE bScanThreadNo);
VOID psNcuSessionAgeingRel(WORD16 wBufferLen, T_psNcuMediaScanMsg *ptSigHead, T_MediaProcThreadPara *ptMediaProcThreadPara);
VOID psNcuSnAgeingFun(BYTE bTaskNo, BYTE bMcsThreadNo, BYTE bScanThreadNo);
VOID psNcuSnAgeingRel(WORD16 wBufferLen, T_psNcuMediaScanMsg *ptSigHead, T_MediaProcThreadPara *ptMediaProcThreadPara);
VOID psNcuAppidRelateAgeingFun(BYTE bTaskNo, BYTE bMcsThreadNo, BYTE bScanThreadNo);
VOID psNcuAppidRelateAgeingRel(WORD16 wBufferLen, T_psNcuMediaScanMsg *ptSigHead, T_MediaProcThreadPara *ptMediaProcThreadPara);
WORD32 psNcuGetCtxAgeTime(const CHAR* pCtxname);
VOID psNcuGetSessionAgeCfg(WORD32 *ptCtxCap, WORD32 *ptScanNum10ms, WORD32 *ptAgeTime);
VOID psNcuGetFlowCtxAgeCfg(WORD32 *ptCtxCap, WORD32 *ptScanNum10ms, WORD32 *ptAgeTime);
VOID psNcuGetSnCtxAgeCfg(WORD32 *ptCtxCap, WORD32 *ptScanNum10ms, WORD32 *ptAgeTime);
#endif