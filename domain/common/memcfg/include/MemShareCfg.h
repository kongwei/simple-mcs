#ifndef _PS_MEM_SHARE_CFG_H_
#define _PS_MEM_SHARE_CFG_H_

#include "tulip.h"
#include "thdm_pub.h"
#include "psUpfCommon.h"
#include "tulip_oss.h" // for XOS_SysLog
#include "SigTrace.h"
#include "ps_mcs_define.h"
#include "ps_db_define_pfu.h"

#define PS_MAX_BUFF_LEN (WORD32)65535
#define BUILD_BUG_ON(condition) ((void)sizeof(char[1 - 2*!!(condition)]))
#define MCS_DYNCTX_NOUNIQ_EXPECTNUM  4000
#define MAX_HOTPATCH_NUM (WORD32)100

#ifdef MCS_STR_ERR_STAT_ITEM_SEG
#undef MCS_STR_ERR_STAT_ITEM_SEG
#endif
#define MCS_STR_ERR_STAT_ITEM_SEG(seg_name)

#ifdef MCS_STR_ERR_STAT_ITEM
#undef MCS_STR_ERR_STAT_ITEM
#endif
#define MCS_STR_ERR_STAT_ITEM(item_name) WORD64  item_name;

#ifdef MCS_STR_ERR_STAT_ITEM_ARRAY
#undef MCS_STR_ERR_STAT_ITEM_ARRAY
#endif
#define MCS_STR_ERR_STAT_ITEM_ARRAY(item_name,array_size) WORD64  item_name[array_size];

#ifdef MCS_STR_ERR_STAT_ITEM_TWO_ARRAY
#undef MCS_STR_ERR_STAT_ITEM_TWO_ARRAY
#endif
#define MCS_STR_ERR_STAT_ITEM_TWO_ARRAY(item_name,array_size_fir,array_size_sec) WORD64  item_name[array_size_fir][array_size_sec];

#ifdef MSC_STAT_ITEM_SEG
#undef MSC_STAT_ITEM_SEG
#endif
#define MSC_STAT_ITEM_SEG(seg_name)

#ifdef MSC_STAT_ITEM
#undef MSC_STAT_ITEM
#endif
#define MSC_STAT_ITEM(item_name) WORD64  item_name;

#define MCS_LOC_STAT_EX(pthread/*线程级统计的首地址*/, \
                        item   /*统计项*/, \
                        num    /*统计值*/)  \
    do { \
        ((pthread->item) += ((WORD64)(num))); \
    }while(0)

#define NCU_PM_55303_STAT_ADD(ITEM, NUM) \
do{\
    if (NULL != g_ptMediaProcThreadPara) \
    { \
        T_NcuCtrlFlowStatPm* PTNCUCTRLFLOW = &(g_ptVpfuShMemVar->tNcuCtrlFlow[g_ptMediaProcThreadPara->bThreadNo%MEDIA_THRD_NUM]);\
        (PTNCUCTRLFLOW->ITEM) += (NUM);\
    } \
}while(0)

#define NCU_PM_55302_STAT_ADD(ITEM, NUM) \
do{\
    if (NULL != g_ptMediaProcThreadPara) \
    { \
        T_NcuSessionStat* PTNCUSESSIONSTAT = &(g_ptVpfuShMemVar->tNcuSessionStat[g_ptMediaProcThreadPara->bThreadNo%MEDIA_THRD_NUM]);\
        (PTNCUSESSIONSTAT->ITEM) += (NUM);\
    } \
}while(0)
#define NCU_PM_55302_STAT_REMOVE(ITEM, NUM) \
do{\
    if (NULL != g_ptMediaProcThreadPara) \
    { \
        T_NcuSessionStat* PTNCUSESSIONSTAT = &(g_ptVpfuShMemVar->tNcuSessionStat[g_ptMediaProcThreadPara->bThreadNo%MEDIA_THRD_NUM]);\
        if((PTNCUSESSIONSTAT->ITEM) >= NUM) \
        {\
            (PTNCUSESSIONSTAT->ITEM) -= (NUM);\
        }\
    } \
}while(0)

#define NCU_PM_55304_STAT_ADD(ITEM, NUM) \
do{\
    if (NULL != g_ptMediaProcThreadPara) \
    { \
        T_NcuUserFlowStatPm* PTNCUUSERFLOW = &(g_ptVpfuShMemVar->tNcuUserFlow[g_ptMediaProcThreadPara->bThreadNo%MEDIA_THRD_NUM]);\
        (PTNCUUSERFLOW->ITEM) += (NUM);\
    } \
}while(0)

