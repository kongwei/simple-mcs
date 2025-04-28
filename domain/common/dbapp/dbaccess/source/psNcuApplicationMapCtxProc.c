/* Started by AICoder, pid:p836ag713db2bdf14c2f0a2b9026aa95c221f684 */
/******************************************************************************
 * 版权所有 (C)2016 深圳市中兴通讯股份有限公司*
 * 模块名          : MCS
 * 文件名          : psNcuApplicationMapCtxProc.c
 * 相关文件        :
 * 文件实现功能     : APPLICATIONMAP上下文增删改查接口
 * 归属团队        : M6
 * 版本           : V7.24.20
 -------------------------------------------------------------------------------
 * 修改记录:
 * 修改日期           版本号           修改人           修改内容
 * 2024-09-20    V7.24.30              wya          create
******************************************************************************/
/**************************************************************************
 *                              头文件(满足最小依赖请按照DDD分层架构逐层依赖)
 **************************************************************************/
#include "psNcuApplicationMapCtxProc.h"
#define USE_DM_UPFCOMM_GETAPPLICATIONMAP_BY_INNERAPPID
#define USE_DM_UPFCOMM_GETAPPLICATIONMAP_BY_APPLICATION
#define USE_DM_UPFCOMM_GETALLAPPLICATIONMAP
#include "dbm_lib_upfcomm.h"
#include "dbmLibComm.h"
#include "applicationmap.h"
#include "r_ncu_applicationmap.h"
#include "ps_ncu_typedef.h"
#include "xdb_pfu_dyntbl_acc.h"
#include "xdb_pfu_com.h"
#include "ps_db_define_ncu.h"
#include "psMcsDebug.h"
#include "MemShareCfg.h"
#include "xdb_core_pfu.h"
#include "xdb_core_pfu_db.h"
/**************************************************************************
 *                              宏(本源文件使用)
 **************************************************************************/

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

/**************************************************************************
 *                              全局变量(本源文件使用)
 **************************************************************************/

/**************************************************************************
 *                              函数实现(函数布局:总分结构|深度优先)
 **************************************************************************/
DBBOOL psNcuGetCfgStrAppidMapByInnerId(WORD32 dwInnerAppId, CHAR *strAppID)
{
    MCS_CHK_RET(NULL == strAppID, FALSE);

    DM_UPFCOMM_GETAPPLICATIONMAP_BY_INNERAPPID_REQ req = {0};
    DM_UPFCOMM_GETAPPLICATIONMAP_BY_INNERAPPID_ACK ack = {0};
    BOOLEAN     bflag = FALSE;

    req.msgType = MSG_CALL;
    req.innerappid = dwInnerAppId;
    ack.retCODE = RC_DBM_ERROR;

    bflag = DBM_CALL((WORD32)DM_UPFCOMM_GETAPPLICATIONMAP_BY_INNERAPPID, (LPSTR)&req, (LPSTR)&ack);

    if(bflag && (RC_DBM_OK == ack.retCODE))
    {
        zte_memcpy_s(strAppID, LEN_APPLICATIONMAP_APPLICATION_MAX+1, ack.application, sizeof(ack.application));
        return TRUE;
    }

    return FALSE;
}

DBBOOL psNcuGetCfgInnerAppIdByStrAppidmap(CHAR *strAppID,WORD32 *pdwInnerAppId)
{
    MCS_CHK_RET(NULL == strAppID, FALSE);
    MCS_CHK_RET(NULL == pdwInnerAppId, FALSE);

    DM_UPFCOMM_GETAPPLICATIONMAP_BY_APPLICATION_REQ req = {0};
    DM_UPFCOMM_GETAPPLICATIONMAP_BY_APPLICATION_ACK ack = {0};
    BOOLEAN     bflag = FALSE;

    //调用现有接口查询
    req.msgType = MSG_CALL;
    zte_strncpy_s(req.application, LEN_APPLICATIONMAP_APPLICATION_MAX+1, strAppID, zte_strnlen_s(strAppID, LEN_APPLICATIONMAP_APPLICATION_MAX));
    ack.retCODE = RC_DBM_ERROR;

    bflag = DBM_CALL((WORD32)DM_UPFCOMM_GETAPPLICATIONMAP_BY_APPLICATION, (LPSTR)&req, (LPSTR)&ack);

    if(bflag && (RC_DBM_OK == ack.retCODE))
    {
        *pdwInnerAppId = ack.innerappid;
        return TRUE;
    }

    return FALSE;
}

