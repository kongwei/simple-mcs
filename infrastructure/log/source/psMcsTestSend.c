#include "psMcsTestSend.h"
#include "MemShareCfg.h"
#include "psMcsEntry.h"
#include "McsIPv4Head.h"
#include "McsIPv6Head.h"
#include "McsTCPHead.h"
#include "McsUDPHead.h"
#include "McsProtoType.h"
#include "ps_ncu_dataApp.h"
#include "ps_ncu_session.h"
#include "psNcuSubscribeCtxProc.h"
#include "dpa_rcvsend.h"
#include "dpa_compat.h"
#include "psNcuReportUDPProc.h"
#include "psNcuReportSBCProc.h"
#include "psMcsDebug.h"
#include "ps_mcs_define.h"
#include "psNcuApplicationMapCtxProc.h"
#include "psNcuUcomMsg.h"
#include "psUpfEvent.h"
#include "ps_css.h"
#include "ps_db_define_pfu.h"
#include "psNcuHeartBeatProc.h"
#include "psNcuReportStructHead.h"

WORD16 encode_heartbeat(BYTE* buffer, BYTE msgtype);
VOID psUpfCommIMSIToString(T_IMSI tIMSI, BYTE *pbIMSI,BYTE bIMSILen);
BOOL psUpfCommString2tBCD(BYTE *tBCD, BYTE *pbDigits, BYTE bDigitLen);
T_psNcuDaAppCtx g_tDaAppCtx = {0};
T_psNcuDaSubScribeCtx g_tSubScribeCtx = {0};
T_psNcuSessionCtx g_tSession = {0};
T_psNcuFlowCtx g_tStream = {0};
WORD32 g_test_upm = 1;
WORD32 g_test_nr = 1;
WORD32 g_set_data = 1;
void test_set_sesion(const char* ipv4, const char* ipv6, const char* dnn, const CHAR* imsi, const char* isdn);
void test_set_sub(const CHAR* corrletion, WORD32 appid, WORD32 subappid);
void test_set_uil();
void test_set_sesion_uli_nr();
void test_set_sesion_uli_eutra();

void test_del_appid(char* appidstr)
{
    psNcuDelAppIDMapCtxByStrAppID(appidstr);
}
void test_add_appid_relate(char* appidstr, WORD32 appid)
{
    BYTE* ptCtxAddr = NULL;
    psNcuCreatAppIDMapCtxByStrAppID(appidstr,appid, &ptCtxAddr);
}
void test_report()
{
    if(g_set_data)
    {
        test_set_sesion("101.102.103.104", "201:202:203:204::","TestDnn", "64004716648707f2", "689117640677f1");
        test_set_sub("CorelationIDTest", 100, 200);
        g_set_data = 0;
    }
   if(g_test_nr)
    {
        g_tSession.bHasUli = 1;
        test_set_sesion_uli_nr();
    }
    else
    {
        g_tSession.bHasUli = 1;
        test_set_sesion_uli_eutra();
    }
    g_tDaAppCtx.ptSubScribeCtx = (void*)&g_tSubScribeCtx;
    g_tDaAppCtx.ptSessionCtx = (void*)&g_tSession;
    g_tDaAppCtx.ptFlowCtxHead = (void*)&g_tStream;
    g_tDaAppCtx.dwStreamNum = 1;
    DEBUG_TRACE(DEBUG_LOW, "test_report sizeof(Session)=%lu, sizeof(Subscribe)=%lu, sizeof(flow)=%lu, sizeof(daappActx)=%lu,sizeof(UserLocation)=%lu\n",sizeof(g_tSession), sizeof(g_tSubScribeCtx), sizeof(g_tStream),sizeof(g_tDaAppCtx),sizeof(UserLocation));
    if(0 == g_test_upm)
    {
        psNcuReportUDPToNwdafProc(&g_tDaAppCtx, 4,RPT_TYPE_LOOP ,enumQosExpNormal);
    }
    else
    {
        psNcuReportToUpmProc(&g_tDaAppCtx, NULL, 4, 0);
    }
}

void test_report_with_subType(BYTE trigger)
{
    if(g_set_data)
    {
         test_set_sesion("101.102.103.104", "201:202:203:204::","TestDnn", "64004716648707f2", "689117640677f1");
         test_set_sub("CorelationIDTest", 100, 200);
        g_set_data = 0;
    }
   if(g_test_nr)
    {
        g_tSession.bHasUli = 1;
        test_set_sesion_uli_nr();
    }
    else
    {
        g_tSession.bHasUli = 1;
        test_set_sesion_uli_eutra();
    }
    g_tSession.bNwdafIPType = IPCOMM_TYPE_IPV4;
    g_tDaAppCtx.ptSubScribeCtx = (void*)&g_tSubScribeCtx;
    g_tDaAppCtx.ptSessionCtx = (void*)&g_tSession;
    g_tDaAppCtx.ptFlowCtxHead = (void*)&g_tStream;
    g_tDaAppCtx.dwStreamNum = 1;
    DEBUG_TRACE(DEBUG_LOW, "test_report sizeof(Session)=%lu, sizeof(Subscribe)=%lu, sizeof(flow)=%lu, sizeof(daappActx)=%lu,sizeof(UserLocation)=%lu\n",sizeof(g_tSession), sizeof(g_tSubScribeCtx), sizeof(g_tStream),sizeof(g_tDaAppCtx),sizeof(UserLocation));
    if(0 == g_test_upm)
    {
        psNcuReportUDPToNwdafProc(&g_tDaAppCtx, 4,RPT_TYPE_LOOP ,trigger);
    }
    else
    {
        psNcuReportToUpmProc(&g_tDaAppCtx, NULL, 4, 0);
    }
}

