/******************************************************************************
版权所有 (C)2003, 深圳市中兴通讯股份有限公司 

模块名          : MCS
文件名          : ps_mcs_typedef.h
相关文件        :
文件实现功能    : 媒体面头文件
作者            :      
版本            : V1.0
-------------------------------------------------------------------------------
修改记录:
日  期      版本        修改人      修改内容
2024.04.03  V1.0                    新建
******************************************************************************/
#ifndef _PS_NCU_TYPEDEF_H_
#define _PS_NCU_TYPEDEF_H_
/**************************************************************************
 *                            其它条件编译选项                            *
***************************************************************************/


/**************************************************************************
 *                           标准、非标准头文件                           *
***************************************************************************/
#include "MemShareCfg.h"

/**************************************************************************
 *                                  常量                                  *
 **************************************************************************/

/**************************************************************************
 *                                数据类型                                *
 **************************************************************************/

/**************************************************************************
 *                              全局变量声明                              *
 **************************************************************************/

/************************************************************************/
/*                         定义相关变量                                */
/************************************************************************/

/**************************************************************************
 *                              宏(对外提供-卫语句)
 **************************************************************************/
#ifndef _mcs_if
#define _mcs_if(a) if(a)
#endif

#ifndef OFF
#define OFF                       ((BYTE)0)
#define ON                        ((BYTE)1)
#endif

// dbcfg相关返回值
#ifndef CFG_RET_OK
#define CFG_RET_OK               0
#define CFG_RET_FAIL             1
#define CFG_RET_CONTINUE         3
#endif

#define NCU_GET_THREADPARA_FROM_DELIVERPKT(ptMediaProcThreadPara, ptBufferPkt) \
{                                                                           \
    ptMediaProcThreadPara->ptPacket = ptBufferPkt;                          \
    ptMediaProcThreadPara->dwCurPktStdSec = (ptBufferPkt)->dwCurStdSec;       \
    PACKET_PROCESS_START(ptBufferPkt, PKTCHK_MCS_PROC);   \
}

#define MCS_PCLINT_NULLPTR_RET_1ARG(PTR1, failRET)              _mcs_if(unlikely(NULL == (PTR1)))   {return failRET;}
#define MCS_PCLINT_NULLPTR_RET_2ARG(PTR1, PTR2, failRET)        _mcs_if(unlikely(NULL == (PTR1) || NULL == (PTR2))) {return failRET;}
#define MCS_PCLINT_NULLPTR_RET_3ARG(PTR1, PTR2, PTR3, failRET)  _mcs_if(unlikely(NULL == (PTR1) || NULL == (PTR2) || NULL == (PTR3)))  {return failRET;}
#define MCS_PCLINT_NULLPTR_RET_4ARG(PTR1, PTR2, PTR3, PTR4,failRET)    \
    if (NULL == (PTR1) || NULL == (PTR2) || NULL == (PTR3) || NULL == (PTR4)) \
    {                                                             \
        return failRET;                                           \
    }
#define MCS_PCLINT_NULLPTR_RET_VOID(PTR1)                       _mcs_if(unlikely(NULL == (PTR1))) { return;}
#define MCS_PCLINT_NULLPTR_RET_2ARG_VOID(PTR1, PTR2)            _mcs_if(unlikely(NULL == (PTR1) || NULL == (PTR2))) { return;}
#define MCS_PCLINT_NULLPTR_RET_3ARG_VOID(PTR1, PTR2, PTR3)      _mcs_if(unlikely(NULL == (PTR1) || NULL == (PTR2) || NULL == (PTR3))) {return;}

