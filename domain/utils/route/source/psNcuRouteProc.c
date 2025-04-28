#include "psNcuRouteProc.h"
#include "ps_mcs_define.h"
#include "ps_ftm_pub.h"
#include "McsIPv4Head.h"
#include "McsIPv6Head.h"
#include "ps_css_interface.h"
#include "psNcuGetCfg.h"
#include "psMcsDebug.h"
#include "psMcsRouteDebug.h"
#include "McsUDPHead.h"
#define FTM_VRF_ZERO_FLAG 0
WORD32 psNcuGetItemIndex(T_psFtmLdshHeadInfo *ptLashInfo);
WORD32 psNcuGetV6ItemSum(T_psFtmV6LdshHeadInfo *ptV6LashInfo);

#define PS_NCU_IPV6_GI_ROUTE_JMP_IP_LEN 16

enum {
    PS_FTM_SMARTGROUP,
    PS_FTM_PORT,
    PS_FTM_GRETUNNEL,
    PS_FTM_V6TUNNEL,
    PS_FTM_IPSEC_TUNNEL,
    PS_FTM_PORT_NOT_SUPPORT
};
typedef struct
{
    WORD32 dwLBFactor_PlcyRoute; /* 分担因子 - 策略路由负荷分担使用*/
    WORD32 dwLBFactor_LDSH;    /* 分担因子 -多下一跳负荷分担使用*/
    WORD32 dwLBFactor_GRE; /* 分担因子 -GRE负荷分担使用*/
    WORD32 dwLBFactor_SMTGRP; /* 分担因子 -SmartGoup 使用*/
    WORD32 dwLBFactor_NAT; /* 分担因子 -NAT使用*/
    WORD32 dwPad;
}T_RouteLBFList;

typedef struct tagT_psMcsRtInfo
{
    /* 路由输出 */
    BYTE    bIPTos;                  /*  IP传输优先级 */
    BYTE    blGreFlag;               /* 是否GRE隧道路由 */
    WORD16  wTunnelEncapLen;
    WORD32  dwOutPortIndex;          /*  出接口索引*/
    /* 路由输入 */
    BYTE bVtNo;                      /* 策略路由模板号 */
    BYTE bPROnly;
    BYTE bNFTranferFlag;
    BYTE bGetMacErrFlag:1;
    BYTE bRsv:7;
    WORD32 dw6To4DstAddr;            /* 本地字节序 */
    T_RouteLBFList   tLdBlceFactor ; /* 路由负荷因子 */
    T_psFtmIPv4AclTcamKey tV4AclKey; /* ipv4 acl */
    T_psFtmIPv6AclTcamKey tV6AclKey; /* ipv6 acl */
}T_psMcsRtInfo;

