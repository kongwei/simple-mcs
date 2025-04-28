
#include "UPFHelp.h"

#include "SCCU_Server/sccu_server.h"
#include "psUpfSCTypes.h"
#include "_db_type.h"
#include "_db_job.h"
#include "_db_job_reg_table.h"
#include "V6PLAT/dbm/dbm.h"
#include "dbmInnerdef.h"
#include "log_agent_pub.h"
#include "das_agent.h"
#include "das_proxy.h"
#include "TMSPAdapter/pzdb_intf_job_pub.h"
#include "HTTP_LB/HTTPInterface.h"
#include "HTTP_LB/httplink/http_link_pub.h"
/*common*/
#include <signal.h>
#include "ps_memsche_comm.h" //for PS_HUGEMEM_FAIL
#include "pos_lib.h"
#include "handshake.h"
#include "appommglobal.h"
#include "ps_css_interface.h"
#include "nff_pub.h"
#include "dpathreadpub.h"
#include "ps_pub.h"
#include "xdb_core_pfu_debug.h"
#include "dpa_rcvsend.h"
#include "kmsclientfunc.h"

/*upf*/
#include "psUpfCommon.h"
#include "psUpfEvent.h"
#include "psUpfJobTypes.h"
#include "_zdb_phy.h"
#include "vppjob.h"
#include "zte_slibc.h"
#include "dpa_compat.h"

/*ncu*/
#include "MemShareCfg.h"
#include "sdb_ncudynlib.h"
#include "dpaThreadCommon.h"
#include "psMcsEntryRegister.h"
#include "ps_gdmm_ncu_entry.h"
#include "SimpleCfgRegister.h"

extern WORD64 psGetCssHugepageSize();
extern VOID psCssSetUpfNcuType();

extern atomic_t_32 dwMediaInitFlag;
#define TULIP_MAIN_PROC_NORT_SCHE

#ifndef SC_NAME_NEAR_COMPUTING
#define SC_NAME_NEAR_COMPUTING     "sc-upf-nc"
#endif
T_TypeDefine g_SelfSCInfo =
{
    SC_NAME_NEAR_COMPUTING
};

#include "thdm_pub.h"

WORD32 psFtmGetShareMem();

BOOL ugmTrieTblMemInit();
BOOL ugmVfwTrieTblMemInit();
BOOL isVfwDeployed();
unsigned eal_cpu_socket_id(unsigned lcore_id);
BYTE getPlatMode();

#define SERVICE_NAME            "NCU" //需要修改为NCU
#define PFU_MEDIA_ENTRY         (TaskEntryProto)psMediaWorkerEntry
#define PFU_RCV_ENTRY           (TaskEntryProto)psNetRcvEntry
#define PFU_RSC_SCAN_ENTRY      (TaskEntryProto)psMediaLoopScanEntry

/*lint -e785*/
T_PSThreadAffinityInfo  gthread_affinity_info_table_Template[] =
{                                                                                                       /*dwSleepMS*/
    {THDM_MEDIA_TYPE_PFU_MEDIA_RECV,    "DispatchThread",   PFU_RCV_ENTRY,          1024*16*8,  44, {0,0,0,0,1,0}},
    {THDM_MEDIA_TYPE_PFU_MEDIA_PROC,    "MediaThread",      PFU_MEDIA_ENTRY,        (2048+512)*16*16,  44, {0,0,0,0,1,0}},
    {THDM_MEDIA_TYPE_PFU_RSC_SCAN  ,    "PFUCSCAN",         PFU_RSC_SCAN_ENTRY,     1024*16*8,  44, {0,0,0,1,10,0}},
};

WORD32  g_dwTemplateThreadTypeNum = sizeof(gthread_affinity_info_table_Template)/sizeof(T_PSThreadAffinityInfo);

extern WORD64 g_hugepage_Size;
BYTE g_bHardwareType;
extern WORD32 psCssGetShareMem();
extern WORD64 psGetAllHugepageSize();
extern uint8_t dpa_set_nff_mode(uint8_t mode);

