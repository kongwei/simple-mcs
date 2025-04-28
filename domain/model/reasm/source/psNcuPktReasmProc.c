/* Started by AICoder, pid:20b6b2ed9d235ba145250a5c906ba75930c005c6 */
/******************************************************************************
 * 版权所有 (C)2016 深圳市中兴通讯股份有限公司*
 * 模块名          : NCU
 * 文件名          : psNcuPktReasmProc.c
 * 相关文件        :
 * 文件实现功能     : NCU报文重组流程
 * 归属团队        : M6
 * 版本           : V1.0
-------------------------------------------------------------------------------
* 修改记录:
* 修改日期            版本号           修改人           修改内容
* 2024-08-27        V7.24.30        zjw            create
******************************************************************************/
/**************************************************************************
 *                              头文件(满足最小依赖请按照DDD分层架构逐层依赖)
 **************************************************************************/
// application    层依赖
#include "psNcuPktReasmProc.h"
// infrastructure
#include "ps_ncu_typedef.h"
#include "psMcsDebug.h"

// 非DDD目录依赖
#include "McsIPv4Head.h"
#include "McsIPv6Head.h"
#include "McsProtoType.h"


/**************************************************************************
 *                              宏(本源文件使用)
 **************************************************************************/
#define NCU_IPV6_FRAG_EXHEAD_LEN_INVALID(wHeaderLen, wBufferLen) ((((wHeaderLen) + 8) > (1280 - 8)) || (((wHeaderLen) + 8) > (wBufferLen)))
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
WORD32 psNcuIpv4ReasmProc(PS_PACKET *ptPacket, T_MediaProcThreadPara *ptMediaProcThreadPara);
WORD32 psNcuIpv6ReasmProc(PS_PACKET *ptPacket, T_MediaProcThreadPara *ptMediaProcThreadPara);
INLINE BOOLEAN  psNcuIsIPV6NeedReasm(PS_PACKET* ptPacket);
INLINE BOOLEAN psNcuIsIPV6SingleBufNeedReasm(PS_PACKET *ptPacket);
BOOLEAN psNcuJudgeIsNeedFrag(PS_PACKET *ptPacket);
WORD32 psNcuPktFragReasmProc(PS_PACKET *ptPacket, T_MediaProcThreadPara *ptMediaProcThreadPara);

/**************************************************************************
 *                              全局变量(本源文件使用)
 **************************************************************************/

/**************************************************************************
 *                              函数实现(函数布局:总分结构|深度优先)
 **************************************************************************/
BOOLEAN psNcuPktFragReasmEntry(PS_PACKET *ptPacket, T_MediaProcThreadPara *ptMediaProcThreadPara)
{
    MCS_PCLINT_NULLPTR_RET_2ARG(ptPacket, ptMediaProcThreadPara, FALSE);
    T_psNcuMcsPerform *ptMcsNcuPerform = (T_psNcuMcsPerform *)ptMediaProcThreadPara->ptMcsStatPointer;
    MCS_PCLINT_NULLPTR_RET_1ARG(ptMcsNcuPerform, FALSE);

    BOOLEAN bIsNeedFrag = psNcuJudgeIsNeedFrag(ptPacket);
    MCS_CHK_RET_LIKELY(FALSE == bIsNeedFrag, TRUE);

    ptMediaProcThreadPara->ucFreePktFlag = MCS_NEED_FREEPKT;
    WORD32 dwResult  = psNcuPktFragReasmProc(ptPacket, ptMediaProcThreadPara);
    if(MCS_RET_SUCCESS != dwResult)
    {
        if(unlikely(MCS_NEED_FREEPKT == ptMediaProcThreadPara->ucFreePktFlag))
        {
            MCS_LOC_STAT_EX(ptMcsNcuPerform, qwFreePktInEntry, 1);
            psCSSPktLinkBuffDesFree(ptPacket);
        }
        return FALSE;
    }
    return TRUE;
}