WORD32 g_set_mtu = 1500;
WORD32 psNcuMcsIPv4RouteGetEnd(PS_PACKET *ptPacket, T_psMcsRtInfo *ptRtOut, WORD32 dwIndex)
{
    WORD32                      dwPortMtu  = 0;/* interface 的mtu */
    WORD32                      dwRouteMtu = 0; /* 最终的路由mtu，结合gre外部隧道计算 */

    T_psFtmPortInfo             *ptFtmPortInfo = psFtmGetPortInfo(dwIndex);
    if (unlikely(NULL == ptFtmPortInfo))
    {
        showV4Route(0,0,NULL);
        return MCS_RET_FAIL;
    }
    /* dwRouteMtu先初始化为 dwPortMtu*/
    dwPortMtu = ptFtmPortInfo->dwMtu;
    dwRouteMtu = dwPortMtu;
    /* 计算内层IP的MTU值，上行报文转发不考虑业务层的MTU */
    if (unlikely(dwRouteMtu <= ptRtOut->wTunnelEncapLen))
    {
        return MCS_RET_FAIL;
    }
    ptPacket->wMtu = (WORD16)(dwRouteMtu - ptRtOut->wTunnelEncapLen);

    /* 填充报文转发需要的其他字段 */
    ptPacket->bInOutFlag = CSS_FWD_TO_PFU_SENDOUT;
    ptPacket->dwOutPort  = ptFtmPortInfo->dwGport;
    /* 获取vlanID */
    WORD16 wVlanId = psCssGetVlanIdByGPort(ptPacket->dwOutPort);
    ptPacket->wVlanId  = wVlanId;
    DEBUG_TRACE(DEBUG_LOW,"vlan=%u", ptPacket->wVlanId);
    
    psCssGetPhysInfoByOutPort(ptPacket); //获取ptPacket->InterfacePortNo等
    /* 根据出port判断是否是本板 */
    /*标记获取出端口标志位，CSS调用----只有本板的时候才能这样获取,非本板css会替换bOutPortNo */
    ptPacket->btGetOutPortFlag = ptPacket->bPfuMacCmpFlag;
    if(ptPacket->bPfuMacCmpFlag)
    {
        //GportExchange_modify
        ptPacket->bOutPortNo = dpa_select_port_by_porttype_lb((BYTE)ptPacket->InterfacePortNo, (BYTE)ptRtOut->tLdBlceFactor.dwLBFactor_LDSH);
    }
    ptPacket->bPFUHeaderType = 3; //ptPacket->bPFUHeaderType = CSS_FWD_PFUTYPE_IPV4_SINGLECAST;
    return MCS_RET_SUCCESS;

}
WORD32 psVpfuMcsIPv6RouteGetEnd(PS_PACKET *ptPacket, T_psMcsRtInfo *ptRtOut, WORD32 dwIndex)
{
    if(NULL == ptRtOut||NULL == ptPacket)
    {
        return MCS_RET_FAIL;
    }
    /* 获取转发接口信息 */
    WORD32                      dwPortMtu  = 0;/* interface 的mtu */
    WORD32                      dwRouteMtu = 0; /* 最终的路由mtu，结合gre外部隧道计算 */

    /* 获取转发接口信息 */
    T_psFtmPortInfo  *ptFtmPortInfo = psFtmGetPortInfo(dwIndex);
    if(unlikely(NULL == ptFtmPortInfo))
    {
        showV6Route(0,0,NULL);
        DEBUG_TRACE(DEBUG_ERR,"dwIndex = %u\n",dwIndex);
        return MCS_RET_FAIL;
    }
    
    /* dwRouteMtu先初始化为 dwPortMtu*/
    dwPortMtu =ptFtmPortInfo->dwV6Mtu;/* ipv6 mtu */
    dwRouteMtu = dwPortMtu;
    /* 计算内层IP的MTU值，上行报文转发不考虑业务层的MTU */
    if(unlikely(dwRouteMtu <= ptRtOut->wTunnelEncapLen))
    {
        DEBUG_TRACE(DEBUG_LOW, "dwRouteMtu=%u,ptRtOut->wTunnelEncapLen=%u\n",dwRouteMtu, ptRtOut->wTunnelEncapLen);
        return MCS_RET_FAIL;
    }
    /*这个是算出来的接口mtuֵ*/
    ptPacket->wMtu = (WORD16)(dwRouteMtu - ptRtOut->wTunnelEncapLen);

    /* 填充报文转发需要的其他字段 */
    // DDD差异Q4:[www]赋值对齐IPv4,相互独立,应OK
    ptPacket->bInOutFlag = CSS_FWD_TO_PFU_SENDOUT;
    ptPacket->dwOutPort = ptFtmPortInfo->dwGport;
    /* 获取vlanID */
    WORD16 wVlanId = psCssGetVlanIdByGPort(ptPacket->dwOutPort);
    ptPacket->wVlanId  = wVlanId;
    DEBUG_TRACE(DEBUG_LOW,"vlan=%u", ptPacket->wVlanId);
    psCssGetPhysInfoByOutPort(ptPacket); //获取ptPacket->InterfacePortNo等

    /* 根据出port判断是否是本板 */
    /*标记获取出端口标志位，CSS调用----只有本板的时候才能这样获取,非本板css会替换bOutPortNo */
    ptPacket->btGetOutPortFlag = ptPacket->bPfuMacCmpFlag;
    if(ptPacket->bPfuMacCmpFlag)
    {
        //GportExchange_modify
        ptPacket->bOutPortNo = dpa_select_port_by_porttype_lb((BYTE)ptPacket->InterfacePortNo, (BYTE)ptRtOut->tLdBlceFactor.dwLBFactor_LDSH);
    }
    /*611002954997 [IPV6测试 - 端口ACL]xGW_V4.12.20.B5 端口关联IPV6 ACL规则，ACL规则按照IP目的地址过滤deny 出向报文，xGW端口将报文流量转发出去*/
    ptPacket->bPFUHeaderType = 7;//ptPacket->bPFUHeaderType = CSS_FWD_PFUTYPE_IPV6_SINGLEINDIR;

    return MCS_RET_SUCCESS;
}

