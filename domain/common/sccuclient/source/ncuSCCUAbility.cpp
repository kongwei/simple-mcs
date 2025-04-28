#include "ncuSCCUAbility.h"
#include "zte_slibc.h"
#include "psUpfSCTypes.h"
#include "upfSc.h"
#include "psUpfPubSub.h"
#include <set>
#include "ps_db_define_pfu.h"
LocalSC  g_sc_ncu;
RemoteSC g_sc_upm(SC_NAME_PFCP_MANAGMENT,"sc_cluster", "usergroup", "Nupf_PacketForward");
RemoteSC g_sc_pfu(SC_NAME_PACKETFORWARD,"sc_cluster", "usergroup", "Nupf_PacketForward");
RemoteSC g_http_lb("sc-http-lb", "http2_proxy", NULL, "CommonS_HTTP_LB");
extern "C"{
WORD32 ncuGetGroupCommId(WORD32 groupId)
{
    return g_sc_ncu.getGroupCommId(groupId);
}
WORD32 ncuGetGroupSCLogicNo(WORD32 groupId)
{
    return g_sc_ncu.getGroupSCLogicNo(groupId);
}
WORD32 ncuGetSelfCommId()
{
    return g_sc_ncu.getSelfCommId();
}

WORD32 pfuGetGroupCommId(WORD32 groupId)
{
    return g_sc_pfu.getGroupCommId(groupId);
}

WORD32 pfuGetGroupSCLogicNo(WORD32 groupId)
{
    return g_sc_pfu.getGroupSCLogicNo(groupId);
}
WORD32 ncuGetUpmCommIdByLogicNo(void)
{
    return g_sc_upm.getGroupCommId(0);
}

WORD32 ncuGetScLogicNo()
{
    return g_sc_ncu.getSelfLogicNo();
}

WORD32 ncuGetMaxScLogicNo()
{
    return g_sc_ncu.getMaxLogicNo();
}

WORD32 ncuGetCommIdByLogicNo(WORD32 LogicNo)
{
    return g_sc_ncu.getCommIdByLogicNo(LogicNo);

}

WORD32 ncuGetHttpLbCommIdByLogicNo(WORD32 dwLogicNo)
{
    return g_http_lb.getCommIdByLogicNo(dwLogicNo);
}

void ncushowUpm()
{
    WORD32 dwNum = g_sc_upm.getGroupMaxNum();
    zte_printf_s("ug_sc_ump.getGroupMaxNum():%u\n",dwNum);
    for(WORD32 i=0;i<=dwNum;i++)
    {
        zte_printf_s("upm group commid:0x%08x\n",g_sc_upm.getGroupCommId(i));
    }
}

void ncushowCommidByGroup()
{

    for(WORD32 i=0;i<=256;i++)
    {
        zte_printf_s("ncu group commid:0x%08x\n",g_sc_ncu.getGroupCommId(i));
    }
}

WORD32 ncuSendMsgToUpm(BYTE *msg, WORD16 wBufferLen, WORD32 msgID, WORD16 wJobType)
{

    return upfSendMsgToJob(msg, wBufferLen, msgID, g_sc_upm.getGroupCommId(0), wJobType);
}

WORD32 ncuGetSelfLogicNo()
{
    return g_sc_ncu.getSelfLogicNo();
}
WORD32 ncuGetMasterScNum()
{
    WORD32 dwTotalSCNum = g_sc_ncu.getMaxLogicNo();
    std::set<int> masterSCList;

    WORD            wLoop;
    T_UserGroupInfo tGrpInfo;

    for (wLoop=0; wLoop<MAX_UPF_GROUP_NUM && masterSCList.size()  < dwTotalSCNum; wLoop++)
    {
        if (SCCU_OK != sccuUserGroupGetSelfValueById(wLoop, &tGrpInfo))
        {
            continue;
        }

        masterSCList.insert(tGrpInfo.dwSCLogicNo);
    }
     return masterSCList.size();
}

BOOL ncuIsSelfGroup(WORD32 groupId)
{
    if(groupId >= UPF_USERGROUP_MAXNUM)
    {
        return FALSE;
    }
    WORD32 logicNo = ncuGetGroupSCLogicNo(groupId);
    WORD32 selfLogicNo = ncuGetSelfLogicNo();
    if(INVALID_LOGIC_NO == logicNo || INVALID_LOGIC_NO == selfLogicNo)
    {
        return FALSE;
    }
    return logicNo == selfLogicNo ? TRUE : FALSE;
}

}