WORD32 g_test_send = 0;
WORD16 g_dstPort = 443;
WORD16 g_srcPort = 3246;
WORD32 g_synnum = 1;
WORD32 g_acknum = 1;
T_IPV6 g_tSrcIp = {1,2,3,4};
T_IPV6 g_tDstIp = {9,8,7,6};
BYTE   g_ncuPktVersion = 0;
WORD16 g_ncuPktAttr    = 0;
BYTE g_delStmFlg = 0;
T_NcuSynSessInfo g_sessioninfo = {0};
T_NcuSynSubInfo  g_subscribeinfo = {0};
T_CorrelationId g_correlationId = {0};
T_Notification  g_notification = {0};
T_NcuSynStmRelInfo  g_stmrelinfo = {0};

void test_set_del_stm_flg(BYTE flg)
{
    g_delStmFlg = flg;
}
void test_set_stm_rel_info(char* srcip, char* dstip, WORD16 srcport, WORD16 dstPort,BYTE pro)
{
    if(NULL == srcip || NULL == dstip)
    {
        return;
    }
    zte_memset_s(&g_stmrelinfo, sizeof(g_stmrelinfo), 0, sizeof(g_stmrelinfo));
    T_NcuSynStmRelInfo* ptStmRelInfo = &g_stmrelinfo;
    ptStmRelInfo->wCliPort = srcport;
    ptStmRelInfo->wSvrPort = dstPort;
    ptStmRelInfo->bProType = pro;
    if(inet_pton(AF_INET,srcip,ptStmRelInfo->tCliIP) && inet_pton(AF_INET,dstip,ptStmRelInfo->tSvrIP))
    {
        ptStmRelInfo->bIPType = IPv4_VERSION;
        return;
    }
    if (inet_pton(AF_INET6,srcip,ptStmRelInfo->tCliIP) && inet_pton(AF_INET6,dstip,ptStmRelInfo->tSvrIP))
    {
        ptStmRelInfo->bIPType = IPv6_VERSION;
        return;
    }
}

/* Started by AICoder, pid:i7385ff3d5ld569149790890602baf1375731ec6 */
void ps_set_subscribe_nwdaf_ip(const char* ipv4, const char* ipv6)
{
    if (ipv4 != NULL)
    {
        inet_pton(AF_INET, ipv4, g_subscribeinfo.NwdafIPv4_N4);
    }
    if (ipv6 != NULL)
    {
        inet_pton(AF_INET6, ipv6, g_subscribeinfo.NwdafIPv6_N4);
    }
}
/* Ended by AICoder, pid:i7385ff3d5ld569149790890602baf1375731ec6 */

