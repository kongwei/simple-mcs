/******************************************************************************
* 版权所有 (C)2016, 深圳市中兴通讯股份有限公司
*
* 模块名          : MCS
* 文件名          : psNcuCtrlStatInterface.h
* 相关文件        :
* 文件实现功能     : 管理JOB统计头文件
* 归属团队        : M6
* 版本           : V1.0
-------------------------------------------------------------------------------
* 修改记录:
* 修改日期            版本号           修改人           修改内容
* 2024-4-12        V7.24.20        wya            create
******************************************************************************/
#ifndef  _PS_NCU_CTRL_STAT_INTERFACE_H_
#define  _PS_NCU_CTRL_STAT_INTERFACE_H_
#ifdef __cplusplus
extern "C" {
#endif
/**************************************************************************
 *                              头文件(满足自包含)
 **************************************************************************/
#include "tulip.h"
/**************************************************************************
 *                              宏(对外提供)
 **************************************************************************/
#ifdef MSC_STAT_ITEM_SEG
#undef MSC_STAT_ITEM_SEG
#endif
#define MSC_STAT_ITEM_SEG(seg_name)

#ifdef MSC_STAT_ITEM
#undef MSC_STAT_ITEM
#endif
#define MSC_STAT_ITEM(item_name) WORD64  item_name;

#ifdef MSC_STAT_ITEM_ARRAY
#undef MSC_STAT_ITEM_ARRAY
#endif
#define MSC_STAT_ITEM_ARRAY(item_name, array_size) WORD64  item_name[array_size];
/**************************************************************************
 *                              数据类型(对外提供)
 **************************************************************************/
typedef struct
{
    #include "psNcuCtrlStatItemDef.h"
}T_psNcuMcsJobStat; /*控制面消息统计*/
extern T_psNcuMcsJobStat g_tNcuMcsJobStat;
/**************************************************************************
 *                              接口(对外提供，最小可见)
 **************************************************************************/
#define JOB_LOC_STAT_ADDONE(item) (g_tNcuMcsJobStat.item++)
/**************************************************************************
 *                              全局变量声明(对外提供，最小可见)
 **************************************************************************/

#ifdef __cplusplus
}
#endif
#endif
//end of file