#include <arpa/inet.h>
#include "sccu_pub_define.h"
#include "sccu_pub_api.h"
#include "Http2Proxy.App2HttpDetect.pb.h"
#include "HTTP_LB/HTTPInterface.h"
#include "httpLbScInfo.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "dpa_rcvsend.h"

#ifdef __cplusplus
}
#endif
#include "psNcuHttpLinkProc.h"
#include "psUpfJobTypes.h"
#include "ncuSCCUAbility.h"
#include "thdm_pub.h"
#include "ps_db_define_ncu.h"
#include "psMcsDebug.h"
#include "psNcuCtrlStatInterface.h"
#include "ps_ncu_typedef.h"
#include "psUpfEvent.h"
#include "psUpfPubSub.h"
#include "ps_db_define_pfu.h"
#include "UPFLog.h"
#include "upfPub.h"
#include "pfuUserGroup.h"
#include "UPFHelp.h"
using namespace std;
using namespace http2proxy;

WORD32 g_LogRptInterval = 30;

BOOL psNcuGetPort(const char* portStart, WORD16* pwNwdafPort)
{
    MCS_CHK_NULL_RET(portStart, FALSE);
    MCS_CHK_NULL_RET(pwNwdafPort, FALSE);

    const char* portEnd = portStart;
    char portStr[10] = {0};
    WORD32 dwLoop = 1;
    while(*(portEnd + 1) >= '0' && *(portEnd + 1) <= '9')
    {
        dwLoop++;
        portEnd++;
        if(dwLoop >= 10)
        {
            return FALSE;
        }
    }
    zte_memcpy_s(portStr, 10, portStart, portEnd - portStart + 1);
    *pwNwdafPort = atoi(portStr);
    return TRUE;
}

/* Started by AICoder, pid:u1ab0q9cfdz14421418709081063e240fd50d59d */
BOOL psNcuGetIPv6AndPortByReportUri(const char* originUri, T_IPComm* tNwdafIP, WORD16* pwNwdafPort)
{
    MCS_CHK_NULL_RET(originUri, FALSE);
    MCS_CHK_NULL_RET(tNwdafIP, FALSE);
    MCS_CHK_NULL_RET(pwNwdafPort, FALSE);

    const char* leftBracket = strchr(originUri, '[');
    MCS_CHK_NULL_RET(leftBracket, FALSE);

    const char* rightBracket = strstr(leftBracket, "]:");
    MCS_CHK_NULL_RET(rightBracket, FALSE);

    if (rightBracket - leftBracket - 1 > 63)
    {
        return FALSE;
    }

    char ipv6[64] = {0};
    zte_memcpy_s(ipv6, 64, leftBracket + 1, rightBracket - leftBracket - 1);
    if(TRUE != inet_pton(AF_INET6, ipv6, tNwdafIP->abAddress))
    {
        return FALSE;
    }
    tNwdafIP->bType = IPCOMM_TYPE_IPV6;

    const char* portStart = NULL;
    if(*(rightBracket + 2) >= '0' && *(rightBracket + 2) <= '9')
    {
        portStart = rightBracket + 2;
    }
    return psNcuGetPort(portStart, pwNwdafPort);
}
/* Ended by AICoder, pid:u1ab0q9cfdz14421418709081063e240fd50d59d */

/* Started by AICoder, pid:e882cq97c6v8fd614b2f09819033df443670c675 */
BOOL psNcuGetIPv4AndPortByReportUri(const char* originUri, T_IPComm* tNwdafIP, WORD16* pwNwdafPort)
{
    MCS_CHK_NULL_RET(originUri, FALSE);
    MCS_CHK_NULL_RET(tNwdafIP, FALSE);
    MCS_CHK_NULL_RET(pwNwdafPort, FALSE);

    const char* ipStart = strstr(originUri, "//");
    MCS_CHK_NULL_RET(ipStart, FALSE);

    const char* ipEnd = strchr(ipStart, ':');
    MCS_CHK_NULL_RET(ipEnd, FALSE);

    if (ipEnd - ipStart - 2 > 15)
    {
        return FALSE;
    }

    char ipv4[16] = {0};
    zte_memcpy_s(ipv4, 16, ipStart + 2, ipEnd - ipStart - 2);
    if(TRUE != inet_pton(AF_INET, ipv4, tNwdafIP->abAddress))
    {
        return FALSE;
    }
    tNwdafIP->bType = IPCOMM_TYPE_IPV4;

    const char* portStart = NULL;
    if(*(ipEnd+1) >= '0' && *(ipEnd+1) <= '9')
    {
        portStart = ipEnd + 1;
    }
    return psNcuGetPort(portStart, pwNwdafPort);
}
/* Ended by AICoder, pid:e882cq97c6v8fd614b2f09819033df443670c675 */

/* Started by AICoder, pid:z0b0es53a8d9e15140fa0b32c0ff252f4c40b49f */
BOOL psNcuGetSchemaByReportUri(const char* reportUri, BYTE* bSchema)
{
    MCS_CHK_NULL_RET(reportUri, FALSE);
    MCS_CHK_NULL_RET(bSchema, FALSE);

    if (NULL != strstr(reportUri, "http://"))
    {
        *bSchema = SCHEMA_HTTP;
        return TRUE;
    }

    if (NULL != strstr(reportUri, "https://"))
    {
        *bSchema = SCHEMA_HTTPS;
        return TRUE;
    }

    return FALSE;
}
/* Ended by AICoder, pid:z0b0es53a8d9e15140fa0b32c0ff252f4c40b49f */

