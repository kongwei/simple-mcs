/******************************************************************************
 * 版权所有 (C)2016 深圳市中兴通讯股份有限公司*
 * 模块名          : MCS
 * 文件名          : psNcuHeartBeatProc.c
 * 相关文件        :
 * 文件实现功能     : Nwdaf链路心跳检测处理流程
 * 归属团队        : 
 * 版本           : V7.24.20
-------------------------------------------------------------------------------
* 修改记录:
* 修改日期            版本号           修改人           修改内容
* 2024-04-28      V7.24.20                   create
******************************************************************************/
/**************************************************************************
 *                              头文件(满足最小依赖请按照DDD分层架构逐层依赖)
 **************************************************************************/
#include "ps_pub.h"
#include "UpfNcuSynInfo.h"

#include "psMcsDebug.h"
#include "McsIPv4Head.h"
#include "McsIPv6Head.h"
#include "zte_slibc.h"
#include "ps_db_define_pfu.h"

#include "psUpfEvent.h"
#include "psUpfJobTypes.h"
#include "psMcsManageJob.h"
#include "psNcuReportStructHead.h"
#include "psNcuReportStructData.h"
#include "psMcsGlobalCfg.h"
#include "psNcuGetCfg.h"
#include "ncuSCCUAbility.h"
#include "dpathreadpub.h"
#include "dpa_parser.h"
#include "McsUDPHead.h"
#include "McsIPv4Head.h"
#include "McsIPv6Head.h"
#include "McsByteOrder.h"
#include "psUpfPubSub.h"
#include "psNcuHeartBeatProc.h"
#include "upfUcomInterface.h"
#include "dpa_rcvsend.h"
#include "psNcuRouteProc.h"
#include "psNcuNWDAFAddrProc.h"
#include "McsHeadCap.h"
#include "ps_packet.h"
#include "psUpfPubSub.h"
#include "dpa_compat.h"
#include "ps_mcs_trace.h"
#include "psNcuDataEncode.h"
#include "psNcuCtrlStatInterface.h"
#include "ps_mcs_define.h"
#include "ps_ncu_typedef.h"

/*Ncu Log test begin*/
#include "UPFLog.h"
#include "UPFSysLogCode.h"
#include "upfPub.h"
/*Ncu Log test end*/

/**************************************************************************
 *                              宏(本源文件使用)
 **************************************************************************/
#define  NCU_SEND_UPM_ALARM   1
#define  NCU_SEND_UPM_SYN     2


BYTE g_ncu_heart_show = 0xFF;
#undef DEBUG_TRACE
#define DEBUG_TRACE(level, ...) \
do { \
    if (unlikely(level >= g_ncu_heart_show)) { \
        zte_printf_s("\n[MCS:%-30s:%u] ", __FUNCTION__, __LINE__); \
        zte_printf_s(__VA_ARGS__); \
    } \
} while (0)

/**************************************************************************
 *                              常量(本源文件使用)
 **************************************************************************/

/**************************************************************************
 *                              数据类型(本源文件使用)
 **************************************************************************/

/**************************************************************************
 *                              外部函数原型(评估后慎重添加)
 **************************************************************************/
extern WORD16 g_report_seq_num;

/**************************************************************************
 *                              局部函数原型(本源文件使用)
 **************************************************************************/
WORD16 psEncodeHeartReqDataProc(BYTE* buffer);
VOID psNcuShowNwdafIplistProc();
WORD32 psNcuCSSGSUMediaSendOut(PS_PACKET *ptPacket);
WORD32 ncuUcomCopySendToJob(PS_PACKET *ptPacket,WORD32 dwJno,WORD32 dwServiceID);
WORD32 psNcuCheckHeartbeat(T_psNcuHeartHead* ptNcuHeartHead, WORD16 wBufferLen, WORD16 wBufferIpUdpOffset);
/**************************************************************************
 *                              全局变量(本源文件使用)
 **************************************************************************/
T_NwdafStatus g_nwdaflinkstatus[NWDAF_LINKNUM_MAX] = {0};
WORD32 g_ncuHeartBeatReqTimer = INVALID_TIMER_ID;

/**************************************************************************
 *                              函数实现(函数布局:总分结构|深度优先)
 **************************************************************************/
/*NWDAF局向IPList同步请求*/
VOID psNcuSendNwdafIplistSyntoNcu()
{
    if(FALSE == ncuIsSelfGroup(0))
    {
        DEBUG_TRACE(DEBUG_LOW, "psNcuSendNwdafIplistSyntoNcu: not 0 group Ncu\n");
        return;
    }
    WORD32 maxLogicNo = ncuGetMaxScLogicNo();
    WORD32 commid = 0;

    WORD32 i = 0;
    for(i=1; i<=maxLogicNo; i++)
    {
        commid = ncuGetCommIdByLogicNo(i);
        if(commid != INVALID_COMM_ID && commid != ncuGetSelfCommId())
        {
            //增加同步统计
            JOB_LOC_STAT_ADDONE(qwSendNwdafIplistSyntoNcu);
            upfSendMsgToJob((BYTE*)(g_nwdaflinkstatus), sizeof(T_NwdafStatus)*NWDAF_LINKNUM_MAX, EV_MSG_NCU_TO_NCU_NWDAFIPLIST_SYNC_REQ,commid,JOB_TYPE_MCS_MANAGE);
        }
    }

    return;
}

VOID psNcuUpdateNwdafIplistProc(BYTE * pMsgBodySrc)
{
    PS_NCU_NULL_CHECK_VOID(pMsgBodySrc);
    if(ncuIsSelfGroup(0))
    {
        DEBUG_TRACE(DEBUG_LOW, "psNcuUpdateNwdafIplistProc: 0 group Ncu Recv Syn NwdaIplist\n");
        return;
    }
    JOB_LOC_STAT_ADDONE(qwRevNwdafIplistSyntoNcu);

    T_NwdafStatus *ptNwdafIplist = (T_NwdafStatus *)pMsgBodySrc;
    WORD32 i = 0;
    for(i = 0; i< NWDAF_LINKNUM_MAX; i++)
    {
        zte_memcpy_s(&g_nwdaflinkstatus[i], sizeof(T_NwdafStatus), ptNwdafIplist, sizeof(T_NwdafStatus));
        ptNwdafIplist++;
    }

    return;
}



INLINE VOID psNcuKillHeartBeatReqTimer()
{
    JOB_LOC_STAT_ADDONE(qwNcuKillHeartBeatReqTimer);
    if(INVALID_TIMER_ID != g_ncuHeartBeatReqTimer )
    {
        XOS_KillTimerByTimerId(g_ncuHeartBeatReqTimer);
        g_ncuHeartBeatReqTimer = INVALID_TIMER_ID;
    }
    return;
}

INLINE VOID psNcuSetHeartBeatReqTimer()
{
    g_ncuHeartBeatReqTimer = XOS_SetLoopTimer(TIMER_NCU_NWDAF_HEARTBEAT_CHACK,g_upfConfig.daSyslinkCfg.echotime*1000,PARAM_NULL);
    if(INVALID_TIMER_ID == g_ncuHeartBeatReqTimer )
    {
        //增加日志
        JOB_LOC_STAT_ADDONE(qwNcuCreatHeartBeatReqTimerFail);
        return;
    }
    JOB_LOC_STAT_ADDONE(qwNcuKillHeartBeatReqTimerSucc);
    return;
}

WORD32 psGetNcuNwdafIPlistIpIsAllNull()
{
    WORD32 bNwdafNum = 0;
    for(bNwdafNum = 0 ; bNwdafNum < NWDAF_LINKNUM_MAX; bNwdafNum++)
    {
       if(g_nwdaflinkstatus[bNwdafNum].tNwdafAddr.bNwdafIPType)
        {
             return MCS_RET_SUCCESS;
        }
    }
    return MCS_RET_FAIL;
}

WORD32 psGetUpmNwdafIPlistIpIsAllNull(BYTE *pMsgBodySrc)
{
    PS_NCU_NULL_CHECK_RET(pMsgBodySrc,MCS_RET_FAIL);
    WORD32 bNwdafNum = 0;
    T_Ncu_NwdafLinkInfo *pUpmToNcuNwdafIPlist = (T_Ncu_NwdafLinkInfo *)pMsgBodySrc;
    for(bNwdafNum = 0 ; bNwdafNum < NWDAF_LINKNUM_MAX; bNwdafNum++)
    {
       if(pUpmToNcuNwdafIPlist[bNwdafNum].bNwdafIPType)
        {
             return MCS_RET_SUCCESS;
        }
    }
    return MCS_RET_FAIL;
}

WORD32 psUpmNwdafIPlistIpIsNotEqualNcuNwdafIP(BYTE *pMsgBodySrc)
{
    PS_NCU_NULL_CHECK_RET(pMsgBodySrc,MCS_RET_FAIL);
    WORD32 dwNwdafNum =    0;
    WORD32 dwNcuNwdafNum = 0;
    BYTE bUpmNwdafValidIpNum = 0;
    BYTE bNcuNwdafValidIpNum = 0;
    BYTE bFindTheSame        = 0;
    T_Ncu_NwdafLinkInfo *pUpmToNcuNwdafIPlist = (T_Ncu_NwdafLinkInfo *)pMsgBodySrc;
    for(dwNwdafNum = 0 ; dwNwdafNum < NWDAF_LINKNUM_MAX; dwNwdafNum++)
    {
       T_Ncu_NwdafLinkInfo *pUpmToNcuNwdafAdrr =  &pUpmToNcuNwdafIPlist[dwNwdafNum];
       if(pUpmToNcuNwdafAdrr->bNwdafIPType)
        {
             bUpmNwdafValidIpNum++;
        }
    }

    for(dwNcuNwdafNum = 0 ; dwNcuNwdafNum < NWDAF_LINKNUM_MAX; dwNcuNwdafNum++)
    {
       if(g_nwdaflinkstatus[dwNcuNwdafNum].tNwdafAddr.bNwdafIPType)
        {
             bNcuNwdafValidIpNum++;
        }
    }

    if(bUpmNwdafValidIpNum != bNcuNwdafValidIpNum)
    {
        //不相等
        JOB_LOC_STAT_ADDONE(qwNcugetUpmNwdafIpNumNotEqual);
        return  MCS_RET_SUCCESS;
    }

    for(dwNwdafNum = 0 ; dwNwdafNum < NWDAF_LINKNUM_MAX; dwNwdafNum++)
    {   
        bFindTheSame  =  0;
        T_Ncu_NwdafLinkInfo *pUpmToNcuNwdafAdrr =  &pUpmToNcuNwdafIPlist[dwNwdafNum];
        if(0 == pUpmToNcuNwdafAdrr->bNwdafIPType)
        {
            continue;
        }
        for(dwNcuNwdafNum = 0 ; dwNcuNwdafNum < NWDAF_LINKNUM_MAX; dwNcuNwdafNum++)
        {
            T_Ncu_NwdafLinkInfo *pNcuNwdafIPlist = &g_nwdaflinkstatus[dwNcuNwdafNum].tNwdafAddr;
            if(0 == memcmp(pUpmToNcuNwdafAdrr->NwdafIPv4, pNcuNwdafIPlist->NwdafIPv4, IPV4_LEN) &&  0 == memcmp(pUpmToNcuNwdafAdrr->NwdafIPv6, pNcuNwdafIPlist->NwdafIPv6, IPV6_LEN))
            {
                bFindTheSame = 1;
                break;
            }
        }
        if(!bFindTheSame)
        {
            //不相等
            JOB_LOC_STAT_ADDONE(qwNcuNcuNwdafIpNotEqualUpmNwdafIp);
            return  MCS_RET_SUCCESS;
        }
    }
    
    return MCS_RET_FAIL;
}

WORD32 psGetNcuUpfIp(T_NcuUpfIpInfo *ptNcuUpfIpInfo)
{
    PS_NCU_NULL_CHECK_RET(ptNcuUpfIpInfo,MCS_RET_FAIL);
    inet_pton(AF_INET,  g_upfConfig.daUpfIpCfg.upfIpv4, ptNcuUpfIpInfo->NcuUpfIPv4);
    inet_pton(AF_INET6, g_upfConfig.daUpfIpCfg.upfIpv6, ptNcuUpfIpInfo->NcuUpfIPv6);
    
    if(FALSE == psCheckIPValid(ptNcuUpfIpInfo->NcuUpfIPv4, IPv4_VERSION) && FALSE == psCheckIPValid(ptNcuUpfIpInfo->NcuUpfIPv6, IPv6_VERSION))
    {
        //增加统计，获取配置失败
        DEBUG_TRACE(DEBUG_LOW, "psGetNcuUpfIp: get NcuupfIp fail\n");
        JOB_LOC_STAT_ADDONE(qwNcuGetUpfIpFail);
        return MCS_RET_FAIL;
    }
    if(FALSE == psCheckIPValid(ptNcuUpfIpInfo->NcuUpfIPv4, IPv4_VERSION) && TRUE == psCheckIPValid(ptNcuUpfIpInfo->NcuUpfIPv6, IPv6_VERSION))
    {
        ptNcuUpfIpInfo->bNcuUpfType = IPCOMM_TYPE_IPV6;
        return MCS_RET_SUCCESS;
    }
    if(TRUE == psCheckIPValid(ptNcuUpfIpInfo->NcuUpfIPv4, IPv4_VERSION) && FALSE == psCheckIPValid(ptNcuUpfIpInfo->NcuUpfIPv6, IPv6_VERSION))
    {
        ptNcuUpfIpInfo->bNcuUpfType = IPCOMM_TYPE_IPV4;
        return MCS_RET_SUCCESS;
    }
    if(TRUE == psCheckIPValid(ptNcuUpfIpInfo->NcuUpfIPv4, IPv4_VERSION) && TRUE == psCheckIPValid(ptNcuUpfIpInfo->NcuUpfIPv6, IPv6_VERSION))
    {
        ptNcuUpfIpInfo->bNcuUpfType = IPCOMM_TYPE_IPV4V6;
        return MCS_RET_SUCCESS;
    }
    return MCS_RET_FAIL;
}

