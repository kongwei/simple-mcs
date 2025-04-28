#ifndef PS_MCS_DEFINE_H_
#define PS_MCS_DEFINE_H_

#include "tulip.h"
#include "ps_pub.h"
#include "psUpfCommon.h"
#include "dbm_lib_upfcomm.h"
#include "dbmLibComm.h"
#include "psMcsGlobalCfg.h"
/*MCS处理返回值*/
#define MCS_RET_SUCCESS         (WORD32)(0) /*成功*/
#define MCS_RET_CONTINUE        (WORD32)(1) /*继续当前流程*/
#define MCS_RET_BREAK           (WORD32)(2) /*中断当前流程*/
#define MCS_RET_EXIST           (WORD32)(3)
#define MCS_RET_FAIL            (WORD32)(0xffffffff) /*失败*/

#define MCS_PKT_DIR_UL           (0)
#define MCS_PKT_DIR_DL           (1)

#define MCS_NEED_FREEPKT         (1)
#define MCS_NONEED_FREEPKT       (0)



#ifndef _mcs_if
#define _mcs_if(a) if(a)
#endif

#ifndef _else_if
#define _else_if(a)  else if(a)
#endif

#ifndef _else
#define _else else
#endif

#ifndef _mcs_else_if
#define _mcs_else_if(a)  else if(a)
#endif

#ifndef _mcs_else
#define _mcs_else  else
#endif

#ifndef _mcs_while
#define _mcs_while(a)  while(a)
#endif
// typedef struct tagT_MediaProcThreadPara


#ifdef __cplusplus
extern "C" {
#endif
extern __thread T_MediaProcThreadPara* g_ptMediaProcThreadPara;
#ifdef __cplusplus
}
#endif
/* 扫描消息号 */
enum
{
    NCU_NOTIFY_SESSION_RELEASE,
    NCU_NOTIFY_FLOW_RELEASE,
    NCU_NOTIFY_SN_RELEASE,
    NCU_NOTIFY_APPID_RELATE_RELATE,
    NCU_NOTIFY_TOTAL,                    /* 新增消息要在前面 */

};

typedef struct
{
    WORD32  dwMessageType;
    WORD16  wMessageLen;
    BYTE    aucResv[2];
}T_psNcuMediaScanMsg;

/**************************************************************************
 *                              接口(对外提供，最小可见)
 **************************************************************************/
static INLINE WORD32 getNcuSoftPara(WORD32 id)
{
    if (id < PFU_SOFTPARA_ID_START)
    {
        return 0;
    }

    if (id - PFU_SOFTPARA_ID_START >= PFU_SOFTPARA_MAX)
    {
        return 0;
    }
    return g_upfConfig.PfuSoftPara[id - PFU_SOFTPARA_ID_START];
}

#endif
