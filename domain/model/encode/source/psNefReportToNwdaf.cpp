#include "ps_mcs_trace.h"
#include "ps_ncu_dataApp.h"
#include "psNefReportToNwdaf.h"
#include "psNcuDataEncode.h"
#include "ps_job_define.h"
#include "ps_nef_interface.h"
#include "psMcsDebug.h"
#include "ncuSCCUAbility.h"
#include "vnfp_PubFun.h"
#include "NFcodec/base/JsonBase.h"
#include "http2Proxy/HTTPInterface.h"
#include "Http2Proxy.App2HttpReq.pb.h"
#include "sccu_pub_define.h"
#include "zte_slibc.h"
#include "ps_mcs_define.h"
#include "psUpfEvent.h"
#include "psUpfJobTypes.h"
#include "SigTrace.h"
#include "McsIPv4Head.h"
#include "McsIPv6Head.h"
#include "HTTP_LB/nghttp2codec/HTTPCodec.h"
#include "Http2Proxy.Http2AppAck.pb.h"
#include "Http2Proxy.Http2AppRes.pb.h"
#include "psNcuGetCfg.h"
#include "psMcsDebug.h"
#include "psNcuHttpLinkProc.h"
#include "ps_ncu_typedef.h"

#define DAF_SCTYPENAME_LEN          32
#define DAF_SERVICETYPENAME_LEN     32
#define APP2HTTP_SESID_EVTEXPOSURE_NOTIFI   "event-exposure-notify"
#define APP2HTTP_SESID_STATISTIC_NOTIFI   "event-statistic-notify"

void psDafFillEventNotifyMsgBody(T_NwdafInfoToUPM *ptNwdafInfo, Nupf_EventExposureNotification_NWDAF *ptDafNotifi, T_psNcuDaAppCtx * ptDaAppCtx);
VOID psDafApp2HTTPReqMsgFillProc(tDafReqHTTP2APP *ptReq,T_NwdafInfoToUPM *ptNwdafInfo, Nupf_EventExposureNotification_NWDAF *ptDafNotifi, T_psNcuSessionCtx* ptSessionCtx);
VOID daf_CodecErrorPrint(JsonContext &context);
WORD32 Daf_EventExposure_Report_EnCode(BYTE *msg, WORD32 *msgLen, Nupf_EventExposureNotification_NWDAF *ptdataDe);
void psNcuDafNotifyRattypeMapProc(UpfEventNotification_nwdaf* pEvtNotify, T_psNcuSessionCtx* ptSessionCtx);

const std::unordered_map<BYTE, RatType> g_RatTypeReversedMap = {
    {_3GPP_RAT_TYPE_NR, NR},
    {_3GPP_RAT_TYPE_WB_E_UTRAN, EUTRA},
    {_3GPP_RAT_TYPE_WLAN, WLAN},
    {_3GPP_RAT_TYPE_Virtual, VIRTUAL},
    {_3GPP_RAT_TYPE_EUTRAN_NB_IoT, NBIOT},
    {_3GPP_RAT_TYPE_LTE_M, LTEM},
    {_3GPP_RAT_TYPE_UTRAN, UTRA},
    {_3GPP_RAT_TYPE_GERAN, GERA},
    {_3GPP_RAT_TYPE_NR_LEO, NR_LEO},
    {_3GPP_RAT_TYPE_NR_MEO, NR_MEO},
    {_3GPP_RAT_TYPE_NR_GEO, NR_GEO},
    {_3GPP_RAT_TYPE_NR_OTHERSAT, NR_OTHER_SAT},
    {_3GPP_RAT_TYPE_NR_REDCAP, NR_REDCAP},
    {_3GPP_RAT_TYPE_WB_E_UTRAN_LEO, WB_E_UTRAN_LEO},
    {_3GPP_RAT_TYPE_WB_E_UTRAN_MEO, WB_E_UTRAN_MEO},
    {_3GPP_RAT_TYPE_WB_E_UTRAN_GEO, WB_E_UTRAN_GEO},
    {_3GPP_RAT_TYPE_WB_E_UTRAN_OTHERSAT, WB_E_UTRAN_OTHERSAT},
    {_3GPP_RAT_TYPE_EUTRAN_NB_IoT_LEO, NB_IOT_LEO},
    {_3GPP_RAT_TYPE_EUTRAN_NB_IoT_MEO, NB_IOT_MEO},
    {_3GPP_RAT_TYPE_EUTRAN_NB_IoT_GEO, NB_IOT_GEO},
    {_3GPP_RAT_TYPE_EUTRAN_NB_IoT_OTHERSAT, NB_IOT_OTHERSAT}
};

#define TRACEMSG_BUFFERLEN 4096

