#include "ncuUserGroup.h"
#include "psMcsDebug.h"
#include "zte_slibc.h"
#include "json/json.h"
#include "vnfp_event_seg.h"
#include "sccu_pub_define.h"
#include "upfSc.h"
#include "ncuSCCUAbility.h"
#include "psUpfEvent.h"
#include "psUpfJobTypes.h"
#include "ps_ncu_session.h"
#include "ps_ncu_typedef.h"
#include "psUpfCommon.h"
#include "MemShareCfg.h"
#include "psUpfPubSub.h"

extern "C" {
    #include "xdb_core_pfu_que.h"
}

T_UserGroupChgNotiStat g_tPfuUserGroupChgNotiStat;
BYTE g_ncuGroupState[MAX_UPF_GROUP_NUM] = {0};

VOID ncuHandleGroupMigrateOutDelSess(WORD32 dwGroupNo);

WORD32 decodeGroupMigrateReq(T_GroupMigrateReq* req, const BYTE* buff, WORD32 buffLen)
{
    if(NULL == req || NULL == buff || 0 == buffLen)
    {
        return 1;
    }
    std::string str;
    str.assign((const char*)buff, buffLen);
    DEBUG_TRACE(DEBUG_LOW, "request msg content: %s", str.c_str());

    Json::Value root;
    Json::Reader reader;
    if(!reader.parse(str, root))
    {
        DEBUG_TRACE(DEBUG_LOW, "invalid json content: %s", str.c_str());
        return 2;
    }

    if(root.isMember("session"))
    {
        req->session = root["session"].asUInt();
    }
    if(root.isMember("commid_src"))
    {
        req->commid_src = root["commid_src"].asUInt();
    }
    if(root.isMember("commid_dst"))
    {
        req->commid_dst = root["commid_dst"].asUInt();
    }
    if(root.isMember("inst_src"))
    {
        req->inst_src = root["inst_src"].asUInt();
    }
    if(root.isMember("inst_dst"))
    {
        req->inst_dst = root["inst_dst"].asUInt();
    }
    if(root.isMember("usrgroup") && root["usrgroup"].isArray() && root["usrgroup"].size() > 0)
    {
        req->usrgroup = root["usrgroup"][0].asUInt();
    }
    return 0;
}

JobID g_tManageJobJid = {0};
JobID g_tSccuServerJID = {0};
bool g_bJIdRetrieved = false;

int retrieveJIDs()
{
    if(g_bJIdRetrieved)
    {
        return 0;
    }

    XOS_STATUS ret = XOS_SUCCESS;

    ret = XOS_GetSelfJID(&g_tManageJobJid);
    if(XOS_SUCCESS != ret)
    {
        DEBUG_TRACE(DEBUG_ERR, "XOS_GetSelfJID failed, ret = %u", ret);
        return 1;
    }
    ret = XOS_Sender(&g_tSccuServerJID);
    if(XOS_SUCCESS != ret)
    {
        DEBUG_TRACE(DEBUG_ERR, "XOS_Sender failed, ret = %u", ret);
        return 2;
    }
    g_bJIdRetrieved = true;
    return 0;
}


