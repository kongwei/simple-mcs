/* Started by AICoder, pid:z4e4fya689gd27e1447f0b3de001c85205b6724e */
/******************************************************************************
 * 版权所有 (C)2016 深圳市中兴通讯股份有限公司*
 * 模块名          : MCS
 * 文件名          : psNcuNWDAFAddrProc.c
 * 相关文件        :
 * 文件实现功能     : NWDAF地址负荷分担
 * 归属团队        : M6
 * 版本           : V7.24.20
-------------------------------------------------------------------------------
* 修改记录:
* 修改日期            版本号           修改人           修改内容
* 2024-04-14        V7.24.20        ss            create
******************************************************************************/
/**************************************************************************
 *                              头文件(满足最小依赖请按照DDD分层架构逐层依赖)
 **************************************************************************/
// application    层依赖
#include "psNcuNWDAFAddrProc.h"
#include "psNcuHeartBeatProc.h"
#include "psMcsGlobalCfg.h"
#include "McsIPv4Head.h"
#include "McsIPv6Head.h"
// 非DDD目录依赖
#include "psMcsDebug.h"
#include "psNcuDataEncode.h"
/**************************************************************************
 *                              宏(本源文件使用)
 **************************************************************************/

#undef DEBUG_TRACE
BYTE g_ncu_nwdafload = 0xff;
#define DEBUG_TRACE(level, ...) \
do { \
    if (unlikely(level >= g_ncu_nwdafload)) { \
        zte_printf_s("\n[MCS:%-30s:%u] ", __FUNCTION__, __LINE__); \
        zte_printf_s(__VA_ARGS__); \
    } \
} while (0)

/**************************************************************************
 *                              常量(本源文件使用)
 **************************************************************************/
T_NwdafAddrLoad nwdafaddrload[NWDAF_LINKNUM_MAX] = { 0 };
T_ThreadNwdafAddr ThreadNwdafAddr[MEDIA_THRD_NUM] = { 0 };
extern T_NwdafStatus g_nwdaflinkstatus[NWDAF_LINKNUM_MAX];
/**************************************************************************
 *                              数据类型(本源文件使用)
 **************************************************************************/

/**************************************************************************
 *                              外部函数原型(评估后慎重添加)
 **************************************************************************/

/**************************************************************************
 *                              局部函数原型(本源文件使用)
 **************************************************************************/
void psNcuAllocAddrforThread(BYTE bIndexPos, void* ipaddr);
BYTE psNcuChooseAddrforThread();
BYTE psNcuNWDAFAddrStatusExist(T_NwdafStatus* ptnwdadasfAddrStatus, WORD32* ptThreadNeedFree, BYTE* ptIndexPos ,BYTE* bcanUsed);
BYTE psNcuNWDAFAddrloadExist(T_NwdafStatus ptnwdadasfAddrStatus[], T_NwdafAddrLoad* ptnwdadasfAddrLoad);
void psShowNWDAFAddrInfo();
/**************************************************************************
 *                              全局变量(本源文件使用)
 **************************************************************************/

/**************************************************************************
 *                              函数实现(函数布局:总分结构|深度优先)
 **************************************************************************/

