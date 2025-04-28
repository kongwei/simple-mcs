/* Started by AICoder, pid:uc8dejd06ejd48d14a8f0b23502e78453a0309cb */
/******************************************************************************
 * 版权所有 (C)2016, 深圳市中兴通讯股份有限公司
 * 模块名          : MCS
 * 文件名          : psNcuNWDAFAddrProc.h
 * 相关文件        :
 * 文件实现功能     : NWDAF地址负荷分担
 * 归属团队        : M6
 * 版本           : V7.24.20
 -------------------------------------------------------------------------------
 * 修改记录:
 * 修改日期           版本号           修改人           修改内容
 * 2024-04-14        V7.24.20        ss            create
 ******************************************************************************/
#ifndef _PS_MCS_NWDAFADDRPROC_H_
#define _PS_MCS_NWDAFADDRPROC_H_
#ifdef __cplusplus
extern "C" {
#endif
/**************************************************************************
 *                              头文件(满足自包含，满足最小依赖)
 **************************************************************************/
#include "ps_pub.h"
#include "ps_typedef.h"
#include "psUpfCommon.h"
/**************************************************************************
 *                              宏(对外提供)
 **************************************************************************/
#define NWDAF_ADDRYTPE_BY_THREAD_IPV6 6
#define NWDAF_ADDRYTPE_BY_THREAD_IPV4 4
#define NWDAF_LINKNUM_MAX          32
/**************************************************************************
 *                              数据类型(对外提供)
 **************************************************************************/


typedef struct tagT_NwdafAddr
{
    T_IPV4 NwdafIPv4;
    T_IPV6 NwdafIPv6;
}T_NwdafAddr;

typedef struct tagT_NwdafAddrbyThread
{
    T_NwdafAddr addr;
    BYTE v4flag : 1;
    BYTE v6flag : 1;
    BYTE Rsv    : 6;
}T_ThreadNwdafAddr;

typedef struct tagT_NwdafAddrList
{
    T_NwdafAddr addr;
    BYTE v4flag : 1;
    BYTE v6flag : 1;
    BYTE Rsv    : 6;
    WORD32 threadnum;
    WORD32 threadno[MEDIA_THRD_NUM];
    WORD32 flag;
}T_NwdafAddrLoad;
/**************************************************************************
 *                              接口(对外提供，最小可见)
 **************************************************************************/
BYTE getAddrByThreadno(WORD32 threadno, BYTE* ipaddr);
void psNcuUpdateNWDAFAddrLoad(T_NwdafStatus nwdadasfAddrStatus[]);
void setNwdafLinkInfoChangeTest(int num, BYTE v6Flag, BYTE v4Flag);
void setNwdafLinkInfoTestLihao(); //删除 lihao
/**************************************************************************
 *                              全局变量声明(对外提供，最小可见)
 **************************************************************************/
extern T_NwdafAddrLoad nwdafaddrload[NWDAF_LINKNUM_MAX] ;
extern T_ThreadNwdafAddr ThreadNwdafAddr[MEDIA_THRD_NUM];
#ifdef __cplusplus
}
#endif
#endif
// end of file
/* Ended by AICoder, pid:uc8dejd06ejd48d14a8f0b23502e78453a0309cb */