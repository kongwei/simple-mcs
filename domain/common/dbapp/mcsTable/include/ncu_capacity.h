/*********************************************************************
* 版权所有(C) 中兴通信股份有限公司. 保留所有权利!
*
* 文件名称： upf_capacity.c
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
#ifndef _NCU_CAPACITY_H
#define _NCU_CAPACITY_H
#ifdef __cplusplus
extern "C" {
#endif

#include "upfncuPub.h"

DWORD  _db_get_ncu_session_capacity();
DWORD _db_get_ncu_capacity_by_tblName(BYTE tblIdx, const char* tblName);
#ifdef __cplusplus
}
#endif
#endif