void psNcuUpdateNWDAFAddrLoad(T_NwdafStatus nwdadasfAddrStatus[])
{
    T_psNcuMcsPerform *ptNcuPerform = psVpfuMcsGetPerformPtr(0);
    if(NULL == ptNcuPerform)
    {
        DEBUG_TRACE(DEBUG_LOW,"ptNcuPerform Null !");
        return;
    }
    int i = 0;
    // 清空nwdafaddrload所有flag
    for (i = 0; i < NWDAF_LINKNUM_MAX; i++)
    {
        nwdafaddrload[i].flag = 0;
    }

    WORD32 thread_free[MEDIA_THRD_NUM] = { 0 };
    BYTE bIndexPos = 0;
    
    
    BYTE bNotFirstFlag = 1;
    BYTE bcanUsed = 0;

    if (DEBUG_LOW >= g_ncu_nwdafload)
    {
        psShowNWDAFAddrInfo();
    }

    for (i = 0; i < NWDAF_LINKNUM_MAX; i++)
    {
        if (FALSE == psCheckIPValid(nwdadasfAddrStatus[i].tNwdafAddr.NwdafIPv4, IPv4_VERSION)
            && FALSE == psCheckIPValid(nwdadasfAddrStatus[i].tNwdafAddr.NwdafIPv6, IPv6_VERSION))
        {
            continue;
        }


        DEBUG_TRACE(DEBUG_LOW, "nwdadasfAddrStatus[%u]! Link[%u,%u], Addr[%u,%u,%u]\n",
            i, nwdadasfAddrStatus[i].bLinkStatus, nwdadasfAddrStatus[i].bLinkIPType,
            nwdadasfAddrStatus[i].tNwdafAddr.bNwdafIPType, nwdadasfAddrStatus[i].tNwdafAddr.bIPv4LinkStatus, nwdadasfAddrStatus[i].tNwdafAddr.bIPv6LinkStatus);

        if (FALSE == psNcuNWDAFAddrStatusExist(&nwdadasfAddrStatus[i], thread_free, &bIndexPos, &bcanUsed))
        {
            bNotFirstFlag = 0;
        }

        DEBUG_TRACE(DEBUG_LOW, "bIndexPos[%u]! bcanUsed[%u] bNotFirstFlag[%u]\n", bIndexPos, bcanUsed, bNotFirstFlag);
    }

    if (DEBUG_LOW >= g_ncu_nwdafload)
    {
        psShowNWDAFAddrInfo();
    }
    if(0 == bcanUsed)
    {
        DEBUG_TRACE(DEBUG_LOW, "can used num = 0\n");
        MCS_LOC_STAT_EX(ptNcuPerform, qwNWDAFAddrListNull, 1);
        zte_memset_s(nwdafaddrload, sizeof(T_NwdafAddrLoad)*NWDAF_LINKNUM_MAX, 0, sizeof(T_NwdafAddrLoad)*NWDAF_LINKNUM_MAX);
        zte_memset_s(ThreadNwdafAddr, sizeof(T_ThreadNwdafAddr)*MEDIA_THRD_NUM, 0, sizeof(T_ThreadNwdafAddr)*MEDIA_THRD_NUM);
        return;
    }

    // 处理删除地址的情况
    BYTE bFoundFlag = 0;
    BYTE bIndexLoad = 0;
    while(bIndexLoad < NWDAF_LINKNUM_MAX)
    {
        bFoundFlag = psNcuNWDAFAddrloadExist(nwdadasfAddrStatus, &nwdafaddrload[bIndexLoad]);
    
        if(bFoundFlag == FALSE)
        {
            DEBUG_TRACE(DEBUG_LOW, "not find nwdafaddrload[%u]\n", bIndexLoad);
            zte_memcpy_s(&thread_free[bIndexPos/sizeof(WORD32)],nwdafaddrload[bIndexLoad].threadnum*sizeof(WORD32), &nwdafaddrload[bIndexLoad].threadno, nwdafaddrload[bIndexLoad].threadnum*sizeof(WORD32));
            bIndexPos = bIndexPos + nwdafaddrload[bIndexLoad].threadnum*sizeof(WORD32);
            zte_memset_s((BYTE*)&nwdafaddrload[bIndexLoad],sizeof(T_NwdafAddrLoad),0,sizeof(T_NwdafAddrLoad));
        }
        
        bIndexLoad++;
    }    

    if (DEBUG_LOW >= g_ncu_nwdafload)
    {
        psShowNWDAFAddrInfo();
    }
    // 第一次分配 / 重新分配
    BYTE bThreadNumMin = 0;
    BYTE bthreadnotemp = 0;
    BYTE bthreadnofree = 0;
    DEBUG_TRACE(DEBUG_LOW, "bNotFirstFlag[%u]\n", bNotFirstFlag);
    if(bNotFirstFlag)
    {
        for (i = 0; i < bIndexPos/sizeof(WORD32); i++)
        {
            bThreadNumMin = psNcuChooseAddrforThread();
            bthreadnotemp = nwdafaddrload[bThreadNumMin].threadnum;
            bthreadnofree = thread_free[i];
            nwdafaddrload[bThreadNumMin].threadno[bthreadnotemp] = bthreadnofree;
            nwdafaddrload[bThreadNumMin].threadnum++;
            zte_memcpy_s((BYTE*)&ThreadNwdafAddr[bthreadnofree].addr,sizeof(T_NwdafAddr),(BYTE*)&nwdafaddrload[bThreadNumMin].addr,sizeof(T_NwdafAddr));
            ThreadNwdafAddr[bthreadnofree].v4flag = nwdafaddrload[bThreadNumMin].v4flag;
            ThreadNwdafAddr[bthreadnofree].v6flag = nwdafaddrload[bThreadNumMin].v6flag;
        }
    }
    else
    {
        for (i = 0; i < NWDAF_LINKNUM_MAX; i++)
        {
            nwdafaddrload[i].threadnum = 0;
            zte_memset_s(&nwdafaddrload[i].threadno, NWDAF_LINKNUM_MAX, 0, NWDAF_LINKNUM_MAX);
        }
        for (i = 0; i < MEDIA_THRD_NUM; i++)
        {
            bThreadNumMin = psNcuChooseAddrforThread();
            bthreadnotemp = nwdafaddrload[bThreadNumMin].threadnum;
            nwdafaddrload[bThreadNumMin].threadno[bthreadnotemp] = i;
            nwdafaddrload[bThreadNumMin].threadnum++;
            zte_memcpy_s((BYTE*)&ThreadNwdafAddr[i].addr, sizeof(T_NwdafAddr), (BYTE*)&nwdafaddrload[bThreadNumMin].addr, sizeof(T_NwdafAddr));
            ThreadNwdafAddr[i].v4flag = nwdafaddrload[bThreadNumMin].v4flag;
            ThreadNwdafAddr[i].v6flag = nwdafaddrload[bThreadNumMin].v6flag;
        }
    }
    return;
}