#define  CREATE_HANDSHAKE_TASK_ERROR_RETURN \
    if(-1 == client_handshake_task_create("arch-sync"))\
    {\
        return (WORD32)(-1);\
    }

BOOLEAN isFpgaSupportNFFForTEP1_1() /*该版本NFF表下发有变化，只有TEP1.1 二阶段使用*/
{
    return FALSE;
}

static inline void psUpfInitC0()
{
    /* NFF回调注册，放在c0֮ǰ, 以下注释为消除编译告警 */
    if(!(isPlatFormTep() && (FALSE == isFpgaSupportNFFForTEP1_1())))
    {
       printf("tep1.0环境nff回调注册暂时移除，ncu不区分tep形态");
    }

    zte_printf_s("pfu mainApp,call dpa_init_c0\n");
    dpa_init_c0();
}

_DB_RET dbs_get_mate_comid(WORD32 selfComId, WORD32 *mateComID)
{
    /* 平台要求n+m SC 此接口mateComID返回0 */
    *mateComID = 0;
    return _DB_SUCCESS;
}

cpu_set_t mask_old;
WORD32 main_thread_affinity_has_set = 0;
#define RSVD_SHARE_CPU_NUM_MAX 64
#define PFU_CHECK_ZTE_STRTOK_S(PTR1) if(NULL == PTR1 || (((VOID *)-1) == PTR1))

void main_thread_affinity_set()
{
    if(BM_CAAS != getPlatMode())
    {
        zte_printf_s("not BM_CAAS, need not set thread affinity\n");
        return;
    }

    uint32_t first_exclusive_cpu = 0;
    uint32_t socket_id = 0;
    char *end = NULL;
    CHAR tmpchar[128] = {0};
    char *exclusive_cpus_list = NULL;
    char *rsvdshare_cpus_list = NULL;
    SIZE_T strmax = (SIZE_T)(sizeof(tmpchar)-1);

    exclusive_cpus_list = getenv("EXCLUSIVE_CPUS_LIST");
    if(NULL == exclusive_cpus_list)
    {
        zte_printf_s("getenv EXCLUSIVE_CPUS_LIST fail!\n");
        return;
    }
    
    zte_printf_s("main_thread_affinity_set: EXCLUSIVE_CPUS_LIST=%s\n", exclusive_cpus_list);
    first_exclusive_cpu = (uint32_t)strtol(exclusive_cpus_list, &end, 10);
    socket_id = eal_cpu_socket_id(first_exclusive_cpu);
    zte_printf_s("main_thread_affinity_set: first exclusive cpu %u, NUMA %u\n", first_exclusive_cpu, socket_id);

    rsvdshare_cpus_list = getenv("RSVDSHARE_CPUS_LIST");
    if(NULL == rsvdshare_cpus_list)
    {
        zte_printf_s("getenv RSVDSHARE_CPUS_LIST fail!\n");
        return;
    }

    zte_printf_s("main_thread_affinity_set: RSVDSHARE_CPUS_LIST=%s\n", rsvdshare_cpus_list);
    if(strlen(rsvdshare_cpus_list) > 127 )
    {
        zte_printf_s("strlen(rsvdshare_cpus_list) > 127\n");
        return;
    }
    memcpy(tmpchar, rsvdshare_cpus_list, strlen(rsvdshare_cpus_list));
    tmpchar[strlen(rsvdshare_cpus_list)] = 0;

    BYTE  bIndex = 0;
    int sharecpus[RSVD_SHARE_CPU_NUM_MAX] = {0};

    CHAR *argv;
    CHAR *savestr = NULL;
    argv = zte_strtok_s(tmpchar, &strmax, ",", &savestr);
    if(NULL == argv || (((VOID *)-1) == argv))
    {
        zte_printf_s("fun %s line %u: get zte_strtok_s is null\n", __FUNCTION__, __LINE__);
        return;
    }

    sharecpus[bIndex] = atoi(argv);
    zte_printf_s("main_thread_affinity_set: bIndex %u, CPU %s\n", bIndex, argv);
    bIndex++;

    while (argv) 
    {
        if(bIndex >= RSVD_SHARE_CPU_NUM_MAX)
        {
            break;
        }
        
        argv = zte_strtok_s(NULL, &strmax, ",", &savestr);
        if(NULL == argv || (((VOID *)-1) == argv))
        {
            sharecpus[bIndex] = 0;
            break;
        }

        sharecpus[bIndex] = atoi(argv);
        zte_printf_s("main_thread_affinity_set: bIndex %u, CPU %s\n", bIndex, argv);

        bIndex++;
    }

    CPU_ZERO(&mask_old);
    if(sched_getaffinity(0, sizeof(mask_old), &mask_old) == -1)
    {
        zte_printf_s("main_thread_affinity_set: sched_getaffinity failed!\n");
        return;
    }

    cpu_set_t mask;
    int i;
    int cpu_count = 0;
    
    CPU_ZERO(&mask);
    for (i = 0; i < bIndex; i++)
    {
        if(eal_cpu_socket_id(sharecpus[i]) == socket_id)
        {
            CPU_SET(sharecpus[i], &mask);
            cpu_count++;
            zte_printf_s("main_thread_affinity_set: CPU_SET %u, NUMA %u\n", sharecpus[i], socket_id);
        }
    }

    if(cpu_count == 0)
    {
        return;
    }

    if(sched_setaffinity(0, sizeof(mask), &mask) == -1)
    {
        zte_printf_s("main_thread_affinity_set: sched_setaffinity failed!\n");
        return;
    }

    main_thread_affinity_has_set = 1;
    zte_printf_s("main_thread_affinity_set ok!\n");
}

