#include "psMcsEntry.h"
#include "UpfNcuSynInfo.h"
#include "psNcuPktSolveFunc.h"
#include "psNcuSubscribeProc.h"
#include "ps_css_interface.h"
#include "psMcsDebug.h"
#include "MemShareCfg.h"
#include "psNcuHeartBeatProc.h"
#include "psMcsRegComm.h"
#include "psNcuScanMsgProc.h"
#include "psUpfEvent.h"
#include "psNcuUcomMsg.h"
#include "psNefReportToNwdaf.h"
#include "ps_ncu_typedef.h"
#include "psNcuPktReasmProc.h"

#ifndef MIN
#define MIN(v1, v2) ((v1) < (v2) ? (v1) : (v2))
#endif
VOID psMcsThdParaInit(T_MediaProcThreadPara *ptMediaProcThreadPara, PS_PACKET *ptPacket);
BOOLEAN psNcuIppdPktProc(PS_PACKET *ptPacket);
void process_packet_by_type(PS_PACKET *ptPacket, T_NcuHeadInfo *ptNcuInfo, T_MediaProcThreadPara *ptMediaProcThreadPara);
void process_registered_func_packet(PS_PACKET *ptPacket, T_MediaProcThreadPara *ptMediaProcThreadPara, pfType_psGwVpfuMcsPktProc pFunc);
WORD32 ncu_c_packet_entry(PS_PACKET *ptPacket);

/*注册函数入口*/
MF_Mcs_RegMsg(CSS_FWD_NWDAF_IPV4,               psNcuHeartPktProc);
MF_Mcs_RegMsg(CSS_FWD_NWDAF_IPV6,               psNcuHeartPktProc);
MF_Mcs_RegMsg(CSS_FWD_UCOM,                     psVpfuNcuUcomMsgProc);
MF_Mcs_RegMsg(CSS_FWD_INNER_LOOPSCAN,           psNcuLoopScanPktProc);
MF_Mcs_RegMsg(CSS_FWD_NCU_TIPC_PKT,             psNcuTipcSBIPktProc);

void psVpfuMcsIPPktRcv(PS_PACKET *ptPacket)
{
    MCS_PCLINT_NULLPTR_RET_2ARG_VOID(ptPacket, ptPacket->pBufferNode);
    T_MediaProcThreadPara *ptMediaProcThreadPara = g_ptMediaProcThreadPara;

    psMcsThdParaInit(ptMediaProcThreadPara, ptPacket);

    DEBUG_TRACE(DEBUG_LOW, 
                     "Thread %u receive Packet wPktAttr=%u, pktlen=%u, dwPowerOnSec=%u，dwCurStdSec=%u\n", 
                     ptPacket->bSthr,  ptPacket->wPktAttr,ptPacket->dwPktLen,ptPacket->dwPowerOnSec,ptPacket->dwCurStdSec);

    T_psNcuMcsPerform           *ptMcsNcuPerform = (T_psNcuMcsPerform*)ptMediaProcThreadPara->ptMcsStatPointer;
    MCS_PCLINT_NULLPTR_RET_VOID(ptMcsNcuPerform);
    MCS_LOC_STAT_EX(ptMcsNcuPerform, qwRcvPktTotal, 1); 

    /* 重组 */
    MCS_CHK_VOID(FALSE == psNcuPktFragReasmEntry(ptPacket, ptMediaProcThreadPara));
    ptPacket = ptMediaProcThreadPara->ptPacket; 

    if (getNcuSoftPara(5060) != 0)
    {
        MCS_LOC_STAT_EX(ptMcsNcuPerform, qwRustEnable, 1);
        ncu_c_packet_entry(ptPacket);
        return;
    }

    /* 报文处理流程 */
    pfType_psGwVpfuMcsPktProc   pFunc = psGwVpfuMcsGetRegFuc(ptPacket->wPktAttr);
    if(likely(NULL != pFunc))
    {
        return process_registered_func_packet(ptPacket, ptMediaProcThreadPara, pFunc);
    }

    BYTE                        *aucPacketBuf = NULL;
    WORD16                      wBufferLen    = 0;
    WORD16                      wBufferOffset = 0;
    PS_GET_BUF_INFO(ptPacket->pBufferNode, aucPacketBuf, wBufferOffset, wBufferLen);
    T_NcuHeadInfo* ptNcuInfo = (T_NcuHeadInfo*)(aucPacketBuf+wBufferOffset);
    if(0 != ptNcuInfo->bVersion)
    {
        psNcuSolveOtherPkt(ptPacket, ptMediaProcThreadPara);
        return;
    }

    BUFF_TRACE(DEBUG_RCV, aucPacketBuf+wBufferOffset, wBufferLen);

    process_packet_by_type(ptPacket, ptNcuInfo, ptMediaProcThreadPara);
    if(unlikely(MCS_NEED_FREEPKT == ptMediaProcThreadPara->ucFreePktFlag))
    {
        MCS_LOC_STAT_EX(ptMcsNcuPerform, qwFreePktInEntry, 1);
        psCSSPktLinkBuffDesFree(ptPacket);
    }
    return;
}