//全部上报接口需要修改，一个消息发送
VOID psNcuAllNwdafLinkSynToUpm(BYTE         bSendUpmType)
{
    BYTE bNwdafNum = 0;
    T_NcuToUpm_NwdafLinkInfoAll tNcuToUpmNwdafLinkInfo = {0};
    for(bNwdafNum = 0 ; bNwdafNum < NWDAF_LINKNUM_MAX; bNwdafNum++)
    {
        if(NCU_SEND_UPM_ALARM == bSendUpmType)
        {
            g_nwdaflinkstatus[bNwdafNum].tNwdafAddr.bIPv4LinkStatus  = NWDAF_LINKSTATE_DOWN;
            g_nwdaflinkstatus[bNwdafNum].tNwdafAddr.bIPv6LinkStatus  = NWDAF_LINKSTATE_DOWN;
            g_nwdaflinkstatus[bNwdafNum].bLinkIPType = 0;
            g_nwdaflinkstatus[bNwdafNum].bSendIpv4Times = 0;
            g_nwdaflinkstatus[bNwdafNum].bSendIpv6Times = 0;
            g_nwdaflinkstatus[bNwdafNum].bLinkStatus = NWDAF_LINKSTATE_DOWN;
        }
        zte_memcpy_s(&tNcuToUpmNwdafLinkInfo.tNwdafLinkInfo[bNwdafNum], sizeof(T_Ncu_NwdafLinkInfo), &g_nwdaflinkstatus[bNwdafNum].tNwdafAddr, sizeof(T_Ncu_NwdafLinkInfo));
    }
    tNcuToUpmNwdafLinkInfo.bValidNwdafLinkNum = NWDAF_LINKNUM_MAX;
    tNcuToUpmNwdafLinkInfo.bEchoSwitch        = g_upfConfig.daSyslinkCfg.echoswitch;
    //给UPM发送EV_MSG_NCU_TO_UPM_DAFLINK
    JOB_LOC_STAT_ADDONE(qwNcuSendUpmAllLinkStatus);
    psNcuSendUpmUpdateLinkStatus(&tNcuToUpmNwdafLinkInfo);
    return;
}


VOID psFindAdrrInOldStatus(T_Ncu_NwdafLinkInfo *pNcuNwdafInfo,T_NwdafStatus * ptNwdaflinkOldStatus, BYTE bType)
{
     PS_NCU_NULL_CHECK_VOID(pNcuNwdafInfo);
     PS_NCU_NULL_CHECK_VOID(ptNwdaflinkOldStatus);
     BYTE IpCount= 0;
     for(IpCount= 0; IpCount < NWDAF_LINKNUM_MAX; IpCount++ )
     {
          T_Ncu_NwdafLinkInfo *pOldNwdafInfo      =  &ptNwdaflinkOldStatus[IpCount].tNwdafAddr;
         if(IPCOMM_TYPE_IPV4 == bType)
         {
             if(0 == memcmp(pNcuNwdafInfo->NwdafIPv4, pOldNwdafInfo->NwdafIPv4, IPV4_LEN))
             {
                 pNcuNwdafInfo->bIPv4LinkStatus = pOldNwdafInfo->bIPv4LinkStatus;
                 return;
             }
             continue;
         }
          else if(IPCOMM_TYPE_IPV6 == bType)
         {
              if(0 == memcmp(pNcuNwdafInfo->NwdafIPv6, pOldNwdafInfo->NwdafIPv6, IPV6_LEN))
              {
                  pNcuNwdafInfo->bIPv6LinkStatus = pOldNwdafInfo->bIPv6LinkStatus;
                  return;
              }
              continue;
         }
     }
     return;
}


WORD32 psGetUpmSubNwDafIplistProc(BYTE *pMsgBodySrc,  T_NwdafStatus *ptNwdaflinkOldStatus, BYTE *bGetNcuUpfIpFail)
{
    PS_NCU_NULL_CHECK_RET(pMsgBodySrc, MCS_RET_FAIL);
    PS_NCU_NULL_CHECK_RET(ptNwdaflinkOldStatus, MCS_RET_FAIL);
    //upm获取NwIplist，并初始化
    T_Ncu_NwdafLinkInfo *pUpmToNcuNwdafIPlist = (T_Ncu_NwdafLinkInfo *)pMsgBodySrc;
    T_NcuUpfIpInfo tNcuUpfIpInfo = {0};
    WORD32 IpCount= 0;
    if(MCS_RET_FAIL == psGetNcuUpfIp(&tNcuUpfIpInfo))
    {
        //增加统计，获取配置失败
        //增加 全部告警并同步到其他NCU
         DEBUG_TRACE(DEBUG_LOW, "psGetUpmSubNwDafIplistProc: get NcuupfIp fail\n");
        *bGetNcuUpfIpFail = 1;
    }
    for(IpCount= 0; IpCount < NWDAF_LINKNUM_MAX; IpCount++ )
    {
        T_Ncu_NwdafLinkInfo *pUpmToNcuNwdafAdrr =  &pUpmToNcuNwdafIPlist[IpCount];
        T_Ncu_NwdafLinkInfo *pNcuNwdafInfo      =  &g_nwdaflinkstatus[IpCount].tNwdafAddr;
        pNcuNwdafInfo->bNwdafIPType             =  pUpmToNcuNwdafAdrr->bNwdafIPType;
        IPV4_COPY(pNcuNwdafInfo->NwdafIPv4, pUpmToNcuNwdafAdrr->NwdafIPv4);
        IPV6_COPY(pNcuNwdafInfo->NwdafIPv6, pUpmToNcuNwdafAdrr->NwdafIPv6);
        switch (pUpmToNcuNwdafAdrr->bNwdafIPType)
        {
            case IPCOMM_TYPE_IPV4:
                //查询NCU的地址
                if(IPCOMM_TYPE_IPV4 == tNcuUpfIpInfo.bNcuUpfType || IPCOMM_TYPE_IPV4V6 == tNcuUpfIpInfo.bNcuUpfType)
                {
                    pNcuNwdafInfo->bIPv4LinkStatus = NWDAF_LINKSTATE_UP;
                    pNcuNwdafInfo->bIPv6LinkStatus = NWDAF_LINKSTATE_DOWN;
                    g_nwdaflinkstatus[IpCount].bLinkIPType = IPCOMM_TYPE_IPV4;
                }
                break;
            case IPCOMM_TYPE_IPV6:
                if(IPCOMM_TYPE_IPV6 == tNcuUpfIpInfo.bNcuUpfType || IPCOMM_TYPE_IPV4V6 == tNcuUpfIpInfo.bNcuUpfType)
                {
                    pNcuNwdafInfo->bIPv4LinkStatus = NWDAF_LINKSTATE_DOWN;
                    pNcuNwdafInfo->bIPv6LinkStatus = NWDAF_LINKSTATE_UP;
                    g_nwdaflinkstatus[IpCount].bLinkIPType = IPCOMM_TYPE_IPV6;
                }
                break;
            case IPCOMM_TYPE_IPV4V6:
                if(IPCOMM_TYPE_IPV4 == tNcuUpfIpInfo.bNcuUpfType)
                {
                    pNcuNwdafInfo->bIPv4LinkStatus = NWDAF_LINKSTATE_UP;
                    pNcuNwdafInfo->bIPv6LinkStatus = NWDAF_LINKSTATE_DOWN;
                    g_nwdaflinkstatus[IpCount].bLinkIPType = IPCOMM_TYPE_IPV4;
                }
                else if(IPCOMM_TYPE_IPV6 == tNcuUpfIpInfo.bNcuUpfType)
                {
                    pNcuNwdafInfo->bIPv4LinkStatus = NWDAF_LINKSTATE_DOWN;
                    pNcuNwdafInfo->bIPv6LinkStatus = NWDAF_LINKSTATE_UP;
                    g_nwdaflinkstatus[IpCount].bLinkIPType = IPCOMM_TYPE_IPV6;
                }
                else if(IPCOMM_TYPE_IPV4V6 == tNcuUpfIpInfo.bNcuUpfType)
                {
                    pNcuNwdafInfo->bIPv4LinkStatus = NWDAF_LINKSTATE_UP;
                    pNcuNwdafInfo->bIPv6LinkStatus = NWDAF_LINKSTATE_UP;
                    g_nwdaflinkstatus[IpCount].bLinkIPType = IPCOMM_TYPE_IPV4V6;
                }
                break;
            default:
                break;
        }

        DEBUG_TRACE(DEBUG_LOW, "bIPv4LinkStatus=%u,%u\n",
            pNcuNwdafInfo->bIPv4LinkStatus, pNcuNwdafInfo->bIPv6LinkStatus
        );

        if (NCECHOSWITCH_ENABLE == g_upfConfig.daSyslinkCfg.echoswitch
            && NWDAF_LINKSTATE_UP == pNcuNwdafInfo->bIPv4LinkStatus)
        {
            if(NWDAF_LINKSTATE_DOWN == pUpmToNcuNwdafAdrr->bIPv4LinkStatus)
            {
                pNcuNwdafInfo->bIPv4LinkStatus = pUpmToNcuNwdafAdrr->bIPv4LinkStatus;
            }
            //在旧的status中检查历史状态，获取历史状态
            psFindAdrrInOldStatus(pNcuNwdafInfo, ptNwdaflinkOldStatus, IPCOMM_TYPE_IPV4);
        }

        if (NCECHOSWITCH_ENABLE == g_upfConfig.daSyslinkCfg.echoswitch
            && NWDAF_LINKSTATE_UP == pNcuNwdafInfo->bIPv6LinkStatus)
        {
            if(NWDAF_LINKSTATE_DOWN == pUpmToNcuNwdafAdrr->bIPv6LinkStatus)
            {
                pNcuNwdafInfo->bIPv6LinkStatus = pUpmToNcuNwdafAdrr->bIPv6LinkStatus;
            }
            //在旧的status中检查历史状态，获取历史状态
            psFindAdrrInOldStatus(pNcuNwdafInfo, ptNwdaflinkOldStatus, IPCOMM_TYPE_IPV6);
        }

        DEBUG_TRACE(DEBUG_LOW, "bIPv4LinkStatus=%u,%u\n",
            pNcuNwdafInfo->bIPv4LinkStatus, pNcuNwdafInfo->bIPv6LinkStatus
        );
        
        if (pNcuNwdafInfo->bIPv4LinkStatus || pNcuNwdafInfo->bIPv6LinkStatus)
        {
            g_nwdaflinkstatus[IpCount].bLinkStatus = NWDAF_LINKSTATE_UP;
        }

        DEBUG_TRACE(DEBUG_LOW, "bIPv4LinkStatus=%u,%u\n",
            pNcuNwdafInfo->bIPv4LinkStatus, pNcuNwdafInfo->bIPv6LinkStatus
        );

        g_nwdaflinkstatus[IpCount].bSendIpv4Times = 0;
        g_nwdaflinkstatus[IpCount].bSendIpv6Times = 0;

    }
    return MCS_RET_SUCCESS;
}

VOID psNcuSendSingleLinkStatus(T_Ncu_NwdafLinkInfo         *ptNwdafLinkInfo)
{
    PS_NCU_NULL_CHECK_VOID(ptNwdafLinkInfo);
    T_NcuToUpm_NwdafLinkInfoAll tNcuToUpmNwdafLinkInfo = {0};
    //填充发送信息
    tNcuToUpmNwdafLinkInfo.bValidNwdafLinkNum = 1;
    tNcuToUpmNwdafLinkInfo.bEchoSwitch        = g_upfConfig.daSyslinkCfg.echoswitch;
    zte_memcpy_s(&tNcuToUpmNwdafLinkInfo.tNwdafLinkInfo[0], sizeof(T_Ncu_NwdafLinkInfo), ptNwdafLinkInfo, sizeof(T_Ncu_NwdafLinkInfo));
    //给UPM发送EV_MSG_NCU_TO_UPM_DAFLINK
    JOB_LOC_STAT_ADDONE(qwNcuSendUpmSingleLinkStatus);
    psNcuSendUpmUpdateLinkStatus(&tNcuToUpmNwdafLinkInfo);
    return;
}

VOID psNcuSendUpmUpdateLinkStatus(T_NcuToUpm_NwdafLinkInfoAll *ptNcuToUpmNwdafLinkInfo)
{
    PS_NCU_NULL_CHECK_VOID(ptNcuToUpmNwdafLinkInfo);
    DEBUG_TRACE(DEBUG_LOW, "psNcuSendUpmUpdateLinkStatus: ptNcuToUpmNwdafLinkInfo num = %d\n",ptNcuToUpmNwdafLinkInfo->bValidNwdafLinkNum);
    DEBUG_TRACE(DEBUG_LOW, "psNcuSendUpmUpdateLinkStatus: ptNcuToUpmNwdafLinkInfo echowitch = %d\n",ptNcuToUpmNwdafLinkInfo->bEchoSwitch);
    if(DEBUG_LOW >= g_ncu_heart_show)
    {
        psNcuShowNwdafIplistProc();
    }
    WORD32  dwRet  = 0;
    dwRet = ncuSendMsgToUpm((BYTE *)ptNcuToUpmNwdafLinkInfo, sizeof(T_NcuToUpm_NwdafLinkInfoAll),EV_MSG_NCU_TO_UPM_DAFLINK, JOB_TYPE_MCS_MANAGE);
    if(UPF_RET_SUCCESS != dwRet) 
    {
        //增加NCU发送UPM消息失败统计
        DEBUG_TRACE(DEBUG_LOW, "psNcuSendUpmUpdateLinkStatus: send EV_MSG_NCU_TO_UPM_DAFLINK fail\n");
        JOB_LOC_STAT_ADDONE(qwNcuSendUpmUpdateLinkStatusFail);
        return;
    }
    //增加发送成功消息统计
    DEBUG_TRACE(DEBUG_LOW, "psNcuSendUpmUpdateLinkStatus: send EV_MSG_NCU_TO_UPM_DAFLINK success\n");
    JOB_LOC_STAT_ADDONE(qwNcuSendUpmUpdateLinkStatusSucc);
    return;
}