void main_thread_affinity_restore()
{
    if(main_thread_affinity_has_set == 0)
    {
        zte_printf_s("main_thread_affinity_has_set == 0\n");
        return;
    }

    if(sched_setaffinity(0, sizeof(mask_old), &mask_old) == -1)
    {
        zte_printf_s("main_thread_affinity_restore: sched_setaffinity failed!\n");
        return;
    }

    zte_printf_s("main_thread_affinity_restore ok!\n");
}
extern BYTE g_bFwdType;
WORD32 AppMainInit(VOID)
{
    SimpleCfgAddAllCfg();
    printf("-----------------------------Create pthread of handshake task-------------------\n");

    CREATE_HANDSHAKE_TASK_ERROR_RETURN

    wait_handshake_stat_symbol(HANDSHAKE_STAT_POWN_ON_OK);

    printf("init subMain! func %s line %d\n", __FUNCTION__, __LINE__);

    psCssSetUpfNcuType();
    printf("psCssSetUpfNcuType:g_bFwdType=%u\n",g_bFwdType);
    atomic_set(&dwMediaInitFlag, 0);
    XOS_SetCriticalProc();
    cfmt_AddProcDbgFunc();
    XOS_SetRawOut(TRUE);
    printf("XOS_SetRawOut success! func %s line %d\n", __FUNCTION__, __LINE__);
    /* 注册UPF的DbName表 */
    psCommCfgRegScDbNameMap("sc-upf-nc", "UpfNcu");


    WORD64 dpdkmem = dpaGetDpdkmemory();
    printf("begin to get dpdkmemory env func %s line %d: dpdkmem %llu\n", __FUNCTION__, __LINE__,dpdkmem);
    if(0 == dpdkmem )
	{
        sleep(3);
        XOS_SysRestart(XOS_SYS_RESTART_NORMAL,"hugememsizemb not exist!\n");
        return (WORD32)(-1);
	}
    _xdb_pfu_help();
    threadinfo();
    WORD64 allmem = psGetAllHugepageSize();
    printf("func %s line %d: all mem 0x%llx\n", __FUNCTION__, __LINE__,allmem);
    if(allmem <= dpdkmem)
    {
        printf("func %s line %d: all %llu (MB),dpdkmem %llu (MB)\n", __FUNCTION__, __LINE__,allmem,dpdkmem);
        return (WORD32 )(-2);
    }
    g_hugepage_Size=(allmem-dpdkmem)*1024*1024UL;

    printf("func %s line %d: huge mem 0x%llx\n", __FUNCTION__, __LINE__,g_hugepage_Size);

    main_thread_affinity_set();
    
    /* 【注意】写锁之后，如果在释放锁之前有return或者中间调用的函数里面有调用XOS_SysRestart(不建议)等，
    需要调用释放接口先释放文件锁，其他流程不要放在锁里面                                   */
    dpa_init_hp_lock();
    if(getenv("LIB_COMBINED") != NULL)
    {
        dpa_set_nff_mode(1); /* support nff multi instance */
        psIPSecDPDKEarlyInit(dpdkmem);//MB, C0 need  hugepages  /* 函数里面有调用XOS_SysRestart，但已调用dpa_init_hp_unlock()释放文件锁                         */
        printf("psCSSDPDKEarlyInit(in combined) %llu hugepage success! func %s line %d\n", dpdkmem, __FUNCTION__, __LINE__);
    }
    else
    {
        psIPSecDPDKEarlyInit(dpdkmem);
        printf("psCSSDPDKEarlyInit %llu hugepage success! func %s line %d\n", dpdkmem, __FUNCTION__, __LINE__);
    }

    if (POS_SUCCESS != HugePageMemInit(g_hugepage_Size))
    {
        printf("HugePageMemInit fail! func %s line %d\n", __FUNCTION__, __LINE__);
        dpa_init_hp_unlock();
        sleep(30);
        XOS_SysRestart(XOS_SYS_RESTART_NORMAL,"HugePageMemInit fail\n");
        return (WORD32)(-3);
    }
    printf("HugePageMemInit success! func %s line %d ,g_hugepage_Size %llu\n", __FUNCTION__, __LINE__,g_hugepage_Size);

    /* 【注意】如果在释放锁之前有return或者中间调用的函数里面有调用XOS_SysRestart(不建议)等，
    需要调用释放接口先释放文件锁，其他流程不要放在锁里面                                      */
    dpa_init_hp_unlock();

    main_thread_affinity_restore();
    
    g_bHardwareType = getRunHardWareType();
    psUpfInitC0();
    printf("AppMainInit! func %s line %d:after init c0\n", __FUNCTION__, __LINE__);
    Send_mmap_begin_Msg();
    wait_handshake_stat_symbol(HANDSHAKE_STAT_WORK);
    printf("AppMainInit! func %s line %d:after wait_handshake_stat_symbol\n", __FUNCTION__, __LINE__);
    if(dpaGetThreadModuleMem())
    {
        printf("AppMainInit! func %s line %d,dpaGetThreadModuleMem return fail \n", __FUNCTION__, __LINE__);
        return (-2);
    }

    printf("AppMainInit! func %s line %d\n", __FUNCTION__, __LINE__);

    if (PS_HUGEMEM_FAIL == psFtmGetShareMem())
    {
        printf("psFtmGetShareMem fail! func %s line %d\n", __FUNCTION__, __LINE__);
        return PS_HUGEMEM_FAIL;
    }

    if(POS_SUCCESS != psGWUGetShareMem())
    {
        printf("psCssGetShareMem fail! func %s line %d\n", __FUNCTION__, __LINE__);
        return (WORD32)(-3);
    }

    printf("AppMainInit! func %s line %d\n", __FUNCTION__, __LINE__);

    if (POS_SUCCESS != psCssGetShareMem())
    {
        printf("psCssGetShareMem fail! func %s line %d\n", __FUNCTION__, __LINE__);
        return (WORD32)(-4);
    }
    printf("AppMainInit! func %s line %d\n", __FUNCTION__, __LINE__);

    if (POS_SUCCESS!=Pos_UbInit())
    {
        printf("Pos_UbInit fail! func %s line %d\n", __FUNCTION__, __LINE__);
        return (WORD32)(-5);
    }
    printf("AppMainInit! func %s line %d\n", __FUNCTION__, __LINE__);

    WORD64 dwUBSize = HugePageMemGetSpareSize();
    printf("AppMainInit! func %s line %d,get commUbSize %llu\n", __FUNCTION__, __LINE__,dwUBSize);
    if (POS_SUCCESS!=ps_comm_UBInit(dwUBSize))
    {
        printf("ps_comm_UBInit fail! func %s line %d\n", __FUNCTION__, __LINE__);
        return (WORD32)(-6);
    }
    printf("AppMainInit! func %s line %d\n", __FUNCTION__, __LINE__);

    psIpsGetEnvCollection();
    if(getenv("LIB_COMBINED") == NULL)  /* not C0 combined, NFF is single instance */
    {
        if(sccu_InitNFF(SCCU_CLIENT_ROLE) < 0)
        {
            printf("init AppMainInit sccu_InitNFF fail!\n");
            return (WORD32)(-1);
        }
    }

    psThreadInfoReg(SERVICE_NAME, g_dwTemplateThreadTypeNum, gthread_affinity_info_table_Template);
    psCreateDynLibReg(create_ncudynlibs);

    lpDBGetAppMateComID = dbs_get_mate_comid;
    /*EC:614008632015 【可靠性测试】    在重启UPM虚机的时候，PFU发消息给UPM会失败，消息发送失败，平台会有printf打印，
    打印过多，会降低PFU CPU性能，导致收到IPM的地址申请应答超时，而导致呼损*/
    XOS_SetTipcLibPrintFlag(FALSE);

    printf("init subMain success! func %s line %d\n", __FUNCTION__, __LINE__);

    return 0;
}