void handleGroupMigrate(WORD32 msgId, T_GroupMigrateReq& req, WORD32& rspMsgId)
{
    switch(msgId)
    {
        case EV_USRGROUP_SCTOXJOB_MIGIN_PRE_REQ:
        {
            DEBUG_TRACE(DEBUG_LOW,"receive EV_USRGROUP_SCTOXJOB_MIGIN_PRE_REQ, session=%u, group=%u", req.session, req.usrgroup);
            g_tPfuUserGroupChgNotiStat.dwScToJobMigInPreReq++;
            rspMsgId = EV_USRGROUP_XJOBTOSC_MIGIN_PRE_RSP;
            break;
        }
        case EV_USRGROUP_SCTOXJOB_MIGIN_DO_REQ:
        {
            DEBUG_TRACE(DEBUG_LOW,"receive EV_USRGROUP_SCTOXJOB_MIGIN_DO_REQ, session=%u, group=%u", req.session, req.usrgroup);
            g_tPfuUserGroupChgNotiStat.dwScToJobMigInDoReq++;
            rspMsgId = EV_USRGROUP_XJOBTOSC_MIGIN_DO_RSP;
            break;
        }
        case EV_USRGROUP_SCTOXJOB_MIGIN_POST_REQ:
        {
            DEBUG_TRACE(DEBUG_LOW,"receive EV_USRGROUP_SCTOXJOB_MIGIN_POST_REQ, session=%u, group=%u", req.session, req.usrgroup);
            g_tPfuUserGroupChgNotiStat.dwScToJobMigInPostReq++;
            rspMsgId = EV_USRGROUP_XJOBTOSC_MIGIN_POST_RSP;
            break;
        }
        case EV_USRGROUP_SCTOXJOB_MIGOUT_PRE_REQ:
        {
            DEBUG_TRACE(DEBUG_LOW,"receive EV_USRGROUP_SCTOXJOB_MIGOUT_PRE_REQ, session=%u, group=%u", req.session, req.usrgroup);
            g_tPfuUserGroupChgNotiStat.dwScToJobMigOutPreReq++;
            rspMsgId = EV_USRGROUP_XJOBTOSC_MIGOUT_PRE_RSP;
            break;
        }
        case EV_USRGROUP_SCTOXJOB_MIGOUT_POST_REQ:
        {
            DEBUG_TRACE(DEBUG_LOW,"receive EV_USRGROUP_SCTOXJOB_MIGOUT_POST_REQ, session=%u, group=%u", req.session, req.usrgroup);
            g_tPfuUserGroupChgNotiStat.dwScToJobMigOutPostReq++;
            rspMsgId = EV_USRGROUP_XJOBTOSC_MIGOUT_POST_RSP;
            break;
        }
        case EV_USRGROUP_SCTOXJOB_MIGOUT_STAT_NOTIFY:
        {
            DEBUG_TRACE(DEBUG_LOW,"receive EV_USRGROUP_SCTOXJOB_MIGOUT_STAT_NOTIFY, session=%u, group=%u", req.session, req.usrgroup);
            g_tPfuUserGroupChgNotiStat.dwScToJobMigOutStatNoti++;
            ncuHandleGroupMigrateOutDelSess(req.usrgroup);
            g_ncuGroupState[req.usrgroup] = NCUROUP_STATE_MIGOUT;
            break;
        }
        case EV_USRGROUP_SCTOXJOB_MIGIN_STAT_NOTIFY:
        {
            DEBUG_TRACE(DEBUG_LOW,"receive EV_USRGROUP_SCTOXJOB_MIGIN_STAT_NOTIFY, session=%u, group=%u", req.session, req.usrgroup);
            g_tPfuUserGroupChgNotiStat.dwScToJobMigInStatNoti++;
            g_ncuGroupState[req.usrgroup] = NCUROUP_STATE_MIGIN;
            if(0 == req.usrgroup)
            {
                WORD32  dwRet  = 0;
                dwRet = ncuSendMsgToUpm((BYTE *)&req, sizeof(T_GroupMigrateReq),EV_MSG_NWDAF_NCU_TO_UPM_NWDAFINFO, JOB_TYPE_MCS_MANAGE);
                if(UPF_RET_SUCCESS != dwRet) 
                {
                    DEBUG_TRACE(DEBUG_LOW,"receive EV_USRGROUP_SCTOXJOB_MIGIN_STAT_NOTIFY, send EV_MSG_NWDAF_NCU_TO_UPM_NWDAFINFO fail ");
                    break;
                }
                DEBUG_TRACE(DEBUG_LOW,"receive EV_USRGROUP_SCTOXJOB_MIGIN_STAT_NOTIFY, send EV_MSG_NWDAF_NCU_TO_UPM_NWDAFINFO success ");
            }

            break;
        }
        default:
        {
            DEBUG_TRACE(DEBUG_ERR,"receive msg false, session=%u, group=%u", req.session, req.usrgroup);
            break;
        }
    }
    return;
}

/* Started by AICoder, pid:z8381yc31cld6fc14b9509281074233b82b0aea1 */
WORD32 encodeGroupMigrateRsp(const T_GroupMigrateRsp* rsp, BYTE* buff, WORD32 buffLen, WORD32* resultLen)
{
    if(rsp == nullptr || buff == nullptr || buffLen == 0 || resultLen == nullptr)
    {
        return USERGROUP_FAIL;
    }

    Json::Value root;
    root["session"] = rsp->session;
    root["result"] = rsp->result;

    Json::FastWriter writer;
    std::string str = writer.write(root);

    WORD32 strLen = str.length();

    if(strLen + 1 > buffLen)
    {
        return USERGROUP_STRLEN_FAIL;
    }

    zte_memcpy_s(buff, 127, str.c_str(), strLen);
    buff[strLen] = '\0'; // Ensure null terminated string

    *resultLen = strLen + 1;

    DEBUG_TRACE(DEBUG_LOW, "response msg content: %s", buff);

    return USERGROUP_SUCC;
}
/* Ended by AICoder, pid:z8381yc31cld6fc14b9509281074233b82b0aea1 */