typedef struct tagT_psNcuMcsPerform
{
    #include "psMcsStatItemDef.h"
} T_psNcuMcsPerform;

typedef struct
{
    BYTE bMcsFirstSoftThreadNo;                             /* 第一个mcs线程软件线程号 */
    BYTE aucPad[7];
    WORD16 wGlobalSN[MEDIA_THRD_NUM];                       /* 全局SN */
    T_psNcuMcsPerform  atMcsVpfuPerform[MEDIA_THRD_NUM+1]; /* 媒体面线程对内性能统计数据区，最后一个数据元素存储历史数据 */
}T_psVNcuMcsShareMem;

typedef struct
{
    BYTE     bNetType;       /* 网元类型 */
    BYTE     bVmNetType;     /* 人物网部署类型 :DB_VM_NET_TYPE_H(1) DB_VM_NET_TYPE_M(2) DB_VM_NET_TYPE_HM(3) */
    WORD16   wMss;           /* MSSֵ */
    BYTE     bRsv[4];
    WORD32   dwCPULogicNo;   /*  */
    WORD32   dwThreadID;     /* 媒体面线程ID */
} T_psMcsConfig;

typedef struct
{
    WORD16      retCODE;         /* 返回码 */
    BYTE        baResv[2];       /* 保证8字节对齐*/
    WORD32      dwDataAreaNum;   /* 返回查询到的上下文个数 */
    BYTE*       pDataAreaAddr;   /* 根据唯一索引查询到的上下文首空间地址 */
    WORD32      adwDataArea[MCS_DYNCTX_NOUNIQ_EXPECTNUM];  /* 返回存放上下文ID的首地址 */
 } MCS_DM_QUERYDYNDATA_ACK;

typedef struct tagPSThreadCrtParaByMcs
{
    BYTE   bSoftThreadNo; /*所有软件线程统一编号，从0开始编号*/
    BYTE   aucPad[3];
    WORD32 dwDbByThreadNo ; /* 根据软件线程号获取的动态上下文库句柄 */
}T_PSThreadCrtParaByMcs;
typedef struct tagThreadInstAffinityParaByMcs
{
    #define MAX_PS_THREAD_INST_NUM 100
    BYTE               bInstNum;      /*实例个数*/
    BYTE               bPad[7];
    T_PSThreadCrtParaByMcs  atThreadParaList[MAX_PS_THREAD_INST_NUM];
}T_PSThreadInstAffinityParaByMcs;

typedef struct
{
    T_psMcsConfig       g_tMcsConfig;

    BYTE                bTeidAllocType;/*Teid 的分配方式，1-cp分配，2-up分配 */
    BYTE                bTeidAllocFlg; /* 只记录第一个消息的分配方式，后续消息以第一个消息为准，校验不过需拒绝会话消息 */
    BYTE                abPad[2];

    WORD16              psMcsGgsnGtpuSeqNum;    /* err ind 消息/gtp echo 消息 */
    WORD32              dwCurSysRate;        /* 当前使用带宽 Mbps---5s计算一次 */
    WORD64              qwLastPktNum;
    WORD64              qwLastPktLen;

    MCS_DM_QUERYDYNDATA_ACK atMcsDynCtxNoUniqAck[MEDIA_THRD_NUM+1];
}T_psMcsGloData;

typedef struct
{
    BYTE                        bvCPUNo;                                /*绑定的vCPU编号*/
    BYTE                        aucPad[3];
    WORD32                      dwhDbByVcpuNo;                          /* 根据vcpu编号计算出来的hdb库柄 */
}T_psMcsVpfuPriData;

