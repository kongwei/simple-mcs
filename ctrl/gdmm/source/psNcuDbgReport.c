/******************************************************************************
* 版权所有 (C)2016, 深圳市中兴通讯股份有限公司
*
* 模块名          : MCS
* 文件名          : psNcuDbgCollReg.c
* 相关文件        :
* 文件实现功能     : 一键采集上报
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
#include "psNcuDbgReport.h"
#include "MemShareCfg.h"
#include "psNcuCtrlStatInterface.h"
#include "psMcsDebug.h"
#include "xdb_core_pfu_dbgreprt.h"
#include "ps_db_define_ncu.h"
#include "xdb_core_pfu_dbgreprt.h"
#include "ps_ncu_typedef.h"
#include "upfNcuDmmComm.h"
/**************************************************************************
 *                              宏(本源文件使用)
 **************************************************************************/
#define MCS_MAX_LINE_CONTENT_LEN 256
#define MCS_MAX_TRACE_STRING_LEN 1000

#define CHECK_SNPRINTF_RETURN(val,failRET) if ( 0 > (val)) { return failRET; }

#define CHECK_AND_ADD_ONELINE(ONELINE,TOTAL,SIZE,MAXSIZE,CURRENT)\
do{\
    SIZE = (int)zte_strnlen_s(ONELINE, 255);\
    if(TOTAL + SIZE >= MAXSIZE)\
    {\
        return -1;\
    }\
    TOTAL += SIZE;\
    zte_strncat_s(CURRENT, MCS_MAX_LINE_CONTENT_LEN, ONELINE, MCS_MAX_LINE_CONTENT_LEN);\
    CURRENT += SIZE;\
}while(0)

/**************************************************************************
 *                              常量(本源文件使用)
 **************************************************************************/

/**************************************************************************
 *                              数据类型(本源文件使用)
 **************************************************************************/

/**************************************************************************
 *                              外部函数原型(评估后慎重添加)
 **************************************************************************/
extern VOID ncugsac();
extern VOID ncujobc();
/**************************************************************************
 *                              局部函数原型(本源文件使用)
 **************************************************************************/
WORD32 psNcuMcsReportMcsInfo(T_psDebugVal *ptDebugVal);
WORD16 psNcuMcsReportThreadStat(T_psDebugVal *ptDebugVal, WORD16 *pwDbgValTotalNum);
WORD16 psNcuMcsReportJobStat(T_psDebugVal *ptDebugVal, WORD16 *pwDbgValTotalNum);
/**************************************************************************
 *                              全局变量(本源文件使用)
 **************************************************************************/
T_psDebugVal dbgVal[PS_MAX_DEBUG_VAL_NUM];
/**************************************************************************
 *                              函数实现(函数布局:总分结构|深度优先)
 **************************************************************************/
int psNcuMcsPrintMcsDbgInfo(char *buf,int maxSize)
{
    char oneLine[MCS_MAX_LINE_CONTENT_LEN]  = "";
    WORD32 totalNum                         =  0;
    WORD32 idx                              = 0;
    int size                                = 0;
    int total                               = 0;
    int ret                                 = 0; 
    char *current                           = NULL;
    static T_psDebugVal dbgVal[PS_MAX_DEBUG_VAL_NUM];
    if(NULL == buf)
    {
        return -1;
    }
    current = buf;
    buf[0] = '\0';
    zte_memset_s(dbgVal, sizeof(T_psDebugVal)*PS_MAX_DEBUG_VAL_NUM, 0, sizeof(T_psDebugVal)*PS_MAX_DEBUG_VAL_NUM);

    /* NCU公共信息采集 */
    char *pScVersion = getenv("intra_nfs_version");
    if(NULL!=pScVersion) 
    {
        ret = zte_snprintf_s(oneLine, MCS_MAX_LINE_CONTENT_LEN, "******-  ScVersion : %-36s  -******\n\n", (char *)pScVersion);
        CHECK_SNPRINTF_RETURN(ret,-1);
        CHECK_AND_ADD_ONELINE(oneLine,total,size,maxSize,current);
    }

    /* NCU统计项采集 */
    totalNum = (WORD32)psNcuMcsReportMcsInfo(dbgVal);

    for(idx = 0; (idx < totalNum) && (idx < PS_MAX_DEBUG_VAL_NUM); idx++)
    {
        ret = zte_snprintf_s(oneLine, MCS_MAX_LINE_CONTENT_LEN, "%-56s %llu\n", (char *)dbgVal[idx].cName, dbgVal[idx].w64Val);
        CHECK_SNPRINTF_RETURN(ret,-1);
        CHECK_AND_ADD_ONELINE(oneLine,total,size,maxSize,current);
    }
    return 0;
}

