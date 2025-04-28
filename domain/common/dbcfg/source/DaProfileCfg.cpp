#include "psMcsGlobalCfg.h"
#include "DaProfileCfg.h"
#define USE_DM_ALL
#include "dbm_lib_upfncu.h"
#include "dbmLibComm.h"
#include "zte_slibc.h"
#include "r_daprofile.h"
#include "psNcuDAProfileCtxProc.h"
void DaProfileCfg::powerOnProc()
{
    return;
}

void DaProfileCfg::show()
{
    return;
}


void DaProfileCfg::cfgNotifyProc(WORD16 wOperation, WORD16 wDataMsgLen, BYTE *lpData)
{
    CfgChgDAProfileProc(wOperation, wDataMsgLen, lpData);
}