static thread_local std::unique_ptr<BYTE[]> TraceMsgBuffer(new BYTE[TRACEMSG_BUFFERLEN]);

void encodeHttpMsg(T_NrfTraceMsg *ptNrfTraceMsg)
{
    if(NULL == ptNrfTraceMsg)
    {
        return;
    }

    /*lint -e737 -e64*/
    T_HTTP_TXTENCODE_IN  tHttpEncodeIn = {0};
    T_HTTP_TXTENCODE_OUT tEncodeOut = {0};
    tEncodeOut.pbOutBuffer = TraceMsgBuffer.get();
    tEncodeOut.uiBufferLen = TRACEMSG_BUFFERLEN;

    tHttpEncodeIn.uiMsgId  = ptNrfTraceMsg->msgId;
    tHttpEncodeIn.pbMsg    = ptNrfTraceMsg->msgData;
    tHttpEncodeIn.uiMsgLen = ptNrfTraceMsg->msgLen;

    DEBUG_TRACE(DEBUG_LOW,
               "\n [encodeHttpMsg] before http2_fullstackencode"
               "\n ptNrfTraceMsg->srcIp      = %s "
               "\n ptNrfTraceMsg->destIp     = %s "
               "\n ptNrfTraceMsg->httpMsgId  = %d "
               "\n ptNrfTraceMsg->msgLen     = %u "
               "\n ptNrfTraceMsg->offset     = %u "
               "\n ptNrfTraceMsg->msgData    = %p "
               "\n ptNrfTraceMsg->msgId    = %u ",
               ptNrfTraceMsg->srcIp,
               ptNrfTraceMsg->destIp,
               ptNrfTraceMsg->httpMsgId,
               ptNrfTraceMsg->msgLen,
               ptNrfTraceMsg->offset,
               ptNrfTraceMsg->msgData,
               ptNrfTraceMsg->msgId);

    ptNrfTraceMsg->msgLen = 0;

    WORD32 ret = http2_txtencode(&tHttpEncodeIn, &tEncodeOut);
    if(HTTP_TRACE_SUCCESS != ret)
    {
        DEBUG_TRACE(DEBUG_LOW,"upmTraceHttpMsg() http2_fullstackencode return fail, ret=%u\n", ret);
        return;
    }

    /* 编码完包含伪造的mac头，上报时不需要再另加macͷ */
    if(tEncodeOut.summary.type != HTTP_CODEC_MSG_REQUEST)
    {
        ptNrfTraceMsg->cause = tEncodeOut.summary.rspcode;
    }
    else
    {
         ptNrfTraceMsg->cause = 0xFFFF;
    }

    zte_strncpy_s(ptNrfTraceMsg->srcIp, TRACE_IP_LEN, (char *)tEncodeOut.summary.srcIpPort, (HTTP_CODEC_IP_LEN - 1));
    zte_strncpy_s(ptNrfTraceMsg->destIp, TRACE_URL_LEN, (char *)tEncodeOut.summary.dstIpPort, (HTTP_CODEC_IP_LEN - 1));
    
    ptNrfTraceMsg->srcIp[TRACE_IP_LEN - 1]   = '\0';
    ptNrfTraceMsg->destIp[TRACE_URL_LEN - 1] = '\0';

    ptNrfTraceMsg->msgData = tEncodeOut.pbOutBuffer;
    ptNrfTraceMsg->msgLen  = (WORD16)tEncodeOut.uiMsgLen;
    ptNrfTraceMsg->offset  = (WORD16)tEncodeOut.uiDataOffset;
    DEBUG_TRACE(DEBUG_LOW,
                       "\n [encodeHttpMsg] after http2_txtencode"
                       "\n ptNrfTraceMsg->srcIp   = %s "
                       "\n ptNrfTraceMsg->destIp  = %s "
                       "\n ptNrfTraceMsg->msgLen  = %u "
                       "\n ptNrfTraceMsg->offset  = %u "
                       "\n ptNrfTraceMsg->msgData = %p "
                       "\n ptNrfTraceMsg->cause   = %d ",
                       ptNrfTraceMsg->srcIp,
                       ptNrfTraceMsg->destIp,
                       ptNrfTraceMsg->msgLen,
                       ptNrfTraceMsg->offset,
                       ptNrfTraceMsg->msgData,
                       ptNrfTraceMsg->cause);

    return;
}


__thread BYTE g_NrfTraceMsgData[65535] = {0};