VOID psNcuSendToNwdafIpLinkReqProc(T_Ncu_NwdafLinkInfo *pNcuNwdafInfo, T_NcuUpfIpInfo tNcuUpfIpInfo, BYTE bLinkIPType)
{
    WORD32 dwMsgId      = EV_MSG_UCOM_NCU_JOB_TO_MCS_REQ;
    T_psNcuHeartHead tNcuHeartHead = {0};
    BYTE bMcsThr = g_ptVpfuMcsShareMem->bMcsFirstSoftThreadNo;
    DEBUG_TRACE(DEBUG_LOW, "psNcuSendToNwdafIpLinkReqProc: bMcsThr %d \n",bMcsThr);
    DEBUG_TRACE(DEBUG_LOW, "psNcuSendToNwdafIpLinkReqProc: Send LinkType %d Link Heart Beat Req\n",bLinkIPType);
    if(IPCOMM_TYPE_IPV4 == bLinkIPType)
    {
        tNcuHeartHead.tNcuAddress.bType = IPCOMM_TYPE_IPV4;
        tNcuHeartHead.tNwdafAddress.bType = IPCOMM_TYPE_IPV4;
        
        IPV4_COPY(tNcuHeartHead.tNcuAddress.abAddress,tNcuUpfIpInfo.NcuUpfIPv4);
        IPV4_COPY(tNcuHeartHead.tNwdafAddress.abAddress,pNcuNwdafInfo->NwdafIPv4);
    }
    else if(IPCOMM_TYPE_IPV6 == bLinkIPType)
    {
        tNcuHeartHead.tNcuAddress.bType = IPCOMM_TYPE_IPV6;
        tNcuHeartHead.tNwdafAddress.bType = IPCOMM_TYPE_IPV6;
        IPV6_COPY(tNcuHeartHead.tNcuAddress.abAddress,tNcuUpfIpInfo.NcuUpfIPv6);
        IPV6_COPY(tNcuHeartHead.tNwdafAddress.abAddress,pNcuNwdafInfo->NwdafIPv6);
    }
    else
    {
        return;
    }
    if(MCS_RET_SUCCESS != upfSendMsgToMcsByUcom((BYTE *)&tNcuHeartHead,dwMsgId,
                                                 sizeof(T_psNcuHeartHead) , bMcsThr))
    {
        DEBUG_TRACE(DEBUG_LOW, "psNcuSendToNwdafIpLinkReqProc: NCU SendReqMsgToMcsByUcom fail!\n");
        JOB_LOC_STAT_ADDONE(qwNcuSendToNwdafIpLinkReqByUcomFail);
        return ;
    }
    DEBUG_TRACE(DEBUG_LOW, "psNcuSendToNwdafIpLinkReqProc: NCU SendReqMsgToMcsByUcom success!\n");
    JOB_LOC_STAT_ADDONE(qwNcuSendToNwdafIpLinkReqByUcomSucc);
    return;
}



void psNcuUcomSendHeartBeatCheckReq(T_MediaProcThreadPara *ptMediaProcThreadPara, T_psNcuHeartHead *ptNcuHeartHead)
{
    MCS_CHECK_NULLPTR_RET_VOID(ptMediaProcThreadPara); 
    MCS_CHECK_NULLPTR_RET_VOID(ptMediaProcThreadPara->ptMcsStatPointer);
    MCS_CHECK_NULLPTR_RET_VOID(ptMediaProcThreadPara->ptPacket);
    MCS_CHECK_NULLPTR_RET_VOID(ptNcuHeartHead);
    
    PS_PACKET          *ptPacket         = ptMediaProcThreadPara->ptPacket;
    T_psNcuMcsPerform *ptNcuPerform = (T_psNcuMcsPerform*)ptMediaProcThreadPara->ptMcsStatPointer;
    BYTE threadID = ptPacket->bSthr % MEDIA_THRD_NUM;
    WORD32 dwResult = SUCCESS;
    BYTE *aucPacketBuf  = NULL; /* 当前mbuf实际提供的内存 */
    WORD16 wBufferLen   = 0;    /* 当前mbuf实际提供的长度 */
    WORD16 wBufferOffset= 0;    /* 当前mbuf偏移 */ 

    BYTE* buffer = &g_ptVpfuShMemVar->aucInnerAckMsg[threadID%MEDIA_THRD_NUM][0];
    WORD16 wNwdafPort = getNwdafPort();
    T_IPV6 tUpfIP = {0};
    T_IPV6 tNwdafIP = {0};

    DEBUG_TRACE(DEBUG_LOW, "psNcuUcomSendHeartBeatCheckReq enter\n");
    WORD16 datalen = psEncodeHeartReqDataProc(buffer);
    if(0 == datalen)
    {
        DEBUG_TRACE(DEBUG_LOW, "psEncodeHeartReqDataProc datalen=0\n");
        MCS_LOC_STAT_EX(ptNcuPerform, qwNcuHeartSendReqDatelenError, 1);
        return;
    }
    DEBUG_TRACE(DEBUG_LOW, "psEncodeHeartReqDataProc encode success,datalen = %d\n",datalen);
    PS_PACKET *ptNewPacket  = psCSSGenPkt(buffer, datalen, threadID);
    if(NULL == ptNewPacket)
    {
        DEBUG_TRACE(DEBUG_LOW, "ptNewPacket=NULL\n");
        MCS_LOC_STAT_EX(ptNcuPerform, qwNcuHeartSendReqPacketNull, 1);
        return;
    }
    PACKET_PROCESS_START(ptNewPacket, PKTCHK_CSS_MEDIA);
    DEBUG_TRACE(DEBUG_LOW, "psEncodeHeartReqDataProc psCSSGenPkt success\n");
    if (IPCOMM_TYPE_IPV4 == ptNcuHeartHead->tNcuAddress.bType)
    {
        IPV4_COPY(tUpfIP,ptNcuHeartHead->tNcuAddress.abAddress);
        IPV4_COPY(tNwdafIP,ptNcuHeartHead->tNwdafAddress.abAddress);

        dwResult = psMcsUDPHeadCap(ptNewPacket, wNwdafPort, wNwdafPort, datalen+8);
        _mcs_if(SUCCESS != dwResult)
        {
            DEBUG_TRACE(DEBUG_LOW,
                        "\n[Mcs]func %s: psMcsUDPHeadCap failed, dwResult = %u",
                        __FUNCTION__, dwResult);
            MCS_LOC_STAT_EX(ptNcuPerform, qwNcuHeartSendReqUDPIpv4HeadCapFail, 1);
            psCSSPktLinkBuffDesFree(ptNewPacket,PKTCHK_CSS_MEDIA);
            return;
        }
        DEBUG_TRACE(DEBUG_LOW, "psEncodeHeartReqDataProc  %d psMcsUDPHeadCap success\n",ptNcuHeartHead->tNcuAddress.bType);
        
        dwResult = psMcsIPv4HeadCap(ptNewPacket, tUpfIP, tNwdafIP, 0, MCS_UDP_PROTOCOL);
        _mcs_if(SUCCESS != dwResult)
        {
            DEBUG_TRACE(DEBUG_LOW,
                        "\n[Mcs]func %s: psMcsIPv4HeadCap failed, dwResult = %u",
                        __FUNCTION__, dwResult);
            MCS_LOC_STAT_EX(ptNcuPerform, qwNcuHeartSendReqIPv4HeadCapFail, 1);
            psCSSPktLinkBuffDesFree(ptNewPacket,PKTCHK_CSS_MEDIA);
            return;
        }
        DEBUG_TRACE(DEBUG_LOW, "psEncodeHeartReqDataProc  %d psMcsIPv4HeadCap success\n",ptNcuHeartHead->tNcuAddress.bType);

        
        if(MCS_RET_SUCCESS != psNcuUlIPV4GiRouteGet(ptNewPacket, tNwdafIP))
        {
            DEBUG_TRACE(DEBUG_LOW, "psNcuUlIPV4GiRouteGet fail\n");
            MCS_LOC_STAT_EX(ptNcuPerform, qwNcuHeartSendReqGetNwdafV4RouteFail, 1);
            psCSSPktLinkBuffDesFree(ptNewPacket, PKTCHK_CSS_MEDIA);
            return;
        }
        DEBUG_TRACE(DEBUG_LOW, "psEncodeHeartReqDataProc  %d psNcuUlIPV4GiRouteGet success\n",ptNcuHeartHead->tNcuAddress.bType);

        PS_GET_BUF_INFO(ptNewPacket->pBufferNode, aucPacketBuf, wBufferOffset, wBufferLen);
        SigTrace_NCU_Node(SIGTRACE_DIRECT_SEND, HEART_BEART_REQUEST, (aucPacketBuf + wBufferOffset), ptNewPacket->dwPktLen);
        PACKET_PROCESS_END(ptNewPacket,PKTCHK_CSS_MEDIA);//报文描述符是否有重复释放

        if (likely(CSS_OK == psNcuCSSGSUMediaSendOut(ptNewPacket)))
        {
            DEBUG_TRACE(DEBUG_LOW, "psCSSGSUMediaSendOut succ\n");
            MCS_LOC_STAT_EX(ptNcuPerform, qwNcuHeartSendReqSucc, 1);
        }
        else
        {
            DEBUG_TRACE(DEBUG_LOW, "psCSSGSUMediaSendOut fail\n");
            MCS_LOC_STAT_EX(ptNcuPerform, qwNcuHeartSendReqFail, 1);
        }
    }
    else if (IPCOMM_TYPE_IPV6 == ptNcuHeartHead->tNcuAddress.bType)
    {
        IPV6_COPY(tUpfIP,ptNcuHeartHead->tNcuAddress.abAddress);
        IPV6_COPY(tNwdafIP,ptNcuHeartHead->tNwdafAddress.abAddress);
        dwResult = psMcsUDPIpv6HeadCap(ptNewPacket, wNwdafPort, wNwdafPort, datalen+8, tUpfIP, tNwdafIP);
        _mcs_if(SUCCESS != dwResult)
        {
            DEBUG_TRACE(DEBUG_LOW,
                        "\n[Mcs]func %s: psMcsUDPIpv6HeadCap failed, dwResult = %u",
                        __FUNCTION__, dwResult);
            MCS_LOC_STAT_EX(ptNcuPerform, qwNcuHeartSendReqUDPIpv6HeadCapFail, 1);
            psCSSPktLinkBuffDesFree(ptNewPacket,PKTCHK_CSS_MEDIA);
            return;
        }
        DEBUG_TRACE(DEBUG_LOW, "psEncodeHeartReqDataProc  %d psMcsUDPIpv6HeadCap success\n",ptNcuHeartHead->tNcuAddress.bType);

        dwResult = psMcsIPv6HeadCap(ptNewPacket, tUpfIP, tNwdafIP, datalen+8, 0, MCS_UDP_PROTOCOL);
        _mcs_if(SUCCESS != dwResult)
        {
            DEBUG_TRACE(DEBUG_LOW,
                        "\n[Mcs]func %s: psMcsIPv6HeadCap failed, dwResult = %u",
                        __FUNCTION__, dwResult);
            MCS_LOC_STAT_EX(ptNcuPerform, qwNcuHeartSendReqIPv6HeadCapFail, 1);
            psCSSPktLinkBuffDesFree(ptNewPacket,PKTCHK_CSS_MEDIA);
            return;
        }
        DEBUG_TRACE(DEBUG_LOW, "psEncodeHeartReqDataProc  %d psMcsIPv6HeadCap success\n",ptNcuHeartHead->tNcuAddress.bType);
        
        if(MCS_RET_SUCCESS != psNcuUlIPV6GiRouteGet(ptNewPacket, tNwdafIP))
        {
            DEBUG_TRACE(DEBUG_LOW, "psNcuUlIPV6GiRouteGet fail\n");
            MCS_LOC_STAT_EX(ptNcuPerform, qwNcuHeartSendReqGetNwdafV6RouteFail, 1);
            psCSSPktLinkBuffDesFree(ptNewPacket, PKTCHK_CSS_MEDIA);
            return;
        }
        DEBUG_TRACE(DEBUG_LOW, "psEncodeHeartReqDataProc  %d psNcuUlIPV6GiRouteGet success\n",ptNcuHeartHead->tNcuAddress.bType);
        PS_GET_BUF_INFO(ptNewPacket->pBufferNode, aucPacketBuf, wBufferOffset, wBufferLen);
        SigTrace_NCU_Node(SIGTRACE_DIRECT_SEND, HEART_BEART_REQUEST, (aucPacketBuf + wBufferOffset), ptNewPacket->dwPktLen);
        PACKET_PROCESS_END(ptNewPacket,PKTCHK_CSS_MEDIA);

        if (likely(CSS_OK == psNcuCSSGSUMediaSendOut(ptNewPacket)))
        {
            DEBUG_TRACE(DEBUG_LOW, "psCSSGSUMediaSendOut succ\n");
            MCS_LOC_STAT_EX(ptNcuPerform, qwNcuHeartSendReqSucc, 1);
        }
        else
        {
            DEBUG_TRACE(DEBUG_LOW, "psCSSGSUMediaSendOut fail\n");
            MCS_LOC_STAT_EX(ptNcuPerform, qwNcuHeartSendReqFail, 1);
        }
    }
    
    return;
}