WORD32 psNcuPktFragReasmProc(PS_PACKET *ptPacket, T_MediaProcThreadPara *ptMediaProcThreadPara)
{
    MCS_PCLINT_NULLPTR_RET_2ARG(ptPacket, ptMediaProcThreadPara, MCS_RET_BREAK);
    WORD32 dwResult = MCS_RET_SUCCESS;
    BYTE  *aucPacketBuf         = NULL;
    WORD16  wBufferLen          = 0;
    WORD16  wBufferOffset       = 0;

    PS_GET_BUF_INFO(ptPacket->pBufferNode, aucPacketBuf, wBufferOffset, wBufferLen);
    BYTE bIpVersion = (*(BYTE *)(aucPacketBuf + wBufferOffset))>>4;
    if(MCS_IP_TECH_IPV4 == bIpVersion)
    {
        dwResult = psNcuIpv4ReasmProc(ptPacket, ptMediaProcThreadPara);
    }
    else if(MCS_IP_TECH_IPV6 == bIpVersion)
    {
        dwResult = psNcuIpv6ReasmProc(ptPacket, ptMediaProcThreadPara);
    }
    return dwResult;
}

WORD32 psNcuIpv4ReasmProc(PS_PACKET *ptPacket, T_MediaProcThreadPara *ptMediaProcThreadPara)
{
    MCS_PCLINT_NULLPTR_RET_2ARG(ptPacket, ptMediaProcThreadPara, MCS_RET_BREAK);
    MCS_PCLINT_NULLPTR_RET_1ARG(ptMediaProcThreadPara->ptMcsStatPointer, MCS_RET_BREAK);

    T_psNcuMcsPerform *ptNcuPerform = (T_psNcuMcsPerform*)ptMediaProcThreadPara->ptMcsStatPointer;
    T_psvPFUPktInfo   *ptPktInfo    = (T_psvPFUPktInfo *)ptPacket->bDataArea;
    MCS_PCLINT_NULLPTR_RET_1ARG(ptPktInfo, MCS_RET_BREAK);
    BYTE              bBufLinkNum   = 1;

    if (NULL != ptPacket->pPktLink)
    {
        MCS_LOC_STAT_EX(ptNcuPerform, qwNcuIpv4ReasmRcvPktLinkCnt, 1);
        ptMediaProcThreadPara->ptPacket = NULL;

        return MCS_RET_BREAK;
    }
    /*上行的分片报文统计*/
    MCS_LOC_STAT_EX(ptNcuPerform, qwNcuIpv4ReasmInPkts, 1);

    PACKET_PROCESS_END(ptPacket, PKTCHK_MCS_PROC);
    ptPacket = psCSSIpv4Reasm(ptPacket);
    if(NULL == ptPacket)
    {
        /*614005805525---描述符被释放，防止 继续使用该描述符*/
        ptMediaProcThreadPara->ptPacket = NULL;
        ptMediaProcThreadPara->ucFreePktFlag = MCS_NONEED_FREEPKT;
        return MCS_RET_FAIL;
    }

    MCS_LOC_STAT_EX(ptNcuPerform, qwNcuIpv4ReasmOutPkts, 1);

    /* 修改问题单611000496577:报文重组后报文描述符发生切换，需要对ptPktInfo重新赋值*/
    /*线程数据区修改临时方案，第一片ptPktInfo回写*/
    NCU_GET_THREADPARA_FROM_DELIVERPKT(ptMediaProcThreadPara, ptPacket);

    /* 重组完成后，重新获取一下buffer个数 */
    PS_GET_BUFNODE_LINK_LEN(ptPacket, bBufLinkNum);
    ptPktInfo->bBufLinkNum = bBufLinkNum;
    return MCS_RET_SUCCESS;
}

