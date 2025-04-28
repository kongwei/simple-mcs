#include "psNcuDataEncode.h"
#include "MemShareCfg.h"
#include "dpa_ps_packet.h"
#include "ps_css_interface.h"
#include "ps_mcs_define.h"
#include "psNcuSubscribeCtxProc.h"
#include "ps_ncu_session.h"
#include "psNcuGetCfg.h"
#include "psNcuReportStructData.h"
#include "psNcuReportStructHead.h"
#include "psNcuRouteProc.h"
#include "zte_slibc.h"
#include "psMcsDebug.h"
#include "ps_ncu_stream.h"
#include "ps_ncu_dataApp.h"
#include "psNcuUliInfoFunc.h"
#include "McsByteOrder.h"
#include "McsIPv4Head.h"
#include "McsIPv6Head.h"
#include "psNcuSubBindAppCtxProc.h"

ExpNormalCorrData g_CorrData[16] = {0};

WORD16 g_report_seq_num = 1;
void psNcuEncodeData(T_Notification* ptNotification, T_CorrelationId* ptCorrelationId, BYTE* buffer, WORD16* ptDataLen);
WORD32 psNcuFillData(T_Notification* ptNotification, T_CorrelationId* ptCorrelationId, T_psNcuDaAppCtx* data, BYTE bRptType, BYTE trigger);

WORD16 psEncodeUDPDataProc(BYTE* buffer, T_psNcuDaAppCtx * ptDaAppCtx, BYTE bRptType, BYTE trigger)
{
    T_psNcuMcsPerform *ptNcuPerform = psGetPerform();
    if(NULL == ptDaAppCtx || NULL == ptNcuPerform || NULL == buffer)
    {
        return 0 ;
    }
    
    WORD16 wDataLen = 0;
    T_ExpDataHead* ptExpDataHead = (void*)buffer;
    T_Notification tNotificationData = {0};
    T_CorrelationId tCorrelationID = {0};
    if(MCS_RET_SUCCESS != psNcuFillData(&tNotificationData, &tCorrelationID, ptDaAppCtx, bRptType, trigger))
    {
        DEBUG_TRACE(DEBUG_LOW, "psNcuFillData Error \n");
        MCS_LOC_STAT_EX(ptNcuPerform, qwFillDataErr, 1);
        return 0;
    }
    psNcuEncodeData(&tNotificationData, &tCorrelationID, buffer +sizeof(T_ExpDataHead), &wDataLen);
    PS_FILL_EXP_HEAD_TO_NWDAF(ptExpDataHead, EXP_REPORT_TYPE, mcs_ntohs(wDataLen));
    BUFF_TRACE(DEBUG_SND, buffer, wDataLen+sizeof(T_ExpDataHead));
    return wDataLen+sizeof(T_ExpDataHead);
}

