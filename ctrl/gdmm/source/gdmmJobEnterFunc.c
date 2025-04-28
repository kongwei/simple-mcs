#include <stdlib.h>
#include "gdmmJobEnterFunc.h"
#include "tulip_oss.h"
#include "ps_dbs_event.h"
#include "ps_mdmm_ncu_proc.h"
#include "ps_mdmm_ncu_def.h"
#include "ps_gdmm_ncu_entry.h"
#include "ps_db_define_ncu.h"
#include "tulip.h"
#include "xdb_pfu_dyntbl_acc.h"
#include "thdm_pub.h"
#include "zte_slibc.h"
#include "psUpfSCTypes.h"
#include "ncuSCCUAbility.h"
#include "psUpfCommon.h"
#include "psMcsGlobalCfg.h"
#include "UPFLog.h"
#include "ps_mdmm_ncu_dyn_pm_proc.h"
#include "psMcsTestSend.h"
#include "MemShareCfg.h"
#include "ps_db_define_pfu.h"
#include "ps_ncu_session.h"
#include "xdb_core_pfu.h"
#include "psNcuSubscribeCtxProc.h"
#include "ps_ncu_dataApp.h"
#include "ps_ncu_stream.h"
#include "psNcuCtxFunc.h"
#include "psNcuDataEncode.h"

#define PS_GDMM_FUNC_NUM (WORD32)(110)
#define THDM_SUCCESS                        (WORD32)0
#define THDM_FAIL                           (WORD32)1
extern VOID getThreadName(CHAR *auThreadName, const CHAR *auSC_TypeName, WORD32 dwPFU_ID, BYTE bvCPUNo);
extern WORD64 pdaGetcpuload(BYTE coreid);
extern WORD32 ScGetSelfLogNo(void);
extern DBBOOL psNcuGetCfgStrAppidMapByInnerId(WORD32 dwInnerAppId, CHAR *strAppID);
typedef VOID (*pfType_psGwVNcuMcsGdmmProcFuncWithMsgLen)(const VOID *msg, const WORD16 wMsgLen);
typedef VOID (*pfType_psGwVNcuMcsGdmmProcFuncWithMsgBody)(VOID *msg);
typedef VOID (*pfType_psGwVNcuMcsGdmmProcFuncWithVoid)();
extern T_PfuDataBase  g_tPfuDbSysLocal[XDB_PFU_DB_NUM];
VOID psMdmm_ToGdmmShowExample(const VOID *msg, const WORD16 wMsgLen);
VOID psNcuMdmm_ToGdmmShowNcuSessionNumByPfuno(const VOID *msg, const WORD16 wMsgLen);
VOID psNcuMdmm_ToGdmmShowNwdafLinkState(const VOID *msg, const WORD16 wMsgLen);
VOID psNcuMdmm_ToGdmmFillQueryNwdafLinkInfo(const T_psGwDMMShowNwdafLinkStateReq *ptGwDMMShowNwdafLinkStateReq, T_psQueryNwdafLinkInfo *ptQueryNwdafLinkInfo);
VOID psNcuMdmm_ToGdmmShowNwdafLinkState_Single(T_psQueryNwdafLinkInfo *ptQueryNwdafLinkInfo, T_psGdmmShowNwdafLinkStateAck *ptGdmmShowNwdafLinkStateTolInfo);
VOID psNcuMdmm_ToGdmmShowNwdafLinkState_All(T_psGdmmShowNwdafLinkStateAck *ptGdmmShowNwdafLinkStateTolInfo);
void psNcuMdmm_ToGdmmShowNwdafLinkState_Detail(T_psGdmmShowNwdafLinkStateAck *ptGdmmShowNwdafLinkStateTolInfo, T_Ncu_NwdafLinkInfo *ptNwdafAddr, WORD16 wIptype);
VOID psMdmm_ToGdmmShowUPVcpuInfo(const VOID *msg, const WORD16 wMsgLen);
VOID psMdmm_ToGdmm_PmObjectQueryInfo(const VOID *msg, WORD16 wMsgLen);
VOID psNcuMdmm_ToGdmmShowNcuinfo(const VOID *msg, const WORD16 wMsgLen);
VOID psNcuMdmm_ToGdmmShowNcuPodCpuUsage(const VOID *msg, const WORD16 wMsgLen);
void psNcuGdmmFillCpuUsageInfo(T_PSThreadCrtPara *ptThreadParaList, const char *pVruName, const char *pPodName);
void psNcuGdmmFillCpuUsageDetailInfo(const char *pVruName, const char *pPodName, WORD16 wCpuNo, BOOLEAN bShareFlg, WORD16 wCpuUsageNo);
typedef struct
{
    WORD32  dwMsgId;
    pfType_psGwVNcuMcsGdmmProcFuncWithMsgLen    psGdmmFuncWithMsgLen;
    pfType_psGwVNcuMcsGdmmProcFuncWithMsgBody   psGdmmFuncWithMsgBody;
    pfType_psGwVNcuMcsGdmmProcFuncWithVoid      psGdmmFuncWithVoid;
}T_GwVNcuMcsGdmmFunc;

T_GwVNcuMcsGdmmFunc g_psVNcuGdmmFuncRegArray[] = {
    {EV_NCU_MDMM_TO_GDMM_SHOWNCUSESSIONNUMBYPFUNO_REQ,      &psNcuMdmm_ToGdmmShowNcuSessionNumByPfuno,      NULL, NULL},
    {EV_NCU_MDMM_TO_GDMM_SHOWUPVCPUINFO_REQ,                &psMdmm_ToGdmmShowUPVcpuInfo,                   NULL, NULL},
    {EV_NCU_MDMM_TO_GDMM_SHOW_NWDAF_LINK_STATE_REQ,         &psNcuMdmm_ToGdmmShowNwdafLinkState,            NULL, NULL},
    {EV_NCU_MDMM_TO_GDMM_SHOWNCUINFO_REQ,                   &psNcuMdmm_ToGdmmShowNcuinfo,                   NULL, NULL},
    {EV_UPF_MDMM_TO_GDMM_QUERY_DYNPMOBJ_REQ,            &psMdmm_ToGdmm_PmObjectQueryInfo,                       NULL, NULL},
    {EV_UPF_MDMM_TO_GDMM_SHOW_NCU_POD_CPU_USAGE_REQ,            &psNcuMdmm_ToGdmmShowNcuPodCpuUsage,            NULL, NULL},
};
const WORD32 g_psdwGdmmFuncNum = sizeof(g_psVNcuGdmmFuncRegArray)/sizeof(T_GwVNcuMcsGdmmFunc);
BYTE  g_dyn_gdmm_printflag = 0;

void psUpfGdmm_StartupFunc(void)
{
    /*
    EC:614007822194 修改死循环时长为5min
    参数1：  要设置的job号,Ϊ0的话表示设置自身
    参数2：   死锁检测时长,  单位:秒,合法范围是[10,3600]和0,如果为0则表示本次操作不修改死锁检测时长，0xffff表示取消该job的死锁检测
    参数3：   死循环检测时长,单位:秒,合法范围是[10,3600]和0,如果为0则表示本次操作不修改死循环检测时长，0xffff表示取消该job的死循环检测
    */
    XOS_SetJobDLTime(0, 0 ,300);
    return;
}
/**********************************************************************
* 函数名称： psUpfGdmm_MasterFunc
* 功能描述： GDMM函数主入口
* 输入参数：
* 输出参数：
* 返 回 值：
* 其它说明：支持四种命令
(1)常见命令，有消息体&&同时校验消息长度
(2)特殊命令，仅有消息体
(3)定时器等无参数函数
(4)DPI命令
* 修改日期            版本号           修改人              修改内容
* ---------------------------------------------------------------------
* 2022-09-26     V7.22.40            wei  [UPF架构守护][MD3]-psUpfGdmm_MasterFunc重构
***********************************************************************/
VOID psUpfGdmm_MasterFunc(WORD32 msgId, VOID *msg, T_psUpmGdmmPrivData *ptGdmmPrivData)
{
    WORD32                      i = 0;
    WORD16                      wMsgLen = 0;

    _DYN_GDMM_PRINTF("********** psUpfGdmm_MasterFunc enter**********\n");
    for(i = 0; i < PS_GDMM_FUNC_NUM && i < g_psdwGdmmFuncNum; i++)
    {
        if(msgId != g_psVNcuGdmmFuncRegArray[i].dwMsgId)
        {
            continue;
        }
        if(g_psVNcuGdmmFuncRegArray[i].psGdmmFuncWithMsgLen)
        {
            GDMM_NCU_NULL_CHK(msg);
            XOS_GetMsgDataLen(&wMsgLen);
            _DYN_GDMM_PRINTF("********** msgId:%u enter with wMsgLen=%u**********\n", msgId, wMsgLen);
            g_psVNcuGdmmFuncRegArray[i].psGdmmFuncWithMsgLen(msg, wMsgLen);
        }
        else if(g_psVNcuGdmmFuncRegArray[i].psGdmmFuncWithVoid)
        {
            _DYN_GDMM_PRINTF("********** msgId:%u enter **********\n", msgId);
            g_psVNcuGdmmFuncRegArray[i].psGdmmFuncWithVoid();
        }
        else if(g_psVNcuGdmmFuncRegArray[i].psGdmmFuncWithMsgBody)
        {
            GDMM_NCU_NULL_CHK(msg);
            _DYN_GDMM_PRINTF("********** msgId:%u enter msgbody**********\n", msgId);
            g_psVNcuGdmmFuncRegArray[i].psGdmmFuncWithMsgBody(msg);
        }
        else
        {
            _DYN_GDMM_PRINTF("********** msgId:%u func error**********\n", msgId);
        }
        return;
    }
    return;
}

WORD32 psNcuGdmmGetCtxTuple(WORD32  dwhTbl)
{
    WORD32                     dwPsmStatus   = THDM_FAIL;
    T_PSThreadInstAffinityPara tThreadPara;
    DWORD                      dwIndex = 0;
    BYTE                       bSoftThreadNo = 0;
    DM_PFU_GETCAPACITY_REQ     tReq;
    DM_PFU_GETCAPACITY_ACK     tAck;
    WORD32                     dwValidTupleNum = 0;

    dwPsmStatus = psGetMediaThreadInfo(THDM_MEDIA_TYPE_PFU_MEDIA_PROC, &tThreadPara);
    if (dwPsmStatus != THDM_SUCCESS)
    {
        _DYN_GDMM_PRINTF("psGetMediaThreadInfo fail !!!!!!!!\n");
        return 0;
    }

    for(dwIndex = 0; dwIndex < MDMM_NCU_MIN(tThreadPara.bInstNum , MAX_PS_THREAD_INST_NUM); dwIndex++)
    {
        bSoftThreadNo = tThreadPara.atThreadParaList[dwIndex].bSoftThreadNo;
        zte_memset_s(&tReq, sizeof(DM_PFU_GETCAPACITY_REQ), 0, sizeof(DM_PFU_GETCAPACITY_REQ));
        tReq.hDB          = _NCU_GET_DBHANDLE(bSoftThreadNo);
        tReq.hTbl         = dwhTbl;

        _pDM_PFU_GETCAPACITY(&tReq, &tAck);
        if (tAck.retCODE != RC_OK)
        {
            _DYN_GDMM_PRINTF("psNcuGdmmGetCtxTuple _pDM_PFU_GETCAPACITY is not RC_OK!!!!!!!!\n");
            return 0;
        }
        dwValidTupleNum += tAck.dwValidTupleNum;
    }
    return dwValidTupleNum;
}