WORD32 psNcuIpv6ReasmProc(PS_PACKET *ptPacket, T_MediaProcThreadPara *ptMediaProcThreadPara)
{
    MCS_PCLINT_NULLPTR_RET_2ARG(ptPacket, ptMediaProcThreadPara, MCS_RET_BREAK);
    MCS_PCLINT_NULLPTR_RET_1ARG(ptMediaProcThreadPara->ptMcsStatPointer, MCS_RET_BREAK);

    T_psNcuMcsPerform *ptNcuPerform = (T_psNcuMcsPerform*)ptMediaProcThreadPara->ptMcsStatPointer;
    T_psvPFUPktInfo   *ptPktInfo    = (T_psvPFUPktInfo *)ptPacket->bDataArea;
    MCS_PCLINT_NULLPTR_RET_1ARG(ptPktInfo, MCS_RET_BREAK);
    BYTE              bBufLinkNum   = 1;

    if(NULL != ptPacket->pPktLink)
    {
        MCS_LOC_STAT_EX(ptNcuPerform, qwNcuIpv6ReasmRcvPktLinkCnt, 1);
        return MCS_RET_BREAK;
    }
    /*上行的分片报文统计*/
    MCS_LOC_STAT_EX(ptNcuPerform, qwNcuIpv6ReasmInPkts, 1);

    PACKET_PROCESS_END(ptPacket, PKTCHK_MCS_PROC);
    ptPacket = psCSSIpv6Reasm(ptPacket);
    if (NULL == ptPacket)
    {
        /*614005805525---描述符被释放，防止 继续使用该描述符*/
        ptMediaProcThreadPara->ptPacket = NULL;
        ptMediaProcThreadPara->ucFreePktFlag = MCS_NONEED_FREEPKT;
        return MCS_RET_FAIL;
    }

    PS_RECORD_IPOFFSET_AFTER_REASM(ptPacket);
    /*上行的分片报文重组后统计*/
    MCS_LOC_STAT_EX(ptNcuPerform, qwNcuIpv6ReasmOutPkts, 1);

    /* 修改问题单611000496577:报文重组后报文描述符发生切换，需要对ptPktInfo重新赋值*/
    /*线程数据区修改，ptPacket,ptPktInfo回写*/
    NCU_GET_THREADPARA_FROM_DELIVERPKT(ptMediaProcThreadPara, ptPacket);

    /* 重组完成后，重新获取一下buffer个数 */
    PS_GET_BUFNODE_LINK_LEN(ptPacket, bBufLinkNum);
    ptPktInfo->bBufLinkNum = bBufLinkNum;
    return MCS_RET_SUCCESS;
}


BOOLEAN psNcuJudgeIsNeedFrag(PS_PACKET *ptPacket)
{
    MCS_PCLINT_NULLPTR_RET_1ARG(ptPacket, FALSE);
    T_psMcsIPv4Head *ptIPv4H   = NULL;
    BYTE  *aucPacketBuf   = NULL;
    WORD16  wBufferLen    = 0;
    WORD16  wBufferOffset = 0;
    WORD16  wFragOffSet   = 0;
    BYTE    bBufLinkNum   = 1;

    PS_GET_BUF_INFO(ptPacket->pBufferNode, aucPacketBuf, wBufferOffset, wBufferLen);
    MCS_CHK_RET(NULL == aucPacketBuf, FALSE);

    BYTE bIpVersion = (*(BYTE *)(aucPacketBuf + wBufferOffset))>>4;
    if(MCS_IP_TECH_IPV4 == bIpVersion) //IPv4_VERSION
    {
        MCS_CHK_RET(wBufferLen < MCS_CAP_IPV4_HEAD_LEN, FALSE);
        ptIPv4H = (T_psMcsIPv4Head *)(aucPacketBuf + wBufferOffset);
        wFragOffSet = mcs_ntohs(ptIPv4H->wOffSet);
        if(0 != (wFragOffSet & 0x3fff))
        {
            return TRUE;
        }
    }
    else if(MCS_IP_TECH_IPV6 == bIpVersion)
    {
        MCS_CHK_RET(wBufferLen < MCS_CAP_IPV6_HEAD_LEN, FALSE);
        PS_GET_BUFNODE_LINK_LEN(ptPacket, bBufLinkNum);
        return psNcuIsIPV6NeedReasm(ptPacket);
    }
    return FALSE;
}