WORD32 psNcuFillData(T_Notification* ptNotification, T_CorrelationId* ptCorrelationId, T_psNcuDaAppCtx* data, BYTE bRptType,BYTE trigger)
{
    T_psNcuMcsPerform *ptNcuPerform = psGetPerform();
    if(NULL == ptNotification ||NULL == ptCorrelationId|| NULL == data || NULL == ptNcuPerform)
    {
        return MCS_RET_FAIL;
    }
    WORD16 ret = 0;
    T_psNcuDaSubScribeCtx* ptSubScribeCtx = data->ptSubScribeCtx;
    T_psNcuSessionCtx* ptSessionCtx = data->ptSessionCtx;

    if(NULL == ptSubScribeCtx || NULL == ptSessionCtx)
    {
        MCS_LOC_STAT_EX(ptNcuPerform, qwFillDataNoScribeOrNoSess, 1);
        return MCS_RET_FAIL;
    }
    
    ptNotification->has_TimeStamp_flg = 1;
    DEBUG_TRACE(DEBUG_LOW, "has_TimeStamp_flg \n");
    T_Time tCurrentTime = {0};
    XOS_GetCurrentTime(&tCurrentTime);
    ptNotification->TimeStamp.dwTimeStamp = mcs_htonl(tCurrentTime.dwSecond + 0xBC17C200);
    ptNotification->TimeStamp.milliisend = mcs_htons(tCurrentTime.wMilliSec);
    ptNotification->has_StartTime_flg = 1;
    DEBUG_TRACE(DEBUG_LOW, "has_StartTime_flg \n");
    ptNotification->StartTime.dwTime =mcs_htonl(data->tCrtStdTime.dwSecond + 0xBC17C200);
    ptNotification->StartTime.milliisend = mcs_htons(data->tCrtStdTime.wMilliSec);
    if(psCheckIPValid(ptSessionCtx->tMSIPv4, IPv4_VERSION))
    {
        DEBUG_TRACE(DEBUG_LOW, "has_UeIpv4Addr_flg \n");
        ptNotification->has_UeIpv4Addr_flg = 1;
        IPV4_COPY(ptNotification->UeIpv4Addr.Ipv4,ptSessionCtx->tMSIPv4);
    }
    if(psCheckIPValid(ptSessionCtx->tMSIPv6, IPv6_VERSION))
    {
        DEBUG_TRACE(DEBUG_LOW, "has_UeIpv6Prefix_flg \n");
        ptNotification->has_UeIpv6Prefix_flg = 1;
        zte_memcpy_s(ptNotification->UeIpv6Prefix.Ipv6Pre, 16, ptSessionCtx->tMSIPv6, 16);
        ptNotification->UeIpv6Prefix.prelen = 64;

    }
    ptNotification->has_UeMacAddr_flg = 0;
    ret = zte_strnlen_s((const char*)ptSessionCtx->Apn, 64);
    if(ret != 0)
    {
        DEBUG_TRACE(DEBUG_LOW, "has_Dnn_flg ,DNN:%s\n", ptSessionCtx->Apn);
        ptNotification->has_Dnn_flg = 1;
        zte_memcpy_s(ptNotification->Dnn.dnn, 64, ptSessionCtx->Apn,ret);
        ptNotification->Dnn.IEBase.length = mcs_htons(ret);
    }
    if(ptSessionCtx->bRatType && ptSessionCtx->bRatType < 255)
    {
        DEBUG_TRACE(DEBUG_LOW, "has_RatType_flg,ratype=%u \n", ptSessionCtx->bRatType);
        ptNotification->has_RatType_flg = 1;
        ptNotification->RatType.bRatType = ptSessionCtx->bRatType;
    }
    else
    {
        MCS_LOC_STAT_EX(ptNcuPerform, qwFillDataRatypeErr, 1);
    }
    if(ptSessionCtx->bImsi[0])
    {
        DEBUG_TRACE(DEBUG_LOW, "has_Supi_flg \n");
        ptNotification->has_Supi_flg = 1;
        zte_memcpy_s(ptNotification->Supi.supi, 8, ptSessionCtx->bImsi,8);
        
    }
    if(ptSessionCtx->bIsdn[0])
    {
        DEBUG_TRACE(DEBUG_LOW, "has_Gpsi_flg \n");
        ptNotification->has_Gpsi_flg = 1;
        zte_memcpy_s(ptNotification->Gpsi.gpsi, 8, ptSessionCtx->bIsdn,8);
    }
    

    DEBUG_TRACE(DEBUG_LOW, "has_Snssai_flg \n");
    ptNotification->has_Snssai_flg = 1;
    ptNotification->Snssai.sst = ptSessionCtx->tSNssai.ucSST;
    zte_memcpy_s(ptNotification->Snssai.sd,3, ptSessionCtx->tSNssai.aucSD,3);

    if(ptSessionCtx->bHasUli)
    {
        DEBUG_TRACE(DEBUG_LOW, "has_UserLocationInfo_flg \n");
        ptNotification->has_UserLocationInfo_flg = 1;
        psNcuFillULiNormal(&ptNotification->UserLocationInfo, &ptSessionCtx->tUserLocation);
    }
    
    ptNotification->has_QosAnaInfo_flg = 1;
    DEBUG_TRACE(DEBUG_LOW, "has_QosAnaInfo_flg \n");
    T_IE_QosAnaInfo* ptQosAna = &(ptNotification->QosAnaInfo);
    ptQosAna->has_ApplicationID_flg = 1;
    zte_memcpy_s(ptQosAna->ApplicationID.Appid,64, ptSubScribeCtx->appidstr, 64);
    DEBUG_TRACE(DEBUG_LOW, "has_ApplicationID_flg,appid=%s \n", ptSubScribeCtx->appidstr);
    WORD16 appidlen = zte_strnlen_s(ptQosAna->ApplicationID.Appid,MAX_APPID_LEN);
    if(appidlen == 0)
    {
        MCS_LOC_STAT_EX(ptNcuPerform, qwFillDataAppidZero, 1);
    }
    ptQosAna->ApplicationID.IEBase.length = mcs_htons(appidlen);

    ptQosAna->has_SubAppID_flg = 1;
    zte_memcpy_s(ptQosAna->SubAppID.SubAppid,64, data->subAppidStr, 64);
    appidlen = zte_strnlen_s(ptQosAna->SubAppID.SubAppid,MAX_APPID_LEN);
    if(appidlen == 0)
    {
        MCS_LOC_STAT_EX(ptNcuPerform, qwFillDataSubAppidZero, 1);
    }
    DEBUG_TRACE(DEBUG_LOW, "has_SubAppID_flg,appid=%s \n", data->subAppidStr);
    ptQosAna->SubAppID.IEBase.length = mcs_htons(appidlen);
    ptQosAna->SubAppID.IEBase.Instance = 1;
    ptQosAna->has_ReportType_flg = 1;
    ptQosAna->ReportType.type = bRptType;
    DEBUG_TRACE(DEBUG_LOW, "has_ReportType_flg,bRptType=%u \n", bRptType);

    T_psNcuDataAnalysis* ptAnalysis =NULL;
    ptNotification->has_EventType_flg = 1;
    ptNotification->EventType.EventType = EVENT_QOS_EXP;  //服务化接口目前只有exp，如果后期有质差，则需要修改如下面if
    ptAnalysis = &data->Analysis;
    if(trigger == enumQosExpSpecial)
    {
        zte_memcpy_s(ptCorrelationId->correlationId, 256, ptSessionCtx->bCorrelationId_Exp,255);
        DEBUG_TRACE(DEBUG_LOW, "enumQosExpSpecial,correlationId=%s \n", ptSessionCtx->bCorrelationId_Exp);
    }
    else if(trigger == enumQosExpNormal)
    {
        //抽样用户需要从全局取correlationid
        if(MCS_RET_FAIL == setCorrelationIDByDnn(ptSessionCtx->Apn, ptCorrelationId->correlationId))
        {
            MCS_LOC_STAT_EX(ptNcuPerform, qwFillDataCorrelationByDnnErr, 1);
            DEBUG_TRACE(DEBUG_LOW, "enumQosExpNormal,correlationId get fail, dnn=%s \n", ptSessionCtx->Apn);
        }
        DEBUG_TRACE(DEBUG_LOW, "enumQosExpNormal,correlationId=%s \n", ptCorrelationId->correlationId);
    }
    


    ptCorrelationId->Base.length = mcs_htons((WORD16)(zte_strnlen_s(ptCorrelationId->correlationId, 255)));

    ptQosAna->has_QosMonReports_flg = 1;
    ptQosAna->QosMonReports.delay_an = mcs_htonl(ptAnalysis->dwUlAvgRtt);
    ptQosAna->QosMonReports.delay_dn = mcs_htonl(ptAnalysis->dwDlAvgRtt);
    ptQosAna->QosMonReports.bandwidth_ul = mcs_htonl(ptAnalysis->dwUlThroughput);
    ptQosAna->QosMonReports.bandwidth_dl = mcs_htonl(ptAnalysis->dwDlThroughput);
    ptQosAna->QosMonReports.lostpackets_ul = mcs_htonl(ptAnalysis->dwUlLossNum);
    ptQosAna->QosMonReports.lostpackets_dl = mcs_htonl(ptAnalysis->dwDlLossNum);
    ptQosAna->QosMonReports.totalpackets_ul = mcs_htonl(ptAnalysis->dwUlTcpPktNum);
    ptQosAna->QosMonReports.totalpackets_dl = mcs_htonl(ptAnalysis->dwDlTcpPktNum);


    return MCS_RET_SUCCESS;
}