WORD32 sendGroupMigrateRsp(WORD32 msgId, BYTE* msg, WORD16 msgLen)
{
    XOS_STATUS ret = XOS_SUCCESS;

    ret = XOS_SendAsynMsg(msgId, msg, msgLen, XOS_MSG_VER0, (VOID*)&g_tSccuServerJID, &g_tManageJobJid);
    if(XOS_SUCCESS != ret)
    {
        DEBUG_TRACE(DEBUG_ERR, "XOS_SendAsynMsg failed: ret = %u, destJno: 0x%08x, destCommId: 0x%08x", ret, g_tSccuServerJID.dwJno, g_tSccuServerJID.dwComId);
        return USERGROUP_SENDRSP_FAIL;
    }

    return USERGROUP_SUCC;
}

void ncuHandleUsrGroupMigrateProc(WORD32 msgId, const BYTE* msg, WORD32 msgLen)
{
    g_tPfuUserGroupChgNotiStat.dwRecvMigrateMsgNum++;
    
    DEBUG_TRACE(DEBUG_LOW, "ncuHandleUsrGroupMigrateProc enter");
    if ((NULL == msg) || (msgLen==0)){ 
        DEBUG_TRACE(DEBUG_ERR, "ncuHandleUsrGroupMigrateProc, input is NULL."); 
        return;
    }

    T_GroupMigrateReq req;
    zte_memset_s(&req, sizeof(T_GroupMigrateReq), 0 , sizeof(T_GroupMigrateReq));
    WORD32 ret = decodeGroupMigrateReq(&req, msg, msgLen);
    if(0 != ret) 
    {
        DEBUG_TRACE(DEBUG_ERR, "decode msg(msgId: %u, msgLen:%u) error, ret code = %u", msgId, msgLen, ret);
        g_tPfuUserGroupChgNotiStat.dwDecodeMigrateMsgErr++;
        return;
    }


    retrieveJIDs();

    WORD32 rspMsgId = 0;

    handleGroupMigrate(msgId, req, rspMsgId);

    if(0 == rspMsgId)
    {
        DEBUG_TRACE(DEBUG_ERR, "rspMsgId is zero");
        return;
    }

    T_GroupMigrateRsp rsp;
    zte_memset_s(&rsp, sizeof(T_GroupMigrateRsp), 0 , sizeof(T_GroupMigrateRsp));
    rsp.session = req.session;

    BYTE buff[128];
    WORD32 rspMsgLen = 0;
    ret = encodeGroupMigrateRsp(&rsp, buff, sizeof(buff), &rspMsgLen);
    if(0 != ret)
    {
        DEBUG_TRACE(DEBUG_ERR, "encode msg error, ret code = %u", ret);
        g_tPfuUserGroupChgNotiStat.dwEncodeMigrateMsgErr++;
        return;
    }

    ret = sendGroupMigrateRsp(rspMsgId, buff, rspMsgLen);
    if(0 != ret)
    {
        DEBUG_TRACE(DEBUG_ERR, "sendGroupMigrateRsp error, ret code = %u", ret);
        g_tPfuUserGroupChgNotiStat.dwSendMigrateRspErr++;
        return;
    }

    DEBUG_TRACE(DEBUG_LOW, "send group migrate response success, msgId=%u, session=%u, result=%u", rspMsgId, rsp.session, rsp.result);
    g_tPfuUserGroupChgNotiStat.dwSendMigrateRspNum++;
    return;

}
/* Started by AICoder, pid:967fduaa642144e1431b0ad960b06e7875139c85 */
void ncuHandleGroupMigrateOutDelSess(WORD32 dwGroupNo)
{
    MCS_CHK_VOID(dwGroupNo >= MAX_UPF_GROUP_NUM); 
    DEBUG_TRACE(DEBUG_LOW, "Rcv migrate-out msg dwGroupNo=%u", dwGroupNo);
    
    T_PSThreadInstAffinityPara *ptVpfuMcsThreadPara = &(g_ptVpfuShMemVar->tVpfuMcsThreadPara);
    WORD32 bInstNum = ptVpfuMcsThreadPara->bInstNum;

    T_psMcsSessDelByGroupArea *ptSessDelByGroup = NULL;
    WORD32 bMcsThr = 0;
    LPT_PfuQueHeadReg pQueHead = NULL;
    
    WORD32 bIndex = 0;
    for(bIndex = 0; bIndex < MAX_PS_THREAD_INST_NUM && bIndex < bInstNum; bIndex++)
    {
        bMcsThr = (ptVpfuMcsThreadPara->atThreadParaList[bIndex].bSoftThreadNo) % MEDIA_THRD_NUM;
        pQueHead = _db_getGroupQueHeadAdrr(_PFU_GET_DBHANDLE(bMcsThr), dwGroupNo);
        MCS_CHK_CONTINUE(NULL == pQueHead);
        
        ptSessDelByGroup = &(g_ptVpfuShMemVar->tSessDelByGroupArea[bMcsThr][dwGroupNo]);
        ptSessDelByGroup->dwDelMsgSendTimes = _xdb_pfu_QueGetLength(pQueHead)/UPF_MAX_SESSDEL_NUM + 1;
        ptSessDelByGroup->dwSesstionDelTime = psFtmGetPowerOnSec();
        ptSessDelByGroup->dwGroupId         = dwGroupNo;
    }
    return;
}
/* Ended by AICoder, pid:967fduaa642144e1431b0ad960b06e7875139c85 */

