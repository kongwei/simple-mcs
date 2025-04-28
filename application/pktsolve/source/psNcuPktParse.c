/* Started by AICoder, pid:zf3aagebcfn2015143140a7cf07021405109b1db */
/******************************************************************************
 * 版权所有 (C)2016 深圳市中兴通讯股份有限公司*
 * 模块名          : NCU
 * 文件名          : psNcuPktParse.c
 * 相关文件        :
 * 文件实现功能     : 用户报文解析
 * 归属团队        : M6
-------------------------------------------------------------------------------
* 修改记录:
* 修改日期            版本号           修改人           修改内容
* 2024-08-12        V7.24.30          wya             create
******************************************************************************/
/**************************************************************************
 *                              头文件(满足最小依赖请按照DDD分层架构逐层依赖)
 **************************************************************************/
#include "psNcuPktParse.h"
#include "tulip.h"
#include "ps_mcs_define.h"
#include "McsProtoType.h"
#include "McsIPv4Head.h"
#include "McsIPv6Head.h"
#include "McsUDPHead.h"
#include "McsTCPHead.h"
#include "McsICMPHead.h"
#include "McsICMPv6Head.h"
#include "zte_slibc.h"
#include "UpfNcuSynInfo.h"
#include "psMcsDebug.h"
#include "MemShareCfg.h"
/**************************************************************************
 *                              宏(本源文件使用)
 **************************************************************************/

/**************************************************************************
 *                              常量(本源文件使用)
 **************************************************************************/

/**************************************************************************
 *                              数据类型(本源文件使用)
 **************************************************************************/

/**************************************************************************
 *                              外部函数原型(评估后慎重添加)
 **************************************************************************/

/**************************************************************************
 *                              局部函数原型(本源文件使用)
 **************************************************************************/
VOID psNcuFillTcpInfo(T_TcpBaseHeader *ptTcpHead, WORD16 wIpHeadLen, T_psNcuPktDesc* ptNcuPktDesc);
WORD32 psNcuParsePktIpv6Info(T_psNcuMcsPerform *ptMcsNcuPerform, T_psNcuPktDesc* ptNcuPktDesc, BYTE* PktInnerData);
WORD32 psNcuParsePktIpv4Info(T_psNcuMcsPerform *ptMcsNcuPerform, T_psNcuPktDesc* ptNcuPktDesc, BYTE* PktInnerData);
WORD32 psNcuParsePktTransLayerInfo(BYTE* PktInnerData, T_psNcuPktDesc* ptNcuPktDesc, WORD16 wIpHeadLen);

/**************************************************************************
 *                              全局变量(本源文件使用)
 **************************************************************************/

/**************************************************************************
 *                              函数实现(函数布局:总分结构|深度优先)
 **************************************************************************/
WORD32 psNcuParsePktInfo(T_MediaProcThreadPara* ptMediaProcThreadPara, VOID *ptPktData)
{
    if (unlikely(NULL == ptMediaProcThreadPara || NULL == ptMediaProcThreadPara->ptMcsStatPointer || NULL == ptPktData))
    {
        return MCS_RET_FAIL;
    }
    T_psNcuMcsPerform *ptMcsNcuPerform = (T_psNcuMcsPerform*)ptMediaProcThreadPara->ptMcsStatPointer;
    T_psNcuPktDesc   *ptPktDesc = &ptMediaProcThreadPara->tPktDesc;
    T_NcuPktInfoHead *ptPktHead = (T_NcuPktInfoHead*)ptPktData;
    BYTE        *ptPktInnerData = (BYTE*)(ptPktHead+1);

    ptPktDesc->bIpType = ptPktHead->bIpType;
    ptPktDesc->bDir    = ptPktHead->bDir;
    ptPktDesc->wPktLen = ptPktHead->wL34Length;

    if(IPv4_VERSION == ptPktHead->bIpType)
    {
        MCS_LOC_STAT_EX(ptMcsNcuPerform, qwRcvCopyPktIpv4, 1);
        NCU_PM_55304_STAT_ADD(qwNcuRcvCpyPktMsgs, 1);
        return psNcuParsePktIpv4Info(ptMcsNcuPerform, &ptMediaProcThreadPara->tPktDesc, ptPktInnerData);
    }

    if(IPv6_VERSION == ptPktHead->bIpType)
    {
        MCS_LOC_STAT_EX(ptMcsNcuPerform, qwRcvCopyPktIpv6, 1);
        NCU_PM_55304_STAT_ADD(qwNcuRcvCpyPktMsgs, 1);
        return psNcuParsePktIpv6Info(ptMcsNcuPerform, &ptMediaProcThreadPara->tPktDesc, ptPktInnerData);
    }

    DEBUG_TRACE(DEBUG_ERR,"IP Type Err!\n");
    MCS_LOC_STAT_EX(ptMcsNcuPerform, qwRcvCopyPktIpTypeErr, 1);
    return MCS_RET_FAIL;
}

