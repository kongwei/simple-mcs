#include "psNcuCtrlStatInterface.h"
#include "httpLbScInfo.h"
#include "sccu_pub_api.h"
#include "ncuSCCUAbility.h"
#include "UPFHelp.h"
#include "zte_slibc.h"

#ifndef HTTPLB_SC_LOGICNO
#define HTTPLB_SC_LOGICNO    16
#endif

WORD32 g_httpLbCommid[HTTPLB_SC_LOGICNO] = {0};
T_ABLInfo g_tHttpProxyAbility;

VOID psHttpLbSCListChgNotifyProc(T_SCCUSCListChgNotifyInfo *pInfo);

T_ABLInfo getHttpProxyAbility()
{
    return g_tHttpProxyAbility;
}

VOID psSetHttpLbCommidByLogicNo(WORD32 dwLogicNo, WORD32 dwCommid)
{
    if (dwLogicNo > HTTPLB_SC_LOGICNO || 0 == dwLogicNo)
    {
        return;
    }
    g_httpLbCommid[dwLogicNo-1] = dwCommid;
    return;
}

WORD32 psGetHttpLbCommidByLogicNo(WORD32 dwLogicNo)
{
    if (dwLogicNo > HTTPLB_SC_LOGICNO || 0 == dwLogicNo)
    {
        return INVALID_COMM_ID;
    }
    return g_httpLbCommid[dwLogicNo-1];
}

VOID psLoadHttpLbAllSCInfo()
{
    for(WORD32 i = 1; i <= HTTPLB_SC_LOGICNO; i++)
    {
        psSetHttpLbCommidByLogicNo(i, ncuGetHttpLbCommIdByLogicNo(i));
    }
}

BOOL psRegHttpLbScInfoChg()
{
    psLoadHttpLbAllSCInfo();

    T_ServiceAbilityInfo tAbilityInfo = {0};
    tAbilityInfo.bServiceInst = (BYTE)(0xff);
    XOS_strncpy((char *)tAbilityInfo.serviceAbility, "http2_proxy", ABILITY_NAME_LENGTH);
    XOS_strncpy((char *)tAbilityInfo.tServiceTypeName, "CommonS_HTTP_LB", SERVICETYPE_NAME_LENGTH);
    XOS_strncpy((char *)tAbilityInfo.tScType, "sc-http-lb", SCTYPE_NAME_LENGTH);

    if(SCCU_OK != sccuGetAbilityTableInfo(&tAbilityInfo, &g_tHttpProxyAbility))
    {
        JOB_LOC_STAT_ADDONE(qwHttpLbGetAbilityTblFail);
        return FALSE;
    }
    if(SCCU_OK != sccuRegSCListChgNotify(&g_tHttpProxyAbility, psHttpLbSCListChgNotifyProc))
    {
        JOB_LOC_STAT_ADDONE(qwHttpLbSCListChgNotifyRegFail);
        return FALSE;
    }

    JOB_LOC_STAT_ADDONE(qwHttpLbSCListChgNotifyRegSucc);
    return TRUE;
}

VOID psHttpLbSCListChgNotifyProc(T_SCCUSCListChgNotifyInfo *pInfo)
{
    if (NULL == pInfo)
    {
        JOB_LOC_STAT_ADDONE(qwHttpLbSCListChgNotifyErr);
        return;
    }

    switch(pInfo->bOpType)
    {
        case SCLIST_CHG_TYPE_TABLE_CREATE:
        {
            psLoadHttpLbAllSCInfo();
            JOB_LOC_STAT_ADDONE(qwHttpLbSCListChgNotifyCrt);
            break;
        }
        case SCLIST_CHG_TYPE_ADD:
        {
            psSetHttpLbCommidByLogicNo(pInfo->SCLogicNo, pInfo->tNewSCRecord.dwCommID);
            JOB_LOC_STAT_ADDONE(qwHttpLbSCListChgNotifyAdd);
            break;
        }
        case SCLIST_CHG_TYPE_DEL:
        {
            psSetHttpLbCommidByLogicNo(pInfo->SCLogicNo, INVALID_COMM_ID);
            JOB_LOC_STAT_ADDONE(qwHttpLbSCListChgNotifyDel);
            break;
        }
        case SCLIST_CHG_TYPE_UPDATE:
        {
            psSetHttpLbCommidByLogicNo(pInfo->SCLogicNo, pInfo->tNewSCRecord.dwCommID);
            JOB_LOC_STAT_ADDONE(qwHttpLbSCListChgNotifyUpd);
            break;
        }
        default:
        {
            JOB_LOC_STAT_ADDONE(qwHttpLbSCListChgNotifyDefault);
            break;
        }
    }

    return;
}

UPF_HELP_REG("mcs", "show httplb logicNo commid", 
VOID psShowHttpLbLogicNoCommidMap())
{
    for(WORD32 i = 1; i <= HTTPLB_SC_LOGICNO; i++)
    {
        zte_printf_s("logicNo : %u, commid : 0x%x\n", i, psGetHttpLbCommidByLogicNo(i));
    }
    return;
}