void NcuSbiSigTrace(const BYTE *msgdata, const WORD16 msglen, T_NrfTraceMsg *ptNrfTraceMsg)
{
    if(NULL == msgdata || NULL == ptNrfTraceMsg)
    {
        return;
    }
    memset(g_NrfTraceMsgData, 0, 65535);
    memcpy(g_NrfTraceMsgData, msgdata, msglen);
    ptNrfTraceMsg->msgData   = g_NrfTraceMsgData;
    ptNrfTraceMsg->msgLen    = msglen;
    ptNrfTraceMsg->httpMsgId = 100;//这个是事件，可以自己定义一个字符串
    /* 上报前先进行HTTP格式编码 */
    encodeHttpMsg(ptNrfTraceMsg);

    return;
}

WORD32 handleDataAnalyticsNcuToUpmReq(BYTE * pMsgBody, T_psNcuDaAppCtx * ptDaAppCtx)
{
    JOB_CHECK_NULL_RET(pMsgBody, JOB_RET_FAIL);
    JOB_CHECK_NULL_RET(ptDaAppCtx, JOB_RET_FAIL);
    JOB_CHECK_NULL_RET(g_ptMediaProcThreadPara, JOB_RET_FAIL);
    WORD32 dwRet = MCS_RET_SUCCESS;
    T_psNcuSessionCtx* ptSessionCtx = (T_psNcuSessionCtx*)ptDaAppCtx->ptSessionCtx;
    DEBUG_TRACE(DEBUG_LOW,"Enter handleDataAnalyticsNcuToUpmReq function.\n");
    
    T_NwdafInfoToUPM *ptNwdafInfo =  (T_NwdafInfoToUPM *)pMsgBody;
    T_psNcuHttpLink *ptNcuHttpLinkCtx = psNcuGetHttpLinkByReportUri(ptNwdafInfo->aucReportUri, g_ptMediaProcThreadPara->dwhDBByThreadNo);
    if(psNcuIsHttpLinkFault(ptNcuHttpLinkCtx, ptSessionCtx))
    {
        if (psNcuIsNeedReportLinkFault(ptSessionCtx))
        {
            dwRet = psNcuReportLinkFaultToPfu(ptSessionCtx);
        }
        if (HTTP_REMOTE_INVALID == ptNcuHttpLinkCtx->remoteStatus) //当链路状态为NO_STATUS时，上报故障后，要继续尝试发送质差上报Notify Req
        {
            return dwRet;
        }
    }

    static __thread Nupf_EventExposureNotification_NWDAF tDafNotifi = {0};
    tDafReqHTTP2APP tDafReqHttp2APP = {0};
    psDafFillEventNotifyMsgBody(ptNwdafInfo, &tDafNotifi, ptDaAppCtx);
    psDafApp2HTTPReqMsgFillProc(&tDafReqHttp2APP,ptNwdafInfo, &tDafNotifi, ptSessionCtx);
    
    dwRet |= psDafApp2HTTPMsgReq(&tDafReqHttp2APP, ptSessionCtx, ptNcuHttpLinkCtx);
    return dwRet;
}

