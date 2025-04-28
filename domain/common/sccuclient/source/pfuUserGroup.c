/* Started by AICoder, pid:d263cpfc60i5761144880ac1d026845d27205f91 */
/******************************************************************************
 * 版权所有 (C)2016 深圳市中兴通讯股份有限公司*
 * 模块名          : MCS
 * 文件名          : pfuUserGroup.c
 * 相关文件        :
 * 文件实现功能     : pfu群信息
 * 归属团队        : M6
 * 版本           : V1.0
-------------------------------------------------------------------------------
* 修改记录:
* 修改日期            版本号           修改人           修改内容
* 2024-04-14        V7.24.20        wya            create
******************************************************************************/
/**************************************************************************
 *                              头文件(满足最小依赖请按照DDD分层架构逐层依赖)
 **************************************************************************/
#include "pfuUserGroup.h"
#include "sccu_pub_define.h"
#include "ncuSCCUAbility.h"
#include "ps_db_define_pfu.h"
#include "psNcuCtrlStatInterface.h"
#include "psNcuHeartBeatProc.h"
#include "UPFHelp.h"
#include "sccu_pub_api.h"
/**************************************************************************
 *                              宏(本源文件使用)
 **************************************************************************/

/**************************************************************************
 *                              常量(本源文件使用)
 **************************************************************************/

/**************************************************************************
 *                              数据类型(本源文件使用)
 **************************************************************************/
typedef struct tag_T_PfuGroupInfo
{
    WORD32 commid;
    WORD32 logicNo;
}T_PfuGroupInfo;

T_PfuGroupInfo g_pfuGroupCommid[UPF_USERGROUP_MAXNUM] = {0};
/**************************************************************************
 *                              外部函数原型(评估后慎重添加)
 **************************************************************************/

/**************************************************************************
 *                              局部函数原型(本源文件使用)
 **************************************************************************/
void loadPfuAllGroupInfo();
VOID ncuSetPfuCommidByGroup(WORD32 group, WORD32 commid);
VOID ncuSetPfuLogicNoByGroup(WORD32 group, WORD32 logicNo);
VOID pfuUserGroupChgNotifyProc(T_SCCUUserGroupChgNotifyInfo *pInfo);
/**************************************************************************
 *                              全局变量(本源文件使用)
 **************************************************************************/

/**************************************************************************
 *                              函数实现(函数布局:总分结构|深度优先)
 **************************************************************************/
BOOL regPfuUserGroupChg()
{
    zte_memset_s(g_pfuGroupCommid, sizeof(g_pfuGroupCommid), 0, sizeof(g_pfuGroupCommid));
    loadPfuAllGroupInfo();

    T_ServiceAbilityInfo abilityInfo = {{0}, {0}, 0xff, {0}, {0}};
    zte_strncpy_s(abilityInfo.tServiceTypeName, SERVICETYPE_NAME_LENGTH, "Nupf_PacketForward", SERVICETYPE_NAME_LENGTH-1);
    zte_memcpy_s(abilityInfo.tScType, SCTYPE_NAME_LENGTH, "sc-upf-pf", SCTYPE_NAME_LENGTH-1);
    zte_memcpy_s(abilityInfo.serviceAbility, ABILITY_NAME_LENGTH, "usergroup", ABILITY_NAME_LENGTH-1);
    T_ABLInfo groupTbl = {0};

    if(SCCU_OK != sccuGetAbilityTableInfo(&abilityInfo, &groupTbl))
    {
        JOB_LOC_STAT_ADDONE(qwPfuUserGroupGetAbilityTblFail);
        return FALSE;
    }

    if(SCCU_OK != sccuRegUserGroupChgNotify(&groupTbl, pfuUserGroupChgNotifyProc, USRGROUP_NOTIFY_LOC))
    {
        JOB_LOC_STAT_ADDONE(qwPfuUserGroupChgNotifyRegCBFail);
        return FALSE;
    }

    JOB_LOC_STAT_ADDONE(qwPfuUserGroupChgNotifyRegSucc);
    return TRUE;
}