WORD32 psNcuSendHeartBeatReqProc()
{
 //获取本地地址
    T_NcuUpfIpInfo tNcuUpfIpInfo = {0};
    BYTE           bIsSynLinkStatus = 0;
    WORD32 IpCount= 0;
    if(MCS_RET_SUCCESS != psGetNcuUpfIp(&tNcuUpfIpInfo))
    {
        //增加统计，获取配置失败
        //全部上报告警
        DEBUG_TRACE(DEBUG_LOW, "psNcuSendHeartBeatReqProc: get Ncu upfIp fail\n");
        psNcuAllNwdafLinkSynToUpm(NCU_SEND_UPM_ALARM);
        psNcuSendNwdafIplistSyntoNcu();
        return MCS_RET_FAIL;
    }

    for(IpCount= 0; IpCount < NWDAF_LINKNUM_MAX; IpCount++ )
    {

         //增加无有效nwdaf地址，立即continue
    
         //发送次数超过重复次数
        if(g_nwdaflinkstatus[IpCount].bSendIpv4Times >= g_upfConfig.daSyslinkCfg.echofailuretimes )
        {
            //给UPM发送EV_MSG_NCU_TO_UPM_DAFLINK
            DEBUG_TRACE(DEBUG_LOW, "psNcuSendHeartBeatReqProc: Ipv4 send timer more than cfg\n");
            JOB_LOC_STAT_ADDONE(qwNcuSendIpv4TimesMoreCfg);
            if(NWDAF_LINKSTATE_DOWN != g_nwdaflinkstatus[IpCount].tNwdafAddr.bIPv4LinkStatus)
            {
                g_nwdaflinkstatus[IpCount].tNwdafAddr.bIPv4LinkStatus = NWDAF_LINKSTATE_DOWN;
                psNcuSendSingleLinkStatus(&g_nwdaflinkstatus[IpCount].tNwdafAddr);
                CHAR content[UPFLOG_MAX_CONTENTLEN] = {0};
                zte_snprintf_s(content, UPFLOG_MAX_CONTENTLEN, "psNcuSendHeartBeatReqProc: NwdafV4Address:%u.%u.%u.%u ,IPv4Link is down ,send timer more than cfg",
                                                   g_nwdaflinkstatus[IpCount].tNwdafAddr.NwdafIPv4[0], g_nwdaflinkstatus[IpCount].tNwdafAddr.NwdafIPv4[1], 
                                                   g_nwdaflinkstatus[IpCount].tNwdafAddr.NwdafIPv4[2], g_nwdaflinkstatus[IpCount].tNwdafAddr.NwdafIPv4[3]);
                upfLog(0xFF, LOGTYPE_DIAGNOSE, "upf_ncu", UPF_SYSLOG_CODE_NCU_HEARTBEAT_REQ, LOGLEVEL_LOG_ERROR, content);
                bIsSynLinkStatus = 1;
            }
            g_nwdaflinkstatus[IpCount].bSendIpv4Times = 0;
        }

        if(g_nwdaflinkstatus[IpCount].bSendIpv6Times >= g_upfConfig.daSyslinkCfg.echofailuretimes )
        {
            //给UPM发送EV_MSG_NCU_TO_UPM_DAFLINK
            DEBUG_TRACE(DEBUG_LOW, "psNcuSendHeartBeatReqProc: Ipv6 send timer more than cfg\n");
            JOB_LOC_STAT_ADDONE(qwNcuSendIpv6TimesMoreCfg);
            if(NWDAF_LINKSTATE_DOWN != g_nwdaflinkstatus[IpCount].tNwdafAddr.bIPv6LinkStatus)
            {
                g_nwdaflinkstatus[IpCount].tNwdafAddr.bIPv6LinkStatus = NWDAF_LINKSTATE_DOWN;
                psNcuSendSingleLinkStatus(&g_nwdaflinkstatus[IpCount].tNwdafAddr);
                CHAR content[UPFLOG_MAX_CONTENTLEN] = {0};
                zte_snprintf_s(content, UPFLOG_MAX_CONTENTLEN, "psNcuSendHeartBeatReqProc: NwdafV6Address:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x, IPv6Link is down , Ipv6 send timer more than cfg",
                                                         g_nwdaflinkstatus[IpCount].tNwdafAddr.NwdafIPv6[0], 
                                                         g_nwdaflinkstatus[IpCount].tNwdafAddr.NwdafIPv6[1], 
                                                         g_nwdaflinkstatus[IpCount].tNwdafAddr.NwdafIPv6[2], 
                                                         g_nwdaflinkstatus[IpCount].tNwdafAddr.NwdafIPv6[3],
                                                         g_nwdaflinkstatus[IpCount].tNwdafAddr.NwdafIPv6[4], 
                                                         g_nwdaflinkstatus[IpCount].tNwdafAddr.NwdafIPv6[5], 
                                                         g_nwdaflinkstatus[IpCount].tNwdafAddr.NwdafIPv6[6], 
                                                         g_nwdaflinkstatus[IpCount].tNwdafAddr.NwdafIPv6[7],
                                                         g_nwdaflinkstatus[IpCount].tNwdafAddr.NwdafIPv6[8], 
                                                         g_nwdaflinkstatus[IpCount].tNwdafAddr.NwdafIPv6[9], 
                                                         g_nwdaflinkstatus[IpCount].tNwdafAddr.NwdafIPv6[10], 
                                                         g_nwdaflinkstatus[IpCount].tNwdafAddr.NwdafIPv6[11],
                                                         g_nwdaflinkstatus[IpCount].tNwdafAddr.NwdafIPv6[12], 
                                                         g_nwdaflinkstatus[IpCount].tNwdafAddr.NwdafIPv6[13], 
                                                         g_nwdaflinkstatus[IpCount].tNwdafAddr.NwdafIPv6[14], 
                                                         g_nwdaflinkstatus[IpCount].tNwdafAddr.NwdafIPv6[15]);
                upfLog(0xFF, LOGTYPE_DIAGNOSE, "upf_ncu", UPF_SYSLOG_CODE_NCU_HEARTBEAT_REQ, LOGLEVEL_LOG_ERROR, content);
                bIsSynLinkStatus = 1;
            }
            g_nwdaflinkstatus[IpCount].bSendIpv6Times = 0;
        }

        g_nwdaflinkstatus[IpCount].bLinkStatus = g_nwdaflinkstatus[IpCount].tNwdafAddr.bIPv4LinkStatus || g_nwdaflinkstatus[IpCount].tNwdafAddr.bIPv6LinkStatus;

        T_Ncu_NwdafLinkInfo *pNcuNwdafInfo  =  &g_nwdaflinkstatus[IpCount].tNwdafAddr;
        switch (g_nwdaflinkstatus[IpCount].bLinkIPType)
        {
            case IPCOMM_TYPE_IPV4:
                //发送ipv4心跳链路请求
                psNcuSendToNwdafIpLinkReqProc(pNcuNwdafInfo, tNcuUpfIpInfo, IPCOMM_TYPE_IPV4);
                g_nwdaflinkstatus[IpCount].bSendIpv4Times++;
                break;
            case IPCOMM_TYPE_IPV6:
                //发送ipv6心跳链路请求
                psNcuSendToNwdafIpLinkReqProc(pNcuNwdafInfo, tNcuUpfIpInfo, IPCOMM_TYPE_IPV6);
                g_nwdaflinkstatus[IpCount].bSendIpv6Times++;
                break;
            case IPCOMM_TYPE_IPV4V6:
                //发送ipv4心跳链路请求
                psNcuSendToNwdafIpLinkReqProc(pNcuNwdafInfo, tNcuUpfIpInfo, IPCOMM_TYPE_IPV4);
                g_nwdaflinkstatus[IpCount].bSendIpv4Times++;
                //发送ipv6心跳链路请求
                psNcuSendToNwdafIpLinkReqProc(pNcuNwdafInfo, tNcuUpfIpInfo, IPCOMM_TYPE_IPV6);
                g_nwdaflinkstatus[IpCount].bSendIpv6Times++;
                break;
            default:
                break;
        }
    }

    if(bIsSynLinkStatus)
    {
        psNcuSendNwdafIplistSyntoNcu();
    }
    //一轮检测完需要全部同步给UPM
    psNcuAllNwdafLinkSynToUpm(NCU_SEND_UPM_SYN);

    return MCS_RET_SUCCESS;
}

VOID psDerictCheckUpfIpAndNwdafIpType()
{
    BYTE IpCount= 0;
    for(IpCount= 0; IpCount < NWDAF_LINKNUM_MAX; IpCount++ )
    {
        T_Ncu_NwdafLinkInfo *pNcuNwdafInfo      =  &g_nwdaflinkstatus[IpCount].tNwdafAddr;
        switch (g_nwdaflinkstatus[IpCount].bLinkIPType)
        {
            case 0:
                if(pNcuNwdafInfo->bNwdafIPType)
                {
                    psNcuSendSingleLinkStatus(pNcuNwdafInfo);
                    CHAR content[UPFLOG_MAX_CONTENTLEN] = {0};
                    zte_snprintf_s(content, UPFLOG_MAX_CONTENTLEN, "psDerictCheckUpfIpAndNwdafIpType: NwdafV4Address:%u.%u.%u.%u  NwdafV6Address:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x,IPv4LinkStatus is %d ,IPv6 LinkStatus is %d",
                                                   g_nwdaflinkstatus[IpCount].tNwdafAddr.NwdafIPv4[0], g_nwdaflinkstatus[IpCount].tNwdafAddr.NwdafIPv4[1], 
                                                   g_nwdaflinkstatus[IpCount].tNwdafAddr.NwdafIPv4[2], g_nwdaflinkstatus[IpCount].tNwdafAddr.NwdafIPv4[3],
                                                   g_nwdaflinkstatus[IpCount].tNwdafAddr.NwdafIPv6[0], g_nwdaflinkstatus[IpCount].tNwdafAddr.NwdafIPv6[1], 
                                                   g_nwdaflinkstatus[IpCount].tNwdafAddr.NwdafIPv6[2], g_nwdaflinkstatus[IpCount].tNwdafAddr.NwdafIPv6[3],
                                                   g_nwdaflinkstatus[IpCount].tNwdafAddr.NwdafIPv6[4], g_nwdaflinkstatus[IpCount].tNwdafAddr.NwdafIPv6[5], 
                                                   g_nwdaflinkstatus[IpCount].tNwdafAddr.NwdafIPv6[6], g_nwdaflinkstatus[IpCount].tNwdafAddr.NwdafIPv6[7],
                                                   g_nwdaflinkstatus[IpCount].tNwdafAddr.NwdafIPv6[8], g_nwdaflinkstatus[IpCount].tNwdafAddr.NwdafIPv6[9], 
                                                   g_nwdaflinkstatus[IpCount].tNwdafAddr.NwdafIPv6[10], g_nwdaflinkstatus[IpCount].tNwdafAddr.NwdafIPv6[11],
                                                   g_nwdaflinkstatus[IpCount].tNwdafAddr.NwdafIPv6[12], g_nwdaflinkstatus[IpCount].tNwdafAddr.NwdafIPv6[13], 
                                                   g_nwdaflinkstatus[IpCount].tNwdafAddr.NwdafIPv6[14], g_nwdaflinkstatus[IpCount].tNwdafAddr.NwdafIPv6[15],
                                                   g_nwdaflinkstatus[IpCount].tNwdafAddr.bIPv4LinkStatus,g_nwdaflinkstatus[IpCount].tNwdafAddr.bIPv6LinkStatus);
                    upfLog(0xFF, LOGTYPE_DIAGNOSE, "upf_ncu", UPF_SYSLOG_CODE_NCU_HEARTBEAT_REQ, LOGLEVEL_LOG_ERROR, content);
                    
                }
                break;
            case IPCOMM_TYPE_IPV4:
            case IPCOMM_TYPE_IPV6:
                if(g_nwdaflinkstatus[IpCount].bLinkIPType != pNcuNwdafInfo->bNwdafIPType)
                {
                     psNcuSendSingleLinkStatus(pNcuNwdafInfo);
                     CHAR content[UPFLOG_MAX_CONTENTLEN] = {0};
                     zte_snprintf_s(content, UPFLOG_MAX_CONTENTLEN, "psDerictCheckUpfIpAndNwdafIpType: NwdafV4Address:%u.%u.%u.%u  NwdafV6Address:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x,IPv4LinkStatus is %d ,IPv6 LinkStatus is %d",
                                               g_nwdaflinkstatus[IpCount].tNwdafAddr.NwdafIPv4[0], g_nwdaflinkstatus[IpCount].tNwdafAddr.NwdafIPv4[1], 
                                               g_nwdaflinkstatus[IpCount].tNwdafAddr.NwdafIPv4[2], g_nwdaflinkstatus[IpCount].tNwdafAddr.NwdafIPv4[3],
                                               g_nwdaflinkstatus[IpCount].tNwdafAddr.NwdafIPv6[0], g_nwdaflinkstatus[IpCount].tNwdafAddr.NwdafIPv6[1], 
                                               g_nwdaflinkstatus[IpCount].tNwdafAddr.NwdafIPv6[2], g_nwdaflinkstatus[IpCount].tNwdafAddr.NwdafIPv6[3],
                                               g_nwdaflinkstatus[IpCount].tNwdafAddr.NwdafIPv6[4], g_nwdaflinkstatus[IpCount].tNwdafAddr.NwdafIPv6[5], 
                                               g_nwdaflinkstatus[IpCount].tNwdafAddr.NwdafIPv6[6], g_nwdaflinkstatus[IpCount].tNwdafAddr.NwdafIPv6[7],
                                               g_nwdaflinkstatus[IpCount].tNwdafAddr.NwdafIPv6[8], g_nwdaflinkstatus[IpCount].tNwdafAddr.NwdafIPv6[9], 
                                               g_nwdaflinkstatus[IpCount].tNwdafAddr.NwdafIPv6[10], g_nwdaflinkstatus[IpCount].tNwdafAddr.NwdafIPv6[11],
                                               g_nwdaflinkstatus[IpCount].tNwdafAddr.NwdafIPv6[12], g_nwdaflinkstatus[IpCount].tNwdafAddr.NwdafIPv6[13], 
                                               g_nwdaflinkstatus[IpCount].tNwdafAddr.NwdafIPv6[14], g_nwdaflinkstatus[IpCount].tNwdafAddr.NwdafIPv6[15],
                                               g_nwdaflinkstatus[IpCount].tNwdafAddr.bIPv4LinkStatus,g_nwdaflinkstatus[IpCount].tNwdafAddr.bIPv6LinkStatus);
                     upfLog(0xFF, LOGTYPE_DIAGNOSE, "upf_ncu", UPF_SYSLOG_CODE_NCU_HEARTBEAT_REQ, LOGLEVEL_LOG_ERROR, content);

                }
                break;
            default:
                break;
        }
    }
    return;
}