//给线程分配地址
void psNcuAllocAddrforThread(BYTE bIndexPos, void* ipaddr)
{
    if(NULL == ipaddr)
    {
        return ;
    }
    BYTE bThreadNumMin = 0;
    BYTE bthreadno = 0;
    int i = 0;
    for (i = 0; i < bIndexPos; i++)
    {
        bThreadNumMin = psNcuChooseAddrforThread();
        bthreadno = nwdafaddrload[bThreadNumMin].threadnum;
        nwdafaddrload[bThreadNumMin].threadno[bthreadno] = i;
        nwdafaddrload[bThreadNumMin].threadnum++;
        zte_memcpy_s((BYTE*)&ThreadNwdafAddr[i].addr,sizeof(T_NwdafAddr),(BYTE*)&nwdafaddrload[bThreadNumMin].addr,sizeof(T_NwdafAddr));
        ThreadNwdafAddr[i].v4flag = nwdafaddrload[bThreadNumMin].v4flag;
        ThreadNwdafAddr[i].v6flag = nwdafaddrload[bThreadNumMin].v6flag;
    }
    return;
}

BYTE getAddrByThreadno(WORD32 threadno, BYTE* ipaddr)
{
    T_psNcuMcsPerform *ptNcuPerform = psVpfuMcsGetPerformPtr(0);
    if(NULL == ptNcuPerform || NULL == ipaddr)
    {
        DEBUG_TRACE(DEBUG_LOW,"getAddrByThreadno para Null !");
        return 0;
    }
    
    if(g_upfConfig.daUpfIpCfg.bPreferAddrType == 1)
    {
        if(ThreadNwdafAddr[threadno].v6flag == 1)
        {
            IP6_COPY((BYTE*)ipaddr, (BYTE*)&ThreadNwdafAddr[threadno].addr.NwdafIPv6);
            return NWDAF_ADDRYTPE_BY_THREAD_IPV6;
        }
        else if(ThreadNwdafAddr[threadno].v4flag == 1)
        {
            IP4_COPY((BYTE*)ipaddr, (BYTE*)&ThreadNwdafAddr[threadno].addr.NwdafIPv4);
            return NWDAF_ADDRYTPE_BY_THREAD_IPV4;
        }
        else
        {            
            MCS_LOC_STAT_EX(ptNcuPerform, qwNWDAFAddrListgetFail, 1);
        }
    }
    else
    {
        if(ThreadNwdafAddr[threadno].v4flag == 1)
        {
            IP4_COPY((BYTE*)ipaddr, (BYTE*)&ThreadNwdafAddr[threadno].addr.NwdafIPv4);
            return NWDAF_ADDRYTPE_BY_THREAD_IPV4;
        }
        else if(ThreadNwdafAddr[threadno].v6flag == 1)
        {
            IP6_COPY((BYTE*)ipaddr, (BYTE*)&ThreadNwdafAddr[threadno].addr.NwdafIPv6);
            return NWDAF_ADDRYTPE_BY_THREAD_IPV6;
        }
        else
        {
            MCS_LOC_STAT_EX(ptNcuPerform, qwNWDAFAddrListgetFail, 1);
        }
    }
    return 0;
}