typedef struct { 
    WORD64 qwNcuRecvSessionSyncMsgNum;                  // NCU接收会话同步消息数
    WORD64 qwNcuRecvSessionNewMsgNum;                   // NCU接收会话新建消息数
    WORD64 qwNcuRecvSessionModifyMsgNum;                // NCU接收会话修改消息数
    WORD64 qwNcuRecvSessionDeleteMsgNum;                // NCU接收会话删除消息数
    WORD64 qwNcuRecvSubscribeMsgNum;                    // NCU接收订阅消息数
    WORD64 qwNcuRecvSubscribeNewMsgNum;                 // NCU接收新增订阅消息数
    WORD64 qwNcuRecvSubscribeModifyMsgNum;              // NCU接收更改订阅消息数,走add
    WORD64 qwNcuRecvSubscribeCancelMsgNum;              // NCU接收取消订阅消息数
    WORD64 qwNcuRecvQoSSubscribeMsgFromN4Num;           // NCU接收来自N4接口的质差保障订阅消息数
    WORD64 qwNcuRecvQoSNewSubscribeMsgFromN4Num;        // NCU接收来自N4接口的质差保障新增订阅消息数
    WORD64 qwNcuRecvQoSModifySubscribeMsgFromN4Num;     // NCU接收来自N4接口的质差保障更改订阅消息数,走add
    WORD64 qwNcuRecvQoSCancelSubscribeMsgFromN4Num;     // NCU接收来自N4接口的质差保障取消订阅消息数
    WORD64 qwNcuRecvExpDataSubscribeMsgFromN4Num;       // NCU接收来自N4接口的用户体验数据订阅消息数
    WORD64 qwNcuRecvExpDataNewSubscribeMsgFromN4Num;    // NCU接收来自N4接口的用户体验数据新增订阅消息数
    WORD64 qwNcuRecvExpDataModifySubscribeMsgFromN4Num; // NCU接收来自N4接口的用户体验数据修改订阅消息数，走add
    WORD64 qwNcuRecvExpDataCancelSubscribeMsgFromN4Num; // NCU接收来自N4接口的用户体验数据取消订阅消息数
    WORD64 qwNcuRecvExpDataSubscribeMsgFromSBI;         // NCU接收来自SBI接口的用户体验数据订阅消息数
    WORD64 qwNcuRecvExpDataNewSubscribeMsgFromSBI;      // NCU接收来自SBI接口的用户体验数据新增订阅消息数
    WORD64 qwNcuRecvExpDataModifySubscribeMsgFromSBI;   // NCU接收来自SBI接口的用户体验数据修改订阅消息数
    WORD64 qwNcuRecvExpDataCancelSubscribeMsgFromSBI;   // NCU接收来自SBI接口的用户体验数据取消订阅消息数
    WORD64 qwNcuReportQoSNumViaSBI;                     // NCU通过SBI口上报质差保障次数
    WORD64 qwNcuReportKeyExpDataNumViaUDP;              // NCU通过UDP协议上报重点用户体验数据次数
    WORD64 qwNcuReportOrdinaryExpDataNumViaUDP;         // NCU通过UDP协议上报普通用户体验数据次数
    WORD64 qwContextPullRequestCount;                   // NCU发送上下文拉取请求次数
    WORD64 qwNcuReportKeyExpDataNumViaSBI;              // NCU通过SBI口上报重点业务体验次数
} T_NcuCtrlFlowStatPm;

typedef struct
{
    WORD64 qwNcuRcvCpyPktMsgs;      // NCU接收复制报文消息数
    WORD64 qwNcuRcvUlCpyPkts;       // NCU接收上行复制报文个数
    WORD64 qwNcuRcvDlCpyPkts;       // NCU接收下行复制报文个数
    WORD64 qwNcuRcvUlCpyPktBytes;   // NCU接收上行复制报文字节数（Byte）
    WORD64 qwNcuRcvDlCpyPktBytes;   // NCU接收下行复制报文字节数（Byte）
    WORD64 qwNcuRcvTrafficRptMsgs;      // NCU接收流量上报消息数
    WORD64 qwNcuRcvUlTrafficRptPktsNum;       // NCU接收上行流量上报报文个数 
    WORD64 qwNcuRcvDlTrafficRptPktsNum;       // NCU接收下行流量上报报文个数
    WORD64 qwNcuRcvUlTrafficRptPktsBytes;   // NCU接收上行流量上报报文字节数（Byte）
    WORD64 qwNcuRcvDlTrafficRptPktsBytes;   // NCU接收下行流量上报报文字节数（Byte） 
}T_NcuUserFlowStatPm;

typedef struct{
    WORD64 qwCurrentSessions;                   // NCU的当前会话数
    WORD64 qwQualityAssuranceSessions;          // NCU的质差保障会话数
    WORD64 qwKeyUserExperienceDataSessions;     // NCU的重点用户体验数据会话数
    WORD64 qwRegularUserExperienceDataSessions; // NCU的普通用户体验数据会话数
} T_NcuSessionStat;

typedef struct
{
    WORD32    dwDelMsgSendTimes;    /*需要发送消息次数*/
    WORD32    dwSesstionDelTime;
    WORD32    dwGroupId;            //ldu群号
    WORD32    dwRsv;
}T_psMcsSessDelByGroupArea;

