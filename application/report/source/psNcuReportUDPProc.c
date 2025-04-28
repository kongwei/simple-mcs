#include "psNcuReportUDPProc.h"
#include "MemShareCfg.h"
#include "psNcuGetCfg.h"
#include "psMcsDebug.h"
#include "ps_mcs_define.h"
#include "McsIPv4Head.h"
#include "McsIPv6Head.h"
#include "McsUDPHead.h"
#include "dpa_rcvsend.h"
#include "dpa_compat.h"
#include "psNcuRouteProc.h"
#include "psNcuDataEncode.h"
#include "ps_mcs_trace.h"
#include "psNcuDataAnalysis.h"
#include "psNcuNWDAFAddrProc.h"
#include "psNcuReportStructHead.h"
#include "ps_ncu_typedef.h"
#ifndef CSS_GLUE_CAST16
    #define CSS_GLUE_CAST16(v) ((WORD16) (v))
#endif
extern uint32_t psCSSDPDKSendOut(PS_PACKET *pPktParaIn);
WORD32 g_send  = 1;
WORD32 getQosSpecialNcuIp(T_psNcuSessionCtx* ptSessionCtx, BYTE* ptUpfIP, BYTE* ptNwdafIP, BYTE* pbRptIpType);
void psNcuReportUDPToNwdafProc(T_psNcuDaAppCtx * ptDaAppCtx, WORD16 threadID, BYTE bRptType,BYTE trigger)
{
    if(NULL == ptDaAppCtx|| threadID >= MEDIA_THRD_NUM)
    {
        return;
    }
    T_psNcuMcsPerform *ptNcuPerform = psVpfuMcsGetPerformPtr(threadID);
    if (NULL == ptNcuPerform)
    {
        return;
    }
    if(bRptType != RPT_TYPE_LOOP && bRptType != RPT_TYPE_STREAM_DEL)
    {
        MCS_LOC_STAT_EX(ptNcuPerform, qwNcuReportUDPNotLoopAndNoDel, 1);
        DEBUG_TRACE(DEBUG_LOW, "unsupport rpt type for udp, rpt=%u\n", bRptType);
        return;
    }
    T_psNcuSessionCtx* ptSessionCtx = ptDaAppCtx->ptSessionCtx;
    if (NULL == ptSessionCtx)
    {
        MCS_LOC_STAT_EX(ptNcuPerform, qwNcuReportUDPNotHasSession, 1);
        return ;
    }
    if(NULL == ptDaAppCtx->ptFlowCtxHead || 0 == ptDaAppCtx->dwStreamNum )
    {
        MCS_LOC_STAT_EX(ptNcuPerform, qwNcuReportUDPWithFlowZero, 1);
        DEBUG_TRACE(DEBUG_LOW,"ptDaAppCtx->ptFlowCtxHead=%p, ptDaAppCtx->dwStreamNum=%u\n", ptDaAppCtx->ptFlowCtxHead,ptDaAppCtx->dwStreamNum);
        return;
    }
    
    T_IPV6 tUpfIP = {0};
    T_IPV6 tNwdafIP = {0};
    BYTE bRptIpType = 0;
    if(trigger == enumQosExpNormal)
    {
        //根据心跳找到好的地址
        BYTE resultIp = getAddrByThreadno(threadID,tNwdafIP);
        if(IPv4_VERSION == resultIp)
        {
            bRptIpType = IPCOMM_TYPE_IPV4;
            MCS_LOC_STAT_EX(ptNcuPerform, getAddrByThreadnoV4, 1);
            getNcuIP(tUpfIP, IPv4_VERSION);
        }
        else if(IPv6_VERSION == resultIp)
        {
            bRptIpType = IPCOMM_TYPE_IPV6;
            MCS_LOC_STAT_EX(ptNcuPerform, getAddrByThreadnoV6, 1);
            getNcuIP(tUpfIP, IPv6_VERSION);
        }
        else
        {
            MCS_LOC_STAT_EX(ptNcuPerform, getAddrByThreadnoFail, 1);
            return;
        }
        
    }
    else if(trigger == enumQosExpSpecial)
    {
        if (MCS_RET_FAIL == getQosSpecialNcuIp(ptSessionCtx, tUpfIP, tNwdafIP, &bRptIpType))
        {
            DEBUG_TRACE(DEBUG_LOW, "no find ip\n");
            MCS_LOC_STAT_EX(ptNcuPerform, qwGetNcuIpFail, 1);
            return;
        }
    }
    BYTE* buffer = &g_ptVpfuShMemVar->aucInnerAckMsg[threadID%MEDIA_THRD_NUM][0];
    WORD16 wNwdafPort = getNwdafPort();
    
    MCS_LOC_STAT_EX(ptNcuPerform, qwRpt2NwdafProc, 1);
    
    WORD16 datalen = psEncodeUDPDataProc(buffer, ptDaAppCtx, bRptType, trigger);
    if(0 == datalen)
    {
        DEBUG_TRACE(DEBUG_LOW, "psEncodeUDPDataProc datalen=0\n");
        return;
    }
    
    PS_PACKET *ptPacket  = psCSSGenPkt(buffer, datalen, threadID);
    
    if(NULL == ptPacket)
    {
        DEBUG_TRACE(DEBUG_LOW, "ptPacket=NULL\n");
        return;
    }
    PACKET_PROCESS_START(ptPacket, PKTCHK_CSS_MEDIA);
    if(bRptIpType == IPCOMM_TYPE_IPV6)
    {
        psMcsUDPIpv6HeadCap(ptPacket, wNwdafPort, wNwdafPort, datalen+8, tUpfIP, tNwdafIP);
        psMcsIPv6HeadCap(ptPacket, tUpfIP, tNwdafIP,  datalen+8, 0, MCS_UDP_PROTOCOL);
        if(MCS_RET_SUCCESS != psNcuUlIPV6GiRouteGet(ptPacket, tNwdafIP))
        {
            DEBUG_TRACE(DEBUG_ERR, "psNcuUlIPV6GiRouteGet fail\n");
            MCS_LOC_STAT_EX(ptNcuPerform, qwGetNwdafV6RouteFail, 1);
            psCSSPktLinkBuffDesFree(ptPacket, PKTCHK_CSS_MEDIA);
            return;
        }
    }
    else if(bRptIpType == IPCOMM_TYPE_IPV4)
    {
        DEBUG_TRACE(DEBUG_LOW, "tUPFIP=%u.%u.%u.%u, tNwdafIP=%u.%u.%u.%u, port=%u\n", tUpfIP[0], tUpfIP[1], tUpfIP[2], tUpfIP[3], tNwdafIP[0], tNwdafIP[1], tNwdafIP[2],tNwdafIP[3], wNwdafPort);
        psMcsUDPHeadCap(ptPacket, wNwdafPort, wNwdafPort, datalen+8);
        psMcsIPv4HeadCap(ptPacket,  tUpfIP, tNwdafIP, 0, MCS_UDP_PROTOCOL);
        if(MCS_RET_SUCCESS != psNcuUlIPV4GiRouteGet(ptPacket, tNwdafIP))
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
    
    /* 支持外层分片 */

    //     _mcs_else
    PKT_TRACE(DEBUG_LOW, ptPacket);
    BYTE *aucPacketBuf  = NULL; /* 当前mbuf实际提供的内存 */
    WORD16 wBufferLen   = 0;    /* 当前mbuf实际提供的长度 */
    WORD16 wBufferOffset= 0;    /* 当前mbuf偏移 */ 
    PS_GET_BUF_INFO(ptPacket->pBufferNode, aucPacketBuf, wBufferOffset, wBufferLen);

    SigTrace_NCU(ptSessionCtx, EXP_REPORT_TYPE, (aucPacketBuf + wBufferOffset), ptPacket->dwPktLen);


    if(g_send == 1)
    {
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
        if (RPT_TYPE_LOOP == bRptType)
        {
            MCS_LOC_STAT_EX(ptNcuPerform, qwTimerLoopRpt, 1);
        }
        else
        {
            MCS_LOC_STAT_EX(ptNcuPerform, qwDelStreamRpt, 1);
        }
        #ifdef FT_TEST
        extern void psCssFtCollectPkt(PS_PACKET* pkt);
        psCssFtCollectPkt(ptPacket);
        psCSSPktLinkBuffDesFree(ptPacket,PKTCHK_NONE);
        return;
        #endif
    }
    else
    {
        if (unlikely(ptPacket->bSthr >= CSS_X86_MAX_THREAD_NUM))
        {
            psCSSPktLinkBuffDesFree(ptPacket,PKTCHK_CSS_MEDIA);
            return;
        }
         BUFFER_NODE *ptBufNode = NULL;
        BYTE *ptBuffer = NULL;
        WORD16  wOffset = 0;
        WORD16  wBufLen = 0;
        PS_GET_FIRST_BUF_INFO(ptPacket, ptBufNode, ptBuffer,wOffset,wBufLen);
        wBufLen = wBufLen;
        WORD32 dwAllHdrSize = sizeof(DPA_ETHERNET_HEAD);

        CSS_VLAN_ID_RANGE_CHECK_CONDITION(ptPacket->wVlanId)
        {
            dwAllHdrSize = sizeof(DPA_ETHERNET_HEAD_WITH_VLAN);
        }
        if(wOffset < dwAllHdrSize)
        {
            DEBUG_TRACE(DEBUG_LOW,"offset=%u, AllHdrSize=%u\n", wOffset, dwAllHdrSize);
            return;
        }
        wOffset -= (WORD16)dwAllHdrSize;
        wBufLen += (WORD16)dwAllHdrSize;
        ptPacket->dwPktLen += dwAllHdrSize;
        PS_SET_BUF_INFO(ptPacket, ptBufNode, wOffset, wBufLen);
        CSS_VLAN_ID_RANGE_CHECK_CONDITION(ptPacket->wVlanId)
        {
            DPA_ETHERNET_HEAD_WITH_VLAN *ptVlanHdr = (DPA_ETHERNET_HEAD_WITH_VLAN *)(ptBuffer+wOffset);
            uint16_t vlanid = 0;
            CSS_MEMCPY(ptVlanHdr->tDstMac, ptPacket->tDestMac, sizeof(T_Mac));
            CSS_MEMCPY(ptVlanHdr->tSrcMac, ptPacket->tSrcMac, sizeof(T_Mac));
            ptVlanHdr->wVlanType = (uint16_t)CSS_FWD_HTON16((uint16_t)CSS_ETHERNET_TYPE_IS_8021Q);
            vlanid = ptPacket->wVlanId | (((WORD16)ptPacket->wVlanPri) << 13);
            ptVlanHdr->wVlanId = (uint16_t)CSS_FWD_HTON16(vlanid);
            ptVlanHdr->wL2PktType = (uint16_t)CSS_FWD_HTON16(ptPacket->wL2PktType);
        }
        else
        {
            DPA_ETHERNET_HEAD *ptEthHdr = (DPA_ETHERNET_HEAD *)(ptBuffer+wOffset);
            CSS_MEMCPY(ptEthHdr->tDstMac,ptPacket->tDestMac,sizeof(T_Mac));
            CSS_MEMCPY(ptEthHdr->tSrcMac,ptPacket->tSrcMac,sizeof(T_Mac));
            ptEthHdr->wL2PktType = CSS_FWD_HTON16(ptPacket->wL2PktType);
        }
        
        WORD32 dwThr = (WORD32)ptPacket->bSthr;

        ptPacket->ShelfNo = (uint8_t)((ptPacket->dwOutPort >> 24) & 0x7F);
        ptPacket->SlotNo = (uint8_t)((ptPacket->dwOutPort >> 22) & 0x3);
        ptPacket->SubSlotNo = (uint8_t)((ptPacket->dwOutPort >> 17) & 0x1F);
        ptPacket->InterfacePortNo = (uint8_t)((ptPacket->dwOutPort >> 12) & 0x1F);
        ptPacket->SubInterfaceNo = (uint16_t)(ptPacket->dwOutPort & 0xFFF);

        psFtmSubPortPerfECount(dwThr, ptPacket->dwOutPort & 0x1FFFF, ptPacket->dwPktLen);

        PACKET_PROCESS_END(ptPacket,PKTCHK_CSS_MEDIA);

        if (likely(CSS_OK == psCSSDPDKSendOut(ptPacket)))
        {
            DEBUG_TRACE(DEBUG_LOW, "psCSSDPDKSendOut succ\n");
            MCS_LOC_STAT_EX(ptNcuPerform, qwSndRpt2NwdafSucc, 1);
            if (RPT_TYPE_LOOP == bRptType)
            {
                MCS_LOC_STAT_EX(ptNcuPerform, qwTimerLoopRpt, 1);
            }
            else
            {
                MCS_LOC_STAT_EX(ptNcuPerform, qwDelStreamRpt, 1);
            }
            return;
        }
        else
        {
            DEBUG_TRACE(DEBUG_LOW, "psCSSDPDKSendOut fail\n");
            MCS_LOC_STAT_EX(ptNcuPerform, qwSndRpt2NwdafFail, 1);
            psCSSPktLinkBuffDesFree(ptPacket,PKTCHK_NONE);
            return;
        }
    }
   
}


WORD32 getQosSpecialNcuIpv4(T_psNcuSessionCtx* ptSessionCtx, BYTE* ptUpfIP, BYTE* tNwdafIP)
{
    MCS_CHK_NULL_RET(ptSessionCtx, MCS_RET_FAIL);
    MCS_CHK_NULL_RET(ptUpfIP, MCS_RET_FAIL);
    MCS_CHK_NULL_RET(tNwdafIP, MCS_RET_FAIL);

    if ((ptSessionCtx->bNwdafIPType & IPCOMM_TYPE_IPV4) && (MCS_RET_SUCCESS == getNcuIP(ptUpfIP, IPv4_VERSION)))
    {
        IPV4_COPY(tNwdafIP, ptSessionCtx->tNwdafIPv4);
        return MCS_RET_SUCCESS;
    }
    return MCS_RET_FAIL;
}

WORD32 getQosSpecialNcuIpv6(T_psNcuSessionCtx* ptSessionCtx, BYTE* ptUpfIP, BYTE* tNwdafIP)
{
    MCS_CHK_NULL_RET(ptSessionCtx, MCS_RET_FAIL);
    MCS_CHK_NULL_RET(ptUpfIP, MCS_RET_FAIL);
    MCS_CHK_NULL_RET(tNwdafIP, MCS_RET_FAIL);

    if ((ptSessionCtx->bNwdafIPType & IPCOMM_TYPE_IPV6) && (MCS_RET_SUCCESS == getNcuIP(ptUpfIP, IPv6_VERSION)))
    {
        IPV6_COPY(tNwdafIP, ptSessionCtx->tNwdafIPv6);
        return MCS_RET_SUCCESS;
    }
    return MCS_RET_FAIL;
}

/* Started by AICoder, pid:ha10d0e947w4fce140dd0aab806ea1481af2877c */
WORD32 getQosSpecialNcuIp(T_psNcuSessionCtx* ptSessionCtx, BYTE* ptUpfIP, BYTE* tNwdafIP, BYTE* pbRptIpType)
{
    MCS_CHK_NULL_RET(ptSessionCtx, MCS_RET_FAIL);
    MCS_CHK_NULL_RET(ptUpfIP, MCS_RET_FAIL);
    MCS_CHK_NULL_RET(tNwdafIP, MCS_RET_FAIL);
    MCS_CHK_NULL_RET(pbRptIpType, MCS_RET_FAIL);

    if (ncuGetPreferAddrType() == 1)
    {
        if (MCS_RET_SUCCESS == getQosSpecialNcuIpv6(ptSessionCtx, ptUpfIP, tNwdafIP))
        {
            *pbRptIpType = IPCOMM_TYPE_IPV6;
            return MCS_RET_SUCCESS;
        }
        if (MCS_RET_SUCCESS == getQosSpecialNcuIpv4(ptSessionCtx, ptUpfIP, tNwdafIP))
        {
            *pbRptIpType = IPCOMM_TYPE_IPV4;
            return MCS_RET_SUCCESS;
        }
    }
    else
    {
        if (MCS_RET_SUCCESS == getQosSpecialNcuIpv4(ptSessionCtx, ptUpfIP, tNwdafIP))
        {
            *pbRptIpType = IPCOMM_TYPE_IPV4;
            return MCS_RET_SUCCESS;
        }
        if (MCS_RET_SUCCESS == getQosSpecialNcuIpv6(ptSessionCtx, ptUpfIP, tNwdafIP))
        {
            *pbRptIpType = IPCOMM_TYPE_IPV6;
            return MCS_RET_SUCCESS;
        }
    }

    return MCS_RET_FAIL;
}
/* Ended by AICoder, pid:ha10d0e947w4fce140dd0aab806ea1481af2877c */

WORD32 getExpSpecialNcuIp(T_psNcuSessionCtx* ptSessionCtx, BYTE* ptUpfIP, BYTE* tNwdafIP, BYTE* pbRptIpType)
{
    MCS_CHK_NULL_RET(ptSessionCtx, MCS_RET_FAIL);
    MCS_CHK_NULL_RET(ptUpfIP, MCS_RET_FAIL);
    MCS_CHK_NULL_RET(tNwdafIP, MCS_RET_FAIL);  
    MCS_CHK_NULL_RET(pbRptIpType, MCS_RET_FAIL); 

    if (ncuGetPreferAddrType() == UPF_IP_PREFER_ADDR_TYPE_IPV6)
    {
        if (MCS_RET_SUCCESS == getQosSpecialNcuIpv6(ptSessionCtx, ptUpfIP, tNwdafIP))
        {
            *pbRptIpType = IPv6_VERSION;
            return MCS_RET_SUCCESS;
        }
        if (MCS_RET_SUCCESS == getQosSpecialNcuIpv4(ptSessionCtx, ptUpfIP, tNwdafIP))
        {
            *pbRptIpType = IPv4_VERSION;
            return MCS_RET_SUCCESS;
        }
    }
    else
    {
        if (MCS_RET_SUCCESS == getQosSpecialNcuIpv4(ptSessionCtx, ptUpfIP, tNwdafIP))
        {
            *pbRptIpType = IPv4_VERSION;
            return MCS_RET_SUCCESS;
        }
        if (MCS_RET_SUCCESS == getQosSpecialNcuIpv6(ptSessionCtx, ptUpfIP, tNwdafIP))
        {
            *pbRptIpType = IPv6_VERSION;
            return MCS_RET_SUCCESS;
        }
    }
    return MCS_RET_FAIL;
}