BOOL psNcuGetLinkIndexByReportUri(const char* reportUri, T_IPComm* ptNwdafIP,
                                  WORD16* pwNwdafPort, WORD32* pdwClientProfileID, BYTE* pbSchema)
{
    MCS_CHK_NULL_RET(reportUri, FALSE);
    MCS_CHK_NULL_RET(ptNwdafIP, FALSE);
    MCS_CHK_NULL_RET(pwNwdafPort, FALSE);
    MCS_CHK_NULL_RET(pdwClientProfileID, FALSE);
    MCS_CHK_NULL_RET(pbSchema, FALSE);

    if (FALSE == psNcuGetSchemaByReportUri(reportUri, pbSchema))
    {
        return FALSE;
    }

    if (TRUE == psNcuGetIPv6AndPortByReportUri(reportUri, ptNwdafIP, pwNwdafPort))
    {
        *pdwClientProfileID = g_upfConfig.wClientProfileV6ID;
        return TRUE;
    }
    if (TRUE == psNcuGetIPv4AndPortByReportUri(reportUri, ptNwdafIP, pwNwdafPort))
    {
        *pdwClientProfileID = g_upfConfig.wClientProfileV4ID;
        return TRUE;
    }

    return FALSE;
}

std::atomic<WORD32> g_scIndex;
std::atomic<BYTE> g_instIndex;
/* Started by AICoder, pid:b7cf5c74ea768ce14b610a80e0c01e5e080606df */
BOOL psNcuGetHttpLbLogicNoAndInstNo(WORD32 *dwScIndex, BYTE *bInstIndex)
{
    MCS_CHK_NULL_RET(dwScIndex, FALSE);
    MCS_CHK_NULL_RET(bInstIndex, FALSE);

    WORD32 dwMaxLogicNo = 0;
    BYTE   bMaxInstNo   = 0;

    T_ABLInfo tABLInfo = getHttpProxyAbility();

    dwMaxLogicNo = sccuGetSCListCurrentMaxID(&tABLInfo);
    if (0 == dwMaxLogicNo)
    {
        return FALSE;
    }
    *dwScIndex = (g_scIndex++) % dwMaxLogicNo + 1;
    T_R_SC_Table sctable = {0};
    WORD32 loop = 0;
    while (loop < dwMaxLogicNo)
    {
        if(loop >= 255)
        {
            break;
        }
        if (SCCU_OK == sccuGetSCRecordByLogicNo(&tABLInfo, *dwScIndex, &sctable) &&
            (SC_STATE_EQUE_WORK(sctable.dwStatus)))
        {
            break;
        }
        loop++;
        *dwScIndex = (g_scIndex++) % dwMaxLogicNo + 1;
    }

    sccuGetSCMaxInstNumByLogicNo(&tABLInfo, *dwScIndex, &bMaxInstNo);
    if (0 == bMaxInstNo)
    {
        *bInstIndex = 1;
    }
    else
    {
        *bInstIndex = (g_instIndex++) % bMaxInstNo + 1;
    }

    return TRUE;
}
/* Ended by AICoder, pid:b7cf5c74ea768ce14b610a80e0c01e5e080606df */

/* Started by AICoder, pid:d93d2ra8b8336221400608a4b04a9a7cb0a223cd */
BOOL psNcuBuildHttpLinkDetectMsg(string &msgBuf, T_MediaProcThreadPara *ptMediaProcThreadPara, T_psNcuHttpLink *ptNcuHttpLinkCtx)
{
    MCS_CHK_NULL_RET(ptNcuHttpLinkCtx, FALSE);
    MCS_CHK_NULL_RET(ptMediaProcThreadPara, FALSE);

    T_psNcuMcsPerform *ptMcsNcuPerform = psGetPerform();
    MCS_CHK_NULL_RET(ptMcsNcuPerform, FALSE);

    App2HttpDetect httpDetect;

    auto appInfo = httpDetect.mutable_appinfo();
    MCS_CHK_NULL_RET(appInfo, FALSE);
    auto jid = appInfo->mutable_jid();
    MCS_CHK_NULL_RET(jid, FALSE);

    WORD32 dwJno = g_ptMediaProcThreadPara->bIntraThreadNo + 1;
    jid->set_jno(dwJno);
    WORD32 dwComid = ncuGetSelfCommId();
    jid->set_comid(dwComid);

    auto instanceKey = httpDetect.mutable_instancekey();
    MCS_CHK_NULL_RET(instanceKey, FALSE);
    CHAR cKey[16] = {0};
    XOS_snprintf(cKey, 16, "ncu%d", ptMediaProcThreadPara->bThreadNo);
    string strKey((const char*)cKey);
    *instanceKey = strKey;

    auto remote = httpDetect.mutable_remote();
    MCS_CHK_NULL_RET(remote, FALSE);
    remote->set_port(ptNcuHttpLinkCtx->wNwdafPort);
    auto ip = remote->mutable_ip();
    MCS_CHK_NULL_RET(ip, FALSE);
    CHAR cIPAddr[64] = {0};
    if (IPCOMM_TYPE_IPV6 == ptNcuHttpLinkCtx->tNwdafIP.bType)
    {
        inet_ntop(AF_INET6, ptNcuHttpLinkCtx->tNwdafIP.abAddress, cIPAddr, 64);
    }
    else if (IPCOMM_TYPE_IPV4 == ptNcuHttpLinkCtx->tNwdafIP.bType)
    {
        inet_ntop(AF_INET, ptNcuHttpLinkCtx->tNwdafIP.abAddress, cIPAddr, 64);
    }
    string strIPAddr((const char*)cIPAddr);
    *ip = strIPAddr;

    httpDetect.set_clientpf(ptNcuHttpLinkCtx->dwClientProfileID);
    httpDetect.set_timeforack(5);

    /* 消息构造完后进行序列化 */
    if(!httpDetect.SerializeToString(&msgBuf))
    {
        MCS_LOC_STAT_EX(ptMcsNcuPerform, HttpDetectSerializeFail, 1);
        return FALSE;
    }
    return TRUE;
}
/* Ended by AICoder, pid:d93d2ra8b8336221400608a4b04a9a7cb0a223cd */

