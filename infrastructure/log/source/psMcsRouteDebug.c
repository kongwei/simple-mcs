#include "psMcsRouteDebug.h"
#include "ps_ftm_pub.h"
#include "tulip.h"
#include "zte_slibc.h"
#include "McsIPv4Head.h"
#include <arpa/inet.h>
extern void show_if_info(WORD32 port);
void showLdshv6(WORD32 ldshindex);
void showLdsh(WORD32 ldshIndex);
void showldportbyindex(WORD32 dwIndex);
void showV6Route(BYTE thread, WORD32 vpnid, CHAR* dstIP)
{
    if(NULL == dstIP)
    {
        return;
    }
    T_IPV6 ipv6={0};
    inet_pton(AF_INET6, dstIP, (void *)&ipv6);
    T_psFtmV6RtmInfo   *ptV6RtmInfo = psFtmV6VrfLookup(thread,vpnid, ipv6);
    if(NULL == ptV6RtmInfo)
    {
        zte_printf_s("\npsFtmV6VrfLookup=NULL\n");
        return;
    }
    zte_printf_s("\ndwFlags        = %u(1:normal, 2:default, 4:direct, 8:LDSH, 16:FRR, 32:smartgroup, 64:tunnel)",ptV6RtmInfo->dwFlags);
    zte_printf_s("\ndwOutPortIndex  = %u ", ptV6RtmInfo->dwOutPortIndex);
    zte_printf_s("\ndwL2Index       = %u ",ptV6RtmInfo->dwL2Index);
    zte_printf_s("\ndwLdshIndex     = 0x%08x ",ptV6RtmInfo->dwLdshIndex);
    if(ptV6RtmInfo->dwFlags == FTM_VRF_LDSH_FLAG)
    {
        showLdshv6(ptV6RtmInfo->dwLdshIndex);
    }
    else
    {
        showldportbyindex(ptV6RtmInfo->dwOutPortIndex);
    }
}
void showV4Route(BYTE thread, WORD32 vpnid, CHAR* dstIP)
{
    if(NULL == dstIP)
    {
        return;
    }
    T_IPV4 ipv4 = {0};
    inet_pton(AF_INET, dstIP, (void *)&ipv4);
    WORD32 dwDstIP = MCS_IP4_ARR_TO_DWORD(ipv4);
    T_psFtmV4RtmInfo  *ptRtmInfo = psFtmV4VrfLoopup(thread,vpnid, dwDstIP);
    if(NULL == ptRtmInfo)
    {
        zte_printf_s("\npsFtmV4VrfLoopup = NULL\n");
        return;
    }
    T_IPV4 v4 = {0};
    MCS_DWORD_TO_IP4_ARR(v4, ptRtmInfo->dwNextJmpIp);
    zte_printf_s(" \n    dwFlags       = %u(1:normal, 2:default, 4:direct, 8:LDSH, 16:FRR, 32:smartgroup, 64:tunnel)",ptRtmInfo->dwFlags);
    zte_printf_s("\n     dwOutPortIndex= %u ",ptRtmInfo->dwOutPortIndex);
    zte_printf_s("\n     dwNextJmpIp   = 0x%08x(%u,%u,%u,%u) ",ptRtmInfo->dwNextJmpIp,v4[0],v4[1],v4[2],v4[3]);
    zte_printf_s("\n     dwLdshIndex   = 0x%08x ",ptRtmInfo->dwLdshIndex);
    zte_printf_s("\n     dwFrrIndex    = 0x%08x ",ptRtmInfo->dwFrrIndex);
    zte_printf_s("\n     wSmartGroupID = 0x%04x ",ptRtmInfo->wSmartGroupID);
    if(ptRtmInfo->dwFlags == FTM_VRF_LDSH_FLAG)
    {
        showLdsh(ptRtmInfo->dwLdshIndex);
    }
    else
    {
        showldportbyindex(ptRtmInfo->dwOutPortIndex);
    }
    
}
void showLdshv6(WORD32 ldshindex)
{
    T_psFtmV6LdshHeadInfo *ptV6LashInfo = psFtmV6GetLdshInfo(ldshindex);
    if(NULL == ptV6LashInfo)
    {
        zte_printf_s("psFtmV6GetLdshInfo = NULL\n");
        return;
    }
    zte_printf_s("\n     dwItemSum      = %u",ptV6LashInfo->dwItemSum);
    zte_printf_s("\n     bLocalItemNum  = %u",ptV6LashInfo->bLocalItemNum);

    WORD32 i = 0;
    for (i = 0; i < CSS_FTM_LDSH_ITEM_NUM; i++)
    {
        T_psFtmV6LdFrrInfo    *ptLdFrrInfo = &ptV6LashInfo->Item[i];
        if(0 == ptLdFrrInfo->dwOutPortIndex)
        {
            continue;
        }
        zte_printf_s("\n abLocalItemIndex= %u dwOutPortIndex=%u,dwNdIndex=%u,wFlag=%u\n",
                ptV6LashInfo->abLocalItemIndex[i],ptLdFrrInfo->dwOutPortIndex,
                ptLdFrrInfo->dwNdIndex,ptLdFrrInfo->wFlag);
        showldportbyindex(ptLdFrrInfo->dwOutPortIndex);
    }
}
void showLdsh(WORD32 ldshIndex)
{
    T_psFtmLdshHeadInfo *ptLashInfo = psFtmGetLdshInfo(ldshIndex);
    if(NULL == ptLashInfo)
    {
        zte_printf_s("psFtmGetLdshInfo = NULL\n");
        return;
    }
    zte_printf_s("\n     dwItemSum      = %u",ptLashInfo->dwItemSum);
    zte_printf_s("\n     bLocalItemNum  = %u",ptLashInfo->dwOldItemSum);
    zte_printf_s("\n     dwItemSum      = %u",ptLashInfo->bLocalItemNum);

    WORD32 i = 0;
    for (i = 0; i < CSS_FTM_LDSH_ITEM_NUM; i++)
    {
        T_psFtmLdFrrInfo    *ptLdFrrInfo = &ptLashInfo->Item[i];
        if(0 == ptLdFrrInfo->dwOutPortIndex)
        {
            continue;
        }
        zte_printf_s("\n   abLocalItemIndex= %u,dwLdshPolicy=%u\n",ptLashInfo->abLocalItemIndex[i], ptLashInfo->dwLdshPolicy[i]);
        T_IPV4 v4 = {0};
        MCS_DWORD_TO_IP4_ARR(v4, ptLdFrrInfo->dwNextHopIp);
        zte_printf_s("\n  dwOutPortIndex=%u,dwNextHopIp=%u(%u.%u.%u.%u), wSmartGroupID=%u,wFlag=%u\n",
                ptLdFrrInfo->dwOutPortIndex,
                ptLdFrrInfo->dwNextHopIp,v4[0],v4[1],v4[2],v4[3],
                ptLdFrrInfo->wSmartGroupID,
                ptLdFrrInfo->wFlag
                );
        showldportbyindex(ptLdFrrInfo->dwOutPortIndex);
    }
}
void showldportbyindex(WORD32 dwIndex)
{
    T_psFtmPortInfo             *ptFtmPortInfo = psFtmGetPortInfo(dwIndex);
    if(NULL == ptFtmPortInfo)
    {
        zte_printf_s("\n  ptFtmPortInfo = NULL");
        return;
    }
    zte_printf_s(" \n     ptFtmPortInfo success  "
            "dwIndex    = %u     "
            "dwStatus   = 0x%08x "
            "dwFapFlowid= %u     "
            "dwVpnId    = %u     "
            "dwMtu      = %u     "
            "dwV6Mtu    = %u     "
            "dwGport    = 0x%08x "
            "wAclIndex  = %u     "
            "bIpsecFlag = %u     "
            "bShelf     = %u     "
            "bSlot      = %u     \n",
            dwIndex,
            ptFtmPortInfo->dwStatus,    /* 端口状态*/
            ptFtmPortInfo->dwFapFlowid, /* FLOW ID*/
            ptFtmPortInfo->dwVpnId,     /* VPN ID */
            ptFtmPortInfo->dwMtu,       /* IPv4 MTUֵ*/
            ptFtmPortInfo->dwV6Mtu,     /* IPv6 MTUֵ*/
            ptFtmPortInfo->dwGport,     /*32位全局端口号*/
            ptFtmPortInfo->wAclIndex,
            ptFtmPortInfo->bIpsecFlag,
            ptFtmPortInfo->bShelf,
            ptFtmPortInfo->bSlot);
    show_if_info(ptFtmPortInfo->dwGport);
}