BYTE psNcuChooseAddrforThread()
{
    BYTE IndexLoad = 0;
    BYTE IndexLoadMin = 0;
    BYTE bThreadNumMin = MEDIA_THRD_NUM - 1;
    while(IndexLoad < NWDAF_LINKNUM_MAX)
    {
        if(nwdafaddrload[IndexLoad].flag == 1 && nwdafaddrload[IndexLoad].threadnum < bThreadNumMin)
        {
            bThreadNumMin = nwdafaddrload[IndexLoad].threadnum;
            IndexLoadMin = IndexLoad;
        }
        IndexLoad++;
    }
    return IndexLoadMin;
}

//判断 nwdadasfAddrStatus[i] 是否存在于 nwdafaddrload
BYTE psNcuNWDAFAddrStatusExist(T_NwdafStatus* ptnwdadasfAddrStatus, WORD32* ptThreadNeedFree, BYTE* ptIndexPos,BYTE* bcanUsed)
{
    if (NULL == ptnwdadasfAddrStatus || NULL == ptThreadNeedFree || NULL == ptIndexPos || NULL == bcanUsed)
    {
        return FALSE;
    }

    BYTE bFoundFlag = 0;
    BYTE bFoundIndex = 0;
    BYTE IndexLoad = 0;
    BYTE bIndexThr = 0;
    while(IndexLoad < NWDAF_LINKNUM_MAX)
    {
        if (IPV4_EQUAL(ptnwdadasfAddrStatus->tNwdafAddr.NwdafIPv4, nwdafaddrload[IndexLoad].addr.NwdafIPv4) &&
            (0 == memcmp((BYTE*)&ptnwdadasfAddrStatus->tNwdafAddr.NwdafIPv6, (BYTE*)&nwdafaddrload[IndexLoad].addr.NwdafIPv6, IPV6_LEN)))
        {
            DEBUG_TRACE(DEBUG_LOW, "find in nwdafaddrload[%u]! bLinkStatus[%u]\n", IndexLoad, ptnwdadasfAddrStatus->bLinkStatus);
            if (ptnwdadasfAddrStatus->bLinkStatus == NWDAF_LINKSTATE_UP)//判断状态
            {
                nwdafaddrload[IndexLoad].flag = 1;
                nwdafaddrload[IndexLoad].v4flag = ptnwdadasfAddrStatus->tNwdafAddr.bIPv4LinkStatus;
                nwdafaddrload[IndexLoad].v6flag = ptnwdadasfAddrStatus->tNwdafAddr.bIPv6LinkStatus;
                for (bIndexThr = 0; bIndexThr < nwdafaddrload[IndexLoad].threadnum; bIndexThr++)
                {//如果双栈情况下有一个地址down了，需要更新状态
                    ThreadNwdafAddr[nwdafaddrload[IndexLoad].threadno[bIndexThr]].v4flag = nwdafaddrload[IndexLoad].v4flag;
                    ThreadNwdafAddr[nwdafaddrload[IndexLoad].threadno[bIndexThr]].v6flag = nwdafaddrload[IndexLoad].v6flag;
                }
                (*bcanUsed)++;
            }
            else
            {
                zte_memcpy_s(&(ptThreadNeedFree[*ptIndexPos / sizeof(WORD32)]), nwdafaddrload[IndexLoad].threadnum * sizeof(WORD32), &nwdafaddrload[IndexLoad].threadno, nwdafaddrload[IndexLoad].threadnum * sizeof(WORD32));
                *ptIndexPos = *ptIndexPos + nwdafaddrload[IndexLoad].threadnum * sizeof(WORD32);
                zte_memset_s((BYTE*)&nwdafaddrload[IndexLoad], sizeof(T_NwdafAddrLoad), 0, sizeof(T_NwdafAddrLoad));
            }
            return TRUE;
        }
        if(bFoundFlag == 0 && nwdafaddrload[IndexLoad].flag == 0)
        {
            bFoundFlag = 1;
            bFoundIndex = IndexLoad;
            DEBUG_TRACE(DEBUG_LOW, "not find in nwdafaddrload! bFoundIndex[%u]\n", bFoundIndex);
        }
        IndexLoad++;
    }

    if (ptnwdadasfAddrStatus->bLinkStatus == NWDAF_LINKSTATE_UP)//判断状态
    {
        DEBUG_TRACE(DEBUG_LOW, "update nwdafaddrload[%u]\n", bFoundIndex);
        nwdafaddrload[bFoundIndex].flag = 1;
        nwdafaddrload[bFoundIndex].v4flag = ptnwdadasfAddrStatus->tNwdafAddr.bIPv4LinkStatus;
        nwdafaddrload[bFoundIndex].v6flag = ptnwdadasfAddrStatus->tNwdafAddr.bIPv6LinkStatus;
        IP4_COPY(nwdafaddrload[bFoundIndex].addr.NwdafIPv4, ptnwdadasfAddrStatus->tNwdafAddr.NwdafIPv4);
        IP6_COPY(nwdafaddrload[bFoundIndex].addr.NwdafIPv6, ptnwdadasfAddrStatus->tNwdafAddr.NwdafIPv6);
        (*bcanUsed)++;
        //重新分配
        int i = 0;
        for (i = 0; i < NWDAF_LINKNUM_MAX; i++)
        {
            nwdafaddrload[i].threadnum = 0;
            zte_memset_s((BYTE*)&nwdafaddrload[i].threadno, MEDIA_THRD_NUM * sizeof(WORD32), 0, MEDIA_THRD_NUM * sizeof(WORD32));
        }
        return FALSE;
    }
    return FALSE;
}