void process_registered_func_packet(PS_PACKET *ptPacket, T_MediaProcThreadPara *ptMediaProcThreadPara, pfType_psGwVpfuMcsPktProc pFunc) 
{
    MCS_PCLINT_NULLPTR_RET_2ARG_VOID(ptPacket, ptMediaProcThreadPara);
    T_psNcuMcsPerform *ptMcsNcuPerform = (T_psNcuMcsPerform*)ptMediaProcThreadPara->ptMcsStatPointer;

    /* 判断是否为IPPD 报文，如果是IPPD报文退出 */
    if(psNcuIppdPktProc(ptPacket))
    {
        MCS_LOC_STAT_EX(ptMcsNcuPerform, qwRcvIPPDPkt, 1);
        return;
    }
    (void)pFunc(ptMediaProcThreadPara);
    /* 异常流程没有正常转发,报文统一在入口处释放 */
    if(unlikely(MCS_NEED_FREEPKT == ptMediaProcThreadPara->ucFreePktFlag))
    {
        MCS_LOC_STAT_EX(ptMcsNcuPerform, qwFreePktInEntry, 1);
        psCSSPktLinkBuffDesFree(ptPacket);
    }
}

void process_packet_by_type(PS_PACKET *ptPacket, T_NcuHeadInfo *ptNcuInfo, T_MediaProcThreadPara *ptMediaProcThreadPara) 
{
    MCS_PCLINT_NULLPTR_RET_2ARG_VOID(ptPacket, ptNcuInfo);  
    T_psNcuMcsPerform           *ptMcsNcuPerform = (T_psNcuMcsPerform*)ptMediaProcThreadPara->ptMcsStatPointer;
    MCS_PCLINT_NULLPTR_RET_VOID(ptMcsNcuPerform); 

    switch(ptNcuInfo->bType)
    {
        case NCU_SESSSION_SYN:
            DEBUG_TRACE(DEBUG_LOW, "psNcuSubScribeProc ptNcuInfo->bType=%u\n", ptNcuInfo->bType);
            psNcuSubScribeProc((void*)(ptNcuInfo + 1), ptMediaProcThreadPara);
            MCS_LOC_STAT_EX(ptMcsNcuPerform, qwRcvCtrlPkt, 1);
            break;
        case NCU_SUBSCRIPTION_SYN:
            DEBUG_TRACE(DEBUG_LOW, "psNcuSubScribeProc ptNcuInfo->bType=%u\n", ptNcuInfo->bType);
            psNcuSubScribeProc((void*)(ptNcuInfo + 1), ptMediaProcThreadPara);
            MCS_LOC_STAT_EX(ptMcsNcuPerform, qwRcvCtrlPkt, 1);
            break;
        case NCU_PKT_COPY:
            DEBUG_TRACE(DEBUG_LOW, "ThreadNo=%u, recv multi copy pkt ptNcuInfo->bType=%u\n", ptMediaProcThreadPara->bThreadNo, ptNcuInfo->bType);
            psNcuMultiPktSolveProc((void*)(ptNcuInfo + 1), ptNcuInfo->bPktnum, ptNcuInfo->wLength, ptMediaProcThreadPara);
            MCS_LOC_STAT_EX(ptMcsNcuPerform, qwRcvCopyPkt, 1);
            break;
        case NCU_PKT_TRAFFICREPORT:
            DEBUG_TRACE(DEBUG_LOW, "ThreadNo=%u, recv multi traffic report pkt ptNcuInfo->bType=%u\n", ptMediaProcThreadPara->bThreadNo, ptNcuInfo->bType);
            psNcuMultiTrafficReportSolveProc((void *)(ptNcuInfo + 1), ptNcuInfo->bPktnum, ptNcuInfo->wLength, ptMediaProcThreadPara);
            MCS_LOC_STAT_EX(ptMcsNcuPerform, qwRcvTrafficReport, 1);
            break;
        default:
            MCS_LOC_STAT_EX(ptMcsNcuPerform, qwRcvDefaultPkt, 1);
            psNcuSolveOtherPkt(ptPacket, ptMediaProcThreadPara);
            break;
    }
}

