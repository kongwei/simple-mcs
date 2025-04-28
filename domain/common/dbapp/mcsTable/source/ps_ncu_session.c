#include "ps_ncu_session.h"
#include "ps_db_define_ncu.h"
#include "xdb_core_pfu.h"
#include "xdb_pfu_com.h"
#include "xdb_core_pfu_db.h"
#include "xdb_core_pfu_table.h"
#include "xdb_core_uniindex.h"
#include "xdb_core_pfu_tblimp_api.h"
#include "xdb_pfu_dyntbl_acc.h"
#include "ps_mcs_define.h"
#include "zte_slibc.h"
#include "ncu_capacity.h"
#include "xdb_core_pfu_nunidx.h"
#include "xdb_pub_pfu.h"


LPT_PfuQueHeadReg g_pGroupQueHead[CSS_X86_MAX_THREAD_NUM+1][MAX_UPF_GROUP_NUM];   //Group队列,用于按群删除用户

void r_ncu_session_UIdxMerge(void* pTuple, void* pKey,DWORD dwKeyLen)
{
    lp_r_ncu_session_idx_tuple lp_r_ncu_session_idx = (lp_r_ncu_session_idx_tuple)pKey;
    lp_r_ncu_session_tuple pAddr = (lp_r_ncu_session_tuple)pTuple;
    lp_r_ncu_session_idx->ddwUPSeid = pAddr->ddwUPSeid;
    return;
}

void r_ncu_session_UIdxCpy(void* pTuple, void* pKey,DWORD dwKeyLen)
{
    lp_r_ncu_session_idx_tuple lp_r_ncu_session_idx = (lp_r_ncu_session_idx_tuple)pKey;
    lp_r_ncu_session_tuple pAddr = (lp_r_ncu_session_tuple)pTuple;
    pAddr->ddwUPSeid = lp_r_ncu_session_idx->ddwUPSeid;
    return;
}

BOOLEAN r_ncu_session_UIdxCmp(void* pTuple, void* pKey,  DWORD dwKeyLen)
{
    lp_r_ncu_session_idx_tuple lpTmp = (lp_r_ncu_session_idx_tuple)pKey;
    lp_r_ncu_session_tuple lpAddr = (lp_r_ncu_session_tuple )pTuple;

    return (lpTmp->ddwUPSeid == lpAddr->ddwUPSeid);
}

void r_ncu_session_imsi_NoUIdxMerge(void* pTuple, void* pKey,DWORD dwKeyLen)
{
    lp_r_ncu_session_idx_imsi_tuple lpTmp  = (lp_r_ncu_session_idx_imsi_tuple)pKey;
    lp_r_ncu_session_tuple lpAddr          = (lp_r_ncu_session_tuple )pTuple;

    psMemCpy(&(lpTmp->tIMSI), &(lpAddr->tIMSI), sizeof(T_IMSI));
    return;
}

void r_ncu_session_imsi_NoUIdxCpy(void* pTuple, void* pKey,DWORD dwKeyLen)
{
    lp_r_ncu_session_idx_imsi_tuple lpTmp  = (lp_r_ncu_session_idx_imsi_tuple)pKey;
    lp_r_ncu_session_tuple lpAddr          = (lp_r_ncu_session_tuple )pTuple;

    psMemCpy(&(lpAddr->tIMSI), &(lpTmp->tIMSI), sizeof(T_IMSI));
    return;
}

