/* Started by AICoder, pid:ea6d2531f2284fb14132094be05ad95955f6ea64 */
/******************************************************************************
 * 版权所有 (C)2016 深圳市中兴通讯股份有限公司*
 * 模块名          : MCS
 * 文件名          : psMcsManageJob.c
 * 相关文件        :
 * 文件实现功能     : NCU JOB流程
 * 归属团队        : 
 * 版本           : V7.24.20
-------------------------------------------------------------------------------
* 修改记录:
* 修改日期            版本号           修改人           修改内容
* 2024-04-25        V7.24.20        m6               modify
******************************************************************************/
/**************************************************************************
 *                              头文件(满足最小依赖请按照DDD分层架构逐层依赖)
 **************************************************************************/
// application    层依赖
// 非DDD目录依赖
#include "psMcsManageJob.h"
#include "tulip_appcore.h"
#include "ps_define_jobtype.h"
#include "tulip_event.h"
//#include "r_upf_usergroup.h" //upfGetConfigMaxGroupID
#include "rte_mempool.h" //CSS_CTRL_THREAD_NO
#include "_zdb_phy.h"  //ltm_table_notify_req

#include "MemShareCfg.h"
#include "psMcsGlobalCfg.h"
#include "SimpleCfgFrameWork.h"
#include "psUpfJobTypes.h"
#include "dbmLibComm.h"
#include "psNcuCtrlStatInterface.h"
#include "vnfp_event_seg.h"
#include "sccu_pub_define.h"
#include "ncuUserGroup.h"
#include "psNcuDbgCollReg.h"
#include "pmm_api.h"
#include "psUpfSessionStruct.h"
#include "ps_ncu_typedef.h"
#include "ps_db_define_pfu.h"
#include "ps_db_define_ncu.h"
#include "psNcuCtxFunc.h"
#include "dpathreadpub.h"
#include "psUpfEvent.h"
#include "ncuSCCUAbility.h"
#include "zte_slibc.h"
#include "psNcuMcsPmReport.h"
#include "psNcuReportStructData.h"
#include "psNcuHeartBeatProc.h"
#include "psNcuScanMsgProc.h"
#include "ncuMcsAlarm.h"
#include "ncuDangerOpAlarm.h"
#include "psNcuNWDAFAddrProc.h"
#include "psNefReportToNwdaf.h"
#include "psNcuDataEncode.h"
#include "psMcsDebug.h"
#include "ps_define_jobtype.h"
#include "das_agent.h"
// #include "ldb_default_fun.h"
#include "psNcuHttpLinkProc.h"
#include "ncuUserGroup.h"
#include "psNcuSCCUSCListChgNotify.h"
#include "pfuUserGroup.h"
#include "httpLbScInfo.h"
#include "psUpfPubSub.h"

/************************************************************************** \
 *                                  宏(本源文件使用)                 \
 **************************************************************************/
#define CASE_USERGROUP_MIGRATE_MSG                                          \
    case EV_USRGROUP_SCTOXJOB_MIGIN_PRE_REQ:                                \
    case EV_USRGROUP_SCTOXJOB_MIGIN_DO_REQ:                                 \
    case EV_USRGROUP_SCTOXJOB_MIGIN_POST_REQ:                               \
    case EV_USRGROUP_SCTOXJOB_MIGOUT_PRE_REQ:                               \
    case EV_USRGROUP_SCTOXJOB_MIGOUT_POST_REQ:                              \
    case EV_USRGROUP_SCTOXJOB_MIGOUT_STAT_NOTIFY:                           \
    case EV_USRGROUP_SCTOXJOB_MIGIN_STAT_NOTIFY                             \

#ifndef S_POWER_OFF
#define S_POWER_OFF (WORD16)(S_START_UP+4)
#endif

#define  NCU_NOT_RCV_UPM_SUB_AND_TIMER_HANDLE    0
#define  NCU_RCV_UPM_SUB                         1
#define  NCU_RCV_UPM_TIMER_HANDLE                2
/************************************************************************** \
 *                                 常量(本源文件使用)               \
 **************************************************************************/
BYTE g_ncu_load_dbgflg = 0;
BYTE g_ncu_cpurate = 0;
WORD32 g_ncu_qausrnum = 0;
WORD32 g_ncu_keyusrnum = 0;
WORD32 g_ncu_sampusrnum = 0;

BYTE g_ncu_sess_dbgflg = 0;
WORD32 g_ncu_session_test = 0;
WORD32 g_ncu_qasession_test = 0;
WORD32 g_ncu_vipsession_test = 0;
WORD32 g_ncu_sampsession_test = 0;
WORD64 g_w64McsMaxCpuLoad = 0;
BYTE g_ncu_rcvupmsub = NCU_NOT_RCV_UPM_SUB_AND_TIMER_HANDLE;
/**************************************************************************
 *                                数据类型(本源文件使用)
 **************************************************************************/

/**************************************************************************
 *                              外部函数原型(评估后慎重添加)
 **************************************************************************/
extern T_UpfSigTraceCfg g_sigtracecfg;
/**************************************************************************
 *                              局部函数原型(本源文件使用)
 **************************************************************************/
VOID psVpfuMcsManageJobStartupProc(WORD32 dwMsgId, VOID *pMsgBody, VOID *pPData);
VOID psVpfuMcsManageJobMasterProc(WORD32 dwMsgId, VOID *pMsgBody, VOID *pPData, WORD16 wMsgLen);
bool RecvConfigNotifyReq(BYTE *pbMsgBody);