//判断 nwdafaddrload 是否存在于 nwdadasfAddrStatus[i]
BYTE psNcuNWDAFAddrloadExist(T_NwdafStatus ptnwdadasfAddrStatus[], T_NwdafAddrLoad* ptnwdadasfAddrLoad)
{
    if(NULL == ptnwdadasfAddrLoad)
    {
        return FALSE;
    }
    BYTE bIndexStatus = 0;

    while(bIndexStatus < NWDAF_LINKNUM_MAX)
    {
        if(IPV4_EQUAL(ptnwdadasfAddrStatus[bIndexStatus].tNwdafAddr.NwdafIPv4,ptnwdadasfAddrLoad->addr.NwdafIPv4)&&
        (0 == memcmp((BYTE*)&ptnwdadasfAddrStatus[bIndexStatus].tNwdafAddr.NwdafIPv6,(BYTE*)&ptnwdadasfAddrLoad->addr.NwdafIPv6,IPV6_LEN)))
        {
            return TRUE;
        }

        bIndexStatus++;        
    }

    return FALSE;    
}

/*以下是调试函数*/
/* Started by AICoder, pid:cfc56ga335f6a7e14cb709a5a024a54452618317 */
void psShowNWDAFAddrInfo()
{
    BYTE i = 0;
    BYTE j = 0;

    for(i = 0; i < 32; i++)
    {

        if(nwdafaddrload[i].v4flag)
        {
            zte_printf_s("\n nwdafaddrload[%d].addr.NwdafIPv4 = %d.%d.%d.%d", i, nwdafaddrload[i].addr.NwdafIPv4[0], nwdafaddrload[i].addr.NwdafIPv4[1],
                                                            nwdafaddrload[i].addr.NwdafIPv4[2], nwdafaddrload[i].addr.NwdafIPv4[3]);
        }
        
        if(nwdafaddrload[i].v6flag)
        {
            zte_printf_s("\n nwdafaddrload[%d].addr.NwdafIPv6 = 0x%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x", i,
            nwdafaddrload[i].addr.NwdafIPv6[0], nwdafaddrload[i].addr.NwdafIPv6[1], nwdafaddrload[i].addr.NwdafIPv6[2], nwdafaddrload[i].addr.NwdafIPv6[3],
            nwdafaddrload[i].addr.NwdafIPv6[4], nwdafaddrload[i].addr.NwdafIPv6[5], nwdafaddrload[i].addr.NwdafIPv6[6], nwdafaddrload[i].addr.NwdafIPv6[7],
            nwdafaddrload[i].addr.NwdafIPv6[8], nwdafaddrload[i].addr.NwdafIPv6[9], nwdafaddrload[i].addr.NwdafIPv6[10], nwdafaddrload[i].addr.NwdafIPv6[11],
            nwdafaddrload[i].addr.NwdafIPv6[12], nwdafaddrload[i].addr.NwdafIPv6[13], nwdafaddrload[i].addr.NwdafIPv6[14], nwdafaddrload[i].addr.NwdafIPv6[15]);
        }
        
        zte_printf_s("\n nwdafaddrload[%d].flag                 = %d", i, nwdafaddrload[i].flag);
        zte_printf_s("\n nwdafaddrload[%d].v4flag               = %d", i, nwdafaddrload[i].v4flag);
        zte_printf_s("\n nwdafaddrload[%d].v6flag               = %d", i, nwdafaddrload[i].v6flag);
        zte_printf_s("\n nwdafaddrload[%d].threadnum            = %d", i, nwdafaddrload[i].threadnum);
        zte_printf_s("\n nwdafaddrload[%d].threadno:             ", i);
        
        for(j = 0; j < nwdafaddrload[i].threadnum; j++)
        {
            zte_printf_s("%d  ", nwdafaddrload[i].threadno[j]);
        }
        zte_printf_s("\n");
    }
    
    return;
}
/* Ended by AICoder, pid:cfc56ga335f6a7e14cb709a5a024a54452618317 */