BOOLEAN r_ncu_session_ReleaseRsc(LPT_PfuDataBase pPfuDbReg, LPT_PfuTableReg pPfuTableReg, DWORD dwTupleNo)
{
    _DB_STATEMENT_TRUE_RTN_VALUE((NULL == pPfuDbReg), FALSE);
    LPT_PfuUniIdxReg    ptPfuUniIdxReg = NULL;

    BOOLEAN dbRet = FALSE;
    
    T_psNcuSessionCtx  *ptVpfuSesionNode = psMcsGetSessionCtxById(dwTupleNo, pPfuDbReg->hDB);
    _DB_STATEMENT_TRUE_RTN_VALUE((NULL == ptVpfuSesionNode), FALSE);

    ptPfuUniIdxReg = xdb_Pfu_Get_UniIdxReg_ByDbReg(pPfuDbReg, DB_HANDLE_IDX_R_NCU_SESSION_UPSEID);
    _DB_STATEMENT_TRUE_RTN_VALUE((NULL == ptPfuUniIdxReg), FALSE);
    xdb_pfu_delete_UniIdx_by_tupleNo(pPfuTableReg, ptPfuUniIdxReg, dwTupleNo);

    LPT_PfuNoUniIdxReg  ptPfuNoUniIdxReg2 = NULL;
    BYTE                aucTupleKey[XDB_IDXKEYLEN_MAX] = {0};
    ptPfuNoUniIdxReg2 = xdb_Pfu_Get_NoUniIdxReg_ByDbReg(pPfuDbReg, DB_HANDLE_IDX_R_NCU_SESSION_IMSI);
    _DB_STATEMENT_TRUE_RTN_VALUE((NULL == ptPfuNoUniIdxReg2), FALSE);
    xdb_pfu_nunidx_merge_by_tupleno(pPfuTableReg, ptPfuNoUniIdxReg2, aucTupleKey, dwTupleNo);
    xdb_pfu_delete_nunidx(pPfuTableReg, ptPfuNoUniIdxReg2, aucTupleKey, dwTupleNo);

    dbRet = xdb_Delete_Tuple(pPfuTableReg,dwTupleNo);
    _DB_STATEMENT_TRUE_RTN_VALUE((FALSE == dbRet), FALSE);
    return TRUE;
}


