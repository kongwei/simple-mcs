#ifndef NCU_SCCUABILITY_H
#define NCU_SCCUABILITY_H

#include "tulip.h"


#ifdef __cplusplus
extern "C"{
#endif

WORD32 ncuSendMsgToUpm(BYTE *msg, WORD16 wBufferLen, WORD32 msgID, WORD16 wJobType);
WORD32 ncuGetGroupCommId(WORD32 groupId);
WORD32 ncuGetGroupSCLogicNo(WORD32 groupId);
WORD32 ncuGetUpmCommIdByLogicNo(void);
WORD32 ncuGetSelfCommId();
WORD32 ncuGetScLogicNo();
WORD32 ncuGetSelfLogicNo(void);
WORD32 pfuGetGroupCommId(WORD32 groupId);
WORD32 pfuGetGroupSCLogicNo(WORD32 groupId);
WORD32 ncuGetMasterScNum();
BOOL ncuIsSelfGroup(WORD32 groupId);
WORD32 ncuGetMaxScLogicNo();
WORD32 ncuGetCommIdByLogicNo(WORD32 LogicNo);
WORD32 ncuGetHttpLbCommIdByLogicNo(WORD32 dwLogicNo);

#ifdef __cplusplus
}
#endif


#endif