VOID psNcuSendApp2HttpReqStat(WORD32 dwMsgID, WORD32 dwScLogicNo)
{
    if (EV_SERVICE_2_HTTP_PB_REQUEST != dwMsgID)
    {
        return;
    }

    T_psNcuMcsPerform *ptMcsNcuPerform = psGetPerform();
    MCS_CHK_NULL_VOID(ptMcsNcuPerform);

    if (1 == dwScLogicNo)
    {
        MCS_LOC_STAT_EX(ptMcsNcuPerform, SendApp2HttpReqLBSc1, 1);
    }
    else if (2 == dwScLogicNo)
    {
        MCS_LOC_STAT_EX(ptMcsNcuPerform, SendApp2HttpReqLBSc2, 1);
    }
    else
    {
        MCS_LOC_STAT_EX(ptMcsNcuPerform, SendApp2HttpReqLBScOther, 1);
    }

    return;
}

BOOL psNcuSendMsg2HttpLb(WORD32 dwMsgID, CHAR *msg, WORD16 msgLen, T_psNcuHttpLink *ptNcuHttpLinkCtx)
{
    DEBUG_TRACE(DEBUG_LOW, "Send msg to HttpLb begin.\n");

    MCS_CHK_NULL_RET(msg, FALSE);
    T_psNcuMcsPerform *ptMcsNcuPerform = psGetPerform();
    MCS_CHK_NULL_RET(ptMcsNcuPerform, FALSE);

    WORD32 dwScIndex = 0;
    BYTE bInstIndex = 0;
    if (NULL == ptNcuHttpLinkCtx)
    {
        psNcuGetHttpLbLogicNoAndInstNo(&dwScIndex, &bInstIndex);
    }
    else
    {
        dwScIndex  = ptNcuHttpLinkCtx->dwPollingLogicNo;
        bInstIndex = ptNcuHttpLinkCtx->bPollingInst;
    }

    DEBUG_TRACE(DEBUG_LOW, "Discovery ABLInfo.\n");
    WORD32 dwRet = 0;
    T_ABLInfo tABLInfo = getHttpProxyAbility();

    DEBUG_TRACE(DEBUG_LOW, "bSoftThreadNo: %u, bIntraThreadNo: %u.\n", g_ptMediaProcThreadPara->bThreadNo, g_ptMediaProcThreadPara->bIntraThreadNo);
    JID_UTIPC tSender = {0};
    tSender.dwJno = g_ptMediaProcThreadPara->bIntraThreadNo + 1;
    tSender.dwComId = ncuGetSelfCommId();
    DEBUG_TRACE(DEBUG_LOW, "Sender Jno:%u, ComId: %u.\n", tSender.dwJno, tSender.dwComId);

    DEBUG_TRACE(DEBUG_LOW, "Get ability JNO.\n");
    JID_UTIPC tReceiver = {0};
    dwRet = sccuGetAbilityJNOByABLInfo(&tABLInfo, &tReceiver.dwJno);
    if (SCCU_OK != dwRet)
    {
        MCS_LOC_STAT_EX(ptMcsNcuPerform, SendMsg2HttpLbGetABLJNOFail, 1);
        return FALSE;
    }
    tReceiver.dwJno = ((tReceiver.dwJno & 0xFFC00) | bInstIndex);
    tReceiver.dwComId = psGetHttpLbCommidByLogicNo(dwScIndex);
    DEBUG_TRACE(DEBUG_LOW, "Receiver Jno:%u, ComId: %u.\n", tReceiver.dwJno, tReceiver.dwComId);

    DEBUG_TRACE(DEBUG_LOW, "Generate packet.\n");
    PS_PACKET *ptPacket = psCSSGenPkt((BYTE *)msg, msgLen, g_ptMediaProcThreadPara->bThreadNo);
    if (NULL == ptPacket)
    {
        MCS_LOC_STAT_EX(ptMcsNcuPerform, SendMsg2HttpLbPacketNull, 1);
        return FALSE;
    }

    psNcuSendApp2HttpReqStat(dwMsgID, dwScIndex);
    DEBUG_TRACE(DEBUG_LOW, "Ncu send msg to tipc.\n");
    dwRet = psCssNCUSendToTipc(ptPacket, &tReceiver, &tSender, dwMsgID);
    if (CSS_OK != dwRet)
    {
        MCS_LOC_STAT_EX(ptMcsNcuPerform, SendMsg2HttpLbFail, 1);
        return FALSE;
    }

    DEBUG_TRACE(DEBUG_LOW, "Send message to HttpLb succ.\n");
    return TRUE;
}

/* Started by AICoder, pid:z38afu06e83a50314b300bad40294c167f7357ca */
BOOL psNcuSendHttpLinkDetectMsg(T_MediaProcThreadPara *ptMediaProcThreadPara, T_psNcuHttpLink *ptNcuHttpLinkCtx)
{
    if (NULL == ptNcuHttpLinkCtx || NULL == ptMediaProcThreadPara)
    {
        return FALSE;
    }

    string msgBuf("");
    if(FALSE == psNcuBuildHttpLinkDetectMsg(msgBuf, ptMediaProcThreadPara, ptNcuHttpLinkCtx))
    {
        return FALSE;
    }
    return psNcuSendMsg2HttpLb(EV_SERVICE_2_HTTP_PB_DETECT, (char *)msgBuf.c_str(), msgBuf.size(), ptNcuHttpLinkCtx);
}
/* Ended by AICoder, pid:z38afu06e83a50314b300bad40294c167f7357ca */