WORD32 psNcuParsePktIpv4Info(T_psNcuMcsPerform *ptMcsNcuPerform, T_psNcuPktDesc* ptNcuPktDesc, BYTE* PktInnerData)
{
    if(unlikely(NULL == ptMcsNcuPerform || NULL == PktInnerData || NULL == ptNcuPktDesc))  
    {
        return MCS_RET_FAIL;
    }

    T_psMcsIPv4Head* ptIpv4Head = (T_psMcsIPv4Head*)PktInnerData;
    if(IPv4_VERSION != ptIpv4Head->btVersion)
    {
        DEBUG_TRACE(DEBUG_ERR,"IP Type[%u] Err!\n",ptIpv4Head->btVersion);
        MCS_LOC_STAT_EX(ptMcsNcuPerform, qwRcvIpv4PktVersionErr, 1);
        return MCS_RET_FAIL;
    }
    
    zte_memcpy_s(ptNcuPktDesc->tSrcIp, IPV6_LEN, ptIpv4Head->tSrcIp, IPV4_LEN);
    zte_memcpy_s(ptNcuPktDesc->tDstIp, IPV6_LEN, ptIpv4Head->tDstIp, IPV4_LEN);
    ptNcuPktDesc->bPro = ptIpv4Head->bPro;
    BYTE bIpHeadLen = ptIpv4Head->btHeadLen*4;
    PktInnerData += bIpHeadLen;
    return psNcuParsePktTransLayerInfo(PktInnerData, ptNcuPktDesc, bIpHeadLen);
}

WORD32 psNcuParsePktIpv6Info(T_psNcuMcsPerform *ptMcsNcuPerform, T_psNcuPktDesc* ptNcuPktDesc, BYTE* PktInnerData)
{
    if(unlikely(NULL == ptMcsNcuPerform || NULL == PktInnerData || NULL == ptNcuPktDesc))  
    {
        return MCS_RET_FAIL;
    }

    T_psMcsIPv6Head* ptIpv6Head = (T_psMcsIPv6Head*)PktInnerData;
    if(IPv6_VERSION != ptIpv6Head->bVersion)
    {
        MCS_LOC_STAT_EX(ptMcsNcuPerform, qwRcvIpv6PktVersionErr, 1);
        return MCS_RET_FAIL;
    }
    zte_memcpy_s(ptNcuPktDesc->tSrcIp, IPV6_LEN, ptIpv6Head->tSrcIp, IPV6_LEN);
    zte_memcpy_s(ptNcuPktDesc->tDstIp, IPV6_LEN, ptIpv6Head->tDstIp, IPV6_LEN);

    BYTE bPro = ptIpv6Head->bNextHeader;
    WORD16 wIpHeadLen = sizeof(T_psMcsIPv6Head);
    PktInnerData += wIpHeadLen;

    while(1)
    {
        if(bPro == MCS_TCP_PROTOCOL || bPro == MCS_UDP_PROTOCOL || bPro == MCS_SCTP_PROTOCOL || bPro == MCS_ICMP_PROTOCOL)
        {
            break;
        }
        switch(bPro)
        {
            case MCS_IPV6_HOPBYHOP_EXHEAD:
            case MCS_IPV6_DEST_EXHEAD:
            case MCS_IPV6_ROUTE_EXHEAD:
            case MCS_IPV6_FRAG_EXHEAD:
            {
                WORD32 dwExHeadLength = 0;
                if(bPro == MCS_IPV6_FRAG_EXHEAD)
                {
                    dwExHeadLength = 8;
                }
                else
                {
                    dwExHeadLength = (WORD32)((*(PktInnerData + 1)+ 1) << 3);
                }
                bPro = *PktInnerData;
                PktInnerData += dwExHeadLength;
                wIpHeadLen += dwExHeadLength;
                continue;
            }
            default:
                return MCS_RET_FAIL;
        }
        break;
    }
    ptNcuPktDesc->bPro = bPro;
    return psNcuParsePktTransLayerInfo(PktInnerData, ptNcuPktDesc, wIpHeadLen);
}