extern WORD32 psGetShareCpuNum(void);
extern WORD32 psGetShareCpuByIndex(uint32_t index);
inline void psMdmm_ToGdmmVcpuInfoCollect(BYTE VcpuType, BYTE SCType, T_psGwDMMNcuShowUpvcpuinfoAck* ptGwDMMShowUpvcpuinfoAck)
{
    GDMM_NCU_NULL_CHK(ptGwDMMShowUpvcpuinfoAck);
    BYTE   bSthIndex = 0;
    WORD32 dwPsmStatus= THDM_SUCCESS;
    T_PSThreadInstAffinityPara  tThreadPara = {0};
    WORD32 dwSelfLogicNo  =  ncuGetScLogicNo();

    _DYN_GDMM_PRINTF("psMdmm_ToGdmmVcpuInfoCollect enter !!!!!!!!\n");

    dwPsmStatus = psGetMediaThreadInfo(VcpuType, &tThreadPara);
    if(THDM_SUCCESS !=dwPsmStatus)
    {
        _DYN_GDMM_PRINTF("Error!! psGetMediaThreadInfo Failed!!!!!!!!\n");
        return;
    }

    BYTE bMaxInstNum = (tThreadPara.bInstNum > SHOWUPVCPUINFOACK_VCPUINFO_NUM) ? SHOWUPVCPUINFOACK_VCPUINFO_NUM : tThreadPara.bInstNum;
    _DYN_GDMM_PRINTF("Before collect !! upvcpuinfoNum  %d  !!!!!\n", ptGwDMMShowUpvcpuinfoAck->upvcpuinfoNum);
    for (bSthIndex = 0; (bSthIndex < bMaxInstNum) && (bSthIndex < MAX_PS_THREAD_INST_NUM); bSthIndex++)
    {
        /*bNeedSleep=TRUE，scan线程,处理时，把所有共享核一起上报*/
        if(TRUE == tThreadPara.atThreadParaList[bSthIndex].bNeedSleep)
        {
            WORD32 dwShareCpuNum = psGetShareCpuNum();
            WORD32 dwShareIndex = 0;

            BYTE   bScanShareFlag = 0; /*判断scan线程是否是共享核的flag,1为共享核，0为独占核*/
            WORD32 dwScanVcpuNo = tThreadPara.atThreadParaList[bSthIndex].bvCPUNo;
            _DYN_GDMM_PRINTF("get dwScanVcpuNo !! dwScanVcpuNo  %d  !!!!!\n",dwScanVcpuNo);
            _DYN_GDMM_PRINTF("get dwShareCpuNum !! dwShareCpuNum  %d  !!!!!\n",dwShareCpuNum);

            for(dwShareIndex = 0; dwShareIndex < dwShareCpuNum && dwShareIndex < MAX_PS_THREAD_INST_NUM; dwShareIndex++)
            {
                WORD32 dwShareVcpuNo = psGetShareCpuByIndex(dwShareIndex);
                _DYN_GDMM_PRINTF("get ShareVcpuNo !! dwShareVcpuNo  %d  !!!!!\n",dwShareVcpuNo);
                if(dwShareVcpuNo == dwScanVcpuNo)
                {
                    bScanShareFlag = 1;
                }
                T_psGwDMMNcuShowUpvcpuinfo *ptShowVcpuinfo =&(ptGwDMMShowUpvcpuinfoAck->upvcpuinfo[ptGwDMMShowUpvcpuinfoAck->upvcpuinfoNum]);
                ptShowVcpuinfo->vcpurate = XOS_GetSingleCpuRate(dwShareVcpuNo);
                ptShowVcpuinfo->sc_typename = SCType;
                ptShowVcpuinfo->sc_logicno = dwSelfLogicNo;
                ptShowVcpuinfo->vcputype=tThreadPara.atThreadParaList[bSthIndex].bMediaType;
                ptShowVcpuinfo->vcpuno = dwShareVcpuNo; /*虚机内编号*/
                ptShowVcpuinfo->vcpuindex = tThreadPara.atThreadParaList[bSthIndex].bvCPUIndex; /*容器内编号，dpdk使用*/
                _DYN_GDMM_PRINTF("[psMdmm_ToGdmmVcpuInfoCollect] sc_typename: %d \n"
                                 "vcpurate  :   %d \n"
                                 "sc_logicno:   %d \n"
                                 "vcputype  :   %d \n"
                                 "vcpuno    :   %d \n"
                                 "vcpuindex :   %d \n",
                                 ptShowVcpuinfo->sc_typename,
                                 ptShowVcpuinfo->vcpurate,
                                 ptShowVcpuinfo->sc_logicno,
                                 ptShowVcpuinfo->vcputype,
                                 ptShowVcpuinfo->vcpuno,
                                 ptShowVcpuinfo->vcpuindex);

                getThreadName(ptShowVcpuinfo->objname, SC_NAME_NEAR_COMPUTING, dwSelfLogicNo, dwShareVcpuNo);
                ptGwDMMShowUpvcpuinfoAck->upvcpuinfoNum++;
            }
            if(0 == bScanShareFlag && 0 != dwScanVcpuNo)/*scan非共享核单独上报，一般为测试场景*/
            {
                T_psGwDMMNcuShowUpvcpuinfo  *ptShowVcpuinfo =&(ptGwDMMShowUpvcpuinfoAck->upvcpuinfo[ptGwDMMShowUpvcpuinfoAck->upvcpuinfoNum]);
                ptShowVcpuinfo->sc_typename = SCType;
                ptShowVcpuinfo->sc_logicno = dwSelfLogicNo;
                ptShowVcpuinfo->vcputype=tThreadPara.atThreadParaList[bSthIndex].bMediaType;
                ptShowVcpuinfo->vcpuno = dwScanVcpuNo;/*虚机内编号*/
                ptShowVcpuinfo->vcpuindex = tThreadPara.atThreadParaList[bSthIndex].bvCPUIndex; /*容器内编号，dpdk使用*/
                ptShowVcpuinfo->vcpurate =(BYTE)pdaGetcpuload(ptShowVcpuinfo->vcpuindex) ;/*scan核为共享核，使用这个接口*/
                getThreadName(ptShowVcpuinfo->objname, SC_NAME_NEAR_COMPUTING, dwSelfLogicNo, dwScanVcpuNo);
                ptGwDMMShowUpvcpuinfoAck->upvcpuinfoNum++;
            }
            continue;
        }

        T_psGwDMMNcuShowUpvcpuinfo  *ptShowVcpuinfo =&(ptGwDMMShowUpvcpuinfoAck->upvcpuinfo[ptGwDMMShowUpvcpuinfoAck->upvcpuinfoNum]);

        ptShowVcpuinfo->sc_typename = SCType;
        ptShowVcpuinfo->sc_logicno = dwSelfLogicNo;
        ptShowVcpuinfo->vcputype=tThreadPara.atThreadParaList[bSthIndex].bMediaType;
        ptShowVcpuinfo->vcpuno = tThreadPara.atThreadParaList[bSthIndex].bvCPUNo; /*虚机内编号*/
        ptShowVcpuinfo->vcpuindex = tThreadPara.atThreadParaList[bSthIndex].bvCPUIndex; /*容器内编号，dpdk使用*/
        if(TRUE == tThreadPara.atThreadParaList[bSthIndex].bNeedSleep ) /*共享核*/
        {
            ptShowVcpuinfo->vcpurate = XOS_GetSingleCpuRate(ptShowVcpuinfo->vcpuno);/*ƽ̨2s钟接口，传入虚机内编号*/
        }
        else
        {
            ptShowVcpuinfo->vcpurate = (BYTE)pdaGetcpuload(ptShowVcpuinfo->vcpuindex);
        }
        getThreadName(ptShowVcpuinfo->objname, SC_NAME_NEAR_COMPUTING, dwSelfLogicNo, ptShowVcpuinfo->vcpuno);
        ptGwDMMShowUpvcpuinfoAck->upvcpuinfoNum++;
    }
    _DYN_GDMM_PRINTF("After collect !! upvcpuinfoNum  %d  !!!!!\n",ptGwDMMShowUpvcpuinfoAck->upvcpuinfoNum);
    return;
}

inline VOID psGdmmShowUPVcpuInfoPreProc(const T_psGwDMMNcuShowUpvcpuinfoReq* ptGwDMMShowUpvcpuinfoReq)
{
    GDMM_NCU_NULL_CHK(ptGwDMMShowUpvcpuinfoReq);
    WORD32 dwSelfLogicNo = ncuGetScLogicNo();

    if((0 != ptGwDMMShowUpvcpuinfoReq->sc_typenameFlag)
        &&((NCUSCTYPE_NEARCOMPUTING != ptGwDMMShowUpvcpuinfoReq->sc_typename) && (NCUSCTYPE_ALL != ptGwDMMShowUpvcpuinfoReq->sc_typename)))
    {
         _DYN_GDMM_PRINTF("msg send err,sc_typename wrong %d!!!!!!!!\n",ptGwDMMShowUpvcpuinfoReq->sc_typename);
          return;
    }

    if((0 != ptGwDMMShowUpvcpuinfoReq->sc_logicnoFlag)
        &&(dwSelfLogicNo != ptGwDMMShowUpvcpuinfoReq->sc_logicno))
    {
        _DYN_GDMM_PRINTF("msg send err,sc_typename wrong %d!!!!!!!!\n",ptGwDMMShowUpvcpuinfoReq->sc_typename);
         return;
    }

    return;
}

VOID psMdmm_ToGdmmShowUPVcpuInfo(const VOID *msg, const WORD16 wMsgLen)
{
    const T_psGwDMMNcuShowUpvcpuinfoReq*    ptGwDMMShowUpvcpuinfoReq = NULL;
    T_psGwDMMNcuShowUpvcpuinfoAck     tGwDMMShowUpvcpuinfoAck;

    _DYN_GDMM_PRINTF("psMdmm_ToGdmmShowUPVcpuInfo enter !!!!!!!!\n");
    GDMM_NCU_NULL_CHK(msg);
    GDMM_NCU_VALID_CHK(sizeof(T_psGwDMMNcuShowUpvcpuinfoReq) != wMsgLen);
    _DYN_GDMM_PRINTF("check msg success !!!!!!!!\n");

    JID                               recvJid;
    JID                               sendJid;
    XOS_STATUS                        xosRet = XOS_SUCCESS;

    zte_memset_s(&tGwDMMShowUpvcpuinfoAck, sizeof(T_psGwDMMNcuShowUpvcpuinfoAck), 0, sizeof(T_psGwDMMNcuShowUpvcpuinfoAck));
    ptGwDMMShowUpvcpuinfoReq = (T_psGwDMMNcuShowUpvcpuinfoReq *)msg;

    _DYN_GDMM_PRINTF("[psMdmm_ToGdmmShowUPVcpuInfo] sc_typenameFlag: %d \n"
                     "sc_typename :%d \n"
                     "sc_logicnoFlag:%d \n"
                     "sc_logicno:%d \n"
                     "vcputypeFlag:%d \n"
                     "vcputype:%d \n",
        ptGwDMMShowUpvcpuinfoReq->sc_typenameFlag,
        ptGwDMMShowUpvcpuinfoReq->sc_typename,
        ptGwDMMShowUpvcpuinfoReq->sc_logicnoFlag,
        ptGwDMMShowUpvcpuinfoReq->sc_logicno,
        ptGwDMMShowUpvcpuinfoReq->vcputypeFlag,
        ptGwDMMShowUpvcpuinfoReq->vcputype);

    psGdmmShowUPVcpuInfoPreProc(ptGwDMMShowUpvcpuinfoReq);
    tGwDMMShowUpvcpuinfoAck.tHead = ptGwDMMShowUpvcpuinfoReq->tHead;
    BYTE vcputype = (BYTE)ptGwDMMShowUpvcpuinfoReq->vcputype;
    BYTE sc_typename = NCUSCTYPE_NEARCOMPUTING;
    _DYN_GDMM_PRINTF("vcputype  %d  sc_typename %d !!!!!!!!\n", vcputype, sc_typename);
    psMdmm_ToGdmmVcpuInfoCollect(vcputype, sc_typename, &tGwDMMShowUpvcpuinfoAck);

    _DYN_GDMM_PRINTF("All Collect  upvcpuinfoNum  %d!!!!!!!!\n", tGwDMMShowUpvcpuinfoAck.upvcpuinfoNum);

    xosRet = XOS_GetSelfJID(&sendJid);
    GDMM_NCU_VALID_CHK(XOS_SUCCESS != xosRet);

    xosRet = XOS_Sender((VOID *)&recvJid);
    GDMM_NCU_VALID_CHK(XOS_SUCCESS != xosRet);
    xosRet = XOS_SendAsynMsg(EV_NCU_GDMM_TO_MDMM_SHOWUPVCPUINFO_ACK, (BYTE *)&tGwDMMShowUpvcpuinfoAck, sizeof(tGwDMMShowUpvcpuinfoAck), XOS_MSG_VER0, &recvJid, &sendJid);
    if(XOS_SUCCESS !=xosRet)
    {
        _DYN_GDMM_PRINTF("Send EV_NCU_GDMM_TO_MDMM_SHOWUPVCPUINFO_ACK  FAIL!! xosRet = %d !!\n",xosRet);
    }
    else
    {
        _DYN_GDMM_PRINTF("Send EV_NCU_GDMM_TO_MDMM_SHOWUPVCPUINFO_ACK  OK !!!!!!!!\n");
    }
    return ;
}

VOID psMdmm_ToGdmmShowExample(const VOID *msg, const WORD16 wMsgLen)
{
    zte_printf_s("psMdmm_ToGdmmShowExample msg=%p,wMsgLen=%u", msg, wMsgLen);
    return ;
}