/* Started by AICoder, pid:e03feu9354ucc56144c10b28502ec22ce0239466 */
T_psNcuHttpLink *psNcuGetHttpLinkByReportUri(const char* reportUri, WORD32 dwhDB)
{
    DEBUG_TRACE(DEBUG_LOW, "Get Http Link by reportUri.\n");
    if (NULL == reportUri) {
        return NULL;
    }

    T_IPComm tNwdafIP = {0};
    WORD32   dwClientProfileID = 0;
    WORD16   wNwdafPort = 0;
    BYTE     bSchema = 0;

    DEBUG_TRACE(DEBUG_LOW, "Get remote host by reportUri.\n");
    if (FALSE == psNcuGetLinkIndexByReportUri(reportUri, &tNwdafIP, &wNwdafPort, &dwClientProfileID, &bSchema))
    {
        return NULL;
    }

    DEBUG_TRACE(DEBUG_LOW, "Query HttpLink.\n");
    T_psNcuHttpLink *ptNcuHttpLinkCtx = psQueryHttpLinkByUniqueIdx(&tNwdafIP, dwClientProfileID, wNwdafPort, bSchema, dwhDB);
    if (NULL == ptNcuHttpLinkCtx)
    {
        DEBUG_TRACE(DEBUG_LOW, "Create HttpLink.\n");
        ptNcuHttpLinkCtx = psCreateHttpLinkByUniqueIdx(&tNwdafIP, dwClientProfileID, wNwdafPort, bSchema, dwhDB);
        if (NULL != ptNcuHttpLinkCtx)
        {
            psNcuGetHttpLbLogicNoAndInstNo(&ptNcuHttpLinkCtx->dwPollingLogicNo, &ptNcuHttpLinkCtx->bPollingInst);
        }
    }
    else
    {
        ptNcuHttpLinkCtx->dwUpdateTimeStamp = psFtmGetPowerOnSec();
    }
    return ptNcuHttpLinkCtx;
}
/* Ended by AICoder, pid:e03feu9354ucc56144c10b28502ec22ce0239466 */

VOID psNcuResetReportLinkFaultFlag(T_psNcuSessionCtx* ptSession, T_NcuSynSubInfo* ptSubScribeInfo)
{
    MCS_CHK_NULL_VOID(ptSession);
    MCS_CHK_NULL_VOID(ptSubScribeInfo);

    MCS_CHK_VOID_LIKELY(0 == ptSession->bHasReportLinkFault);

    if (0 == memcmp(ptSession->Uri_Ana, ptSubScribeInfo->aucReportUri, AUCREPORTURI_MAXLEN))
    {
        return;
    }

    T_IPComm tNwdafIP_sess = {0};
    WORD32   dwCltProfID_sess = 0;
    WORD16   wNwdafPort_sess = 0;
    BYTE     bSchema_sess = 0;
    if (FALSE == psNcuGetLinkIndexByReportUri(ptSession->Uri_Ana, &tNwdafIP_sess,
                                              &wNwdafPort_sess, &dwCltProfID_sess, &bSchema_sess))
    {
        return;
    }

    T_IPComm tNwdafIP_sub = {0};
    WORD32   dwCltProfID_sub = 0;
    WORD16   wNwdafPort_sub = 0;
    BYTE     bSchema_sub = 0;
    if (FALSE == psNcuGetLinkIndexByReportUri(ptSubScribeInfo->aucReportUri, &tNwdafIP_sub,
                                              &wNwdafPort_sub, &dwCltProfID_sub, &bSchema_sub))
    {
        return;
    }

    if ((dwCltProfID_sess != dwCltProfID_sub) ||
        (wNwdafPort_sess != wNwdafPort_sub) ||
        (bSchema_sess != bSchema_sub) ||
        (0 != memcmp(&tNwdafIP_sess, &tNwdafIP_sub, sizeof(T_IPComm))))
    {
        ptSession->bHasReportLinkFault = 0;
    }

    return;
}

WORD32 psNcuGetHttpLinkInfo(T_MediaProcThreadPara *ptMediaProcThreadPara, T_NcuSynSubInfo *ptSubScribeInfo)
{
    DEBUG_TRACE(DEBUG_LOW, "Get http link info for QosAna.\n");

    MCS_CHK_NULL_RET(ptSubScribeInfo, MCS_RET_FAIL);
    MCS_CHK_NULL_RET(ptMediaProcThreadPara, MCS_RET_FAIL);
    T_psNcuMcsPerform *ptMcsNcuPerform = (T_psNcuMcsPerform*)(ptMediaProcThreadPara->ptMcsStatPointer);
    MCS_CHK_NULL_RET(ptMcsNcuPerform, MCS_RET_FAIL);

    MCS_LOC_STAT_EX(ptMcsNcuPerform, GetHttpLinkInfoBegin, 1);
    if (enumQosAna != ptMediaProcThreadPara->bSubType)
    {
        MCS_LOC_STAT_EX(ptMcsNcuPerform, SubTypeNotQosAna, 1);
        return MCS_RET_SUCCESS;
    }

    T_IPComm tNwdafIP = {0};
    WORD32   dwClientProfileID = 0;
    WORD16   wNwdafPort = 0;
    BYTE     bSchema = 0;

    DEBUG_TRACE(DEBUG_LOW, "Get remote host By ReportUri.\n");
    if (FALSE == psNcuGetLinkIndexByReportUri(ptSubScribeInfo->aucReportUri, &tNwdafIP,
                                              &wNwdafPort, &dwClientProfileID, &bSchema))
    {
        MCS_LOC_STAT_EX(ptMcsNcuPerform, GetHttpLinkIndexFail, 1);
        return MCS_RET_FAIL;
    }

    WORD32 dwhDB = ptMediaProcThreadPara->dwhDBByThreadNo;
    T_psNcuHttpLink *ptNcuHttpLinkCtx = psQueryHttpLinkByUniqueIdx(&tNwdafIP, dwClientProfileID, wNwdafPort, bSchema, dwhDB);
    if (NULL != ptNcuHttpLinkCtx)
    {
        MCS_LOC_STAT_EX(ptMcsNcuPerform, HttpLinkCtxExist, 1);
        DEBUG_TRACE(DEBUG_LOW, "Http link ctx exists.\n");
        ptNcuHttpLinkCtx->dwUpdateTimeStamp = psFtmGetPowerOnSec();
        if (HTTP_REMOTE_VALID != ptNcuHttpLinkCtx->remoteStatus)
        {
            MCS_LOC_STAT_EX(ptMcsNcuPerform, HttpLinkCtxExistNotValid, 1);
            psNcuSendHttpLinkDetectMsg(ptMediaProcThreadPara, ptNcuHttpLinkCtx);
        }
        return MCS_RET_SUCCESS;
    }

    DEBUG_TRACE(DEBUG_LOW, "Create http link ctx.\n");
    ptNcuHttpLinkCtx = psCreateHttpLinkByUniqueIdx(&tNwdafIP, dwClientProfileID, wNwdafPort, bSchema, dwhDB);
    if (NULL == ptNcuHttpLinkCtx)
    {
        MCS_LOC_STAT_EX(ptMcsNcuPerform, CrtHttpLinkCtxFail, 1);
        return MCS_RET_FAIL;
    }

    DEBUG_TRACE(DEBUG_LOW, "Get httplb scLogicNO and InstNo.\n");
    if (FALSE == psNcuGetHttpLbLogicNoAndInstNo(&ptNcuHttpLinkCtx->dwPollingLogicNo, &ptNcuHttpLinkCtx->bPollingInst))
    {
        MCS_LOC_STAT_EX(ptMcsNcuPerform, GetScLogicNoAndInstNoFail, 1);
        return MCS_RET_FAIL;
    }

    DEBUG_TRACE(DEBUG_LOW, "Send http link detect msg.\n");
    if (FALSE == psNcuSendHttpLinkDetectMsg(ptMediaProcThreadPara, ptNcuHttpLinkCtx))
    {
        MCS_LOC_STAT_EX(ptMcsNcuPerform, SendHttpDetectMsgFail, 1);
        return MCS_RET_FAIL;
    }

    MCS_LOC_STAT_EX(ptMcsNcuPerform, GetHttpLinkInfoSucc, 1);
    DEBUG_TRACE(DEBUG_LOW, "Get http link info success.\n");
    return MCS_RET_SUCCESS;
}

