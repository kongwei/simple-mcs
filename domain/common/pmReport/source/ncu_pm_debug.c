
#include "tulip.h"
#include "UPFHelp.h"
#include "zte_slibc.h"
#include "MemShareCfg.h"

UPF_HELP_REG("ncu", 
"show ncu session stat",
VOID psShowNcuSessionStat())
{
    WORD32 dwThdIdx                             = 0;
    WORD64 qwCurrentSessions                    = 0;   // NCU的当前会话数
    WORD64 qwQualityAssuranceSessions           = 0;   // NCU的质差保障会话数
    WORD64 qwKeyUserExperienceDataSessions      = 0;   // NCU的重点用户体验数据会话数
    WORD64 qwRegularUserExperienceDataSessions  = 0;   // NCU的普通用户体验数据会话数
    T_NcuSessionStat *ptNcuSessionStat          = NULL;

    for (dwThdIdx = 0; dwThdIdx < MEDIA_THRD_NUM; dwThdIdx++)
    {
        ptNcuSessionStat = &(g_ptVpfuShMemVar->tNcuSessionStat[dwThdIdx]);

        qwCurrentSessions                    += ptNcuSessionStat->qwCurrentSessions;
        qwQualityAssuranceSessions           += ptNcuSessionStat->qwQualityAssuranceSessions;
        qwKeyUserExperienceDataSessions      += ptNcuSessionStat->qwKeyUserExperienceDataSessions;
        qwRegularUserExperienceDataSessions  += ptNcuSessionStat->qwRegularUserExperienceDataSessions;
    }
    zte_printf_s("qwCurrentSessions = %llu\n", qwCurrentSessions);
    zte_printf_s("qwQualityAssuranceSessions = %llu\n", qwQualityAssuranceSessions);
    zte_printf_s("qwKeyUserExperienceDataSessions = %llu\n", qwKeyUserExperienceDataSessions);
    zte_printf_s("qwRegularUserExperienceDataSessions = %llu\n", qwRegularUserExperienceDataSessions);

    return;
}