WORD16 psNcuGetMtu(T_IPV6 tDstIP, BYTE iptype, WORD16 threadID)
{
    DEBUG_TRACE(DEBUG_LOW, "psNcuGetMtu\n");
    T_psNcuMcsPerform *ptNcuPerform = psVpfuMcsGetPerformPtr(threadID);
    BYTE buffer[16] = {0};
    WORD16 wMtu = 0;
    WORD32 ret = MCS_RET_FAIL;
    PS_PACKET *ptPacket  = psCSSGenPkt(buffer, 10, threadID);
    WORD16 wIpUdpLen = 0;
    if(NULL == ptPacket)
    {
        MCS_LOC_STAT_EX(ptNcuPerform, qwPktAllocFailForMtu, 1);
        return 0;
    }
    if(IPv4_VERSION == iptype)
    {
        T_IPV4 tIPv4 = {0};
        IPV4_COPY(tIPv4, tDstIP);
        ret = psNcuUlIPV4GiRouteGet(ptPacket,tIPv4);
        wIpUdpLen = MCS_IPV4_HEAD_LEN + MCS_UDP_HEAD_LEN;
    }
    else if(IPv6_VERSION == iptype)
    {
        ret = psNcuUlIPV6GiRouteGet(ptPacket, tDstIP);
        wIpUdpLen = MCS_IPV6_HEAD_LEN + MCS_UDP_HEAD_LEN;
    }
    else
    {

    }
    if(MCS_RET_SUCCESS == ret)
    {
        wMtu = ptPacket->wMtu;
        MCS_LOC_STAT_EX(ptNcuPerform, qwGetMtuSuccByRoute, 1);
    }
    else
    {
        MCS_LOC_STAT_EX(ptNcuPerform, qwGetMtuFailByRoute, 1);
    }
    psCSSPktLinkBuffDesFree(ptPacket);
    return wMtu > wIpUdpLen ? wMtu - wIpUdpLen : 0;
}