// 推荐使用，比上面的宏短
#define MCS_CHK_RET(PTR1, retValue)             if(unlikely(PTR1)) {return retValue;}
#define MCS_CHK_RET_LIKELY(PTR1, retValue)      if(PTR1)           {return retValue;}
#define MCS_CHK_RET_STAT(PTR1, RET, STATITEM)   if(unlikely(PTR1)) {MCS_LOC_STAT_EX(ptNcuPerform, STATITEM, 1); return RET;}
#define MCS_CHK_RET_STAT_LIKELY(PTR1, RET, STATITEM) if(PTR1)      {MCS_LOC_STAT_EX(ptNcuPerform, STATITEM, 1); return RET;}
#define MCS_CHK_VOID(PTR1)                      if(unlikely(PTR1)) {return;}
#define MCS_CHK_VOID_LIKELY(PTR1)               if(PTR1)           {return;}
#define MCS_CHK_VOID_STAT(PTR1, STATITEM)       if(unlikely(PTR1)) {MCS_LOC_STAT_EX(ptNcuPerform, STATITEM, 1); return;}
#define MCS_CHK_NULL_VOID(PTR1)                 if(unlikely(NULL==(PTR1))) {return;}
#define MCS_CHK_NULL_VOID_STAT(PTR1, STATITEM)  if(unlikely(NULL==PTR1)) {MCS_LOC_STAT_EX(ptNcuPerform, STATITEM, 1); return;}
#define MCS_CHK_NULL_RET(PTR1, RET)             if(unlikely(NULL==(PTR1))) {return RET;}
#define MCS_CHK_NULL_RET_STAT(PTR1, RET, STATITEM)   if(unlikely(NULL==(PTR1))) {MCS_LOC_STAT_EX(ptNcuPerform, STATITEM, 1); return RET;}
#define MCS_CHK_CONTINUE(EXPR)                  {if(unlikely(EXPR)) {continue;}}
#define MCS_CHK_CONTINUE_LIKELY(EXPR)           {if(EXPR) {continue;}}
#define MCS_CHK_CONTINUE_STAT(EXPR, STATITEM)   if(unlikely(EXPR)) {MCS_LOC_STAT_EX(ptNcuPerform, STATITEM, 1); continue;}
#define MCS_CHK_CONTINUE_SET(EXPR, PARA, VALUE)   if(unlikely(EXPR)) {PARA = VALUE; continue;}
#define MCS_CHK_BREAK(EXPR)                 {if(unlikely(EXPR)) {break;}}
#define MCS_CHK_BREAK_STAT(EXPR, STATITEM)  {if(unlikely(EXPR)) {MCS_LOC_STAT_EX(ptNcuPerform, STATITEM, 1); break;}}
#define MCS_CHK_BREAK_LIKELY(EXPR)  {if(EXPR) {break;}}
#define MCS_CHK_STAT(EXPR, STATITEM) if(unlikely(EXPR)) {MCS_LOC_STAT_EX(ptNcuPerform, STATITEM, 1);}
#define MCS_CHK_SET(EXPR, DST, SRC)  if(EXPR)  {(DST) = (SRC);}

/* 条件执行过程 */
#define M_PRECONDITONAL_CALL_RET(precondition, Proc, ret) do{\
    if((precondition))\
    {\
        (ret) = (Proc);\
    }\
}while(0)

#define M_PRECONDITONAL_CALL(precondition, Proc) do{\
    if((precondition))\
    {\
        (Proc);\
    }\
}while(0)

#define M_MATCH_CONDITONAL_CALL(condition, value, Proc) do{\
    if((value)==(condition))\
    {\
        (Proc);\
    }\
}while(0)

#define MCS_CONDITIONAL_ASSIGN(CONDITION, VAL1, VAL2) ((CONDITION)?(VAL1):(VAL2))

#define MCS_LOG_IPV4_FIELD(IP) IP[0],IP[1],IP[2],IP[3]
#define MCS_LOG_IPV6_FIELD(IP) IP[0],IP[1],IP[2],IP[3],IP[4],IP[5],IP[6],IP[7],IP[8],IP[9],IP[10],IP[11],IP[12],IP[13],IP[14],IP[15]
#define MCS_LOG_MAC_FIELD(MAC) MAC[0],MAC[1],MAC[2],MAC[3],MAC[4],MAC[5]

#endif