UPF_HELP_REG("ncu", 
"show ncu ctrl flow stat",
VOID psShowNcuCtrlFlowStat())
{
    WORD32               dwThdIdx             = 0;
    T_NcuCtrlFlowStatPm  tNcuCtrlFlowStat     = {0};
    T_NcuCtrlFlowStatPm *ptNcuCtrlFlow        = NULL;

    for (dwThdIdx = 0; dwThdIdx < MEDIA_THRD_NUM; dwThdIdx++)
    {
        ptNcuCtrlFlow = &(g_ptVpfuShMemVar->tNcuCtrlFlow[dwThdIdx]);

        tNcuCtrlFlowStat.qwNcuRecvSessionSyncMsgNum                  += ptNcuCtrlFlow->qwNcuRecvSessionSyncMsgNum;
        tNcuCtrlFlowStat.qwNcuRecvSessionNewMsgNum                   += ptNcuCtrlFlow->qwNcuRecvSessionNewMsgNum;
        tNcuCtrlFlowStat.qwNcuRecvSessionModifyMsgNum                += ptNcuCtrlFlow->qwNcuRecvSessionModifyMsgNum;
        tNcuCtrlFlowStat.qwNcuRecvSessionDeleteMsgNum                += ptNcuCtrlFlow->qwNcuRecvSessionDeleteMsgNum;
        tNcuCtrlFlowStat.qwNcuRecvSubscribeMsgNum                    += ptNcuCtrlFlow->qwNcuRecvSubscribeMsgNum;
        tNcuCtrlFlowStat.qwNcuRecvSubscribeNewMsgNum                 += ptNcuCtrlFlow->qwNcuRecvSubscribeNewMsgNum;
        tNcuCtrlFlowStat.qwNcuRecvSubscribeModifyMsgNum              += ptNcuCtrlFlow->qwNcuRecvSubscribeModifyMsgNum;
        tNcuCtrlFlowStat.qwNcuRecvSubscribeCancelMsgNum              += ptNcuCtrlFlow->qwNcuRecvSubscribeCancelMsgNum;
        tNcuCtrlFlowStat.qwNcuRecvQoSSubscribeMsgFromN4Num           += ptNcuCtrlFlow->qwNcuRecvQoSSubscribeMsgFromN4Num;
        tNcuCtrlFlowStat.qwNcuRecvQoSNewSubscribeMsgFromN4Num        += ptNcuCtrlFlow->qwNcuRecvQoSNewSubscribeMsgFromN4Num;
        tNcuCtrlFlowStat.qwNcuRecvQoSModifySubscribeMsgFromN4Num     += ptNcuCtrlFlow->qwNcuRecvQoSModifySubscribeMsgFromN4Num;
        tNcuCtrlFlowStat.qwNcuRecvQoSCancelSubscribeMsgFromN4Num     += ptNcuCtrlFlow->qwNcuRecvQoSCancelSubscribeMsgFromN4Num;
        tNcuCtrlFlowStat.qwNcuRecvExpDataSubscribeMsgFromN4Num       += ptNcuCtrlFlow->qwNcuRecvExpDataSubscribeMsgFromN4Num;
        tNcuCtrlFlowStat.qwNcuRecvExpDataNewSubscribeMsgFromN4Num    += ptNcuCtrlFlow->qwNcuRecvExpDataNewSubscribeMsgFromN4Num;
        tNcuCtrlFlowStat.qwNcuRecvExpDataModifySubscribeMsgFromN4Num += ptNcuCtrlFlow->qwNcuRecvExpDataModifySubscribeMsgFromN4Num;
        tNcuCtrlFlowStat.qwNcuRecvExpDataCancelSubscribeMsgFromN4Num += ptNcuCtrlFlow->qwNcuRecvExpDataCancelSubscribeMsgFromN4Num;
        tNcuCtrlFlowStat.qwNcuRecvExpDataSubscribeMsgFromSBI         += ptNcuCtrlFlow->qwNcuRecvExpDataSubscribeMsgFromSBI;
        tNcuCtrlFlowStat.qwNcuRecvExpDataNewSubscribeMsgFromSBI      += ptNcuCtrlFlow->qwNcuRecvExpDataNewSubscribeMsgFromSBI;
        tNcuCtrlFlowStat.qwNcuRecvExpDataModifySubscribeMsgFromSBI   += ptNcuCtrlFlow->qwNcuRecvExpDataModifySubscribeMsgFromSBI;
        tNcuCtrlFlowStat.qwNcuRecvExpDataCancelSubscribeMsgFromSBI   += ptNcuCtrlFlow->qwNcuRecvExpDataCancelSubscribeMsgFromSBI;
        tNcuCtrlFlowStat.qwNcuReportQoSNumViaSBI                     += ptNcuCtrlFlow->qwNcuReportQoSNumViaSBI;
        tNcuCtrlFlowStat.qwNcuReportKeyExpDataNumViaUDP              += ptNcuCtrlFlow->qwNcuReportKeyExpDataNumViaUDP;
        tNcuCtrlFlowStat.qwNcuReportOrdinaryExpDataNumViaUDP         += ptNcuCtrlFlow->qwNcuReportOrdinaryExpDataNumViaUDP;
        tNcuCtrlFlowStat.qwContextPullRequestCount                   += ptNcuCtrlFlow->qwContextPullRequestCount;
        tNcuCtrlFlowStat.qwNcuReportKeyExpDataNumViaSBI              += ptNcuCtrlFlow->qwNcuReportKeyExpDataNumViaSBI;
    }
    zte_printf_s("qwNcuRecvSessionSyncMsgNum = %llu\n", tNcuCtrlFlowStat.qwNcuRecvSessionSyncMsgNum);
    zte_printf_s("qwNcuRecvSessionNewMsgNum = %llu\n", tNcuCtrlFlowStat.qwNcuRecvSessionNewMsgNum);
    zte_printf_s("qwNcuRecvSessionModifyMsgNum = %llu\n", tNcuCtrlFlowStat.qwNcuRecvSessionModifyMsgNum);
    zte_printf_s("qwNcuRecvSessionDeleteMsgNum = %llu\n", tNcuCtrlFlowStat.qwNcuRecvSessionDeleteMsgNum);
    zte_printf_s("qwNcuRecvSubscribeMsgNum = %llu\n", tNcuCtrlFlowStat.qwNcuRecvSubscribeMsgNum);
    zte_printf_s("qwNcuRecvSubscribeNewMsgNum = %llu\n", tNcuCtrlFlowStat.qwNcuRecvSubscribeNewMsgNum);
    zte_printf_s("qwNcuRecvSubscribeModifyMsgNum = %llu\n", tNcuCtrlFlowStat.qwNcuRecvSubscribeModifyMsgNum);
    zte_printf_s("qwNcuRecvSubscribeCancelMsgNum = %llu\n", tNcuCtrlFlowStat.qwNcuRecvSubscribeCancelMsgNum);
    zte_printf_s("qwNcuRecvQoSSubscribeMsgFromN4Num = %llu\n", tNcuCtrlFlowStat.qwNcuRecvQoSSubscribeMsgFromN4Num);
    zte_printf_s("qwNcuRecvQoSNewSubscribeMsgFromN4Num = %llu\n", tNcuCtrlFlowStat.qwNcuRecvQoSNewSubscribeMsgFromN4Num);
    zte_printf_s("qwNcuRecvQoSModifySubscribeMsgFromN4Num = %llu\n", tNcuCtrlFlowStat.qwNcuRecvQoSModifySubscribeMsgFromN4Num);
    zte_printf_s("qwNcuRecvQoSCancelSubscribeMsgFromN4Num = %llu\n", tNcuCtrlFlowStat.qwNcuRecvQoSCancelSubscribeMsgFromN4Num);
    zte_printf_s("qwNcuRecvExpDataSubscribeMsgFromN4Num = %llu\n", tNcuCtrlFlowStat.qwNcuRecvExpDataSubscribeMsgFromN4Num);
    zte_printf_s("qwNcuRecvExpDataNewSubscribeMsgFromN4Num = %llu\n", tNcuCtrlFlowStat.qwNcuRecvExpDataNewSubscribeMsgFromN4Num);
    zte_printf_s("qwNcuRecvExpDataModifySubscribeMsgFromN4Num = %llu\n", tNcuCtrlFlowStat.qwNcuRecvExpDataModifySubscribeMsgFromN4Num);
    zte_printf_s("qwNcuRecvExpDataCancelSubscribeMsgFromN4Num = %llu\n", tNcuCtrlFlowStat.qwNcuRecvExpDataCancelSubscribeMsgFromN4Num);
    zte_printf_s("qwNcuRecvExpDataSubscribeMsgFromSBI = %llu\n", tNcuCtrlFlowStat.qwNcuRecvExpDataSubscribeMsgFromSBI);
    zte_printf_s("qwNcuRecvExpDataNewSubscribeMsgFromSBI = %llu\n", tNcuCtrlFlowStat.qwNcuRecvExpDataNewSubscribeMsgFromSBI);
    zte_printf_s("qwNcuRecvExpDataModifySubscribeMsgFromSBI = %llu\n", tNcuCtrlFlowStat.qwNcuRecvExpDataModifySubscribeMsgFromSBI);
    zte_printf_s("qwNcuRecvExpDataCancelSubscribeMsgFromSBI = %llu\n", tNcuCtrlFlowStat.qwNcuRecvExpDataCancelSubscribeMsgFromSBI);
    zte_printf_s("qwNcuReportQoSNumViaSBI = %llu\n", tNcuCtrlFlowStat.qwNcuReportQoSNumViaSBI);
    zte_printf_s("qwNcuReportKeyExpDataNumViaUDP = %llu\n", tNcuCtrlFlowStat.qwNcuReportKeyExpDataNumViaUDP);
    zte_printf_s("qwNcuReportOrdinaryExpDataNumViaUDP = %llu\n", tNcuCtrlFlowStat.qwNcuReportOrdinaryExpDataNumViaUDP);
    zte_printf_s("qwContextPullRequestCount = %llu\n", tNcuCtrlFlowStat.qwContextPullRequestCount);
    zte_printf_s("qwNcuReportKeyExpDataNumViaSBI = %llu\n", tNcuCtrlFlowStat.qwNcuReportKeyExpDataNumViaSBI);
    return;
}

