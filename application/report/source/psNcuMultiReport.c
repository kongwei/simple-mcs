#include "psNcuMultiReport.h"
#include "psNcuRouteProc.h"
#include "psNcuNWDAFAddrProc.h"
#include "ps_ncu_session.h"
#include "McsIPv6Head.h"
#include "McsIPv4Head.h"
#include "MemShareCfg.h"
#include "psMcsDebug.h"
#include "psNcuGetCfg.h"
#include "psNcuDataEncode.h"
#include "psNcuDataAnalysis.h"
#include "McsUDPHead.h"
#include "dpa_rcvsend.h"
#include "ps_ncu_typedef.h"
#include "dpa_compat.h"
#include "psNcuReportUDPProc.h"

#define NCU_NO_NEED_RPT(ptDaAppCtx) (NULL == ptDaAppCtx || NULL == ptDaAppCtx->ptFlowCtxHead || 0 == ptDaAppCtx->dwStreamNum )
#define MULTI_STAT(pNcuPerform, group_len) \
do{\
    if(group_len>=3){\
        MCS_LOC_STAT_EX(pNcuPerform, qwMultiRptGroupNumMoreThan2, 1);\
    }\
    else {\
        MCS_LOC_STAT_EX(pNcuPerform, qwMultiRptGroupNumLess2, 1);\
    }\
}while(0)
T_psMultiGroupData g_tMultiGroup[MEDIA_THRD_NUM][GROUP_RPT_MAX_NUM] = {0};

void psNcuReportWithBuffer(T_psMultiGroupData* ptGroupData, BYTE* buffer, WORD16 datalen, WORD16 threadID, BYTE trigger);

WORD32 psCSSGSUMediaSendOutRust(PS_PACKET *ptPkt)
{
#ifndef FT_TEST
    return psCSSGSUMediaSendOut(ptPkt);
#else
    return 0;
#endif
}

void getNormalExpMultiRptInfo(WORD16 threadID, T_psMultiGroupData* ptGroupData)
{
    if(NULL == ptGroupData || 0 == ptGroupData->bAppCtxNum)
    {
        return;
    }
    T_psNcuMcsPerform *ptNcuPerform = psVpfuMcsGetPerformPtr(threadID);
    BYTE bResult = getAddrByThreadno(threadID,ptGroupData->tNwdafIP);
    if(0 == bResult)
    {
        MCS_LOC_STAT_EX(ptNcuPerform, getAddrByThreadnoFail, 1);
        return;
    }
    ptGroupData->bIsValid = TRUE;
    ptGroupData->bIPType = bResult;
    WORD16 wMtu = psNcuGetMtu(ptGroupData->tNwdafIP,bResult,threadID);
    if(0 == wMtu)
    {
        MCS_LOC_STAT_EX(ptNcuPerform, getMtuFailForMultiRpt, 1);
        return;
    }
    ptGroupData->wMtu = wMtu;
    return;
}


