#include "MemShareCfg.h"
#include "ps_ftm_pub.h"
#include "ps_memsche.h"
#include "zte_slibc.h"
T_psNcuShareMem    *g_ptVpfuShMemVar = NULL; /* 提供给媒体面使用的共享内存  */
T_psVNcuMcsShareMem  *g_ptVpfuMcsShareMem = NULL;
T_psGlobalVar g_psGlobalVar;

extern WORD32 HugePageMemMalloc(CHAR* memName, WORD64 *pMemAddr, WORD64 *pMemSize);

WORD32 psGWUGetShareMem()
{
    WORD64 ullVirMem = 0; 
    WORD64 ullMemSize = 0; 
    
    /* PGW MCS share 内存申请*/
    ullMemSize = (WORD64)VIR_MEM_VPFUMCS_SHARE_SIZE;   //MCS的值要用全局变量初始化，在ctrl中
    #ifdef FT_TEST
    ullMemSize = (56*1024*1024);
    if(PS_HUGEMEM_OK != HugePageMemMallocFT("PS_HUGEMEM_PGW_MCS", &ullVirMem, &ullMemSize))
    #else
    if(PS_HUGEMEM_OK != HugePageMemMalloc("PS_HUGEMEM_PGW_MCS", &ullVirMem, &ullMemSize))
    #endif
    {
       XOS_SysLog(LOG_ALERT,"############[MCS]Huge Page get mem block PS_HUGEMEM_PGW_MCS failed, memSize = 0x%08x!! Func: %s  Line: %u\n",
                  VIR_MEM_VPFUMCS_SHARE_SIZE, __FUNCTION__, __LINE__);
       zte_printf_s("############[MCS]Huge Page get mem block PS_HUGEMEM_PGW_MCS failed, memSize = 0x%llx!! Func: %s  Line: %d\n", 
               (WORD64)VIR_MEM_VPFUMCS_SHARE_SIZE, __FUNCTION__, __LINE__);
       return PS_HUGEMEM_FAIL;
    }
    XOS_SysLog(LOG_ALERT, "#############[MCS]Huge Page get mem PS_HUGEMEM_PGW_MCS block sucess, vir_addr = %p, memSize = 0x%llx\n",(void*)ullVirMem, ullMemSize);
    zte_printf_s("#############[MCS]Huge Page get mem PS_HUGEMEM_PGW_MCS block sucess, vir_addr = %p, memSize = 0x%llx\n",(void*)ullVirMem, ullMemSize);
    Vir_Mem_MSC_Share_Addr = (ULONG)ullVirMem;

    /*给共享内存指针赋值*/
    g_ptVpfuShMemVar = (T_psNcuShareMem *)ullVirMem;

    /* 修改为编译时检查，下面的运行时检查注掉 */
    BUILD_BUG_ON((sizeof(T_psNcuShareMem)+sizeof(T_psVNcuMcsShareMem)) >= VIR_MEM_VPFUMCS_SHARE_SIZE);

    if((ULONG)g_ptVpfuShMemVar%8 != 0)
    {
        zte_printf_s( "[MCS]:g_ptVpfuShMemVar %p not 8 aline!! Func: %s  Line: %d\n" ,g_ptVpfuShMemVar, __FUNCTION__, __LINE__);
        return PS_HUGEMEM_FAIL;
    }

    zte_memset_s(g_ptVpfuShMemVar, sizeof(T_psNcuShareMem), 0, sizeof(T_psNcuShareMem));

    /* V7底层不会再调用psVpfuGetMediaShMemOfMcs */
    g_ptVpfuMcsShareMem = (T_psVNcuMcsShareMem *)(ullVirMem + sizeof(T_psNcuShareMem));
    zte_memset_s(g_ptVpfuMcsShareMem, sizeof(T_psVNcuMcsShareMem), 0, sizeof(T_psVNcuMcsShareMem));
    
    return PS_HUGEMEM_OK;
}

DWORD _db_get_capacity_by_tblName(BYTE tblIdx, char *tblName)
{
    return 1;
}