BYTE ncuGetUserOccupancy(DWORD dwDataType);
BYTE ncuGetCpuUsage();
WORD64 ncuGetMaxCpuload();
WORD32 ncuHandleLoadStatusProc(UPM_TO_NCU_DALOAD_REQ *ptNcuLoadReq);
VOID psVpfuTraceManage();
VOID psNcuHandleSubCoreSyncReq(VOID* pMsgBody);
WORD32 ncuHandleSessLoadProc(UPM_TO_NCU_SESSION_REQ *ptNcuLoadReq);
VOID ncuSendToUpmSynSubCoreReqProc();
BOOLEAN ncuGetSingleTypeUsrUsageOccupancy(NCU_TO_UPM_DALOAD_RSP *ptNcuLoadRsp);
VOID psNcuSetLinkCheckTimer(VOID *pPData);
VOID psNcuUpdateLinkCheckTimer(VOID *pPData);
void psNcuSendThrdNumToPfu(PFU_TO_NCU_ECHO_REQ * ptPfuEchoReq);
/**************************************************************************
 *                              全局变量(本源文件使用)
 **************************************************************************/
extern T_NwdafStatus g_nwdaflinkstatus[NWDAF_LINKNUM_MAX];
/**************************************************************************
 *                              函数实现(函数布局:总分结构|深度优先)
 **************************************************************************/

/**********************************************************************
* 函数名称：  psVpfuMcsManageJobEntry
* 功能描述：  媒体面管理Job入口函数
* 访问的表：
* 修改的表：
* 输入参数：
        wState       JOB״̬
        dwMsgId      消息ID
        pMsgBody     消息体指针
        pPData       私有数据区指针
        bSame        是否相同字节序
* 输出参数：
* 返 回 值：  无
* 其它说明：
***********************************************************************/
VOID psVpfuMcsManageJobEntry(WORD16 wState, WORD32 dwMsgId, VOID *pMsgBody, VOID *pPData, BOOLEAN bSame)
{
    JID    tJid = {0};
    WORD16 wMsgLen=0;

    /* 判断入参是否有效 */
    if(NULL == pMsgBody)
    {
        return;
    }

    /* 判断入参是否有效 */
    if(NULL == pPData)
    {
        return;
    }

    /* 获取当前处理的消息的发送者 */
    if(XOS_SUCCESS != XOS_Sender(&tJid))
    {
        XOS_SysLog(LOG_NOTICE,"XOS_Sender return fail\n");
    }

    XOS_GetMsgDataLen(&wMsgLen);
    switch(wState)
    {
        /* 初始业务状态 */
        case S_StartUp:
        {
            psVpfuMcsManageJobStartupProc(dwMsgId, pMsgBody, pPData);
            break;
        }
        /* 主用状态 */
        case S_Work:
        {
            psVpfuMcsManageJobMasterProc(dwMsgId, pMsgBody, pPData,wMsgLen);
            XOS_SetDefaultNextState();
            break;
        }
        default:
        {
            XOS_SetDefaultNextState();
            break;
        }
    }

    return;
}