/* 临时设置，后续需要查capacity表*/
WORD32 _db_get_ncusession_capacity()
{
#ifndef FT_TEST
    return _db_get_ncu_session_capacity();
#else
    return 20;
#endif
}
BOOL create_r_ncu_session(DWORD dwDbHandle)
{
    DWORD dwDynTblCapacity = 0;
    WORD32   wLoop = 0;
    LPT_PfuTableReg      pTableReg = NULL;
    LPT_PfuUniIdxReg     pPfuUniIdxReg = NULL;
    LPT_PfuNoUniIdxReg   pPfuNoUniIdxReg    = NULL;

    dwDynTblCapacity = _db_get_ncusession_capacity();
    zte_printf_s("ncu session capacity: %u\n", dwDynTblCapacity);
    _DB_STATEMENT_TRUE_RTN_VALUE((0 == dwDynTblCapacity), FALSE);

    /* 创建表 */
    XOS_SysLog(LOG_EMERGENCY, "DB Module: create table r_pfu_session start!\n");
    pTableReg = XDB_PFU_CREATE_TABLE(dwDbHandle, DB_HANDLE_R_NCUSESSION, R_NCU_SESSION, 
                                     dwDynTblCapacity, sizeof(T_psNcuSessionCtx), 
                                     sizeof(r_ncu_session_tuple));
    if (NULL == pTableReg)
    {
        XOS_SysLog(LOG_EMERGENCY, "[File:%s],[line:%d]DB Module:create table r_ncu_session failed!\n", __FILE__, __LINE__);
        return FALSE;
    }

    /* 创建索引 */
    pPfuUniIdxReg = XDB_PFU_CREATE_UNIIDX(dwDbHandle, DB_HANDLE_IDX_R_NCU_SESSION_UPSEID, XDB_UNIDX_NORMARL,
                                          dwDynTblCapacity, XDB_MOD_TABLE(dwDynTblCapacity),
                                          sizeof(r_ncu_session_idx_tuple));
    if (NULL == pPfuUniIdxReg)
    {
        XOS_SysLog(LOG_EMERGENCY, "[File:%s],[line:%d]DB Module: create index DB_HANDLE_IDX_R_NCU_SESSION_UPSEID failed!\n", __FILE__, __LINE__);
        return FALSE;
    }
    xdb_pfu_UniIdx_Cpy_Reg(pPfuUniIdxReg, r_ncu_session_UIdxCpy);
    xdb_pfu_UniIdx_Merge_Reg(pPfuUniIdxReg, r_ncu_session_UIdxMerge);
    xdb_pfu_UniIdx_Cmp_Reg(pPfuUniIdxReg, r_ncu_session_UIdxCmp);

    /* 创建非唯一索引IMSI */
    pPfuNoUniIdxReg = XDB_PFU_CREATE_NUNIDX(dwDbHandle, DB_HANDLE_IDX_R_NCU_SESSION_IMSI,
                                            dwDynTblCapacity, XDB_MOD_TABLE(dwDynTblCapacity),
                                            sizeof(r_ncu_session_idx_imsi_tuple));
    _mcs_if (NULL == pPfuNoUniIdxReg)
    {
        XOS_SysLog(LOG_EMERGENCY, "[File:%s],[line:%d]DB Module: create index DB_HANDLE_IDX_R_NCU_SESSION_IMSI failed!\n", __FILE__, __LINE__);
        return FALSE;
    }
    xdb_pfu_nunidx_cpy_reg(pPfuNoUniIdxReg, r_ncu_session_imsi_NoUIdxCpy);
    xdb_pfu_nunidx_merge_reg(pPfuNoUniIdxReg, r_ncu_session_imsi_NoUIdxMerge);
    
    /*********************************************************
    以下创建Group队列,用于按群删除用户
    *********************************************************/
    LPT_PfuQueGrpReg pSessionCtxQueGrpReg = XDB_PFU_CREATEALLQUEOFQUEGRP(dwDbHandle, DB_HANDLE_QUE_GRP_R_PFU_SESSION_GROUP, dwDynTblCapacity, PFU_SESSION_GROUP_QUEHANDLE, MAX_UPF_GROUP_NUM);
    _DB_STATEMENT_TRUE_RTN_VALUE((NULL == pSessionCtxQueGrpReg), FALSE);

    xdb_pfu_QueGrp_lock_set(pSessionCtxQueGrpReg);

    WORD32 dwVCpuNo = _PFU_GET_CPUNO(dwDbHandle);
    _DB_STATEMENT_TRUE_RTN_VALUE((dwVCpuNo > CSS_X86_MAX_THREAD_NUM), FALSE);
    for(wLoop=0; wLoop < MAX_UPF_GROUP_NUM; wLoop++)
    {
        g_pGroupQueHead[dwVCpuNo][wLoop] = xdb_Pfu_Get_QueReg(dwDbHandle, (PFU_SESSION_GROUP_QUEHANDLE+wLoop));
    }
    XOS_SysLog(LOG_EMERGENCY, "r_ncu_session.c, [line:%d]DB Module:PFU_SESSION_GROUP_QUEHANDLE=%x,DB_HANDLE_QUE_GRP_R_PFU_SESSION_GROUP=%x!\n",  __LINE__,PFU_SESSION_GROUP_QUEHANDLE,DB_HANDLE_QUE_GRP_R_PFU_SESSION_GROUP);

    xdb_Set_ReleaseRsc_Overload(pTableReg,r_ncu_session_ReleaseRsc);
    XOS_SysLog(LOG_EMERGENCY, "DB Module: create r_ncu_session table and index success!\n");

    return TRUE;
}