void test_set_pkt(char* srcip, char* dstip, WORD16 srcport, WORD16 dstPort, WORD32 synnum, WORD32 acknum)
{
    if(NULL == srcip || NULL == dstip)
    {
        return;
    }
    g_dstPort = dstPort;
    g_srcPort = srcport;
    g_synnum = synnum;
    g_acknum = acknum;
    zte_memset_s(g_tSrcIp, sizeof(g_tSrcIp), 0, sizeof(g_tSrcIp));
    zte_memset_s(g_tDstIp, sizeof(g_tDstIp), 0, sizeof(g_tDstIp));
    if(inet_pton(AF_INET,srcip,g_tSrcIp) && inet_pton(AF_INET,dstip,g_tDstIp))
    {
        return;
    }
    inet_pton(AF_INET6,srcip,g_tSrcIp);
    inet_pton(AF_INET6,dstip,g_tDstIp);
    return;
}
void ps_set_correlationid(CHAR* str)
{
    zte_memset_s(&g_correlationId, sizeof(T_CorrelationId),0,sizeof(T_CorrelationId));
    zte_strncpy_s(g_correlationId.correlationId, 255, str, zte_strnlen_s(str, 255));
    g_correlationId.Base.length = mcs_htons(zte_strnlen_s(g_correlationId.correlationId, 255)+1);
}
void ps_set_notification(char* v4, char* v6, char* dnn, char* imsi)
{
    zte_memset_s(&g_notification, sizeof(T_Notification),0,sizeof(T_Notification));
    g_notification.has_EventType_flg = 1;
    g_notification.EventType.EventType = EVENT_QOS_EXP;
    g_notification.has_UeIpv4Addr_flg = 1;
    inet_pton(AF_INET,v4,g_notification.UeIpv4Addr.Ipv4);
    g_notification.has_UeIpv6Prefix_flg = 1;
    inet_pton(AF_INET6,v6,g_notification.UeIpv6Prefix.Ipv6Pre);
    g_notification.has_UeMacAddr_flg = 0;
    g_notification.has_TimeStamp_flg = 1;
    g_notification.TimeStamp.dwTimeStamp = psFtmGetCurStdSec()+ 100000 +0xBC17C200;
    g_notification.has_StartTime_flg = 1;
    g_notification.StartTime.dwTime = psFtmGetCurStdSec()+ 10000 + 0xBC17C200;
    g_notification.has_Dnn_flg = 1;
    zte_strncpy_s(g_notification.Dnn.dnn,32, dnn, zte_strnlen_s(dnn,32));
    g_notification.has_Snssai_flg = 1;
    g_notification.Snssai.sst = 3;
    g_notification.Snssai.sd[0] = 'F';
    g_notification.Snssai.sd[1] = 'F';
    g_notification.Snssai.sd[2] = 'F';
    g_notification.has_Supi_flg = 1;
    psUpfCommString2tBCD(g_notification.Supi.supi,(BYTE*)imsi,zte_strnlen_s(imsi, 15));
    g_notification.has_Gpsi_flg = 1;
    psUpfCommString2tBCD(g_notification.Gpsi.gpsi,(BYTE*)imsi,zte_strnlen_s(imsi, 15));
    g_notification.has_RatType_flg = 1;
    g_notification.RatType.bRatType = RATE_TYPE_NR;


}
void psNcuFillDataTest(T_Notification* ptNotification, T_CorrelationId* ptCorrelationId)
{
    zte_memcpy_s(ptCorrelationId, sizeof(T_CorrelationId),&g_correlationId,sizeof(T_CorrelationId));
    zte_memcpy_s(ptNotification, sizeof(T_Notification), &g_notification, sizeof(T_Notification));

}
void ps_set_correlation(CHAR* cord)
{
    zte_memcpy_s(g_subscribeinfo.aucCorid, 255, cord, zte_strnlen_s(cord,255));
    g_subscribeinfo.bCorID_len = zte_strnlen_s(cord,255);
}
void ps_set_appid_relate(WORD32 subappid1, WORD32 subappid2, WORD32 subappid3, WORD32 subappid4)
{   
    g_subscribeinfo.wAppidNum = 4;
    g_subscribeinfo.appid[0] = subappid1;
    g_subscribeinfo.appid[1] = subappid2;
    g_subscribeinfo.appid[2] = subappid3;
    g_subscribeinfo.appid[3] = subappid4;
}
void ps_set_session(CHAR* imsi, CHAR* isdn, BYTE rateType, char* ipv4, char* ipv6, char* dnn)
{
    if(NULL != imsi)
    {
        psUpfCommString2tBCD(g_sessioninfo.bImsi, (BYTE *)imsi, zte_strnlen_s(imsi, 15));
    }
    if(NULL != isdn)
    {
        psUpfCommString2tBCD(g_sessioninfo.bIsdn, (BYTE *)isdn, zte_strnlen_s(isdn, 15));
    }
    if(NULL != ipv4)
    {
        inet_pton(AF_INET,ipv4,g_sessioninfo.tMSIPv4);
        g_sessioninfo.bMSIPtype |= 1;
    }
    if(NULL != ipv6)
    {
        inet_pton(AF_INET6, ipv6, g_sessioninfo.tMSIPv6);
        g_sessioninfo.bMSIPtype |= 2;
    }
    if(NULL != dnn)
    {
        zte_memcpy_s(g_sessioninfo.apnname, 64, dnn, zte_strnlen_s(dnn, 64));
    }

    g_sessioninfo.bRatType = rateType;
}
/*
void ps_set_uli(WORD64 cgi, WORD64 sai, WORD64 rai, WORD64 tai, WORD64 ecgi, WORD64 ncgi)
{
    if(0!= cgi)
    {
        g_sessioninfo.tUli.flags.btCGI = 1;
        g_sessioninfo.tUli.ddwCGI = cgi;
    }
    if(0!= sai)
    {
        g_sessioninfo.tUli.flags.btSAI = 1;
        g_sessioninfo.tUli.ddwSAI = sai;
    }
    if(0!= rai)
    {
        g_sessioninfo.tUli.flags.btRAI = 1;
        g_sessioninfo.tUli.ddwRAI = rai;
    }
    if(0!= tai)
    {
        g_sessioninfo.tUli.flags.btTAI = 1;
        g_sessioninfo.tUli.ddwTAI = tai;
    }
    if(0!= ecgi)
    {
        g_sessioninfo.tUli.flags.btECGI = 1;
        g_sessioninfo.tUli.ddwECGI = ecgi;
    }
    if(0!= ncgi)
    {
        g_sessioninfo.tUli.flags.btNCGI = 1;
        g_sessioninfo.tUli.ddwNCGI = ncgi;
    }
    g_sessioninfo.tUli_Flag = 1;
}
*/
void ps_set_snssai(WORD16 sliceid, BYTE sst, CHAR* sd)
{
    g_sessioninfo.tSNssai.ucSST = sst;
    g_sessioninfo.tSNssai.SliceId = sliceid;
    zte_memcpy_s(g_sessioninfo.tSNssai.aucSD, 3, sd, zte_strnlen_s(sd, 3));
}
WORD32 ps_set_appid_subappid(WORD32 appid, WORD32 subappid)
{
    if(0 ==g_test_send)
    {
        return MCS_RET_FAIL;
    }
    BYTE threadID  = 4;
    BYTE* buffer = &g_ptVpfuShMemVar->aucInnerAckMsg[threadID%MEDIA_THRD_NUM][0]; //后面要写死线程为4，用宏
    WORD16 wDataLen = sizeof(T_NcuHeadInfo);
    T_NcuHeadInfo* ptNcuInfo = (T_NcuHeadInfo*)buffer;
    ptNcuInfo->bType = NCU_SESSSION_SYN;
    ptNcuInfo->bVersion = 0;
    ptNcuInfo->wLength = 0;

    T_NcuSynSubInfoHead* ptSubInfoHead = (T_NcuSynSubInfoHead*)(buffer+wDataLen);
    wDataLen += sizeof(T_NcuSynSubInfoHead);

    T_NcuSynSubAppidInfo* ptSynAppidRelate = (T_NcuSynSubAppidInfo*)(buffer+wDataLen);
    ptSynAppidRelate->dwAppid = appid;
    ptSynAppidRelate->dwSubAppid = subappid;
    ptSubInfoHead->bType = NCU_SYNSUBAPPID;
    PS_PACKET *ptPacket = psCSSGenPkt(buffer, wDataLen+sizeof(T_NcuSynSubAppidInfo), threadID);
    if(NULL == ptPacket)
    {
        return MCS_RET_FAIL;
    }
    psVpfuMcsIPPktRcv(ptPacket);

    return MCS_RET_SUCCESS;

}