void psDafFillEventNotifyMsgBody(T_NwdafInfoToUPM *ptNwdafInfo, Nupf_EventExposureNotification_NWDAF *ptDafNotifi,T_psNcuDaAppCtx * ptDaAppCtx)
{
    if(NULL == ptNwdafInfo || NULL == ptDafNotifi || NULL == ptDaAppCtx)
    {
        return;
    }
    zte_memset_s(ptDafNotifi, sizeof(Nupf_EventExposureNotification_NWDAF), 0, sizeof(Nupf_EventExposureNotification_NWDAF));
    zte_memcpy_s(ptDafNotifi->correlationId, NUPF_NOTIFID_LEN_M-1, ptNwdafInfo->aucCorid, MIN((NUPF_NOTIFID_LEN_M-1),(AUCCORID_MAXLEN)));
    ptDafNotifi->num = 1;
    UpfEventNotification_nwdaf* pEvtNotify = &(ptDafNotifi->notificationItems[0]);
    pEvtNotify->eventType = QOS_ANA; 
    pEvtNotify->timeStamp_Flag = 1;
    XOS_GetCurrentTime(&pEvtNotify->timeStamp);
    
    pEvtNotify->qosAnaInfo_Flag = 1;
    zte_memcpy_s(&pEvtNotify->qosAnaInfo,sizeof(QosAnalysisInfo),&ptNwdafInfo->tQosAnalysisInfo,sizeof(QosAnalysisInfo));
    pEvtNotify->startTime_Flag = 1;
    pEvtNotify->startTime.dwSecond  = ptDaAppCtx->tCrtStdTime.dwSecond;
    pEvtNotify->startTime.wMilliSec = ptDaAppCtx->tCrtStdTime.wMilliSec;
    
    T_psNcuSessionCtx* ptSessionCtx = (T_psNcuSessionCtx*)ptDaAppCtx->ptSessionCtx;
    if(NULL == ptSessionCtx)
    {
        DEBUG_TRACE(DEBUG_LOW,"No find Session\n");
        return;
    }
    char ipstr[64] = {0};
    if(psCheckIPValid(ptSessionCtx->tMSIPv4, IPv4_VERSION) && inet_ntop(AF_INET, ptSessionCtx->tMSIPv4, ipstr, 64))
    {
        DEBUG_TRACE(DEBUG_LOW, "has_UeIpv4Addr_flg \n");
        pEvtNotify->ueIpv4Addr_Flag = 1;
        zte_memcpy_s(pEvtNotify->ueIpv4Addr, OAPI_IPV4_LEN, ipstr, zte_strnlen_s(ipstr,OAPI_IPV4_LEN));
        zte_memset_s(ipstr,64, 0 , 64);
    }
    T_IPV6 ipv6  = {0};
    IPV6_PREFIX_COPY(ipv6, ptSessionCtx->tMSIPv6);
    if(psCheckIPValid(ptSessionCtx->tMSIPv6, IPv6_VERSION) && inet_ntop(AF_INET6, ipv6, ipstr, 64))
    {
        DEBUG_TRACE(DEBUG_LOW, "has_UeIpv6Prefix_flg \n");
        pEvtNotify->ueIpv6Prefix_Flag = 1;
        zte_strncat_s(ipstr,64,"/64", 3);
        zte_memcpy_s(pEvtNotify->ueIpv6Prefix, OAPI_IPV6_PREFIX_LEN, ipstr, zte_strnlen_s(ipstr,OAPI_IPV6_PREFIX_LEN));
    }
    if(ptSessionCtx->Apn[0])
    {
        pEvtNotify->dnn_Flag = 1;
        DEBUG_TRACE(DEBUG_LOW, "has_dnn_flg \n");
        zte_memcpy_s(&(pEvtNotify->dnn),DNN_LEN, ptSessionCtx->Apn, 64);
    }
    psNcuDafNotifyRattypeMapProc(pEvtNotify, ptSessionCtx);

    pEvtNotify->ueMacAddr_Flag = 0;
    if(ptSessionCtx->bImsi[0])
    {
        DEBUG_TRACE(DEBUG_LOW, "has_Supi_flg \n");
        pEvtNotify->supi_Flag = 1;
        zte_memcpy_s(pEvtNotify->supi, OAPI_IMSI_LEN, ptSessionCtx->bImsi,IMSI_LEN);
        
    }
    if(ptSessionCtx->bIsdn[0])
    {
        DEBUG_TRACE(DEBUG_LOW, "has_Gpsi_flg \n");
        pEvtNotify->gpsi_Flag = 1;
        pEvtNotify->gpsi.len = 8;
        pEvtNotify->gpsi.type = GPSI_TYPE_MSISDN;
        zte_memcpy_s(pEvtNotify->gpsi.value, 8, ptSessionCtx->bIsdn,8);
    }
    
    DEBUG_TRACE(DEBUG_LOW, "has_Snssai_flg \n");
    pEvtNotify->snssai_Flag = 1;
    pEvtNotify->snssai.sst = ptSessionCtx->tSNssai.ucSST;
    pEvtNotify->snssai.btSdFg = 1;
    zte_memcpy_s(pEvtNotify->snssai.sd,SBI_SD_LEN, ptSessionCtx->tSNssai.aucSD,3);
    return;
}