VOID psNcuGetRemoteHostByLinkCtx(T_psNcuHttpLink* ptNcuHttpLinkCtx, HTTP_REMOTE_HOST_INFO *ptRemoteHost)
{
    MCS_CHK_NULL_VOID(ptNcuHttpLinkCtx);
    MCS_CHK_NULL_VOID(ptRemoteHost);

    ptRemoteHost->dwProfileId = ptNcuHttpLinkCtx->dwClientProfileID;
    ptRemoteHost->wRemotePort = ptNcuHttpLinkCtx->wNwdafPort;

    if (SCHEMA_HTTPS == ptNcuHttpLinkCtx->bSchema)
    {
        XOS_strncpy(ptRemoteHost->chSchema, "https", HTTP_MAX_SCHEMA_LENGTH);
    }
    else
    {
        XOS_strncpy(ptRemoteHost->chSchema, "http", HTTP_MAX_SCHEMA_LENGTH);
    }

    if (IPCOMM_TYPE_IPV6 == ptNcuHttpLinkCtx->tNwdafIP.bType)
    {
        inet_ntop(AF_INET6, ptNcuHttpLinkCtx->tNwdafIP.abAddress, ptRemoteHost->chRemoteIp, HTTP_MAX_IPADDRESS_LEN);
    }
    else if (IPCOMM_TYPE_IPV4 == ptNcuHttpLinkCtx->tNwdafIP.bType)
    {
        inet_ntop(AF_INET, ptNcuHttpLinkCtx->tNwdafIP.abAddress, ptRemoteHost->chRemoteIp, HTTP_MAX_IPADDRESS_LEN);
    }

    JOB_TRACE(DEBUG_LOW, JOB_TYPE_MCS_MANAGE, "dwProfileId: %u, wRemotePort: %u, chRemoteIp: %s, chSchema: %s.\n",
              ptRemoteHost->dwProfileId, ptRemoteHost->wRemotePort, ptRemoteHost->chRemoteIp, ptRemoteHost->chSchema);
    return;
}

VOID psNcuRemoteHostStatusChgStat(HTTP_REMOTE_STATUS oldStatus, HTTP_REMOTE_STATUS newStatus)
{
    if (HTTP_REMOTE_VALID == newStatus)
    {
        JOB_LOC_STAT_ADDONE(qwNcuRemoteStatusValid);
        if (HTTP_REMOTE_VALID != oldStatus)
        {
            JOB_LOC_STAT_ADDONE(qwRemoteStatusChangeToValid);
        }
    }
    else if (HTTP_REMOTE_INVALID == newStatus)
    {
        JOB_LOC_STAT_ADDONE(qwNcuRemoteStatusInvalid);
        if (HTTP_REMOTE_INVALID != oldStatus)
        {
            JOB_LOC_STAT_ADDONE(qwRemoteStatusChangeToInvalid);
        }
    }
    else
    {
        JOB_LOC_STAT_ADDONE(qwNcuRemoteNoStatus);
        if (HTTP_NO_STATUS != oldStatus)
        {
            JOB_LOC_STAT_ADDONE(qwRemoteStatusChangeToNoStatus);
        }
    }

    return;
}