/* Started by AICoder, pid:48684u68beu2f16145a30a3e104d752d35d88340 */
void psShowNWDAFLinkAddrbyThr(BYTE i)
{
    BYTE bAddrtype = 0;
    T_IPV4V6 addr = {0};    
    bAddrtype = getAddrByThreadno(i, (BYTE*)&addr);

    if(NWDAF_ADDRYTPE_BY_THREAD_IPV4 == bAddrtype)
    {
        zte_printf_s("\n NwdafIPv4 = %d.%d.%d.%d", ThreadNwdafAddr[i].addr.NwdafIPv4[0], ThreadNwdafAddr[i].addr.NwdafIPv4[1],
                                                        ThreadNwdafAddr[i].addr.NwdafIPv4[2], ThreadNwdafAddr[i].addr.NwdafIPv4[3]);
    }
    else if(NWDAF_ADDRYTPE_BY_THREAD_IPV6 == bAddrtype)
    {
        zte_printf_s("\n NwdafIPv6 = 0x%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x",
        ThreadNwdafAddr[i].addr.NwdafIPv6[0], ThreadNwdafAddr[i].addr.NwdafIPv6[1], ThreadNwdafAddr[i].addr.NwdafIPv6[2], ThreadNwdafAddr[i].addr.NwdafIPv6[3],
        ThreadNwdafAddr[i].addr.NwdafIPv6[4], ThreadNwdafAddr[i].addr.NwdafIPv6[5], ThreadNwdafAddr[i].addr.NwdafIPv6[6], ThreadNwdafAddr[i].addr.NwdafIPv6[7],
        ThreadNwdafAddr[i].addr.NwdafIPv6[8], ThreadNwdafAddr[i].addr.NwdafIPv6[9], ThreadNwdafAddr[i].addr.NwdafIPv6[10], ThreadNwdafAddr[i].addr.NwdafIPv6[11],
        ThreadNwdafAddr[i].addr.NwdafIPv6[12], ThreadNwdafAddr[i].addr.NwdafIPv6[13], ThreadNwdafAddr[i].addr.NwdafIPv6[14], ThreadNwdafAddr[i].addr.NwdafIPv6[15]);
    }
    else
    {
        zte_printf_s("\n getAddrByThreadno fail\n");
    }
    
    zte_printf_s("\n bAddrtype(4 -- IPV4;6 -- IPV6) = %d", bAddrtype);
    
    return;
}
/* Ended by AICoder, pid:48684u68beu2f16145a30a3e104d752d35d88340 */


