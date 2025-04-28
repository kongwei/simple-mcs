#include "psMcsDebug.h"
#include "McsIPv4Head.h"
#include "MemShareCfg.h"
#include "zte_slibc.h"
#include "psNcuSnCtxProc.h"
#include "xdb_core_pfu.h"
#include "ps_db_define_pfu.h"
#include "ps_ncu_typedef.h"
#include "ps_mcs_define.h"
#include "UPFHelp.h"
WORD32 g_ncu_show = -1;
WORD32 g_buff_trace = 0;
WORD32 g_job_show = -1;
extern DBBOOL _xdb_pfu_show_tbl_mem(DWORD dwDbHandle);
extern DBBOOL _xdb_pfu_show_tbl_mem_ex(DWORD dwDbHandle, DWORD *pTotalTupleNum, BYTE bInstNum);
extern DBBOOL _xdb_pfu_show_total_tbl_mem(DWORD dwDbHandle, DWORD *pTotalTupleNum);

/* 热补丁专用调试开关 */
WORD32 g_hotPatch_1 = 0;
WORD32 g_hotPatch_2 = 0;
WORD32 g_hotPatch_3 = 0;
WORD32 g_hotPatch_4 = 0;
WORD32 g_hotPatch_5 = 0;
WORD32 g_hotPatch_6 = 0;
WORD32 g_hotPatch_7 = 0;
WORD32 g_hotPatch_8 = 0;
WORD32 g_hotPatch_9 = 0;
WORD32 g_hotPatch_10 = 0;

/* 热补丁专用函数 */
VOID psNcuHotPatchVoid_1() {}
VOID psNcuHotPatchVoid_2() {}
VOID psNcuHotPatchVoid_3() {}
VOID psNcuHotPatchVoid_4() {}
VOID psNcuHotPatchVoid_5() {}
VOID psNcuHotPatchInput_1(WORD32 inputA, WORD32 inputB) {}
VOID psNcuHotPatchInput_2(WORD32 inputA, WORD32 inputB) {}
VOID psNcuHotPatchInput_3(WORD32 inputA, WORD32 inputB) {}
VOID psNcuHotPatchInput_4(WORD32 inputA, WORD32 inputB) {}
VOID psNcuHotPatchInput_5(WORD32 inputA, WORD32 inputB) {}

T_psNcuMcsPerform *psVpfuMcsGetPerformPtr(WORD32 dwSthrIdx)
{
    if(unlikely(NULL == g_ptVpfuMcsShareMem))
    {
        return NULL;
    }
    return (T_psNcuMcsPerform *)&(g_ptVpfuMcsShareMem->atMcsVpfuPerform[dwSthrIdx%MEDIA_THRD_NUM]);
}