WORD32 psNcuMcsReportMcsInfo(T_psDebugVal *ptDebugVal)
{
    WORD16 totalNum = 0;

    psNcuMcsReportThreadStat(ptDebugVal,&totalNum);
    psNcuMcsReportJobStat(ptDebugVal,&totalNum);

    return (WORD32)totalNum;
}

T_psNcuMcsPerform g_tNcuMcsPerform = {0};
WORD16 psNcuMcsReportThreadStat(T_psDebugVal *ptDebugVal, WORD16 *pwDbgValTotalNum)
{
    if(NULL == g_ptVpfuMcsShareMem || NULL == ptDebugVal || NULL == pwDbgValTotalNum)
    {
        zte_printf_s("g_ptVpfuMcsShareMem is null!\n");
        return 0;
    }

    zte_memset_s(&g_tNcuMcsPerform, sizeof(T_psNcuMcsPerform), 0, sizeof(T_psNcuMcsPerform));
    WORD32 dwID = 0;
    WORD16 wOriginNum = *pwDbgValTotalNum;
    T_psNcuMcsPerform* ptMcsPerform = &g_tNcuMcsPerform;

    for(dwID=0; dwID < MEDIA_THRD_NUM; dwID++)
    {
        WORD32  dwTotal  = sizeof(T_psNcuMcsPerform)/sizeof(WORD64);
        WORD32  dwIndex  = 0;
        WORD64  *pgwSum  = (WORD64*)&g_tNcuMcsPerform;
        WORD64  *pgwTemp = (WORD64*)psVpfuMcsGetPerformPtr(dwID);

        if(NULL == pgwTemp )
        {
            zte_printf_s("g_ptVpfuMcsShareMem is null!\n");
            return 0;
        }

        for(dwIndex = 0; dwIndex < dwTotal;  dwIndex++)
        {
            pgwSum[dwIndex] += pgwTemp[dwIndex];
        }
    }

    #define MCS_MAX_TEMP_CONTENT_LEN 256
    char McsReortStrTemp[MCS_MAX_TEMP_CONTENT_LEN]  = "";

#ifdef MCS_STR_ERR_STAT_ITEM_SEG
#undef MCS_STR_ERR_STAT_ITEM_SEG
#endif
#define MCS_STR_ERR_STAT_ITEM_SEG(seg_name) do \
{ \
    zte_snprintf_s(McsReortStrTemp,MCS_MAX_TEMP_CONTENT_LEN,"---------------------%-14s---------------------",#seg_name); \
    psVpfuDmmReportOneStat(McsReortStrTemp, 0, ptDebugVal, pwDbgValTotalNum); \
}while(0);


#ifdef MCS_STR_ERR_STAT_ITEM
#undef MCS_STR_ERR_STAT_ITEM
#endif
#define MCS_STR_ERR_STAT_ITEM(item_name) do \
{ \
    if(ptMcsPerform->item_name>0) \
    { \
        zte_snprintf_s(McsReortStrTemp,MCS_MAX_TEMP_CONTENT_LEN,"%s",#item_name); \
        psVpfuDmmReportOneStat(McsReortStrTemp, ptMcsPerform->item_name, ptDebugVal, pwDbgValTotalNum); \
    } \
}while(0);

#ifdef MCS_STR_ERR_STAT_ITEM_ARRAY
#undef MCS_STR_ERR_STAT_ITEM_ARRAY
#endif
#define MCS_STR_ERR_STAT_ITEM_ARRAY(item_name, array_size) do \
{ \
    int i=0; \
    for(i=0; i<array_size; i++) \
    { \
        if(ptMcsPerform->item_name[i]>0) \
        { \
            zte_snprintf_s(McsReortStrTemp,MCS_MAX_TEMP_CONTENT_LEN,"%s[%2d]",#item_name,i); \
            psVpfuDmmReportOneStat(McsReortStrTemp, ptMcsPerform->item_name[i], ptDebugVal, pwDbgValTotalNum); \
        } \
    } \
}while(0);

#ifdef MCS_STR_ERR_STAT_ITEM_TWO_ARRAY
#undef MCS_STR_ERR_STAT_ITEM_TWO_ARRAY
#endif
#define MCS_STR_ERR_STAT_ITEM_TWO_ARRAY(item_name, array_size_fir, array_size_sec) do \
{ \
    int i=0; \
    int j=0; \
    for(i=0; i<array_size_fir; i++) \
        for(j=0;j<array_size_sec;j++) \
        { \
            if(ptMcsPerform->item_name[i][j]>0) \
            { \
                zte_snprintf_s(McsReortStrTemp,MCS_MAX_TEMP_CONTENT_LEN,"%s[%2d][%2d]", #item_name, i, j); \
                psVpfuDmmReportOneStat(McsReortStrTemp, ptMcsPerform->item_name[i][j], ptDebugVal, pwDbgValTotalNum); \
            } \
        } \
}while(0);

#include "psMcsStatItemDef.h"

    return *pwDbgValTotalNum - wOriginNum;
}

WORD16 psNcuMcsReportJobStat(T_psDebugVal *ptDebugVal, WORD16 *pwDbgValTotalNum)
{
    if(NULL == ptDebugVal || NULL == pwDbgValTotalNum)
    {
        return 0;
    }

    WORD16 wOriginNum = *pwDbgValTotalNum;
    psVpfuDmmReportOneStat("-------------------       MCS JOB        -------------------", 0, ptDebugVal, pwDbgValTotalNum);

    #define MCS_MAX_TEMP_CONTENT_LEN 256
    char McsReortStrTemp[MCS_MAX_TEMP_CONTENT_LEN]  = "";

#ifdef MSC_STAT_ITEM_SEG
#undef MSC_STAT_ITEM_SEG
#endif
#define MSC_STAT_ITEM_SEG(seg_name) do \
{ \
    zte_snprintf_s(McsReortStrTemp,MCS_MAX_TEMP_CONTENT_LEN,"---------------------%-14s---------------------",#seg_name); \
    psVpfuDmmReportOneStat(McsReortStrTemp, 0, ptDebugVal, pwDbgValTotalNum); \
}while(0);


#ifdef MSC_STAT_ITEM
#undef MSC_STAT_ITEM
#endif
#define MSC_STAT_ITEM(item_name) do \
{ \
    if(g_tNcuMcsJobStat.item_name>0) \
    { \
        zte_snprintf_s(McsReortStrTemp,MCS_MAX_TEMP_CONTENT_LEN,"%s",#item_name); \
        psVpfuDmmReportOneStat(McsReortStrTemp, g_tNcuMcsJobStat.item_name, ptDebugVal, pwDbgValTotalNum); \
    } \
}while(0);

#ifdef MSC_STAT_ITEM_ARRAY
#undef MSC_STAT_ITEM_ARRAY
#endif
#define MSC_STAT_ITEM_ARRAY(item_name, array_size) do \
{ \
    int i=0; \
    for(i=0; i<array_size; i++) \
    { \
        if(g_tNcuMcsJobStat.item_name[i]>0) \
        { \
            zte_snprintf_s(McsReortStrTemp,MCS_MAX_TEMP_CONTENT_LEN,"%s[%2d]",#item_name,i); \
            psVpfuDmmReportOneStat(McsReortStrTemp, g_tNcuMcsJobStat.item_name[i], ptDebugVal, pwDbgValTotalNum); \
        } \
    } \
}while(0);

#include "psNcuCtrlStatItemDef.h"

    return *pwDbgValTotalNum - wOriginNum;
}

int psNcuMcsClearMcsDbgInfo(void)
{
    ncugsac();
    ncujobc();
    return 0;
}

WORD32 psVpfuMcsReportXdbAccDbgInfo(T_psDebugVal *ptDebugVal)
{
    WORD16 totalNum =0;

    xdbAccShowDebugCallback(ptDebugVal,&totalNum);
    return (WORD32)totalNum;
}

WORD32 psVpfuMcsReportXdbIdxDbgInfo(T_psDebugVal *ptDebugVal, DWORD dwstartthrd, DWORD dwendthrd)
{
    WORD16 totalNum =0;

    xdbIdxShowDebugbyThrdCallback(ptDebugVal,&totalNum, dwstartthrd, dwendthrd);
    return (WORD32)totalNum;
}

WORD32 psVpfuMcsReportXdbDbgInfo(T_psDebugVal *ptDebugVal, DWORD dwstartthrd, DWORD dwendthrd)
{
    WORD16 totalNum =0;

    xdbShowDebugbyThrdCallback(ptDebugVal,&totalNum, dwstartthrd, dwendthrd);
    return (WORD32)totalNum;
}

/* Started by AICoder, pid:h5a1a9c245we09d14dca0882709a0f32b0f81b3f */
int psVpfuMcsWriteXdbDbgInfo(char *buf, int maxSize, T_psDebugVal *ptDebugVal, WORD32 totalNum)
{
    if(NULL == buf || NULL == ptDebugVal)
    {
        return -1;
    }
    buf[0] = '\0';

    char oneLine[MCS_MAX_LINE_CONTENT_LEN] = "";
    char *current = buf;
    WORD32    idx = 0;
    int      size = 0;
    int     total = 0;
    int      ilen = 0;

    for(idx = 0; (idx < PS_MAX_DEBUG_VAL_NUM) && (idx < totalNum); idx++)
    {
        //目前最长的统计项是qwPktProcErrStat[UL][MCS_ERR_6in46to4_v4HeadEncapErr],长度是53
        ilen = zte_snprintf_s(oneLine, MCS_MAX_LINE_CONTENT_LEN, "%-56s %llu\n", (char *)&ptDebugVal[idx].cName, ptDebugVal[idx].w64Val);
        if(ilen < 0)
        {
            break;
        }

        size = zte_strnlen_s(oneLine, MCS_MAX_LINE_CONTENT_LEN-1);
        if(total + size >= maxSize)
        {
            break;
        }

        total += size;
        zte_strncat_s(current,maxSize, oneLine, size); /*lint !e732 */
        current += size;
    }
    return 0;
}
/* Ended by AICoder, pid:h5a1a9c245we09d14dca0882709a0f32b0f81b3f */

/* Started by AICoder, pid:ec44b84c7d7ac2d14c29097040691e4ff620eb1a */
int psVpfuMcsPrintXdbAccDbgInfo(char *buf,int maxSize)
{
    if(NULL == buf)
    {
        return -1;
    }
    buf[0] = '\0';

    char oneLine[MCS_MAX_LINE_CONTENT_LEN] = "";
    char   *current = buf;
    WORD32 totalNum = 0;
    WORD32      idx = 0;
    int        size = 0;
    int       total = 0;
    int        ilen = 0;
    static T_psDebugVal dbgVal[PS_MAX_DEBUG_VAL_NUM];
    zte_memset_s(dbgVal, sizeof(T_psDebugVal)*PS_MAX_DEBUG_VAL_NUM, 0, sizeof(T_psDebugVal)*PS_MAX_DEBUG_VAL_NUM);

    totalNum = (WORD32)psVpfuMcsReportXdbAccDbgInfo(dbgVal);
    for(idx = 0; (idx < PS_MAX_DEBUG_VAL_NUM) && (idx < totalNum); idx++)
    {
        //目前最长的统计项是qwPktProcErrStat[UL][MCS_ERR_6in46to4_v4HeadEncapErr],长度是53
        ilen = zte_snprintf_s(oneLine, MCS_MAX_LINE_CONTENT_LEN, "%-56s %llu\n", (char *)&dbgVal[idx].cName, dbgVal[idx].w64Val);
        if(ilen < 0)
        {
            break;
        }

        size = zte_strnlen_s(oneLine,MCS_MAX_LINE_CONTENT_LEN-1);
        if(total + size >= maxSize)
        {
            break;
        }

        total += size;
        zte_strncat_s(current,maxSize, oneLine, size); /*lint !e732 */
        current += size;
    }
    return 0;
}
/* Ended by AICoder, pid:ec44b84c7d7ac2d14c29097040691e4ff620eb1a */

/* Started by AICoder, pid:e6ed44891fo4f89149410b12502a6d196870daeb */
int psVpfuMcsPrintXdbDbgInfo_PartOne(char *buf, int maxSize)
{
    WORD32 totalNum = 0;
    zte_memset_s(dbgVal, sizeof(T_psDebugVal)*PS_MAX_DEBUG_VAL_NUM, 0, sizeof(T_psDebugVal)*PS_MAX_DEBUG_VAL_NUM);
    totalNum = (WORD32)psVpfuMcsReportXdbDbgInfo(dbgVal, 0, 9);
    psVpfuMcsWriteXdbDbgInfo(buf, maxSize, dbgVal, totalNum);
    return 0;
}
/* Ended by AICoder, pid:e6ed44891fo4f89149410b12502a6d196870daeb */

int psVpfuMcsPrintXdbDbgInfo_PartTwo(char *buf,int maxSize)
{
    WORD32 totalNum = 0;
    zte_memset_s(dbgVal, sizeof(T_psDebugVal)*PS_MAX_DEBUG_VAL_NUM, 0, sizeof(T_psDebugVal)*PS_MAX_DEBUG_VAL_NUM);
    totalNum = (WORD32)psVpfuMcsReportXdbDbgInfo(dbgVal, 10, 19);
    psVpfuMcsWriteXdbDbgInfo(buf,maxSize, dbgVal, totalNum);
    return 0;
}

int psVpfuMcsPrintXdbDbgInfo_PartThree(char *buf,int maxSize)
{
    WORD32 totalNum = 0;
    zte_memset_s(dbgVal, sizeof(T_psDebugVal)*PS_MAX_DEBUG_VAL_NUM, 0, sizeof(T_psDebugVal)*PS_MAX_DEBUG_VAL_NUM);
    totalNum = (WORD32)psVpfuMcsReportXdbDbgInfo(dbgVal, 20, 29);
    psVpfuMcsWriteXdbDbgInfo(buf,maxSize, dbgVal, totalNum);
    return 0;
}

int psVpfuMcsPrintXdbDbgInfo_PartFour(char *buf,int maxSize)
{
    WORD32 totalNum = 0;
    zte_memset_s(dbgVal, sizeof(T_psDebugVal)*PS_MAX_DEBUG_VAL_NUM, 0, sizeof(T_psDebugVal)*PS_MAX_DEBUG_VAL_NUM);
    totalNum = (WORD32)psVpfuMcsReportXdbDbgInfo(dbgVal, 30, 39);
    psVpfuMcsWriteXdbDbgInfo(buf,maxSize, dbgVal, totalNum);
    return 0;
}

int psVpfuMcsPrintXdbDbgInfo_PartFive(char *buf,int maxSize)
{
    WORD32 totalNum = 0;
    zte_memset_s(dbgVal, sizeof(T_psDebugVal)*PS_MAX_DEBUG_VAL_NUM, 0, sizeof(T_psDebugVal)*PS_MAX_DEBUG_VAL_NUM);
    totalNum = (WORD32)psVpfuMcsReportXdbDbgInfo(dbgVal, 40, XDB_NCU_DB_NUM);
    psVpfuMcsWriteXdbDbgInfo(buf,maxSize, dbgVal, totalNum);
    return 0;
}

int psVpfuMcsPrintXdbIdxDbgInfo_PartOne(char *buf,int maxSize)
{
    WORD32 totalNum = 0;
    zte_memset_s(dbgVal, sizeof(T_psDebugVal)*PS_MAX_DEBUG_VAL_NUM, 0, sizeof(T_psDebugVal)*PS_MAX_DEBUG_VAL_NUM);
    totalNum = (WORD32)psVpfuMcsReportXdbIdxDbgInfo(dbgVal, 0, 9);
    psVpfuMcsWriteXdbDbgInfo(buf, maxSize, dbgVal, totalNum);
    return 0;
}

int psVpfuMcsPrintXdbIdxDbgInfo_PartTwo(char *buf,int maxSize)
{
    WORD32 totalNum = 0;
    zte_memset_s(dbgVal, sizeof(T_psDebugVal)*PS_MAX_DEBUG_VAL_NUM, 0, sizeof(T_psDebugVal)*PS_MAX_DEBUG_VAL_NUM);
    totalNum = (WORD32)psVpfuMcsReportXdbIdxDbgInfo(dbgVal, 10, 19);
    psVpfuMcsWriteXdbDbgInfo(buf, maxSize, dbgVal, totalNum);
    return 0;
}

int psVpfuMcsPrintXdbIdxDbgInfo_PartThree(char *buf,int maxSize)
{
    WORD32 totalNum = 0;
    zte_memset_s(dbgVal, sizeof(T_psDebugVal)*PS_MAX_DEBUG_VAL_NUM, 0, sizeof(T_psDebugVal)*PS_MAX_DEBUG_VAL_NUM);
    totalNum = (WORD32)psVpfuMcsReportXdbIdxDbgInfo(dbgVal, 20, 29);
    psVpfuMcsWriteXdbDbgInfo(buf, maxSize, dbgVal, totalNum);
    return 0;
}

int psVpfuMcsPrintXdbIdxDbgInfo_PartFour(char *buf,int maxSize)
{
    WORD32 totalNum = 0;
    zte_memset_s(dbgVal, sizeof(T_psDebugVal)*PS_MAX_DEBUG_VAL_NUM, 0, sizeof(T_psDebugVal)*PS_MAX_DEBUG_VAL_NUM);
    totalNum = (WORD32)psVpfuMcsReportXdbIdxDbgInfo(dbgVal, 30, 39);
    psVpfuMcsWriteXdbDbgInfo(buf, maxSize, dbgVal, totalNum);
    return 0;
}

int psVpfuMcsPrintXdbIdxDbgInfo_PartFive(char *buf,int maxSize)
{
    WORD32 totalNum = 0;
    zte_memset_s(dbgVal, sizeof(T_psDebugVal)*PS_MAX_DEBUG_VAL_NUM, 0, sizeof(T_psDebugVal)*PS_MAX_DEBUG_VAL_NUM);
    totalNum = (WORD32)psVpfuMcsReportXdbIdxDbgInfo(dbgVal, 40, XDB_NCU_DB_NUM);
    psVpfuMcsWriteXdbDbgInfo(buf, maxSize, dbgVal, totalNum);
    return 0;
}

void psVpfuMcsClearXdbDbgInfo(void)
{
    xdbClearDebugCallback();
    return;
}

void psVpfuMcsClearXdbAccDbgInfo(void)
{
    xdbAccClearDebugCallback();
    return;
}

void psVpfuMcsClearXdbIdxDbgInfo(void)
{
    xdbIdxClearDebugCallback();
    return;
}
//end of file