T_psNcuSessionCtx *psMcsGetSessionCtxById(WORD32 dwCtxId, WORD32 hDB)
{
    DM_PFU_QUERYDYNDATA_BYTUPLENO_REQ tQueryReq = {0}; /* 查询请求 */
    DM_PFU_QUERYDYNDATA_BYTUPLENO_ACK tQueryAck = {0}; /* 查询应答 */
    _DB_STATEMENT_TRUE_RTN_VALUE((0 == dwCtxId), NULL);
    tQueryReq.hDB               = hDB; /* 库句柄待定 */
    tQueryReq.hTbl              = DB_HANDLE_R_NCUSESSION;
    tQueryReq.dwDataAreaIndex   = dwCtxId;
    _pDM_PFU_QUERYDYNDATA_BYTUPLENO(&tQueryReq, &tQueryAck);
    _DB_STATEMENT_TRUE_RTN_VALUE((RC_OK != tQueryAck.retCODE), NULL);
    return (T_psNcuSessionCtx*)tQueryAck.ptDataAreaAddr;/*返回数据区指针 */
}

T_psNcuSessionCtx*  psQuerySessionByUpseid(WORD64 ddwUPSeid, WORD32 hDB)
{
    DM_PFU_QUERYDYNDATA_BYIDX_REQ tQueryReq = {0}; /* 查询请求 */
    DM_PFU_QUERYDYNDATA_BYIDX_ACK tQueryAck = {0}; /* 查询应答 */

    tQueryReq.hDB         = hDB; /* 库句柄待定 */
    tQueryReq.hTbl        = DB_HANDLE_R_NCUSESSION;
    tQueryReq.dwExpectNum = 1;
    tQueryReq.dataAreaKey.ucKeyFlag = XDB_KEYFLAG_UNIQUE;
    tQueryReq.dataAreaKey.hIdx      = DB_HANDLE_IDX_R_NCU_SESSION_UPSEID;
    tQueryReq.dataAreaKey.ucKeyLen  = sizeof(WORD64);

    CSS_MEMCPY(tQueryReq.dataAreaKey.aucKey,&ddwUPSeid,sizeof(WORD64));
    _pDM_PFU_QUERYDYNDATA_BYIDX(&tQueryReq,&tQueryAck);

    if((RC_OK == tQueryAck.retCODE)&&(NULL != tQueryAck.pDataAreaAddr))
    {
        return (T_psNcuSessionCtx *)tQueryAck.pDataAreaAddr;
    }
    return NULL;
}
T_psNcuSessionCtx*  psCreateSessionByUpseid(WORD64 ddwUPSeid, WORD32 hDB)
{
    DM_PFU_ALLOCDYNDATA_BYIDX_REQ tAllocReq = {0};
    DM_PFU_ALLOCDYNDATA_BYIDX_ACK tAllocAck = {0};

    tAllocReq.hDB  = hDB; /* 库句柄待定 */
    tAllocReq.hTbl = DB_HANDLE_R_NCUSESSION;
    tAllocReq.dataAreaKey.ucKeyFlag = XDB_KEYFLAG_UNIQUE;
    tAllocReq.dataAreaKey.hIdx      = DB_HANDLE_IDX_R_NCU_SESSION_UPSEID;
    tAllocReq.dataAreaKey.ucKeyLen  = sizeof(WORD64);

    CSS_MEMCPY(tAllocReq.dataAreaKey.aucKey,&ddwUPSeid,sizeof(WORD64));

    _pDM_PFU_ALLOCDYNDATA_BYIDX(&tAllocReq, &tAllocAck);
    if((RC_OK == tAllocAck.retCODE || RC_EXIST == tAllocAck.retCODE) && (NULL != tAllocAck.ptDataAreaAddr))
    {
        T_psNcuSessionCtx  *ptMcsNcuSessionCtx = (T_psNcuSessionCtx *)tAllocAck.ptDataAreaAddr;
        ptMcsNcuSessionCtx->dwSessionCtxId = tAllocAck.dwDataAreaIndex;
        ptMcsNcuSessionCtx->ddwUPSeid = ddwUPSeid;
        return ptMcsNcuSessionCtx;
    }
    return NULL;
}
WORD32  psDelSessionByUpseid(WORD64 ddwUPSeid,WORD32 hDB)
{
    DM_PFU_RELEASEDYNDATA_BYIDX_REQ tReleaseReq = {0};
    DM_PFU_RELEASEDYNDATA_BYIDX_ACK tReleaseAck = {0};

    tReleaseReq.hDB         = hDB;
    tReleaseReq.hTbl        = DB_HANDLE_R_NCUSESSION;
    tReleaseReq.dataAreaKey.ucKeyFlag = XDB_KEYFLAG_UNIQUE;
    tReleaseReq.dataAreaKey.hIdx      = DB_HANDLE_IDX_R_NCU_SESSION_UPSEID;
    tReleaseReq.dataAreaKey.ucKeyLen  = sizeof(WORD64);

    zte_memcpy_s(&tReleaseReq.dataAreaKey.aucKey,sizeof(WORD64),&ddwUPSeid,sizeof(WORD64));

    tReleaseAck.retCODE = RC_ERROR;

    _pDM_PFU_RELEASEDYNDATA_BYIDX(&tReleaseReq, &tReleaseAck);
    if(RC_OK == tReleaseAck.retCODE)
    {
        return MCS_RET_SUCCESS;
    }

    return MCS_RET_FAIL;
}

