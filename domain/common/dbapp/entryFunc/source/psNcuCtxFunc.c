#include "psNcuCtxFunc.h"
#include "ps_mcs_define.h"
#include "xdb_pfu_dyntbl_acc.h"
#include "McsHeadCap.h"

/**************************************************************************
 *                                   宏                                   *
 **************************************************************************/

/**************************************************************************
 *                                  常量                                  *
 **************************************************************************/


 /*************************************************************************
 *                                数据类型                                *
 **************************************************************************/


/**************************************************************************
 *                                全局变量                                *
 **************************************************************************/

/**************************************************************************
 *                              局部函数原型                              *
 **************************************************************************/

/**************************************************************************
                                外部函数实现                              *
***************************************************************************/

/**************************************************************************
 *                                函数实现                                *
 **************************************************************************/
/***********************************************************************
* 函数名称：psVpfuMcsGetCtxIdxById
* 功能描述：根据上下文id获取上下文索引头指针(注意:返回的是数据区前的索引区)
* 输入参数：dwLocalTEIDU- teidu信息
*           hDB:多核的句柄(实际就是根据线程号算出来的)
*           hTbl:上下文句柄
* 输出参数：
* 返 回 值：上下文指针 or NULL
* 其它说明：无
***********************************************************************
* 修改日期        版本号     修改人        修改内容
* --------------------------------------------------------
*2016.01.13        V1.0       dyb           Create
***********************************************************************/
inline void* psVpfuMcsGetCtxIdxById(WORD32 dwCtxId,WORD32 hDB,WORD32 hTbl)
{
    DM_PFU_QUERYDYNDATA_BYTUPLENO_REQ tQueryReq = {0}; /* 查询请求 */
    DM_PFU_QUERYDYNDATA_BYTUPLENO_ACK tQueryAck = {0}; /* 查询应答 */

    tQueryReq.hDB              = hDB; /* 库句柄待定 */
    tQueryReq.hTbl             = hTbl;
    tQueryReq.dwDataAreaIndex  = dwCtxId;
    _pDM_PFU_QUERYDYNDATAIDX_BYTUPLENO(&tQueryReq,  &tQueryAck);
    if(RC_OK == tQueryAck.retCODE)/*614003656806 db返回指针异常，这里保护以下*/
    {
        return (void*)tQueryAck.ptDataAreaAddr;/*返回数据区指针 */
    }
    else
    {
        return NULL;
    }
}

/***********************************************************************
* 函数名称：psVpfuMcsGetCtxById
* 功能描述：根据上下文id获取上下文
* 输入参数：dwLocalTEIDU- teidu信息
*           hDB:多核的句柄(实际就是根据线程号算出来的)
*           hTbl:上下文句柄
* 输出参数：
* 返 回 值：上下文指针 or NULL
* 其它说明：无
***********************************************************************
* 修改日期        版本号     修改人           修改内容
* --------------------------------------------------------
*2014.03.27        V1.0      tang.haitao       Create
***********************************************************************/
void *psVpfuMcsGetCtxById(WORD32 dwCtxId, WORD32 hDB, WORD32 hTbl)
{
    DM_PFU_QUERYDYNDATA_BYTUPLENO_REQ tQueryReq = {0}; /* 查询请求 */
    DM_PFU_QUERYDYNDATA_BYTUPLENO_ACK tQueryAck = {0}; /* 查询应答 */

    if(0 == dwCtxId)
    {
        return NULL;
    }
    tQueryReq.hDB               = hDB; /* 库句柄待定 */
    tQueryReq.hTbl              = hTbl;
    tQueryReq.dwDataAreaIndex   = dwCtxId;
    _pDM_PFU_QUERYDYNDATA_BYTUPLENO(&tQueryReq, &tQueryAck);
    if(RC_OK == tQueryAck.retCODE)/*614003656806 db返回指针异常，这里保护以下*/
    {
        return (void*)tQueryAck.ptDataAreaAddr;/*返回数据区指针 */
    }
    else
    {
        return NULL;
    }
}
/***********************************************************************
* 函数名称：psVpfuMcsDelCtxById
* 功能描述：根据上下文id删除上下文
* 输入参数：dwLocalTEIDU- teidu信息
*           hDB:多核的句柄(实际就是根据线程号算出来的)
*           hTbl:上下文句柄
* 输出参数：
* 返 回 值：上下文指针 or NULL
* 其它说明：无
*******************************************************fv****************
* 修改日期        版本号     修改人           修改内容
* --------------------------------------------------------
*2015.08.13        V1.0      gao.shencun       Create
***********************************************************************/
WORD32 psVpfuMcsDelCtxById(WORD32 dwCtxId, WORD32 hDB, WORD32 hTbl)
{
    DM_PFU_RELEASEDYNDATA_BYTUPLENO_REQ tRelReq = {0};
    DM_PFU_RELEASEDYNDATA_BYTUPLENO_ACK tRelAck;

    tRelReq.hDB             = hDB; /* 库句柄待定 */
    tRelReq.hTbl            = hTbl;
    tRelReq.dwDataAreaIndex = dwCtxId;
    _pDM_PFU_RELEASEDYNDATA_BYTUPLENO(&tRelReq,&tRelAck);
    if(RC_OK == tRelAck.retCODE)
    {
        return MCS_RET_SUCCESS;
    }
    return MCS_RET_FAIL;
}

/**********************************************************************
* 函数名称： psVpfuMcsGetCapacity
* 功能描述： 调用DB接口，获取当前的上下文资源容量
* 输入参数： DWORD dwDataType     数据库的资源类型
* 输出参数： dwTableCapacity:表容量
*            dwValidTupleNum:表有效条目
* 返 回 值： SUCCESS    成功
*            ERROR      失败
* 其它说明： 此函数控制核和媒体核均可使用
* 修改日期        版本号     修改人           修改内容
* --------------------------------------------------------
* 2014.5.1        1.0        tang.haitao        创建
***********************************************************************/
DWORD psVpfuMcsGetCapacity(DWORD dwDataType, WORD32 hDB, WORD32 *dwTableCapacity,WORD32 *dwValidTupleNum)
{
    DM_PFU_GETCAPACITY_REQ tQuery_Req  = {0};
    DM_PFU_GETCAPACITY_ACK tQuery_Ack  = {0};

    if ((NULL == dwTableCapacity) || (NULL == dwValidTupleNum))
    {
        return ERROR_MEDIA;
    }

    tQuery_Req.hDB       = hDB;
    tQuery_Req.hTbl      = dwDataType;
    /* afterchecktht: dbs 接口暂时删除 */
    _pDM_PFU_GETCAPACITY(&tQuery_Req, &tQuery_Ack);
    if (RC_OK != tQuery_Ack.retCODE)
    {
        return ERROR_MEDIA;
    }
    *dwTableCapacity  = tQuery_Ack.dwTableCapacity;
    *dwValidTupleNum  = tQuery_Ack.dwValidTupleNum;
    return SUCCESS;

}

// end of file