VOID psNcuMdmm_ToGdmmShowNcuSessionNumByPfuno(const VOID *msg, const WORD16 wMsgLen)
{
    const T_psGwDMMShowNcuSessionNumByPfunoReq*    ptGwDMMShowPfuSessionNumByPfunoReq = NULL;
    T_psGwDMMShowNcuSessionNumByPfunoAck     tGwDMMShowPfuSessionNumByPfunoAck;
    zte_memset_s(&tGwDMMShowPfuSessionNumByPfunoAck, sizeof(tGwDMMShowPfuSessionNumByPfunoAck), 0, sizeof(tGwDMMShowPfuSessionNumByPfunoAck));

    JID                               recvJid;
    JID                               sendJid;
    XOS_STATUS                        xosRet;

    _DYN_GDMM_PRINTF("psNcuMdmm_ToGdmmShowNcuSessionNumByPfuno enter !!!!!!!!\n");
    GDMM_NCU_NULL_CHK(msg);
    GDMM_NCU_VALID_CHK(sizeof(T_psGwDMMShowNcuSessionNumByPfunoReq) != wMsgLen);
    _DYN_GDMM_PRINTF("check msg success !!!!!!!!\n");

    ptGwDMMShowPfuSessionNumByPfunoReq = (T_psGwDMMShowNcuSessionNumByPfunoReq *)msg;
    zte_memcpy_s(&tGwDMMShowPfuSessionNumByPfunoAck.tGwDMMShowNcuSessionNumByPfunoReq, sizeof(T_psGwDMMShowNcuSessionNumByPfunoReq), ptGwDMMShowPfuSessionNumByPfunoReq, sizeof(T_psGwDMMShowNcuSessionNumByPfunoReq));

    GDMM_NCU_VALID_CHK(tGwDMMShowPfuSessionNumByPfunoAck.tGwDMMShowNcuSessionNumByPfunoReq.addrvirtualvalue >= MAX_SHOWNCUSESSIONNUMBYPFUNOACK_PFULOGICNO_NUM);

    tGwDMMShowPfuSessionNumByPfunoAck.tGwDMMShowNcuSessionNumByPfunoReq.pfulogicno[tGwDMMShowPfuSessionNumByPfunoAck.tGwDMMShowNcuSessionNumByPfunoReq.addrvirtualvalue] = tGwDMMShowPfuSessionNumByPfunoAck.tGwDMMShowNcuSessionNumByPfunoReq.tHead.dwCurrentLogicNo;
    _DYN_GDMM_PRINTF("PFU LOGIC NO = %u\n",tGwDMMShowPfuSessionNumByPfunoAck.tGwDMMShowNcuSessionNumByPfunoReq.pfulogicno[tGwDMMShowPfuSessionNumByPfunoAck.tGwDMMShowNcuSessionNumByPfunoReq.addrvirtualvalue]);

    tGwDMMShowPfuSessionNumByPfunoAck.tGwDMMShowNcuSessionNumByPfunoReq.ncusessionnum[tGwDMMShowPfuSessionNumByPfunoAck.tGwDMMShowNcuSessionNumByPfunoReq.addrvirtualvalue] = psNcuGdmmGetCtxTuple(DB_CTX_R_NCU_SESSION);
    _DYN_GDMM_PRINTF("R_PFU_SESSION num = %u\n",tGwDMMShowPfuSessionNumByPfunoAck.tGwDMMShowNcuSessionNumByPfunoReq.ncusessionnum[tGwDMMShowPfuSessionNumByPfunoAck.tGwDMMShowNcuSessionNumByPfunoReq.addrvirtualvalue]);

    tGwDMMShowPfuSessionNumByPfunoAck.tGwDMMShowNcuSessionNumByPfunoReq.addrvirtualvalue++;

    xosRet = XOS_GetSelfJID(&sendJid);
    GDMM_NCU_VALID_CHK(XOS_SUCCESS != xosRet);

    xosRet = XOS_Sender((VOID *)&recvJid);
    GDMM_NCU_VALID_CHK(XOS_SUCCESS != xosRet);

    GDMM_NCU_VALID_CHK(XOS_SUCCESS != XOS_SendAsynMsg(EV_NCU_GDMM_TO_MDMM_SHOWNCUSESSIONNUMBYPFUNO_ACK,(BYTE *)&tGwDMMShowPfuSessionNumByPfunoAck, sizeof(tGwDMMShowPfuSessionNumByPfunoAck), XOS_MSG_VER0, &recvJid, &sendJid));
    return ;
}

#define NWDAF_TYPE_IPV4   1
#define NWDAF_TYPE_IPV6   2

extern T_NwdafStatus g_nwdaflinkstatus[NWDAF_LINKNUM_MAX];
VOID psNcuMdmm_ToGdmmShowNwdafLinkState(const VOID *msg, const WORD16 wMsgLen)
{
    _DYN_GDMM_PRINTF("psNcuMdmm_ToGdmmShowNwdafLinkState enter !!!!!!!!\n");
    GDMM_NCU_NULL_CHK(msg);
    GDMM_NCU_VALID_CHK(sizeof(T_psGwDMMShowNwdafLinkStateReq) != wMsgLen);
    _DYN_GDMM_PRINTF("check msg success !!!!!!!!\n");

    const T_psGwDMMShowNwdafLinkStateReq *ptGwDMMShowNwdafLinkStateReq = NULL;
    T_psGdmmShowNwdafLinkStateAck tGdmmShowNwdafLinkStateTolInfo = {0};
    T_psQueryNwdafLinkInfo tQueryNwdafLinkInfo = {0};

    JID                               recvJid;
    JID                               sendJid;
    XOS_STATUS                        xosRet;

    ptGwDMMShowNwdafLinkStateReq = (T_psGwDMMShowNwdafLinkStateReq *)msg;
    _DYN_GDMM_PRINTF("ptGwDMMShowNwdafLinkStateReq->querymode   = %u\n", ptGwDMMShowNwdafLinkStateReq->querymode);
    _DYN_GDMM_PRINTF("ptGwDMMShowNwdafLinkStateReq->ipv4address = %s\n", ptGwDMMShowNwdafLinkStateReq->ipv4address);
    _DYN_GDMM_PRINTF("ptGwDMMShowNwdafLinkStateReq->ipv6address = %s\n", ptGwDMMShowNwdafLinkStateReq->ipv6address);

    if(QUERY_MODE_SINGLE == ptGwDMMShowNwdafLinkStateReq->querymode)
    {
        psNcuMdmm_ToGdmmFillQueryNwdafLinkInfo(ptGwDMMShowNwdafLinkStateReq, &tQueryNwdafLinkInfo);
        psNcuMdmm_ToGdmmShowNwdafLinkState_Single(&tQueryNwdafLinkInfo, &tGdmmShowNwdafLinkStateTolInfo);

    }
    else
    {
        psNcuMdmm_ToGdmmShowNwdafLinkState_All(&tGdmmShowNwdafLinkStateTolInfo);
    }

    xosRet = XOS_GetSelfJID(&sendJid);
    GDMM_NCU_VALID_CHK(XOS_SUCCESS != xosRet);

    xosRet = XOS_Sender((VOID *)&recvJid);
    GDMM_NCU_VALID_CHK(XOS_SUCCESS != xosRet);

    GDMM_NCU_VALID_CHK(XOS_SUCCESS != XOS_SendAsynMsg(EV_NCU_GDMM_TO_MDMM_SHOW_NWDAF_LINK_STATE_ACK,(BYTE *)&tGdmmShowNwdafLinkStateTolInfo, sizeof(tGdmmShowNwdafLinkStateTolInfo), XOS_MSG_VER0, &recvJid, &sendJid));
    return ;
}

VOID psNcuMdmm_ToGdmmFillQueryNwdafLinkInfo(const T_psGwDMMShowNwdafLinkStateReq *ptGwDMMShowNwdafLinkStateReq, T_psQueryNwdafLinkInfo *ptQueryNwdafLinkInfo)
{
    GDMM_NCU_NULL_CHK(ptGwDMMShowNwdafLinkStateReq);
    GDMM_NCU_NULL_CHK(ptQueryNwdafLinkInfo);

    WORD32 dwIPV4Addr = 0;
    if(NWDAF_TYPE_IPV4 == ptGwDMMShowNwdafLinkStateReq->iptype)
    {
        ptQueryNwdafLinkInfo->iptype = ptGwDMMShowNwdafLinkStateReq->iptype;
        inet_pton(AF_INET, ptGwDMMShowNwdafLinkStateReq->ipv4address, &dwIPV4Addr);
        IPV4_COPY(ptQueryNwdafLinkInfo->bIPV4, &dwIPV4Addr);
        _DYN_GDMM_PRINTF("ptQueryNwdafLinkInfo->bIPV4: %u.%u.%u.%u \n", ptQueryNwdafLinkInfo->bIPV4[0], ptQueryNwdafLinkInfo->bIPV4[1], ptQueryNwdafLinkInfo->bIPV4[2], ptQueryNwdafLinkInfo->bIPV4[3]);
    }
    else if (NWDAF_TYPE_IPV6 == ptGwDMMShowNwdafLinkStateReq->iptype)
    {
        ptQueryNwdafLinkInfo->iptype = ptGwDMMShowNwdafLinkStateReq->iptype;
        T_IPV6 abIP;
        inet_pton(AF_INET6, ptGwDMMShowNwdafLinkStateReq->ipv6address, abIP);
        IPV6_COPY(ptQueryNwdafLinkInfo->bIPV6, abIP);
        _DYN_GDMM_PRINTF("ptQueryNwdafLinkInfo->bIPV6: %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x \n", 
            ptQueryNwdafLinkInfo->bIPV6[0], ptQueryNwdafLinkInfo->bIPV6[1], ptQueryNwdafLinkInfo->bIPV6[2], ptQueryNwdafLinkInfo->bIPV6[3],
            ptQueryNwdafLinkInfo->bIPV6[4], ptQueryNwdafLinkInfo->bIPV6[5], ptQueryNwdafLinkInfo->bIPV6[6], ptQueryNwdafLinkInfo->bIPV6[7],
            ptQueryNwdafLinkInfo->bIPV6[8], ptQueryNwdafLinkInfo->bIPV6[9], ptQueryNwdafLinkInfo->bIPV6[10], ptQueryNwdafLinkInfo->bIPV6[11],
            ptQueryNwdafLinkInfo->bIPV6[12], ptQueryNwdafLinkInfo->bIPV6[13], ptQueryNwdafLinkInfo->bIPV6[14], ptQueryNwdafLinkInfo->bIPV6[15]);
    }
    return;
}

VOID psNcuMdmm_ToGdmmShowNwdafLinkState_Single(T_psQueryNwdafLinkInfo *ptQueryNwdafLinkInfo, T_psGdmmShowNwdafLinkStateAck *ptGdmmShowNwdafLinkStateTolInfo)
{
    GDMM_NCU_NULL_CHK(ptQueryNwdafLinkInfo);
    GDMM_NCU_NULL_CHK(ptGdmmShowNwdafLinkStateTolInfo);

    WORD32 i;
    for(i=0; i<MAX_SHOWNWDAFLINKSTATEACK_LINK_STATE_NUM; i++)
    {
        T_Ncu_NwdafLinkInfo *ptNwdafAddr = &g_nwdaflinkstatus[i].tNwdafAddr;
        if(NWDAF_TYPE_IPV4 == ptQueryNwdafLinkInfo->iptype)
        {
            
            GDMM_NCU_VALID_CONTINUE(NWDAF_TYPE_IPV6 == ptNwdafAddr->bNwdafIPType);
            GDMM_NCU_VALID_CONTINUE(0 != memcmp(ptNwdafAddr->NwdafIPv4, ptQueryNwdafLinkInfo->bIPV4, 4));
            /*fill info */
            psNcuMdmm_ToGdmmShowNwdafLinkState_Detail(ptGdmmShowNwdafLinkStateTolInfo, ptNwdafAddr, NWDAF_TYPE_IPV4);
            break;
        }
        else
        {
            GDMM_NCU_VALID_CONTINUE(NWDAF_TYPE_IPV4 == ptNwdafAddr->bNwdafIPType);
            GDMM_NCU_VALID_CONTINUE(0 != memcmp(ptNwdafAddr->NwdafIPv6, ptQueryNwdafLinkInfo->bIPV6, 16));
            psNcuMdmm_ToGdmmShowNwdafLinkState_Detail(ptGdmmShowNwdafLinkStateTolInfo, ptNwdafAddr, NWDAF_TYPE_IPV6);
            break;
        }
    }
    ptGdmmShowNwdafLinkStateTolInfo->wResult = (ptGdmmShowNwdafLinkStateTolInfo->linkNum>0)?QUE_RST_SUCC:QUE_RST_FAIL;
    return;
}