VOID psDafApp2HTTPReqMsgFillProc(tDafReqHTTP2APP *ptReq,T_NwdafInfoToUPM *ptNwdafInfo, Nupf_EventExposureNotification_NWDAF *ptDafNotifi, T_psNcuSessionCtx* ptSessionCtx)
{
    if(NULL == ptReq || NULL == ptNwdafInfo || NULL == ptDafNotifi)
    {
        return;
    }
    T_psNcuMcsPerform *ptNcuPerform = psGetPerform();
    WORD32 ret = 0;
    JID tSelf = {0};
    zte_memset_s(ptReq, sizeof(tDafReqHTTP2APP), 0, sizeof(tDafReqHTTP2APP));

    ptReq->version = TRUE;
    zte_strncpy_s((char *)ptReq->method, DAF_HTTP_METHOD_LEN, "POST",  DAF_HTTP_METHOD_LEN - 1); 
    zte_strncpy_s((char *)ptReq->url, DAF_HTTP_URL_LEN, (const char*)ptNwdafInfo->aucReportUri, AUCREPORTURI_MAXLEN);
    zte_strncpy_s((char *)ptReq->contentType, DAF_HTTP_CONTENTTYPE_LEN, "application/json", DAF_HTTP_CONTENTTYPE_LEN -1);
    ptReq->traced = 1; //信令跟踪使用，置1后，发送给HTTPLB后，HTTPLB会回消息
    ptReq->profileID = psGetClientProfileidByUri((char*)ptReq->url); // profileid可配置，全局配置。
    DEBUG_TRACE(DEBUG_LOW,"[DAF]url = %s profileid=%u\n", ptReq->url, ptReq->profileID);    
    if(0 == ptReq->profileID && NULL != ptNcuPerform)
    {
        MCS_LOC_STAT_EX(ptNcuPerform, qwHttpProfileIDZero, 1);
    }

    zte_snprintf_s((char *)ptReq->userID, DAF_HTTP_USERID_LEN, "%u", g_ptMediaProcThreadPara->bThreadNo);
    if(NULL !=ptSessionCtx)
    {
        zte_snprintf_s((char *)ptReq->sessionID, DAF_HTTP_SESSIONID_LEN, "%llu", ptSessionCtx->ddwUPSeid);
    }
    DEBUG_TRACE(DEBUG_LOW,"[DAF]userID = %s\n", ptReq->userID);    

    DEBUG_TRACE(DEBUG_LOW,"[DAF]sessionID = %s\n", ptReq->sessionID);    

    tDafHTTP2AppInfo * ptAppInfo = &(ptReq->appInfo);

    
    WORD32 UPMCommonId = ncuGetSelfCommId();
    ptAppInfo->serviceTypeID = pubGetServiceTypeByCommID(UPMCommonId);
    ptAppInfo->serviceInst   = pubGetServiceInstanceByCommID(UPMCommonId);
    ptAppInfo->scTypeID      = pubGetScTypeByCommID(UPMCommonId);
    ptAppInfo->userGroupId   = 0;

    ret = XOS_GetSelfJID(&tSelf);
    if(XOS_SUCCESS != ret)
    {
        DEBUG_TRACE(DEBUG_LOW,"[DAF]get job id fail!\n");
    }
    ptAppInfo->jobID.dwJno    = g_ptMediaProcThreadPara->bIntraThreadNo + 1;
    DEBUG_TRACE(DEBUG_LOW,"psDafApp2HTTPReqMsgFillProc UPMCommonId=%u,ptAppInfo->jobID.dwJno = %u\n",UPMCommonId,ptAppInfo->jobID.dwJno);
    ptAppInfo->jobID.dwComId  = UPMCommonId;
    DEBUG_TRACE(DEBUG_LOW,"serviceTypeID = %u,serviceInst = %u, scTypeID = %u\n",ptAppInfo->serviceTypeID,ptAppInfo->serviceInst,ptAppInfo->scTypeID);

    Daf_EventExposure_Report_EnCode(ptReq->MsgData, &ptReq->dwMsgLen, ptDafNotifi);
    return;
}


WORD32 Daf_EventExposure_Report_EnCode(BYTE *msg, WORD32 *msgLen, Nupf_EventExposureNotification_NWDAF *ptdataDe)
{
    JOB_CHECK_NULL_RET(msg, JOB_RET_FAIL);
    JOB_CHECK_NULL_RET(msgLen, JOB_RET_FAIL);
    JOB_CHECK_NULL_RET(ptdataDe, JOB_RET_FAIL);
    JsonContext                 context;
    JsonContextInit(&context);
    if(TRUE != jsonEncode_Nupf_EventExposureNotification_NWDAF(&context, ptdataDe, msg, DAF_HTTP_MSG_DATA_LEN))
    {
        DEBUG_TRACE(DEBUG_LOW,"encode fail !\n");
        daf_CodecErrorPrint(context);
    }
    *msgLen = context.encodeLength;
    DEBUG_TRACE(DEBUG_LOW,"MsgData:%s encode-Length:%u !\n", msg, *msgLen);
    return JOB_RET_SUCCESS;
}

VOID daf_CodecErrorPrint(JsonContext &context)
{
    DEBUG_TRACE(DEBUG_LOW,"daf_CodecErrorPrint in:errorCode=%u, errorCount=%u, hasTrancate=%u, errorPath=%s!\n",
        context.errorCode, context.errorCount, context.hasTruncate, context.errorPath);
    for(WORD32 i =0; i<context.errorCount && i<JSON_ERROR_STACK_SIZE; i++)
    {
        DEBUG_TRACE(DEBUG_LOW,"errorInfos[%u]:lineno=%u, message=%s, filename=%s!", i,
            context.errorInfos[i].lineno,
            context.errorInfos[i].message,
            context.errorInfos[i].filename);
    }
}