VOID psVpfuMcsManageJobStartupProc(WORD32 dwMsgId,VOID *pMsgBody, VOID *pPData)
{
    if(NULL == pMsgBody || NULL == pPData)
    {
        return;
    }

    T_psManageJobPriData *ptManageJobPritData = (T_psManageJobPriData *)pPData;

    switch(dwMsgId)
    {
        case EV_MASTER_POWER_ON:
        case EV_TIMER_TRIGGER_MAXGROUPID:
        {
            zte_printf_s("[McsManageJob]Receive EV_MASTER_POWER_ON message!!! \n");

            RTE_PER_LCORE(_lcore_id) = CSS_CTRL_THREAD_NO;

            /* 先检查私有数据区空间, 并初始化私有数据区 */
            XOS_TestVarSpace( sizeof(T_psManageJobPriData) );
            memset(ptManageJobPritData, 0, sizeof(T_psManageJobPriData));

            /*********** 主用上电其他处理 ************/
            psVpfuMcsShMemInit();

            /*日志存储初始化，创建目录*/

            /*NCU表容量告警注册*/
            psdbs_ncu_capp_tbl_alm_reg();

            ptManageJobPritData->dwPFURegisterTimerId = XOS_SetLoopTimer(TIMER_NO_14,5000,PARAM_NULL);
            if(INVALID_TIMER_ID == ptManageJobPritData->dwPFURegisterTimerId)
            {
                XOS_SysLog(LOG_ALERT,"[McsManageJob] TIMER_MANAGEJOB_PFURegister Apply Failed!! file:%s, line:%d\n",__FILE__, __LINE__);
            }
            
            XOS_SetLoopTimer(TIMER_NO_15,60000,PARAM_NULL);
            XOS_SetLoopTimer(TIMER_NO_16,1000,PARAM_NULL);
            XOS_SetLoopTimer(TIMER_NO_18,5000,PARAM_NULL);
            XOS_SetLoopTimer(TIMER_NO_20,100,PARAM_NULL);
            ptManageJobPritData->dwRewriteTimerId = XOS_SetLoopTimer(TIMER_NO_21,2000,PARAM_NULL);
            XOS_SetLoopTimer(TIMER_NO_23,10000,PARAM_NULL);
            ptManageJobPritData->dwRewriteV6TimerId = XOS_SetLoopTimer(TIMER_NO_24,2000,PARAM_NULL);

            SimpleCfgRegCfgAgentNotify(JOB_TYPE_MCS_MANAGE);
            SimpleCfgGetConfig(JOB_TYPE_MCS_MANAGE); //获取默认配置

            WORD32 dwLTMRet = ltm_table_notify_req(1);/*LTM 所有表配置变更，其中包括SCGroup表*/
            zte_printf_s("ltm_table_notify_req  dwLTMRet = %u\n",dwLTMRet);

            PmAppRegistFunc();



            //在license和配置上电读取完毕后返回dpa回调值

            XOS_SetLoopTimer(EV_TIMER_29, 5000,PARAM_NULL);

            psNcuSetLinkCheckTimer(pPData);

            upf_ncu_collect_dbg_info_reg();

            /* 上电读取一次,后续直接用全局变量值,不需要配置变更 */

            /* 初始化 JOB私有数据区 */
            ptManageJobPritData->dwUpdateSNTimerId = (DWORD)INVALID_TIMER_ID; 
            ptManageJobPritData->bUpdateSNThrdNum = 0; 
            ptManageJobPritData->dwUpdateSNAgain = MCS_MANAGEJOB_UPDATE_SN_NOAGAIN; 

            //1s定时同步NCU上的Nwdaf IP list
            XOS_SetLoopTimer(TIMER_NCU_TO_NCU_SYN_NWDAFIPLIST, 1000,PARAM_NULL);

            zte_printf_s("VpfuMcsManageJob power on success\n");
            XOS_PowerOnAck(EV_POWER_ON_SUCCESS, NULL, 0, 0);
            JOB_LOC_STAT_ADDONE(qwEV_POWER_ON_SUCCESS);

            ncuSCCUInitSCInfo();
            ncuSendToUpmSynSubCoreReqProc();

            regPfuUserGroupChg();
            psRegHttpLbScInfoChg();
            dasSetProxyCount(1);
            DasIsInTipcOverDPDKMode();/*业务告知DasAgent当前SC处于tipc over dpdk模式*/

            /* 状态迁移  */
            XOS_SetNextState(S_Work);
            break;
        }
        case EV_POWER_OFF:
        {
            JOB_LOC_STAT_ADDONE(qwEV_POWER_OFF);
            zte_printf_s("[McsManageJob]Receive EV_POWER_OFF!!! file:%s, line:%d  \n", __FILE__, __LINE__);
            break;
        }
        default:
        {
            zte_printf_s("[McsManageJob]unknown messages 0x%08x!!! file:%s, line:%d \n", dwMsgId, __FILE__, __LINE__);
            JOB_LOC_STAT_ADDONE(qwStartup_UnknownMsg);
            /* 状态迁移 */
            XOS_SetDefaultNextState();
            break;
        }
    }

    return;
}
WORD32 g_show_job_msgid=0;
VOID psVpfuMcsManageJobMasterProc(WORD32 dwMsgId, VOID *pMsgBodySrc, VOID *pPData,WORD16 wMsgLen)
{
    if(NULL == pPData || NULL == pMsgBodySrc)
    {
        return;
    } 
    if(0 == g_upfConfig.ncuLicenseCfg.hasGetLicCfg)
    {
      psMcsGetNcuLicCfg();
    }


    switch (dwMsgId)
    {
        CASE_USERGROUP_MIGRATE_MSG:
        {
            ncuHandleUsrGroupMigrateProc(dwMsgId, (const BYTE*)pMsgBodySrc, wMsgLen);
            break;
        }
        case EV_CFGCHG_TUPLE_NOTIFY_DBS_TO_APP_REQ:
        {
            bool bRet = false;
            bRet = RecvConfigNotifyReq((BYTE*)pMsgBodySrc);
            psNcuUpdateLinkCheckTimer(pPData);
            break;
        }
        case EV_MSG_UPM_TO_NCU_DALOAD_REQ:
        {
            ncuHandleLoadStatusProc((UPM_TO_NCU_DALOAD_REQ *)pMsgBodySrc);
            break;
        }
        case EV_TIMER_18:
        {
            g_w64McsMaxCpuLoad = ncuGetMaxCpuload();
            psNcuMcsPmmCfgMeasure();
            psVpfuTraceManage();
            psdbs_ncu_capp_report_tblcap_alarm();
            break;
        }
        case EV_TIMER_16:
        {
            DanagerousOpAlarmProc();
            PsNcuSystemLogTest();   //删除 zjw
            break;
        }
        case EV_TIMER_NCU_TO_NCU_SYN_NWDAFIPLIST:
        {
            JOB_LOC_STAT_ADDONE(qwRcvTimerNcu2NcuSynNwdafIpList);
            psNcuSendNwdafIplistSyntoNcu();
            psNcuUpdateNWDAFAddrLoad(g_nwdaflinkstatus); // 临时解决方案 lihao
            break;
        }
        case EV_MSG_NCU_TO_NCU_NWDAFIPLIST_SYNC_REQ:
        {
            //根据消息里的NWDAF的IPList更新全局变量
            //0号群不处理此消息
            JOB_LOC_STAT_ADDONE(qwRcvNcu2NcuNwdafIpListSynReq);
            psNcuUpdateNwdafIplistProc(pMsgBodySrc);
            psNcuUpdateNWDAFAddrLoad(g_nwdaflinkstatus);
            break;
        }
        case EV_MSG_UPM_TO_NCU_SUB:
        {
            //只有0号群处理订阅消息，触发心跳请求
            JOB_LOC_STAT_ADDONE(qwRcvUpm2NcuSub);
            g_ncu_rcvupmsub = NCU_RCV_UPM_SUB;
            psNcuRecSubToHeartBeatProc(pMsgBodySrc);
            psNcuUpdateNWDAFAddrLoad(g_nwdaflinkstatus);
            break;
        }
        case EV_MSG_UPM_TO_NCU_SUB_TIMER:
        {
            //只有0号群处理订阅消息，触发心跳请求
            JOB_LOC_STAT_ADDONE(qwRcvUpm2NcuSubTimer);
            if( MCS_RET_SUCCESS == psUpmNwdafIPlistIpIsNotEqualNcuNwdafIP(pMsgBodySrc)|| NCU_NOT_RCV_UPM_SUB_AND_TIMER_HANDLE == g_ncu_rcvupmsub)
            {
              JOB_LOC_STAT_ADDONE(qwRcvUpm2NcuSubTimerAndHandle);
              g_ncu_rcvupmsub = NCU_RCV_UPM_TIMER_HANDLE;
              psNcuRecSubToHeartBeatProc(pMsgBodySrc);
            }
            break;
        }
        case EV_TIMER_15:
        {
            /*防止上电获取不到线程信息*/
            psNcuInitAgeingScan();
            break;
        }
        case EV_MSG_UPM_TO_NCU_SUB_CORE_SYNC_REQ:
        {
            psNcuHandleSubCoreSyncReq(pMsgBodySrc);
            JOB_LOC_STAT_ADDONE(qwRcvUpm2NcuSubCoreSyncReq);
            break;
        }
        //定时触发心跳检测请求
        case EV_TIMER_NCU_NWDAF_HEARTBEAT_CHACK:
        {
            //统计发送次数
            //根据发送次数确定是否需要告警或者同步心跳信息到其他NCU
            if(FALSE == ncuIsSelfGroup(0))
            {
                DEBUG_TRACE(DEBUG_LOW, "psNcuSendHeartBeatReqProc: not 0 group ncu handle\n");
                return;
            }
            JOB_LOC_STAT_ADDONE(qwRcvHeatBeatCheckTimer);
            psNcuSendHeartBeatReqProc();
            break;
            
        }
        case EV_MSG_NWDAF_TO_NCU_PKT: //从其他NCU互转过来的报文
        {
            //接收到心跳响应处理
            JOB_LOC_STAT_ADDONE(qwRcvMsgNcuToNcuRspPkt);
            psNcuRcvNwdafRspPktProc(pMsgBodySrc);
            break;
        }
        case EV_MSG_UCOM_MCS_TO_JOB_RSQ:
        {
            JOB_LOC_STAT_ADDONE(qwRcvMsgTheardToJobHeartRsp);
            psNcuHeartRsqProc(pMsgBodySrc, wMsgLen);
            break;
        }
		case EV_MSG_UPM_TO_NCU_SESSION_REQ:
        {
          ncuHandleSessLoadProc((UPM_TO_NCU_SESSION_REQ *)pMsgBodySrc);
          break;
        }
        case EV_TIMER_12:
        {
            psNcuDetectAndGetValidLink();
            break;
        }
        case EV_TIMER_20:
        {
            sendGroupMigrateOutReqToMediaThread();
            break;
        }
        case EV_MSG_PFU_TO_NCU_MANAGE_ECHO_REQUEST:
        {
            psNcuSendThrdNumToPfu((PFU_TO_NCU_ECHO_REQ *)pMsgBodySrc);
            break;
        }
        default:
        {
            break;
        }
    }
    if(g_show_job_msgid <= 100)
    {
        g_show_job_msgid ++;
        zte_printf_s("psVpfuMcsManageJobMasterProc msgID=%u, msgLen=%u\n", dwMsgId, wMsgLen);
    }
    return;


}