typedef struct
{
    /* 提供给内部消息合并跨buffer的数据区 */
#define PS_MAX_PFCPPKT_LEN     (WORD32)65535 /* 接收PFCP协议报文最大长度:达到IP协议最大长度,保证接收的N4报文都回Response */
    BYTE aucInnerMsg[MEDIA_THRD_NUM][PS_MAX_PFCPPKT_LEN];    /*接收pfcp消息缓存区 */
    BYTE aucInnerAckMsg[MEDIA_THRD_NUM][PS_MAX_PFCPPKT_LEN]; /*发送pfcp消息缓存区 */

    /* 全局变量数据区 */
    T_psMcsGloData              tGwGloData;                                  /* 媒体面线程的全局变量数据区，各个线程共用  */
    T_psShMemSigTrc             tShMemSigTrcTb;
    /* 获取的线程信息begin */
    BYTE                        dwGetThreadParaFlag;                         /* 获取线程信息标志 */
    BYTE                        bHaveMcsVecNum;                              /* mcsvec线程个数 */
    WORD16                      g_wMmeFaultGlobalSN;                         /* MME故障全局SN放到共享内存里*/
    WORD32                      g_dwLastStatSec;
    WORD32                      g_dwCurStatSec;
    WORD32                      g_dwPowerOnStdSec;

    T_NcuSessionStat           tNcuSessionStat[MEDIA_THRD_NUM+1];        /* PO55302  NCU资源测量 */
    T_NcuCtrlFlowStatPm        tNcuCtrlFlow[MEDIA_THRD_NUM+1];           /* PO55303  NCU控制流测量 */
    T_NcuUserFlowStatPm        tNcuUserFlow[MEDIA_THRD_NUM+1];           /* PO55304  NCU用户流测量 */
    T_psMcsVpfuPriData         atGwPriData[MEDIA_THRD_NUM+1];  /* 媒体面线程的性能统计数据区，包括内部统计和外部统计，最后一个数据元素存储历史数据 */
    T_PSThreadInstAffinityPara      tVpfuMcsThreadPara;         /* MEDIA_TYPE_PFU_MEDIA_PROC线程参数 */
    T_PSThreadInstAffinityPara      tVpfuRcvThreadPara;         /* MEDIA_TYPE_PFU_MEDIA_RECV线程参数 */
    T_PSThreadInstAffinityParaByMcs tVpfuMcsThreadParaByMcs;    /* 经过mcs汇总过的MEDIA_TYPE_PFU_MEDIA_PROC线程参数 */
    T_PSThreadInstAffinityParaByMcs tVpfuRcvThreadParaByMcs;    /* 经过mcs汇总过的MEDIA_TYPE_PFU_MEDIA_RECV线程参数 */
    /* 获取的线程信息end */
    
    T_psMcsSessDelByGroupArea  tSessDelByGroupArea[MEDIA_THRD_NUM][MAX_UPF_GROUP_NUM];  /*按群批量删除会话*/
}T_psNcuShareMem;
extern T_psNcuShareMem    *g_ptVpfuShMemVar;
extern T_psVNcuMcsShareMem  *g_ptVpfuMcsShareMem;
WORD32 psVpfuGetThreadParaFlag();
WORD32 psGWUGetShareMem();
VOID psVpfuMcsShMemInit(void);
BOOLEAN psNcuIsFirstSoftThread();
T_NcuSessionStat* psNcuGetSessionStat(WORD32 dwThreadNo);
T_NcuCtrlFlowStatPm* psNcuGetCtrlFlowStat(WORD32 dwThreadNo);
T_NcuUserFlowStatPm* psNcuGetUserFlowStat(WORD32 dwThreadNo);
BYTE* psNcuGetAckMsgBuffer(WORD32 dwThreadNo);
WORD32 ncuGetNcuMcsInstNum();
WORD32 ncuGetNcuMcsThreadNoByInstIndex(WORD32 bSthIndex);

static INLINE BOOLEAN psNcuMcsSelfThreadIsFirstSoftThread()
{
    if(unlikely(NULL == g_ptMediaProcThreadPara || NULL == g_ptVpfuMcsShareMem))
    {
        return 0;
    }
    return (g_ptMediaProcThreadPara->bThreadNo == g_ptVpfuMcsShareMem->bMcsFirstSoftThreadNo);
}

#endif