WORD32 psCheckExpSpecialAddAppCtxToGroup(T_psNcuDaAppCtx * ptDaAppCtx, T_psMultiGroupData* ptGroupData, WORD16 threadID)
{
    if(NULL == ptGroupData || NULL == ptDaAppCtx)
    {
        return MCS_RET_FAIL;
    }
    T_psNcuMcsPerform *ptNcuPerform = psVpfuMcsGetPerformPtr(threadID);
    T_psNcuSessionCtx* ptSessionCtx = ptDaAppCtx->ptSessionCtx;
    T_IPV6 tUpfIP = {0};
    T_IPV6 tNwdafIP = {0};
    BYTE bRptIpType = 0;
    if(NULL == ptSessionCtx)
    {
        return MCS_RET_FAIL;
    }
    if (MCS_RET_FAIL == getExpSpecialNcuIp(ptSessionCtx, tUpfIP, tNwdafIP, &bRptIpType))
    {
        DEBUG_TRACE(DEBUG_LOW, "no find ip\n");
        MCS_LOC_STAT_EX(ptNcuPerform, qwGetNcuIpFail, 1);
        return MCS_RET_FAIL;
    }
    
    WORD32 dwIndex = 1;
    for(dwIndex=1;dwIndex <GROUP_RPT_MAX_NUM ;dwIndex++)
    {
        if((TRUE == ptGroupData[dwIndex].bIsValid) && ptGroupData[dwIndex].bIPType == bRptIpType && (0==memcmp(tNwdafIP,ptGroupData[dwIndex].tNwdafIP, IPV6_LEN)))
        {
            if(ptGroupData[dwIndex].bAppCtxNum < GROUP_APP_MAX_NUM)
            {
                MCS_LOC_STAT_EX(ptNcuPerform, qwExpSpecialMultiRptGetOne, 1);
                ptGroupData[dwIndex].ptAppCtx[ptGroupData[dwIndex].bAppCtxNum++] = ptDaAppCtx;
            }
            else
            {
                MCS_LOC_STAT_EX(ptNcuPerform, qwExpSpecialMultiRptMoreThan10, 1);
                return MCS_RET_FAIL;
            }
            return MCS_RET_SUCCESS;
        }
        if(FALSE == ptGroupData[dwIndex].bIsValid)
        {
            MCS_LOC_STAT_EX(ptNcuPerform, qwExpSpecialMultiRptGetOne, 1);
            ptGroupData[dwIndex].bIsValid = TRUE;
            ptGroupData[dwIndex].ptAppCtx[0] = ptDaAppCtx;
            ptGroupData[dwIndex].bAppCtxNum = 1;
            IPV6_COPY(ptGroupData[dwIndex].tNwdafIP,tNwdafIP);
            ptGroupData[dwIndex].bIPType = bRptIpType;
            ptGroupData[dwIndex].wMtu  = psNcuGetMtu(tNwdafIP,bRptIpType,threadID);
            return MCS_RET_SUCCESS;
        }
    }
    if(dwIndex == GROUP_RPT_MAX_NUM)
    {
        MCS_LOC_STAT_EX(ptNcuPerform, qwMultiRptMoreThan32, 1);
        return MCS_RET_FAIL;
    }
    return MCS_RET_SUCCESS;
}
WORD32 psCheckExpNormalAddAppCtxToGroup(T_psNcuDaAppCtx * ptDaAppCtx, T_psMultiGroupData* ptGroupData, WORD16 threadID)
{
    if(NULL == ptDaAppCtx || NULL == ptGroupData)
    {
        return MCS_RET_FAIL;
    }
    T_psNcuMcsPerform *ptNcuPerform = psVpfuMcsGetPerformPtr(threadID);
    if(ptGroupData[GROUP_EXP_NORMAL].bAppCtxNum < GROUP_APP_MAX_NUM)
    {
        MCS_LOC_STAT_EX(ptNcuPerform, qwExpNormalMultiRptGetOne, 1);
        ptGroupData[GROUP_EXP_NORMAL].ptAppCtx[ptGroupData[GROUP_EXP_NORMAL].bAppCtxNum++] = ptDaAppCtx;
    }
    else
    {
        MCS_LOC_STAT_EX(ptNcuPerform, qwExpNormalMultiRptMoreThan10, 1);
        return MCS_RET_FAIL;
    }
    return MCS_RET_SUCCESS;
}


void psNcuReSetDataInfo(T_psMultiGroupData* ptGroupData, WORD16 threadID)
{
    WORD32 dwIndex = 0;
    BYTE bAppCtxExpNum = 0;
    BYTE bAppCtxIndex = 0;
    T_psNcuDaAppCtx * ptDaAppCtx = NULL;
    for(dwIndex=0; dwIndex < GROUP_RPT_MAX_NUM; dwIndex++)
    {
        if(FALSE == ptGroupData[dwIndex].bIsValid)
        {
            continue;
        }
        bAppCtxExpNum = ptGroupData[dwIndex].bAppCtxNum;
        for(bAppCtxIndex=0;bAppCtxIndex<ptGroupData[dwIndex].bAppCtxNum && bAppCtxIndex<GROUP_APP_MAX_NUM;bAppCtxIndex++)
        {
            ptDaAppCtx = ptGroupData[dwIndex].ptAppCtx[bAppCtxIndex];
            psNcuRestoreDataAppAfterRpt(ptDaAppCtx, threadID);
        }
    }
}