VOID psNcuMdmm_ToGdmmShowNwdafLinkState_All(T_psGdmmShowNwdafLinkStateAck *ptGdmmShowNwdafLinkStateTolInfo)
{
    GDMM_NCU_NULL_CHK(ptGdmmShowNwdafLinkStateTolInfo);

    WORD32 i;
    for(i=0; i<MAX_SHOWNWDAFLINKSTATEACK_LINK_STATE_NUM; i++)
    {
        T_Ncu_NwdafLinkInfo *ptNwdafAddr = &g_nwdaflinkstatus[i].tNwdafAddr;
        if(NWDAF_TYPE_IPV4 & ptNwdafAddr->bNwdafIPType)
        {
            psNcuMdmm_ToGdmmShowNwdafLinkState_Detail(ptGdmmShowNwdafLinkStateTolInfo, ptNwdafAddr, NWDAF_TYPE_IPV4);
        }
        if(NWDAF_TYPE_IPV6 & ptNwdafAddr->bNwdafIPType)
        {
            psNcuMdmm_ToGdmmShowNwdafLinkState_Detail(ptGdmmShowNwdafLinkStateTolInfo, ptNwdafAddr, NWDAF_TYPE_IPV6);
        }
    }
    ptGdmmShowNwdafLinkStateTolInfo->wResult = (ptGdmmShowNwdafLinkStateTolInfo->linkNum>0)?QUE_RST_SUCC:QUE_RST_FAIL;
    return;
}

void psNcuMdmm_ToGdmmShowNwdafLinkState_Detail(T_psGdmmShowNwdafLinkStateAck *ptGdmmShowNwdafLinkStateTolInfo, T_Ncu_NwdafLinkInfo *ptNwdafAddr, WORD16 wIptype)
{
    GDMM_NCU_NULL_CHK(ptNwdafAddr);
    WORD32 dwIPV4Addr = 0;
    BYTE bIPV4[4] = {0};
    BYTE bIPV6[16] = {0};

    GDMM_NCU_VALID_CHK(ptGdmmShowNwdafLinkStateTolInfo->linkNum >= MAX_SHOWNWDAFLINKSTATEACK_LINK_STATE_NUM);
    psShowNwdafLinkStateAckInfo *ptGdmmShowNwdafLinkStateAckInfo = &ptGdmmShowNwdafLinkStateTolInfo->info[ptGdmmShowNwdafLinkStateTolInfo->linkNum];
    if(NWDAF_TYPE_IPV4 == wIptype)
    {
        ptGdmmShowNwdafLinkStateAckInfo->iptype = NWDAF_TYPE_IPV4;
        upf_snprintf(ptGdmmShowNwdafLinkStateAckInfo->nwdafaddr, MAX_SHOWNWDAFLINKSTATE_ACK_ADDRESS_LEN, "%u.%u.%u.%u",
                    ptNwdafAddr->NwdafIPv4[0], ptNwdafAddr->NwdafIPv4[1], ptNwdafAddr->NwdafIPv4[2], ptNwdafAddr->NwdafIPv4[3]);

        inet_pton(AF_INET, g_upfConfig.daUpfIpCfg.upfIpv4, &dwIPV4Addr);
        IPV4_COPY(bIPV4, &dwIPV4Addr);
        upf_snprintf(ptGdmmShowNwdafLinkStateAckInfo->localaddr, MAX_SHOWNWDAFLINKSTATE_ACK_ADDRESS_LEN, "%u.%u.%u.%u", bIPV4[0], bIPV4[1], bIPV4[2], bIPV4[3]);

        ptGdmmShowNwdafLinkStateAckInfo->linkstate = ptNwdafAddr->bIPv4LinkStatus;
        ptGdmmShowNwdafLinkStateTolInfo->linkNum++;
    }
    else 
    {
        ptGdmmShowNwdafLinkStateAckInfo->iptype = NWDAF_TYPE_IPV6;
        upf_snprintf(ptGdmmShowNwdafLinkStateAckInfo->nwdafaddr, MAX_SHOWNWDAFLINKSTATE_ACK_ADDRESS_LEN, "%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x",
                        ptNwdafAddr->NwdafIPv6[0],ptNwdafAddr->NwdafIPv6[1],ptNwdafAddr->NwdafIPv6[2],ptNwdafAddr->NwdafIPv6[3],
                        ptNwdafAddr->NwdafIPv6[4],ptNwdafAddr->NwdafIPv6[5],ptNwdafAddr->NwdafIPv6[6],ptNwdafAddr->NwdafIPv6[7],
                        ptNwdafAddr->NwdafIPv6[8],ptNwdafAddr->NwdafIPv6[9],ptNwdafAddr->NwdafIPv6[10],ptNwdafAddr->NwdafIPv6[11],
                        ptNwdafAddr->NwdafIPv6[12],ptNwdafAddr->NwdafIPv6[13],ptNwdafAddr->NwdafIPv6[14],ptNwdafAddr->NwdafIPv6[15]);

        /*fill info */
        zte_memset_s(bIPV6, sizeof(bIPV6), 0, sizeof(bIPV6));
        inet_pton(AF_INET6, g_upfConfig.daUpfIpCfg.upfIpv6, bIPV6);
        upf_snprintf(ptGdmmShowNwdafLinkStateAckInfo->localaddr, MAX_SHOWNWDAFLINKSTATE_ACK_ADDRESS_LEN, "%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x",
                        bIPV6[0],bIPV6[1],bIPV6[2],bIPV6[3],bIPV6[4],bIPV6[5],bIPV6[6],bIPV6[7],bIPV6[8],bIPV6[9],bIPV6[10],bIPV6[11],bIPV6[12],bIPV6[13],bIPV6[14],bIPV6[15]);

        ptGdmmShowNwdafLinkStateAckInfo->linkstate = ptNwdafAddr->bIPv6LinkStatus;
        ptGdmmShowNwdafLinkStateTolInfo->linkNum++;
    }
    _DYN_GDMM_PRINTF("\n ptGdmmShowNwdafLinkStateAckInfo: iptype = %u , linkstate = %u \n", ptGdmmShowNwdafLinkStateAckInfo->iptype, ptGdmmShowNwdafLinkStateAckInfo->linkstate);
    _DYN_GDMM_PRINTF("\n ptGdmmShowNwdafLinkStateAckInfo: nwdafaddr %s \n", ptGdmmShowNwdafLinkStateAckInfo->nwdafaddr);
    _DYN_GDMM_PRINTF("\n ptGdmmShowNwdafLinkStateAckInfo: localaddr %s \n", ptGdmmShowNwdafLinkStateAckInfo->localaddr);
    return;
}

VOID psMdmm_ToGdmm_PmObjectVcpuInfoProc(const VOID *msg)
{
    T_Gdmm2Mdmm_PMObjInfo     tPO60908Ack = {0};
    T_Mdmm2Gdmm_PMObjectQueryInfo*  ptReq = NULL;
    JID         recvJid;
    JID         sendJid;
    XOS_STATUS  xosRet;
    WORD32 dwPsmStatus= THDM_SUCCESS;
    BYTE    bSthIndex = 0;
    BYTE    bSthCnt = 0;
    T_PSThreadInstAffinityPara  tThreadPara = {0};
    CHAR    cTmp[128];
    GDMM_NCU_NULL_CHK(msg);
    WORD32 dwSelfLogicNo  =  ScGetSelfLogNo();
    dwPsmStatus = psGetMediaThreadInfo(THDM_MEDIA_TYPE_ALL, &tThreadPara);
    GDMM_NCU_VALID_CHK(THDM_SUCCESS != dwPsmStatus);

    ptReq = (T_Mdmm2Gdmm_PMObjectQueryInfo*)msg;
    tPO60908Ack.dwRelativeMotidCnt = 1;
    tPO60908Ack.dwMotids[0] = PM_SYNC_MOTID_60908;
    BYTE bMaxInstNum = MIN(tThreadPara.bInstNum, SHOWUPVCPUINFOACK_VCPUINFO_NUM);
    for (bSthIndex = 0; bSthIndex < bMaxInstNum; bSthIndex++)
    {
        if(TRUE == tThreadPara.atThreadParaList[bSthIndex].bNeedSleep)
        {
            WORD32 dwShareCpuNum = psGetShareCpuNum();
            WORD32 dwShareIndex = 0;
            BYTE   bScanShareFlag = 0;
            WORD32 dwScanVcpuNo = tThreadPara.atThreadParaList[bSthIndex].bvCPUNo;
            for(dwShareIndex = 0; dwShareIndex < dwShareCpuNum; dwShareIndex++)
            {
                GDMM_NCU_VALID_BREAK(dwShareIndex == MAX_PS_THREAD_INST_NUM);
                WORD32 dwShareVcpuNo = psGetShareCpuByIndex(dwShareIndex);
                if(dwShareVcpuNo == dwScanVcpuNo)
                {
                    bScanShareFlag = 1;
                }
                memset(cTmp, 0, sizeof(cTmp));
                getThreadName(cTmp, SC_NAME_NEAR_COMPUTING, dwSelfLogicNo, dwShareVcpuNo);
                GDMM_NCU_VALID_BREAK(UPF_MDMM_PM_SYNC_MAX_OBJ_EACH_MOTID <= bSthCnt);
                strncpy(tPO60908Ack.tTotalPMObject[bSthCnt++].cPMObjectName, cTmp, sizeof(tPO60908Ack.tTotalPMObject[0].cPMObjectName)-1);
            }
            if(0 == bScanShareFlag && 0 != dwScanVcpuNo)
            {
                memset(cTmp, 0, sizeof(cTmp));
                getThreadName(cTmp, SC_NAME_NEAR_COMPUTING, dwSelfLogicNo, dwScanVcpuNo);
                GDMM_NCU_VALID_BREAK(UPF_MDMM_PM_SYNC_MAX_OBJ_EACH_MOTID <= bSthCnt);
                strncpy(tPO60908Ack.tTotalPMObject[bSthCnt++].cPMObjectName, cTmp, sizeof(tPO60908Ack.tTotalPMObject[0].cPMObjectName)-1);
            }
            continue;
        }
        memset(cTmp, 0, sizeof(cTmp));
        getThreadName(cTmp, SC_NAME_NEAR_COMPUTING, dwSelfLogicNo, tThreadPara.atThreadParaList[bSthIndex].bvCPUNo);
        GDMM_NCU_VALID_BREAK(UPF_MDMM_PM_SYNC_MAX_OBJ_EACH_MOTID <= bSthCnt);
        strncpy(tPO60908Ack.tTotalPMObject[bSthCnt++].cPMObjectName, cTmp, sizeof(tPO60908Ack.tTotalPMObject[0].cPMObjectName)-1);
    }
    tPO60908Ack.dwValidCnt = bSthCnt;

    xosRet = XOS_GetSelfJID(&sendJid);
    GDMM_NCU_VALID_CHK(XOS_SUCCESS != xosRet);
    xosRet = XOS_Sender((VOID *)&recvJid);
    GDMM_NCU_VALID_CHK(XOS_SUCCESS != xosRet);
    xosRet = XOS_SendAsynMsg(EV_UPF_MDMM_TO_GDMM_QUERY_DYNPMOBJ_ACK,(BYTE *)&tPO60908Ack, sizeof(T_Gdmm2Mdmm_PMObjInfo), XOS_MSG_VER0, &recvJid, &sendJid);
    if(XOS_SUCCESS !=xosRet)
    {
        _DYN_GDMM_PRINTF("Send EV_UPF_MDMM_TO_GDMM_QUERY_DYNPMOBJ_ACK FAIL!! xosRet = %d !!\n",xosRet);
    }
    return;
}

VOID psMdmm_ToGdmm_PmObjectPfuLogicNoProc(const VOID *msg)
{
    T_Gdmm2Mdmm_PMObjInfo     tPfuLogicNoAck = {0};
    T_Mdmm2Gdmm_PMObjectQueryInfo*  ptReq = NULL;
    JID         recvJid;
    JID         sendJid;
    XOS_STATUS  xosRet;

    GDMM_NCU_NULL_CHK(msg);
    ptReq = (T_Mdmm2Gdmm_PMObjectQueryInfo*)msg;

    WORD32 dwSelfLogicNo  =  ScGetSelfLogNo();
    tPfuLogicNoAck.dwRelativeMotidCnt = 1;
    tPfuLogicNoAck.dwMotids[0] = PM_SYNC_MOTID_60907;

    tPfuLogicNoAck.dwValidCnt = 1;
    XOS_snprintf(tPfuLogicNoAck.tTotalPMObject[0].cPMObjectName, 10, "%d", dwSelfLogicNo);

    xosRet = XOS_GetSelfJID(&sendJid);
    GDMM_NCU_VALID_CHK(XOS_SUCCESS != xosRet);

    xosRet = XOS_Sender((VOID *)&recvJid);
    GDMM_NCU_VALID_CHK(XOS_SUCCESS != xosRet);

    xosRet = XOS_SendAsynMsg(EV_UPF_MDMM_TO_GDMM_QUERY_DYNPMOBJ_ACK,(BYTE *)&tPfuLogicNoAck, sizeof(T_Gdmm2Mdmm_PMObjInfo), XOS_MSG_VER0, &recvJid, &sendJid);
    if(XOS_SUCCESS !=xosRet)
    {
        _DYN_GDMM_PRINTF("Send EV_UPF_MDMM_TO_GDMM_QUERY_DYNPMOBJ_ACK FAIL!! xosRet = %d !!\n",xosRet);
    }
    return;
}

