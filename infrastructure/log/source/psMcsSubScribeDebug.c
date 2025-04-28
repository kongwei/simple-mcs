#include "psMcsSubScribeDebug.h"
#include "ps_ncu_session.h"
#include "psNcuSubscribeCtxProc.h"
#include "MemShareCfg.h"
#include "psMcsDebug.h"
#include "zte_slibc.h"
#include "UPFHelp.h"
typedef struct{
    WORD64 ddwUPSeid;
    WORD32 dwhDB;
}T_Save_Session;
extern VOID psUpfCommIMSIToString(T_IMSI tIMSI, BYTE *pbIMSI,BYTE bIMSILen);
T_Save_Session g_last_session = {0};
void printT_psNcuSessionCtx(T_psNcuSessionCtx *ctx);
void printT_psNcuDaSubScribeCtx(T_psNcuDaSubScribeCtx *ctx);
void show_last_session();
void show_last_subscribe();
void set_last_session(WORD64 ddwUPSeid, WORD32 dwhDB)
{
    g_last_session.ddwUPSeid = ddwUPSeid;
    g_last_session.dwhDB = dwhDB;
}
UPF_HELP_REG("mcs","show_last_user",
void show_last_user())
{
    show_last_session();
    show_last_subscribe();
}

UPF_HELP_REG("mcs","show_last_session",
void show_last_session())
{
    T_psNcuSessionCtx* ptSession = psQuerySessionByUpseid(g_last_session.ddwUPSeid, g_last_session.dwhDB);
    if(NULL == ptSession)
    {
        zte_printf_s("ptSessionCtx           = NULL, upseid=%llu, dwhdb=%u\n", g_last_session.ddwUPSeid, g_last_session.dwhDB);
        return;
    }
    zte_printf_s("ptSessionCtx           = %p, upseid=%llu, dwhdb=%u\n", ptSession, g_last_session.ddwUPSeid, g_last_session.dwhDB);
    printT_psNcuSessionCtx(ptSession);
}

UPF_HELP_REG("mcs","show_last_subscribe",
void show_last_subscribe())
{
    MCS_DM_QUERYDYNDATA_ACK *ptMcsDynCtxNoUniqAck = &g_ptVpfuShMemVar->tGwGloData.atMcsDynCtxNoUniqAck[0];
    WORD32 dwNum = psVpfuMcsGetAllSubScribeCtxByUPseid(g_last_session.ddwUPSeid, g_last_session.dwhDB, (BYTE*)ptMcsDynCtxNoUniqAck);
    if(0 == dwNum)
    {
        zte_printf_s("psVpfuMcsGetAllSubScribeCtxByUPseid=0, upseid=%llu, dwhdb=%u\n", g_last_session.ddwUPSeid, g_last_session.dwhDB);
        return;
    }
    WORD32 dwIndex =0;
    WORD32 dwCtxId = 0;
    zte_printf_s("psVpfuMcsGetAllSubScribeCtxByUPseid=%u, upseid=%llu, dwhdb=%u\n", dwNum, g_last_session.ddwUPSeid, g_last_session.dwhDB);
    for(dwIndex=0; dwIndex <EXPECT_SUBSCRIBE_NUM && dwIndex<dwNum; dwIndex++)
    {
        dwCtxId = ptMcsDynCtxNoUniqAck->adwDataArea[dwIndex];
        T_psNcuDaSubScribeCtx  *ptSubScribeCtx = psMcsGetsubscribeCtxById(dwCtxId, g_last_session.dwhDB);
        if(NULL == ptSubScribeCtx)
        {
            continue;
        }
        printT_psNcuDaSubScribeCtx(ptSubScribeCtx);
    }
}