VOID psNcuRecSubToHeartBeatProc(BYTE * pMsgBodySrc)
{
    if(NULL == pMsgBodySrc)
    return;

    BYTE  bGetNcuUpfIpFail  = 0;
    T_NwdafStatus  tNwdaflinkOldStatus[NWDAF_LINKNUM_MAX] = {0};
    //判断是否为0号群
    if(FALSE == ncuIsSelfGroup(0))
    {
        DEBUG_TRACE(DEBUG_LOW, "psNcuRecSubToHeartBeatProc: not 0 group ncu\n");
        return;
    }
    DEBUG_TRACE(DEBUG_LOW, "psNcuRecSubToHeartBeatProc\n");
    //1、杀死循环定时器
    psNcuKillHeartBeatReqTimer();

    //2、清空nwdafIplist全局变量，默认认为链路都是通的
    //3、获取消息中的地址列表，根据消息中的iplist列表填充全局变量
    zte_memcpy_s(tNwdaflinkOldStatus, sizeof(T_NwdafStatus)*NWDAF_LINKNUM_MAX, g_nwdaflinkstatus, sizeof(T_NwdafStatus)*NWDAF_LINKNUM_MAX);
    zte_memset_s(g_nwdaflinkstatus, sizeof(T_NwdafStatus)*NWDAF_LINKNUM_MAX , 0, sizeof(T_NwdafStatus)*NWDAF_LINKNUM_MAX);
    if(MCS_RET_SUCCESS != psGetUpmSubNwDafIplistProc(pMsgBodySrc, tNwdaflinkOldStatus, &bGetNcuUpfIpFail))
    {
        JOB_LOC_STAT_ADDONE(qwNcuGetUpmSubNwDafIplistFail);
        DEBUG_TRACE(DEBUG_LOW, "psNcuRecSubToHeartBeatProc: psGetUpmSubNwDafIplistProc fail\n");
        return;
    }

    //同步状态到NCU

    if(NCECHOSWITCH_DISABLE == g_upfConfig.daSyslinkCfg.echoswitch)
    {
        //开关关闭，不告警，也不检测
        DEBUG_TRACE(DEBUG_LOW, "psNcuRecSubToHeartBeatProc: echoswitch is close\n");
        JOB_LOC_STAT_ADDONE(qwNcuEchoSwitchClose);
        psNcuAllNwdafLinkSynToUpm(NCU_SEND_UPM_SYN);
        psNcuSendNwdafIplistSyntoNcu();
        return;
    }

    //开关打开，但获取UPFIP失败
    if(g_upfConfig.daSyslinkCfg.echoswitch && 1 == bGetNcuUpfIpFail)
    {
        DEBUG_TRACE(DEBUG_LOW, "psNcuRecSubToHeartBeatProc: echoswitch is open,but get ncu upfip fail\n");
        JOB_LOC_STAT_ADDONE(qwNcuEchoSwitchOpenButGetUpfIpFail);
        psNcuAllNwdafLinkSynToUpm(NCU_SEND_UPM_ALARM);
        psNcuSendNwdafIplistSyntoNcu();
    }

    psDerictCheckUpfIpAndNwdafIpType();
    
    //4、发送所有链路的心跳请求
    psNcuSendHeartBeatReqProc();
    //5、起循环定时器
    psNcuSetHeartBeatReqTimer();
    return;
}

WORD32 psNcuFindRspNwDafIpIsValueNum(T_psNcuHeartHead tNcuHeartHead, BYTE *pIsValueNum)
{
    BYTE bNwdafNum = 0;
    for(bNwdafNum = 0; bNwdafNum < NWDAF_LINKNUM_MAX; bNwdafNum++)
    {
        if(IPCOMM_TYPE_IPV4 == tNcuHeartHead.tNwdafAddress.bType)
        {
           if(0 == memcmp(g_nwdaflinkstatus[bNwdafNum].tNwdafAddr.NwdafIPv4, tNcuHeartHead.tNwdafAddress.abAddress, IPV4_LEN))
           {
               *pIsValueNum = bNwdafNum;
               DEBUG_TRACE(DEBUG_LOW, "find Nwdafip success order num = %u\n",bNwdafNum);
               return UPF_RET_SUCCESS;
           }
        }
        else if(IPCOMM_TYPE_IPV6 == tNcuHeartHead.tNwdafAddress.bType)
        {
            if(0 == memcmp(g_nwdaflinkstatus[bNwdafNum].tNwdafAddr.NwdafIPv6, tNcuHeartHead.tNwdafAddress.abAddress, IPV6_LEN))
            {
                *pIsValueNum = bNwdafNum;
                DEBUG_TRACE(DEBUG_LOW, "find Nwdafip order success num = %u\n",bNwdafNum);
                return UPF_RET_SUCCESS;
            }
        }
    }
    return UPF_RET_FAILED;
}

VOID psNcuRcvRspUpdateSingleLinkStatus(BYTE bIpType, BYTE bNwdafListNum)
{
    if(IPCOMM_TYPE_IPV4 == bIpType)
    {
        DEBUG_TRACE(DEBUG_LOW, "find Nwdafip bIPv4LinkStatus = %u\n",g_nwdaflinkstatus[bNwdafListNum].tNwdafAddr.bIPv4LinkStatus);
        if(NWDAF_LINKSTATE_DOWN == g_nwdaflinkstatus[bNwdafListNum].tNwdafAddr.bIPv4LinkStatus )
        {
            g_nwdaflinkstatus[bNwdafListNum].tNwdafAddr.bIPv4LinkStatus = NWDAF_LINKSTATE_UP;
            g_nwdaflinkstatus[bNwdafListNum].bLinkStatus = NWDAF_LINKSTATE_UP;
            psNcuSendSingleLinkStatus(&g_nwdaflinkstatus[bNwdafListNum].tNwdafAddr);
            CHAR content[UPFLOG_MAX_CONTENTLEN] = {0};
            zte_snprintf_s(content, UPFLOG_MAX_CONTENTLEN, "psNcuRcvRspUpdateSingleLinkStatus: NwdafV4Address:%u.%u.%u.%u ,IPv4Link restore up ",
                                                   g_nwdaflinkstatus[bNwdafListNum].tNwdafAddr.NwdafIPv4[0], g_nwdaflinkstatus[bNwdafListNum].tNwdafAddr.NwdafIPv4[1], 
                                                   g_nwdaflinkstatus[bNwdafListNum].tNwdafAddr.NwdafIPv4[2], g_nwdaflinkstatus[bNwdafListNum].tNwdafAddr.NwdafIPv4[3]);
            upfLog(0xFF, LOGTYPE_DIAGNOSE, "upf_ncu", UPF_SYSLOG_CODE_NCU_HEARTBEAT_REQ, LOGLEVEL_LOG_ERROR, content);
        }
        DEBUG_TRACE(DEBUG_LOW, "update Nwdafip bIPv4LinkStatus = %u\n",g_nwdaflinkstatus[bNwdafListNum].tNwdafAddr.bIPv4LinkStatus);
        g_nwdaflinkstatus[bNwdafListNum].bSendIpv4Times = 0;
    }
    else if(IPCOMM_TYPE_IPV6 == bIpType)
    {
        DEBUG_TRACE(DEBUG_LOW, "find Nwdafip bIPv6LinkStatus = %u\n",g_nwdaflinkstatus[bNwdafListNum].tNwdafAddr.bIPv6LinkStatus);
        if(NWDAF_LINKSTATE_DOWN == g_nwdaflinkstatus[bNwdafListNum].tNwdafAddr.bIPv6LinkStatus )
        {
            g_nwdaflinkstatus[bNwdafListNum].tNwdafAddr.bIPv6LinkStatus = NWDAF_LINKSTATE_UP;
            g_nwdaflinkstatus[bNwdafListNum].bLinkStatus = NWDAF_LINKSTATE_UP;
            psNcuSendSingleLinkStatus(&g_nwdaflinkstatus[bNwdafListNum].tNwdafAddr);
            CHAR content[UPFLOG_MAX_CONTENTLEN] = {0};
            zte_snprintf_s(content, UPFLOG_MAX_CONTENTLEN, "psNcuRcvRspUpdateSingleLinkStatus: NwdafV6Address:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x,IPv6Link restore up",
                                                         g_nwdaflinkstatus[bNwdafListNum].tNwdafAddr.NwdafIPv6[0], 
                                                         g_nwdaflinkstatus[bNwdafListNum].tNwdafAddr.NwdafIPv6[1], 
                                                         g_nwdaflinkstatus[bNwdafListNum].tNwdafAddr.NwdafIPv6[2], 
                                                         g_nwdaflinkstatus[bNwdafListNum].tNwdafAddr.NwdafIPv6[3],
                                                         g_nwdaflinkstatus[bNwdafListNum].tNwdafAddr.NwdafIPv6[4], 
                                                         g_nwdaflinkstatus[bNwdafListNum].tNwdafAddr.NwdafIPv6[5], 
                                                         g_nwdaflinkstatus[bNwdafListNum].tNwdafAddr.NwdafIPv6[6], 
                                                         g_nwdaflinkstatus[bNwdafListNum].tNwdafAddr.NwdafIPv6[7],
                                                         g_nwdaflinkstatus[bNwdafListNum].tNwdafAddr.NwdafIPv6[8], 
                                                         g_nwdaflinkstatus[bNwdafListNum].tNwdafAddr.NwdafIPv6[9], 
                                                         g_nwdaflinkstatus[bNwdafListNum].tNwdafAddr.NwdafIPv6[10], 
                                                         g_nwdaflinkstatus[bNwdafListNum].tNwdafAddr.NwdafIPv6[11],
                                                         g_nwdaflinkstatus[bNwdafListNum].tNwdafAddr.NwdafIPv6[12], 
                                                         g_nwdaflinkstatus[bNwdafListNum].tNwdafAddr.NwdafIPv6[13], 
                                                         g_nwdaflinkstatus[bNwdafListNum].tNwdafAddr.NwdafIPv6[14], 
                                                         g_nwdaflinkstatus[bNwdafListNum].tNwdafAddr.NwdafIPv6[15]);
             upfLog(0xFF, LOGTYPE_DIAGNOSE, "upf_ncu", UPF_SYSLOG_CODE_NCU_HEARTBEAT_REQ, LOGLEVEL_LOG_ERROR, content);
        }
        DEBUG_TRACE(DEBUG_LOW, "update Nwdafip bIPv6LinkStatus = %u\n",g_nwdaflinkstatus[bNwdafListNum].tNwdafAddr.bIPv6LinkStatus);
        g_nwdaflinkstatus[bNwdafListNum].bSendIpv6Times = 0;
    }
    return;
}

VOID psNcuRcvRspUpdateLinkStatus(BYTE bNwdafListNum)
{
    g_nwdaflinkstatus[bNwdafListNum].bLinkStatus = g_nwdaflinkstatus[bNwdafListNum].tNwdafAddr.bIPv4LinkStatus || g_nwdaflinkstatus[bNwdafListNum].tNwdafAddr.bIPv6LinkStatus;
    return;
}

WORD32 psNcuGetNwdafRspIpProc (BYTE *pMsgBodySrc, T_psNcuHeartHead *ptNcuHeartHead)
{
    PS_NCU_NULL_CHECK_RET(pMsgBodySrc, MCS_RET_FAIL);
    PS_NCU_NULL_CHECK_RET(ptNcuHeartHead, MCS_RET_FAIL);
    T_psMcsIPv4Head *ptIPv4H = NULL;
    T_psMcsIPv6Head *ptIPv6H = NULL;
    BYTE    ucHeadLen        = 0;
    BYTE    bIPType        = 0;

    bIPType = *(pMsgBodySrc) >>4;

    DEBUG_TRACE(DEBUG_LOW,"\n psNcuGetNwdafRspIpProc get bIPType = %d  \n",bIPType);
    if(MCS_IP_TECH_IPV4   ==  bIPType)
    {
        ptIPv4H = (T_psMcsIPv4Head *)(pMsgBodySrc);
        ucHeadLen = (ptIPv4H->btHeadLen) << 2;

        /* 报文描述符长度校验 */
        if(unlikely(ucHeadLen < MCS_CAP_IPV4_HEAD_LEN))
        {
            return MCS_RET_FAIL;
        }

        ptNcuHeartHead->tNwdafAddress.bType = IPCOMM_TYPE_IPV4;
        IPV4_COPY(ptNcuHeartHead->tNwdafAddress.abAddress,ptIPv4H->tSrcIp);
    }
    else if (MCS_IP_TECH_IPV6   ==  bIPType)
    {
        /*解析IPͷ*/

        ptIPv6H = (T_psMcsIPv6Head *)(pMsgBodySrc);

        ptNcuHeartHead->tNwdafAddress.bType = IPCOMM_TYPE_IPV6;
        IPV6_COPY(ptNcuHeartHead->tNwdafAddress.abAddress,ptIPv6H->tSrcIp);
    }
    else
    {
        return MCS_RET_FAIL;
    }
    return MCS_RET_SUCCESS;
}

VOID psNcuRcvNwdafRspPktProc(BYTE *pMsgBodySrc)
{
    PS_NCU_NULL_CHECK_VOID(pMsgBodySrc);
    BYTE  bIsValueNum = 0;

    T_psNcuHeartHead tNcuHeartHead = {0};
    if(UPF_RET_SUCCESS != psNcuGetNwdafRspIpProc (pMsgBodySrc, &tNcuHeartHead))
    {
        return;
    }

    if(IPCOMM_TYPE_IPV4 == tNcuHeartHead.tNwdafAddress.bType)
    {
        DEBUG_TRACE(DEBUG_LOW,"psNcuRcvNwdafRspPktProc: Nwdafip=%u.%u.%u.%u\n",tNcuHeartHead.tNwdafAddress.abAddress[0],
                                                                               tNcuHeartHead.tNwdafAddress.abAddress[1],
                                                                               tNcuHeartHead.tNwdafAddress.abAddress[2],
                                                                               tNcuHeartHead.tNwdafAddress.abAddress[3]);
    }
    else if(IPCOMM_TYPE_IPV6 == tNcuHeartHead.tNwdafAddress.bType)
    {
        DEBUG_TRACE(DEBUG_LOW,"psNcuRcvNwdafRspPktProc: Nwdafip=%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x \n",
                                                                               tNcuHeartHead.tNwdafAddress.abAddress[0],
                                                                               tNcuHeartHead.tNwdafAddress.abAddress[1],
                                                                               tNcuHeartHead.tNwdafAddress.abAddress[2],
                                                                               tNcuHeartHead.tNwdafAddress.abAddress[3],
                                                                               tNcuHeartHead.tNwdafAddress.abAddress[4],
                                                                               tNcuHeartHead.tNwdafAddress.abAddress[5],
                                                                               tNcuHeartHead.tNwdafAddress.abAddress[6],
                                                                               tNcuHeartHead.tNwdafAddress.abAddress[7],
                                                                               tNcuHeartHead.tNwdafAddress.abAddress[8],
                                                                               tNcuHeartHead.tNwdafAddress.abAddress[9],
                                                                               tNcuHeartHead.tNwdafAddress.abAddress[10],
                                                                               tNcuHeartHead.tNwdafAddress.abAddress[11],
                                                                               tNcuHeartHead.tNwdafAddress.abAddress[12],
                                                                               tNcuHeartHead.tNwdafAddress.abAddress[13],
                                                                               tNcuHeartHead.tNwdafAddress.abAddress[14],
                                                                               tNcuHeartHead.tNwdafAddress.abAddress[15]);
    }
    else
    {
        return;
    }
    
    if (UPF_RET_SUCCESS == psNcuFindRspNwDafIpIsValueNum(tNcuHeartHead, &bIsValueNum))
    {
        //若历史状态为DOWN,则给UPM发起告警恢复消息
        psNcuRcvRspUpdateSingleLinkStatus(tNcuHeartHead.tNwdafAddress.bType, bIsValueNum);
        psNcuRcvRspUpdateLinkStatus(bIsValueNum);
    }
    return;
}

