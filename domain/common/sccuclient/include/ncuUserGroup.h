#ifndef NCUUSERGROUP_H
#define NCUUSERGROUP_H
#ifdef __cplusplus
extern "C"{
#endif

#include "tulip.h"
#include "ps_db_define_pfu.h"

#define USERGROUP_SUCC             0
#define USERGROUP_FAIL             1
#define USERGROUP_STRLEN_FAIL      2
#define USERGROUP_SENDRSP_FAIL     3

#define NCUROUP_STATE_DEFAULT             0
#define NCUROUP_STATE_MIGIN               1
#define NCUROUP_STATE_MIGOUT              2
extern BYTE g_ncuGroupState[MAX_UPF_GROUP_NUM];

struct T_UserGroupChgNotiStat
{
    WORD32 dwArgErrCnt;
    WORD32 dwAddCnt;
    WORD32 dwDelCnt;
    WORD32 dwUpdateCnt;

    WORD32 dwScChgArgErrCnt;
    WORD32 dwScChgAddCnt;
    WORD32 dwScChgDelCnt;
    WORD32 dwScChgUpdateCnt;
    WORD32 dwScChgCreatCnt;
    WORD32 dwScChgUpdateCntToSM;
    WORD32 dwScChgUpdateCntToMS;
    WORD32 dwScDiffWithUpmManageJob;
    WORD32 dwScDiffWithLdbSyncJob;
    
    WORD32 dwRecvMigrateMsgNum;
    WORD32 dwDecodeMigrateMsgErr;
    WORD32 dwHandleMigrateMsgWithoutLDB;
    WORD32 dwHandleMigrateMsgWithLDB;
    WORD32 dwEncodeMigrateMsgErr;
    WORD32 dwSendMigrateRspNum;
    WORD32 dwSendMigrateRspErr;

    WORD32 dwTblSyncNotFinishWhenMigIn;
    WORD32 dwWaitUgmTblSyncTimerCount;

    WORD32 dwScToJobMigInPreReq;
    WORD32 dwScToJobMigInDoReq;
    WORD32 dwScToJobMigInPostReq;
    WORD32 dwScToJobMigOutPreReq;
    WORD32 dwScToJobMigOutPostReq;
    WORD32 dwScToJobMigOutStatNoti;
    WORD32 dwScToJobMigInStatNoti;

    WORD32 dwPfuToSccuRouteUpdReq;
    WORD32 dwPfuToSccuRouteUpdSendErr;
    WORD32 dwSccuToPfuRouteUpdAck;

    WORD32 dwSyncToMngPreMigInAck;
    WORD32 dwSyncToMngPreMigOutAck;
    WORD32 dwSyncToMngMigDoAck;
    WORD32 dwMangeJobSendSessDelByGroupMigFail;
    WORD32 dwMangeJobSendSessDelByGroupMigSucc;
    WORD32 dwMangeJobRcvSessDelByGroupMig;
};

typedef struct 
{
    WORD32 session;
    WORD32 commid_src;
    WORD32 commid_dst;
    WORD32 inst_src;
    WORD32 inst_dst;
    WORD32 usrgroup;
}T_GroupMigrateReq;

typedef struct 
{
    WORD32 session;
    WORD32 result;    
}T_GroupMigrateRsp;

void ncuHandleUsrGroupMigrateProc(WORD32 msgId, const BYTE* msg, WORD32 msgLen);
VOID sendGroupMigrateOutReqToMediaThread();
BYTE getNcuGroupStateByGroupNo(WORD32 dwGroupNo);

#ifdef __cplusplus
}
#endif

#endif