VOID psNcuGetRemoteHostStatus(T_psNcuHttpLink* ptNcuHttpLinkCtx)
{
    MCS_CHK_NULL_VOID(ptNcuHttpLinkCtx);

    if (0 == g_upfConfig.daHttpCfg.bCheckSwitch)
    {
        return;
    }

    HTTP_GET_REMOTE_STATUS_REQ tReq = {0};
    HTTP_GET_REMOTE_STATUS_ACK tAck = {0};
    tReq.dwHostsNum = 1;
    psNcuGetRemoteHostByLinkCtx(ptNcuHttpLinkCtx, &tReq.tRemoteHosts[0]);

    if(!http_get_remote_host_status(&tReq, &tAck) || 0 == tAck.dwStatusNum)
    {
        JOB_LOC_STAT_ADDONE(qwNcuGetRemoteStatusFail);
        return;
    }
    JOB_TRACE(DEBUG_LOW, JOB_TYPE_MCS_MANAGE, "StatusNum = %u, RemoteStatus = %u\n", tAck.dwStatusNum, tAck.enRemoteStatus[0]);

    psNcuRemoteHostStatusChgStat(ptNcuHttpLinkCtx->remoteStatus, tAck.enRemoteStatus[0]);
    if (ptNcuHttpLinkCtx->remoteStatus != tAck.enRemoteStatus[0])
    {
        HTTP_REMOTE_STATUS oldRemoteStatus = ptNcuHttpLinkCtx->remoteStatus;
        ptNcuHttpLinkCtx->remoteStatus = tAck.enRemoteStatus[0];
        WORD32 dwCurrentTime = psFtmGetPowerOnSec();
        if (0 == ptNcuHttpLinkCtx->dwLogRptTimeStamp ||
            (dwCurrentTime >= ptNcuHttpLinkCtx->dwLogRptTimeStamp + g_LogRptInterval))
        {
            CHAR content[UPFLOG_MAX_CONTENTLEN] = {0};
            zte_snprintf_s(content, UPFLOG_MAX_CONTENTLEN, "Remote status changes from %u to %u. ProfileId: %u, RemoteIp: %s, RemotePort: %u, Schema: %s.",
                           oldRemoteStatus, ptNcuHttpLinkCtx->remoteStatus, tReq.tRemoteHosts[0].dwProfileId,
                           tReq.tRemoteHosts[0].chRemoteIp, tReq.tRemoteHosts[0].wRemotePort, tReq.tRemoteHosts[0].chSchema);
            upfLog(0xFF, LOGTYPE_DIAGNOSE, "upf_ncu", UPF_SYSLOG_CODE_NCU_REMOTESTATUS_CHANGE, LOGLEVEL_LOG_ERROR, content);
            ptNcuHttpLinkCtx->dwLogRptTimeStamp = dwCurrentTime;
        }
    }

    return;
}

VOID psNcuClearHttpScheduleLink(T_psNcuHttpLink* ptNcuHttpLinkCtx)
{
    MCS_CHK_NULL_VOID(ptNcuHttpLinkCtx);

    ptNcuHttpLinkCtx->dwLinkIndex        = 0;
    ptNcuHttpLinkCtx->dwLogicNo          = 0;
    ptNcuHttpLinkCtx->tHttpLbJid.dwJno   = 0;
    ptNcuHttpLinkCtx->tHttpLbJid.dwComId = INVALID_COMM_ID;
    zte_memset_s(ptNcuHttpLinkCtx->bSequence, HTTP_MAX_SEQUENCE_LEN, 0, HTTP_MAX_SEQUENCE_LEN);
    return;
}

/* Started by AICoder, pid:75be2i5212gf44e1496f0ad2f044e2515e15841e */
VOID psNcuGetHttpScheduleLink(T_psNcuHttpLink* ptNcuHttpLinkCtx, BYTE bSoftThreadNo)
{
    if (NULL == ptNcuHttpLinkCtx) {
        return;
    }

    if (HTTP_REMOTE_INVALID == ptNcuHttpLinkCtx->remoteStatus)
    {
        psNcuClearHttpScheduleLink(ptNcuHttpLinkCtx);
        return;
    }

    HTTP_SCHEDULE_LINK_REQ_EX tReq = {0};
    HTTP_SCHEDULE_LINK_ACK_EX tAck;
    zte_memset_s(&tAck, sizeof(tAck), 0, sizeof(tAck));

    psNcuGetRemoteHostByLinkCtx(ptNcuHttpLinkCtx, &tReq.tRemoteHost);

    zte_snprintf_s(tReq.userId, HTTP_MAX_USERID_LEN, "%u", bSoftThreadNo);
    tReq.userIdLen = zte_strnlen_s(tReq.userId, HTTP_MAX_USERID_LEN);
    JOB_TRACE(DEBUG_LOW, JOB_TYPE_MCS_MANAGE, "userId = %s, userIdLen = %u\n", tReq.userId, tReq.userIdLen);
    tReq.userIdScope = USERID_FOR_SC_AND_THREAD;

    if (!http_schedule_link_and_sc_ex(&tReq, &tAck) || (HTTP_LINK_INVALID == tAck.enResult))
    {
        psNcuClearHttpScheduleLink(ptNcuHttpLinkCtx);
        JOB_LOC_STAT_ADDONE(qwNcuGetHttpScheduleLinkFail);
        return;
    }

    ptNcuHttpLinkCtx->dwLinkIndex        = tAck.dwLinkIndex;
    ptNcuHttpLinkCtx->dwLogicNo          = tAck.dwLogicNo;
    ptNcuHttpLinkCtx->tHttpLbJid.dwJno   = tAck.tScJid.dwJno;
    ptNcuHttpLinkCtx->tHttpLbJid.dwComId = tAck.tScJid.dwComId;
    zte_memcpy_s(ptNcuHttpLinkCtx->bSequence, HTTP_MAX_SEQUENCE_LEN, tAck.bSequence, HTTP_MAX_SEQUENCE_LEN);
    JOB_TRACE(DEBUG_LOW, JOB_TYPE_MCS_MANAGE, "dwLinkIndex: %u, dwLogicNo: %u.\n", ptNcuHttpLinkCtx->dwLinkIndex, ptNcuHttpLinkCtx->dwLogicNo);
    JOB_TRACE(DEBUG_LOW, JOB_TYPE_MCS_MANAGE, "dwJno: %u, dwComId: %u.\n", ptNcuHttpLinkCtx->tHttpLbJid.dwJno, ptNcuHttpLinkCtx->tHttpLbJid.dwComId);

    JOB_LOC_STAT_ADDONE(qwNcuGetHttpScheduleLinkSucc);
    return;
}
/* Ended by AICoder, pid:75be2i5212gf44e1496f0ad2f044e2515e15841e */