VOID psMdmm_ToGdmm_PmObjectQueryInfo(const VOID *msg, WORD16 wMsgLen)
{
    GDMM_NCU_NULL_CHK(msg);
    GDMM_NCU_VALID_CHK(sizeof(T_Mdmm2Gdmm_PMObjectQueryInfo) != wMsgLen);
    _DYN_GDMM_PRINTF("********** EV_UPF_MDMM_TO_GDMM_QUERY_DYNPMOBJ_REQ enter**********\n");
    psMdmm_ToGdmm_PmObjectVcpuInfoProc(msg);
    psMdmm_ToGdmm_PmObjectPfuLogicNoProc(msg);
    return;
}

T_psGwDMMShowNcuinfoAck     tGwDMMShowPfuinfoAck = {0};
MCS_DM_QUERYDYNDATA_ACK     tMcsDynCtxNoUniqAck  = {0};
void psVpfGdmmGetAllSubScriberCtxByUpseid(WORD64 UPSeid)
{
    WORD32 dwIdx  = 0;
    WORD32 ctxid  = 0;
    _DYN_GDMM_PRINTF("[psVpfGdmmGetAllSubScriberCtxByUpseid] enter!\n");
    MCS_DM_QUERYDYNDATA_ACK *ack = &g_ptVpfuShMemVar->tGwGloData.atMcsDynCtxNoUniqAck[MEDIA_THRD_NUM];
    for (dwIdx = 0; dwIdx < XDB_PFU_DB_NUM; dwIdx++)
    {
        if (NULL == g_tPfuDbSysLocal[dwIdx].pDataBaseMemAddr)
        {
            _DYN_GDMM_PRINTF("[psVpfGdmmGetAllSubScriberCtxByUpseid]g_tPfuDbSysLocal[%d].pDataBaseMemAddr is NULL\n", dwIdx);
            continue;
        }

        WORD32 dwSubscribeNum = psVpfuMcsGetAllSubScribeCtxByUPseid(UPSeid, g_tPfuDbSysLocal[dwIdx].hDB, (BYTE*)ack);
        WORD32 dwAppidList[EXPECT_SUBSCRIBE_NUM] = {0};
        WORD32 num = 0;
        WORD32 subIdx;
        WORD32 dwCtxId = 0;
        _DYN_GDMM_PRINTF("dwSubscribeNum:%u\n", dwSubscribeNum);
        for (subIdx = 0; subIdx < EXPECT_SUBSCRIBE_NUM && subIdx < dwSubscribeNum; subIdx++)
        {
            dwCtxId = ack->adwDataArea[subIdx];
            _DYN_GDMM_PRINTF("loop sub index dwCtxId:%u \n", dwCtxId);
            T_psNcuDaSubScribeCtx *ptSubScribeCtx = psMcsGetsubscribeCtxById(dwCtxId, g_tPfuDbSysLocal[dwIdx].hDB);
            if (NULL != ptSubScribeCtx)
            {
                dwAppidList[num] = ptSubScribeCtx->dwAppId;
                _DYN_GDMM_PRINTF("[psVpfGdmmGetAllSubScriberCtxByUpseid].dwAppidList[num]:%u\n", dwAppidList[num]);
                num++;
            }
        }

        WORD32 streamIdx;

        for(streamIdx = 0; streamIdx < num; streamIdx++)
        {
            _DYN_GDMM_PRINTF("loop stream index streamIdx:%u \n", streamIdx);
            WORD32 dwAppctxNum = psVpfuMcsGetAllDaAppCtxByAppid(UPSeid, dwAppidList[streamIdx], g_tPfuDbSysLocal[dwIdx].hDB, (BYTE*)ack);
            _DYN_GDMM_PRINTF("loop stream index dwAppctxNum:%u \n", dwAppctxNum);
            for(ctxid = 0; ctxid < dwAppctxNum && ctxid < MAX_SHOWNCUSUBSCRIBERSESSIONSINFOACK_ATUSERFLOW_NUM; ctxid++)
            {
                _DYN_GDMM_PRINTF("loop stream index ctxid:%u \n", ctxid);
                dwCtxId = ack->adwDataArea[ctxid];
                T_psNcuDaAppCtx* DaCtxTmp = psMcsGetDaAppCtxById(dwCtxId, g_tPfuDbSysLocal[dwIdx].hDB);
                GDMM_NCU_VALID_CONTINUE(NULL == DaCtxTmp);
                //获取流信息填充ack
                _DYN_GDMM_PRINTF("stream info get success\n");
                T_psNcuFlowCtx *ptTmpStream = (T_psNcuFlowCtx *)DaCtxTmp->ptFlowCtxHead;
                if (NULL == ptTmpStream)
                {
                    _DYN_GDMM_PRINTF("ptTmpStream is NULL\n");
                    continue;
                }
                else
                {
                    _DYN_GDMM_PRINTF("trans streams begin\n");
                    WORD32 dwLoop = 0;
                    WORD32 flowIdx;
                    while (NULL != ptTmpStream && tGwDMMShowPfuinfoAck.UserFlowTotalNumber < MAX_SHOWNCUSUBSCRIBERSESSIONSINFOACK_ATUSERFLOW_NUM)
                    {
                        flowIdx = tGwDMMShowPfuinfoAck.UserFlowTotalNumber;
                        lp_r_flowctx_idx_tuple ptQuintuple = (lp_r_flowctx_idx_tuple)psVpfuMcsGetCtxIdxById(ptTmpStream->dwFlowID, ptTmpStream->dwhDBByThreadNo, DB_HANDLE_R_NCUFLOWCTX);
                        if(NULL == ptQuintuple)
                        {
                            ptTmpStream = (T_psNcuFlowCtx *)ptTmpStream->ptNcuFlowAppNext;
                            dwLoop++;
                            if (dwLoop >= 10)
                            {
                                return;
                            }
                            continue;
                        }
                        _DYN_GDMM_PRINTF("after get tuple tupletype:%u\n", ptQuintuple->bIPType);
                        if (4 == ptQuintuple->bIPType)
                        {
                            inet_ntop(AF_INET, ptQuintuple->tSvrIP, tGwDMMShowPfuinfoAck.atuserflow[flowIdx].networkip, sizeof(tGwDMMShowPfuinfoAck.ueipv4address));
                            inet_ntop(AF_INET, ptQuintuple->tCliIP, tGwDMMShowPfuinfoAck.atuserflow[flowIdx].subscriberip, sizeof(tGwDMMShowPfuinfoAck.ueipv4address));
                            _DYN_GDMM_PRINTF("subscriberip: %s\n", tGwDMMShowPfuinfoAck.atuserflow[flowIdx].subscriberip);
                            _DYN_GDMM_PRINTF("networkip:    %s\n", tGwDMMShowPfuinfoAck.atuserflow[flowIdx].networkip);
                        }
                        else
                        {
                            zte_snprintf_s(tGwDMMShowPfuinfoAck.atuserflow[flowIdx].networkip, sizeof(tGwDMMShowPfuinfoAck.atuserflow[flowIdx].networkip), "%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x",
                                           ptQuintuple->tSvrIP[0], ptQuintuple->tSvrIP[1], ptQuintuple->tSvrIP[2], ptQuintuple->tSvrIP[3],
                                           ptQuintuple->tSvrIP[4], ptQuintuple->tSvrIP[5], ptQuintuple->tSvrIP[6], ptQuintuple->tSvrIP[7],
                                           ptQuintuple->tSvrIP[8], ptQuintuple->tSvrIP[9], ptQuintuple->tSvrIP[10], ptQuintuple->tSvrIP[11],
                                           ptQuintuple->tSvrIP[12], ptQuintuple->tSvrIP[13], ptQuintuple->tSvrIP[14], ptQuintuple->tSvrIP[15]);
                            zte_snprintf_s(tGwDMMShowPfuinfoAck.atuserflow[flowIdx].subscriberip, sizeof(tGwDMMShowPfuinfoAck.atuserflow[flowIdx].subscriberip), "%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x",
                                           ptQuintuple->tCliIP[0], ptQuintuple->tCliIP[1], ptQuintuple->tCliIP[2], ptQuintuple->tCliIP[3],
                                           ptQuintuple->tCliIP[4], ptQuintuple->tCliIP[5], ptQuintuple->tCliIP[6], ptQuintuple->tCliIP[7],
                                           ptQuintuple->tCliIP[8], ptQuintuple->tCliIP[9], ptQuintuple->tCliIP[10], ptQuintuple->tCliIP[11],
                                           ptQuintuple->tCliIP[12], ptQuintuple->tCliIP[13], ptQuintuple->tCliIP[14], ptQuintuple->tCliIP[15]);
                            _DYN_GDMM_PRINTF("v6 subscriberip: %s\n", tGwDMMShowPfuinfoAck.atuserflow[flowIdx].subscriberip);
                            _DYN_GDMM_PRINTF("v6 networkip:    %s\n", tGwDMMShowPfuinfoAck.atuserflow[flowIdx].networkip);
                        }
                        tGwDMMShowPfuinfoAck.atuserflow[flowIdx].subscriberport = ptQuintuple->wCliPort;
                        tGwDMMShowPfuinfoAck.atuserflow[flowIdx].networkport    = ptQuintuple->wSvrPort;
                        psNcuGetCfgStrAppidMapByInnerId(DaCtxTmp->dwAppid, tGwDMMShowPfuinfoAck.atuserflow[flowIdx].applicationid);
                        tGwDMMShowPfuinfoAck.atuserflow[flowIdx].subappId       = DaCtxTmp->dwSubAppid;
                        tGwDMMShowPfuinfoAck.atuserflow[flowIdx].qci            = ptTmpStream->b5QI;
                        _DYN_GDMM_PRINTF("[psVpfGdmmGetAllSubScriberCtxByUpseid]flowIdx       :%u\n", flowIdx);
                        _DYN_GDMM_PRINTF("[psVpfGdmmGetAllSubScriberCtxByUpseid]subscriberport:%u\n", tGwDMMShowPfuinfoAck.atuserflow[flowIdx].subscriberport);
                        _DYN_GDMM_PRINTF("[psVpfGdmmGetAllSubScriberCtxByUpseid]networkport   :%u\n", tGwDMMShowPfuinfoAck.atuserflow[flowIdx].networkport);
                        _DYN_GDMM_PRINTF("[psVpfGdmmGetAllSubScriberCtxByUpseid]applicationid :%s\n", tGwDMMShowPfuinfoAck.atuserflow[flowIdx].applicationid);
                        _DYN_GDMM_PRINTF("[psVpfGdmmGetAllSubScriberCtxByUpseid]subappId      :%u\n", tGwDMMShowPfuinfoAck.atuserflow[flowIdx].subappId);
                        _DYN_GDMM_PRINTF("[psVpfGdmmGetAllSubScriberCtxByUpseid]qci           :%u\n", tGwDMMShowPfuinfoAck.atuserflow[flowIdx].qci);
                        zte_strncpy_s(tGwDMMShowPfuinfoAck.atuserflow[flowIdx].subAppidStr, MAX_SHOWNCUSUBSCRIBERSESSIONSINFOACKATUSERFLOW_SUBAPPIDSTR_LEN, DaCtxTmp->subAppidStr, MAX_SHOWNCUSUBSCRIBERSESSIONSINFOACKATUSERFLOW_SUBAPPIDSTR_LEN-1);
                        tGwDMMShowPfuinfoAck.UserFlowTotalNumber++;
                        dwLoop++;
                        if (dwLoop >= 10)
                        {
                            return;
                        }
                        ptTmpStream = (T_psNcuFlowCtx *)ptTmpStream->ptNcuFlowAppNext;
                    }
                }
            }
        }
    }
    return ;
}

