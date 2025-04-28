/******************************************************************************
* 版权所有 (C)2016, 深圳市中兴通讯股份有限公司
*
* 模块名          : MCS
* 文件名          : psNcuDbgReport.h
* 相关文件        :
* 文件实现功能     : 一键采集上报头文件
* 归属团队        : M6
* 版本           : V1.0
-------------------------------------------------------------------------------
* 修改记录:
* 修改日期            版本号           修改人           修改内容
* 2024-4-12        V7.24.20        wya            create
******************************************************************************/
#ifndef  _PS_NCU_DBG_REPORT_H_
#define  _PS_NCU_DBG_REPORT_H_
#ifdef __cplusplus
extern "C" {
#endif
/**************************************************************************
 *                              头文件(满足自包含)
 **************************************************************************/
#include "tulip.h"
#include "ps_struct_pub.h"
/**************************************************************************
 *                              宏(对外提供)
 **************************************************************************/

/**************************************************************************
 *                              数据类型(对外提供)
 **************************************************************************/

/**************************************************************************
 *                              接口(对外提供，最小可见)
 **************************************************************************/
int psNcuMcsPrintMcsDbgInfo(char *buf,int maxSize);
int psNcuMcsClearMcsDbgInfo(void);
int psVpfuMcsPrintLdbDbgInfo(char *buf,int maxSize);
int psVpfuMcsPrintXdbDbgInfo_PartOne(char *buf,int maxSize);
int psVpfuMcsPrintXdbDbgInfo_PartTwo(char *buf,int maxSize);
int psVpfuMcsPrintXdbDbgInfo_PartThree(char *buf,int maxSize);
int psVpfuMcsPrintXdbDbgInfo_PartFour(char *buf,int maxSize);
int psVpfuMcsPrintXdbDbgInfo_PartFive(char *buf,int maxSize);
int psVpfuMcsPrintXdbAccDbgInfo(char *buf,int maxSize);
int psVpfuMcsPrintXdbIdxDbgInfo_PartOne(char *buf,int maxSize);
int psVpfuMcsPrintXdbIdxDbgInfo_PartTwo(char *buf,int maxSize);
int psVpfuMcsPrintXdbIdxDbgInfo_PartThree(char *buf,int maxSize);
int psVpfuMcsPrintXdbIdxDbgInfo_PartFour(char *buf,int maxSize);
int psVpfuMcsPrintXdbIdxDbgInfo_PartFive(char *buf,int maxSize);
void psVpfuMcsClearXdbDbgInfo(void);
void psVpfuMcsClearXdbAccDbgInfo(void);
void psVpfuMcsClearXdbIdxDbgInfo(void);
/**************************************************************************
 *                              全局变量声明(对外提供，最小可见)
 **************************************************************************/

#ifdef __cplusplus
}
#endif
#endif
//end of file