BOOL psNcuDetectAndGetValidLink(void)
{
    JOB_TRACE(DEBUG_LOW, JOB_TYPE_MCS_MANAGE, "psNcuDetectAndGetValidLink Begin.\n");
    T_PSThreadInstAffinityPara tInstPara = {0};
    WORD32 dwRet = psGetMediaThreadInfo(THDM_MEDIA_TYPE_PFU_MEDIA_PROC, &tInstPara);
    if (THDM_SUCCESS != dwRet)
    {
        JOB_LOC_STAT_ADDONE(qwNcuGetMediaThreadInfoFail);
        return FALSE;
    }

    JOB_TRACE(DEBUG_LOW, JOB_TYPE_MCS_MANAGE, "Inst Num: %u.\n", tInstPara.bInstNum);
    for(BYTE bInstNum = 0; bInstNum < tInstPara.bInstNum; bInstNum++)
    {
        BYTE bSoftThreadNo  = tInstPara.atThreadParaList[bInstNum].bSoftThreadNo;
        BYTE bIntraThreadNo = tInstPara.atThreadParaList[bInstNum].bIntraThreadNo;
        WORD32 dwDbHandle = _NCU_GET_DBHANDLE(bSoftThreadNo);
        for (WORD32 dwCtxId = 1; dwCtxId <= ctR_ncu_httplink_CAPACITY; dwCtxId++)
        {
            T_psNcuHttpLink* ptNcuHttpLinkCtx = psMcsGetHttpLinkCtxById(dwCtxId, dwDbHandle);
            if (NULL == ptNcuHttpLinkCtx)
            {
                JOB_LOC_STAT_ADDONE(qwNcuHttpLinkCtxNull);
                continue;
            }

            JOB_TRACE(DEBUG_LOW, JOB_TYPE_MCS_MANAGE, "bSoftThreadNo: %u, bIntraThreadNo: %u, dwCtxId: %u.\n", bSoftThreadNo, bIntraThreadNo, dwCtxId);
            WORD32 dwCurrentTime = psFtmGetPowerOnSec();
            if (dwCurrentTime >= ptNcuHttpLinkCtx->dwUpdateTimeStamp)
            {
                if ((dwCurrentTime - ptNcuHttpLinkCtx->dwUpdateTimeStamp) >= (g_upfConfig.daHttpCfg.dwAgingTime * 60))
                {
                    psDelHttpLinkByUniqueIdx(&ptNcuHttpLinkCtx->tNwdafIP, ptNcuHttpLinkCtx->dwClientProfileID,
                                             ptNcuHttpLinkCtx->wNwdafPort, ptNcuHttpLinkCtx->bSchema, dwDbHandle);
                    JOB_TRACE(DEBUG_LOW, JOB_TYPE_MCS_MANAGE, "ptNcuHttpLinkCtx Delete.\n");
                    JOB_LOC_STAT_ADDONE(qwNcuHttpLinkCtxRscRecycle);
                    continue;
                }
            }

            psNcuGetRemoteHostStatus(ptNcuHttpLinkCtx);
            psNcuGetHttpScheduleLink(ptNcuHttpLinkCtx, bSoftThreadNo);
        }
    }

    return TRUE;
}

/* Started by AICoder, pid:i9526xa879if99f14a380ac2a01c0820f9103cec */
bool psNcuIsHttpLinkFault(T_psNcuHttpLink* ptNcuHttpLinkCtx, T_psNcuSessionCtx* ptSessionCtx)
{
    if (NULL == ptNcuHttpLinkCtx || NULL == ptSessionCtx)
    {
        return false;
    }

    T_psNcuMcsPerform *ptMcsNcuPerform = psGetPerform();
    MCS_CHK_NULL_RET(ptMcsNcuPerform, false);

    if (HTTP_REMOTE_VALID != ptNcuHttpLinkCtx->remoteStatus)
    {
        MCS_LOC_STAT_EX(ptMcsNcuPerform, IsHttpLinkFaultStatusNotValid, 1);
        if (0 == g_upfConfig.daHttpCfg.bCheckSwitch)
        {
            MCS_LOC_STAT_EX(ptMcsNcuPerform, IsHttpLinkFaultSwitchClose, 1);
            ptNcuHttpLinkCtx->remoteStatus = HTTP_REMOTE_VALID;
            return false;
        }

        MCS_LOC_STAT_EX(ptMcsNcuPerform, IsHttpLinkFaultTrue, 1);
        return true;
    }

    MCS_LOC_STAT_EX(ptMcsNcuPerform, IsHttpLinkFaultStatusValid, 1);
    ptSessionCtx->bHasReportLinkFault = 0;
    return false;
}
/* Ended by AICoder, pid:i9526xa879if99f14a380ac2a01c0820f9103cec */

bool psNcuIsNeedReportLinkFault(T_psNcuSessionCtx* ptSessionCtx)
{
    MCS_CHK_NULL_RET(ptSessionCtx, false);
    T_psNcuMcsPerform *ptMcsNcuPerform = psGetPerform();
    MCS_CHK_NULL_RET(ptMcsNcuPerform, false);

    if (NCCHECKFAILUREPOLICY_NONE == g_upfConfig.daHttpCfg.bCheckFailurePolicy)
    {
        MCS_LOC_STAT_EX(ptMcsNcuPerform, IsNeedReportLinkFaultNone, 1);
        return false;
    }

    if (NCREPORTPOLICY_ONCE == g_upfConfig.daHttpCfg.bReportPolicy)
    {
        if (0 == ptSessionCtx->bHasReportLinkFault)
        {
            MCS_LOC_STAT_EX(ptMcsNcuPerform, IsNeedReportLinkOnceTrue, 1);
            return true;
        }
        MCS_LOC_STAT_EX(ptMcsNcuPerform, IsNeedReportLinkHasReport, 1);
        return false;
    }

   if (NCREPORTPOLICY_ALWAYS == g_upfConfig.daHttpCfg.bReportPolicy)
    {
        MCS_LOC_STAT_EX(ptMcsNcuPerform, IsNeedReportLinkAlways, 1);
        return true;
    }

    MCS_LOC_STAT_EX(ptMcsNcuPerform, IsNeedReportLinkHasFalse, 1);
    return false;
}