#define NCU_ENCODE_LOC_BASE_FIELD(LenField) \
do{\
    wIElen = sizeof(LenField) + (LenField);    \
    zte_memcpy_s(bufferCur, wIElen, (BYTE*)&(LenField), wIElen);\
    bufferCur+= wIElen;\
    *ptDataLen+=wIElen;\
}while(0)

void psNcuEncodeData(T_Notification* ptNotification, T_CorrelationId* ptCorrelation, BYTE* buffer, WORD16* ptDataLen)
{
    if(NULL == ptNotification || NULL == ptCorrelation|| NULL == buffer || NULL == ptDataLen)
    {
        return;
    }
    BYTE * bufferCur = buffer;
    WORD16 wIElen = sizeof(T_IE_Base);
    ptNotification->Base.type = mcs_htons(IETYPE_NotificationBase);
    T_IE_Base* ptNotificationIEHead = (T_IE_Base*)bufferCur;

    zte_memcpy_s(bufferCur,wIElen, &ptNotification->Base,wIElen);
    bufferCur+= wIElen;
    *ptDataLen+=wIElen;
    NCU_ENCODE_IE(ptNotification, EventType);
    NCU_ENCODE_IE(ptNotification, UeIpv4Addr);
    NCU_ENCODE_IE(ptNotification, UeIpv6Prefix);
    NCU_ENCODE_IE(ptNotification, UeMacAddr);
    NCU_ENCODE_IE(ptNotification, TimeStamp);
    NCU_ENCODE_IE(ptNotification, StartTime);
    NCU_ENCODE_IE(ptNotification, Dnn);
    NCU_ENCODE_IE(ptNotification, Snssai);
    NCU_ENCODE_IE(ptNotification, Supi);
    NCU_ENCODE_IE(ptNotification, Gpsi);
    NCU_ENCODE_IE(ptNotification, RatType);
    if(ptNotification->has_UserLocationInfo_flg)
    {
        WORD16 curTotalLen = *ptDataLen;
        T_IE_UserLocationInfo* ptUserLocationInfo = &(ptNotification->UserLocationInfo);
        ptUserLocationInfo->Base.type = mcs_htons(IETYPE_UserLocationInfo);
        ptUserLocationInfo->Base.Instance = 0;
        ptUserLocationInfo->Base.Spare = 0;
        wIElen = sizeof(T_IE_Base) + 1;
        zte_memcpy_s(bufferCur, wIElen, (BYTE*)ptUserLocationInfo, wIElen);
        T_IE_Base* ptULIHead = (T_IE_Base*)bufferCur;
        bufferCur+= wIElen;
        *ptDataLen += wIElen;
        if (ptUserLocationInfo->btEutraLocation)
        {
            T_EutraLocation* ptEutraLocation = &(ptUserLocationInfo->eutraLocation);
            zte_memcpy_s(bufferCur, 2, (BYTE*)ptEutraLocation, 2);
            bufferCur+= 2;
            *ptDataLen+=2;
            NCU_ENCODE_LOC_BASE_FIELD(ptEutraLocation->TAI_PlmnLen);
            NCU_ENCODE_LOC_BASE_FIELD(ptEutraLocation->TAI_TacLen);
            NCU_ENCODE_LOC_BASE_FIELD(ptEutraLocation->ECGI_PlmnLen);
            NCU_ENCODE_LOC_BASE_FIELD(ptEutraLocation->ECGI_EutraCellLen);
            if(ptEutraLocation->ALI)
            {
                wIElen = 4;
                zte_memcpy_s(bufferCur, wIElen, (BYTE*)&(ptEutraLocation->ALI_ageOfLIvalue), wIElen);
                bufferCur+= wIElen;
                *ptDataLen+=wIElen;
            }
            if(ptEutraLocation->LTM)
            {
                NCU_ENCODE_LOC_BASE_FIELD(ptEutraLocation->LTMLen);
            }
            if(ptEutraLocation->GGI)
            {
                NCU_ENCODE_LOC_BASE_FIELD(ptEutraLocation->GGILen);
            }
            if(ptEutraLocation->GDI)
            {
                NCU_ENCODE_LOC_BASE_FIELD(ptEutraLocation->GDILen);
            }
            if (ptEutraLocation->NGENB)
            {
                wIElen = sizeof(T_GNB_HEAD);
                zte_memcpy_s(bufferCur, wIElen, (BYTE*)&(ptEutraLocation->head), wIElen);
                bufferCur += wIElen;
                *ptDataLen += wIElen;
                NCU_ENCODE_LOC_BASE_FIELD(ptEutraLocation->NgeNb_Plmn_len);
                if (ptEutraLocation->head.gNbf)
                {
                    wIElen = 2 + ptEutraLocation->NgeNb_gNb_len;
                    zte_memcpy_s(bufferCur, wIElen, (BYTE*)&(ptEutraLocation->NgeNb_gNb_bit_len), wIElen);
                    bufferCur += wIElen;
                    *ptDataLen += wIElen;
                }
                if (ptEutraLocation->head.ngeNbf)
                {
                    NCU_ENCODE_LOC_BASE_FIELD(ptEutraLocation->NgeNb_ngeNbID_len);
                }
            }
        }

        if (ptUserLocationInfo->btNrLocation)
        {
            T_NrLocation* ptNrlocation = &(ptUserLocationInfo->nrLocation);
            zte_memcpy_s(bufferCur, 2, (BYTE*)ptNrlocation, 2);
            bufferCur+= 2;
            *ptDataLen+=2;
            NCU_ENCODE_LOC_BASE_FIELD(ptNrlocation->TAI_PlmnLen);
            NCU_ENCODE_LOC_BASE_FIELD(ptNrlocation->TAI_TacLen);
            NCU_ENCODE_LOC_BASE_FIELD(ptNrlocation->NCGI_PlmnLen);
            NCU_ENCODE_LOC_BASE_FIELD(ptNrlocation->NCGI_NrCellLen);
            if(ptNrlocation->ALI)
            {
                wIElen = 4;
                zte_memcpy_s(bufferCur, wIElen, (BYTE*)&(ptNrlocation->ALI_ageOfLIvalue), wIElen);
                bufferCur+= wIElen;
                *ptDataLen+=wIElen;
            }
            if(ptNrlocation->LTM)
            {
                NCU_ENCODE_LOC_BASE_FIELD(ptNrlocation->LTMLen);
            }
            if(ptNrlocation->GGI)
            {
                NCU_ENCODE_LOC_BASE_FIELD(ptNrlocation->GGILen);
            }
            if(ptNrlocation->GDI)
            {
                NCU_ENCODE_LOC_BASE_FIELD(ptNrlocation->GDILen);
            }
            if(ptNrlocation->GNB)
            {
                wIElen = sizeof(T_GNB_HEAD);
                zte_memcpy_s(bufferCur, wIElen, (BYTE*)&(ptNrlocation->head), wIElen);
                bufferCur+= wIElen;
                *ptDataLen+=wIElen;
                NCU_ENCODE_LOC_BASE_FIELD(ptNrlocation->GNB_Plmn_len);
                if(ptNrlocation->head.gNbf)
                {
                    wIElen = 2+ptNrlocation->NGNB_gNb_len;
                    zte_memcpy_s(bufferCur, wIElen, (BYTE*)&(ptNrlocation->GNB_gNb_bit_len), wIElen);
                    bufferCur+= wIElen;
                    *ptDataLen+=wIElen;
                }
                if(ptNrlocation->head.ngeNbf)
                {
                    NCU_ENCODE_LOC_BASE_FIELD(ptNrlocation->GNB_ngeNbID_len);
                }
            }
        }
        
        if (ptUserLocationInfo->btGeraLoction)
        {
            T_GeraLocation* ptGeraLocation = &(ptUserLocationInfo->geraLocation);
            DEBUG_TRACE(DEBUG_LOW, "ptUserLocationInfo->btGeraLoction=1\n");
            NCU_ENCODE_IE_FIELD(ptGeraLocation, CGI, tCGI);
            NCU_ENCODE_IE_FIELD(ptGeraLocation, SAI, tSAI);
            NCU_ENCODE_IE_FIELD(ptGeraLocation, LAI, tLAI);
            NCU_ENCODE_IE_FIELD(ptGeraLocation, RAI, tRAI);
        }
        
        ptULIHead->length = mcs_htons(*ptDataLen - curTotalLen - sizeof(T_IE_Base)); 
    }
    
    if(ptNotification->has_QosAnaInfo_flg)
    {
        WORD16 curTotalLen = *ptDataLen;
        T_IE_QosAnaInfo* ptQosAnaInfo = &(ptNotification->QosAnaInfo);
        ptQosAnaInfo->Base.type = mcs_htons(IETYPE_QosAnaInfo);
        ptQosAnaInfo->Base.Spare = 0;
        wIElen = sizeof(T_IE_Base);
        zte_memcpy_s(bufferCur,wIElen, &ptQosAnaInfo->Base,wIElen);
        T_IE_Base* ptQosAnaInfoHead = (T_IE_Base*)bufferCur;
        bufferCur+= wIElen;
        *ptDataLen+=wIElen;
        NCU_ENCODE_IE(ptQosAnaInfo, ApplicationID);
        NCU_ENCODE_IE(ptQosAnaInfo, SubAppID);
        NCU_ENCODE_IE(ptQosAnaInfo, ReportType);
        NCU_ENCODE_IE(ptQosAnaInfo, QosMonReports);
        ptQosAnaInfoHead->length = mcs_htons(*ptDataLen - curTotalLen - sizeof(T_IE_Base));
    }
    ptNotificationIEHead->length = mcs_htons(*ptDataLen - sizeof(T_IE_Base));
    ptCorrelation->Base.type = mcs_htons(IETYPE_CorrelationId);
    wIElen = mcs_htons(ptCorrelation->Base.length)+sizeof(T_IE_Base);
    zte_memcpy_s(bufferCur,wIElen, ptCorrelation,wIElen);
    bufferCur+= wIElen;
    *ptDataLen+=wIElen;

    DEBUG_TRACE(DEBUG_LOW, "buffer offset=%lu, datalen=%u\n",bufferCur-buffer, *ptDataLen);
}