WORD32 ps_make_ncu_subscribe(BYTE subscribeType, WORD64 upseid, WORD32 appid, BYTE subtype, BYTE threadID)
{
    if(0 ==g_test_send)
    {
        return MCS_RET_FAIL;
    }
    BYTE* buffer = &g_ptVpfuShMemVar->aucInnerAckMsg[threadID%MEDIA_THRD_NUM][0];
    WORD16 wDataLen = sizeof(T_NcuHeadInfo);
    T_NcuHeadInfo* ptNcuInfo = (T_NcuHeadInfo*)buffer;
    ptNcuInfo->bType = NCU_SESSSION_SYN;
    ptNcuInfo->bVersion = 0;
    ptNcuInfo->wLength = 0;

    T_NcuSynSubInfoHead* ptSubInfoHead = (T_NcuSynSubInfoHead*)(buffer+wDataLen);
    wDataLen += sizeof(T_NcuSynSubInfoHead);
    ptSubInfoHead->bType = subscribeType;
    ptSubInfoHead->upseid = upseid;
    ptSubInfoHead->appid = appid;
    ptSubInfoHead->bSubType = subtype;
    T_NcuSynSubInfo* ptSubScribeInfo = NULL;
    T_NcuSynSessInfo* ptSessionInfo = NULL;
    switch(subscribeType)
    {
        case NCU_ADD_SUBSCRIPTION:
            ptNcuInfo->bType = NCU_SUBSCRIPTION_SYN;
            ptSubScribeInfo = (T_NcuSynSubInfo*)(buffer+wDataLen);
            wDataLen += sizeof(T_NcuSynSubInfo);
            zte_memcpy_s(ptSubScribeInfo,sizeof(T_NcuSynSubInfo), &g_subscribeinfo, sizeof(T_NcuSynSubInfo));
            break;
        case NCU_CANCEL_SUBSCRIPTION:
            ptNcuInfo->bType = NCU_SUBSCRIPTION_SYN;
            break;
        case NCU_DEL_SESSION:
            break;
        case NCU_MODIFY_SESSION:
        {
            ptSessionInfo = (T_NcuSynSessInfo*)(buffer+wDataLen);
            zte_memcpy_s(ptSessionInfo, sizeof(T_NcuSynSessInfo),&g_sessioninfo, sizeof(T_NcuSynSessInfo));
            wDataLen += sizeof(T_NcuSynSessInfo);
            break;
        }
        case NCU_ADD_SESSION:
        {
            ptSessionInfo = (T_NcuSynSessInfo*)(buffer+wDataLen);
            zte_memcpy_s(ptSessionInfo, sizeof(T_NcuSynSessInfo),&g_sessioninfo, sizeof(T_NcuSynSessInfo));
            wDataLen += sizeof(T_NcuSynSessInfo);
            break;
        }
        case NCU_DEL_STREAM:
            T_NcuSynStmRelInfo* ptStmRelInfo = (T_NcuSynStmRelInfo*)(buffer+wDataLen);
            zte_memcpy_s(ptStmRelInfo, sizeof(T_NcuSynStmRelInfo),&g_stmrelinfo, sizeof(T_NcuSynStmRelInfo));
            wDataLen += sizeof(T_NcuSynStmRelInfo);
            break;
        default:
            return MCS_RET_FAIL;
    }

    PS_PACKET *ptPacket = psCSSGenPkt(buffer, wDataLen, threadID);
    if(NULL == ptPacket)
    {
        return MCS_RET_FAIL;
    }
    psVpfuMcsIPPktRcv(ptPacket);
    zte_memset_s(&g_subscribeinfo, sizeof(g_subscribeinfo), 0, sizeof(g_subscribeinfo));
    return MCS_RET_SUCCESS;
}

void ps_make_ncu_ucom_msg(BYTE threadID, WORD32 dwMsgType, WORD32 dwGroupId, BYTE iptype)
{
    BYTE *buffer = &(g_ptVpfuShMemVar->aucInnerAckMsg[threadID%MEDIA_THRD_NUM][0]);
    T_psUpfUcomHead *ptUcomhead = (T_psUpfUcomHead*)buffer;
    ptUcomhead->dwMsgType = dwMsgType;
    WORD16 wDataLen = sizeof(T_psUpfUcomHead);
    switch (dwMsgType)
    {
        case EV_MSG_MANAGE_TO_MEDIA_POST_MIG_OUT_REQ:
        {
            T_psMcsSessDelByGroupArea *ptMsgBody = (T_psMcsSessDelByGroupArea*)(buffer + wDataLen);
            ptMsgBody->dwGroupId = dwGroupId;
            wDataLen += sizeof(T_psMcsSessDelByGroupArea);
            break;
        }
        case EV_MSG_UCOM_NCU_JOB_TO_MCS_REQ:
        {
            T_psNcuHeartHead* ptHeartHead = (T_psNcuHeartHead*)(buffer+wDataLen);
            ptHeartHead->tNcuAddress.bType = iptype;
            ptHeartHead->tNwdafAddress.bType = iptype;
            IPV6_COPY(ptHeartHead->tNcuAddress.abAddress, g_tSrcIp);
            IPV6_COPY(ptHeartHead->tNwdafAddress.abAddress, g_tDstIp);
            ptHeartHead->wNcuPort = g_srcPort;
            ptHeartHead->wNwdafPort = g_srcPort;
            wDataLen += sizeof(T_psNcuHeartHead);
            break;
        }
    }

    PS_PACKET *ptPacket = psCSSGenPkt(buffer, wDataLen, threadID);
    if (NULL != ptPacket)
    {
        ptPacket->wPktAttr = CSS_FWD_UCOM;
        psVpfuMcsIPPktRcv(ptPacket);
    }
}