void psNcuHeartRsqProc(BYTE* msg, WORD32 msgLen)
{
    if(!(ncuIsSelfGroup(0)))
    {
        WORD32 commId = ncuGetGroupCommId(0);

        if(0 != upfSendMsgToJob((BYTE*)msg, msgLen, EV_MSG_NWDAF_TO_NCU_PKT, commId, JOB_TYPE_MCS_MANAGE))
        {
            JOB_LOC_STAT_ADDONE(qwNcuHeartRcvRspSendOtherNcuFail);
            return;
        }
        JOB_LOC_STAT_ADDONE(qwNcuHeartRcvRspSendOtherNcuSucc);
        return;
    }
    psNcuRcvNwdafRspPktProc(msg);
    return;
}

void psNcuHeartRspToJob(T_MediaProcThreadPara *ptMediaProcThreadPara)
{
    MCS_CHECK_NULLPTR_RET_VOID(ptMediaProcThreadPara); 
    MCS_CHECK_NULLPTR_RET_VOID(ptMediaProcThreadPara->ptMcsStatPointer);
    MCS_CHECK_NULLPTR_RET_VOID(ptMediaProcThreadPara->ptPacket);
    T_psNcuMcsPerform *ptNcuPerform = (T_psNcuMcsPerform *)ptMediaProcThreadPara->ptMcsStatPointer;
    PS_PACKET *ptPacket = ptMediaProcThreadPara->ptPacket;
    
    ptMediaProcThreadPara->ucFreePktFlag = MCS_NONEED_FREEPKT;
    if(CSS_OK != ncuUcomCopySendToJob(ptPacket, JOB_TYPE_MCS_MANAGE, EV_MSG_UCOM_MCS_TO_JOB_RSQ))
    {
        MCS_LOC_STAT_EX(ptNcuPerform, qwNcuHeartUcomCopySendRspFail, 1);
    }
    else
    {
        MCS_LOC_STAT_EX(ptNcuPerform, qwNcuHeartUcomCopySendRspSucc, 1);
    }

    return;
}

WORD16 psEncodeHeartRspDataProc(BYTE* buffer)
{
    T_ExpDataHead* ptExpDataHead = (void*)buffer;
    T_RecovertTimeStampId tNcuHeartData = {0};
    tNcuHeartData.Base.type = mcs_ntohs(IETYPE_Recovery_Time_Stamp);
    tNcuHeartData.Base.length = mcs_ntohs(4);
    tNcuHeartData.dwTimeStamp = mcs_ntohl(psFtmGetPowerOnSec());
    WORD16 wDataLen = sizeof(T_RecovertTimeStampId);

    zte_memcpy_s(buffer +sizeof(T_ExpDataHead),wDataLen,&tNcuHeartData,wDataLen);
    PS_FILL_EXP_HEAD_TO_NWDAF(ptExpDataHead, HEART_BREAT_RESPONSE, mcs_ntohs(wDataLen));
    BUFF_TRACE(DEBUG_SND, buffer, wDataLen+sizeof(T_ExpDataHead));
    return wDataLen+sizeof(T_ExpDataHead);
}

WORD16 psEncodeHeartReqDataProc(BYTE* buffer)
{
    T_ExpDataHead* ptExpDataHead = (void*)buffer;
    T_RecovertTimeStampId tNcuHeartData = {0};
    tNcuHeartData.Base.type = mcs_ntohs(IETYPE_Recovery_Time_Stamp);
    tNcuHeartData.Base.length = mcs_ntohs(4);
    tNcuHeartData.dwTimeStamp = mcs_ntohl(psFtmGetPowerOnSec());
    WORD16 wDataLen = sizeof(T_RecovertTimeStampId);

    zte_memcpy_s(buffer +sizeof(T_ExpDataHead),wDataLen,&tNcuHeartData,wDataLen);
    PS_FILL_EXP_HEAD_TO_NWDAF(ptExpDataHead, HEART_BEART_REQUEST, mcs_ntohs(wDataLen));
    BUFF_TRACE(DEBUG_SND, buffer, wDataLen+sizeof(T_ExpDataHead));
    return wDataLen+sizeof(T_ExpDataHead);
}


void psNcuHeartReqProc(T_MediaProcThreadPara *ptMediaProcThreadPara, T_psNcuHeartHead *ptNcuHeartHead)
{
    MCS_CHECK_NULLPTR_RET_VOID(ptMediaProcThreadPara); 
    MCS_CHECK_NULLPTR_RET_VOID(ptMediaProcThreadPara->ptMcsStatPointer);
    MCS_CHECK_NULLPTR_RET_VOID(ptMediaProcThreadPara->ptPacket);
    MCS_CHECK_NULLPTR_RET_VOID(ptNcuHeartHead);
    
    PS_PACKET          *ptPacket         = ptMediaProcThreadPara->ptPacket;
    T_psNcuMcsPerform *ptNcuPerform = (T_psNcuMcsPerform*)ptMediaProcThreadPara->ptMcsStatPointer;
    BYTE threadID = ptPacket->bSthr % MEDIA_THRD_NUM;
    WORD32 dwResult = SUCCESS;
    BYTE *aucPacketBuf  = NULL; /* 当前mbuf实际提供的内存 */
    WORD16 wBufferLen   = 0;    /* 当前mbuf实际提供的长度 */
    WORD16 wBufferOffset= 0;    /* 当前mbuf偏移 */

    BYTE* buffer = &g_ptVpfuShMemVar->aucInnerAckMsg[threadID%MEDIA_THRD_NUM][0];
    WORD16 wNwdafPort = getNwdafPort();
    T_IPV6 tUpfIP = {0};
    T_IPV6 tNwdafIP = {0};
    
    WORD16 datalen = psEncodeHeartRspDataProc(buffer);
    if(0 == datalen)
    {
        MCS_LOC_STAT_EX(ptNcuPerform, qwNcuHeartEncodeRspFail, 1);
        DEBUG_TRACE(DEBUG_LOW, "psEncodeHeartRspDataProc datalen=0\n");
        return;
    }
    PS_PACKET *ptNewPacket  = psCSSGenPkt(buffer, datalen, threadID);
    if(NULL == ptNewPacket)
    {
        MCS_LOC_STAT_EX(ptNcuPerform, qwNcuHeartGenPacketFail, 1);
        DEBUG_TRACE(DEBUG_LOW, "ptNewPacket=NULL\n");
        return;
    }
    PACKET_PROCESS_START(ptNewPacket, PKTCHK_CSS_MEDIA);
    if (IPCOMM_TYPE_IPV4 == ptNcuHeartHead->tNcuAddress.bType)
    {
        IPV4_COPY(tUpfIP,ptNcuHeartHead->tNcuAddress.abAddress);
        IPV4_COPY(tNwdafIP,ptNcuHeartHead->tNwdafAddress.abAddress);

        dwResult = psMcsUDPHeadCap(ptNewPacket, wNwdafPort, wNwdafPort, datalen+8);
        _mcs_if(SUCCESS != dwResult)
        {
            DEBUG_TRACE(DEBUG_LOW,
                        "\n[Mcs]func %s: psMcsUDPHeadCap failed, dwResult = %u",
                        __FUNCTION__, dwResult);
            MCS_LOC_STAT_EX(ptNcuPerform, qwNcuHeartUDPHeadCapFail, 1);
            psCSSPktLinkBuffDesFree(ptNewPacket, PKTCHK_CSS_MEDIA);
            return;
        }
        
        dwResult = psMcsIPv4HeadCap(ptNewPacket, tUpfIP, tNwdafIP, 0, MCS_UDP_PROTOCOL);
        _mcs_if(SUCCESS != dwResult)
        {
            DEBUG_TRACE(DEBUG_LOW,
                        "\n[Mcs]func %s: psMcsIPv4HeadCap failed, dwResult = %u",
                        __FUNCTION__, dwResult);
            MCS_LOC_STAT_EX(ptNcuPerform, qwNcuHeartIPv4HeadCapFail, 1);
            psCSSPktLinkBuffDesFree(ptNewPacket, PKTCHK_CSS_MEDIA);
            return;
        }

        if(MCS_RET_SUCCESS != psNcuUlIPV4GiRouteGet(ptNewPacket, tNwdafIP))
        {
            DEBUG_TRACE(DEBUG_LOW, "psNcuUlIPV4GiRouteGet fail\n");
            MCS_LOC_STAT_EX(ptNcuPerform, qwNcuHeartGetNwdafV4RouteFail, 1);
            psCSSPktLinkBuffDesFree(ptNewPacket, PKTCHK_CSS_MEDIA);
            return;
        }

        PS_GET_BUF_INFO(ptNewPacket->pBufferNode, aucPacketBuf, wBufferOffset, wBufferLen);
        SigTrace_NCU_Node(SIGTRACE_DIRECT_SEND, HEART_BREAT_RESPONSE, (aucPacketBuf + wBufferOffset), ptNewPacket->dwPktLen);
        PACKET_PROCESS_END(ptNewPacket,PKTCHK_CSS_MEDIA);

        if (likely(CSS_OK == psNcuCSSGSUMediaSendOut(ptNewPacket)))
        {
            DEBUG_TRACE(DEBUG_LOW, "psCSSDPDKSendOut succ\n");
            MCS_LOC_STAT_EX(ptNcuPerform, qwNcuHeartSendRspSucc, 1);
        }
        else
        {
            DEBUG_TRACE(DEBUG_LOW, "psCSSDPDKSendOut fail\n");
            MCS_LOC_STAT_EX(ptNcuPerform, qwNcuHeartSendRspFail, 1);
        }
    }
    else if (IPCOMM_TYPE_IPV6 == ptNcuHeartHead->tNcuAddress.bType)
    {
        IPV6_COPY(tUpfIP,ptNcuHeartHead->tNcuAddress.abAddress);
        IPV6_COPY(tNwdafIP,ptNcuHeartHead->tNwdafAddress.abAddress);
        dwResult = psMcsUDPIpv6HeadCap(ptNewPacket, wNwdafPort, wNwdafPort, datalen+8, tUpfIP, tNwdafIP);
        _mcs_if(SUCCESS != dwResult)
        {
            DEBUG_TRACE(DEBUG_LOW,
                        "\n[Mcs]func %s: psMcsUDPIpv6HeadCap failed, dwResult = %u",
                        __FUNCTION__, dwResult);
            MCS_LOC_STAT_EX(ptNcuPerform, qwNcuHeartUDPHeadCapFail, 1);
            psCSSPktLinkBuffDesFree(ptNewPacket, PKTCHK_CSS_MEDIA);
            return;
        }
        
        dwResult = psMcsIPv6HeadCap(ptNewPacket, tUpfIP, tNwdafIP, datalen+8, 0, MCS_UDP_PROTOCOL);
        _mcs_if(SUCCESS != dwResult)
        {
            DEBUG_TRACE(DEBUG_LOW, "psMcsIPv6HeadCap failed, dwResult = %u", dwResult);
            MCS_LOC_STAT_EX(ptNcuPerform, qwNcuHeartIPv6HeadCapFail, 1);
            psCSSPktLinkBuffDesFree(ptNewPacket, PKTCHK_CSS_MEDIA);
            return;
        }

        if(MCS_RET_SUCCESS != psNcuUlIPV6GiRouteGet(ptNewPacket, tNwdafIP))
        {
            DEBUG_TRACE(DEBUG_LOW, "psNcuUlIPV6GiRouteGet fail\n");
            MCS_LOC_STAT_EX(ptNcuPerform, qwNcuHeartGetNwdafV6RouteFail, 1);
            psCSSPktLinkBuffDesFree(ptNewPacket, PKTCHK_CSS_MEDIA);
            return;
        }
        PS_GET_BUF_INFO(ptNewPacket->pBufferNode, aucPacketBuf, wBufferOffset, wBufferLen);
        SigTrace_NCU_Node(SIGTRACE_DIRECT_SEND, HEART_BREAT_RESPONSE, (aucPacketBuf + wBufferOffset), ptNewPacket->dwPktLen);
        PACKET_PROCESS_END(ptNewPacket,PKTCHK_CSS_MEDIA);

        if (likely(CSS_OK == psNcuCSSGSUMediaSendOut(ptNewPacket)))
        {
            DEBUG_TRACE(DEBUG_LOW, "psCSSDPDKSendOut succ\n");
            MCS_LOC_STAT_EX(ptNcuPerform, qwNcuHeartSendRspSucc, 1);
        }
        else
        {
            DEBUG_TRACE(DEBUG_LOW, "psCSSDPDKSendOut fail\n");
            MCS_LOC_STAT_EX(ptNcuPerform, qwNcuHeartSendRspFail, 1);
        }
    }
    
    return;
}