void psVpfGdmmGetAllSessionCtxByUpseid(WORD64 UPSeid, T_psSessionInfoForNcu *ptSessionInfo)
{
    GDMM_NCU_NULL_CHK(ptSessionInfo); 
    GDMM_NCU_VALID_CHK(ptSessionInfo->dwPfuSessCount >= MAX_PFU_SESSION_NUM);

    T_psNcuSessionCtx *ptVpfuSesionNode = NULL;
    WORD32 dwIdx  = 0;

    for (dwIdx = 0; dwIdx < XDB_PFU_DB_NUM; dwIdx++)
    {
        if (NULL == g_tPfuDbSysLocal[dwIdx].pDataBaseMemAddr)
        {
            _DYN_GDMM_PRINTF("[psVpfGdmmGetAllSessionCtxByUpseid]g_tPfuDbSysLocal[dwIdx].pDataBaseMemAddr is NULL\n");
            continue;
        }

        ptVpfuSesionNode = psQuerySessionByUpseid(UPSeid, g_tPfuDbSysLocal[dwIdx].hDB);
        GDMM_NCU_VALID_CONTINUE(NULL == ptVpfuSesionNode);
        ptSessionInfo->ptSessionCtx[ptSessionInfo->dwPfuSessCount] = (void *)ptVpfuSesionNode;
        ptSessionInfo->qwUpSeid[ptSessionInfo->dwPfuSessCount] = ptVpfuSesionNode->ddwUPSeid;
        ptSessionInfo->whDB[ptSessionInfo->dwPfuSessCount] = g_tPfuDbSysLocal[dwIdx].hDB;
        _DYN_GDMM_PRINTF("[psVpfGdmmGetAllSessionCtxByUpseid]ddwUPSeid:%llu\n", ptSessionInfo->qwUpSeid[ptSessionInfo->dwPfuSessCount]);
        _DYN_GDMM_PRINTF("[psVpfGdmmGetAllSessionCtxByUpseid]hDB      :%u\n"  , ptSessionInfo->whDB[ptSessionInfo->dwPfuSessCount]);
        ptSessionInfo->dwPfuSessCount++;
        if (ptSessionInfo->dwPfuSessCount >= MAX_PFU_SESSION_NUM)
        {
            return;
        }
    }
    return ;
}

void psVpfGdmmGetAllSessionCtxByIMSI(BYTE *ptIMSI, T_psSessionInfoForNcu *ptSessionInfo)
{
    GDMM_NCU_NULL_CHK(ptIMSI); 
    GDMM_NCU_NULL_CHK(ptSessionInfo); 
    if(ptSessionInfo->dwPfuSessCount >= MAX_PFU_SESSION_NUM)
    {
        return;
    }

    T_psNcuSessionCtx * ptVpfuSesionNode = NULL;
    WORD32 dwIdx    = 0;
    WORD32 ctxnum   = 0;
    WORD32 i        = 0;
    WORD32 ctxid    = 0;
    /* 动态命令无法获取具体的线程号，使用额外MEDIA_THRD_NUM数据区 */
    MCS_DM_QUERYDYNDATA_ACK *ack = &g_ptVpfuShMemVar->tGwGloData.atMcsDynCtxNoUniqAck[MEDIA_THRD_NUM];
    for(dwIdx = 0; dwIdx < XDB_PFU_DB_NUM; dwIdx++)
    {
        if (NULL == g_tPfuDbSysLocal[dwIdx].pDataBaseMemAddr)
        {
            _DYN_GDMM_PRINTF("[psVpfGdmmGetSessionCtxByIMSI]g_tPfuDbSysLocal[dwIdx].pDataBaseMemAddr is NULL\n");
            continue;
        }

        /* 获取session ctx */
        ctxnum = psVpfuMcsGetSessionCtxNumByIMSI(ptIMSI, g_tPfuDbSysLocal[dwIdx].hDB, (BYTE*)ack);
        if (0 == ctxnum)
        {
            _DYN_GDMM_PRINTF("[psVpfGdmmGetSessionCtxByIMSI]ctxnum is [0], hdb: %d\n", g_tPfuDbSysLocal[dwIdx].hDB);
            continue;
        }

        for (i = 0; i < MIN(ctxnum, MCS_DYNCTX_NOUNIQ_EXPECTNUM); i++)
        {
            ctxid = ack->adwDataArea[i];
            ptVpfuSesionNode = (T_psNcuSessionCtx *)psMcsGetSessionCtxById(ctxid, g_tPfuDbSysLocal[dwIdx].hDB);
            GDMM_NCU_VALID_CONTINUE(NULL == ptVpfuSesionNode);
            ptSessionInfo->ptSessionCtx[ptSessionInfo->dwPfuSessCount] = (void *)ptVpfuSesionNode;
            ptSessionInfo->whDB[ptSessionInfo->dwPfuSessCount] = g_tPfuDbSysLocal[dwIdx].hDB;
            ptSessionInfo->qwUpSeid[ptSessionInfo->dwPfuSessCount] = ptVpfuSesionNode->ddwUPSeid;
            _DYN_GDMM_PRINTF("[psVpfGdmmGetSessionCtxByIMSI]i:%d dwIdx:%d ddwUPSeid: %llu, hdb: %d\n", i, dwIdx, ptVpfuSesionNode->ddwUPSeid, g_tPfuDbSysLocal[dwIdx].hDB);
            ptSessionInfo->dwPfuSessCount++;
            if(ptSessionInfo->dwPfuSessCount >= MAX_PFU_SESSION_NUM)
            {
                return;
            }
        }
    }
    return ;
}

#define MML_STR_BCD_LEN 17
static void  XGW_BCD_To_String( BYTE * inPutBCD, CHAR * outPutStr )
{
    BYTE BCDcode[8];
    BYTE j = 0;
    BYTE i = 0;
    CHAR strtemp[MML_STR_BCD_LEN];
    if (NULL == inPutBCD || NULL == outPutStr)
    {
        return ;
    }
    zte_memset_s(strtemp, sizeof(strtemp), 0, sizeof(strtemp));
    zte_memcpy_s(BCDcode, sizeof(BCDcode), inPutBCD, sizeof(BCDcode));
    for ( i = 0 ; i < 8 ; i ++)
    {
        strtemp[j]   = BCDcode[i]%16 +'0';
        if (15 == (strtemp[j]-'0'))
        {
            strtemp[j]   = '\0';
            break;
        }
        strtemp[j+1] = BCDcode[i]/16 +'0';
        if (15 == (strtemp[j+1]-'0'))
        {
            strtemp[j+1] = '\0';
            break;
        }
        j+=2;
    }
    zte_memcpy_s(outPutStr,sizeof(strtemp), strtemp, sizeof(strtemp));
    return ;
}