void ps_make_ncu_pkt_setdata(WORD64 upseid, WORD32 subappid, BYTE iptype, BYTE dir, BYTE threadID, WORD16 payload, WORD32 pktTimsStamp)
{
    BYTE* buffer = &g_ptVpfuShMemVar->aucInnerAckMsg[threadID%MEDIA_THRD_NUM][0];
    T_NcuPktInfoHead* ptPktHead = (T_NcuPktInfoHead*)(buffer+sizeof(T_NcuHeadInfo));
    ptPktHead->wL34Length = payload;
    ptPktHead->dwTimestamp = pktTimsStamp;
    ps_make_ncu_pkt(upseid, subappid, iptype, dir, threadID);

}
void ps_make_ncu_pkt_slowflowflg(BYTE slowflowflg, BYTE threadID)
{
    BYTE* buffer = &g_ptVpfuShMemVar->aucInnerAckMsg[threadID%MEDIA_THRD_NUM][0];
    T_NcuPktInfoHead* ptPktHead = (T_NcuPktInfoHead*)(buffer+sizeof(T_NcuHeadInfo));
    ptPktHead->bSlowStmFlag = slowflowflg;
}
void ps_make_ncu_pkt(WORD64 upseid, WORD32 subappid, BYTE iptype, BYTE dir, BYTE threadID)
{
    if(0 ==g_test_send)
    {
        return;
    }
    BYTE* buffer = &g_ptVpfuShMemVar->aucInnerAckMsg[threadID%MEDIA_THRD_NUM][0];
    WORD16 wDataLen = sizeof(T_NcuHeadInfo);
    T_NcuHeadInfo* ptNcuInfo = (T_NcuHeadInfo*)buffer;
    ptNcuInfo->bType = NCU_PKT_COPY;
    ptNcuInfo->bVersion = g_ncuPktVersion;
    ptNcuInfo->wLength = 64+sizeof(T_NcuPktInfoHead);
    ptNcuInfo->bPktnum = 1;

    T_NcuPktInfoHead* ptPktHead = (T_NcuPktInfoHead*)(buffer+wDataLen);
    wDataLen += sizeof(T_NcuPktInfoHead);
    ptPktHead->b5QI = 1;
    ptPktHead->bDir = dir;
    ptPktHead->bIpType = iptype;
    ptPktHead->subappid = subappid;
    ptPktHead->upseid = upseid;
    ptPktHead->bDelStream = g_delStmFlg;

    if(iptype == 4)
    {
        T_psMcsIPv4Head * ipHeader = (T_psMcsIPv4Head*)(buffer+wDataLen);
        wDataLen+= sizeof(T_psMcsIPv4Head);
        ipHeader->btVersion = 4;
        ipHeader->btHeadLen = 5;
        ipHeader->bTos      = 0;
        ipHeader->wPktLen   = mcs_htons(123);
        ipHeader->wId       = mcs_htons(1);
        ipHeader->wOffSet   = 0;
        ipHeader->bTtl      = 0x80;
        ipHeader->bPro      = MCS_TCP_PROTOCOL;
        if(dir ==MCS_PKT_DIR_UL)
        {
            IPV4_COPY(ipHeader->tSrcIp, g_tSrcIp);
            IPV4_COPY(ipHeader->tDstIp, g_tDstIp);
        }
        else
        {
            IPV4_COPY(ipHeader->tSrcIp, g_tDstIp);
            IPV4_COPY(ipHeader->tDstIp, g_tSrcIp);
        }
        ipHeader->wChkSum   = 0;
        ipHeader->wChkSum   = guIp4ChkSum(ipHeader);
    }
    else
    {
        T_psMcsIPv6Head* ptStrm = (T_psMcsIPv6Head *)(buffer+wDataLen);
        wDataLen+= sizeof(T_psMcsIPv6Head);
        ptStrm->bVersion       = 6;
        ptStrm->dwFlowLabel    = mcs_htons(0);
        ptStrm->dwFlowLabel_h  = mcs_htons(0);
        ptStrm->bTC            = 0;
        ptStrm->bTC_l          = 0;
        ptStrm->wPayloadLen    = mcs_htons(123);
        ptStrm->bNextHeader    = MCS_TCP_PROTOCOL;
        ptStrm->bHopLimit      = 0x80;
        if(dir ==MCS_PKT_DIR_UL)
        {
            IPV6_COPY(ptStrm->tSrcIp, g_tSrcIp);
            IPV6_COPY(ptStrm->tDstIp, g_tDstIp);
        }
        else 
        {
            IPV6_COPY(ptStrm->tSrcIp, g_tDstIp);
            IPV6_COPY(ptStrm->tDstIp, g_tSrcIp);
        }
    }

    T_psMcsTcpHead* ptTcpHeader = (T_psMcsTcpHead*)(buffer+wDataLen);
    wDataLen += sizeof(T_psMcsTcpHead);
    if(dir ==MCS_PKT_DIR_UL)
    {
        ptTcpHeader->wSrcPort = mcs_htons(g_srcPort);
        ptTcpHeader->wDstPort = mcs_htons(g_dstPort);
    }
    else
    {
        ptTcpHeader->wSrcPort = mcs_htons(g_dstPort);
        ptTcpHeader->wDstPort = mcs_htons(g_srcPort);
    }
    ptTcpHeader->bSynFlag = 0;
    ptTcpHeader->bAckFlag = 1;
    ptTcpHeader->bHeaderLen = 5;
    ptTcpHeader->dwSeqNum = mcs_htonl(g_synnum);
    ptTcpHeader->dwAckNum = mcs_htonl(g_acknum); 
    WORD32 dwBuffLen = sizeof(T_NcuHeadInfo) + sizeof(T_NcuPktCopyInfo);
    PS_PACKET *ptPacket = psCSSGenPkt(buffer, dwBuffLen, threadID);
    if(NULL == ptPacket)
    {
        return;
    }
    ptPacket->wPktAttr  = g_ncuPktAttr;
    psVpfuMcsIPPktRcv(ptPacket);
}