WORD32 psVpfuMcsUpdSessionCtxByImsi(BYTE *ptIMSI, WORD32 dwCtxId, WORD32 hDB)
{
    _DB_STATEMENT_TRUE_RTN_VALUE((NULL == ptIMSI), MCS_RET_FAIL);
    DM_PFU_UPDATEDYNDATA_REQ tReq = {0};
    DM_PFU_UPDATEDYNDATA_ACK tAck = {0};

    tReq.hDB  = hDB;
    tReq.hTbl = DB_HANDLE_R_NCUSESSION;
    tReq.ucUpdateMethodFlag = 0;
    tReq.dwDataAreaIndex = dwCtxId;
    tReq.dataAreaKey.ucKeyLen  = sizeof(T_IMSI);
    tReq.dataAreaKey.ucKeyFlag = XDB_KEYFLAG_NOUNIQUE;
    tReq.dataAreaKey.hIdx = DB_HANDLE_IDX_R_NCU_SESSION_IMSI;

    CSS_MEMCPY(tReq.dataAreaKey.aucKey, ptIMSI, sizeof(T_IMSI));
    _pDM_PFU_UPDATEDYNDATA_BYNOUNIIDX(&tReq, &tAck);
    if (RC_OK != tAck.retCODE)
    {
        if(RC_EXIST ==tAck.retCODE)
        {
            return MCS_RET_EXIST;
        }
        return MCS_RET_FAIL;
    }
    return MCS_RET_SUCCESS;
}