/* Started by AICoder, pid:hafd832337r2f2f14d15090a206d9f2f01417d15 */
void printT_psNcuSessionCtx(T_psNcuSessionCtx *ctx) 
{
    if(NULL == ctx)
    {
        return;
    }
    zte_printf_s("ptSessionCtx->ddwUPSeid           = %llu\n", ctx->ddwUPSeid);
    zte_printf_s("ptSessionCtx->dwSessionCtxId      = %u\n", ctx->dwSessionCtxId);
    zte_printf_s("ptSessionCtx->dwSessionID         = %u\n", ctx->dwSessionID);
    zte_printf_s("ptSessionCtx->dwUpdateTimeStamp   = %u\n", ctx->dwUpdateTimeStamp);
    zte_printf_s("ptSessionCtx->dwCreateTimeStamp   = %u\n", ctx->dwCreateTimeStamp);
    zte_printf_s("ptSessionCtx->Apn                 = %s\n", ctx->Apn);
    
    BYTE imsiStr[17] = {0};
    psUpfCommIMSIToString(ctx->bImsi, imsiStr, 16);
    zte_printf_s("ptSessionCtx->bImsi               = %s\n", imsiStr);
    BYTE isdnStr[17] = {0};
    psUpfCommIMSIToString(ctx->bIsdn, isdnStr, 16);
    zte_printf_s("ptSessionCtx->bIsdn               = %s\n", isdnStr);

    char ipv6str[64] = {0};
    inet_ntop(AF_INET6, ctx->tMSIPv6, ipv6str, 64);
    zte_printf_s("ptSessionCtx->tMSIPv6             = %s\n", ipv6str);
    inet_ntop(AF_INET6, ctx->tNwdafIPv6, ipv6str, 64);
    zte_printf_s("ptSessionCtx->tNwdafIPv6          = %s\n", ipv6str);
    zte_printf_s("ptSessionCtx->tNwdafIPv4          = %u.%u.%u.%u\n", ctx->tNwdafIPv4[0],ctx->tNwdafIPv4[1],ctx->tNwdafIPv4[2],ctx->tNwdafIPv4[3]); // 假设 tNwdafIPv4 是无符号整数
    zte_printf_s("ptSessionCtx->tMSIPv4             = %u.%u.%u.%u\n", ctx->tMSIPv4[0],ctx->tMSIPv4[1],ctx->tMSIPv4[2],ctx->tMSIPv4[3]);
    zte_printf_s("ptSessionCtx->bRatType            = %u\n", ctx->bRatType);
    zte_printf_s("ptSessionCtx->bHasUli             = %u\n", ctx->bHasUli);
    zte_printf_s("ptSessionCtx->bMSIPType           = %u\n", ctx->bMSIPType); 
    zte_printf_s("ptSessionCtx->bNwdafIPType        = %u\n", ctx->bNwdafIPType);
    zte_printf_s("ptSessionCtx->Uri_Exp             = %s\n", ctx->Uri_Exp);
    zte_printf_s("ptSessionCtx->Uri_Ana             = %s\n", ctx->Uri_Ana);
    zte_printf_s("ptSessionCtx->bCorrelationId_Exp  = %s\n", ctx->bCorrelationId_Exp);
    zte_printf_s("ptSessionCtx->bCorrelationId_Ana  = %s\n", ctx->bCorrelationId_Ana);
}
/* Ended by AICoder, pid:hafd832337r2f2f14d15090a206d9f2f01417d15 */

/* Started by AICoder, pid:c617ew6495ccfa71482609201054b3137ef2602f */
void printT_psNcuDaSubScribeCtx(T_psNcuDaSubScribeCtx *ctx) {
    zte_printf_s("ptSubScribeCtx->dwSubscribeCtxId  = %u\n", ctx->dwSubscribeCtxId);
    zte_printf_s("ptSubScribeCtx->dwAppId           = %u\n", ctx->dwAppId);
    zte_printf_s("ptSubScribeCtx->ddwUPSeid         = %llu\n", ctx->ddwUPSeid);
    zte_printf_s("ptSubScribeCtx->dwUpdateTimeStamp = %u\n", ctx->dwUpdateTimeStamp);
    zte_printf_s("ptSubScribeCtx->dwCreateTimeStamp = %u\n", ctx->dwCreateTimeStamp);
    zte_printf_s("ptSubScribeCtx->appidstr          = %s\n", ctx->appidstr);
    zte_printf_s("ptSubScribeCtx->bSubType          = %u\n", ctx->bSubType);
}
/* Ended by AICoder, pid:c617ew6495ccfa71482609201054b3137ef2602f */