/*lint -e762*/
    VOID Sccu_ClientInitJobEntry(WORD16 wState, WORD32 dwMsgId, VOID *pMsgBody, VOID *pPData, BOOLEAN bSame);
    VOID Sccu_ClientJobEntry(WORD16 wState, WORD32 dwMsgId, VOID *pMsgBody, VOID *pPData, BOOLEAN bSame);
    VOID psVpfuMcsManageJobEntry(WORD16 wState, WORD32 dwMsgId, VOID *pMsgBody, VOID *pPData, BOOLEAN bSame);
    VOID psThdmConfigEntry(WORD16 State, WORD32 MsgId, VOID *pMsgBody, VOID *pPData, BOOLEAN IsSame);
    VOID psThdmTaskEntry(WORD16 State, WORD32 MsgId, VOID *pMsgBody, VOID *pPData, BOOLEAN IsSame);
    VOID psFtmEntry(WORD16 wState, WORD32 dwMsgId, VOID *pMsgBody, VOID *pPData, BOOLEAN bSame);
    VOID CSS_Entry(WORD16 wState, WORD32 dwMsgId, VOID *pMsgBody, VOID *pPData, BOOLEAN bSame);
    VOID HicutsBuildEntry(WORD16 wState, WORD32 dwMsgId, VOID *pMsgBody, VOID *pPData, BOOLEAN bSame);
    VOID PmAgtMgr(WORD16 wState , WORD32 wEvent, LPVOID pInMsgPara, LPVOID p, BOOLEAN b);
    VOID TT_PmAgtMgr(WORD16 wState , WORD32 wEvent, LPVOID pInMsgPara, LPVOID p, BOOLEAN b);
    VOID psVpfuUdrManageJobEntry(WORD16 wState, WORD32 dwMsgId, VOID *pMsgBody, VOID *pPData, BOOLEAN bSame);
    VOID psUpfGdmmEntry(WORD16 wState, WORD32 dwMsgId, VOID *pMsgBody, VOID *pPData, BOOLEAN bSame);
    VOID FmAgtApp(WORD16  wState, WORD32  dwMsgId, VOID * pMsgPara, VOID *pvPrvData, BOOLEAN bIsSameEndian);
    VOID TT_FmAgtApp(WORD16  wState, WORD32  dwMsgId, VOID * pMsgPara, VOID *pvPrvData, BOOLEAN bIsSameEndian);
    VOID ugmClientJobEntry(WORD16 wState, WORD32 dwMsgId, VOID *pMsgBody, VOID *pPData, BOOLEAN bSame);
    
    VOID psUpfPfuCollectEntry(WORD16 wState, WORD32 dwMsgId, VOID *pMsgBody, VOID *pPData, BOOLEAN bSame);
    VOID PS_COMM_CFG_Entry(WORD16 wState, WORD32 dwMsgId, VOID *pMsgBody, VOID *pPData, BOOLEAN bSame);
    VOID CommResourceAlarmEntry(WORD16 wState, WORD32 dwMsgId, VOID *pMsgBody, VOID *pPData, BOOLEAN bSame);
    VOID psUpfLogManageJobEntry(WORD16 wState, WORD32 dwMsgId, VOID *ptMsgBody, VOID *ptPData, BOOLEAN IsSame);
    VOID dasAgentJobEntry(WORD16 state, WORD32 dwMsgId, VOID *data, VOID *param, BOOLEAN isSame);
    
    VOID psVncuGetLicenseCheckJobEntry(WORD16 wState,
                         WORD32 dwMsgId,
                         VOID *pMsgBody,
                         VOID *pPData,
                         BOOLEAN bSame);