WORD32 psVpfuMcsGetSessionCtxNumByIMSI(BYTE *ptIMSI, WORD32 hDB, BYTE *pbQueryAckBuff)
{
    DM_PFU_QUERYDYNDATA_BYIDX_REQ tQueryReq = {0}; /* 查询请求 */
    DM_PFU_QUERYDYNDATA_BYIDX_ACK *ptQueryAck = (DM_PFU_QUERYDYNDATA_BYIDX_ACK *)pbQueryAckBuff; /* 查询应答 */
    WORD32 dwCtxNum = 0;

    tQueryReq.hDB         = hDB; /* 库句柄待定 */
    tQueryReq.hTbl        = DB_HANDLE_R_NCUSESSION;
    tQueryReq.dwExpectNum = EXPECT_SESSION_NUM;
    tQueryReq.dataAreaKey.ucKeyFlag = XDB_KEYFLAG_NOUNIQUE;
    tQueryReq.dataAreaKey.hIdx      = DB_HANDLE_IDX_R_NCU_SESSION_IMSI;
    tQueryReq.dataAreaKey.ucKeyLen  = sizeof(T_IMSI);

    CSS_MEMCPY(tQueryReq.dataAreaKey.aucKey, ptIMSI, sizeof(T_IMSI));
    _pDM_PFU_QUERYDYNDATA_BYIDX(&tQueryReq, ptQueryAck);

    XOS_SysLog(LOG_EMERGENCY,"\n[Mcs]psVpfuMcsGetSessionCtxNumByIMSI: hDB = 0x%x tQueryReq.dataAreaKey.aucKey:0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x",
                        hDB,tQueryReq.dataAreaKey.aucKey[0], tQueryReq.dataAreaKey.aucKey[1], tQueryReq.dataAreaKey.aucKey[2], tQueryReq.dataAreaKey.aucKey[3]
                        , tQueryReq.dataAreaKey.aucKey[4], tQueryReq.dataAreaKey.aucKey[5], tQueryReq.dataAreaKey.aucKey[6], tQueryReq.dataAreaKey.aucKey[7]);

    if(RC_OK == ptQueryAck->retCODE)
    {
        dwCtxNum = ptQueryAck->dwDataAreaNum;
        XOS_SysLog(LOG_EMERGENCY,
                        "\n[Mcs]func:%s line:%d"
                        "\n dwhTbl  = %u"
                        "\n dwhIdx  = %u"
                        "\n dwCtxNum is %u ",
                        __FUNCTION__,__LINE__,
                        tQueryReq.hTbl,
                        tQueryReq.dataAreaKey.hIdx,
                        dwCtxNum);
        return dwCtxNum;/*返回会话上下文个数 */
    }
    else
    {
        XOS_SysLog(LOG_EMERGENCY,"\n[Mcs]psVpfuMcsGetSessionCtxNumByIMSI: query failed!!");
        return dwCtxNum;
    }
}

LPT_PfuQueHeadReg _db_getGroupQueHeadAdrr(DWORD dwDbHandle,WORD32 dwGroupId)
{
    _DB_STATEMENT_TRUE_RTN_VALUE((dwGroupId >= MAX_UPF_GROUP_NUM), NULL);
    WORD32 dwVCpuNo = _PFU_GET_CPUNO(dwDbHandle);
    _DB_STATEMENT_TRUE_RTN_VALUE((dwVCpuNo > CSS_X86_MAX_THREAD_NUM), NULL);

    return g_pGroupQueHead[dwVCpuNo][dwGroupId];
}

BOOLEAN  psGroupQueInsert(DWORD dwDbHandle,WORD32 dwTupleNo, WORD32 dwGroupId, DWORD dwhQueGrpReg)
{
    _DB_STATEMENT_TRUE_RTN_VALUE((dwGroupId >= MAX_UPF_GROUP_NUM), FALSE);

    /* 获取库注册区 */
    LPT_PfuDataBase pDbReg = xdb_Pfu_Get_DbReg(dwDbHandle);
    _DB_STATEMENT_TRUE_RTN_VALUE((NULL == pDbReg), FALSE);

    /* 首先将元素从队列组中强制删除 */
    _xdb_pfu_DelTupleFromQueGrp(pDbReg, dwTupleNo, dwhQueGrpReg);

    LPT_PfuQueHeadReg pQueHead = _db_getGroupQueHeadAdrr(dwDbHandle,dwGroupId);
    BOOLEAN bRet =  _xdb_pfu_QueInsert(pQueHead, dwTupleNo);

    return bRet;
}

BOOLEAN psGroupQueDelete(DWORD dwDbHandle,WORD32 dwTupleNo, WORD32 dwGroupId)
{
    _DB_STATEMENT_TRUE_RTN_VALUE((dwGroupId >= MAX_UPF_GROUP_NUM), FALSE);

    LPT_PfuQueHeadReg pQueHead = _db_getGroupQueHeadAdrr(dwDbHandle,dwGroupId);
    BOOLEAN bRet =  _xdb_pfu_QueDelete(pQueHead, dwTupleNo);

    return bRet;
}

LPT_PfuQueHeadReg (*psGetPfuQueHeadReg())[MAX_UPF_GROUP_NUM] {
    return g_pGroupQueHead;
}
LPT_PfuDataBase psGetPfuDbReg()
{
    return  g_tPfuDbSysLocal;
}
