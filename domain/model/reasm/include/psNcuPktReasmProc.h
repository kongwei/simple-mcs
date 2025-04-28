/* Started by AICoder, pid:o119fkddf7y7b841484d08ebe0453a4808339c03 */
/******************************************************************************
 * 版权所有 (C)2016, 深圳市中兴通讯股份有限公司
 * 模块名          : NCU
 * 文件名          : psNcuPktReasmProc.h
 * 相关文件        :
 * 文件实现功能     : NCU报文重组流程
 * 归属团队        : M6
 * 版本           : V1.0
 -------------------------------------------------------------------------------
 * 修改记录:
 * 修改日期           版本号           修改人           修改内容
 * 2024-08-27        V7.24.30        zjw            create
 ******************************************************************************/
#ifndef _PS_NCU_PSNCPKTREASMPROC_H_
#define _PS_NCU_PSNCPKTREASMPROC_H_
#ifdef __cplusplus
extern "C" {
#endif
/**************************************************************************
 *                              头文件(满足自包含，满足最小依赖)
 **************************************************************************/
#include "ps_packet.h"
#include "ps_mcs_define.h"

/**************************************************************************
 *                              宏(对外提供)
 **************************************************************************/
#define PS_RECORD_IPOFFSET_AFTER_REASM(ptPacket) do{ \
    if(NULL == ptPacket) \
    { \
        break; \
    } \
    BYTE *ptBuffer = NULL; \
    WORD16  wOffset = 0; \
    WORD16  wBufLen = 0; \
    PS_GET_BUF_INFO(ptPacket->pBufferNode, ptBuffer, wOffset, wBufLen); \
    T_psvPFUPktInfo *ptPktDataArea = (T_psvPFUPktInfo *)ptPacket->bDataArea; \
    ptPktDataArea->wIPOffetWhenReasm = wOffset; \
}while(0)

/**************************************************************************
 *                              数据类型(对外提供)
 **************************************************************************/

/**************************************************************************
 *                              接口(对外提供，最小可见)
 **************************************************************************/
BOOLEAN psNcuPktFragReasmEntry(PS_PACKET *ptPacket, T_MediaProcThreadPara *ptMediaProcThreadPara);

/**************************************************************************
 *                              全局变量声明(对外提供，最小可见)
 **************************************************************************/

#ifdef __cplusplus
}
#endif
#endif
// end of file
/* Ended by AICoder, pid:o119fkddf7y7b841484d08ebe0453a4808339c03 */