extern WORD32 g_dwNcuDynLibInitOver;
bool RecvConfigNotifyReq(BYTE* pbMsgBody)
{
    if (NULL == pbMsgBody)
        return false;

    T_DBM_NOTIFY_ACK_TUPLE  tTupleAck = { 0 };
    if (!DBM_APP_Notify_Ack_Standard(pbMsgBody, &tTupleAck) || NULL == tTupleAck.lpData)
    {
        return false;
    }
    
    if (0 == g_dwNcuDynLibInitOver)
        return false;
    SimpleCfgRecvConfigNotifyReq(JOB_TYPE_MCS_MANAGE, pbMsgBody);
    return true;
}

WORD32 ncuHandleLoadStatusProc(UPM_TO_NCU_DALOAD_REQ *ptNcuLoadReq)
{
    MCS_CHK_NULL_RET(ptNcuLoadReq, UPF_RET_FAILED);

    NCU_TO_UPM_DALOAD_RSP tNcuLoadRsp = {0};
    tNcuLoadRsp.dwSeqNum = ptNcuLoadReq->dwSeqNum;
    tNcuLoadRsp.bNcuLogicNo = (BYTE)ncuGetScLogicNo();

    MCS_CHK_RET(2 == g_ncu_load_dbgflg, UPF_RET_FAILED);

    if (1 == g_ncu_load_dbgflg)
    {
        tNcuLoadRsp.bCpuUsage = g_ncu_cpurate;
        tNcuLoadRsp.dwQaUserNum   = g_ncu_qausrnum;
        tNcuLoadRsp.dwKeyUserNum  = g_ncu_keyusrnum;
        tNcuLoadRsp.dwSampUserNum = g_ncu_sampusrnum;
    }
    else
    {
        tNcuLoadRsp.bCpuUsage = ncuGetCpuUsage();
        if(!ncuGetSingleTypeUsrUsageOccupancy(&tNcuLoadRsp))
        {
            JOB_LOC_STAT_ADDONE(qwNcuCalNcuDaloadUsrOcpFail);
        }
    }

    if (UPF_RET_SUCCESS != ncuSendMsgToUpm((BYTE *)&tNcuLoadRsp, sizeof(NCU_TO_UPM_DALOAD_RSP), EV_MSG_NCU_TO_UPM_DALOAD_RSP, JOB_TYPE_MCS_MANAGE))
    {
        JOB_LOC_STAT_ADDONE(qwNcuSendToUpmDaloadRspFail);
        return UPF_RET_FAILED;
    }

    JOB_LOC_STAT_ADDONE(qwNcuSendToUpmDaloadRspSucc);

    return UPF_RET_SUCCESS;
}