bool psDafApp2HTTPMsgReq(tDafReqHTTP2APP* pHTTP2APPReq, T_psNcuSessionCtx* ptSessionCtx, void *ptNcuHttpLinkCtxVoid)
{
    http2proxy::App2HttpReq appReq;
    std::string app2HttpReq("");
    char sctypeName[DAF_SCTYPENAME_LEN]= {0};
    char servicetypeName[DAF_SERVICETYPENAME_LEN] = {0};
    
    JOB_CHECK_NULL_RET(pHTTP2APPReq,false);
    
    appReq.set_accept((char*)pHTTP2APPReq->accept);
    appReq.set_method((char*)pHTTP2APPReq->method);
    appReq.set_url((char*)pHTTP2APPReq->url);
    appReq.set_sessionid((char*)pHTTP2APPReq->sessionID);
    appReq.set_contenttype((char*)(pHTTP2APPReq->contentType));
    appReq.set_userid((char*)(pHTTP2APPReq->userID));
    appReq.set_profileid(pHTTP2APPReq->profileID);
    appReq.set_traced(pHTTP2APPReq->traced);
    appReq.set_reqtype(http2proxy::NORMAL_REQ);
    appReq.set_version(http2proxy::HTTP_2);
    T_psNcuHttpLink* ptNcuHttpLinkCtx = (T_psNcuHttpLink*)ptNcuHttpLinkCtxVoid;
    if (NULL != ptNcuHttpLinkCtx)
    {
        appReq.set_sequence((char*)(ptNcuHttpLinkCtx->bSequence));
    }

    auto pHttp2AppInfo = appReq.mutable_appinfo();
    JOB_CHECK_NULL_RET(pHttp2AppInfo,false);
    XOS_snprintf(sctypeName,DAF_SCTYPENAME_LEN-1,"%u",pHTTP2APPReq->appInfo.scTypeID);
    XOS_snprintf(servicetypeName,DAF_SERVICETYPENAME_LEN-1,"%u",pHTTP2APPReq->appInfo.serviceTypeID);
    pHttp2AppInfo->set_sctypename(sctypeName);
    pHttp2AppInfo->set_serviceability((char*)(pHTTP2APPReq->appInfo.serviceAbility));
    pHttp2AppInfo->set_serviceinst(pHTTP2APPReq->appInfo.serviceInst);
    pHttp2AppInfo->set_servicetypename(servicetypeName);
    pHttp2AppInfo->set_usergroupid(pHTTP2APPReq->appInfo.userGroupId);
    
    auto pHttpJid= pHttp2AppInfo->mutable_jid();
    JOB_CHECK_NULL_RET(pHttpJid,false);
    pHttpJid->set_comid(pHTTP2APPReq->appInfo.jobID.dwComId);
    pHttpJid->set_jno(pHTTP2APPReq->appInfo.jobID.dwJno);

    if (pHTTP2APPReq->dwMsgLen > 0)
    {
        DEBUG_TRACE(DEBUG_LOW,"psDafApp2HTTPMsgReq app Pb data[%s]\n",pHTTP2APPReq->MsgData);
        appReq.set_data((char*)pHTTP2APPReq->MsgData);
    }

    /* 消息构造完后进行序列化 */
    if(!appReq.SerializeToString(&app2HttpReq))
    {
        DEBUG_TRACE(DEBUG_LOW,"psDafApp2HTTPMsgReq app Pb Req SerializeToString fail\n");
        return false;
    }
    if(NULL != ptSessionCtx)
    {
        DEBUG_TRACE(DEBUG_LOW,"Enter SigTrace_NCU_SBI function.\n");
        SigTrace_NCU_SBI(ptSessionCtx, (BYTE*)app2HttpReq.c_str(), app2HttpReq.size());
    }
    #ifdef FT_TEST
        extern BYTE ftRptData[2048];
        zte_memset_s(ftRptData, 2048, '\0', 2048);
        zte_memcpy_s(ftRptData, 2048, pHTTP2APPReq->MsgData, pHTTP2APPReq->dwMsgLen);
        return 1;
    #endif
    return psNcuSendNotifyReqToNwdaf((BYTE*)app2HttpReq.c_str(), app2HttpReq.size(), EV_SERVICE_2_HTTP_PB_REQUEST, ptNcuHttpLinkCtx);
}

