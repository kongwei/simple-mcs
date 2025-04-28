#ifndef PS_NEF_REPORT_TO_NWDAF_H_
#define PS_NEF_REPORT_TO_NWDAF_H_
#include "ps_ncu_dataApp.h"
#include "psUpfCommon.h"

#ifdef __cplusplus
extern "C" {
#endif
#include "tulip.h"
#include "ps_ncu_httplink.h"
#include "ps_ncu_session.h"

#define DAF_HTTP_METHOD_LEN         10
#define DAF_HTTP_URL_LEN            385
#define DAF_HTTP_SESSIONID_LEN      32
#define DAF_HTTP_ACCEPT_LEN         32
#define DAF_HTTP_CONTENTTYPE_LEN    32
#define DAF_HTTP_USERID_LEN         32
#define DAF_HTTP_MSG_DATA_LEN       1024*10
#define DAF_SERVICEABILITY_LEN      32

typedef struct
{
    WORD32 serviceTypeID;   //服务类型标识
    WORD32 serviceInst;     //服务实例
    WORD32 scTypeID;         //SC type
    WORD32 userGroupId;     //用户分组
    BYTE   serviceAbility[DAF_SERVICEABILITY_LEN];  //服务能力id
    JID    jobID;
}tDafHTTP2AppInfo;

typedef struct
{
    BYTE   version;
    BYTE   pad[5];
    BYTE   method[DAF_HTTP_METHOD_LEN];
    BYTE   url[DAF_HTTP_URL_LEN];
    BYTE   sessionID[DAF_HTTP_SESSIONID_LEN];
    BYTE   accept[DAF_HTTP_ACCEPT_LEN];
    BYTE   contentType[DAF_HTTP_CONTENTTYPE_LEN];
    BYTE   userID[DAF_HTTP_USERID_LEN];
    tDafHTTP2AppInfo appInfo;
    WORD32 profileID;
    bool   traced;
    WORD32 dwMsgLen;
    BYTE   MsgData[DAF_HTTP_MSG_DATA_LEN];
}tDafReqHTTP2APP;

WORD32 handleDataAnalyticsNcuToUpmReq(BYTE * pMsgBody, T_psNcuDaAppCtx * ptDaAppCtx);
VOID psNcuTipcSBIPktProc(T_MediaProcThreadPara *ptMediaProcThreadPara);
bool psDafApp2HTTPMsgReq(tDafReqHTTP2APP* pHTTP2APPReq, T_psNcuSessionCtx* ptSessionCtx, void *ptNcuHttpLinkCtx);
#ifdef __cplusplus
}
#endif


#endif