/**********************************************************************
* 函数名称：psVpfuNcuUcomMsgProc
* 功能描述：CSS_FWD_UCOM 地址属性报文处理
* 输入参数：
* 输出参数：无
* 返 回 值：无
* 其它说明：无
* 修改日期        版本号       修改人           修改内容
* --------------------------------------------------------
*                                                         创建
***********************************************************************/
VOID  psVpfuNcuUcomMsgProc(T_MediaProcThreadPara *ptMediaProcThreadPara)
{
    DEBUG_TRACE(DEBUG_LOW,"\n[MCS] CSS_FWD_UCOM packet handle");
    if(NULL == ptMediaProcThreadPara)
    {
        return;
    }
    PS_PACKET *ptPacket    = ptMediaProcThreadPara->ptPacket;
    T_psNcuMcsPerform *ptNcuPerform = (T_psNcuMcsPerform *)ptMediaProcThreadPara->ptMcsStatPointer;

    if(NULL == ptPacket || NULL == ptNcuPerform)
    {
        return;
    }

    BYTE *aucPacketBuf = NULL;
    WORD16 wBufferLen = 0;
    WORD16 wBufferOffset = 0;

    MCS_LOC_STAT_EX(ptNcuPerform, qwRcvUcomMsg, 1);

    PS_GET_BUF_INFO(ptPacket->pBufferNode, aucPacketBuf, wBufferOffset, wBufferLen);
    aucPacketBuf = aucPacketBuf + wBufferOffset;
    T_psUpfUcomHead *ptNcuUcomHead = (T_psUpfUcomHead *)(aucPacketBuf);
    aucPacketBuf +=  sizeof(T_psUpfUcomHead);

    DEBUG_TRACE(DEBUG_LOW,"\n[MCS] VPFU Rcv Mcs Ucom Msg dwMsgType(0x%08x) wMsgLen(%u)! func:%s line:%d",
                      ptNcuUcomHead->dwMsgType, ptNcuUcomHead->wMsgLen, __FUNCTION__, __LINE__);
    
    psNcuUcomMediaProc(ptMediaProcThreadPara, aucPacketBuf, ptNcuUcomHead);

    return ;
}

void psNcuSolveOtherPkt(PS_PACKET *ptPacket, T_MediaProcThreadPara *ptMediaProcThreadPara)
{
    MCS_PCLINT_NULLPTR_RET_VOID(ptPacket);
    MCS_PCLINT_NULLPTR_RET_2ARG_VOID(ptMediaProcThreadPara, ptMediaProcThreadPara->ptMcsStatPointer);

    T_psNcuMcsPerform *ptMcsNcuPerform = (T_psNcuMcsPerform *)ptMediaProcThreadPara->ptMcsStatPointer;
    DEBUG_TRACE(DEBUG_LOW, "psNcuSolveOtherPkt\n");

    ptMediaProcThreadPara->ucFreePktFlag = MCS_NONEED_FREEPKT; 
    T_psFtmIPPoolAction tIppoolAction    = {0};
    WORD32 ret = psCssNotMediaPktProc(ptPacket, &tIppoolAction);
    if(CSS_OK == ret)
    {
        MCS_LOC_STAT_EX(ptMcsNcuPerform, qwRcvNotMediaPktOk, 1);
        return;
    }
    if(CSS_CONTINUE == ret)
    {
        psCSSPktLinkBuffDesFree(ptPacket);
        MCS_LOC_STAT_EX(ptMcsNcuPerform, qwRcvNotMediaPktContinue, 1);
        return;
    }
    MCS_LOC_STAT_EX(ptMcsNcuPerform, qwRcvNotMediaPktFail, 1);
    return;
}

VOID psMcsThdParaInit(T_MediaProcThreadPara *ptMediaProcThreadPara, PS_PACKET *ptPacket)
{
    if(NULL == ptMediaProcThreadPara || NULL == ptPacket)
    {
        return ;
    }
    ptMediaProcThreadPara->ddwUPseid            = 0;
    ptMediaProcThreadPara->dwAppid              = 0;
    ptMediaProcThreadPara->dwSubAppid           = 0;
    ptMediaProcThreadPara->ptPacket             = ptPacket;
    ptMediaProcThreadPara->dwCurPktStdSec       = ptPacket->dwCurStdSec;
    ptMediaProcThreadPara->dwPktPowOnSec        = ptPacket->dwPowerOnSec;
    ptMediaProcThreadPara->bSubType             = 0;
    ptMediaProcThreadPara->ucFreePktFlag        = MCS_NEED_FREEPKT;
    zte_memset_s(&ptMediaProcThreadPara->tPktDesc, sizeof(T_psNcuPktDesc), 0, sizeof(T_psNcuPktDesc));
    return ;
}

BOOLEAN psNcuIppdPktProc(PS_PACKET *ptPacket)
{
    MCS_PCLINT_NULLPTR_RET_1ARG(ptPacket, FALSE);

    if(CSS_FWD_NWDAF_IPV4 != ptPacket->wPktAttr && CSS_FWD_NWDAF_IPV6 != ptPacket->wPktAttr)
    {
        return FALSE;
    }

    if(FALSE == isIPPDPktRcvProc(ptPacket, 0))
    {
        return FALSE;
    }

    /* 是IPPD报文的话，会在isIPPDPktRcvProc中释放掉报文描述符 */
    DEBUG_TRACE(DEBUG_LOW, "Ncu Rcv Ippd Pkt\n");
    return TRUE;
}