BOOL ncuDecodeDafReqHTTP2APPMsg(void *msg, WORD32 msgLen, char* sessionID)
{
    T_psNcuMcsPerform *ptMcsNcuPerform = psGetPerform();
    if (NULL == msg || NULL == ptMcsNcuPerform || NULL == sessionID)
    {
        return FALSE;
    }

    http2proxy::Http2AppAck AppAck;
    DEBUG_TRACE(DEBUG_LOW,"upmDecodeDafReqHTTP2APPMsg exter.\n");

    if (!AppAck.ParseFromArray(msg,(int)msgLen)) 
    {
        MCS_LOC_STAT_EX(ptMcsNcuPerform, QosAnaNotifyReqAckParseFail, 1);
        DEBUG_TRACE(DEBUG_LOW,"AppAck.ParseFromArray fail.\n");
        return FALSE;
    }
    if(AppAck.has_req())
    {
        DEBUG_TRACE(DEBUG_LOW,"AppAck.has_req().\n");
        XOS_strncpy(sessionID, AppAck.req().sessionid().c_str(), DAF_HTTP_SESSIONID_LEN);
    }
    DEBUG_TRACE(DEBUG_LOW,"upmDecodeDafReqHTTP2APPMsg exit.\n");


    return TRUE;

}

BOOL ncuDecodeDafRspHTTP2APPMsg(void *msg, WORD32 msgLen, char* sessionID)
{
    T_psNcuMcsPerform *ptMcsNcuPerform = psGetPerform();
    if (NULL == msg || NULL == ptMcsNcuPerform || NULL == sessionID)
    {
        return FALSE;
    }

    http2proxy::Http2AppRes pbRsp;
    if (!pbRsp.ParseFromArray(msg, (int)msgLen))
    {
        MCS_LOC_STAT_EX(ptMcsNcuPerform, QosAnaNotifyRspParseFail, 1);
        return FALSE;
    }
    WORD32 dwRspCode = (WORD32)pbRsp.responsecode();
    if (HTTP_204 == dwRspCode)
    {
        MCS_LOC_STAT_EX(ptMcsNcuPerform, rcvQosAnaNotifySuccRsp, 1);
    }
    else
    {
        MCS_LOC_STAT_EX(ptMcsNcuPerform, rcvQosAnaNotifyErrRsp, 1);
    }

    XOS_strncpy(sessionID, pbRsp.sessionid().c_str(), DAF_HTTP_SESSIONID_LEN);
    return TRUE;
}

VOID NcuHandleDataAnalyticsEncodeReqAck(WORD32 dwMsgID, BYTE * pMsgBody, WORD16 wMsgLen)
{
    DEBUG_TRACE(DEBUG_LOW,"Enter NcuHandleDataAnalyticsEncodeReqAck function.\n");
    if(NULL == pMsgBody)  /* 主用UPM才处理消息 */
    {
        return;
    }

    char sessionID[DAF_HTTP_SESSIONID_LEN] = {0};
    BOOL blRet = FALSE;
    BYTE bDirect = 0;    //0:send,1:recv
    if (EV_HTTP_2_SERVICE_PB_REQUEST_ACK == dwMsgID)
    {
        bDirect = 0;
        blRet = ncuDecodeDafReqHTTP2APPMsg(pMsgBody, wMsgLen, sessionID);
    }
    else if (EV_HTTP_2_SERVICE_PB_RESPONSE == dwMsgID)
    {
        bDirect = 1;
        blRet = ncuDecodeDafRspHTTP2APPMsg(pMsgBody, wMsgLen, sessionID);
    }
    if(FALSE == blRet)
    {
        DEBUG_TRACE(DEBUG_LOW,"Enter upmDecodeDafReqHTTP2APPMsg fail.\n");
        return;
    }
    DEBUG_TRACE(DEBUG_LOW, "[DAF]sessionID = %s\n", sessionID);
    WORD64 ddwUPSeid = 0;
    WORD32 dwhDB = g_ptMediaProcThreadPara->dwhDBByThreadNo;
    if(1 != zte_sscanf_s(sessionID, "%llu", NULL, &ddwUPSeid))
    {
        DEBUG_TRACE(DEBUG_LOW, "sessionID fail.\n");
        return;
    }
    DEBUG_TRACE(DEBUG_LOW,"NcuHandleDataAnalyticsEncodeReqAck ddwUPSeid= %llu, dwhDB= %u\n", ddwUPSeid, dwhDB);
    T_psNcuSessionCtx* ptSession = psQuerySessionByUpseid(ddwUPSeid, dwhDB);
    if(NULL == ptSession)  /* 主用UPM才处理消息 */
    {
        DEBUG_TRACE(DEBUG_LOW,"upmDecodeDafReqHTTP2APPMsg ptSession NULL.\n");
        return;
    }
    SigTrace_NCU_SBI_Job(ptSession, pMsgBody, wMsgLen, dwMsgID, bDirect);
    return ;
}