void psVpfuGdmmFillPfuSessionProc(T_psSessionInfoForNcu *ptSessionInfo, const T_psGwDMMShowNcuinfoReq *ptGwDMMShowPfuinfoReq, 
                                    BYTE *pbNewFileFlag)
{
    GDMM_NCU_NULL_CHK(ptSessionInfo);
    GDMM_NCU_NULL_CHK(ptGwDMMShowPfuinfoReq);
    GDMM_NCU_NULL_CHK(pbNewFileFlag);
    _DYN_GDMM_PRINTF("psVpfuGdmmFillPfuSessionProc enter !!!!!!\n");

    JID                         recvJid;
    JID                         sendJid;
    XOS_STATUS                  xosRet;
    WORD32                      index = 0;
    T_psNcuSessionCtx           *ptVpfuSesionNode = NULL;
    
    xosRet = XOS_GetSelfJID(&sendJid);
    GDMM_NCU_VALID_CHK(XOS_SUCCESS != xosRet);
    xosRet = XOS_Sender((VOID *)&recvJid);
    GDMM_NCU_VALID_CHK(XOS_SUCCESS != xosRet);

    _DYN_GDMM_PRINTF("ptSessionInfo->dwPfuSessCount: %u\n", ptSessionInfo->dwPfuSessCount);
    for(index = 0; index < ptSessionInfo->dwPfuSessCount && index < MAX_PFU_SESSION_NUM; index++)
    {
        _DYN_GDMM_PRINTF("index: %u\n", index);
        zte_memset_s(&tGwDMMShowPfuinfoAck, sizeof(T_psGwDMMShowNcuinfoAck), 0, sizeof(T_psGwDMMShowNcuinfoAck));
        /* req and result */
        zte_memcpy_s(&tGwDMMShowPfuinfoAck.tGwDMMShowPfuinfoReq, sizeof(T_psGwDMMShowNcuinfoReq), ptGwDMMShowPfuinfoReq, sizeof(T_psGwDMMShowNcuinfoReq));
        tGwDMMShowPfuinfoAck.bSessionNum = MDMM_NCU_MIN(ptSessionInfo->dwPfuSessCount, MAX_PFU_SESSION_NUM);
        tGwDMMShowPfuinfoAck.bCurrSessionNum = index + 1;
        _DYN_GDMM_PRINTF("bSessionNum: %u\n", tGwDMMShowPfuinfoAck.bSessionNum);
        _DYN_GDMM_PRINTF("bCurrSessionNum: %u\n", tGwDMMShowPfuinfoAck.bCurrSessionNum);
        tGwDMMShowPfuinfoAck.result = QUE_SESSION_RST_SUCC;

        _DYN_GDMM_PRINTF("ptVpfuMcsMsipCtx!=null!\n");
        ptVpfuSesionNode = (T_psNcuSessionCtx *)ptSessionInfo->ptSessionCtx[index];
        GDMM_NCU_VALID_CONTINUE(NULL == ptVpfuSesionNode);

        /* upseid, imsi, msisdn */
        upf_snprintf(tGwDMMShowPfuinfoAck.upseid, MAX_SHOWPFUSUBSCRIBERSESSIONSINFOACK_UPSEID_LEN, "0x%016llx", ptSessionInfo->qwUpSeid[index]);
        _DYN_GDMM_PRINTF("tGwDMMShowPfuinfoAck.upseid=%s\n",tGwDMMShowPfuinfoAck.upseid);
        XGW_BCD_To_String(ptVpfuSesionNode->bImsi,   tGwDMMShowPfuinfoAck.imsi);
        XGW_BCD_To_String(ptVpfuSesionNode->bIsdn, tGwDMMShowPfuinfoAck.msisdn);
        _DYN_GDMM_PRINTF("tGwDMMShowPfuinfoAck.imsi=%s\n",   tGwDMMShowPfuinfoAck.imsi);
        _DYN_GDMM_PRINTF("tGwDMMShowPfuinfoAck.msisdn=%s\n", tGwDMMShowPfuinfoAck.msisdn);
        /* iptype, ipaddr */
        tGwDMMShowPfuinfoAck.ueipversion = ptVpfuSesionNode->bMSIPType;
        if(IPCOMM_TYPE_IPV4 == ptVpfuSesionNode->bMSIPType)
        {
            inet_ntop(AF_INET, ptVpfuSesionNode->tMSIPv4, tGwDMMShowPfuinfoAck.ueipv4address, sizeof(tGwDMMShowPfuinfoAck.ueipv4address));
        }
        else if(IPCOMM_TYPE_IPV6 == ptVpfuSesionNode->bMSIPType)
        {
            inet_ntop(AF_INET6, ptVpfuSesionNode->tMSIPv6, tGwDMMShowPfuinfoAck.ueipv6address, sizeof(tGwDMMShowPfuinfoAck.ueipv6address));
        }
        else
        {
            inet_ntop(AF_INET, ptVpfuSesionNode->tMSIPv4,  tGwDMMShowPfuinfoAck.ueipv4address, sizeof(tGwDMMShowPfuinfoAck.ueipv4address));
            inet_ntop(AF_INET6, ptVpfuSesionNode->tMSIPv6, tGwDMMShowPfuinfoAck.ueipv6address, sizeof(tGwDMMShowPfuinfoAck.ueipv6address));
        }

        /* prefix, sclogicno */
        tGwDMMShowPfuinfoAck.sclogicno              = ncuGetScLogicNo();
        _DYN_GDMM_PRINTF("sclogicno=%d\n", tGwDMMShowPfuinfoAck.sclogicno);
        /* vru, mgtip */
        CHAR* vru_name = getenv("vru_name");
        if(NULL != vru_name)
        {
            zte_strncpy_s(tGwDMMShowPfuinfoAck.vruinst, MAX_SHOWNCUSUBSCRIBERSESSIONSINFOACK_VRUINST_LEN, vru_name, MAX_SHOWNCUSUBSCRIBERSESSIONSINFOACK_VRUINST_LEN-1);
        }
        CHAR* local_mgt_ip = getenv("local_mgt_ip");
        if(NULL != local_mgt_ip)
        {
            zte_strncpy_s(tGwDMMShowPfuinfoAck.mgtip, MAX_SHOWNCUSUBSCRIBERSESSIONSINFOACK_MGTIP_LEN, local_mgt_ip,MAX_SHOWNCUSUBSCRIBERSESSIONSINFOACK_MGTIP_LEN-1);
        }
        /* snssai */
        _DYN_GDMM_PRINTF("ptVpfuSesionNode->tSNssai.ucSST=%d\n",ptVpfuSesionNode->tSNssai.ucSST);
        _DYN_GDMM_PRINTF("ptVpfuSesionNode->tSNssai.aucSD=%02x%02x%02x\n",ptVpfuSesionNode->tSNssai.aucSD[0],ptVpfuSesionNode->tSNssai.aucSD[1],ptVpfuSesionNode->tSNssai.aucSD[2]);
        if(SNSSAI_EMBB == ptVpfuSesionNode->tSNssai.ucSST)
        {
            upf_snprintf(tGwDMMShowPfuinfoAck.snssai, MAX_SHOWNCUSUBSCRIBERSESSIONSINFOACK_SNSSAI_LEN, "%s-%02x%02x%02x", "eMBB", ptVpfuSesionNode->tSNssai.aucSD[0], ptVpfuSesionNode->tSNssai.aucSD[1],ptVpfuSesionNode->tSNssai.aucSD[2]);
        }
        else if(SNSSAI_URLLC == ptVpfuSesionNode->tSNssai.ucSST)
        {
            upf_snprintf(tGwDMMShowPfuinfoAck.snssai, MAX_SHOWNCUSUBSCRIBERSESSIONSINFOACK_SNSSAI_LEN, "%s-%02x%02x%02x", "URLLC", ptVpfuSesionNode->tSNssai.aucSD[0], ptVpfuSesionNode->tSNssai.aucSD[1],ptVpfuSesionNode->tSNssai.aucSD[2]);
        }
        else if(SNSSAI_MIOT == ptVpfuSesionNode->tSNssai.ucSST)
        {
            upf_snprintf(tGwDMMShowPfuinfoAck.snssai, MAX_SHOWNCUSUBSCRIBERSESSIONSINFOACK_SNSSAI_LEN, "%s-%02x%02x%02x", "MloT", ptVpfuSesionNode->tSNssai.aucSD[0], ptVpfuSesionNode->tSNssai.aucSD[1],ptVpfuSesionNode->tSNssai.aucSD[2]);
        }
        else if(SNSSAI_V2X == ptVpfuSesionNode->tSNssai.ucSST)
        {
            upf_snprintf(tGwDMMShowPfuinfoAck.snssai, MAX_SHOWNCUSUBSCRIBERSESSIONSINFOACK_SNSSAI_LEN, "%s-%02x%02x%02x", "V2X", ptVpfuSesionNode->tSNssai.aucSD[0], ptVpfuSesionNode->tSNssai.aucSD[1],ptVpfuSesionNode->tSNssai.aucSD[2]);
        }
        else
        {
            upf_snprintf(tGwDMMShowPfuinfoAck.snssai, MAX_SHOWNCUSUBSCRIBERSESSIONSINFOACK_SNSSAI_LEN, "%u-%02x%02x%02x", ptVpfuSesionNode->tSNssai.ucSST, ptVpfuSesionNode->tSNssai.aucSD[0], ptVpfuSesionNode->tSNssai.aucSD[1],ptVpfuSesionNode->tSNssai.aucSD[2]);
        }

        /* apn, rattype, stamp */
        zte_strncpy_s(tGwDMMShowPfuinfoAck.apn_dnn, MAX_SHOWNCUSUBSCRIBERSESSIONSINFOACK_APN_DNN_LEN, ptVpfuSesionNode->Apn, MAX_SHOWNCUSUBSCRIBERSESSIONSINFOACK_APN_DNN_LEN-1);
        tGwDMMShowPfuinfoAck.rattype                = ptVpfuSesionNode->bRatType;
        tGwDMMShowPfuinfoAck.UpdateTimeStamp        = ptVpfuSesionNode->dwUpdateTimeStamp;
        _DYN_GDMM_PRINTF("tGwDMMShowPfuinfoAck.rattype=%d\n",           tGwDMMShowPfuinfoAck.rattype);
        _DYN_GDMM_PRINTF("tGwDMMShowPfuinfoAck.UpdateTimeStamp=%d\n",   tGwDMMShowPfuinfoAck.UpdateTimeStamp);
        /* QCI/5GQI待确认 */

        /* 详细信息获取 */
        if(ptGwDMMShowPfuinfoReq->type == 1)
        {
            _DYN_GDMM_PRINTF("ptVpfuSesionNode->bSubType = %u\n",           ptVpfuSesionNode->bSubType);
            zte_memcpy_s(tGwDMMShowPfuinfoAck.subInfo.CorralationIdNormal, AUCCORID_MAXLEN, "NA", sizeof("NA")+1);
            zte_memcpy_s(tGwDMMShowPfuinfoAck.subInfo.CorralationIdAna, AUCCORID_MAXLEN, "NA", sizeof("NA")+1);
            zte_memcpy_s(tGwDMMShowPfuinfoAck.subInfo.UriAna, AUCREPORTURI_MAXLEN, "NA", sizeof("NA")+1);
            zte_memcpy_s(tGwDMMShowPfuinfoAck.subInfo.CorralationIdExp, AUCCORID_MAXLEN, "NA", sizeof("NA")+1);
            zte_memcpy_s(tGwDMMShowPfuinfoAck.subInfo.UriExp, AUCREPORTURI_MAXLEN, "NA", sizeof("NA")+1);
            zte_memcpy_s(tGwDMMShowPfuinfoAck.subInfo.NWDAFIP,   MAX_SHOWNCUSUBSCRIBERSESSIONSINFOACK_NWDAFIP_LEN, "NA", sizeof("NA")+1);
            zte_memcpy_s(tGwDMMShowPfuinfoAck.subInfo.NWDAFIPv6, MAX_SHOWNCUSUBSCRIBERSESSIONSINFOACK_NWDAFIP_LEN, "NA", sizeof("NA")+1);
            if (ptVpfuSesionNode->bSubType & enumQosExpNormal)
            {
                WORD32 k;
                //use dnn 查询
                for(k = 0; k < 16; k++)
                {
                    _DYN_GDMM_PRINTF("g_CorrData[k].isValid = %u\n",  g_CorrData[k].isValid);
                    _DYN_GDMM_PRINTF("g_CorrData[k].Dnn     = %s\n",  g_CorrData[k].Dnn);
                    _DYN_GDMM_PRINTF("ptVpfuSesionNode->Apn = %s\n",  ptVpfuSesionNode->Apn);
                    if(g_CorrData[k].isValid && (0 == strcmp(g_CorrData[k].Dnn, ptVpfuSesionNode->Apn)))
                    {
                        zte_memcpy_s(tGwDMMShowPfuinfoAck.subInfo.CorralationIdNormal, AUCCORID_MAXLEN, g_CorrData[k].CorrelationId, AUCCORID_MAXLEN);
                        _DYN_GDMM_PRINTF("g_CorrData[k].CorrelationId = %s\n", g_CorrData[k].CorrelationId);
                        tGwDMMShowPfuinfoAck.SubscribeTotalNumber++;
                    }
                }
            }
            if (ptVpfuSesionNode->bSubType & enumQosExpSpecial)
            {
                _DYN_GDMM_PRINTF("ptVpfuSesionNode->bSubType is enumQosExpSpecial\n");
                zte_memcpy_s(tGwDMMShowPfuinfoAck.subInfo.CorralationIdExp, AUCCORID_MAXLEN, ptVpfuSesionNode->bCorrelationId_Exp, AUCCORID_MAXLEN);
                zte_memcpy_s(tGwDMMShowPfuinfoAck.subInfo.UriExp, AUCREPORTURI_MAXLEN, ptVpfuSesionNode->Uri_Exp, AUCREPORTURI_MAXLEN);
                _DYN_GDMM_PRINTF("CorralationIdExp :%s\n", tGwDMMShowPfuinfoAck.subInfo.CorralationIdExp);
                _DYN_GDMM_PRINTF("UriExp           :%s\n", tGwDMMShowPfuinfoAck.subInfo.UriExp);
                tGwDMMShowPfuinfoAck.SubscribeTotalNumber++;
            }
            if (ptVpfuSesionNode->bSubType & enumQosAna)
            {
                _DYN_GDMM_PRINTF("ptVpfuSesionNode->bSubType is enumQosAna\n");
                zte_memcpy_s(tGwDMMShowPfuinfoAck.subInfo.CorralationIdAna, AUCCORID_MAXLEN, ptVpfuSesionNode->bCorrelationId_Ana, AUCCORID_MAXLEN);
                zte_memcpy_s(tGwDMMShowPfuinfoAck.subInfo.UriAna, AUCREPORTURI_MAXLEN, ptVpfuSesionNode->Uri_Ana, AUCREPORTURI_MAXLEN);
                _DYN_GDMM_PRINTF("CorralationIdAna :%s\n", tGwDMMShowPfuinfoAck.subInfo.CorralationIdAna);
                _DYN_GDMM_PRINTF("UriAna           :%s\n", tGwDMMShowPfuinfoAck.subInfo.UriAna);
                tGwDMMShowPfuinfoAck.SubscribeTotalNumber++;
            }

            if (IPCOMM_TYPE_IPV4 & ptVpfuSesionNode->bNwdafIPType)
            {
                _DYN_GDMM_PRINTF("ptVpfuSesionNode->bNwdafIPType is ipv4\n");
                inet_ntop(AF_INET, ptVpfuSesionNode->tNwdafIPv4, tGwDMMShowPfuinfoAck.subInfo.NWDAFIP, sizeof(tGwDMMShowPfuinfoAck.ueipv4address));
            }
            if (IPCOMM_TYPE_IPV6 & ptVpfuSesionNode->bNwdafIPType)
            {
                _DYN_GDMM_PRINTF("ptVpfuSesionNode->bNwdafIPType is ipv6\n");
                inet_ntop(AF_INET6, ptVpfuSesionNode->tNwdafIPv6, tGwDMMShowPfuinfoAck.subInfo.NWDAFIPv6, sizeof(tGwDMMShowPfuinfoAck.ueipv6address));
            }
            _DYN_GDMM_PRINTF("tGwDMMShowPfuinfoAck.SubscribeTotalNumber is %d\n", tGwDMMShowPfuinfoAck.SubscribeTotalNumber);
            //组装流信息
            psVpfGdmmGetAllSubScriberCtxByUpseid(ptSessionInfo->qwUpSeid[index]);
        }
        _DYN_GDMM_PRINTF("have session bSessionNum2: %u\n", tGwDMMShowPfuinfoAck.bSessionNum);
        _DYN_GDMM_PRINTF("have session bCurrSessionNum2: %u\n", tGwDMMShowPfuinfoAck.bCurrSessionNum);
        if (XOS_SUCCESS != XOS_SendAsynMsg(EV_NCU_GDMM_TO_MDMM_SHOWNCUINFO_ACK,(BYTE *)&tGwDMMShowPfuinfoAck, sizeof(tGwDMMShowPfuinfoAck), XOS_MSG_VER0, &recvJid, &sendJid))
        {
            _DYN_GDMM_PRINTF("[File:%s],[line:%d]DB Module:send EV_NCU_GDMM_TO_MDMM_SHOWPFUINFO_ACK msg fail!\n",__FILE__, __LINE__);
            return ;
        }
    }
    return;
}

WORD64 hexCharToNum(char c)
{
    if (c >= '0' && c <= '9')
    {
        return c - '0';
    }
    else if (c >= 'a' && c <= 'f')
    {
        return c - 'a' + 10;
    }
    else if (c >= 'A' && c <= 'F')
    {
        return c - 'A' + 10;
    }
    else
    {
        return 0;
    }
}

WORD64 hexStringToWORD64(const char *str)
{
    int begin = 0;
    int i;
    if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X'))
        begin = 2;
    WORD64 num = 0;
    for (i = begin; str[i] != '\0' && i < 32; i++)
    {
        num = num * 16 + hexCharToNum(str[i]);
    }
    return num;
}