VOID ncuSendToUpmSynSubCoreReqProc()
{
    NCU_TO_UPM_SUB_CORE_SYNC_REQ ncuToUpmSynSubCoreReq = {0};
    ncuToUpmSynSubCoreReq.dwNcuCommid = ncuGetSelfCommId();

    if (UPF_RET_SUCCESS != ncuSendMsgToUpm((BYTE *)&ncuToUpmSynSubCoreReq, sizeof(NCU_TO_UPM_SUB_CORE_SYNC_REQ), EV_MSG_NCU_TO_UPM_SUB_CORE_SYNC_REQ, JOB_TYPE_MCS_MANAGE))
    {
        JOB_LOC_STAT_ADDONE(qwNcuSendToUpmSynSubCoreReqFail);
        return;
    }
    JOB_LOC_STAT_ADDONE(qwNcuSendToUpmSynSubCoreReqSucess);
    return;
}

BYTE ncuGetUserOccupancy(DWORD dwDataType)
{
    WORD32 dwUseNum       = 0;
    WORD32 dwCapacity     = 0;
    WORD32 dwSoftThreadNo = 0;
    WORD32 dwRateSum      = 0;
    WORD32 dwhDB          = 0xff;
    WORD32 i              = 0;
    BYTE   bInstNum       = g_ptVpfuShMemVar->tVpfuMcsThreadPara.bInstNum;
    BYTE   bRate          = 0;

    MCS_CHK_RET(0 == bInstNum, 0);

    WORD32 dwTotalCapcity = g_upfConfig.ncuLicenseCfg.dwKeyServQaCapacity + g_upfConfig.ncuLicenseCfg.dwKeyUserDataColCapacity + g_upfConfig.ncuLicenseCfg.dwSampUserDataColCapacity;
    MCS_CHK_RET(0 == dwTotalCapcity, 0);

    for(i = 0; (i < bInstNum) && (i < MAX_PS_THREAD_INST_NUM); i++)
    {
        dwCapacity = 0;
        dwUseNum = 0;
        dwSoftThreadNo = (WORD32)((g_ptVpfuShMemVar->tVpfuMcsThreadPara.atThreadParaList[i].bSoftThreadNo) % MAX_PS_THREAD_INST_NUM);

        dwhDB = _NCU_GET_DBHANDLE(dwSoftThreadNo);
        psVpfuMcsGetCapacity(dwDataType, dwhDB, &dwCapacity, &dwUseNum);

        dwRateSum += ((100 * dwUseNum) / dwTotalCapcity);
    }

    bRate = (BYTE)(dwRateSum / bInstNum);
    bRate = (bRate < 100) ? bRate : 100;

    return bRate;
}

BOOLEAN ncuGetSingleTypeUsrUsageOccupancy(NCU_TO_UPM_DALOAD_RSP *ptNcuLoadRsp)
{
    MCS_CHK_NULL_RET(ptNcuLoadRsp, FALSE);

    WORD32 i = 0;

    T_NcuSessionStat *ptNcuSessionStat = NULL;
    T_NcuSessionStat tNcuSessnStatStat = {0};

    for (i = 0; i < MEDIA_THRD_NUM; i++)
    {
        ptNcuSessionStat = &(g_ptVpfuShMemVar->tNcuSessionStat[i]);

        tNcuSessnStatStat.qwQualityAssuranceSessions += ptNcuSessionStat->qwQualityAssuranceSessions;
        tNcuSessnStatStat.qwKeyUserExperienceDataSessions += ptNcuSessionStat->qwKeyUserExperienceDataSessions;
        tNcuSessnStatStat.qwRegularUserExperienceDataSessions += ptNcuSessionStat->qwRegularUserExperienceDataSessions;
    }
    ptNcuLoadRsp->dwQaUserNum = tNcuSessnStatStat.qwQualityAssuranceSessions;
    ptNcuLoadRsp->dwKeyUserNum = tNcuSessnStatStat.qwKeyUserExperienceDataSessions;
    ptNcuLoadRsp->dwSampUserNum = tNcuSessnStatStat.qwRegularUserExperienceDataSessions;

    return TRUE;
}

