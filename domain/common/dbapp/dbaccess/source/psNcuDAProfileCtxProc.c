/* Started by AICoder, pid:708ffvfe7e928aa1499908b6c03ecf940f611ebd */
/******************************************************************************
 * 版权所有 (C)2016 深圳市中兴通讯股份有限公司*
 * 模块名          : MCS
 * 文件名          : psNcuDAProfileCtxProc.c
 * 相关文件        :
 * 文件实现功能     : DAProfile上下文
 * 归属团队        : M6
 * 版本           : V7.24.30
 -------------------------------------------------------------------------------
 * 修改记录:
 * 修改日期           版本号           修改人           修改内容
 * 2024-09-20    V7.24.30              wya          create
******************************************************************************/
/**************************************************************************
 *                              头文件(满足最小依赖请按照DDD分层架构逐层依赖)
 **************************************************************************/
#define USE_DM_UPFCOMM_GETALLR_DAPROFILE
#include "dbm_lib_upfcomm.h"
#include "psNcuDAProfileCtxProc.h"
#include "ps_ncu_typedef.h"
#include "xdb_pfu_dyntbl_acc.h"
#include "xdb_pfu_com.h"
#include "ps_db_define_ncu.h"
#include "dbmLibComm.h"
#include "r_daprofile.h"
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
T_VpfuDAProfileCtx* allocDAProfileCtx(lp_r_dyn_daprofile_idx_application idx, WORD32 *pdwDAProfileCtxId)
{
    _DB_STATEMENT_TRUE_RTN_VALUE(NULL== idx, NULL);
    _DB_STATEMENT_TRUE_RTN_VALUE(NULL== pdwDAProfileCtxId, NULL);

    DM_PFU_ALLOCDYNDATA_BYIDX_REQ   tAllocReq = {0};
    DM_PFU_ALLOCDYNDATA_BYIDX_ACK   tAllocAck = {0};

    tAllocReq.hDB = _NCU_DBHANDLE_COMM;
    tAllocReq.hTbl = DB_HANDLE_R_NCUDAPROFILE;
    tAllocReq.dataAreaKey.ucKeyFlag = XDB_KEYFLAG_UNIQUE;
    tAllocReq.dataAreaKey.hIdx = DB_HANDLE_IDX_R_NCU_DAPROFILE_APPLICATION;
    tAllocReq.dataAreaKey.ucKeyLen = (BYTE)sizeof(r_dyn_daprofile_idx_application);

    zte_memcpy_s(tAllocReq.dataAreaKey.aucKey, tAllocReq.dataAreaKey.ucKeyLen, (BYTE *)idx->application, tAllocReq.dataAreaKey.ucKeyLen);
    tAllocAck.retCODE = RC_ERROR;
    _pDM_PFU_ALLOCDYNDATA_BYIDX(&tAllocReq, &tAllocAck);

    *pdwDAProfileCtxId = tAllocAck.dwDataAreaIndex;
    if (RC_OK != tAllocAck.retCODE && RC_EXIST != tAllocAck.retCODE)
    {
        return NULL;
    }

    if (NULL == tAllocAck.ptDataAreaAddr)
    {
        return NULL;
    }
    return (T_VpfuDAProfileCtx *)tAllocAck.ptDataAreaAddr;
}

DBBOOL queryDAProfileCtx(lp_r_dyn_daprofile_idx_application idx,T_VpfuDAProfileCtx **ptDAProfileTuple)
{
    _DB_STATEMENT_TRUE_RTN_VALUE(NULL== idx, FALSE);
    _DB_STATEMENT_TRUE_RTN_VALUE(NULL== ptDAProfileTuple, FALSE);

    DM_PFU_QUERYDYNDATA_BYIDX_REQ tQueryReq = {0}; /* 查询请求 */
    DM_PFU_QUERYDYNDATA_BYIDX_ACK tQueryAck = {0}; /* 查询应答 */

    tQueryReq.hDB = _NCU_DBHANDLE_COMM;
    tQueryReq.hTbl = DB_HANDLE_R_NCUDAPROFILE;
    tQueryReq.dwExpectNum = 1;
    tQueryReq.dataAreaKey.ucKeyFlag = XDB_KEYFLAG_UNIQUE;
    tQueryReq.dataAreaKey.hIdx = DB_HANDLE_IDX_R_NCU_DAPROFILE_APPLICATION;
    tQueryReq.dataAreaKey.ucKeyLen = (BYTE)sizeof(r_dyn_daprofile_idx_application);

    zte_memcpy_s(tQueryReq.dataAreaKey.aucKey, tQueryReq.dataAreaKey.ucKeyLen, (BYTE *)idx->application, tQueryReq.dataAreaKey.ucKeyLen);

    _pDM_PFU_QUERYDYNDATA_BYIDX(&tQueryReq,&tQueryAck);

    if((RC_OK == tQueryAck.retCODE)&&(NULL != tQueryAck.pDataAreaAddr))/*614003656806 db返回指针异常，这里保护以下*/
    {
        *ptDAProfileTuple = (T_VpfuDAProfileCtx *)tQueryAck.pDataAreaAddr;
        return TRUE;
    }
    return FALSE;
}

