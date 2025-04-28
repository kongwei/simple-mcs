#ifndef PS_NCU_DATA_ENCODE_H_
#define PS_NCU_DATA_ENCODE_H_
#include "ps_ncu_dataApp.h"

typedef struct
{
    CHAR Dnn[64];
    CHAR CorrelationId[256];
    BYTE isValid;
}ExpNormalCorrData;

extern ExpNormalCorrData g_CorrData[16];

#ifdef __cplusplus
extern "C" {
#endif
WORD16 psEncodeUDPDataProc(BYTE* buffer, T_psNcuDaAppCtx * ptDaAppCtx, BYTE bRptType, BYTE trigger);
BOOLEAN psCheckIPValid(BYTE* ip_addr, BYTE bIpType);
WORD32 setCorrelationIDByDnn(const CHAR* dnn, CHAR* correlation);
#ifdef __cplusplus
}
#endif
#endif
