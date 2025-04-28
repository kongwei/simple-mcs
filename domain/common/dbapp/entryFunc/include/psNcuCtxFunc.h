#ifndef _PS_NCU_CTX_FUNC_H_
#define _PS_NCU_CTX_FUNC_H_
#ifdef __cplusplus
extern "C" {
#endif
#include "tulip.h"

void  *psVpfuMcsGetCtxIdxById(WORD32 dwCtxId,WORD32 hDB,WORD32 hTbl);
void  *psVpfuMcsGetCtxById(WORD32 dwCtxId, WORD32 hDB, WORD32 hTbl);
WORD32 psVpfuMcsDelCtxById(WORD32 dwCtxId, WORD32 hDB, WORD32 hTbl);
DWORD psVpfuMcsGetCapacity(DWORD dwDataType, WORD32 hDB, WORD32 *dwTableCapacity,WORD32 *dwValidTupleNum);


#ifdef __cplusplus
}
#endif

#endif