WORD32 ReallocAppIdMapCtxByInnerID(WORD32 dwInnerAppId, CHAR *strAppID, BYTE **ptCtxAddr)
{
    /* 固定在媒体面1号线程上增加 */
    if(0 == psNcuMcsSelfThreadIsFirstSoftThread())
    {
        return 0;
    }

    /* 读取配置成功，则固定在媒体面第一个线程上重新生成动态上下文 */
    if (!psNcuGetCfgStrAppidMapByInnerId(dwInnerAppId, strAppID))
    {
        return 0;
    }

    return psNcuCreatAppIDMapCtxByStrAppID(strAppID, dwInnerAppId, ptCtxAddr);
}

WORD32 ReallocAppIdMapCtxByStrAppid(CHAR *strAppID,BYTE **ptCtxAddr)
{
    WORD32 dwInnerAppId = 0;
    /* 固定在媒体面1号线程上增加 */
    if(0 == psNcuMcsSelfThreadIsFirstSoftThread())
    {
        return 0;
    }

    /* 读取配置成功，则固定在媒体面第一个线程上重新生成动态上下文 */
    if (!psNcuGetCfgInnerAppIdByStrAppidmap(strAppID, &dwInnerAppId))
    {
        return 0;
    }

    return psNcuCreatAppIDMapCtxByStrAppID(strAppID, dwInnerAppId, ptCtxAddr);
}

WORD32 psNcuGetDynCtxInnerAppIDByStrAppID(CHAR *aucStrAppID)
{
    MCS_CHK_RET(NULL==aucStrAppID, INVALIDINNERAPPID);

    DM_PFU_QUERYDYNDATA_BYIDX_REQ tQueryReq = {0};
    DM_PFU_QUERYDYNDATA_BYIDX_ACK tQueryAck = {0};
    r_ncu_appidmap_tuple   *ptAppIdmapIndex = NULL;
    LPT_PfuTableReg         pTempTblRegAddr = NULL;

    BYTE ucKeyLen  = zte_strnlen_s(aucStrAppID, LEN_APPLICATIONMAP_APPLICATION_MAX);

    tQueryReq.hDB         = _NCU_DBHANDLE_COMM;
    tQueryReq.hTbl        = DB_HANDLE_R_NCU_APPIDMAP;
    tQueryReq.dwExpectNum = 1;
    tQueryReq.dataAreaKey.ucKeyFlag = XDB_KEYFLAG_UNIQUE;
    tQueryReq.dataAreaKey.hIdx      = DB_HANDLE_IDX_R_NCU_APPIDMAP_NAME;
    tQueryReq.dataAreaKey.ucKeyLen  = ucKeyLen;

    zte_memcpy_s(tQueryReq.dataAreaKey.aucKey, XDB_PFU_MAX_KEYVALUE_LEN, aucStrAppID, ucKeyLen);

    _pDM_PFU_QUERYDYNDATA_BYIDX(&tQueryReq,&tQueryAck);
    if ((RC_OK != tQueryAck.retCODE) || (NULL == tQueryAck.pDataAreaAddr))
    {
        tQueryAck.adwDataArea[0] = ReallocAppIdMapCtxByStrAppid(aucStrAppID, &tQueryAck.pDataAreaAddr);
        tQueryAck.retCODE = (0 == tQueryAck.adwDataArea[0]) ? RC_ERROR : RC_OK;
    }

    if((RC_OK == tQueryAck.retCODE)&&(NULL != tQueryAck.pDataAreaAddr))
    {
        pTempTblRegAddr = xdb_Pfu_Get_TableReg(tQueryReq.hDB, tQueryReq.hTbl);
        MCS_CHK_RET(NULL==pTempTblRegAddr, INVALIDINNERAPPID);
        ptAppIdmapIndex = (r_ncu_appidmap_tuple *)(tQueryAck.pDataAreaAddr - pTempTblRegAddr->wDataFieldOffset + PFU_TUPLEHEAD_SIZE);
        DEBUG_TRACE(DEBUG_LOW,"\n[Mcs]psNcuGetDynCtxInnerAppIDByStrAppID: query suc!! aucStrAppID = %s,innerappid = %u\n",aucStrAppID,ptAppIdmapIndex->innerappid);
        return ptAppIdmapIndex->innerappid;
    }
    else
    {
        DEBUG_TRACE(DEBUG_LOW,"\n[Mcs]psNcuGetDynCtxInnerAppIDByStrAppID: query failed!! aucStrAppID = %s\n",aucStrAppID);
        return INVALIDINNERAPPID;
    }
}