BYTE* psQueryTblByUnidx(WORD32 hDB,WORD32 hTbl,WORD32 hIdx,BYTE ucKeyLen,BYTE *aucKey)
{
    DM_PFU_QUERYDYNDATA_BYIDX_REQ tQueryReq = {0};
    DM_PFU_QUERYDYNDATA_BYIDX_ACK tQueryAck = {0};
    _DB_STATEMENT_TRUE_RTN_VALUE(NULL == aucKey, NULL);

    tQueryReq.hDB  = hDB;
    tQueryReq.hTbl = hTbl;
    tQueryReq.dwExpectNum = 1;
    tQueryReq.dataAreaKey.ucKeyFlag = XDB_KEYFLAG_UNIQUE;
    tQueryReq.dataAreaKey.hIdx = hIdx;
    tQueryReq.dataAreaKey.ucKeyLen = ucKeyLen;
    zte_memcpy_s(tQueryReq.dataAreaKey.aucKey, XDB_PFU_MAX_KEYVALUE_LEN, aucKey, ucKeyLen);
    _pDM_PFU_QUERYDYNDATA_BYIDX(&tQueryReq, &tQueryAck);
    if (RC_OK != tQueryAck.retCODE || NULL == tQueryAck.pDataAreaAddr)
    {
        return NULL;
    }

    return tQueryAck.pDataAreaAddr;
}
DBBOOL psDeleteTblByUnidx(WORD32 hDB,WORD32 hTbl,WORD32 hIdx,BYTE ucKeyLen,BYTE *aucKey)
{
    DM_PFU_RELEASEDYNDATA_BYIDX_REQ tReleaseReq = {0}; 
    DM_PFU_RELEASEDYNDATA_BYIDX_ACK tReleaseAck = {0}; 

    tReleaseReq.hDB  = hDB; 
    tReleaseReq.hTbl = hTbl;
    tReleaseReq.dataAreaKey.ucKeyFlag = XDB_KEYFLAG_UNIQUE;
    tReleaseReq.dataAreaKey.hIdx = hIdx;
    tReleaseReq.dataAreaKey.ucKeyLen = ucKeyLen;    
    zte_memcpy_s(tReleaseReq.dataAreaKey.aucKey, XDB_PFU_MAX_KEYVALUE_LEN, aucKey, ucKeyLen);
    _pDM_PFU_RELEASEDYNDATA_BYIDX(&tReleaseReq,&tReleaseAck);
    if (RC_OK == tReleaseAck.retCODE)
    {
        return TRUE;
    }
    return FALSE;
}