/*lint +e762*/
#define CONFIGAGENT_REG_TYBLE_MAIN_EX(Pri, SchId)  {JOB_TYPE_DBM,      (Pri),  (SchId),    1,  "DBM_Entry",    &DbmJobEntry,       4096*128,   4096*10,   1,  1,  XOS_L1_SCHE | JOB_NEED_POWERON_ACK | JOB_NEED_S2M_CHGOVER | JOB_NEED_M2S_CHGOVER | JOB_NEED_POWEROFF_ACK, 30*1000, 1800*1000, 0, 0, 0, 0}
#define DMMCLIENT_REG_TABLE_MAIN_EX(Pri, SchId)    {JOB_TYPE_DMMCLIENT,(Pri),  (SchId),    1,  "DMM_Client",   &DmmClientJobEntry, 4096*64,   4096*15,   1,  1,  XOS_L1_SCHE | JOB_NEED_POWERON_ACK | JOB_NEED_S2M_CHGOVER | JOB_NEED_M2S_CHGOVER | JOB_NEED_NO_RT_SCHE, 30*1000, 30*1000, 0, 0, 0, 0}

T_ossJobCreatReg g_atJobRegTbl[] =
{
    /*   wJobType                       wPri    wSchIndex   acName          PEntry                  DataSize    */
    ZXDB_JOB_REG_TABLE(                 14,                 11, _DB_L1_SCHE),
    JOBREG_ZDB_PHY(14,31),
    JOBREG_CCSP_ADAPTER(14,45),
    CONFIGAGENT_REG_TYBLE_MAIN_EX(         14,     14),
    KMSCLIENT_REG_MAIN_NO_RT(14, 46),
    DMMCLIENT_REG_TABLE_MAIN_EX(           14,    13),

    JOB_CSS_COMM_CFG(                   14,     33),
    {JOB_TYPE_MCS_GET_LICENSE,           14,     41, 1,      "LicenseGetCheck",     &psVncuGetLicenseCheckJobEntry,              64*4096, 10*4096, 1, 1, XOS_L1_SCHE|JOB_NEED_POWERON_ACK|JOB_NEED_NO_RT_SCHE, 120*1000, 120*1000, 300*1000, 30*1000, 30*1000, 30*1000},
    UPF_JOBREG_NONREALTIME(JOB_TYPE_THDM_CONFIG,    14,     15,         "THDM_Config",  &psThdmConfigEntry,     10*4096),
	JOBREG_SCCU_CLIENT(                 19,     12),
    HTTPLINK_REG_TABLE_MAIN_NO_RT(14, 40),
    {JOBTYPE_AGENT_PM,                  14,     16, 1,      "PmAgtMgr",     &PmAgtMgr,              256*4096, 64*4096, 1, 1, XOS_L1_SCHE|JOB_NEED_NO_RT_SCHE, 120*1000, 120*1000, 300*1000, 30*1000, 30*1000, 30*1000},
	{JOBTYPE_AGENT_APP_FM,              14,     17, 1,      "AlarmAgtMgr",  &FmAgtApp,              64*4096, 64*4096, 1, 1, XOS_L1_SCHE|JOB_NEED_NO_RT_SCHE, 120*1000, 120*1000, 300*1000, 30*1000, 30*1000, 30*1000},
    
    JOBREG_DAS_AGENT(                 14,     42),
    JOBREG_DAS_PROXY(JOB_TYPE_DAS_AGENT_PROXY_NCU,    14,       47),

    UPF_JOBREG_NONREALTIME(JOB_TYPE_MCS_MANAGE,     14,     18,         "NcuMcsManage",&psVpfuMcsManageJobEntry,128*4096),
    UPF_JOBREG_NONREALTIME_WITH_POWEROFF(JOB_TYPE_THDM_TASK,      14,     20,         "THDM_Task",    &psThdmTaskEntry,       10*4096),
    LOG_AGENT_MAIN(14, 36),
    UPF_JOBREG_NONREALTIME(JOB_TYPE_UPF_LOG_MANAGE,   14,     39,      "UpfLogManage",&psUpfLogManageJobEntry, 10*4096),
    UPF_JOBREG_NONREALTIME(JOB_TYPE_CSS_CTRL,       14,     21,         "CSS_ENTRY",    &CSS_Entry,             64*4096),
    UPF_JOBREG_NONREALTIME(JOB_TYPE_CSS_FTM,        14,     23,         "FTM",          &psFtmEntry,            10*4096),
    UPF_JOBREG_NONREALTIME(JOB_TYPE_HICUTS_BULID,   14,     24,         "HicutsBuild",  &HicutsBuildEntry,      64*4096),
    UPF_JOBREG_NONREALTIME(JOB_TYPE_UPF_GDMM,       14,     26,         "UPF_GDMM",     &psUpfGdmmEntry,        128*4096),
    
    JOB_COMM_RESOURCE_ALARM(            14,     38),
    
};

