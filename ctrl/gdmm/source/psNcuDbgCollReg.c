/******************************************************************************
* 版权所有 (C)2016, 深圳市中兴通讯股份有限公司
*
* 模块名          : MCS
* 文件名          : psNcuDbgCollReg.c
* 相关文件        :
* 文件实现功能     : 一键采集注册
* 归属团队        : M6
* 版本           : V1.0
-------------------------------------------------------------------------------
* 修改记录:
* 修改日期            版本号           修改人           修改内容
* 2024-4-12        V7.24.20        wya            create
******************************************************************************/
/**************************************************************************
 *                              头文件(满足最小依赖,请按照DDD分层架构逐层依赖)
 **************************************************************************/
#include "psNcuDbgCollReg.h"
#include "vnfp_pub.h"
#include "dmmClientApi.h"
#include "psNcuDbgReport.h"
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
void upf_ncu_collect_dbg_info_reg()
{
    dmmcRegDebugCallBack("UPF_NCU_MCS",         pubGetSelfContainerNo(), (TF_debugCallBack*)psNcuMcsPrintMcsDbgInfo,   (TF_clearCallBack*)psNcuMcsClearMcsDbgInfo);
    dmmcRegDebugCallBack("UPF_NCU_XDB_PartOne", pubGetSelfContainerNo(), (TF_debugCallBack *)psVpfuMcsPrintXdbDbgInfo_PartOne, (TF_clearCallBack *)psVpfuMcsClearXdbDbgInfo);
    dmmcRegDebugCallBack("UPF_NCU_XDB_PartTwo", pubGetSelfContainerNo(), (TF_debugCallBack *)psVpfuMcsPrintXdbDbgInfo_PartTwo, (TF_clearCallBack *)psVpfuMcsClearXdbDbgInfo);
    dmmcRegDebugCallBack("UPF_NCU_XDB_PartThree", pubGetSelfContainerNo(), (TF_debugCallBack *)psVpfuMcsPrintXdbDbgInfo_PartThree, (TF_clearCallBack *)psVpfuMcsClearXdbDbgInfo);
    dmmcRegDebugCallBack("UPF_NCU_XDB_PartFour", pubGetSelfContainerNo(), (TF_debugCallBack *)psVpfuMcsPrintXdbDbgInfo_PartFour, (TF_clearCallBack *)psVpfuMcsClearXdbDbgInfo);
    dmmcRegDebugCallBack("UPF_NCU_XDB_PartFive", pubGetSelfContainerNo(), (TF_debugCallBack *)psVpfuMcsPrintXdbDbgInfo_PartFive, (TF_clearCallBack *)psVpfuMcsClearXdbDbgInfo);
    dmmcRegDebugCallBack("UPF_NCU_XDB_ACC", pubGetSelfContainerNo(), (TF_debugCallBack *)psVpfuMcsPrintXdbAccDbgInfo, (TF_clearCallBack *)psVpfuMcsClearXdbAccDbgInfo);
    dmmcRegDebugCallBack("UPF_NCU_XDB_IDX_PartOne", pubGetSelfContainerNo(), (TF_debugCallBack *)psVpfuMcsPrintXdbIdxDbgInfo_PartOne, (TF_clearCallBack *)psVpfuMcsClearXdbIdxDbgInfo);
    dmmcRegDebugCallBack("UPF_NCU_XDB_IDX_PartTwo", pubGetSelfContainerNo(), (TF_debugCallBack *)psVpfuMcsPrintXdbIdxDbgInfo_PartTwo, (TF_clearCallBack *)psVpfuMcsClearXdbIdxDbgInfo);
    dmmcRegDebugCallBack("UPF_NCU_XDB_IDX_PartThree", pubGetSelfContainerNo(), (TF_debugCallBack *)psVpfuMcsPrintXdbIdxDbgInfo_PartThree, (TF_clearCallBack *)psVpfuMcsClearXdbIdxDbgInfo);
    dmmcRegDebugCallBack("UPF_NCU_XDB_IDX_PartFour", pubGetSelfContainerNo(), (TF_debugCallBack *)psVpfuMcsPrintXdbIdxDbgInfo_PartFour, (TF_clearCallBack *)psVpfuMcsClearXdbIdxDbgInfo);
    dmmcRegDebugCallBack("UPF_NCU_XDB_IDX_PartFive", pubGetSelfContainerNo(), (TF_debugCallBack *)psVpfuMcsPrintXdbIdxDbgInfo_PartFive, (TF_clearCallBack *)psVpfuMcsClearXdbIdxDbgInfo);
}



//end of file