VOID getDAProfileAllCfg()
{
    DM_UPFCOMM_GETALLR_DAPROFILE_REQ     req = { 0 };
    DM_UPFCOMM_GETALLR_DAPROFILE_ACK     ack = { 0 };
    r_dyn_daprofile_idx_application     tIdx = {0};
    T_VpfuDAProfileCtx              *ptTuple = NULL;
    BOOLEAN                            bflag = FALSE;
    WORD16                                 i = 0;
    WORD32                  dwDAProfileCtxId = 0;
    WORD32                            dwLoop = R_DAPROFILE_CAPACITY;
    do
    {
        req.msgType = MSG_CALL;
        req.dwTupleNo = ack.dwTupleNo;
        ack.retCODE = RC_DBM_ERROR;
        bflag = DBM_CALL(DM_UPFCOMM_GETALLR_DAPROFILE, (LPSTR)&req, (LPSTR)&ack);
        if(TRUE != bflag ||RC_DBM_OK != ack.retCODE)
        {
            XOS_SysLog(LOG_EMERGENCY,"Func: getDAProfileAllCfg DBM_CALL FALSE!");
            return;
        }
        
        for (i = 0; (i < ack.dwValidNum) && (i < UPFNCU_GETALL_MAX); i++)
        {
            dwLoop--;
            zte_memcpy_s(tIdx.application, sizeof(tIdx.application), ack.application[i], sizeof(tIdx.application));
            ptTuple = (T_VpfuDAProfileCtx *)allocDAProfileCtx(&tIdx,&dwDAProfileCtxId);
            if (NULL == ptTuple)
            {
                continue;
            }
            ptTuple->ulbwswitch = ack.ulbwswitch[i];
            ptTuple->ulbwthreshold = ack.ulbwthreshold[i];
            ptTuple->dlbwswitch = ack.dlbwswitch[i];
            ptTuple->dlbwthreshold = ack.dlbwthreshold[i];
            ptTuple->andelayswitch = ack.andelayswitch[i];
            ptTuple->andelaythreshold = ack.andelaythreshold[i];
            ptTuple->dndelayswitch = ack.dndelayswitch[i];
            ptTuple->dndelaythreshold = ack.dndelaythreshold[i];
            ptTuple->ulplrswitch = ack.ulplrswitch[i];
            ptTuple->ulplrthreshold = ack.ulplrthreshold[i];
            ptTuple->dlplrswitch = ack.dlplrswitch[i];
            ptTuple->dlplrthreshold = ack.dlplrthreshold[i];
            ptTuple->checkpoortimes = ack.checkpoortimes[i];
            ptTuple->checkgoodtimes = ack.checkgoodtimes[i];
            ptTuple->reportevent = ack.reportevent[i];
            ptTuple->detecttime = ack.detecttime[i];
            ptTuple->reportpoortime = ack.reportpoortime[i]*5;
            ptTuple->reportgoodtime = ack.reportgoodtime[i]*5;
            ptTuple->bwzerothreshold = ack.bwzerothreshold[i];
            zte_memcpy_s(ptTuple->application, sizeof(ptTuple->application), ack.application[i], sizeof(ptTuple->application));
        }
    }while(!ack.blEnd && dwLoop);
    return;
}