WORD32 g_dwNumOfAppJob = sizeof(g_atJobRegTbl) / sizeof (T_ossJobCreatReg);



/**************************************************************************
*                            宏                                           *
**************************************************************************/
/* 设置本进程Tulip定时器频率/HZ（只能定义为10或者100） 
 * 如果不定义这个宏值，则缺省为10，即定时器精度为100ms */
#define TULIP_CLOCK_RATE 100
/* 使能异常日志记录完整功能 */
#define TULIP_THREAD_ALONE_SIGSTACK_ENABLE
/* 使能文件模块功能 */
#define TULIP_FILE_MODULE_ENABLE
/* 使能内存模块功能 */
#define TULIP_MEM_MODULE_ENABLE
/* 用户共享交换块大小 Ϊ4K的倍数,最大64K */
#define TULIP_MEM_USER_SBLOCK_SIZE     4096
/* 用户初始共享交换块个数 */
#define TULIP_MEM_USER_SBLOCK_NUM      400
/* 动态内存申请阈值,单位为K,就是这个值，默认为0即不打开功能*/
/* 使能符号表模块功能 */
#define TULIP_SYMTAB_MODULE_ENABLE
/* 使能重定向模块功能 */
#define TULIP_IORDR_MODULE_ENABLE
/* 使能定时器模块功能 */
#define TULIP_TIMER_MODULE_ENABLE
/* 使能调度通信模块功能 */
#define TULIP_SCHE_COMM_MODULE_ENABLE
/* 使能SysLog模块功能 */
#define TULIP_SYSLOG_MODULE_ENABLE


typedef WORD32 (*APPINITFUNCPTR)(VOID);
APPINITFUNCPTR   g_pAppMainInit = AppMainInit;

#ifndef FT_TEST
#include "tulip_main.c"
#endif