BOOLEAN psCheckIPValid(BYTE* ip_addr, BYTE bIpType)
{
    if(NULL == ip_addr)
    {
        return FALSE;
    }
    switch(bIpType)
    {
        case IPv6_VERSION:
        {
            T_IPV6 tIPV6SetZero = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
            T_IPV6 tIPV6SetF    = {255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255};
            if(0 == memcmp(ip_addr, tIPV6SetZero, sizeof(T_IPV6)))
            {
                return FALSE;
            }
            if(0 == memcmp(ip_addr, tIPV6SetF, sizeof(T_IPV6)))
            {
                return FALSE;
            }
            break;
        }
        case IPv4_VERSION:
        {
            T_IPV4 tIPV4SetZero = {0, 0, 0, 0};
            T_IPV4 tIPV4SetF    =  {255, 255, 255, 255};
            if(0 == memcmp(ip_addr, &tIPV4SetZero, sizeof(T_IPV4)))
            {
                return FALSE;
            }
            if(0 == memcmp(ip_addr, &tIPV4SetF,    sizeof(T_IPV4)))
            {
                return FALSE;
            }
            break;
        }
        default: 
        {
             return FALSE;
        }
        break;
    }
    return TRUE;
}

WORD32 setCorrelationIDByDnn(const CHAR* dnn, CHAR* correlation)
{
    if(NULL == dnn || NULL == correlation)
    {
        return MCS_RET_FAIL;
    }
    WORD32 dwIndex = 0;
    for(dwIndex = 0; dwIndex < 16; dwIndex++)
    {
        if(1 == g_CorrData[dwIndex].isValid && 0 == memcmp(dnn, g_CorrData[dwIndex].Dnn, 64))
        {
            zte_memcpy_s(correlation, 256, g_CorrData[dwIndex].CorrelationId, 255);
            return MCS_RET_SUCCESS;
        }
    }
    return MCS_RET_FAIL;
}