VOID CfgChgDAProfileProc(WORD16 wOperation, WORD16 wDataMsgLen, BYTE *lpData)
{
    _DB_STATEMENT_TRUE_RTN_NONE(NULL == lpData);
    R_DAPROFILE_TUPLE                   *ptupleNew = NULL;
    T_VpfuDAProfileCtx                    *ptTuple = NULL;
    r_dyn_daprofile_idx_application             idx = {0};
    WORD32                            dwDAProfileCtxId= 0;

    if (_DB_CFGCHG_TUPLE_NOTIFY_INSERT == wOperation)
    {
        if (sizeof(R_DAPROFILE_TUPLE) != wDataMsgLen)
        {
            XOS_SysLog(LOG_EMERGENCY,"recv daprofile errLen!");
            return;
        }
        ptupleNew = (R_DAPROFILE_TUPLE *)lpData;
        zte_memcpy_s(idx.application, sizeof(idx.application), ptupleNew->application, sizeof(idx.application));
        ptTuple = allocDAProfileCtx(&idx,&dwDAProfileCtxId);
        _DB_STATEMENT_TRUE_RTN_NONE(NULL == ptTuple);
        ptTuple->ulbwswitch = ptupleNew->ulbwswitch;
        ptTuple->ulbwthreshold = ptupleNew->ulbwthreshold;
        ptTuple->dlbwswitch = ptupleNew->dlbwswitch;
        ptTuple->dlbwthreshold = ptupleNew->dlbwthreshold;
        ptTuple->andelayswitch = ptupleNew->andelayswitch;
        ptTuple->andelaythreshold = ptupleNew->andelaythreshold;
        ptTuple->dndelayswitch = ptupleNew->dndelayswitch;
        ptTuple->dndelaythreshold = ptupleNew->dndelaythreshold;
        ptTuple->ulplrswitch = ptupleNew->ulplrswitch;
        ptTuple->ulplrthreshold = ptupleNew->ulplrthreshold;
        ptTuple->dlplrswitch = ptupleNew->dlplrswitch;
        ptTuple->dlplrthreshold = ptupleNew->dlplrthreshold;
        ptTuple->checkpoortimes = ptupleNew->checkpoortimes;
        ptTuple->checkgoodtimes = ptupleNew->checkgoodtimes;
        ptTuple->reportevent = ptupleNew->reportevent;
        ptTuple->detecttime = ptupleNew->detecttime;
        ptTuple->reportpoortime = (ptupleNew->reportpoortime)*5;
        ptTuple->reportgoodtime = (ptupleNew->reportgoodtime)*5;
        ptTuple->bwzerothreshold = ptupleNew->bwzerothreshold;
        zte_memcpy_s(ptTuple->application, sizeof(ptTuple->application), ptupleNew->application, sizeof(ptTuple->application));
        return;
    }
    else if(_DB_CFGCHG_TUPLE_NOTIFY_MODIFY == wOperation)
    {
        if (2 * sizeof(R_DAPROFILE_TUPLE) != wDataMsgLen)
        {
            XOS_SysLog(LOG_EMERGENCY,"recv modify daprofile errLen!");
            return;
        }
        ptupleNew = (R_DAPROFILE_TUPLE *)(lpData + sizeof(R_DAPROFILE_TUPLE));
        zte_memcpy_s(idx.application, sizeof(idx.application), ptupleNew->application, sizeof(idx.application));
        ptTuple = (T_VpfuDAProfileCtx *)psQueryTblByUnidx(_NCU_DBHANDLE_COMM,DB_HANDLE_R_NCUDAPROFILE,DB_HANDLE_IDX_R_NCU_DAPROFILE_APPLICATION,sizeof(idx),(BYTE *)&idx);
        _DB_STATEMENT_TRUE_RTN_NONE(NULL == ptTuple);

        ptTuple->ulbwswitch = ptupleNew->ulbwswitch;
        ptTuple->ulbwthreshold = ptupleNew->ulbwthreshold;
        ptTuple->dlbwswitch = ptupleNew->dlbwswitch;
        ptTuple->dlbwthreshold = ptupleNew->dlbwthreshold;
        ptTuple->andelayswitch = ptupleNew->andelayswitch;
        ptTuple->andelaythreshold = ptupleNew->andelaythreshold;
        ptTuple->dndelayswitch = ptupleNew->dndelayswitch;
        ptTuple->dndelaythreshold = ptupleNew->dndelaythreshold;
        ptTuple->ulplrswitch = ptupleNew->ulplrswitch;
        ptTuple->ulplrthreshold = ptupleNew->ulplrthreshold;
        ptTuple->dlplrswitch = ptupleNew->dlplrswitch;
        ptTuple->dlplrthreshold = ptupleNew->dlplrthreshold;
        ptTuple->checkpoortimes = ptupleNew->checkpoortimes;
        ptTuple->checkgoodtimes = ptupleNew->checkgoodtimes;
        ptTuple->reportevent = ptupleNew->reportevent;
        ptTuple->detecttime = ptupleNew->detecttime;
        ptTuple->reportpoortime = (ptupleNew->reportpoortime)*5;
        ptTuple->reportgoodtime = (ptupleNew->reportgoodtime)*5;
        ptTuple->bwzerothreshold = ptupleNew->bwzerothreshold;
        zte_memcpy_s(ptTuple->application, sizeof(ptTuple->application), ptupleNew->application, sizeof(ptTuple->application));
        return;
    }
    else if (_DB_CFGCHG_TUPLE_NOTIFY_DELETE == wOperation)
    {
        if (sizeof(R_DAPROFILE_TUPLE) != wDataMsgLen)
        {
            XOS_SysLog(LOG_EMERGENCY,"recv delete daprofile errLen!");
            return;
        }

        ptupleNew = (R_DAPROFILE_TUPLE *)lpData;
        zte_memcpy_s(idx.application, sizeof(idx.application), ptupleNew->application, sizeof(idx.application));
        if(FALSE ==psDeleteTblByUnidx(_NCU_DBHANDLE_COMM,DB_HANDLE_R_NCUDAPROFILE,DB_HANDLE_IDX_R_NCU_DAPROFILE_APPLICATION,sizeof(idx), (BYTE *)&idx))
        {
            XOS_SysLog(LOG_EMERGENCY,"recv delete daprofile fail!");
            return;
        }
        return;
    }
    return;
}
/* Ended by AICoder, pid:708ffvfe7e928aa1499908b6c03ecf940f611ebd */