WORD32 psNcuUlIPV4GiRouteGet(PS_PACKET* pkt, T_IPV4 ptNwdafIP)
{
    if(NULL == pkt)
    {
        return MCS_RET_FAIL;
    }
    T_psNcuMcsPerform *ptNcuPerform = psVpfuMcsGetPerformPtr(pkt->bSthr);
    if(NULL == ptNcuPerform)
    {
        return MCS_RET_FAIL;
    }
    T_psMcsRtInfo  tRtInfo = {0}; 
    T_psMcsRtInfo  *ptRtInfo = &tRtInfo; 
    pkt->wVpnId = psGetVrfV4(); //需要查vrf
    if(0 == pkt->wVpnId)
    {
        MCS_LOC_STAT_EX(ptNcuPerform, qwRouteVrfIsZero, 1);
    }
    WORD32 dwIndex = 0x0;/*出接口的索引*/

    WORD32 dwDstIP = MCS_IP4_ARR_TO_DWORD(ptNwdafIP);
    pkt->wMtu = g_set_mtu;
    #ifndef FT_TEST
    T_psFtmV4RtmInfo  *ptRtmInfo = psFtmV4VrfLoopup(pkt->bSthr, (WORD32)pkt->wVpnId, dwDstIP);
    DEBUG_TRACE(DEBUG_LOW,"psFtmV4VrfLoopup:sthr=%d, vpnid=%d, dstip=%u.%u.%u.%u\n", pkt->bSthr, pkt->wVpnId, ptNwdafIP[0], ptNwdafIP[1], ptNwdafIP[2], ptNwdafIP[3]);
    if (unlikely(NULL == ptRtmInfo))
    {
        return MCS_RET_FAIL;
    }
    DEBUG_TRACE(DEBUG_LOW," \n[Mcs]After psFtmV4VrfLoopup, func:%s line:%d"
            "\n     dwFlags       = %u(1:normal, 2:default, 4:direct, 8:LDSH, 16:FRR, 32:smartgroup, 64:tunnel)"
            "\n     dwOutPortIndex= %u "
            "\n     dwNextJmpIp   = 0x%08x "
            "\n     dwLdshIndex   = 0x%08x "
            "\n     dwFrrIndex    = 0x%08x "
            "\n     wSmartGroupID = 0x%04x ",
            __FUNCTION__,__LINE__,
            ptRtmInfo->dwFlags,                     /*路由下驱动标记*/
            ptRtmInfo->dwOutPortIndex,              /*出接口的索引*/
            ptRtmInfo->dwNextJmpIp,                 /*路由实际的下一跳，GRE隧道生效时，为隧道索引*/
            ptRtmInfo->dwLdshIndex,                 /*负荷分担索引*/
            ptRtmInfo->dwFrrIndex,                  /*FRR索引*/
            ptRtmInfo->wSmartGroupID
            );
    switch(ptRtmInfo->dwFlags)
    {
        case FTM_VRF_ZERO_FLAG: /* 普通路由 */
        case FTM_VRF_DEFAULT_FLAG: /* 默认路由 */
        {
            dwIndex = ptRtmInfo->dwOutPortIndex;
            pkt->dwNextHop = ptRtmInfo->dwNextJmpIp;
            break;
        }
        case  FTM_VRF_TETUL_FLAG:
        {
            T_psFtmTeTunnelInfo *ptFtmTeTunnelInfo =  psFtmGetTeTunnelById(ptRtmInfo->dwTeTunnelId);

            if(NULL == ptFtmTeTunnelInfo)
            {
               return MCS_RET_FAIL;
            }
            dwIndex = ptFtmTeTunnelInfo->dwOutPortIndex;
            pkt->dwNextHop = ptFtmTeTunnelInfo->dwNextJmpIp;
            break;
        }
        case FTM_VRF_TEFRR_FLAG:
        {
            T_psFtmTeFec *ptFtmTeFec =  psFtmGetTeFrrInfo(ptRtmInfo->dwFrrIndex);
            if(NULL == ptFtmTeFec)
            {
                return MCS_RET_FAIL;
            }
            dwIndex = ptFtmTeFec->dwOutPortIndex;
            pkt->dwNextHop = ptFtmTeFec->dwNextJmpIp;
            break;
        }
         case FTM_VRF_DIRECT_FLAG:
        {
            /* 直连路由，直接使用报文的目的地址作为下一跳 */
            dwIndex = ptRtmInfo->dwOutPortIndex;
            pkt->dwNextHop = dwDstIP;
            break;
        }
        case FTM_VRF_LOCALPKT_FLAG:
        {
            return MCS_RET_FAIL;
        }
        case FTM_VRF_LDSH_FLAG:
        {
            T_psFtmLdshHeadInfo *ptLashInfo = NULL;
            ptLashInfo = psFtmGetLdshInfo(ptRtmInfo->dwLdshIndex);
            if(NULL == ptLashInfo)
            {
                return MCS_RET_FAIL;
            }
            dwIndex = ptLashInfo->dwItemSum;
            if(0 ==  ptLashInfo->dwItemSum)
            {
                return MCS_RET_FAIL;
            }
            dwIndex = psNcuGetItemIndex(ptLashInfo);
            if(dwIndex >= FTM_LDSH_ITEM_NUM)
            {
                return MCS_RET_FAIL;
            }
           DEBUG_TRACE(DEBUG_LOW,"ptLashInfo->Item %d = index\n", dwIndex);
            T_psFtmLdFrrInfo    *ptLdFrrInfo = &ptLashInfo->Item[dwIndex];
            volatile BYTE bFlag = ptLdFrrInfo->wFlag;
            switch (bFlag)
            {
                case PS_FTM_PORT:
                case PS_FTM_SMARTGROUP:
                case PS_FTM_IPSEC_TUNNEL:
                    dwIndex = ptLdFrrInfo->dwOutPortIndex;
                    pkt->dwNextHop = ptLdFrrInfo->dwNextHopIp;
                    DEBUG_TRACE(DEBUG_LOW,"ptLdFrrInfo->dwOutPortIndex %d = index\n", dwIndex);
                    break;
                case PS_FTM_GRETUNNEL:
                     return MCS_RET_FAIL;
                default:
                     return MCS_RET_FAIL;
            }
            break;
        }
        case FTM_VRF_FRR_FLAG:
        {
            T_psFtmFrrHeadInfo *ptFrrHeadInfo   = psFtmGetFrrInfo(ptRtmInfo->dwFrrIndex);
            if(NULL == ptFrrHeadInfo)
            {
                return MCS_RET_FAIL;
            }
            if(ptFrrHeadInfo->dwMsFlag >= FTM_FRR_MAX_ITEM_NUM)
            {
                return MCS_RET_FAIL;
            }
            T_psFtmLdFrrInfo    *ptLdFrrInfo = &ptFrrHeadInfo->Item[ptFrrHeadInfo->dwMsFlag];
            if(0xFFFFFFFF != ptLdFrrInfo->dwLdFrrIndex)
            {
                 if(2 == ptLdFrrInfo->dwLdFrrFlags)
                {
                    T_psFtmFrrHeadInfo *ptFrrHeadInfoSecond   = psFtmGetFrrInfo(ptLdFrrInfo->dwLdFrrIndex);
                    if(NULL != ptFrrHeadInfoSecond)
                    {
                        T_psFtmLdFrrInfo *ptTmpLevel2Frr = &(ptFrrHeadInfoSecond->Item[0]);
                        dwIndex = ptTmpLevel2Frr->dwOutPortIndex;
                        pkt->dwNextHop = ptTmpLevel2Frr->dwNextHopIp;
                        break;
                    }
                }
                if(4 == ptLdFrrInfo->dwLdFrrFlags)
                {
                    T_psFtmTeFec *ptTmpFtmTeFec   = psFtmGetTeFrrInfo(ptLdFrrInfo->dwLdFrrIndex);
                    if(NULL != ptTmpFtmTeFec)
                    {
                        dwIndex = ptTmpFtmTeFec->dwOutPortIndex;
                        pkt->dwNextHop = ptTmpFtmTeFec->dwNextJmpIp;
                        break;
                    }
                }
            }
            volatile BYTE bFlag = ptLdFrrInfo->wFlag;
            switch (bFlag)
            {
                case PS_FTM_PORT:
                case PS_FTM_SMARTGROUP:
                case PS_FTM_IPSEC_TUNNEL:
                    dwIndex = ptLdFrrInfo->dwOutPortIndex;
                    pkt->dwNextHop = ptLdFrrInfo->dwNextHopIp;
                    break;
                case PS_FTM_GRETUNNEL:
                     return MCS_RET_FAIL;
                default:
                     return MCS_RET_FAIL;
            }
            break;
        }
        case FTM_VRF_TUNNEL_FLAG:
            return MCS_RET_FAIL;
        case FTM_L3VPN_IPSEC_FLAG:
            dwIndex = ptRtmInfo->dwOutPortIndex;
            pkt->dwNextHop = ptRtmInfo->dwNextJmpIp;
            break;
        default:
            return MCS_RET_FAIL;
    }

    if(MCS_RET_SUCCESS != psNcuMcsIPv4RouteGetEnd(pkt,ptRtInfo,dwIndex))
    {
        return MCS_RET_FAIL;
    }
     pkt->bPfuMacCmpFlag = 1;
    
    if(CSS_OK == psCssGetIpv4SendOutMacAddr(pkt))
    {
        DEBUG_TRACE(DEBUG_LOW,"\n[Mcs]psVpfuMcsGetIPv4MacAddr Succeed !");
        
        return SUCCESS;
    }
    else
    {
        DEBUG_TRACE(DEBUG_LOW,"\n[Mcs]psVpfuMcsGetIPv4MacAddr failed !");
        return MCS_RET_FAIL;
    }
    #endif
    return MCS_RET_SUCCESS;
}
WORD32 psNcuUlIPV6GiRouteGet(PS_PACKET* pkt, T_IPV6 ptNwdafIP)
{
    
    if(NULL == pkt)
    {
        return MCS_RET_FAIL;
    }
    T_psNcuMcsPerform *ptNcuPerform = psVpfuMcsGetPerformPtr(pkt->bSthr);
    if(NULL == ptNcuPerform)
    {
        return MCS_RET_FAIL;
    }
    T_psMcsRtInfo  tRtInfo = {0}; 
    T_psMcsRtInfo  *ptRtInfo = &tRtInfo; 
    pkt->wVpnId = psGetVrfV6(); //需要查vrf
    if(0 == pkt->wVpnId)
    {
        MCS_LOC_STAT_EX(ptNcuPerform, qwRouteVrfIsZero, 1);
    }
    WORD32 dwIndex = 0x0;/*出接口的索引*/
    pkt->wMtu = g_set_mtu;
    #ifndef FT_TEST
     DEBUG_TRACE(DEBUG_LOW, "vpnid=%u nwdafip = %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x \n",
        pkt->wVpnId, ptNwdafIP[0],ptNwdafIP[1],ptNwdafIP[2],ptNwdafIP[3],ptNwdafIP[4],ptNwdafIP[5],ptNwdafIP[6],ptNwdafIP[7],ptNwdafIP[8],
         ptNwdafIP[9],ptNwdafIP[10],ptNwdafIP[11],ptNwdafIP[12],ptNwdafIP[13],ptNwdafIP[14],ptNwdafIP[15]);
    T_psFtmV6RtmInfo   *ptV6RtmInfo = psFtmV6VrfLookup(pkt->bSthr,
                                                       (WORD32)pkt->wVpnId,
                                                       ptNwdafIP);
    if (unlikely(NULL == ptV6RtmInfo))
    {
         DEBUG_TRACE(DEBUG_LOW, "ptV6RtmInfo=NULL\n");
        return MCS_RET_FAIL;
    }
    DEBUG_TRACE(DEBUG_LOW, " psFtmV6VrfLookup \ndwFlags       = %u(1:normal, 2:default, 4:direct, 8:LDSH, 16:FRR, 32:smartgroup, 64:tunnel)"
            "\n     dwOutPortIndex= %u "
            "\n     dwL2Index   = %u "
            "\n     dwLdshIndex   = 0x%08x ",
            ptV6RtmInfo->dwFlags,                     /*路由下驱动标记*/
            ptV6RtmInfo->dwOutPortIndex,              /*出接口的索引*/
            ptV6RtmInfo->dwL2Index, 
           ptV6RtmInfo->dwLdshIndex                 /*负荷分担索引*/
            );
    switch(ptV6RtmInfo->dwFlags)
    {
        case FTM_VRF_ZERO_FLAG: /* 普通路由 */
        case FTM_VRF_DEFAULT_FLAG: /* 默认路由 */
        {
            DEBUG_TRACE(DEBUG_LOW, "FTM_VRF_ZERO_FLAG/FTM_VRF_DEFAULT_FLAG,%u\n", ptV6RtmInfo->dwFlags);
            dwIndex = ptV6RtmInfo->dwOutPortIndex;
            pkt->dwNextHop = ptV6RtmInfo->dwL2Index;
            zte_memcpy_s(pkt->abNextJmpIp, PS_NCU_IPV6_GI_ROUTE_JMP_IP_LEN, ptV6RtmInfo->abNextJmpIp, PS_NCU_IPV6_GI_ROUTE_JMP_IP_LEN);
            break;
        }
        case FTM_VRF_DIRECT_FLAG:
        {
            /* 直连路由，直接使用报文的目的地址作为下一跳 */
            DEBUG_TRACE(DEBUG_LOW, "FTM_VRF_DIRECT_FLAG\n");
            pkt->bPFUHeaderType = 7;
            dwIndex = ptV6RtmInfo->dwOutPortIndex;
            pkt->dwNextHop = 0;
            zte_memset_s(pkt->abNextJmpIp, PS_NCU_IPV6_GI_ROUTE_JMP_IP_LEN, 0, PS_NCU_IPV6_GI_ROUTE_JMP_IP_LEN);
            break;
        }
        case  FTM_VRF_TUNNEL_FLAG:
        {   //暂不支持
            DEBUG_TRACE(DEBUG_LOW, "FTM_VRF_TUNNEL_FLAG\n");
            return MCS_RET_FAIL;
            /* GRE隧道路由 */
            /* 获取IPSec端口信息 */

        }
        case FTM_VRF_FRR_FLAG:
        {
             DEBUG_TRACE(DEBUG_LOW, "FTM_VRF_FRR_FLAG\n");
            return MCS_RET_FAIL;
        }
        case FTM_VRF_LDSH_FLAG:
        {
             DEBUG_TRACE(DEBUG_LOW, "FTM_VRF_LDSH_FLAG\n");
            T_psFtmV6LdFrrInfo  *ptV6LdFrrInfo = NULL;
            T_psFtmV6LdshHeadInfo *ptV6LashInfo = psFtmV6GetLdshInfo(ptV6RtmInfo->dwLdshIndex);
            if(NULL == ptV6LashInfo)
            {
                return MCS_RET_FAIL;
            }
            WORD32 dwItemSum = 0;
            dwItemSum = psNcuGetV6ItemSum(ptV6LashInfo);            
            if (dwItemSum >= FTM_LDSH6_ITEM_NUM)
            {
                DEBUG_TRACE(DEBUG_LOW, "dwItemSum>=FTM_LDSH6_ITEM_NUM\n");
                return MCS_RET_FAIL;
            }
            else
            {
                dwIndex = dwItemSum;
            }
            DEBUG_TRACE(DEBUG_ERR,"dwIndex = %u\n",dwIndex);
            ptV6LdFrrInfo = &(ptV6LashInfo->Item[dwIndex]);
            volatile BYTE bFlag = ptV6LdFrrInfo->wFlag;
             DEBUG_TRACE(DEBUG_LOW, "ptV6LdFrrInfo->wFlag =%u\n",bFlag);
            switch (bFlag)
            {
                case PS_FTM_PORT:
                case PS_FTM_SMARTGROUP:
                case PS_FTM_IPSEC_TUNNEL:
                {
                    DEBUG_TRACE(DEBUG_LOW, "ptV6LdFrrInfo->wFlag normal\n");
                    /* 更新下一跳接口和地址 */
                    dwIndex = ptV6LdFrrInfo->dwOutPortIndex;
                    pkt->dwNextHop = ptV6LdFrrInfo->dwNdIndex;
                    zte_memcpy_s(pkt->abNextJmpIp, PS_NCU_IPV6_GI_ROUTE_JMP_IP_LEN, ptV6LdFrrInfo->abNextNextHopIp, PS_NCU_IPV6_GI_ROUTE_JMP_IP_LEN);
                    break;
                }

                case PS_FTM_GRETUNNEL:/*LDSH+GRE*/
                {
                    DEBUG_TRACE(DEBUG_LOW, "ptV6LdFrrInfo->wFlag PS_FTM_GRETUNNEL\n");
                    return MCS_RET_FAIL;
                    /* GRE隧道路由 */
                }
                case PS_FTM_V6TUNNEL:
                {/*看承载的代码暂时也是这样处理的*/
                DEBUG_TRACE(DEBUG_LOW, "ptV6LdFrrInfo->wFlag PS_FTM_V6TUNNEL\n");
                    return MCS_RET_FAIL;
                    /* 611003118393更新下一跳接口和地址 */
                    //         __FUNCTION__,__LINE__,


                    // //6in4隧道负荷分担流程，wxd10241275，20181120

                }
                default:
                {
                    DEBUG_TRACE(DEBUG_LOW, "ptV6LdFrrInfo->wFlag other\n");
                    return MCS_RET_FAIL;
                }
            }
            break;
        }
        case FTM_L3VPN_IPSEC_FLAG:
            dwIndex = ptV6RtmInfo->dwOutPortIndex;
            pkt->dwNextHop = ptV6RtmInfo->dwL2Index;
            zte_memcpy_s(pkt->abNextJmpIp, PS_NCU_IPV6_GI_ROUTE_JMP_IP_LEN, ptV6RtmInfo->abNextJmpIp, PS_NCU_IPV6_GI_ROUTE_JMP_IP_LEN);
            break;
        default:
            return MCS_RET_FAIL;
    }

    if(MCS_RET_SUCCESS != psVpfuMcsIPv6RouteGetEnd(pkt,ptRtInfo,dwIndex))
    {
        DEBUG_TRACE(DEBUG_LOW, "psVpfuMcsIPv6RouteGetEnd fail\n");
        return MCS_RET_FAIL;
    }
     pkt->bPfuMacCmpFlag = 1;
    
    if(CSS_OK == psCssGetIpv6SendOutMacAddr(pkt,ptNwdafIP))
    {
        DEBUG_TRACE(DEBUG_LOW,"\n[Mcs]psVpfuMcsGetIPv6MacAddr Succeed\n");
        
        return SUCCESS;
    }
    else
    {
        DEBUG_TRACE(DEBUG_LOW,"\n[Mcs]psVpfuMcsGetIPv6MacAddr failed\n");
        return MCS_RET_FAIL;
    }
    #endif
    return MCS_RET_SUCCESS;
}