/**********************************************************************
* 函数名称：  psVpfuMcsShMemInit
* 功能描述：  mcs初始化共享内存
* 访问的表：
* 修改的表：
* 输入参数：

* 输出参数：
* 返 回 值：  无
* 其它说明：  ԭpgw psMcsManageJobBuffSInit
***********************************************************************/
VOID psVpfuMcsShMemInit(void)
{
    /* wjgu 2010 2 27  */
    g_ptVpfuShMemVar = (T_psNcuShareMem *)Vir_Mem_MSC_Share_Addr;
    /* 修改为编译时检查，下面的运行时检查注掉 */
    BUILD_BUG_ON(sizeof(T_psNcuShareMem) > VIR_MEM_VPFUMCS_SHARE_SIZE);    
    /*if (sizeof(T_psVpfuShMemVar) > VIR_MEM_VPFUMCS_SHARE_SIZE)  //for KW
    {
        PS_MCSMANAGE_TRACE(PRN_LEVEL_HIGHEST, "\n[McsManageJob]Init share memory failed! need size=%lu, actural size=%u, func:%s line:%d", sizeof(T_psVpfuShMemVar), VIR_MEM_VPFUMCS_SHARE_SIZE, __FUNCTION__, __LINE__);
        printf("\n sizeof(T_psVpfuShMemVar) > VIR_MEM_VPFUMCS_SHARE_SIZE \n");
        XOS_ASSERT(0);
        return ;
    }*/
    if (NULL == g_ptVpfuShMemVar)
    {
        printf("\n NULL == g_ptVpfuShMemVar \n");
        XOS_ASSERT(0);
        return ;
    }
    /*统计相关*/
    g_ptVpfuShMemVar->g_dwLastStatSec   = psFtmGetPowerOnSec();
    g_ptVpfuShMemVar->g_dwPowerOnStdSec = psFtmGetCurStdSec();
    return;
}
WORD32 psVpfuGetThreadParaFlag()
{
    return g_ptVpfuShMemVar->dwGetThreadParaFlag;
}
BOOLEAN psNcuIsFirstSoftThread()
{
    return psNcuMcsSelfThreadIsFirstSoftThread();
}
T_NcuSessionStat* psNcuGetSessionStat(WORD32 dwThreadNo)
{
    if(unlikely(NULL == g_ptVpfuShMemVar || dwThreadNo >= MEDIA_THRD_NUM))
    {
        return NULL;
    }
    return &(g_ptVpfuShMemVar->tNcuSessionStat[dwThreadNo]);
}
T_NcuCtrlFlowStatPm* psNcuGetCtrlFlowStat(WORD32 dwThreadNo)
{
    if(unlikely(NULL == g_ptVpfuShMemVar || dwThreadNo >= MEDIA_THRD_NUM))
    {
        return NULL;
    }
    return &(g_ptVpfuShMemVar->tNcuCtrlFlow[dwThreadNo]);
}
T_NcuUserFlowStatPm* psNcuGetUserFlowStat(WORD32 dwThreadNo)
{
    if(unlikely(NULL == g_ptVpfuShMemVar || dwThreadNo >= MEDIA_THRD_NUM))
    {
        return NULL;
    }
    return &(g_ptVpfuShMemVar->tNcuUserFlow[dwThreadNo]);
}
BYTE* psNcuGetAckMsgBuffer(WORD32 dwThreadNo)
{
    if(unlikely(NULL == g_ptVpfuShMemVar || dwThreadNo >= MEDIA_THRD_NUM))
    {
        return NULL;
    }
    return &(g_ptVpfuShMemVar->aucInnerAckMsg[dwThreadNo][0]);
}

//以下2接口给Rust调用
WORD32 ncuGetNcuMcsInstNum()
{
    if(unlikely(NULL == g_ptVpfuShMemVar))
    {
        return 0;
    }
    return (WORD32)(g_ptVpfuShMemVar->tVpfuMcsThreadPara.bInstNum);  
}

WORD32 ncuGetNcuMcsThreadNoByInstIndex(WORD32 bSthIndex)
{
    if(unlikely(NULL == g_ptVpfuShMemVar || bSthIndex >= MAX_PS_THREAD_INST_NUM))
    {
      return 0;
    }
    return (WORD32)(g_ptVpfuShMemVar->tVpfuMcsThreadPara.atThreadParaList[bSthIndex].bSoftThreadNo);
}