void psNcuReportMultiProc(T_psMultiGroupData* ptGroupData, WORD16 threadID)
{
    T_psNcuMcsPerform *ptNcuPerform = psVpfuMcsGetPerformPtr(threadID);
    WORD32 dwIndex = 0;
    BYTE trigger = enumQosExpNormal;
    BYTE bAppCtxExpNum = 0;
    WORD16 wMtu = 0;
    T_psNcuDaAppCtx * ptDaAppCtx = NULL;
    BYTE* buffer = &g_ptVpfuShMemVar->aucInnerAckMsg[threadID%MEDIA_THRD_NUM][0];
    for(dwIndex=0; dwIndex < GROUP_RPT_MAX_NUM; dwIndex++)
    {
        MCS_CHK_CONTINUE(FALSE == ptGroupData[dwIndex].bIsValid);
        if(dwIndex != GROUP_EXP_NORMAL)
        {
            trigger = enumQosExpSpecial;
        }
        bAppCtxExpNum = ptGroupData[dwIndex].bAppCtxNum;
        wMtu = ptGroupData[dwIndex].wMtu;
        BYTE bAppCtxIndex = 0;
        WORD16 datalen = 0, tmplen = 0, groupnum = 0;
        for(bAppCtxIndex=0;bAppCtxIndex<ptGroupData[dwIndex].bAppCtxNum && bAppCtxIndex<GROUP_APP_MAX_NUM;bAppCtxIndex++)
        {
            ptDaAppCtx = ptGroupData[dwIndex].ptAppCtx[bAppCtxIndex];
            if(NCU_NO_NEED_RPT(ptDaAppCtx))
            {
                MCS_LOC_STAT_EX(ptNcuPerform, qwNcuReportUDPWithFlowZero, 1);
                DEBUG_TRACE(DEBUG_LOW,"ptDaAppCtx->ptFlowCtxHead=%p, ptDaAppCtx->dwStreamNum=%u\n", ptDaAppCtx->ptFlowCtxHead,ptDaAppCtx->dwStreamNum);
                continue;
            }
            tmplen = psEncodeUDPDataProc(buffer+datalen, ptDaAppCtx, RPT_TYPE_LOOP, trigger);
            MCS_CHK_CONTINUE(tmplen == 0);
            if(datalen+ tmplen > wMtu)
            {
                if(datalen==0)
                {
                    MCS_LOC_STAT_EX(ptNcuPerform, qwMultiRptMtuLessThenOnePkt, 1);
                    continue;
                }
                MULTI_STAT(ptNcuPerform, groupnum);
                psNcuReportWithBuffer(&ptGroupData[dwIndex],buffer, datalen,threadID,trigger);
                psNcuReportWithBuffer(&ptGroupData[dwIndex],buffer+datalen, tmplen,threadID,trigger);
                datalen = 0;
                tmplen = 0;
            }
            datalen += tmplen;
            groupnum ++;
        }
        if(0 != datalen)
        {
            psNcuReportWithBuffer(&ptGroupData[dwIndex],buffer, datalen,threadID,trigger);
        }
    }
}
void psNcuReportWithBuffer(T_psMultiGroupData* ptGroupData, BYTE* buffer, WORD16 datalen, WORD16 threadID, BYTE trigger)
{
    T_psNcuMcsPerform *ptNcuPerform = psVpfuMcsGetPerformPtr(threadID);
    if(NULL == ptGroupData || NULL == buffer || NULL == ptNcuPerform)
    {
        return;
    }
    
    WORD16 wNwdafPort = getNwdafPort();
    MCS_LOC_STAT_EX(ptNcuPerform, qwRpt2NwdafProc, 1);
    PS_PACKET *ptPacket  = psCSSGenPkt(buffer, datalen, threadID);
    
    if(NULL == ptPacket)
    {
        DEBUG_TRACE(DEBUG_LOW, "ptPacket=NULL\n");
        return;
    }
    PACKET_PROCESS_START(ptPacket, PKTCHK_CSS_MEDIA);
    T_IPV6 tUpfIP = {0};
    getNcuIP(tUpfIP, ptGroupData->bIPType);
    if(ptGroupData->bIPType == IPv6_VERSION)
    {
        psMcsUDPIpv6HeadCap(ptPacket, wNwdafPort, wNwdafPort, datalen+8, tUpfIP, ptGroupData->tNwdafIP);
        psMcsIPv6HeadCap(ptPacket, tUpfIP, ptGroupData->tNwdafIP,  datalen+8, 0, MCS_UDP_PROTOCOL);
        if(MCS_RET_SUCCESS != psNcuUlIPV6GiRouteGet(ptPacket, ptGroupData->tNwdafIP))
        {
            DEBUG_TRACE(DEBUG_ERR, "psNcuUlIPV6GiRouteGet fail\n");
            MCS_LOC_STAT_EX(ptNcuPerform, qwGetNwdafV6RouteFail, 1);
            psCSSPktLinkBuffDesFree(ptPacket, PKTCHK_CSS_MEDIA);
            return;
        }
    }
    else if(ptGroupData->bIPType == IPv4_VERSION)
    {
        DEBUG_TRACE(DEBUG_LOW, "tUPFIP=%u.%u.%u.%u, tNwdafIP=%u.%u.%u.%u, port=%u\n", tUpfIP[0], tUpfIP[1], tUpfIP[2], tUpfIP[3], ptGroupData->tNwdafIP[0], ptGroupData->tNwdafIP[1], ptGroupData->tNwdafIP[2],ptGroupData->tNwdafIP[3], wNwdafPort);
        psMcsUDPHeadCap(ptPacket, wNwdafPort, wNwdafPort, datalen+8);
        psMcsIPv4HeadCap(ptPacket,  tUpfIP, ptGroupData->tNwdafIP, 0, MCS_UDP_PROTOCOL);
        if(MCS_RET_SUCCESS != psNcuUlIPV4GiRouteGet(ptPacket, ptGroupData->tNwdafIP))
        {
            DEBUG_TRACE(DEBUG_LOW, "psNcuUlIPV4GiRouteGet fail\n");
            MCS_LOC_STAT_EX(ptNcuPerform, qwGetNwdafV4RouteFail, 1);
            psCSSPktLinkBuffDesFree(ptPacket, PKTCHK_CSS_MEDIA);
            return;
        }
    }
    else
    {
        DEBUG_TRACE(DEBUG_LOW, "no find any ip no report\n");
        psCSSPktLinkBuffDesFree(ptPacket, PKTCHK_CSS_MEDIA);
        return;
    }
    PKT_TRACE(DEBUG_LOW, ptPacket);
    

    #ifndef FT_TEST
    if(CSS_OK != psCSSGSUMediaSendOut(ptPacket))
    {
        
        MCS_LOC_STAT_EX(ptNcuPerform, qwSndRpt2NwdafFail, 1);
        return;
    }
    #endif
    
    MCS_LOC_STAT_EX(ptNcuPerform, qwSndRpt2NwdafSucc, 1);
    if (trigger == enumQosExpNormal)
    {
        NCU_PM_55303_STAT_ADD(qwNcuReportOrdinaryExpDataNumViaUDP, 1);
    }
    else if (trigger == enumQosExpSpecial)
    {
        NCU_PM_55303_STAT_ADD(qwNcuReportKeyExpDataNumViaUDP, 1);
    }

    MCS_LOC_STAT_EX(ptNcuPerform, qwTimerLoopRpt, 1);

    #ifdef FT_TEST
    extern void psCssFtCollectPkt(PS_PACKET* pkt);
    psCssFtCollectPkt(ptPacket);
    psCSSPktLinkBuffDesFree(ptPacket,PKTCHK_NONE);
    return;
    #endif
}