VOID pfuUserGroupChgNotifyProc(T_SCCUUserGroupChgNotifyInfo *pInfo)
{
    if((NULL == pInfo) || (pInfo->dwUserGroupNo >= UPF_USERGROUP_MAXNUM))
    {
        JOB_LOC_STAT_ADDONE(qwPfuUserGroupChgNotifyErr);
        return;
    }

    switch(pInfo->bOpType)
    {
        case USERGROUP_CHG_TYPE_TABLE_CREATE:
        {
            loadPfuAllGroupInfo();
            JOB_LOC_STAT_ADDONE(qwPfuUserGroupChgNotifyCrt);
            break;
        }
        case USERGROUP_CHG_TYPE_ADD:
        {
            ncuSetPfuCommidByGroup(pInfo->dwUserGroupNo, pInfo->tNewRecord.dwCommId);
            ncuSetPfuLogicNoByGroup(pInfo->dwUserGroupNo, pInfo->tNewRecord.dwSCLogicNo);
            JOB_LOC_STAT_ADDONE(qwPfuUserGroupChgNotifyAdd);
            break;
        }
        case USERGROUP_CHG_TYPE_DEL:
        {
            ncuSetPfuCommidByGroup(pInfo->dwUserGroupNo, 0);
            ncuSetPfuLogicNoByGroup(pInfo->dwUserGroupNo, 0);
            JOB_LOC_STAT_ADDONE(qwPfuUserGroupChgNotifyDel);
            break;
        }
        case USERGROUP_CHG_TYPE_UPDATE:
        {
            ncuSetPfuCommidByGroup(pInfo->dwUserGroupNo, pInfo->tNewRecord.dwCommId);
            ncuSetPfuLogicNoByGroup(pInfo->dwUserGroupNo, pInfo->tNewRecord.dwSCLogicNo);
            JOB_LOC_STAT_ADDONE(qwPfuUserGroupChgNotifyUpd);
            break;
        }
        default:
        {
            JOB_LOC_STAT_ADDONE(qwPfuUserGroupChgNotifyDefault);
            break;
        }
    }
}

void loadPfuAllGroupInfo()
{
    WORD32 i = 0;
    for(i = 0; i < UPF_USERGROUP_MAXNUM; i++)
    {
        ncuSetPfuCommidByGroup(i, pfuGetGroupCommId(i));
        ncuSetPfuLogicNoByGroup(i, pfuGetGroupSCLogicNo(i));
    }
}

WORD32 ncuGetPfuCommidByGroup(WORD32 group)
{
    if (group >= UPF_USERGROUP_MAXNUM)
    {
        return INVALID_COMM_ID;
    }
    return g_pfuGroupCommid[group].commid;
}

WORD32 ncuGetPfuLogicNoByGroup(WORD32 group)
{
    if (group >= UPF_USERGROUP_MAXNUM)
    {
        return INVALID_LOGIC_NO;
    }
    return g_pfuGroupCommid[group].logicNo;
}

VOID ncuSetPfuCommidByGroup(WORD32 group, WORD32 commid)
{
    if (group >= UPF_USERGROUP_MAXNUM)
    {
        return ;
    }
    g_pfuGroupCommid[group].commid = commid;
}

VOID ncuSetPfuLogicNoByGroup(WORD32 group, WORD32 logicNo)
{
    if (group >= UPF_USERGROUP_MAXNUM)
    {
        return ;
    }
    g_pfuGroupCommid[group].logicNo = logicNo;
}

UPF_HELP_REG("pfu", "show pfu group commid logicNo", 
VOID showPfuGroupCommidMap())
{
    WORD32 i = 0;
    for(i = 0; i < UPF_USERGROUP_MAXNUM; i++)
    {
        zte_printf_s("group : %u, commid : 0x%x, logicNo : %u\n", i, ncuGetPfuCommidByGroup(i), ncuGetPfuLogicNoByGroup(i));
    }
}

UPF_HELP_REG("pfu", "clear pfu group commid logicNo", 
VOID clearPfuGroupCommidMap())
{
    WORD32 i = 0;
    for (i = 0; i < UPF_USERGROUP_MAXNUM; i++)
    {
        ncuSetPfuCommidByGroup(i, INVALID_COMM_ID);
        ncuSetPfuLogicNoByGroup(i, INVALID_LOGIC_NO);
    }
}

/* Ended by AICoder, pid:d263cpfc60i5761144880ac1d026845d27205f91 */