WORD32 psNcuGetItemIndex(T_psFtmLdshHeadInfo *ptLashInfo)
{
    if(ptLashInfo == NULL)
    {
        return FTM_LDSH_ITEM_NUM;
    }
    WORD32 dwIndex = 0;
    BYTE bLocalItemNum = 0;
    bLocalItemNum = ptLashInfo->bLocalItemNum;
    if(0 != bLocalItemNum)
    {
        dwIndex = ptLashInfo->abLocalWeight[15 % FTM_LDSH_ITEM_NUM];
        dwIndex = dwIndex%bLocalItemNum;
        dwIndex = ptLashInfo->abLocalItemIndex[dwIndex];
    }

    return dwIndex;
}

WORD32 psNcuGetV6ItemSum(T_psFtmV6LdshHeadInfo *ptV6LashInfo)
{
    if(ptV6LashInfo == NULL)
    {
        return FTM_LDSH6_ITEM_NUM;
    }

    WORD32  dwRtShareIndex = 100 & 0xffff;
    WORD32 dwItemSum = ptV6LashInfo->dwItemSum;
    if(dwItemSum == 0)
    {
        DEBUG_TRACE(DEBUG_LOW, "dwItemSum=0\n");
        return FTM_LDSH6_ITEM_NUM;
    }
    BYTE bLocalItemNum = 0;
    bLocalItemNum = ptV6LashInfo->bLocalItemNum;
    if(0 != bLocalItemNum)
    {
        /* 使用本板的负荷分担信息 */
        dwItemSum = ptV6LashInfo->abLocalWeight[dwRtShareIndex % 100];
        /* 611003620291:负荷分担都考虑权重，且对权重取值进行保护 */

        dwItemSum = dwItemSum%bLocalItemNum;
        if(FTM_LDSH6_ITEM_NUM <= dwItemSum)/* 数组越界保护 */
        {
            return FTM_LDSH6_ITEM_NUM;
        }
        dwItemSum = ptV6LashInfo->abLocalItemIndex[dwItemSum];
    }
    else
    {
        /* 611003602405:取权重 */
        WORD32 dwWeightIndex = 10 & 0xffff;
        BYTE bWeight = ptV6LashInfo->abWeight[dwWeightIndex % 100];
        dwItemSum = bWeight % dwItemSum;
    }

    return dwItemSum;
}