BOOL psNcuGetMsgFromMbuffer(PS_PACKET *ptPacket, BYTE *aucDestBuf, WORD16 wMaxSize)
{
    MCS_CHK_NULL_RET(ptPacket, FALSE);
    MCS_CHK_NULL_RET(aucDestBuf, FALSE);

    if(unlikely(ptPacket->dwPktLen > wMaxSize))
    {
        return FALSE;
    }
    WORD32 dwPktLen    = 0;
    WORD32 dwBufferLen = 0;
    WORD16 wOffset     = 0;
    BYTE *ptBuffer     = NULL;
    struct rte_mbuf *pCurBufferNode = ptPacket->pBufferNode;
    struct rte_mbuf *pNextBufferNode = NULL;
    while((NULL != pCurBufferNode) && (dwPktLen < ptPacket->dwPktLen))
    {
        PS_GET_BUF_INFO(pCurBufferNode, ptBuffer, wOffset, dwBufferLen);
        if(unlikely((dwPktLen + dwBufferLen) > wMaxSize))
        {
            break;
        }
        CSS_MEMCPY((aucDestBuf+dwPktLen), (ptBuffer+wOffset), dwBufferLen);
        PS_GET_NEXT_BUF_NODE(pCurBufferNode, pNextBufferNode);
        pCurBufferNode = pNextBufferNode;
        dwPktLen += dwBufferLen;
    }
    if(ptPacket->dwPktLen != dwPktLen)
    {
        return FALSE;
    }

    return TRUE;
}

/* Started by AICoder, pid:fcc5a39993cedd314d1d094c50afd51b5fa904b9 */
VOID psNcuTipcSBIPktProc(T_MediaProcThreadPara *ptMediaProcThreadPara)
{
    MCS_CHK_NULL_VOID(ptMediaProcThreadPara);
    PS_PACKET *ptPacket = ptMediaProcThreadPara->ptPacket;
    MCS_CHK_NULL_VOID(ptPacket);
    T_psNcuMcsPerform *ptMcsNcuPerform = (T_psNcuMcsPerform*)ptMediaProcThreadPara->ptMcsStatPointer;
    MCS_CHK_NULL_VOID(ptMcsNcuPerform);

    if (EV_HTTP_2_SERVICE_PB_REQUEST_ACK == ptPacket->dwMsgID)
    {
        MCS_LOC_STAT_EX(ptMcsNcuPerform, rcvHttp2ServiceRequestAck, 1);
    }
    else if (EV_HTTP_2_SERVICE_PB_RESPONSE == ptPacket->dwMsgID)
    {
        MCS_LOC_STAT_EX(ptMcsNcuPerform, rcvHttp2ServiceResponse, 1);
    }
    else
    {
        MCS_LOC_STAT_EX(ptMcsNcuPerform, rcvHttp2ServiceOther, 1);
        return;
    }

    MCS_LOC_STAT_EX(ptMcsNcuPerform, getHttpMsgFromMbuffer, 1);
    BYTE threadID = ptPacket->bSthr % MEDIA_THRD_NUM;
    BYTE* msgBuffer = &g_ptVpfuShMemVar->aucInnerMsg[threadID][0];
    if (FALSE == psNcuGetMsgFromMbuffer(ptPacket, msgBuffer, PS_MAX_PFCPPKT_LEN))
    {
        MCS_LOC_STAT_EX(ptMcsNcuPerform, getHttpMsgFromMbufferErr, 1);
        return;
    }

    NcuHandleDataAnalyticsEncodeReqAck(ptPacket->dwMsgID, msgBuffer, ptPacket->dwPktLen);
    return;
}
/* Ended by AICoder, pid:fcc5a39993cedd314d1d094c50afd51b5fa904b9 */
void psNcuDafNotifyRattypeMapProc(UpfEventNotification_nwdaf* pEvtNotify, T_psNcuSessionCtx* ptSessionCtx)
{
    MCS_PCLINT_NULLPTR_RET_2ARG_VOID(pEvtNotify, ptSessionCtx);
    BYTE bRatType = ptSessionCtx->bRatType;
    MCS_CHK_VOID(bRatType >= 255);
    auto it = g_RatTypeReversedMap.find(bRatType);
    /* Started by AICoder, pid:f082b586c4n09ba14e3008b8b032660ac163f8bf */
    if (it != g_RatTypeReversedMap.end()) {
        pEvtNotify->ratType = it->second;
    } 
    /* Ended by AICoder, pid:f082b586c4n09ba14e3008b8b032660ac163f8bf */
    if(pEvtNotify->ratType != 0)
    {
        pEvtNotify->ratType_Flag = 1;
    }
    DEBUG_TRACE(DEBUG_LOW, "ptSessionCtx->bRatType= %u, pEvtNotify->ratType= %u\n", ptSessionCtx->bRatType, pEvtNotify->ratType);
    return;
}