INLINE BOOLEAN  psNcuIsIPV6NeedReasm(PS_PACKET* ptPacket)
{
    MCS_PCLINT_NULLPTR_RET_1ARG(ptPacket, FALSE);

    return psNcuIsIPV6SingleBufNeedReasm(ptPacket);
}

/**********************************************************************
* 函数名称： psNcuIsIPV6SingleBufNeedReasm
* 功能描述： 判断ipv6是否需要重组
* 输入参数：
* 输出参数：
* 返 回 值：
* 其它说明：
* 修改日期            版本号           修改人              修改内容
* ---------------------------------------------------------------------
* 2016-11-25        V1.0           miao.caibing         create
***********************************************************************/
INLINE BOOLEAN psNcuIsIPV6SingleBufNeedReasm(PS_PACKET *ptPacket)
{
    MCS_PCLINT_NULLPTR_RET_1ARG(ptPacket, FALSE);
    BYTE                        *aucPacketBuf = NULL;
    WORD16                      wBufferLen    = 0;
    WORD16                      wBufferOffset = 0;
    WORD16                      wHeaderLen    = MCS_IPV6_BASE_HEADER_LENGTH;/*固定头长*/
    BYTE                        bLoopCnt = 0;

    PS_GET_BUF_INFO(ptPacket->pBufferNode, aucPacketBuf, wBufferOffset, wBufferLen);
    MCS_CHK_RET((wHeaderLen > wBufferLen), FALSE);
    T_psMcsIPv6Head             *ptIPv6H = (T_psMcsIPv6Head *)(aucPacketBuf + wBufferOffset);
    BYTE                        bNxtHeaderType = ptIPv6H->bNextHeader;

    MCS_CHK_RET_LIKELY(MCS_TCP_PROTOCOL == bNxtHeaderType, FALSE);/*性能提升优化，大多情况不存在扩展头*/
    while((bLoopCnt++) <= 10)
    {
        MCS_CHK_RET_LIKELY(MCS_TCP_PROTOCOL == bNxtHeaderType, FALSE);/*性能提升优化，大多情况不存在扩展头*/
        switch (bNxtHeaderType)
        {
            case MCS_IPV6_FRAG_EXHEAD:
            {
                /* 扩展头最小长度8 个字节 */
                if((1 == getNcuSoftPara(5005)) &&
                        (NCU_IPV6_FRAG_EXHEAD_LEN_INVALID(wHeaderLen, wBufferLen)) &&
                        g_ptMediaProcThreadPara != NULL)
                {
                    T_psNcuMcsPerform *ptNcuPerform = (T_psNcuMcsPerform*)g_ptMediaProcThreadPara->ptMcsStatPointer;
                    // DDD差异2024Q2:MCS_LOC_STAT变更为MCS_LOC_STAT_EX
                    MCS_CHK_STAT((ptNcuPerform != NULL), qwNcuIPv6ExtHeadLenInvalidSingleBuf);
                    return FALSE;
                }
                return TRUE;
            }
            case MCS_IPV6_HOPBYHOP_EXHEAD:/*协议规定只有这三种头能在扩展头前面*/
            case MCS_IPV6_DEST_EXHEAD:
            case MCS_IPV6_ROUTE_EXHEAD:
            {
                /* 扩展头最小长度8 个字节 */
                if((wHeaderLen + 8) > wBufferLen)
                {
                    return FALSE;
                }
                BYTE * ptr = (aucPacketBuf + wBufferOffset) +wHeaderLen;
                bNxtHeaderType = *ptr;/*下一个扩展头类型*/
                /* 扩展头长度在第二个字节以8字节为单位，不包含头8字节 */
                ptr = (aucPacketBuf + wBufferOffset) +wHeaderLen +1;
                if((*ptr * 8 +8) > wBufferLen)
                {
                    return FALSE;
                }
                wHeaderLen += (*ptr * 8 +8);/*总长度*/
                break;
            }
            default: return FALSE;
        }
    }
    return FALSE;
}

/* Ended by AICoder, pid:20b6b2ed9d235ba145250a5c906ba75930c005c6 */