WORD32 g_show_mul = 0;
void psNcuShowMultiRptData()
{
    if(0 == g_show_mul)
    {
        return;
    }
    WORD32 dwThrdIdx = 0;
    WORD32 dwGroupIdx = 0;
    WORD32 dwAppIdx   = 0;
    T_psMultiGroupData* ptMultiGroup = NULL;
    for(;dwThrdIdx<MEDIA_THRD_NUM;dwThrdIdx++)
    {
        for(dwGroupIdx=0;dwGroupIdx<GROUP_RPT_MAX_NUM;dwGroupIdx++)
        {
            ptMultiGroup =  &g_tMultiGroup[dwThrdIdx][dwGroupIdx];
            MCS_CHK_CONTINUE(0 == ptMultiGroup->bIsValid);
            zte_printf_s("\nThread %d group %d has multiRptData:\n", dwThrdIdx, dwGroupIdx);
            /* Started by AICoder, pid:t6b87rea0149273146360b1b60b9cb09721694f7 */
            zte_printf_s("wMtu: %u\n", ptMultiGroup->wMtu);
            zte_printf_s("bTrigger: %u\n", ptMultiGroup->bTrigger);
            zte_printf_s("bIPType: %u\n", ptMultiGroup->bIPType);
            if(4 == ptMultiGroup->bIPType)
            {
                zte_printf_s("  NwdafIPv4: %u.%u.%u.%u\n", ptMultiGroup->tNwdafIP[0],ptMultiGroup->tNwdafIP[1],ptMultiGroup->tNwdafIP[2],ptMultiGroup->tNwdafIP[3]);
            }
            else
            {
                zte_printf_s("  NwdafIPv6: %02x%02x:%02x%02x:%02x%02x:%02x%02x:\n", ptMultiGroup->tNwdafIP[0],ptMultiGroup->tNwdafIP[1],ptMultiGroup->tNwdafIP[2],ptMultiGroup->tNwdafIP[3],
                ptMultiGroup->tNwdafIP[4],ptMultiGroup->tNwdafIP[5],ptMultiGroup->tNwdafIP[6],ptMultiGroup->tNwdafIP[7]);
            }
            zte_printf_s("bAppCtxNum: %u\n", ptMultiGroup->bAppCtxNum);
            for (dwAppIdx = 0; dwAppIdx < GROUP_APP_MAX_NUM && dwAppIdx < ptMultiGroup->bAppCtxNum; ++dwAppIdx) 
            {
                zte_printf_s("\tptAppCtx[%d]: %p\n",dwAppIdx, ptMultiGroup->ptAppCtx[dwAppIdx]);
            }
            /* Ended by AICoder, pid:t6b87rea0149273146360b1b60b9cb09721694f7 */
        }
        
    }
}

T_psMultiGroupData* psGetMultiGroup(WORD16 threadID, WORD16 groupID) 
{
    return &(g_tMultiGroup[threadID%MEDIA_THRD_NUM][groupID%GROUP_RPT_MAX_NUM]);
}