void ps_make_ncu_trafficrpt(WORD64 upseid, WORD32 subappid, BYTE iptype, BYTE dir, BYTE threadID)
{
    if(0 ==g_test_send)
    {
        return;
    }
    BYTE* buffer = &g_ptVpfuShMemVar->aucInnerAckMsg[threadID%MEDIA_THRD_NUM][0];
    WORD16 wDataLen = sizeof(T_NcuHeadInfo);
    T_NcuHeadInfo* ptNcuInfo = (T_NcuHeadInfo*)buffer;
    zte_memset_s(ptNcuInfo, sizeof(T_NcuHeadInfo), 0, sizeof(T_NcuHeadInfo));
    ptNcuInfo->bType = NCU_PKT_TRAFFICREPORT;
    ptNcuInfo->bVersion = 0;
    ptNcuInfo->wLength = sizeof(T_NcuTrafficReportInfo);
    ptNcuInfo->bPktnum = 1;

    T_NcuTrafficReportInfo* ptTrafficReportInfo= (T_NcuTrafficReportInfo*)(buffer+wDataLen);
    zte_memset_s(ptTrafficReportInfo, sizeof(T_NcuTrafficReportInfo), 0, sizeof(T_NcuTrafficReportInfo));
    wDataLen += sizeof(T_NcuTrafficReportInfo);
    ptTrafficReportInfo->b5QI = 1;
    ptTrafficReportInfo->bSlowStmFlag = 1;
    ptTrafficReportInfo->bDir = dir;
    ptTrafficReportInfo->bIPType = iptype;
    ptTrafficReportInfo->dwSubAppid = subappid;
    ptTrafficReportInfo->ddwUpSeid = upseid;
    ptTrafficReportInfo->bProType = MCS_UDP_PROTOCOL;
    ptTrafficReportInfo->dwReportPktNum = 1;
    ptTrafficReportInfo->dwTrafficLength = 1024;
    if(dir == 1)
    {
        ptTrafficReportInfo->bProType = MCS_TCP_PROTOCOL;
        ptTrafficReportInfo->bSpecialStmFlag = 1;
    }
    ptTrafficReportInfo->bDelStream = g_delStmFlg;
    ptTrafficReportInfo->wCliPort = g_srcPort;
    ptTrafficReportInfo->wSvrPort = g_dstPort;
    IPV6_COPY(ptTrafficReportInfo->tSvrIP, g_tDstIp);
    IPV6_COPY(ptTrafficReportInfo->tCliIP, g_tSrcIp);
    
    PS_PACKET *ptPacket = psCSSGenPkt(buffer, wDataLen, threadID);
    if(NULL == ptPacket)
    {
        return;
    }
    psVpfuMcsIPPktRcv(ptPacket);
}

/* Started by AICoder, pid:q1d0c6733f8a7eb14ce20a570023e938ee263175 */
void ps_make_heartbeat(BYTE ipType, BYTE threadID, BYTE msgtype)
{
    BYTE* buffer = &g_ptVpfuShMemVar->aucInnerAckMsg[threadID % MEDIA_THRD_NUM][0];
    WORD16 datalen = encode_heartbeat(buffer, msgtype);

    PS_PACKET *ptPacket = psCSSGenPkt(buffer, datalen, threadID);
    if(NULL == ptPacket)
    {
        return;
    }

    T_IPV6 tUpfIP = {0};
    T_IPV6 tNwdafIP = {0};
    WORD16 wNwdafPort = g_srcPort;
    IPV6_COPY(tNwdafIP, g_tSrcIp);
    IPV6_COPY(tUpfIP, g_tDstIp);

    if(ipType == IPCOMM_TYPE_IPV6)
    {
        ptPacket->wPktAttr  = CSS_FWD_NWDAF_IPV6;
        psMcsUDPIpv6HeadCap(ptPacket, wNwdafPort, wNwdafPort, datalen+8, tNwdafIP, tUpfIP);
        psMcsIPv6HeadCap(ptPacket, tNwdafIP, tUpfIP, datalen+8, 0, MCS_UDP_PROTOCOL);
    }
    else if(ipType == IPCOMM_TYPE_IPV4)
    {
        ptPacket->wPktAttr  = CSS_FWD_NWDAF_IPV4;
        DEBUG_TRACE(DEBUG_LOW, "tUPFIP=%u.%u.%u.%u, tNwdafIP=%u.%u.%u.%u, port=%u\n", tUpfIP[0], tUpfIP[1], tUpfIP[2], tUpfIP[3], tNwdafIP[0], tNwdafIP[1], tNwdafIP[2],tNwdafIP[3], wNwdafPort);
        psMcsUDPHeadCap(ptPacket, wNwdafPort, wNwdafPort, datalen+8);
        psMcsIPv4HeadCap(ptPacket, tNwdafIP, tUpfIP, 0, MCS_UDP_PROTOCOL);
    }

    psVpfuMcsIPPktRcv(ptPacket);
}
/* Ended by AICoder, pid:q1d0c6733f8a7eb14ce20a570023e938ee263175 */

WORD16 encode_heartbeat(BYTE* buffer, BYTE msgtype)
{
    if (NULL == buffer) return 0;
    if (HEART_BEART_REQUEST == msgtype)
    {
        return psEncodeHeartReqDataProc(buffer);
    }
    if (HEART_BREAT_RESPONSE == msgtype)
    {
        return psEncodeHeartRspDataProc(buffer);
    }
    return 0;
}