WORD32 g_show_data = 0;
/* Started by AICoder, pid:t832113a2ff489514c590ae52069c9258a61852e */
void printT_NcuSessionInfo(void *info)
{
    if((-1 == g_ncu_show && 0 == g_show_data) || NULL == info)
    {
        return;
    }
    T_NcuSynSessInfo* ptSessionInfo = (T_NcuSynSessInfo*)info;
    zte_printf_s("\nptSessionInfo data ======>sizeof(T_NcuSynSessInfo)=%lu\n",sizeof(T_NcuSynSessInfo));
    zte_printf_s("ddwUPSeid: %llu\n", ptSessionInfo->ddwUPSeid);
    zte_printf_s("dwSessionID: %u\n", ptSessionInfo->dwSessionID);
    zte_printf_s("apnname: %s\n", ptSessionInfo->apnname);
    zte_printf_s("bRatType: %d\n", ptSessionInfo->bRatType);
    zte_printf_s("bMSIPtype: %d\n", ptSessionInfo->bMSIPtype);
    
    BYTE imsiStr[17] = {0};
    psUpfCommIMSIToString(ptSessionInfo->bImsi, imsiStr, 16);
    zte_printf_s(">bImsi = %s\n", imsiStr);
    BYTE isdnStr[17] = {0};
    psUpfCommIMSIToString(ptSessionInfo->bIsdn, isdnStr, 16);
    zte_printf_s("bIsdn  = %s\n", isdnStr);
    zte_printf_s("tUli_Flag: %d\n", ptSessionInfo->tUli_Flag);
    zte_printf_s("tMSIPv4  = %u.%u.%u.%u\n", ptSessionInfo->tMSIPv4[0],ptSessionInfo->tMSIPv4[1],ptSessionInfo->tMSIPv4[2],ptSessionInfo->tMSIPv4[3]);
    char ipv6str[64] = {0};
    inet_ntop(AF_INET6, ptSessionInfo->tMSIPv6, ipv6str, 64);
    zte_printf_s("tMSIPv6 = %s\n", ipv6str);
}
/* Ended by AICoder, pid:t832113a2ff489514c590ae52069c9258a61852e */
void printT_NcuSynSubInfo(void *info) 
{
    if((-1 == g_ncu_show && 0 == g_show_data)|| NULL == info)
    {
        return;
    }
    T_NcuSynSubInfo* ptNcuSynSubInfo = (T_NcuSynSubInfo*)info;
    int i;

    zte_printf_s("\nT_NcuSynSubInfo:data ======>sizeof(T_NcuSynSubInfo)=%lu\n",sizeof(T_NcuSynSubInfo));
    zte_printf_s("  wAppidNum: %u\n", ptNcuSynSubInfo->wAppidNum);
    for(i = 0; i < ptNcuSynSubInfo->wAppidNum; i++) {
        zte_printf_s("  appid[%d]: %u\n", i, ptNcuSynSubInfo->appid[i]);
    }
    zte_printf_s("  bIplistNum: %u\n", ptNcuSynSubInfo->bIplistNum);
    zte_printf_s("  NwdafIPv4: %u.%u.%u.%u\n", ptNcuSynSubInfo->NwdafIPv4_N4[0],ptNcuSynSubInfo->NwdafIPv4_N4[1],ptNcuSynSubInfo->NwdafIPv4_N4[2],ptNcuSynSubInfo->NwdafIPv4_N4[3]);
    zte_printf_s("  NwdafIPv6: %02x%02x:%02x%02x:%02x%02x:%02x%02x:\n", ptNcuSynSubInfo->NwdafIPv6_N4[0],ptNcuSynSubInfo->NwdafIPv6_N4[1],ptNcuSynSubInfo->NwdafIPv6_N4[2],ptNcuSynSubInfo->NwdafIPv6_N4[3],
    ptNcuSynSubInfo->NwdafIPv6_N4[4],ptNcuSynSubInfo->NwdafIPv6_N4[5],ptNcuSynSubInfo->NwdafIPv6_N4[6],ptNcuSynSubInfo->NwdafIPv6_N4[7]);
 
    zte_printf_s("  bCorID_len: %u\n", ptNcuSynSubInfo->bCorID_len);
    zte_printf_s("  bReportUri_len: %u\n", ptNcuSynSubInfo->bReportUri_len);
    zte_printf_s("  aucReportUri: %s\n", ptNcuSynSubInfo->aucReportUri);
    zte_printf_s("  aucCorid: %s\n", ptNcuSynSubInfo->aucCorid);

}

/* Started by AICoder, pid:r4d99572ea2a5ae146210a631057671568332f82 */
void printT_NcuSynSubInfoHead(void *info)
{
    if((-1 == g_ncu_show && 0 == g_show_data) || NULL == info)
    {
        return;
    }
    T_NcuSynSubInfoHead* ptSubInfoHead = (T_NcuSynSubInfoHead*)info;
    zte_printf_s("T_NcuSynSubInfoHead data=====>sizeof(T_NcuSynSubInfoHead)=%lu\n",sizeof(T_NcuSynSubInfoHead));
    zte_printf_s("appid: %u\n", ptSubInfoHead->appid);
    zte_printf_s("bType: %d\n", ptSubInfoHead->bType);
    zte_printf_s("bSubType: %d [0. 质差， 1. 全球通的体验  3. 普通体验]\n", ptSubInfoHead->bSubType);
    zte_printf_s("upseid: %llu\n", ptSubInfoHead->upseid);
}
/* Ended by AICoder, pid:r4d99572ea2a5ae146210a631057671568332f82 */