WORD32 psNcuCheckHeartMsg(T_MediaProcThreadPara *ptMediaProcThreadPara, T_ExpDataHead* ptExpDataHead, T_RecovertTimeStampId *ptNcuHeartData)
{
    PS_PACKET          *ptPacket         = ptMediaProcThreadPara->ptPacket;
    T_psNcuMcsPerform *ptNcuPerform = (T_psNcuMcsPerform*)ptMediaProcThreadPara->ptMcsStatPointer;
    BYTE  *aucPacketBuf        = NULL; /* 当前mbuf实际提供的内存 */ 
    WORD16 wBufferLen          = 0;    /* 当前mbuf实际提供的长度 */ 
    WORD16 wBufferOffset       = 0;    /* 当前mbuf偏移 */ 

    PS_GET_BUF_INFO(ptPacket->pBufferNode, aucPacketBuf, wBufferOffset, wBufferLen);

    DEBUG_TRACE(DEBUG_LOW,"\n [Ncu]func ptExpDataHead->Length=0x%04x\n" , ptExpDataHead->Length);
    DEBUG_TRACE(DEBUG_LOW,"\n [Ncu]func ptNcuHeartData->Base.type=0x%04xu\n", ptNcuHeartData->Base.type);
    DEBUG_TRACE(DEBUG_LOW,"\n [Ncu]func ptExpDataHead->Length=0x%04x\n", mcs_htons(ptExpDataHead->Length));
    DEBUG_TRACE(DEBUG_LOW,"\n [Ncu]func ptNcuHeartData->Base.type=0x%04x\n", mcs_htons(ptNcuHeartData->Base.type));
    if (mcs_htons(ptExpDataHead->Length) < sizeof(T_RecovertTimeStampId))
    {
        DEBUG_TRACE(DEBUG_LOW,"\n [Ncu]func ptExpDataHead->Length=%u\n", ptExpDataHead->Length);
        MCS_LOC_STAT_EX(ptNcuPerform, qwNcuHeartMsgCheckFail, 1);
        return MCS_RET_FAIL;
    }
    
    if (IETYPE_Recovery_Time_Stamp != mcs_htons(ptNcuHeartData->Base.type))
    {
        DEBUG_TRACE(DEBUG_LOW,"\n [Ncu]func ptNcuHeartData->Base.type=%u\n", ptNcuHeartData->Base.type);
        MCS_LOC_STAT_EX(ptNcuPerform, qwNcuHeartMsgTypeCheckFail, 1);
        return MCS_RET_FAIL;
    }

    return MCS_RET_SUCCESS;
}

WORD32 psNcuIpUdpDecap(T_MediaProcThreadPara *ptMediaProcThreadPara, T_psNcuHeartHead *ptNcuHeartHead, WORD16 *pwBufferOffset)
{
    BYTE   *aucPacketBuf  = NULL; /* 当前mbuf实际提供的内存 */
    T_psMcsIPv4Head *ptIPv4H = NULL;
    T_psMcsIPv6Head *ptIPv6H = NULL;
    T_psMcsUdpHead *ptUdpH = NULL;
    PS_PACKET* ptPacket = NULL;
    WORD16  wBufferLen    = 0;    /* 当前mbuf实际提供的长度 */
    WORD16  wBufferOffset = 0;    /* 当前mbuf偏移 */
    BYTE    ucHeadLen        = 0;
    BYTE    bIPType        = 0;


    ptPacket = (PS_PACKET* )ptMediaProcThreadPara->ptPacket;

    PS_GET_BUF_INFO(ptPacket->pBufferNode, aucPacketBuf, wBufferOffset, wBufferLen);
    DEBUG_TRACE(DEBUG_LOW,"\n [Ncu]  psNcuIpUdpDecap start  *pwBufferOffset =%u\n", wBufferOffset);
    bIPType = *(aucPacketBuf + wBufferOffset) >>4;
    if(MCS_IP_TECH_IPV4   ==  bIPType)
    {
        if (unlikely(wBufferLen < ( MCS_CAP_IPV4_HEAD_LEN + MCS_CAP_UDP_HEAD_LEN )))
        {
            return MCS_RET_FAIL;
        }

        ptIPv4H = (T_psMcsIPv4Head *)(aucPacketBuf + wBufferOffset );

        ucHeadLen = (ptIPv4H->btHeadLen) << 2;

        /* 报文描述符长度校验 */
        if(unlikely(ucHeadLen < MCS_CAP_IPV4_HEAD_LEN))
        {
            return MCS_RET_FAIL;
        }

        ptNcuHeartHead->tNwdafAddress.bType = IPCOMM_TYPE_IPV4;
        IPV4_COPY(ptNcuHeartHead->tNwdafAddress.abAddress,ptIPv4H->tSrcIp);
        ptNcuHeartHead->tNcuAddress.bType = IPCOMM_TYPE_IPV4;
        IPV4_COPY(ptNcuHeartHead->tNcuAddress.abAddress,ptIPv4H->tDstIp);
    }
    else
    {
        /*解析IPͷ*/
        if (unlikely(wBufferLen < ( MCS_CAP_IPV6_HEAD_LEN + MCS_CAP_UDP_HEAD_LEN )))
        {
            return MCS_RET_FAIL;
        }

        ptIPv6H = (T_psMcsIPv6Head *)(aucPacketBuf + wBufferOffset );

        ptNcuHeartHead->tNwdafAddress.bType = IPCOMM_TYPE_IPV6;
        IPV6_COPY(ptNcuHeartHead->tNwdafAddress.abAddress,ptIPv6H->tSrcIp);
        ptNcuHeartHead->tNcuAddress.bType = IPCOMM_TYPE_IPV6;
        IPV6_COPY(ptNcuHeartHead->tNcuAddress.abAddress,ptIPv6H->tDstIp);

        ucHeadLen = MCS_CAP_IPV6_HEAD_LEN;
    }

    /*UDP头解析*/
    ptUdpH = (T_psMcsUdpHead *)(aucPacketBuf + wBufferOffset + ucHeadLen);
    ptNcuHeartHead->wNwdafPort = mcs_ntohs(ptUdpH->wSrcPort);/*记录下源端口号，回响应的时候使用*/
    ptNcuHeartHead->wNcuPort = mcs_ntohs(ptUdpH->wDstPort);

    /* 更新描述符偏移和长度 */
    *pwBufferOffset += ucHeadLen + MCS_CAP_UDP_HEAD_LEN ;
    DEBUG_TRACE(DEBUG_LOW,"\n [Ncu]  psNcuIpUdpDecap end  *pwBufferOffset =%u\n", *pwBufferOffset);

    PS_SET_BUF_INFO(ptPacket, ptPacket->pBufferNode, wBufferOffset,wBufferLen);

    return MCS_RET_SUCCESS;
}

WORD32 psFindNwdafIpListAdrr(T_IPComm tNwdafAddress)
{
    BYTE IpCount = 0;
    //在订阅地址比较
     for(IpCount= 0; IpCount < NWDAF_LINKNUM_MAX; IpCount++ )
     {
          T_Ncu_NwdafLinkInfo *pNwdafInfo      =  &g_nwdaflinkstatus[IpCount].tNwdafAddr;
         if(IPCOMM_TYPE_IPV4 == tNwdafAddress.bType)
         {
             if(0 == memcmp(tNwdafAddress.abAddress, pNwdafInfo->NwdafIPv4, IPV4_LEN))
             {
                 return MCS_RET_SUCCESS;
             }
             continue;
         }
          else if(IPCOMM_TYPE_IPV6 == tNwdafAddress.bType)
         {
              if(0 == memcmp(tNwdafAddress.abAddress, pNwdafInfo->NwdafIPv6, IPV6_LEN))
              {
                   return MCS_RET_SUCCESS;
              }
              continue;
         }
     }
     return MCS_RET_FAIL;
}


VOID psNcuHeartPktProc(T_MediaProcThreadPara *ptMediaProcThreadPara)
{
    DEBUG_TRACE(DEBUG_LOW, "psNcuHeartPktProc enter \n");
    MCS_CHECK_NULLPTR_RET_VOID(ptMediaProcThreadPara); 
    MCS_CHECK_NULLPTR_RET_VOID(ptMediaProcThreadPara->ptMcsStatPointer);
    MCS_CHECK_NULLPTR_RET_VOID(ptMediaProcThreadPara->ptPacket);

    PS_PACKET          *ptPacket         = ptMediaProcThreadPara->ptPacket;
    T_psNcuMcsPerform *ptNcuPerform = (T_psNcuMcsPerform*)ptMediaProcThreadPara->ptMcsStatPointer;

    WORD32 dwResult = SUCCESS;
    T_psNcuHeartHead tNcuHeartHead = {0};
    WORD16  wBufferIpUdpOffset = 0;

    dwResult = psNcuIpUdpDecap(ptMediaProcThreadPara,&tNcuHeartHead,&wBufferIpUdpOffset);
    _mcs_if(SUCCESS != dwResult)
    {
        DEBUG_TRACE(DEBUG_LOW, "\n[Ncu]: psNcuIpUdpDecap failed, dwResult = %u", dwResult);
        MCS_LOC_STAT_EX(ptNcuPerform, qwNcuHeartIpUdpDecapFail, 1);
        return;
    }

    BYTE  *aucPacketBuf        = NULL; /* 当前mbuf实际提供的内存 */ 
    WORD16 wBufferLen          = 0;    /* 当前mbuf实际提供的长度 */ 
    WORD16 wBufferOffset       = 0;    /* 当前mbuf偏移 */ 

    PS_GET_BUF_INFO(ptPacket->pBufferNode, aucPacketBuf, wBufferOffset, wBufferLen);
    DEBUG_TRACE(DEBUG_LOW,"\n [Ncu] wBufferLen = %u , sizeof(T_ExpDataHead) = %lu,wBufferIpUdpOffset = %u !\n", wBufferLen , sizeof(T_ExpDataHead),wBufferIpUdpOffset);
    MCS_CHECK_NULLPTR_RET_VOID(aucPacketBuf);

    if (MCS_RET_SUCCESS != psNcuCheckHeartbeat(&tNcuHeartHead, wBufferLen, wBufferIpUdpOffset))
    {
        return ;
    }

    T_ExpDataHead* ptExpDataHead = (T_ExpDataHead*)(aucPacketBuf + wBufferIpUdpOffset + wBufferOffset);
    MCS_CHECK_NULLPTR_RET_VOID(ptExpDataHead); 

    DEBUG_TRACE(DEBUG_LOW,"\n [Ncu]func ptExpDataHead->MessageType=%u, sizeof(T_ExpDataHead) = %lu \n",  ptExpDataHead->MessageType,sizeof(T_ExpDataHead));
    if (HEART_BEART_REQUEST == ptExpDataHead->MessageType)
    {
        T_RecovertTimeStampId *ptNcuHeartData = (T_RecovertTimeStampId *)(aucPacketBuf + wBufferIpUdpOffset + sizeof(T_ExpDataHead)+wBufferOffset);
        MCS_CHECK_NULLPTR_RET_VOID(ptNcuHeartData);
        dwResult = psNcuCheckHeartMsg(ptMediaProcThreadPara,ptExpDataHead,ptNcuHeartData);
        _mcs_if(MCS_RET_SUCCESS != dwResult)
        {
            return;
        }
        SigTrace_NCU_Node(SIGTRACE_DIRECT_RECV, HEART_BEART_REQUEST, (aucPacketBuf + wBufferOffset), ptPacket->dwPktLen);
        MCS_LOC_STAT_EX(ptNcuPerform, qwNcuHeartRcvReq, 1);
        
        if(NCACKPOLICY_ACKSUB == g_upfConfig.daSyslinkCfg.echoackpolicy && MCS_RET_SUCCESS != psFindNwdafIpListAdrr(tNcuHeartHead.tNwdafAddress))
        {
            MCS_LOC_STAT_EX(ptNcuPerform, qwNcuHeartRcvReqNoNwdaflList, 1);
            DEBUG_TRACE(DEBUG_LOW, "g_upfConfig.daSyslinkCfg.echoackpolicy = %u\n", g_upfConfig.daSyslinkCfg.echoackpolicy);
            return;
        }
        psNcuHeartReqProc(ptMediaProcThreadPara,&tNcuHeartHead);
    }
    else if (HEART_BREAT_RESPONSE == ptExpDataHead->MessageType)
    {
        T_RecovertTimeStampId *ptNcuHeartData = (T_RecovertTimeStampId *)(aucPacketBuf + wBufferIpUdpOffset + sizeof(ptExpDataHead));
        MCS_CHECK_NULLPTR_RET_VOID(ptNcuHeartData);
        SigTrace_NCU_Node(SIGTRACE_DIRECT_RECV, HEART_BREAT_RESPONSE, (aucPacketBuf + wBufferOffset), ptPacket->dwPktLen);
        MCS_LOC_STAT_EX(ptNcuPerform, qwNcuHeartRcvRsp, 1);
        psNcuHeartRspToJob(ptMediaProcThreadPara);
    }
    else
    {
        MCS_LOC_STAT_EX(ptNcuPerform, qwNcuHeartNoRcvMsg, 1);
    }
    
    return;
}

WORD32 psNcuCheckHeartbeat(T_psNcuHeartHead* ptNcuHeartHead, WORD16 wBufferLen, WORD16 wBufferIpUdpOffset)
{
    MCS_CHK_RET(NULL == ptNcuHeartHead, MCS_RET_FAIL); 
   if (IPCOMM_TYPE_IPV4 == ptNcuHeartHead->tNcuAddress.bType)
    {
        MCS_CHK_RET(wBufferLen < sizeof(T_ExpDataHead)+wBufferIpUdpOffset, MCS_RET_FAIL);
        /* 目的地址检查 */
        T_IPV4 expectDstIpv4 = {0};
        inet_pton(AF_INET,  g_upfConfig.daUpfIpCfg.upfIpv4, expectDstIpv4);
        WORD32 expectDstIp = MCS_IP4_ARR_TO_DWORD(expectDstIpv4);
        WORD32 recvDstIp   = MCS_IP4_ARR_TO_DWORD(ptNcuHeartHead->tNcuAddress.abAddress);
        DEBUG_TRACE(DEBUG_LOW,"\n [Ncu]func dspIP(exp:%x;rcv:%x)!\n", expectDstIp, recvDstIp);
        MCS_CHK_RET(expectDstIp != recvDstIp, MCS_RET_FAIL);
    }
    else if (IPCOMM_TYPE_IPV6 == ptNcuHeartHead->tNcuAddress.bType)
    {
        MCS_CHK_RET(wBufferLen < sizeof(T_ExpDataHead)+wBufferIpUdpOffset, MCS_RET_FAIL);
        /* 目的地址检查 */
        T_IPV6 expectDstIpv6 = {0};
        inet_pton(AF_INET6, g_upfConfig.daUpfIpCfg.upfIpv6, expectDstIpv6);
        DEBUG_TRACE(DEBUG_LOW,"\n [Ncu]func dspIP(exp:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x;rcv:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x)!\n", 
                              MCS_LOG_IPV6_FIELD(expectDstIpv6),
                              MCS_LOG_IPV6_FIELD(ptNcuHeartHead->tNcuAddress.abAddress));
        if(0 != memcmp(expectDstIpv6 , ptNcuHeartHead->tNcuAddress.abAddress, IPV6_LEN))
        {
            return MCS_RET_FAIL;
        }
    }
    else
    {
        return MCS_RET_FAIL;
    }
    
    /* 源端口号检查 */
    DEBUG_TRACE(DEBUG_LOW,"\n [Ncu]func srcPort(exp:%u;rcv:%u)!\n", g_upfConfig.daSyslinkCfg.udpport, ptNcuHeartHead->wNwdafPort);
    MCS_CHK_RET(g_upfConfig.daSyslinkCfg.udpport != ptNcuHeartHead->wNwdafPort, MCS_RET_FAIL);

    /* 目的端口号检查 */
    DEBUG_TRACE(DEBUG_LOW,"\n [Ncu]func dstPort(exp:%u;rcv:%u)!\n", g_upfConfig.daSyslinkCfg.udpport, ptNcuHeartHead->wNcuPort);
    MCS_CHK_RET(g_upfConfig.daSyslinkCfg.udpport != ptNcuHeartHead->wNcuPort, MCS_RET_FAIL);

    /*vrfid 检查*/

    return MCS_RET_SUCCESS;
}