void test_set_sesion_uli_eutra()
{
    UserLocation* ptUli = &g_tSession.tUserLocation;
    zte_memset_s(ptUli, sizeof(UserLocation), 0, sizeof(UserLocation));
    ptUli->eutraLocFg = 1;
    ptUli->eutraLocation.tai.plmn.btMCC1 = 5;
    ptUli->eutraLocation.tai.plmn.btMCC2 = 5;
    ptUli->eutraLocation.tai.plmn.btMCC3 = 5;
    ptUli->eutraLocation.tai.plmn.btMNC1 = 4;
    ptUli->eutraLocation.tai.plmn.btMNC2 = 4;
    ptUli->eutraLocation.tai.plmn.btMNC3 = 4;
    ptUli->eutraLocation.tai.bTAC[0] = 2;
    ptUli->eutraLocation.tai.bTAC[1] = 2;

    ptUli->eutraLocation.ecgi.plmnId.btMCC1 = 5;
    ptUli->eutraLocation.ecgi.plmnId.btMCC2 = 5;
    ptUli->eutraLocation.ecgi.plmnId.btMCC3 = 5;
    ptUli->eutraLocation.ecgi.plmnId.btMNC1 = 4;
    ptUli->eutraLocation.ecgi.plmnId.btMNC2 = 4;
    ptUli->eutraLocation.ecgi.plmnId.btMNC3 = 4;
    ptUli->eutraLocation.ecgi.eutraCellId[0] = 3;
    ptUli->eutraLocation.ecgi.eutraCellId[1] = 3;
    ptUli->eutraLocation.ecgi.eutraCellId[2] = 3;
    ptUli->eutraLocation.ecgi.eutraCellId[3] = 3;

    ptUli->eutraLocation.btAgeOfLocationInformationFg = 1;
    ptUli->eutraLocation.ageOfLocationInformation = 12;
    ptUli->eutraLocation.btUeLocationTimestampFg = 1;
    ptUli->eutraLocation.ueLocationTimestamp.time.dwSecond = mcs_htonl(psFtmGetCurStdSec() + 0xBC17C200);
    ptUli->eutraLocation.ueLocationTimestamp.zone = 8;
}

void test_set_sesion_uli_nr()
{
    UserLocation* ptUli = &g_tSession.tUserLocation;
    zte_memset_s(ptUli, sizeof(UserLocation), 0, sizeof(UserLocation));
    ptUli->nrLocFg = 1;
    
    ptUli->nrLocation.tai.plmn.btMCC1 = 5;
    ptUli->nrLocation.tai.plmn.btMCC2 = 5;
    ptUli->nrLocation.tai.plmn.btMCC3 = 5;
    ptUli->nrLocation.tai.plmn.btMNC1 = 4;
    ptUli->nrLocation.tai.plmn.btMNC2 = 4;
    ptUli->nrLocation.tai.plmn.btMNC3 = 4;
    ptUli->nrLocation.tai.bTAC[0] = 3;
    ptUli->nrLocation.tai.bTAC[1] = 3;
    ptUli->nrLocation.tai.bTAC[2] = 3;

    ptUli->nrLocation.ncgi.plmnId.btMCC1 = 5;
    ptUli->nrLocation.ncgi.plmnId.btMCC2 = 5;
    ptUli->nrLocation.ncgi.plmnId.btMCC3 = 5;
    ptUli->nrLocation.ncgi.plmnId.btMNC1 = 4;
    ptUli->nrLocation.ncgi.plmnId.btMNC2 = 4;
    ptUli->nrLocation.ncgi.plmnId.btMNC3 = 4;
    ptUli->nrLocation.ncgi.nrCellId[0] = 5;
    ptUli->nrLocation.ncgi.nrCellId[1] = 5;
    ptUli->nrLocation.ncgi.nrCellId[2] = 5;
    ptUli->nrLocation.ncgi.nrCellId[3] = 5;
    ptUli->nrLocation.ncgi.nrCellId[4] = 5;

    ptUli->nrLocation.btAgeOfLocationInformationFg = 1;
    ptUli->nrLocation.ageOfLocationInformation = 13;
    ptUli->nrLocation.btUeLocationTimestampFg = 1;
    ptUli->nrLocation.ueLocationTimestamp.time.dwSecond = mcs_htonl(psFtmGetCurStdSec() + 0xBC17C200);
    ptUli->nrLocation.ueLocationTimestamp.zone = 8;
}