VOID gsastat(T_psNcuMcsPerform* ptMcsPerform)
{
    if(NULL == ptMcsPerform) return;
#undef MCS_STR_ERR_STAT_ITEM_SEG
#define MCS_STR_ERR_STAT_ITEM_SEG(seg_name) \
{ \
    zte_printf_s("\n  --------------------------------- %-12s -----------------------------  \n", #seg_name); \
}

#undef MCS_STR_ERR_STAT_ITEM
#define MCS_STR_ERR_STAT_ITEM(item_name)  do \
{ \
    if(ptMcsPerform->item_name>0) \
        zte_printf_s("  %40s : %llu\n", #item_name, ptMcsPerform->item_name); \
}while(0);

#undef MCS_STR_ERR_STAT_ITEM_ARRAY
#define MCS_STR_ERR_STAT_ITEM_ARRAY(item_name, array_size) do\
{\
    int i=0;\
    for(i=0; i<array_size; i++)\
    {\
        if(ptMcsPerform->item_name[i]>0) \
            zte_printf_s("  %36s[%2d] : %llu\n", #item_name, i, ptMcsPerform->item_name[i]); \
    }\
}while(0);

#undef MCS_STR_ERR_STAT_ITEM_TWO_ARRAY
#define MCS_STR_ERR_STAT_ITEM_TWO_ARRAY(item_name, array_size_fir, array_size_sec) do\
{\
    int i=0;\
    int j=0;\
    for(i=0; i<array_size_fir; i++)\
        for(j=0;j<array_size_sec;j++)\
        {\
            if(ptMcsPerform->item_name[i][j]>0)\
                zte_printf_s("  %36s[%2d][%2d] : %llu\n", #item_name, i, j, ptMcsPerform->item_name[i][j]);\
        }\
}while(0);

    #include "psMcsStatItemDef.h"

    zte_printf_s("\n");
    return;
}

VOID psVpfuMcsPrintStat(T_psNcuMcsPerform* ptMcsPerform)
{
    if(NULL == ptMcsPerform) return;
    gsastat(ptMcsPerform);
    
    zte_printf_s("\n");
    return;
}
VOID psNcuMcsShowStat(WORD32 dwThreadID)
{
    static T_psNcuMcsPerform tMcsPerform = {0};
    WORD32  dwID = 0,dwBegin = 0,dwEnd = 0;

    zte_memset_s(&tMcsPerform, sizeof(tMcsPerform), 0 ,sizeof(tMcsPerform));
    if(dwThreadID >= MEDIA_THRD_NUM)
    {
        dwBegin = 0;
        dwEnd = MEDIA_THRD_NUM;
    }
    else
    {
        dwBegin = dwThreadID;
        dwEnd = dwThreadID+1;
    }

#define VPFU_MCS_STAT_SUM(ptSum, ptSrcStat) \
{ \
    for(dwID = dwBegin; dwID < dwEnd; dwID++) \
    { \
        WORD32  dwTotal  = sizeof(T_psNcuMcsPerform)/sizeof(WORD64); \
        WORD32  dwIndex  = 0; \
        WORD64  *pgwSum  = (WORD64*)ptSum; \
        WORD64  *pgwTemp = (WORD64*)ptSrcStat(dwID); \
        if(NULL==pgwTemp) return ; \
        for(dwIndex = 0; dwIndex < dwTotal;  dwIndex++) \
        { \
            pgwSum[dwIndex] += pgwTemp[dwIndex]; \
        } \
    } \
}
    VPFU_MCS_STAT_SUM(&tMcsPerform, psVpfuMcsGetPerformPtr);

    /*打印*/
    psVpfuMcsPrintStat(&tMcsPerform);
    return;
}
UPF_HELP_REG("mcs","psNcuShowAllThreadStat",
VOID psNcuShowAllThreadStat())
{
    T_PSThreadInstAffinityPara *ptThrPara = &g_ptVpfuShMemVar->tVpfuMcsThreadPara;
    WORD32 dwThreadIndex = 0;
    zte_printf_s("\n ============================ Total MediaThread Num: %02u =========================",ptThrPara->bInstNum);
    for(dwThreadIndex = 0; (dwThreadIndex < ptThrPara->bInstNum); dwThreadIndex++)
    {
        zte_printf_s("\n  ====================== ThreadIdx:%02u (ThreadNo:%02u,vCpuNo:%02u) ==================",
                dwThreadIndex,
                ptThrPara->atThreadParaList[dwThreadIndex].bSoftThreadNo,
                ptThrPara->atThreadParaList[dwThreadIndex].bvCPUNo);
        psNcuMcsShowStat(ptThrPara->atThreadParaList[dwThreadIndex].bSoftThreadNo);
    }
    zte_printf_s("\n");
    return;
}

UPF_HELP_REG("mcs","ncugsa",
VOID ncugsa(WORD32 dwThreadNum))
{
    if(dwThreadNum == 255)
    {
        psNcuShowAllThreadStat();
    }
    else if(dwThreadNum == 0)
    {
        psNcuMcsShowStat(MEDIA_THRD_NUM);
    }
    else
    {
        psNcuMcsShowStat(dwThreadNum);
    }


    zte_printf_s("\n Tips: ncugsa(255):StatByThd.");
    zte_printf_s("\n\n");
    return;
}

VOID psNcuMcsCleanStat(WORD32 dwThreadID)
{
    T_psNcuMcsPerform *ptNcuPerform = NULL;
    WORD32 dwID    = 0;
    WORD32 dwBegin = 0;
    WORD32 dwEnd   = 0;

    if(dwThreadID >= MEDIA_THRD_NUM)
    {
        dwBegin = 0;
        dwEnd   = MEDIA_THRD_NUM;
    }
    else
    {
        dwBegin = dwThreadID;
        dwEnd   = dwThreadID+1;
    }

    for(dwID = dwBegin; dwID<dwEnd; dwID++)
    {
        ptNcuPerform = psVpfuMcsGetPerformPtr(dwID);
        if(NULL != ptNcuPerform)
        {
            zte_memset_s(ptNcuPerform, sizeof(T_psNcuMcsPerform), 0, sizeof(T_psNcuMcsPerform));
        }
    }
    return;
}

/* 清空热补丁统计项[start，end] start从0开始，end最大99*/
/* Started by AICoder, pid:p9f32j52873b8f6145d90907a0d8dc3228a220a8 */
VOID psNcuCleanHotpatchStat(WORD32 start, WORD32 end)
{
    // 检查输入参数是否有效
    if ((start > end) || (end >= MAX_HOTPATCH_NUM))
    {
        zte_printf_s("Invalid range: start > end or array out of range");
        return;
    }

    // 检查共享内存指针是否为空
    if (g_ptVpfuMcsShareMem == NULL)
    {
        zte_printf_s("g_ptVpfuMcsShareMem is null!");
        return;
    }

    // 获取线程数量
    WORD32 dwID = 0;
    WORD32 dwIdx = 0;

    // 遍历所有线程并清空相应的热补丁状态
    for (dwID = 0; dwID < MEDIA_THRD_NUM; dwID++)
    {
        T_psNcuMcsPerform *ptNcuPerform = psVpfuMcsGetPerformPtr(dwID);
        if (ptNcuPerform != NULL)
        {
            for (dwIdx = start; dwIdx < end + 1; dwIdx++)
            {
                ptNcuPerform->HotpatchStat[dwIdx] = 0;
            }
        }
    }
}
/* Ended by AICoder, pid:p9f32j52873b8f6145d90907a0d8dc3228a220a8 */

UPF_HELP_REG("mcs","ncugsac",
VOID ncugsac())
{
    psNcuMcsCleanStat(MEDIA_THRD_NUM);
    return;
}

/* Started by AICoder, pid:v2086j927e2e246149c10aed5014c41e23f7a601 */
UPF_HELP_REG("mcs","ncushowtipcstat",
VOID ncushowtipcstat())
{
    T_psNcuMcsPerform *ptNcuPerform = NULL;
    WORD32 dwID    = 0;
    WORD64 item_name = 0;

    for(; dwID < MEDIA_THRD_NUM; dwID++)
    {
        ptNcuPerform = psVpfuMcsGetPerformPtr(dwID);
        if(NULL != ptNcuPerform)
        {
            item_name += ptNcuPerform->SendQosAnaNotifyByLinkSucc;
        }
    }

    zte_printf_s("SendQosAnaNotifyByLinkSucc = %llu\n", item_name);
}
/* Ended by AICoder, pid:v2086j927e2e246149c10aed5014c41e23f7a601 */

VOID psVpfuMcsShowNcuMcsSnCtxInfo(T_NcuMcsSnCtx *ptNcuMcsSnCtx)
{
    if (NULL == ptNcuMcsSnCtx)
    {
        return ;
    }
    zte_printf_s("\n     dwFlowId                            : %u", ptNcuMcsSnCtx->dwFlowId);
    zte_printf_s("\n     dwSeqNumber                         : %u", ptNcuMcsSnCtx->dwSeqNum);
    zte_printf_s("\n     bDir                                : %u", ptNcuMcsSnCtx->bDir);
    zte_printf_s("\n     ddwTimeStamp                        : %llu", ptNcuMcsSnCtx->ddwTimeStamp);
    zte_printf_s("\n     dwNcuMcsSnCtxID                     : %u", ptNcuMcsSnCtx->dwCtxID);
    zte_printf_s("\n     wPayLoad                            : %u", ptNcuMcsSnCtx->wPayLoad);  
    zte_printf_s("\n     ptSnNext                            : %p", ptNcuMcsSnCtx->ptSnNext);
    zte_printf_s("\n     ptSnPrev                            : %p", ptNcuMcsSnCtx->ptSnPrev);
    return;
}

VOID vncushowtblmem(WORD32 ThreadNo)
{
    extern atomic_t_32 dwMediaInitFlag;
    WORD16 wThreadIndex = 0;
    WORD32 dwMediaInitCnt;

    dwMediaInitCnt = atomic_read_32(&dwMediaInitFlag);
    MCS_CHK_VOID(0 == dwMediaInitCnt || NULL == g_ptVpfuShMemVar);

    if(ThreadNo >= MEDIA_THRD_NUM)
    {
        zte_printf_s("\n Common db tbl info:\n");
        _xdb_pfu_show_tbl_mem(_PFU_DBHANDLE_COMM);

        T_PSThreadInstAffinityParaByMcs *ptThreadInstAffinityParaByMcs = &g_ptVpfuShMemVar->tVpfuMcsThreadParaByMcs;
        zte_printf_s("\n Mcs ThreadNum(%u): \n ",ptThreadInstAffinityParaByMcs->bInstNum);
        for(wThreadIndex = 0; (wThreadIndex < MAX_PS_THREAD_INST_NUM)&&(wThreadIndex < ptThreadInstAffinityParaByMcs->bInstNum); wThreadIndex++)
        {
            ThreadNo =  ptThreadInstAffinityParaByMcs->atThreadParaList[wThreadIndex].bSoftThreadNo;
            zte_printf_s("ThreadNo (%u) db tbl info:\n",ThreadNo);
            _xdb_pfu_show_tbl_mem(ThreadNo);
        }

    }
    else
    {
        zte_printf_s("\n ");
        _xdb_pfu_show_tbl_mem(ThreadNo);

    }
    zte_printf_s("\n ");
    return;
}
UPF_HELP_REG("mcs","vncushowtotaltblmem",
VOID vncushowtotaltblmem())
{
    WORD16 wThreadIndex = 0;
    WORD32 ThreadNo     = 0;
    DWORD  g_dwTotalTupleNum[XDB_PFU_TABLE_NUM] = {0};

    zte_printf_s("\n Common db tbl info:\n");
    _xdb_pfu_show_tbl_mem(_PFU_DBHANDLE_COMM);

    T_PSThreadInstAffinityParaByMcs *ptThreadInstAffinityParaByMcs = &g_ptVpfuShMemVar->tVpfuMcsThreadParaByMcs;
    zte_printf_s("\n Mcs ThreadNum(%u): \n",ptThreadInstAffinityParaByMcs->bInstNum);
    for(wThreadIndex = 0; (wThreadIndex < MAX_PS_THREAD_INST_NUM)&&(wThreadIndex < ptThreadInstAffinityParaByMcs->bInstNum); wThreadIndex++)
    {
        ThreadNo =  ptThreadInstAffinityParaByMcs->atThreadParaList[wThreadIndex].bSoftThreadNo;
        _xdb_pfu_show_total_tbl_mem(ThreadNo,g_dwTotalTupleNum);
    }
    _xdb_pfu_show_tbl_mem_ex(ThreadNo, g_dwTotalTupleNum, ptThreadInstAffinityParaByMcs->bInstNum);
    return;
}

UPF_HELP_REG("mcs","vncushowthreadpara",
VOID vncushowthreadpara())
{
    WORD32 dwThreadIndex = 0;

#define VPFU_MCS_SHOW_THRD_PARA(ptInstPara, ptInstPara_mcs) \
{ \
    for(dwThreadIndex = 0; (dwThreadIndex < MAX_PS_THREAD_INST_NUM)&&(dwThreadIndex < ptInstPara->bInstNum); dwThreadIndex++) \
    {  \
        zte_printf_s("\n     Item(%u) :bvCPUNo(%u) bSoftThreadNo(%u) bIntraThreadNo(%u) bNeedSleep(%u) dwSleepMS(%u)", \
                                            dwThreadIndex, \
                                            ptInstPara->atThreadParaList[dwThreadIndex].bvCPUNo, \
                                            ptInstPara->atThreadParaList[dwThreadIndex].bSoftThreadNo, \
                                            ptInstPara->atThreadParaList[dwThreadIndex].bIntraThreadNo, \
                                            ptInstPara->atThreadParaList[dwThreadIndex].bNeedSleep, \
                                            ptInstPara->atThreadParaList[dwThreadIndex].dwSleepMS); \
        zte_printf_s("\n              :bSoftThreadNo(%u) dwDbByThreadNo(%u)", \
                                            ptInstPara_mcs->atThreadParaList[dwThreadIndex].bSoftThreadNo, \
                                            ptInstPara_mcs->atThreadParaList[dwThreadIndex].dwDbByThreadNo); \
    } \
}
    T_PSThreadInstAffinityPara *ptThreadInstAffinityPara            = &g_ptVpfuShMemVar->tVpfuMcsThreadPara;
    T_PSThreadInstAffinityParaByMcs *ptThreadInstAffinityParaByMcs  = &g_ptVpfuShMemVar->tVpfuMcsThreadParaByMcs;
    zte_printf_s("\n dwGetThreadParaFlag(%u)", g_ptVpfuShMemVar->dwGetThreadParaFlag);
    zte_printf_s("\n bHaveMcsVecNum(%u)", g_ptVpfuShMemVar->bHaveMcsVecNum);
    zte_printf_s("\n\n Mcs ThreadPara, ThreadNum(%u - %u)", ptThreadInstAffinityPara->bInstNum,ptThreadInstAffinityParaByMcs->bInstNum);
    VPFU_MCS_SHOW_THRD_PARA(ptThreadInstAffinityPara,  ptThreadInstAffinityParaByMcs);

    ptThreadInstAffinityPara       = &g_ptVpfuShMemVar->tVpfuRcvThreadPara;
    ptThreadInstAffinityParaByMcs  = &g_ptVpfuShMemVar->tVpfuRcvThreadParaByMcs;
    zte_printf_s("\n\n Recv ThreadPara,ThreadNum(%u - %u)",ptThreadInstAffinityPara->bInstNum,ptThreadInstAffinityParaByMcs->bInstNum);
    VPFU_MCS_SHOW_THRD_PARA(ptThreadInstAffinityPara, ptThreadInstAffinityParaByMcs);
    zte_printf_s("\n");
    return;
}

UPF_HELP_REG("mcs","bufftraceon",
VOID bufftraceon(WORD32 dir))
{
    g_buff_trace = dir;
}
UPF_HELP_REG("mcs","bufftraceoff",
VOID bufftraceoff())
{
    g_buff_trace = 0;
}
UPF_HELP_REG("mcs","ncuopen",
VOID ncuopen(WORD32 level))
{
    g_ncu_show = level;
}
UPF_HELP_REG("mcs","ncuclose",
VOID ncuclose())
{
    g_ncu_show = -1;
}

UPF_HELP_REG("mcs","show ncuhelp",
VOID ncuhelp())
{
    zte_printf_s("\n --------------------------NCU Debug Fun---------------------------");
    zte_printf_s("\n vncushowthreadpara()                             -- show media Thread Para ");
    zte_printf_s("\n vncushowtblmem(ThreadNo)                         -- show media Thread db ctx ");
    zte_printf_s("\n vncushowtotaltblmem()                            -- show all media Thread db ctx ");
    zte_printf_s("\n ncugsa()                                         -- show all media stat ");
    zte_printf_s("\n ncugsac()                                        -- clear all media stat ");
    zte_printf_s("\n ncujob()                                         -- show all media job stat ");
    zte_printf_s("\n ncujobc()                                        -- clear all media job stat ");
    zte_printf_s("\n ncuopen(level)                                   -- open media log ");
    zte_printf_s("\n ncuclose()                                       -- close media log ");
    zte_printf_s("\n bufftraceon(dir)                                 -- open media pkt buff log");
    zte_printf_s("\n bufftraceoff()                                   -- close media pkt buff log ");
    zte_printf_s("\n");
}

