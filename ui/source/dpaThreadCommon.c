#include "dpaThreadCommon.h"
#include "MemShareCfg.h"
#include "dpathreadpub.h"
#include "dpa_rcvsend.h"
#include "psNcuScanMsgProc.h"

extern T_VecFwdVar *pVecFwdVar;

typedef VOID (*T_CtxScanProcFunPtr)(WORD32 dwThreadNo);
T_psPfuScanStat  tPfuScanStat = {0};
T_CtxScanProcFunPtr  CtxScanProcFunAarry[SCAN_MODULE_TOTAL]=
{
    psNcuScanProc//mcs
};

WORD32 psScanTreadProc(WORD32 dwThreadNo)
{
    WORD32 dwFuncIndex = 0;
    tPfuScanStat.gwGotCallBackNum++;
    if(0 == psVpfuGetThreadParaFlag())
    {
        return -1;
    }
    for(dwFuncIndex = SCAN_MODULE_MCS; dwFuncIndex < SCAN_MODULE_TOTAL; dwFuncIndex++)
    {
        if(NULL != CtxScanProcFunAarry[dwFuncIndex] && tPfuScanStat.bModuleProcFlag[dwFuncIndex])
        {
            tPfuScanStat.gwModuleProcNum[dwFuncIndex]++;
            CtxScanProcFunAarry[dwFuncIndex](dwThreadNo);
        }
    }
    return 0;
}


int psScanTreadLocalResInit(BYTE bThrId)
{
    WORD32 dwFuncIndex = 0;
    /* 做一些初始化的动作 */
    memset(&tPfuScanStat,0, sizeof(tPfuScanStat));
    /* 614004683455:增加开关 */
    for(dwFuncIndex = SCAN_MODULE_MCS; dwFuncIndex < SCAN_MODULE_TOTAL; dwFuncIndex++)
    {
        tPfuScanStat.bModuleProcFlag[dwFuncIndex] = 1;
    }
    dpaSanThreadProcRegister((T_ThreadTimerProcFunPtr)psScanTreadProc, bThrId);
    return 0;
}


BOOLEAN  is_upf_lig_integrated()
{
    return  FALSE;
}