VOID sendGroupMigrateOutReqToMediaThread()
{
    T_psMcsSessDelByGroupArea *ptSessDelByGroup = NULL;
    WORD32 dwGroupNo = 0;
    BYTE   bMcsThr   = 0;
    BYTE   bIndex    = 0;

    T_PSThreadInstAffinityPara *ptVpfuMcsThreadPara = &(g_ptVpfuShMemVar->tVpfuMcsThreadPara);
    WORD32 bInstNum = ptVpfuMcsThreadPara->bInstNum;
    for(bIndex = 0; bIndex < MAX_PS_THREAD_INST_NUM && bIndex < bInstNum; bIndex++)
    {
        for(dwGroupNo = 0; dwGroupNo < MAX_UPF_GROUP_NUM; dwGroupNo++)
        {
            bMcsThr = (ptVpfuMcsThreadPara->atThreadParaList[bIndex].bSoftThreadNo) % MEDIA_THRD_NUM;
            ptSessDelByGroup = &(g_ptVpfuShMemVar->tSessDelByGroupArea[bMcsThr][dwGroupNo]);
            MCS_CHK_CONTINUE_LIKELY(0 == ptSessDelByGroup->dwDelMsgSendTimes);

            if(MCS_RET_SUCCESS != upfSendMsgToMcsByUcom((BYTE*)ptSessDelByGroup, EV_MSG_MANAGE_TO_MEDIA_POST_MIG_OUT_REQ,
                                                        sizeof(T_psMcsSessDelByGroupArea), bMcsThr))
            {
            }
            (ptSessDelByGroup->dwDelMsgSendTimes)--;
            if(0 == ptSessDelByGroup->dwDelMsgSendTimes)
            {
                zte_memset_s(ptSessDelByGroup, sizeof(T_psMcsSessDelByGroupArea), 0, sizeof(T_psMcsSessDelByGroupArea));
            }
        }
    }
    return;
}

void pfushowgroupstat()
{
    zte_printf_s("----------------Group Migrate----------------\n");
    zte_printf_s("dwRecvMigrateMsgNum=%u\n", g_tPfuUserGroupChgNotiStat.dwRecvMigrateMsgNum);
    zte_printf_s("dwDecodeMigrateMsgErr=%u\n", g_tPfuUserGroupChgNotiStat.dwDecodeMigrateMsgErr);
    zte_printf_s("dwEncodeMigrateMsgErr=%u\n", g_tPfuUserGroupChgNotiStat.dwEncodeMigrateMsgErr);
    zte_printf_s("dwSendMigrateRspNum=%u\n", g_tPfuUserGroupChgNotiStat.dwSendMigrateRspNum);
    zte_printf_s("dwSendMigrateRspErr=%u\n", g_tPfuUserGroupChgNotiStat.dwSendMigrateRspErr);
    zte_printf_s("\n");
    zte_printf_s("dwScToJobMigInPreReq=%u\n", g_tPfuUserGroupChgNotiStat.dwScToJobMigInPreReq);
    zte_printf_s("dwScToJobMigInDoReq=%u\n", g_tPfuUserGroupChgNotiStat.dwScToJobMigInDoReq);
    zte_printf_s("dwScToJobMigInPostReq=%u\n", g_tPfuUserGroupChgNotiStat.dwScToJobMigInPostReq);
    zte_printf_s("dwScToJobMigOutPreReq=%u\n", g_tPfuUserGroupChgNotiStat.dwScToJobMigOutPreReq);
    zte_printf_s("dwScToJobMigOutPostReq=%u\n", g_tPfuUserGroupChgNotiStat.dwScToJobMigOutPostReq);
    zte_printf_s("dwScToJobMigOutStatNoti=%u\n", g_tPfuUserGroupChgNotiStat.dwScToJobMigOutStatNoti);
    zte_printf_s("dwScToJobMigInStatNoti=%u\n", g_tPfuUserGroupChgNotiStat.dwScToJobMigInStatNoti);

}

void pfuresetgroupstat()
{
    zte_memset_s(&g_tPfuUserGroupChgNotiStat, sizeof(g_tPfuUserGroupChgNotiStat), 0 , sizeof(g_tPfuUserGroupChgNotiStat));
}

extern "C" {
    BYTE getNcuGroupStateByGroupNo(WORD32 dwGroupNo)
    {
        if(dwGroupNo >= MAX_UPF_GROUP_NUM)
        {
            return NCUROUP_STATE_DEFAULT;
        }

        return g_ncuGroupState[dwGroupNo];
    }
}