UPF_HELP_REG("ncu", 
"show ncu user flow stat",
VOID psShowNcuUserFlowStat())
{
    WORD32 dwThdIdx                     = 0;
    WORD64 qwNcuRcvCpyPktMsgs           = 0;    // NCU接收复制报文消息数
    WORD64 qwNcuRcvUlCpyPkts            = 0;    // NCU接收上行复制报文个数
    WORD64 qwNcuRcvDlCpyPkts            = 0;    // NCU接收下行复制报文个数
    WORD64 qwNcuRcvUlCpyPktBytes        = 0;    // NCU接收上行复制报文字节数（Byte）
    WORD64 qwNcuRcvDlCpyPktBytes        = 0;    // NCU接收下行复制报文字节数（Byte）
    WORD64 qwNcuRcvTrafficRptMsgs       = 0;      //NCU接收流量上报消息数 
    WORD64 qwNcuRcvUlTrafficRptPktsNum     = 0;       //NCU接收上行流量上报报文个数 
    WORD64 qwNcuRcvDlTrafficRptPktsNum     = 0;       //NCU接收下行流量上报报文个数
    WORD64 qwNcuRcvUlTrafficRptPktsBytes= 0;   //NCU接收上行流量上报报文字节数（Byte）
    WORD64 qwNcuRcvDlTrafficRptPktsBytes= 0;   //NCU接收下行流量上报报文字节数（Byte）
    T_NcuUserFlowStatPm *ptNcuUserFlow  = NULL;

    for (dwThdIdx = 0; dwThdIdx < MEDIA_THRD_NUM; dwThdIdx++)
    {
        ptNcuUserFlow = &(g_ptVpfuShMemVar->tNcuUserFlow[dwThdIdx]);

        qwNcuRcvCpyPktMsgs          += ptNcuUserFlow->qwNcuRcvCpyPktMsgs;
        qwNcuRcvUlCpyPkts           += ptNcuUserFlow->qwNcuRcvUlCpyPkts;
        qwNcuRcvDlCpyPkts           += ptNcuUserFlow->qwNcuRcvDlCpyPkts;
        qwNcuRcvUlCpyPktBytes       += ptNcuUserFlow->qwNcuRcvUlCpyPktBytes;
        qwNcuRcvDlCpyPktBytes       += ptNcuUserFlow->qwNcuRcvDlCpyPktBytes;
        qwNcuRcvTrafficRptMsgs          += ptNcuUserFlow->qwNcuRcvTrafficRptMsgs;
        qwNcuRcvUlTrafficRptPktsNum        += ptNcuUserFlow->qwNcuRcvUlTrafficRptPktsNum;
        qwNcuRcvDlTrafficRptPktsNum        += ptNcuUserFlow->qwNcuRcvDlTrafficRptPktsNum;
        qwNcuRcvUlTrafficRptPktsBytes   += ptNcuUserFlow->qwNcuRcvUlTrafficRptPktsBytes;
        qwNcuRcvDlTrafficRptPktsBytes   += ptNcuUserFlow->qwNcuRcvDlTrafficRptPktsBytes;
    }
    zte_printf_s("qwNcuRcvCpyPktMsgs = %llu\n", qwNcuRcvCpyPktMsgs);
    zte_printf_s("qwNcuRcvUlCpyPkts = %llu\n", qwNcuRcvUlCpyPkts);
    zte_printf_s("qwNcuRcvDlCpyPkts = %llu\n", qwNcuRcvDlCpyPkts);
    zte_printf_s("qwNcuRcvUlCpyPktBytes = %llu\n", qwNcuRcvUlCpyPktBytes);
    zte_printf_s("qwNcuRcvDlCpyPktBytes = %llu\n", qwNcuRcvDlCpyPktBytes);
    zte_printf_s("qwNcuRcvTrafficRptMsgs = %llu\n", qwNcuRcvTrafficRptMsgs);
    zte_printf_s("qwNcuRcvUlTrafficRptPktsNum = %llu\n", qwNcuRcvUlTrafficRptPktsNum);
    zte_printf_s("qwNcuRcvDlTrafficRptPktsNum = %llu\n", qwNcuRcvDlTrafficRptPktsNum);
    zte_printf_s("qwNcuRcvUlTrafficRptPktsBytes = %llu\n", qwNcuRcvUlTrafficRptPktsBytes);
    zte_printf_s("qwNcuRcvDlTrafficRptPktsBytes = %llu\n", qwNcuRcvDlTrafficRptPktsBytes);

    return;
}