WORD32 psNcuParsePktTransLayerInfo(BYTE* PktInnerData, T_psNcuPktDesc* ptNcuPktDesc, WORD16 wIpHeadLen)
{
    if(NULL == PktInnerData || NULL == ptNcuPktDesc)
    {
        return MCS_RET_FAIL;
    }

    switch(ptNcuPktDesc->bPro)
    {
        case MCS_TCP_PROTOCOL:
        {
            T_TcpBaseHeader *ptTcpHead = (T_TcpBaseHeader *)(PktInnerData);
            psNcuFillTcpInfo(ptTcpHead, wIpHeadLen, ptNcuPktDesc);
            break;
        }
        case MCS_UDP_PROTOCOL:
        case MCS_SCTP_PROTOCOL:
        {
            T_psMcsUdpHead* ptUdpH = (T_psMcsUdpHead *)(PktInnerData);
            ptNcuPktDesc->wSrcPort  = mcs_ntohs(ptUdpH->wSrcPort);
            ptNcuPktDesc->wDstPort = mcs_ntohs(ptUdpH->wDstPort);
            break;
        }
        case MCS_ICMP_PROTOCOL:
        {
            break;
        }
        case MCS_ICMP6_PROTOCOL:
        {
            break;
        }
        default:
            return MCS_RET_FAIL;
    }

    return MCS_RET_SUCCESS;
}

VOID psNcuFillTcpInfo(T_TcpBaseHeader *ptTcpHead, WORD16 wIpHeadLen, T_psNcuPktDesc* ptNcuPktDesc)
{
    if (NULL == ptTcpHead || NULL == ptNcuPktDesc)
    {
        return ;
    }
    WORD16 wIpTcpHeadLen = (ptTcpHead->bHeaderLen << 2) + wIpHeadLen;

    ptNcuPktDesc->wSrcPort = mcs_ntohs(ptTcpHead->wSrcPort);
    ptNcuPktDesc->wDstPort = mcs_ntohs(ptTcpHead->wDstPort);
    ptNcuPktDesc->dwSeqNumber = mcs_ntohl(ptTcpHead->dwSeqNumber);
    ptNcuPktDesc->dwAckNumber = mcs_ntohl(ptTcpHead->dwAckNumber);
    ptNcuPktDesc->bSynFlag = ptTcpHead->bSynFlag;
    ptNcuPktDesc->bFinFlag = ptTcpHead->bFinFlag;
    ptNcuPktDesc->bRstFlag = ptTcpHead->bRstFlag;
    ptNcuPktDesc->bAckFlag = ptTcpHead->bAckFlag;
    ptNcuPktDesc->wPayloadSize = ptNcuPktDesc->wPktLen > wIpTcpHeadLen ? ptNcuPktDesc->wPktLen - wIpTcpHeadLen : 0;
}

BOOLEAN psNcuIsPktHasSynAckFlg(T_psNcuPktDesc *ptPktDesc)
{
    if (NULL == ptPktDesc)
    {
        return FALSE;
    }

    BYTE bFlg = ptPktDesc->bSynFlag | ptPktDesc->bFinFlag | ptPktDesc->bRstFlag;

    if (bFlg)
    {
        return TRUE;
    }
    return FALSE;
}

/* Ended by AICoder, pid:zf3aagebcfn2015143140a7cf07021405109b1db */
