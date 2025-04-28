#ifndef DPA_THREAD_COMMON_H_
#define DPA_THREAD_COMMON_H_

#include "thdm_pub.h"
/* 扫描线程驻留模块 */
typedef enum
{
    SCAN_MODULE_MCS= 0,         /* mcs 模块*/
    SCAN_MODULE_TOTAL
}Thread_ScanModule;

typedef struct
{
    /*统计值 */
    WORD32 gwGotCallBackNum;                   /* 被承载调用次数 */
    BYTE   bModuleProcFlag[SCAN_MODULE_TOTAL]; /* 各模块资源回收开关 */
    WORD64 gwModuleProcNum[SCAN_MODULE_TOTAL]; /* 各模块调用次数 */

} T_psPfuScanStat;

void psMediaLoopScanEntry(T_PSThreadAffinityInfo  *ptThreadAffinityInfo);
void psNetRcvEntry(T_PSThreadAffinityInfo  *ptThreadAffinityInfo);

#endif