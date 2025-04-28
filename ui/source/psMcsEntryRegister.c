#include "psMcsEntryRegister.h"
#include "psMcsEntry.h"
#include "MemShareCfg.h"
#include "dpathreadpub.h"
#include "ps_db_define_ncu.h"
#include "psNcuReport.h"
#include "psNcuReportTimer.h"
extern void test_report();
extern WORD32  psGetMediaThreadInfo(BYTE bMediaType,T_PSThreadInstAffinityPara* pPSThreadInstAffinitytable);
VOID psVpfuGetMediaThreadPara()
{
    T_PSThreadInstAffinityPara tThreadPara = {0}; 
    T_PSThreadInstAffinityParaByMcs *pThreadInstAffinityParaByMcs = NULL; 
    WORD32 dwPsmStatus   = THDM_FAIL; 
    WORD32 dwThreadIndex = 0;

#define VPFU_GET_MEDIA_THD_PARA(bMediaType, T_VpfuThreadPara) \
do{ \
    memset(&tThreadPara,0,sizeof(T_PSThreadInstAffinityPara));\
    dwPsmStatus = THDM_FAIL; \
    dwPsmStatus = psGetMediaThreadInfo(bMediaType,&tThreadPara);\
    if(THDM_SUCCESS == dwPsmStatus)\
    {\
        pThreadInstAffinityParaByMcs = &g_ptVpfuShMemVar->T_VpfuThreadPara##ByMcs;\
        memset(&g_ptVpfuShMemVar->T_VpfuThreadPara,0,sizeof(T_PSThreadInstAffinityPara));\
        memcpy(&g_ptVpfuShMemVar->T_VpfuThreadPara, &tThreadPara,sizeof(T_PSThreadInstAffinityPara));\
        pThreadInstAffinityParaByMcs->bInstNum = tThreadPara.bInstNum;\
        for(dwThreadIndex = 0; ((dwThreadIndex < tThreadPara.bInstNum) && (dwThreadIndex < MAX_PS_THREAD_INST_NUM));dwThreadIndex++)\
        {\
            BYTE bSoftThreadNo = tThreadPara.atThreadParaList[dwThreadIndex].bSoftThreadNo%CSS_X86_MAX_THREAD_NUM;\
            BYTE bvCPUNo       = tThreadPara.atThreadParaList[dwThreadIndex].bvCPUNo;\
            pThreadInstAffinityParaByMcs->atThreadParaList[dwThreadIndex].bSoftThreadNo = bSoftThreadNo;\
            pThreadInstAffinityParaByMcs->atThreadParaList[dwThreadIndex].dwDbByThreadNo = _NCU_GET_DBHANDLE(bSoftThreadNo);\
            g_ptVpfuShMemVar->atGwPriData[bSoftThreadNo].bvCPUNo       = bvCPUNo;\
            g_ptVpfuShMemVar->atGwPriData[bSoftThreadNo].dwhDbByVcpuNo = _NCU_GET_DBHANDLE(bSoftThreadNo);\
        }\
        if(THDM_MEDIA_TYPE_PFU_MEDIA_PROC == bMediaType)\
        {\
            /* 获取第一个个mcs线程的软件线程号 */\
            g_ptVpfuMcsShareMem->bMcsFirstSoftThreadNo = pThreadInstAffinityParaByMcs->atThreadParaList[0].bSoftThreadNo;\
        }\
    }\
}while(0)

    /* 获取媒体面业务线程的线程情况 */
    VPFU_GET_MEDIA_THD_PARA(THDM_MEDIA_TYPE_PFU_MEDIA_PROC, tVpfuMcsThreadPara);

    /* 获取disp线程的线程情况 */
    VPFU_GET_MEDIA_THD_PARA(THDM_MEDIA_TYPE_PFU_MEDIA_RECV, tVpfuRcvThreadPara);

    g_ptVpfuShMemVar->dwGetThreadParaFlag++;
    return;
}


VOID psSetMcsThreadPara(T_MediaProcThreadPara *ptMediaProcThreadPara,BYTE bSthr)
{
    if(NULL == ptMediaProcThreadPara)
    {
        return;
    }

    ptMediaProcThreadPara->bThreadNo            = bSthr;
    ptMediaProcThreadPara->dwhDBByThreadNo      = _NCU_GET_DBHANDLE(bSthr);
    ptMediaProcThreadPara->ptMcsStatPointer     = (VOID*)&(g_ptVpfuMcsShareMem->atMcsVpfuPerform[bSthr]);
    return;
}

WORD32 g_test_thread1 = 0xff;
WORD32 g_time_int = 10000;
WORD32 g_num = 0;
void psVpfuMcs5MsTimeProc(WORD32 dwThreadNo)
{
    
    if(dwThreadNo == g_test_thread1)
    {
        g_num++;
        if(0 != g_time_int && g_num %g_time_int == 10)
        {
            test_report();
        }
    }
}

#define MCS_US_NUM_PER_SECOND (1000 * 1000)
INT psMcsLocalResInit(T_MediaProcThreadPara *ptMediaProcThreadPara,BYTE bThrId)
{
    if(NULL == ptMediaProcThreadPara)
    {
        return -1;
    }
    /* pgw\sgw内存初始化 */
    psVpfuMcsShMemInit();
    if(NULL == g_ptVpfuShMemVar)
    {
        zte_printf_s("\n[Mcs]g_ptVpfuShMemVar = %p, func:%s line:%d", g_ptVpfuShMemVar, __FUNCTION__, __LINE__);
        return -1;
    }

    /* 初始化全局变量 */





    if(bThrId >= MEDIA_THRD_NUM)
    {
         return -1;
    }

    if(0 == psVpfuGetThreadParaFlag())
    {
        psVpfuGetMediaThreadPara();
    }
    /*T_PSThreadCrtPara* ptThrCrtPara = psMcsGetThreadCrtParaBySthr(bThrId); //after psVpfuGetMediaThreadPara
    if(NULL != ptThrCrtPara)
    {
        ptMediaProcThreadPara->bvCPUIndex = ptThrCrtPara->bvCPUIndex;
        zte_printf_s("psMcsLocalResInit: bThrId(%u), bvCPUIndex(%u)\n", bThrId, ptThrCrtPara->bvCPUIndex);
    }*/

    /* 上电时初始化大文件和小文件正则表达式 gao.shencun, 2015.09.09 */
    psSetMcsThreadPara(ptMediaProcThreadPara, bThrId);
    ptMediaProcThreadPara->ddwCpuHz = rte_get_tsc_hz();

    /* 注册业务媒体面报文转发函数 */
    dpaMediaThreadProcRegister((T_MediaServiceFunPtr)psVpfuMcsIPPktRcv, (WORD32)bThrId);
    /* 注册一个定时器处理消息 */
    dpaMediaThread5msTimerFunRegister((T_ThreadTimerProcFunPtr)psVpfuMcs5MsTimeProc,bThrId);

    /* 注册硬件卸载获取控制消息函数 */

    /* 注册硬件卸载生成控制消息扩展头函数 */

    /* 注册硬件卸载承载变更函数 */




    psNcuDataReportTimerInit();

    return 0;
}