WORD32 psNcuSendNotifyReqToNwdaf(BYTE *msg, WORD16 msgLen, WORD32 dwMsgID, T_psNcuHttpLink* ptNcuHttpLinkCtx)
{
    MCS_CHK_NULL_RET(msg, MCS_RET_FAIL);
    T_psNcuMcsPerform *ptMcsNcuPerform = psGetPerform();
    MCS_CHK_NULL_RET(ptMcsNcuPerform, MCS_RET_FAIL);

    MCS_LOC_STAT_EX(ptMcsNcuPerform, SendQosAnaNotifyBegin, 1);

    DEBUG_TRACE(DEBUG_LOW, "psNcuSendNotifyReqToNwdaf Begin.\n");

    DEBUG_TRACE(DEBUG_LOW, "bSoftThreadNo: %u, bIntraThreadNo: %u.\n", g_ptMediaProcThreadPara->bThreadNo, g_ptMediaProcThreadPara->bIntraThreadNo);

    if (NULL != ptNcuHttpLinkCtx &&
        0 != ptNcuHttpLinkCtx->tHttpLbJid.dwJno &&
        INVALID_COMM_ID != ptNcuHttpLinkCtx->tHttpLbJid.dwComId)
    {
        PS_PACKET *ptPacket = psCSSGenPkt(msg, msgLen, g_ptMediaProcThreadPara->bThreadNo);
        if (NULL == ptPacket)
        {
            MCS_LOC_STAT_EX(ptMcsNcuPerform, SendQosAnaNotifyPktNull, 1);
            return MCS_RET_FAIL;
        }

        JID_UTIPC tSender = {0};
        JID_UTIPC tReceiver = {0};
        tSender.dwJno     = g_ptMediaProcThreadPara->bIntraThreadNo + 1;
        tSender.dwComId   = ncuGetSelfCommId();
        DEBUG_TRACE(DEBUG_LOW, "Sender Jno:%u, ComId: %u.\n", tSender.dwJno, tSender.dwComId);
        tReceiver.dwJno   = ptNcuHttpLinkCtx->tHttpLbJid.dwJno;
        tReceiver.dwComId = ptNcuHttpLinkCtx->tHttpLbJid.dwComId;
        DEBUG_TRACE(DEBUG_LOW, "Receiver Jno:%u, ComId: %u.\n", tReceiver.dwJno, tReceiver.dwComId);

        psNcuSendApp2HttpReqStat(dwMsgID, ptNcuHttpLinkCtx->dwPollingLogicNo);
        WORD32 dwRet = psCssNCUSendToTipc(ptPacket, &tReceiver, &tSender, dwMsgID);
        if (CSS_OK != dwRet)
        {
            MCS_LOC_STAT_EX(ptMcsNcuPerform, SendQosAnaNotifyByLinkFail, 1);
            return MCS_RET_FAIL;
        }
        MCS_LOC_STAT_EX(ptMcsNcuPerform, SendQosAnaNotifyByLinkSucc, 1);
        return MCS_RET_SUCCESS;
    }

    if(FALSE == psNcuSendMsg2HttpLb(EV_SERVICE_2_HTTP_PB_REQUEST, (CHAR *)msg, msgLen, ptNcuHttpLinkCtx))
    {
        MCS_LOC_STAT_EX(ptMcsNcuPerform, SendQosAnaNotifyByPollingFail, 1);
        return MCS_RET_FAIL;
    }

    MCS_LOC_STAT_EX(ptMcsNcuPerform, SendQosAnaNotifyByPollingSucc, 1);
    DEBUG_TRACE(DEBUG_LOW, "psNcuSendNotifyReqToNwdaf Succ.\n");
    return MCS_RET_SUCCESS;
}

WORD32 psNcuReportLinkFaultToPfu(T_psNcuSessionCtx* ptSessionCtx)
{
    MCS_CHK_NULL_RET(ptSessionCtx, MCS_RET_FAIL);
    MCS_CHK_NULL_RET(g_ptMediaProcThreadPara, MCS_RET_FAIL);
    T_psNcuMcsPerform *ptMcsNcuPerform = psGetPerform();
    MCS_CHK_NULL_RET(ptMcsNcuPerform, MCS_RET_FAIL);

    MCS_LOC_STAT_EX(ptMcsNcuPerform, ReportLinkFaultToPfuBegin, 1);
    WORD32 dwRet = MCS_RET_FAIL;

    T_NCU_PFU_HTTP_LINK_RPT tLinkFaultData = {0};
    tLinkFaultData.ddwUpseid = ptSessionCtx->ddwUPSeid;

    WORD32 dwPfuThreadNo = GET_THREAD_FROM_UPSEID(ptSessionCtx->ddwUPSeid);
    WORD32 dwGroupId     = GET_USERGROUP_FROM_UPSEID(ptSessionCtx->ddwUPSeid);
    WORD32 dwPfuCommId   = ncuGetPfuCommidByGroup(dwGroupId);
    if(INVALID_COMM_ID == dwPfuCommId)
    {
        MCS_LOC_STAT_EX(ptMcsNcuPerform, ReportLinkFaultInvalidCommId, 1);
        return MCS_RET_FAIL;
    }

    dwRet = upfSendNFFMsgToSc((BYTE *)&tLinkFaultData, sizeof(T_NCU_PFU_HTTP_LINK_RPT), EV_MSG_NCU_TO_PFU_HTTP_LINK_RPT,
                              dwPfuCommId, g_ptMediaProcThreadPara->bThreadNo, dwPfuThreadNo);
    if (UPF_RET_SUCCESS != dwRet)
    {
        MCS_LOC_STAT_EX(ptMcsNcuPerform, ReportLinkFaultToPfuFail, 1);
        return MCS_RET_FAIL;
    }
    ptSessionCtx->bHasReportLinkFault = 1;
    MCS_LOC_STAT_EX(ptMcsNcuPerform, ReportLinkFaultToPfuSucc, 1);
    return MCS_RET_SUCCESS;
}

UPF_HELP_REG("ncu", "set Log Report Interval",
VOID psSetLogRptInterval(WORD32 dwLogRptInterval))
{
    g_LogRptInterval = dwLogRptInterval;
    return;
}
