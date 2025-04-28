/*********************************************************************
* 版权所有(C) 中兴通信股份有限公司. 保留所有权利!
*
* 文件名称： ncu_capacity.c
* 文件标识：
* 内容摘要： upf获取表容量公共函数
* 其它说明：
* 当前版本：
* 作    者：
* 完成日期：
**********************************************************************/

/**************************************************************************
*                            头文件                                      *
**************************************************************************/
#include "ncu_capacity.h"
#include "ps_ncu_typedef.h"

// dbm库依赖
#define USE_DM_UPFNCU_GETR_NC_CAPACITY
#include "dbm_lib_upfncu.h"
// #include "upfncuPub.h"

extern BYTE getVruSizeCap();
extern BOOL DBM_CALL(WORD32 EventNo, LPSTR iParam, LPSTR oParam);

DWORD  _db_get_ncu_session_capacity();
BOOL _db_get_ncu_capacity(WORD32 tblIdx, LP_DM_UPFNCU_GETR_NC_CAPACITY_REQ ptReq, LP_DM_UPFNCU_GETR_NC_CAPACITY_ACK ptAck);

DWORD _db_get_ncu_capacity_by_tblName(BYTE tblIdx, const char* tblName)
{
    DWORD  dwcapacity = 10;
    BOOL   bflag = FALSE;

    //从配置表中获取
    DM_UPFNCU_GETR_NC_CAPACITY_REQ req = { 0 };
    DM_UPFNCU_GETR_NC_CAPACITY_ACK ack = { 0 };
    bflag = _db_get_ncu_capacity(tblIdx, &req, &ack);
    MCS_CHK_RET(!bflag || (RC_DBM_OK != ack.retCODE), 10);

    if (NCVALUETYPE_RATE == ack.valuetype)
    {
        DWORD dwpfusessioncapacity = 0;
        dwpfusessioncapacity = _db_get_ncu_session_capacity();
        dwcapacity = ack.rate * dwpfusessioncapacity / 100;
    }
    else
    {
        dwcapacity = ack.capacity;
    }
    if (0 == dwcapacity)
    {
        dwcapacity = 1;
    }
    if (NULL != tblName)
    {
        zte_printf_s("%s capacity: %u\n", tblName, dwcapacity);
    }
    return dwcapacity;
}


BOOL _db_get_ncu_capacity(WORD32 tblIdx, LP_DM_UPFNCU_GETR_NC_CAPACITY_REQ ptReq, LP_DM_UPFNCU_GETR_NC_CAPACITY_ACK ptAck)
{
    MCS_CHK_RET((NULL == ptReq || NULL == ptAck), FALSE);
    BYTE  bVruSize = getVruSizeCap();
    BOOL  bflag = FALSE;

    //调用现有接口查询
    ptReq->msgType = MSG_CALL;
    ptReq->ctxname = tblIdx;
    ptAck->retCODE = RC_DBM_ERROR;
    ptReq->vrusize = bVruSize;

    bflag = DBM_CALL(DM_UPFNCU_GETR_NC_CAPACITY, (LPSTR)ptReq, (LPSTR)ptAck);

    /*按规格查找不到时，按照ALL查找*/
    if ((RC_DBM_OK != ptAck->retCODE) || !bflag)
    {
        ptReq->vrusize = (BYTE)NCVRUSIZECAP_ALL;
        bflag = DBM_CALL(DM_UPFNCU_GETR_NC_CAPACITY, (LPSTR)ptReq, (LPSTR)ptAck);
    }
    return bflag;
}

DWORD  _db_get_ncu_session_capacity()
{
    BOOLEAN bflag = FALSE;
    DM_UPFNCU_GETR_NC_CAPACITY_REQ req = { 0 };
    DM_UPFNCU_GETR_NC_CAPACITY_ACK ack = { 0 };

    req.msgType = MSG_CALL;
    req.ctxname = NCCTXNAME_R_NCU_SESSION;
    req.vrusize = getVruSizeCap();
    ack.retCODE = RC_DBM_ERROR;

    bflag = DBM_CALL((WORD32)DM_UPFNCU_GETR_NC_CAPACITY, (LPSTR)&req, (LPSTR)&ack);
    MCS_CHK_RET(!bflag || (RC_DBM_OK != ack.retCODE), 0);
    return ack.capacity;
}