BYTE ncuGetCpuUsage()
{
    BYTE bNcuCpuUsage = 0;
    BYTE bCputemp = 0;
    WORD32 dwCputemp = 0;
    WORD32 j = 0;
    WORD32 dwPsmStatus = THDM_FAIL;

    enum THDM_MEDIA_TYPE threadType = THDM_MEDIA_TYPE_PFU_PFUC;
    T_PSThreadInstAffinityPara tThreadInstAffinityPara;
    T_PSThreadInstAffinityPara *ptThreadInstAffinityPara = &tThreadInstAffinityPara;

    for (; threadType < THDM_MEDIA_TYPE_END; threadType++)
    { // 每个线程内部取平均，线程间取最大
        dwPsmStatus = psGetMediaThreadInfo(threadType, ptThreadInstAffinityPara);

        MCS_CHK_CONTINUE(THDM_SUCCESS != dwPsmStatus);

        dwCputemp = 0;
        bCputemp = 0;

        for (j = 0; j < ptThreadInstAffinityPara->bInstNum && j < MAX_PS_THREAD_INST_NUM; j++)
        {
            dwCputemp += (WORD32)(pdaGetcpuload(ptThreadInstAffinityPara->atThreadParaList[j].bvCPUIndex));
        }

        if (0 != ptThreadInstAffinityPara->bInstNum)
        {
            bCputemp = (BYTE)(dwCputemp / ptThreadInstAffinityPara->bInstNum);
        }

        bNcuCpuUsage = (bNcuCpuUsage > bCputemp) ? bNcuCpuUsage : bCputemp;
    }

    return bNcuCpuUsage;
}

WORD64 g_ddwMaxCpuTest = 0;
WORD64 ncuGetMaxCpuload()
{
    WORD64 ddwCputemp = 0;
    WORD64 ddwMaxCpuload = 0;
    WORD32 j = 0;
    WORD32 dwPsmStatus = THDM_FAIL;

    enum THDM_MEDIA_TYPE threadType = THDM_MEDIA_TYPE_PFU_MEDIA_PROC;
    T_PSThreadInstAffinityPara tThreadInstAffinityPara;
    T_PSThreadInstAffinityPara *ptThreadInstAffinityPara = &tThreadInstAffinityPara;

    dwPsmStatus = psGetMediaThreadInfo(threadType, ptThreadInstAffinityPara);

    MCS_CHK_RET(THDM_SUCCESS != dwPsmStatus, 0);

    for (j = 0; j < ptThreadInstAffinityPara->bInstNum && j < MAX_PS_THREAD_INST_NUM; j++)
    {
        ddwCputemp = pdaGetcpuload(ptThreadInstAffinityPara->atThreadParaList[j].bvCPUIndex);
        if(ddwCputemp > ddwMaxCpuload)
        {
            ddwMaxCpuload = ddwCputemp;
        }
    }
    if(0 == g_ddwMaxCpuTest)
    {
        return ddwMaxCpuload;
    }
    else 
    {
        return g_ddwMaxCpuTest;
    }
}

WORD32 ncuHandleSessLoadProc(UPM_TO_NCU_SESSION_REQ *ptNcuLoadReq)
{
    MCS_CHK_NULL_RET(ptNcuLoadReq, UPF_RET_FAILED);
    MCS_CHK_NULL_RET(g_ptVpfuShMemVar,UPF_RET_FAILED);
    NCU_TO_UPM_SESSION_RSP tNcuLoadRsp = {0};
    WORD32 dwID        = 0;

    T_NcuSessionStat *ptNcuSessionStat        = NULL;
    T_NcuSessionStat  tNcuSessionStatStat     = {0};
    tNcuLoadRsp.dwSeqNum = ptNcuLoadReq->dwSeqNum;
    tNcuLoadRsp.bNcuLogicNo = ncuGetScLogicNo();

    MCS_CHK_RET(2 == g_ncu_sess_dbgflg, UPF_RET_FAILED);
    if(g_ncu_sess_dbgflg)//for test
    {
        tNcuLoadRsp.dwNcuSession   = g_ncu_session_test;
        tNcuLoadRsp.dwNcuQaSession = g_ncu_qasession_test;
        tNcuLoadRsp.dwNcuVipSession = g_ncu_vipsession_test;
        tNcuLoadRsp.dwNcuSampSession = g_ncu_sampsession_test;
    }
    else
    {
        for(dwID = 0; dwID < MEDIA_THRD_NUM; dwID++)
        {
            ptNcuSessionStat = &(g_ptVpfuShMemVar->tNcuSessionStat[dwID]);

            tNcuSessionStatStat.qwCurrentSessions                    += ptNcuSessionStat->qwCurrentSessions;
            tNcuSessionStatStat.qwQualityAssuranceSessions           += ptNcuSessionStat->qwQualityAssuranceSessions;
            tNcuSessionStatStat.qwKeyUserExperienceDataSessions      += ptNcuSessionStat->qwKeyUserExperienceDataSessions;
            tNcuSessionStatStat.qwRegularUserExperienceDataSessions  += ptNcuSessionStat->qwRegularUserExperienceDataSessions;
        }
        tNcuLoadRsp.dwNcuSession = tNcuSessionStatStat.qwCurrentSessions;
        tNcuLoadRsp.dwNcuQaSession =  tNcuSessionStatStat.qwQualityAssuranceSessions;
        tNcuLoadRsp.dwNcuVipSession =  tNcuSessionStatStat.qwKeyUserExperienceDataSessions;
        tNcuLoadRsp.dwNcuSampSession =  tNcuSessionStatStat.qwRegularUserExperienceDataSessions;
    }
    if (UPF_RET_SUCCESS != ncuSendMsgToUpm((BYTE *)&tNcuLoadRsp, sizeof(NCU_TO_UPM_SESSION_RSP), EV_MSG_NCU_TO_UPM_SESSION_RSP, JOB_TYPE_MCS_MANAGE))
    {
        return UPF_RET_FAILED;
    }

    return UPF_RET_SUCCESS;
}