void setNwdafIpinThread(CHAR* ipv4str,CHAR* ipv6str)
{
    BYTE i = 0;
    for(i=0;i<MEDIA_THRD_NUM;i++)
    {
        inet_pton(AF_INET, ipv4str, ThreadNwdafAddr[i].addr.NwdafIPv4);
        inet_pton(AF_INET6, ipv6str, ThreadNwdafAddr[i].addr.NwdafIPv6);
        ThreadNwdafAddr[i].v4flag = 1;
        ThreadNwdafAddr[i].v6flag = 1;
    }
    return ;
    
}


void setNwdafLinkInfoTestLihao() //删除 lihao
{

    T_Ncu_NwdafLinkInfo* ptNwdafAddr = NULL;
    WORD32 i = 0;
    for (i = 0; i < NWDAF_LINKNUM_MAX; i++)
    {
        if (g_nwdaflinkstatus[i].tNwdafAddr.bNwdafIPType != 0)
        {
            ptNwdafAddr = &g_nwdaflinkstatus[i].tNwdafAddr;
            break;
        }
    }

    if (ptNwdafAddr == NULL)
    {
        return;
    }

    
    for (i = 0; i < MEDIA_THRD_NUM; i++)
    {
        IPV4_COPY(ThreadNwdafAddr[i].addr.NwdafIPv4, ptNwdafAddr->NwdafIPv4);
        IPV6_COPY(ThreadNwdafAddr[i].addr.NwdafIPv6, ptNwdafAddr->NwdafIPv6);
        ThreadNwdafAddr[i].v4flag = ptNwdafAddr->bIPv4LinkStatus;
        ThreadNwdafAddr[i].v6flag = ptNwdafAddr->bIPv6LinkStatus;
    }
}


// end of file
/* Ended by AICoder, pid:z4e4fya689gd27e1447f0b3de001c85205b6724e */