VOID showNcuAllCorrData()
{
    WORD32 i = 0;
    for (; i < 16; i++)
    {
        ExpNormalCorrData* ptExpNormalCorrData = &g_CorrData[i];
        if (!ptExpNormalCorrData->isValid)
        {
            continue;
        }
        zte_printf_s("index  : %u\n", i);
        zte_printf_s("dnn    : %s\n", ptExpNormalCorrData->Dnn);
        zte_printf_s("coreid : %s\n", ptExpNormalCorrData->CorrelationId);
    }
}

VOID clearNcuAllCorrData()
{
    WORD32 i = 0;
    for (; i < 16; i++)
    {
        ExpNormalCorrData* ptExpNormalCorrData = &g_CorrData[i];
        zte_memset_s(ptExpNormalCorrData, sizeof(ExpNormalCorrData), 0, sizeof(ExpNormalCorrData));
    }
}

VOID setNcuCorrData(WORD32 id, CHAR* dnn, CHAR* coreId)
{
    if (id > 15 || NULL == dnn || NULL == coreId)
    {
        return ;
    }
    ExpNormalCorrData* ptExpNormalCorrData = &g_CorrData[id];
    zte_memcpy_s(ptExpNormalCorrData->Dnn, sizeof(ptExpNormalCorrData->Dnn), dnn, zte_strnlen_s(dnn, 63));
    zte_memcpy_s(ptExpNormalCorrData->CorrelationId, sizeof(ptExpNormalCorrData->CorrelationId), coreId, zte_strnlen_s(coreId, 255));
}