BYTE g_NcuLogFlg = 0;
/* NCU Log 测试函数 */
VOID PsNcuSystemLogTest()
{
    CHAR content[UPFLOG_MAX_CONTENTLEN] = {0};
    if(1 == g_NcuLogFlg)
    {
        /* 测试普通上报1 error diagnose */
        zte_snprintf_s(content, UPFLOG_MAX_CONTENTLEN, "[PsNcuSystemLogTest][LOGLEVEL_LOG_ERROR LOGTYPE_DIAGNOSE Test1!!!]");
        upfLog(0xFF, LOGTYPE_DIAGNOSE, "upf_ncu", UPF_SYSLOG_CODE_NCU_HEARTBEAT_REQ, LOGLEVEL_LOG_ERROR, content);
        /* 测试普通上报2 error diagnose */
        zte_memset_s(content, UPFLOG_MAX_CONTENTLEN , 0, UPFLOG_MAX_CONTENTLEN);
        zte_snprintf_s(content, UPFLOG_MAX_CONTENTLEN, "[PsNcuSystemLogTest][LOGLEVEL_LOG_ERROR LOGTYPE_DIAGNOSE Test2!!!]");
        upfLog(0xFF, LOGTYPE_DIAGNOSE, "upf_ncu", UPF_SYSLOG_CODE_NCU_HEARTBEAT_REQ, LOGLEVEL_LOG_ERROR, content);
        /* 测试普通上报3 error diagnose */
        zte_memset_s(content, UPFLOG_MAX_CONTENTLEN , 0, UPFLOG_MAX_CONTENTLEN);
        zte_snprintf_s(content, UPFLOG_MAX_CONTENTLEN, "[PsNcuSystemLogTest][LOGLEVEL_LOG_ERROR LOGTYPE_DIAGNOSE Test3!!!]");
        upfLog(0xFF, LOGTYPE_DIAGNOSE, "upf_ncu", UPF_SYSLOG_CODE_NCU_HEARTBEAT_REQ, LOGLEVEL_LOG_ERROR, content);
        /* 测试普通上报4 error diagnose */
        zte_memset_s(content, UPFLOG_MAX_CONTENTLEN , 0, UPFLOG_MAX_CONTENTLEN);
        zte_snprintf_s(content, UPFLOG_MAX_CONTENTLEN, "[PsNcuSystemLogTest][LOGLEVEL_LOG_ERROR LOGTYPE_DIAGNOSE Test4!!!]");
        upfLog(0xFF, LOGTYPE_DIAGNOSE, "upf_ncu", UPF_SYSLOG_CODE_NCU_HEARTBEAT_REQ, LOGLEVEL_LOG_ERROR, content);
        /* 测试普通上报5 error diagnose */
        zte_memset_s(content, UPFLOG_MAX_CONTENTLEN , 0, UPFLOG_MAX_CONTENTLEN);
        zte_snprintf_s(content, UPFLOG_MAX_CONTENTLEN, "[PsNcuSystemLogTest][LOGLEVEL_LOG_ERROR LOGTYPE_DIAGNOSE Test5!!!]");
        upfLog(0xFF, LOGTYPE_DIAGNOSE, "upf_ncu", UPF_SYSLOG_CODE_NCU_HEARTBEAT_REQ, LOGLEVEL_LOG_ERROR, content);
        /* 测试普通上报 critical diagnose */
        zte_memset_s(content, UPFLOG_MAX_CONTENTLEN , 0, UPFLOG_MAX_CONTENTLEN);
        zte_snprintf_s(content, UPFLOG_MAX_CONTENTLEN, "[PsNcuSystemLogTest][LOGLEVEL_LOG_CRITICAL LOGTYPE_DIAGNOSE Test!!!]");
        upfLog(0xFF, LOGTYPE_DIAGNOSE, "upf_ncu", UPF_SYSLOG_CODE_NCU_HEARTBEAT_REQ, LOGLEVEL_LOG_CRITICAL, content);

        /* 测试普通上报 error servicen fail */
        zte_memset_s(content, UPFLOG_MAX_CONTENTLEN , 0, UPFLOG_MAX_CONTENTLEN);
        zte_snprintf_s(content, UPFLOG_MAX_CONTENTLEN, "[PsNcuSystemLogTest][LOGLEVEL_LOG_ERROR LOGTYPE_SERVICE_FAIL Test!!!]");
        upfLog(0xFF, LOGTYPE_SERVICE_FAIL, "upf_ncu", UPF_SYSLOG_CODE_NCU_HEARTBEAT_REQ, LOGLEVEL_LOG_ERROR, content);

        /* 测试普通上报 error debug */
        zte_memset_s(content, UPFLOG_MAX_CONTENTLEN , 0, UPFLOG_MAX_CONTENTLEN);
        zte_snprintf_s(content, UPFLOG_MAX_CONTENTLEN, "[PsNcuSystemLogTest][LOGLEVEL_LOG_ERROR LOGTYPE_DEBUG Test!!!]");
        upfLog(0xFF, LOGTYPE_DEBUG, "upf_ncu", UPF_SYSLOG_CODE_NCU_HEARTBEAT_REQ, LOGLEVEL_LOG_ERROR, content);

    }
    if(2 == g_NcuLogFlg)
    {
        /* 计数输出 前5个上报 error diagnose */
        zte_memset_s(content, UPFLOG_MAX_CONTENTLEN , 0, UPFLOG_MAX_CONTENTLEN);
        zte_snprintf_s(content, UPFLOG_MAX_CONTENTLEN, "[PsNcuSystemLogTest][LOGLEVEL_LOG_ERROR LOGTYPE_DIAGNOSE First5 Rpt Test!!!]");
        upfLogFirstN(0xFF, 5, LOGTYPE_DIAGNOSE, "upf_ncu", UPF_SYSLOG_CODE_NCU_HEARTBEAT_REQ, LOGLEVEL_LOG_ERROR, content, __FILE__, 1);
    }
    if(3 == g_NcuLogFlg)
    {
        /* 定时10s输出 error diagnose */
        zte_memset_s(content, UPFLOG_MAX_CONTENTLEN , 0, UPFLOG_MAX_CONTENTLEN);
        zte_snprintf_s(content, UPFLOG_MAX_CONTENTLEN, "[PsNcuSystemLogTest][LOGLEVEL_LOG_ERROR LOGTYPE_DIAGNOSE EveryDuration Test!!!]");
        upfLogEveryDuration(0xFF, 10, LOGTYPE_DIAGNOSE, "upf_ncu", UPF_SYSLOG_CODE_NCU_HEARTBEAT_REQ, LOGLEVEL_LOG_ERROR, content, __FILE__, 1);
    }
    if(4 == g_NcuLogFlg)
    {
        /* 计数输出每10个上报 error diagnose */
        zte_memset_s(content, UPFLOG_MAX_CONTENTLEN , 0, UPFLOG_MAX_CONTENTLEN);
        zte_snprintf_s(content, UPFLOG_MAX_CONTENTLEN, "[PsNcuSystemLogTest][LOGLEVEL_LOG_ERROR LOGTYPE_DIAGNOSE Every10 Rpt Test!!!]");
        upfLogEveryN(0xFF,10,LOGTYPE_DIAGNOSE,"upf_mcs",UPF_SYSLOG_CODE_NCU_HEARTBEAT_REQ,LOGLEVEL_LOG_ERROR,content,__FILE__, 1);
    }
    return;
}



VOID psNcuShowNwdafIplistProc()
{
    int j = 0;
    for (j = 0; j < NWDAF_LINKNUM_MAX; j++)
    {

        T_Ncu_NwdafLinkInfo* nwdaflink = &g_nwdaflinkstatus[j].tNwdafAddr;
        /* 空链路不打印 */
        if (0 == nwdaflink->bNwdafIPType)
        {
            continue;
        }

        zte_printf_s("\n ************************* NwdafLinkInfo *************************** ");
        zte_printf_s("\n   bNwdafIPType(1:V4,2:V6,3:V4V6) : %u "
            "\n   bIPv4LinkStatus:(0:Down,1:Up)  : %u "
            "\n   bIPv6LinkStatus:(0:Down,1:Up)  : %u "
            "\n   NwdafV4Address:%u.%u.%u.%u "
            "\n   NwdafV6Address:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x \n",
            nwdaflink->bNwdafIPType, nwdaflink->bIPv4LinkStatus, nwdaflink->bIPv6LinkStatus,
            nwdaflink->NwdafIPv4[0], nwdaflink->NwdafIPv4[1], nwdaflink->NwdafIPv4[2], nwdaflink->NwdafIPv4[3],
            nwdaflink->NwdafIPv6[0], nwdaflink->NwdafIPv6[1], nwdaflink->NwdafIPv6[2], nwdaflink->NwdafIPv6[3],
            nwdaflink->NwdafIPv6[4], nwdaflink->NwdafIPv6[5], nwdaflink->NwdafIPv6[6], nwdaflink->NwdafIPv6[7],
            nwdaflink->NwdafIPv6[8], nwdaflink->NwdafIPv6[9], nwdaflink->NwdafIPv6[10], nwdaflink->NwdafIPv6[11],
            nwdaflink->NwdafIPv6[12], nwdaflink->NwdafIPv6[13], nwdaflink->NwdafIPv6[14], nwdaflink->NwdafIPv6[15]);
    }
}

VOID psNcuSetNwdafLinkProc(CHAR* ipv4str,CHAR* ipv6str, BYTE bIpv4Status, BYTE bIpv6Status)
{
    WORD32 i = 0;
    T_IPV4 NwdafIPv4 = {0};
    T_IPV6 NwdafIPv6 = {0};
    if(!inet_pton(AF_INET, ipv4str, NwdafIPv4))
    {
        zte_printf_s("Invalid IPv4 address");
        return ;
    }
    if(!inet_pton(AF_INET6, ipv6str,NwdafIPv6))
    {
        zte_printf_s("Invalid IPv6 address");
        return ;
    }
    for(i = 0; i< NWDAF_LINKNUM_MAX; i++)
    {
       T_Ncu_NwdafLinkInfo* nwdaflink = &g_nwdaflinkstatus[i].tNwdafAddr;
       if(0 == memcmp(NwdafIPv4, nwdaflink->NwdafIPv4, IPV4_LEN) &&  0 == memcmp(NwdafIPv6, nwdaflink->NwdafIPv6, IPV6_LEN))
        {
            nwdaflink->bIPv4LinkStatus = bIpv4Status;
            nwdaflink->bIPv6LinkStatus = bIpv6Status;
            zte_printf_s("\n *************************After NwdafLinkInfo *************************** ");
            zte_printf_s("\n   bNwdafIPType(1:V4,2:V6,3:V4V6) : %u "
            "\n   bIPv4LinkStatus:(0:Down,1:Up)  : %u "
            "\n   bIPv6LinkStatus:(0:Down,1:Up)  : %u "
            "\n   NwdafV4Address:%u.%u.%u.%u "
            "\n   NwdafV6Address:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x \n",
            nwdaflink->bNwdafIPType, nwdaflink->bIPv4LinkStatus, nwdaflink->bIPv6LinkStatus,
            nwdaflink->NwdafIPv4[0], nwdaflink->NwdafIPv4[1], nwdaflink->NwdafIPv4[2], nwdaflink->NwdafIPv4[3],
            nwdaflink->NwdafIPv6[0], nwdaflink->NwdafIPv6[1], nwdaflink->NwdafIPv6[2], nwdaflink->NwdafIPv6[3],
            nwdaflink->NwdafIPv6[4], nwdaflink->NwdafIPv6[5], nwdaflink->NwdafIPv6[6], nwdaflink->NwdafIPv6[7],
            nwdaflink->NwdafIPv6[8], nwdaflink->NwdafIPv6[9], nwdaflink->NwdafIPv6[10], nwdaflink->NwdafIPv6[11],
            nwdaflink->NwdafIPv6[12], nwdaflink->NwdafIPv6[13], nwdaflink->NwdafIPv6[14], nwdaflink->NwdafIPv6[15]);
        }
    }
    return ;
}

void ncuhearton(WORD32 level)
{
    g_ncu_heart_show = level;
}

void ncuheartclose()
{
    g_ncu_heart_show = 0xff;
}

WORD32 psNcuCSSGSUMediaSendOut(PS_PACKET *ptPacket)
{
    #ifdef FT_TEST
    extern void psCssFtCollectPkt(PS_PACKET* pkt);
    psCssFtCollectPkt(ptPacket);
    psCSSPktLinkBuffDesFree(ptPacket,PKTCHK_NONE);
    return CSS_OK;
    #else
    return psCSSGSUMediaSendOut(ptPacket);
    #endif
}

WORD32 ncuUcomCopySendToJob(PS_PACKET *ptPacket,WORD32 dwJno,WORD32 dwServiceID)
{
    #ifdef FT_TEST
    extern void psCssFtCollectPkt(PS_PACKET* pkt);
    psCssFtCollectPkt(ptPacket);
    psCSSPktLinkBuffDesFree(ptPacket,PKTCHK_NONE);
    return XOS_SUCCESS;
    #else
    return upfUcomCopySendToJob(ptPacket, dwJno, dwServiceID);
    #endif
}