UPF_HELP_REG("ncu", 
"clear ncu session stat",
VOID psClearNcuSessionStat())
{
    zte_memset_s(g_ptVpfuShMemVar->tNcuSessionStat, sizeof(T_NcuSessionStat) * (MEDIA_THRD_NUM + 1), 0, sizeof(T_NcuSessionStat) * (MEDIA_THRD_NUM + 1));
    return;
}

UPF_HELP_REG("ncu", 
"clear ncu ctrl flow stat",
VOID psClearNcuCtrlFlowStat())
{
    zte_memset_s(g_ptVpfuShMemVar->tNcuCtrlFlow, sizeof(T_NcuCtrlFlowStatPm) * (MEDIA_THRD_NUM + 1), 0, sizeof(T_NcuCtrlFlowStatPm) * (MEDIA_THRD_NUM + 1));
    return;
}

UPF_HELP_REG("ncu", 
"clear ncu user flow stat",
VOID psClearNcuUserFlowStat())
{
    zte_memset_s(g_ptVpfuShMemVar->tNcuUserFlow, sizeof(T_NcuUserFlowStatPm) * (MEDIA_THRD_NUM + 1), 0, sizeof(T_NcuUserFlowStatPm) * (MEDIA_THRD_NUM + 1));
    return;
}