void test_set_sesion(const char* ipv4, const char* ipv6, const char* dnn,const CHAR* imsi, const char* isdn)
{
    if(NULL != ipv4)
    {
        inet_pton(AF_INET,ipv4,g_tSession.tMSIPv4);
    }
    if(NULL != ipv6)
    {
        inet_pton(AF_INET6,ipv6,g_tSession.tMSIPv6);
    }
    if(NULL != dnn)
    {
        zte_memcpy_s(g_tSession.Apn, 64, dnn,zte_strnlen_s(dnn, 64));
    }
    g_tSession.bRatType = 10;
    if(NULL != imsi)
    {
        zte_printf_s("imsi: %s\n", imsi);
        zte_printf_s("isdn:%s\n", isdn);
        psUpfCommString2tBCD(g_tSession.bImsi, (BYTE*)imsi, 15);
        psUpfCommString2tBCD(g_tSession.bIsdn, (BYTE*)isdn, 15);
        zte_printf_s("\n[has_Supi_flg] imsi:0x%x-0x%x-0x%x-0x%x-0x%x-0x%x-0x%x-0x%x\n",
            g_tSession.bImsi[0],
            g_tSession.bImsi[1],
            g_tSession.bImsi[2],
            g_tSession.bImsi[3],
            g_tSession.bImsi[4],
            g_tSession.bImsi[5],
            g_tSession.bImsi[6],
            g_tSession.bImsi[7]);
        zte_printf_s("\n[has_Supi_flg] bIsdn:0x%x-0x%x-0x%x-0x%x-0x%x-0x%x-0x%x-0x%x\n",
            g_tSession.bIsdn[0],
            g_tSession.bIsdn[1],
            g_tSession.bIsdn[2],
            g_tSession.bIsdn[3],
            g_tSession.bIsdn[4],
            g_tSession.bIsdn[5],
            g_tSession.bIsdn[6],
            g_tSession.bIsdn[7]);
        CHAR imsistr[16] = {0};
        CHAR isdnstr[16] = {0};
        psUpfCommIMSIToString(g_tSession.bImsi,(BYTE*)imsistr,15);
        psUpfCommIMSIToString(g_tSession.bIsdn,(BYTE*)isdnstr,15);
        zte_printf_s("imsistr: %s\n", imsistr);
        zte_printf_s("isdnstr:%s\n", isdnstr);
    }
    g_tSession.tSNssai.SliceId =1;
    g_tSession.tSNssai.ucSST = 2;
    g_tSession.tSNssai.aucSD[0] = 0xff;
    g_tSession.tSNssai.aucSD[1] = 0xff;
    g_tSession.tSNssai.aucSD[2] = 0xff;
}
void test_set_sub(const CHAR* corrletion, WORD32 appid, WORD32 subappid)
{
    if(NULL == corrletion)
    {
        return;
    }
    g_tDaAppCtx.dwAppid = appid;
    zte_memcpy_s(g_tDaAppCtx.subAppidStr, 64, "testSubAppid", 12);
    zte_memcpy_s(g_tSubScribeCtx.appidstr,64, "testAppid", 9);
    zte_memcpy_s(g_tSession.bCorrelationId_Ana, 255, corrletion, zte_strnlen_s(corrletion,255));
    zte_memcpy_s(g_tSession.bCorrelationId_Exp, 255, corrletion, zte_strnlen_s(corrletion,255));
    g_tDaAppCtx.dwSubAppid = subappid;
}
void test_set_session_rattype(BYTE rattype)
{
    g_tSession.bRatType = rattype;
}
#define MAX_CHAR_IMSI_LEN_UPF           (WORD32)15   /* IMSI的最大长度 */
#define MIN_CHAR_ISMI_LEN_UPF           (WORD32)6    /* IMSI的最小长度 */

VOID psUpfCommIMSIToString(T_IMSI tIMSI,
                           BYTE *pbIMSI,
                           BYTE bIMSILen)
{
    WORD16 bLen;
    WORD16 bLoop;
    BYTE bIMSIHigh;
    BYTE bIMSILow;
    BYTE *pbIMSIValue;

    if (NULL == pbIMSI)
    {
        return;
    }

    if (NULL == tIMSI)
    {
        *pbIMSI = 0;

        return;
    }

    pbIMSIValue = pbIMSI;

    if (bIMSILen < MIN_CHAR_ISMI_LEN_UPF)
    {
        *pbIMSI = 0;

        return;
    }

    if (MAX_CHAR_IMSI_LEN_UPF< bIMSILen)
    {
        bLen = MAX_CHAR_IMSI_LEN_UPF;
    }
    else
    {
        bLen = bIMSILen;
    }

    for (bLoop = 0; bLoop < (bLen + 1) / 2; bLoop++)
    {
        bIMSIHigh = (BYTE)(tIMSI[bLoop]>>4) & 0x0f;
        bIMSILow = (BYTE)(tIMSI[bLoop]) & 0x0f;

        if (0x0f == bIMSILow)
        {
            break;
        }
        else if (bIMSILow > 0x9)
        {
            *pbIMSI = 0;

            return;
        }
        else
        {
            ;
        }

        if ((bLoop == (bLen + 1) / 2 - 1) && (0 == (bLen + 1) % 2))
        {
            *pbIMSIValue++ = (BYTE)(bIMSILow + 48);
        }
        else
        {
            *pbIMSIValue++ = (BYTE)(bIMSILow + 48);

            if (0x0f == bIMSIHigh)
            {
                break;
            }
            else if (bIMSIHigh > 0x9)
            {
                *pbIMSI = 0;

                return;
            }
            else
            {
                ;
            }

           *pbIMSIValue++ = (BYTE)(bIMSIHigh + 48);
        }
    }

    *pbIMSIValue = 0;

    return;
}

BOOL psUpfCommString2tBCD(BYTE *tBCD, BYTE *pbDigits, BYTE bDigitLen)
{
    if(NULL == pbDigits)
    {
        return false;
    }

    if(NULL == tBCD)
    {
        return false;
    }

    if(bDigitLen < MIN_CHAR_ISMI_LEN_UPF)
    {
        return false;
    }

    WORD16 bLen;
    WORD16 bLoop;
    BYTE bBCDHigh;
    BYTE bBCDLow;

    bLen = MIN(bDigitLen, (MAX_CHAR_IMSI_LEN_UPF+1)); /*MAX:16*/
    zte_memset_s(tBCD, IMSI_LEN, 0xFF, IMSI_LEN);

    for (bLoop = 0; bLoop < (bLen + 1) / 2; bLoop++)
    {
        bBCDLow = *pbDigits++;
        bBCDHigh = *pbDigits++;

        if ((bBCDLow < 0x30) || (bBCDLow > 0x39))
        {
            return false;
        }

        if ((bLoop == (bLen + 1) / 2 - 1) && (0 == (bLen + 1) % 2))
        {
            tBCD[bLoop] = 0xf0 + (bBCDLow - 48);
        }
        else
        {
            if ((bBCDHigh < 0x30) || (bBCDHigh > 0x39))
            {
                return false;
            }

            tBCD[bLoop] = (BYTE)((bBCDHigh - 48) * 16) + (bBCDLow - 48);
        }
    }

    return true;
}