void psNcuSendThrdNumToPfu(PFU_TO_NCU_ECHO_REQ * ptPfuEchoReq)
{   
    MCS_CHK_NULL_VOID(ptPfuEchoReq);
    T_PSThreadInstAffinityPara tInstPara = {0};
    WORD32 dwRet = psGetMediaThreadInfo(THDM_MEDIA_TYPE_PFU_MEDIA_PROC, &tInstPara);
    if ((THDM_SUCCESS != dwRet) || (tInstPara.bInstNum > MAX_PS_THREAD_INST_NUM))
    { 
        JOB_LOC_STAT_ADDONE(qwNcuRcvPfuEchoReqGetThrdNumFail);
        return;
    }
    NCU_TO_PFU_ECHO_RES tNcuEchoRes = {0};
    tNcuEchoRes.dwSeqNum = ptPfuEchoReq->dwSeqNum;
    tNcuEchoRes.dwNcuThrdNum = tInstPara.bInstNum;
    WORD32 dwCommId = ptPfuEchoReq->dwPfuCommid;
    if (UPF_RET_SUCCESS != upfSendMsgToJob((BYTE *)&tNcuEchoRes, sizeof(NCU_TO_PFU_ECHO_RES), EV_MSG_NCU_TO_PFU_MANAGE_ECHO_RESPONSE, dwCommId, JOB_TYPE_MCS_MANAGE))
    {
        JOB_LOC_STAT_ADDONE(qwNcuRcvPfuEchoReqSendResFail);
        return;
    }

    JOB_LOC_STAT_ADDONE(qwNcuRcvPfuEchoReqSendResSucc);
    return;

}
VOID psVpfuSigtraceOvertimeStop()
{
    WORD32 Current_Time = XOS_GetSysRunSecFromPowerOn(); 
    WORD32 SigOverTimeThresholdsec = (WORD32)g_sigtracecfg.bSigOverTimeThreshold_Hours*3600;
    WORD32 bIndex;
    for(bIndex=SIGTRACE_TASK_NUM_MAX;bIndex!=0;bIndex--)
    {
        if(0 == GetSigTraceID(bIndex-1))
        {
            continue;
        }

        if(0 == SigTraceTaskIsValid(bIndex-1))
        {
            continue;
        }

        WORD32 createtime = GetSigTraceCreatetime(bIndex-1);
        _mcs_if(Current_Time >= (createtime + SigOverTimeThresholdsec))
        {
            Trace_RmTask(GetSigTraceID(bIndex-1));
            JOB_LOC_STAT_ADDONE(qwSigTraceRemoveOverTimeTaskByJob);
        }
    }
    return;
}

VOID psVpfuTraceManage()
{
    if(0 != *pSigTraceTaskNum && 0 != g_sigtracecfg.bSigOverTimeThreshold_Hours)
    {
        psVpfuSigtraceOvertimeStop();
    }

    return;
}

VOID psNcuHandleSubCoreSyncReq(VOID* pMsgBody)
{
    MCS_CHK_NULL_VOID(pMsgBody);
    T_Upm2NcuSubData* ptSubData = (T_Upm2NcuSubData*)pMsgBody;
    if (ptSubData->id == 0 || ptSubData->id > 16)
    {
        JOB_LOC_STAT_ADDONE(qwHandleUpm2NcuInvalidId);
        return ;
    }
    ExpNormalCorrData* ptExpNormalCorrData = &g_CorrData[ptSubData->id-1];
    if (DEL_SUB == ptSubData->opCode)
    {
        zte_memset_s(ptExpNormalCorrData, sizeof(ExpNormalCorrData), 0, sizeof(ExpNormalCorrData));
        return ;
    }
    if (ADD_SUB == ptSubData->opCode)
    {
        ptExpNormalCorrData->isValid = 1;
        zte_memcpy_s(ptExpNormalCorrData->Dnn, sizeof(ptExpNormalCorrData->Dnn), ptSubData->dnn, sizeof(ptSubData->dnn));
        zte_memcpy_s(ptExpNormalCorrData->CorrelationId, sizeof(ptExpNormalCorrData->CorrelationId), ptSubData->notifyCoreId, sizeof(ptSubData->notifyCoreId));
    }
}

