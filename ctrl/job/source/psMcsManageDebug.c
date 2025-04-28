/* Started by AICoder, pid:ea6d2531f2284fb14132094be05ad95955f6ea64 */
/******************************************************************************
 * 版权所有 (C)2016 深圳市中兴通讯股份有限公司*
 * 模块名          : MCS
 * 文件名          : psMcsManageDebug.c
 * 相关文件        :
 * 文件实现功能     : NCU MCS Debug函数实现
 * 归属团队        : 
 * 版本           : V7.24.20
-------------------------------------------------------------------------------
* 修改记录:
* 修改日期            版本号           修改人           修改内容
* 2024-04-25        V7.24.20        m6               创建
******************************************************************************/
/**************************************************************************
 *                              头文件(满足最小依赖请按照DDD分层架构逐层依赖)
 **************************************************************************/
#include "tulip.h"
#include "UPFHelp.h"
#include "zte_slibc.h"
#include "MemShareCfg.h"
#include "ps_mcs_define.h"

#include "psUpfSessionStruct.h"

/**************************************************************************
 *                                  宏(本源文件使用)                 
 **************************************************************************/

/************************************************************************** 
 *                                 常量(本源文件使用)                
 **************************************************************************/

/**************************************************************************
 *                                数据类型(本源文件使用)
 **************************************************************************/

/**************************************************************************
 *                              外部函数原型(评估后慎重添加)
 **************************************************************************/
extern T_psNcuShareMem *g_ptVpfuShMemVar;

/**************************************************************************
 *                              局部函数原型(本源文件使用)
 **************************************************************************/

/**************************************************************************
 *                              全局变量(本源文件使用)
 **************************************************************************/
/* 从psMcsManageJob.c中引用的全局变量 */
extern BYTE g_ncu_sess_dbgflg;
extern WORD32 g_ncu_session_test;
extern WORD32 g_ncu_qasession_test;
extern WORD32 g_ncu_vipsession_test;
extern WORD32 g_ncu_sampsession_test;

/**************************************************************************
 *                              函数实现(函数布局:总分结构|深度优先)
 **************************************************************************/

/**********************************************************************
* 函数名称：  showncusessinfo
* 功能描述：  显示NCU会话信息
* 访问的表：
* 修改的表：
* 输入参数：  无
* 输出参数：  无
* 返 回 值：  无
* 其它说明：
***********************************************************************/
UPF_HELP_REG("ncu",
             "show ncusessinfo",
             void showncusessinfo())
{
     WORD32 dwID        = 0;
    T_NcuSessionStat *ptNcuSessionStat        = NULL;
    T_NcuSessionStat  tNcuSessionStatStat     = {0};
       if(!g_ptVpfuShMemVar) return;

       for(dwID = 0; dwID < MEDIA_THRD_NUM; dwID++)
      {
        ptNcuSessionStat = &(g_ptVpfuShMemVar->tNcuSessionStat[dwID]);

        tNcuSessionStatStat.qwCurrentSessions                    += ptNcuSessionStat->qwCurrentSessions;
        tNcuSessionStatStat.qwQualityAssuranceSessions           += ptNcuSessionStat->qwQualityAssuranceSessions;
      }
    zte_printf_s("\n NCU qwCurrentSessions = %llu, qwQualityAssuranceSessions = %llu\n", tNcuSessionStatStat.qwCurrentSessions ,  tNcuSessionStatStat.qwQualityAssuranceSessions);

    return;
}

/**********************************************************************
* 函数名称：  setncusessinfo
* 功能描述：  设置NCU会话信息用于测试
* 访问的表：
* 修改的表：
* 输入参数：  bNcuSessDbgFlg - 测试标志
*             dwsess - 会话数
*             dwQasess - QA会话数
*             dwVipsess - VIP会话数
*             dwSampsess - 采样会话数
* 输出参数：  无
* 返 回 值：  无
* 其它说明：
***********************************************************************/
UPF_HELP_REG("ncu",
    "set setncusessinfo",
    void setncusessinfo(BYTE bNcuSessDbgFlg, WORD32 dwsess, WORD32 dwQasess, WORD32 dwVipsess, WORD32 dwSampsess))
{
    g_ncu_sess_dbgflg = bNcuSessDbgFlg;
    g_ncu_session_test = dwsess;
    g_ncu_qasession_test = dwQasess;
    g_ncu_vipsession_test = dwVipsess;
    g_ncu_sampsession_test = dwSampsess;

    return;
}

/* Ended by AICoder, pid:ea6d2531f2284fb14132094be05ad95955f6ea64 */ 
BYTE ncuGetCpuUsage();
BOOLEAN ncuGetSingleTypeUsrUsageOccupancy(NCU_TO_UPM_DALOAD_RSP *ptNcuLoadRsp);

UPF_HELP_REG("ncu",
             "show nculoadinfo",
             void shownculoadinfo())
{
    NCU_TO_UPM_DALOAD_RSP tNcuLoadRsp = {0};

    BYTE bCpuRate = ncuGetCpuUsage();
    ncuGetSingleTypeUsrUsageOccupancy(&tNcuLoadRsp);

    zte_printf_s("\n NCU bCpuRate = %u", bCpuRate);
    zte_printf_s("\n NCU dwQaUserNum = %u", tNcuLoadRsp.dwQaUserNum);
    zte_printf_s("\n NCU dwKeyUserNum = %u", tNcuLoadRsp.dwKeyUserNum);
    zte_printf_s("\n NCU dwSampUserNum = %u", tNcuLoadRsp.dwSampUserNum);

    return;
}

extern BYTE g_ncu_load_dbgflg;
extern BYTE g_ncu_cpurate;
extern WORD32 g_ncu_qausrnum;
extern WORD32 g_ncu_keyusrnum;
extern WORD32 g_ncu_sampusrnum;
UPF_HELP_REG("ncu",
             "set setnculoadinfo",
             void setnculoadinfoeach(BYTE bNcuLoadDbgFlg, BYTE bCpuRate, WORD32 bQaUsrNum, WORD32 bKeyUsrNum, WORD32 bSampUsrNum))
{
    g_ncu_load_dbgflg = bNcuLoadDbgFlg;
    g_ncu_cpurate = bCpuRate;
    g_ncu_qausrnum = bQaUsrNum;
    g_ncu_keyusrnum = bKeyUsrNum;
    g_ncu_sampusrnum = bSampUsrNum;

    return;
}

UPF_HELP_REG("ncu",
    "set setnculoadinfo",
    void setnculoadinfo(BYTE bNcuLoadDbgFlg, BYTE bCpuRate, WORD32 bUsrNum))
{
    g_ncu_load_dbgflg = bNcuLoadDbgFlg;
    g_ncu_cpurate = bCpuRate;
    g_ncu_qausrnum = bUsrNum * 480000 / 100;
    g_ncu_keyusrnum = bUsrNum * 480000 / 100;
    g_ncu_sampusrnum = bUsrNum * 960000 / 100;
    return;
}
