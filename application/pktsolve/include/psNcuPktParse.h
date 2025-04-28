#ifndef PS_NCU_PKT_PARSE_H_
#define PS_NCU_PKT_PARSE_H_

#include "psUpfCommon.h"
#include "xdb_pfu_dyntbl_acc.h"
WORD32 psNcuParsePktInfo(T_MediaProcThreadPara* ptMediaProcThreadPara, VOID *ptPktData);
BOOLEAN psNcuIsPktHasSynAckFlg(T_psNcuPktDesc *ptPktDesc);
#endif