VOID psVncuGetLicenseCheckJobStartupProc(WORD32 dwMsgId,
                         VOID *pMsgBody,
                         VOID *pPData)
{
    MCS_PCLINT_NULLPTR_RET_VOID(pMsgBody);
    MCS_PCLINT_NULLPTR_RET_VOID(pPData);

    switch(dwMsgId)
    {
        /*sgw pgw处理相同*/
        case EV_MASTER_POWER_ON:
        case EV_TIMER_GET_LICENSE_CHECK:
        {
            WORD32 dwRet   = 1;
            BYTE   bStatus = 0;
            dwRet = DBMGetLicenseSyncStatus(&bStatus);
            zte_printf_s("[psVncuuGetLicenseJobStartupProc]bStatus=%u\n",bStatus);
            /* pbStatus 状态  0: 未同步  1: 已同步 */
            zte_printf_s("GetLicenseCheckJob power on success\n");
            XOS_PowerOnAck(EV_POWER_ON_SUCCESS, NULL, 0, 0);
            /* 状态迁移  */
            XOS_SetNextState(S_WORK);
            break;
        }
        case EV_POWER_OFF:
        {
            zte_printf_s("[GetLicenseJob]Receive EV_POWER_OFF!!! file:%s, line:%d  \n", __FILE__, __LINE__);
            break;
        }
        default:
        {
           zte_printf_s("[GetLicenseJob]unknown messages 0x%08x!!! file:%s, line:%d \n", dwMsgId, __FILE__, __LINE__);
            /* 状态迁移 */
            XOS_SetDefaultNextState();
            break;
        }
    }

    return;
}
/**********************************************************************
* 函数名称：  psVncuGetLicenseCheckJobEntry
* 功能描述：  获取License Check Job入口函数,详细描述
* 访问的表：
* 修改的表：
* 输入参数：
            wState       JOB״̬
            dwMsgId      消息ID
            pMsgBody     消息体指针
            pPData       私有数据区指针
            bSame        是否相同字节序
* 输出参数：
* 返 回 值：  无
* 其它说明：
***********************************************************************/
VOID psVncuGetLicenseCheckJobEntry(WORD16 wState,
                         WORD32 dwMsgId,
                         VOID *pMsgBody,
                         VOID *pPData,
                         BOOLEAN bSame)
{
    MCS_PCLINT_NULLPTR_RET_VOID(pMsgBody);
    MCS_PCLINT_NULLPTR_RET_VOID(pPData);

    switch(wState)
    {
        /* 初始业务状态 */
        case S_StartUp:
        {
            psVncuGetLicenseCheckJobStartupProc(dwMsgId, pMsgBody, pPData);
            break;
        }
        /* 主用状态 */
        case S_WORK:
        {
            zte_printf_s("[psVncuGetLicenseJobEntry]Change to S_Work\n");
            XOS_SetDefaultNextState();
            break;

        }
        case S_POWER_OFF:
        {
            zte_printf_s("\n[GetLicenseJob]Power off status recive msg  0x%08x \n",dwMsgId);
            XOS_SetDefaultNextState();
            break;
        }
        default:
        {
            XOS_SetDefaultNextState();
            break;
        }
    }
    return;
}

VOID psNcuSetLinkCheckTimer(VOID *pPData)
{
    if(NULL == pPData)
    {
        return;
    }

    T_psManageJobPriData *ptManageJobPritData = (T_psManageJobPriData *)pPData;
    ptManageJobPritData->dwLinkCheckTimerId = INVALID_TIMER_ID;
    ptManageJobPritData->dwLinkCheckTime = g_upfConfig.daHttpCfg.dwCheckTime * 1000;
    ptManageJobPritData->dwLinkCheckTimerId = XOS_SetLoopTimer(TIMER_NO_12, ptManageJobPritData->dwLinkCheckTime, PARAM_NULL);
    if(INVALID_TIMER_ID == ptManageJobPritData->dwLinkCheckTimerId)
    {
        JOB_LOC_STAT_ADDONE(qwSetLinkCheckTimerFail);
        return;
    }
    JOB_LOC_STAT_ADDONE(qwSetLinkCheckTimerSucc);
    return;
}

VOID psNcuUpdateLinkCheckTimer(VOID *pPData)
{
    if(NULL == pPData)
    {
        return;
    }
    T_psManageJobPriData *ptManageJobPritData = (T_psManageJobPriData *)pPData;
    WORD32 dwLinkCheckTime = g_upfConfig.daHttpCfg.dwCheckTime * 1000;
    if (dwLinkCheckTime == ptManageJobPritData->dwLinkCheckTime)
    {
        return;
    }

    if(INVALID_TIMER_ID != ptManageJobPritData->dwLinkCheckTimerId)
    {
        WORD32 ret = XOS_KillTimerByTimerId(ptManageJobPritData->dwLinkCheckTimerId);
        if (XOS_SUCCESS != ret)
        {
            JOB_LOC_STAT_ADDONE(qwKillLinkCheckTimerFail);
        }
        ptManageJobPritData->dwLinkCheckTimerId = INVALID_TIMER_ID;
    }
    ptManageJobPritData->dwLinkCheckTime = dwLinkCheckTime;
    ptManageJobPritData->dwLinkCheckTimerId = XOS_SetLoopTimer(TIMER_NO_12, ptManageJobPritData->dwLinkCheckTime, PARAM_NULL);
    if(INVALID_TIMER_ID == ptManageJobPritData->dwLinkCheckTimerId)
    {
        JOB_LOC_STAT_ADDONE(qwResetLinkCheckTimerFail);
        return;
    }
    JOB_LOC_STAT_ADDONE(qwResetLinkCheckTimerSucc);
    return;
}
// end of file
/* Ended by AICoder, pid:ea6d2531f2284fb14132094be05ad95955f6ea64 */
