/******************************************************************************
 * 版权所有 (C)2016, 深圳市中兴通讯股份有限公司
 * 模块名          : MCS
 * 文件名          : psNcuHeartBeatProc.h
 * 相关文件        :
 * 文件实现功能     : Nwdaf链路心跳检测处理流程
 * 归属团队        : 
 * 版本           : V7.24.20
 -------------------------------------------------------------------------------
 * 修改记录:
 * 修改日期           版本号           修改人           修改内容
 * 2024-04-28        V7.24.20                  create
 ******************************************************************************/
#ifndef _PS_NCU_HEART_BEAT_PROC_H_
#define _PS_NCU_HEART_BEAT_PROC_H_
#ifdef __cplusplus
extern "C" {
#endif
/**************************************************************************
 *                              头文件(满足自包含，满足最小依赖)
 **************************************************************************/
#include "ps_mcs_define.h"
#include "psNcuReportStructData.h"

/**************************************************************************
 *                              宏(对外提供)
 **************************************************************************/
#define NWDAF_LINKNUM_MAX          32
#define NWDAF_LINKSTATE_DOWN       0
#define NWDAF_LINKSTATE_UP         1


#define IETYPE_Recovery_Time_Stamp  13

#define PS_MAX_INNERMSG_LEN     10240


#ifndef INVALID_COMM_ID
#define INVALID_COMM_ID   0xFFFFFFFF
#endif
#ifndef INVALID_LOGIC_NO
#define INVALID_LOGIC_NO  0
#endif



#ifndef PS_NCU_NULL_CHECK_VOID
#define PS_NCU_NULL_CHECK_VOID(x)\
    if(unlikely(NULL == (x)))\
    {\
        return;\
    }
#endif

#ifndef PS_NCU_NULL_CHECK_RET
#define PS_NCU_NULL_CHECK_RET(x,y)\
    if(unlikely(NULL == (x)))\
    {\
        return y;\
    }
#endif

/**************************************************************************
 *                              数据类型(对外提供)
 **************************************************************************/

#pragma pack(1)
typedef struct
{
    T_IE_Base   Base;
    WORD32      dwTimeStamp;
} T_RecovertTimeStampId;
#pragma pack()

typedef struct tagT_psNcuHeartHead
{
    T_IPComm tNcuAddress;
    T_IPComm tNwdafAddress;
    WORD16 wNcuPort;
    WORD16 wNwdafPort;
    WORD32 dwRsv;
}T_psNcuHeartHead;

/**************************************************************************
 *                              接口(对外提供，最小可见)
 **************************************************************************/

/**************************************************************************
 *                              全局变量声明(对外提供，最小可见)
 **************************************************************************/
VOID psNcuSendNwdafIplistSyntoNcu();
VOID psNcuUpdateNwdafIplistProc(BYTE * pMsgBodySrc);
WORD32 psGetNcuUpfIp(T_NcuUpfIpInfo *ptNcuUpfIpInfo);
VOID psNcuAllNwdafLinkSynToUpm(BYTE         bSendUpmType);
WORD32 psGetUpmSubNwDafIplistProc(BYTE *pMsgBodySrc,  T_NwdafStatus *ptNwdaflinkOldStatus, BYTE *bGetNcuUpfIpFail);
VOID psNcuSendUpmUpdateLinkStatus(T_NcuToUpm_NwdafLinkInfoAll *ptNcuToUpmNwdafLinkInfo);
WORD32 psNcuSendHeartBeatReqProc();
VOID psNcuRecSubToHeartBeatProc(BYTE * pMsgBodySrc);
WORD32 psNcuIpUdpDecap(T_MediaProcThreadPara *ptMediaProcThreadPara, T_psNcuHeartHead *ptNcuHeartHead, WORD16 *pwBufferOffset);
void psNcuEncodeHeartBeatData(BYTE* buffer, T_RecovertTimeStampId *ptRecovertTimeStampId, WORD16* ptDataLen);
WORD16 psEncodeHeartBeatUDPDataProc(BYTE* buffer);
VOID psNcuRcvNwdafPktProc(BYTE *pMsgBodySrc, WORD32 dwMsgLen);
void psNcuHeartBeatProc(void* ptPktDataList, WORD16 wLength, T_MediaProcThreadPara *ptMediaProcThreadPara);
VOID psNcuHeartPktProc(T_MediaProcThreadPara *ptMediaProcThreadPara);
void psNcuHeartRsqProc(BYTE* msg, WORD32 msgLen);
VOID psNcuRcvNwdafRspPktProc(BYTE *pMsgBodySrc);
void psNcuUcomSendHeartBeatCheckReq(T_MediaProcThreadPara *ptMediaProcThreadPara, T_psNcuHeartHead *ptNcuHeartHead);
WORD32 psGetNcuNwdafIPlistIpIsAllNull();
WORD32 psGetUpmNwdafIPlistIpIsAllNull(BYTE *pMsgBodySrc);
WORD32 psUpmNwdafIPlistIpIsNotEqualNcuNwdafIP(BYTE *pMsgBodySrc);

VOID PsNcuSystemLogTest();  /* Logtest */
WORD32 psFindNwdafIpListAdrr(T_IPComm tNwdafAddress);
void psNcuHeartRspToJob(T_MediaProcThreadPara *ptMediaProcThreadPara);

WORD16 psEncodeHeartReqDataProc(BYTE* buffer);
WORD16 psEncodeHeartRspDataProc(BYTE* buffer);
#ifdef __cplusplus
}
#endif
#endif