DBBOOL psNcuGetDynCtxStrAppIDByInnerId(WORD32 dwInnerAppID, CHAR *strAppID)
{
    MCS_CHK_RET(NULL == strAppID, FALSE);

    DM_PFU_QUERYDYNDATA_BYIDX_REQ tQueryReq = {0};
    DM_PFU_QUERYDYNDATA_BYIDX_ACK tQueryAck = {0};
    r_ncu_appidmap_tuple *ptAppIdmapIndex = NULL;
    LPT_PfuTableReg       pTempTblRegAddr = NULL;
    CHAR                  aucStrAppid[LEN_APPLICATIONMAP_APPLICATION_MAX + 1]   = {0};

    tQueryReq.hDB         = _NCU_DBHANDLE_COMM;
    tQueryReq.hTbl        = DB_HANDLE_R_NCU_APPIDMAP;
    tQueryReq.dwExpectNum = 1;
    tQueryReq.dataAreaKey.ucKeyFlag = XDB_KEYFLAG_UNIQUE;
    tQueryReq.dataAreaKey.hIdx      = DB_HANDLE_IDX_R_NCU_APPIDMAP_INNERID;
    tQueryReq.dataAreaKey.ucKeyLen  = sizeof(WORD32);
    *((WORD32 *)&tQueryReq.dataAreaKey.aucKey[0]) = dwInnerAppID;

    _pDM_PFU_QUERYDYNDATA_BYIDX(&tQueryReq,&tQueryAck);
    if ((RC_OK != tQueryAck.retCODE) || (NULL == tQueryAck.pDataAreaAddr))
    {
        tQueryAck.adwDataArea[0] = ReallocAppIdMapCtxByInnerID(dwInnerAppID, aucStrAppid, &tQueryAck.pDataAreaAddr);
        tQueryAck.retCODE = (0 == tQueryAck.adwDataArea[0]) ? RC_ERROR : RC_OK;
    }

    if((RC_OK == tQueryAck.retCODE)&&(NULL != tQueryAck.pDataAreaAddr))
    {
        pTempTblRegAddr = xdb_Pfu_Get_TableReg(tQueryReq.hDB, tQueryReq.hTbl);
        MCS_CHK_RET(NULL==pTempTblRegAddr, FALSE);
        ptAppIdmapIndex = (r_ncu_appidmap_tuple *)(tQueryAck.pDataAreaAddr - pTempTblRegAddr->wDataFieldOffset + PFU_TUPLEHEAD_SIZE);
        zte_memcpy_s(strAppID, LEN_APPLICATIONMAP_APPLICATION_MAX+1, ptAppIdmapIndex->application, sizeof(ptAppIdmapIndex->application));
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

void psNcuUpdateAppIDMapByInnerId(WORD32 dwTupleNo, WORD32 dwInnerID)
{
    DM_PFU_UPDATEDYNDATA_REQ    tReq = {0};
    DM_PFU_UPDATEDYNDATA_ACK    tAck = {0};

    tReq.hDB                   = _NCU_DBHANDLE_COMM;
    tReq.hTbl                  = DB_HANDLE_R_NCU_APPIDMAP;
    tReq.ucUpdateMethodFlag    = 1; //1：表示索引在要先删除，然后向指定记录插入索引
    tReq.dwDataAreaIndex       = dwTupleNo;
    tReq.dataAreaKey.ucKeyFlag = XDB_KEYFLAG_UNIQUE;
    tReq.dataAreaKey.hIdx      = DB_HANDLE_IDX_R_NCU_APPIDMAP_INNERID;
    *((WORD32 *)&tReq.dataAreaKey.aucKey[0]) = dwInnerID;
    tReq.dataAreaKey.ucKeyLen  = sizeof(WORD32);

    _pDM_PFU_UPDATEDYNDATA_BYUNIIDX(&tReq, &tAck);
    if(RC_OK != tAck.retCODE)
    {
        DEBUG_TRACE(DEBUG_LOW,"\n[Mcs]psNcuUpdateAppIDMapByInnerId: updata index failed!! dwTupleNo = %u,dwInnerID = %u\n",dwTupleNo,dwInnerID);
    }
    return;
}

WORD32 psNcuCreatAppIDMapCtxByStrAppID(CHAR *aucStrAppID,WORD32 dwInnerID,BYTE **ptCtxAddr)
{
    MCS_CHK_RET(NULL == aucStrAppID,0);
    MCS_CHK_RET(NULL == ptCtxAddr,0);
    DM_PFU_ALLOCDYNDATA_BYIDX_REQ tAllocReq = {0};
    DM_PFU_ALLOCDYNDATA_BYIDX_ACK tAllocAck = {0};

    BYTE ucKeyLen  = zte_strnlen_s(aucStrAppID, LEN_APPLICATIONMAP_APPLICATION_MAX);

    tAllocReq.hDB                   = _NCU_DBHANDLE_COMM;
    tAllocReq.hTbl                  = DB_HANDLE_R_NCU_APPIDMAP;
    tAllocReq.dataAreaKey.ucKeyFlag = XDB_KEYFLAG_UNIQUE;
    tAllocReq.dataAreaKey.hIdx      = DB_HANDLE_IDX_R_NCU_APPIDMAP_NAME;
    tAllocReq.dataAreaKey.ucKeyLen  = ucKeyLen;

    zte_memcpy_s(tAllocReq.dataAreaKey.aucKey, XDB_PFU_MAX_KEYVALUE_LEN, aucStrAppID, ucKeyLen);

    _pDM_PFU_ALLOCDYNDATA_BYIDX(&tAllocReq, &tAllocAck);
    if((RC_OK == tAllocAck.retCODE || RC_EXIST == tAllocAck.retCODE) && (NULL != tAllocAck.ptDataAreaAddr))
    {
        psNcuUpdateAppIDMapByInnerId(tAllocAck.dwDataAreaIndex,dwInnerID);
        *ptCtxAddr = tAllocAck.ptDataAreaAddr;
        return tAllocAck.dwDataAreaIndex;
    }
    else
    {
        DEBUG_TRACE(DEBUG_LOW,"\n[Mcs]psNcuCreatAppIDMapCtxByStrAppID: Creat failed!! aucStrAppID = %s\n",aucStrAppID);
        return 0;
    }
}

VOID psNcuDelAppIDMapCtxByStrAppID(CHAR *aucStrAppID)
{
    MCS_CHK_VOID(NULL==aucStrAppID);

    DM_PFU_RELEASEDYNDATA_BYIDX_REQ  tReq = {0};
    DM_PFU_RELEASEDYNDATA_BYIDX_ACK  tAck = {0};
    BYTE   ucKeyLen  = zte_strnlen_s(aucStrAppID, LEN_APPLICATIONMAP_APPLICATION_MAX);

    tReq.hDB                   = _NCU_DBHANDLE_COMM;
    tReq.hTbl                  = DB_HANDLE_R_NCU_APPIDMAP;
    tReq.dataAreaKey.ucKeyFlag = XDB_KEYFLAG_UNIQUE;
    tReq.dataAreaKey.hIdx      = DB_HANDLE_IDX_R_NCU_APPIDMAP_NAME;
    tReq.dataAreaKey.ucKeyLen  = ucKeyLen;

    zte_memcpy_s(tReq.dataAreaKey.aucKey, XDB_PFU_MAX_KEYVALUE_LEN, aucStrAppID, ucKeyLen);
    _pDM_PFU_RELEASEDYNDATA_BYIDX(&tReq, &tAck);
    if(RC_OK != tAck.retCODE)
    {
        DEBUG_TRACE(DEBUG_LOW,"psNcuDelAppIDMapCtxByStrAppID failed! aucStrAppID = %s",aucStrAppID);
    }

    return;
}

WORD32 psNcuGetApplicationMapForCfg()
{
    WORD32 i = 0;
    BOOLEAN bflag = FALSE;
    DM_UPFCOMM_GETALLAPPLICATIONMAP_REQ tReq = {0};
    DM_UPFCOMM_GETALLAPPLICATIONMAP_ACK tAck = {0};
    BYTE    *ptCtxAddr = NULL;

    do
    {
        tReq.msgType   = MSG_CALL;
        tReq.dwTupleNo = tAck.dwTupleNo;
        tAck.retCODE   = RC_DBM_ERROR;
        bflag = DBM_CALL(DM_UPFCOMM_GETALLAPPLICATIONMAP, (LPSTR)&tReq, (LPSTR)&tAck);
        MCS_CHK_RET((TRUE != bflag), CFG_RET_OK);
        MCS_CHK_RET((RC_DBM_OK != tAck.retCODE), CFG_RET_OK);

        /* 配置转成动态表 */
        for (i = 0; (i < tAck.dwValidNum) && (i<UPFNCU_GETALL_MAX); i++)
        {
            psNcuCreatAppIDMapCtxByStrAppID(&tAck.application[i][0], tAck.innerappid[i], &ptCtxAddr);
        }
    }while( !tAck.blEnd );

    zte_printf_s( "[ApplicationMap] call psNcuGetApplicationMapForCfg succ!\n" );

    return CFG_RET_OK;
}
/* Ended by AICoder, pid:p836ag713db2bdf14c2f0a2b9026aa95c221f684 */