VOID psNcuMdmm_ToGdmmShowNcuinfo(const VOID *msg, const WORD16 wMsgLen)
{
    const T_psGwDMMShowNcuinfoReq*    ptGwDMMShowPfuinfoReq = NULL;
    zte_memset_s(&tGwDMMShowPfuinfoAck, sizeof(T_psGwDMMShowNcuinfoAck), 0, sizeof(T_psGwDMMShowNcuinfoAck));
    zte_memset_s(&tMcsDynCtxNoUniqAck,  sizeof(MCS_DM_QUERYDYNDATA_ACK), 0, sizeof(MCS_DM_QUERYDYNDATA_ACK));

    JID                         recvJid;
    JID                         sendJid;
    XOS_STATUS                  xosRet;
    T_IMSI                      tIMSI        = {0};
    WORD64                      UPseid       = 0;
    T_psSessionInfoForNcu       tSessionInfo = {0};
    BYTE bNewFileFlag = 0;
    _DYN_GDMM_PRINTF("psNcuMdmm_ToGdmmShowNcuinfo enter !!!!!!!!\n");
    GDMM_NCU_NULL_CHK(msg);
    GDMM_NCU_VALID_CHK(sizeof(T_psGwDMMShowNcuinfoReq) != wMsgLen);
    _DYN_GDMM_PRINTF("check msg success !!!!!!!!\n");

    ptGwDMMShowPfuinfoReq = (T_psGwDMMShowNcuinfoReq *)msg;
    zte_memcpy_s(&tGwDMMShowPfuinfoAck.tGwDMMShowPfuinfoReq, sizeof(T_psGwDMMShowNcuinfoReq), ptGwDMMShowPfuinfoReq, sizeof(T_psGwDMMShowNcuinfoReq));
    tGwDMMShowPfuinfoAck.bSessionNum = 0;
    tGwDMMShowPfuinfoAck.bCurrSessionNum = 0;
    _DYN_GDMM_PRINTF("bSessionNum init: %u\n", tGwDMMShowPfuinfoAck.bSessionNum);
    _DYN_GDMM_PRINTF("bCurrSessionNum init: %u\n", tGwDMMShowPfuinfoAck.bCurrSessionNum);
    xosRet = XOS_GetSelfJID(&sendJid);
    GDMM_NCU_VALID_CHK(XOS_SUCCESS != xosRet);

    xosRet = XOS_Sender((VOID *)&recvJid);
    GDMM_NCU_VALID_CHK(XOS_SUCCESS != xosRet);

    /* 使用 imsi 作为查询条件 */
    if(ptGwDMMShowPfuinfoReq->imsiFlag)
    {
        /* IMSI使用BCD编码 */
        psUpfCommString2tBCD(tIMSI, (BYTE*)(ptGwDMMShowPfuinfoReq->imsi), zte_strnlen_s(ptGwDMMShowPfuinfoReq->imsi, MAX_SHOWNCUSUBSCRIBERSESSIONSINFOREQ_IMSI_LEN));
        _DYN_GDMM_PRINTF("[psNcuMdmm_ToGdmmShowNcuinfo] IMSI:%x %x %x %x %x %x %x %x \n", tIMSI[0], tIMSI[1], tIMSI[2], tIMSI[3], tIMSI[4], tIMSI[5], tIMSI[6], tIMSI[7]);
        psVpfGdmmGetAllSessionCtxByIMSI(tIMSI, &tSessionInfo);
    }
    else if(ptGwDMMShowPfuinfoReq->upseidFlag)
    {
        UPseid = hexStringToWORD64(ptGwDMMShowPfuinfoReq->upseid);
        _DYN_GDMM_PRINTF("[psNcuMdmm_ToGdmmShowNcuinfo] UPseid:%llu \n", UPseid);
        psVpfGdmmGetAllSessionCtxByUpseid(UPseid, &tSessionInfo);
    }

    _DYN_GDMM_PRINTF("tSessionInfo.dwPfuSessCount = %u\n", tSessionInfo.dwPfuSessCount);
    if (0 == tSessionInfo.dwPfuSessCount)
    {
        tGwDMMShowPfuinfoAck.result = QUE_SESSION_RST_FAIL;
        if(XOS_SUCCESS != XOS_SendAsynMsg(EV_NCU_GDMM_TO_MDMM_SHOWNCUINFO_ACK,(BYTE *)&tGwDMMShowPfuinfoAck, sizeof(tGwDMMShowPfuinfoAck), XOS_MSG_VER0, &recvJid, &sendJid))
        {
            _DYN_GDMM_PRINTF("[File:%s],[line:%d]DB Module:send EV_NCU_GDMM_TO_MDMM_SHOWNCUINFO_ACK msg fail!\n",__FILE__, __LINE__);
        }
        _DYN_GDMM_PRINTF("no session bSessionNum: %u\n", tGwDMMShowPfuinfoAck.bSessionNum);
        _DYN_GDMM_PRINTF("no session bCurrSessionNum: %u\n", tGwDMMShowPfuinfoAck.bCurrSessionNum);
        return;
    }

    /*Ack赋值处理*/
    if(0 < tSessionInfo.dwPfuSessCount)
    {
        psVpfuGdmmFillPfuSessionProc(&tSessionInfo, ptGwDMMShowPfuinfoReq, &bNewFileFlag);
        _DYN_GDMM_PRINTF("have session bSessionNum: %u\n", tGwDMMShowPfuinfoAck.bSessionNum);
        _DYN_GDMM_PRINTF("have session bCurrSessionNum: %u\n", tGwDMMShowPfuinfoAck.bCurrSessionNum);
    }
    return;
}

/* Started by AICoder, pid:u95fem5aa2mda4314ca60bba3050ed99e8d7d8b0 */
/* Started by AICoder Product */
static T_psGdmmShowNcuPodCpuUsageAck g_tGdmmShowNcuPodCpuUsageAck;

VOID psNcuMdmm_ToGdmmShowNcuPodCpuUsage(const VOID *msg, const WORD16 wMsgLen)
{
    _DYN_GDMM_PRINTF("psNcuMdmm_ToGdmmShowNcuPodCpuUsage enter !\n");
    GDMM_NCU_NULL_CHK(msg);

    T_psGdmmShowNcuPodCpuUsageReq *ptGdmmShowNcuPodCpuUsageReq = NULL;
    GDMM_UPF_VALID_CHECK_ACTION(sizeof(T_psGdmmShowNcuPodCpuUsageReq) != wMsgLen, _DYN_GDMM_PRINTF("Error! wMsgLen Failed!\n"));
    ptGdmmShowNcuPodCpuUsageReq = (T_psGdmmShowNcuPodCpuUsageReq *)msg;
    _DYN_GDMM_PRINTF("[psNcuMdmm_ToGdmmShowNcuPodCpuUsage] sclogicnoFlag:%d\n sclogicno :%d \n", ptGdmmShowNcuPodCpuUsageReq->sclogicnoFlag, ptGdmmShowNcuPodCpuUsageReq->sclogicno);
    WORD32 dwSelfLogicNo = ScGetSelfLogNo();
    if ((0 != ptGdmmShowNcuPodCpuUsageReq->sclogicnoFlag) && (dwSelfLogicNo != ptGdmmShowNcuPodCpuUsageReq->sclogicno))
    {
        _DYN_GDMM_PRINTF("msg send err, sclogicno wrong %d!\n", ptGdmmShowNcuPodCpuUsageReq->sclogicno);
        return;
    }

    T_PSThreadInstAffinityPara tThreadPara = {0};
    WORD32 dwPsmStatus = psGetMediaThreadInfo(THDM_MEDIA_TYPE_ALL, &tThreadPara);
    GDMM_UPF_VALID_CHECK_ACTION(THDM_SUCCESS != dwPsmStatus, _DYN_GDMM_PRINTF("Error! psGetMediaThreadInfo Failed!\n"));

    char *pVruName = getenv("vru_name");
    char *pPodName = getenv("POD_NAME");
    GDMM_UPF_VALID_CHECK_ACTION((NULL == pVruName || NULL == pPodName), _DYN_GDMM_PRINTF("Error! get VruName or PodName Fail!\n"));

    WORD32 i = 0;
    zte_memset_s(&g_tGdmmShowNcuPodCpuUsageAck, sizeof(T_psGdmmShowNcuPodCpuUsageAck), 0, sizeof(T_psGdmmShowNcuPodCpuUsageAck));
    BYTE bMaxInstNum = MIN(tThreadPara.bInstNum, MAX_SHOWNCUPODCPUUSAGEACK_VCPUINFO_NUM);
    for (i = 0; i < bMaxInstNum && i < MAX_PS_THREAD_INST_NUM; i++)
    {
        psNcuGdmmFillCpuUsageInfo(&tThreadPara.atThreadParaList[i], pVruName, pPodName);
    }
    _DYN_GDMM_PRINTF("After collect! vcpuinfoNum:%d!\n", g_tGdmmShowNcuPodCpuUsageAck.vcpuinfoNum);

    JID recvJid;
    JID sendJid;
    XOS_STATUS xosRet = XOS_GetSelfJID(&sendJid);
    GDMM_NCU_VALID_CHK(XOS_SUCCESS != xosRet);
    xosRet = XOS_Sender((VOID *)&recvJid);
    GDMM_NCU_VALID_CHK(XOS_SUCCESS != xosRet);

    xosRet = XOS_SendAsynMsg(EV_UPF_GDMM_TO_MDMM_SHOW_NCU_POD_CPU_USAGE_ACK, (BYTE *)&g_tGdmmShowNcuPodCpuUsageAck, sizeof(g_tGdmmShowNcuPodCpuUsageAck), XOS_MSG_VER0, &recvJid, &sendJid);
    _DYN_GDMM_PRINTF("Send EV_UPF_GDMM_TO_MDMM_SHOW_NCU_POD_CPU_USAGE_ACK Rst! xosRet = %d !!\n", xosRet);
    return;
}

void psNcuGdmmFillCpuUsageInfo(T_PSThreadCrtPara *ptThreadParaList, const char *pVruName, const char *pPodName)
{
    GDMM_NCU_NULL_CHK(pVruName);
    GDMM_NCU_NULL_CHK(pPodName);
    GDMM_NCU_NULL_CHK(ptThreadParaList);

    _DYN_GDMM_PRINTF("get dwScanVcpuNo:%d !\n", ptThreadParaList->bvCPUNo);
    WORD32 dwShareCpuNum = psGetShareCpuNum();
    _DYN_GDMM_PRINTF("get dwShareCpuNum:%d !\n", dwShareCpuNum);

    if (TRUE == ptThreadParaList->bNeedSleep)
    {
        WORD32 i = 0;
        for (i = 0; i < dwShareCpuNum && i < MAX_PS_THREAD_INST_NUM; i++)
        {
            WORD32 dwShareVcpuNo = psGetShareCpuByIndex(i);
            _DYN_GDMM_PRINTF("get ShareVcpuNo:%d !\n", dwShareVcpuNo);
            psNcuGdmmFillCpuUsageDetailInfo(pVruName, pPodName, ptThreadParaList->bvCPUIndex, TRUE, dwShareVcpuNo);
        }
        return;
    }

    psNcuGdmmFillCpuUsageDetailInfo(pVruName, pPodName, ptThreadParaList->bvCPUIndex, FALSE, ptThreadParaList->bvCPUIndex);
    return;
}

void psNcuGdmmFillCpuUsageDetailInfo(const char *pVruName, const char *pPodName, WORD16 wCpuNo, BOOLEAN bShareFlg, WORD16 wCpuUsageNo)
{
    T_ShowNcuPodCpuUsageAckResultinfo *ptShowNcuPodCpuUsage = &(g_tGdmmShowNcuPodCpuUsageAck.vcpuinfo[g_tGdmmShowNcuPodCpuUsageAck.vcpuinfoNum]);
    char *pCpuAttribute_Share = "shared_cpu";
    char *pCpuAttribute_exclusive = "exclusive_cpu";
    BYTE CpuUsage = 0;

    zte_strncpy_s(ptShowNcuPodCpuUsage->vruname, MAX_SHOWNCUPODCPUUSAGEACK_VRUNAME_LEN, pVruName, MAX_SHOWNCUPODCPUUSAGEACK_VRUNAME_LEN - 1);
    zte_strncpy_s(ptShowNcuPodCpuUsage->podname, MAX_SHOWNCUPODCPUUSAGEACK_PODNAME_LEN, pPodName, MAX_SHOWNCUPODCPUUSAGEACK_PODNAME_LEN - 1);
    ptShowNcuPodCpuUsage->cpuno = wCpuNo;
    if (bShareFlg)
    {
        CpuUsage = XOS_GetSingleCpuRate(wCpuUsageNo);
        zte_strncpy_s(ptShowNcuPodCpuUsage->cpuattribute, MAX_SHOWNCUPODCPUUSAGEACK_CPUATTRIBUTE_LEN, pCpuAttribute_Share, MAX_SHOWNCUPODCPUUSAGEACK_CPUATTRIBUTE_LEN - 1);
    }
    else
    {
        CpuUsage = (BYTE)pdaGetcpuload(wCpuUsageNo);
        zte_strncpy_s(ptShowNcuPodCpuUsage->cpuattribute, MAX_SHOWNCUPODCPUUSAGEACK_CPUATTRIBUTE_LEN, pCpuAttribute_exclusive, MAX_SHOWNCUPODCPUUSAGEACK_CPUATTRIBUTE_LEN - 1);
    }
    zte_snprintf_s(ptShowNcuPodCpuUsage->cpuusage, MAX_SHOWNCUPODCPUUSAGEACK_CPUUSAGE_LEN, "%d%%", CpuUsage);
    g_tGdmmShowNcuPodCpuUsageAck.vcpuinfoNum++;
    return;
}
/* Ended by AICoder Product */
/* Ended by AICoder, pid:u95fem5aa2mda4314ca60bba3050ed99e8d7d8b0 */
