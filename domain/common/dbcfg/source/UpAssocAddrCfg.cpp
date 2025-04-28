/* Started by AICoder, pid:z01c3n389894e011418b09d5509ed84da6e54118 */
#include "UpAssocAddrCfg.h"
#include "psMcsGlobalCfg.h"
#include "dbm_lib_upfcomm.h"
#include "dbmLibComm.h"
#define USE_DM_ALL
#include "r_upassocaddr.h"
#include "dbm_lib_upf.h"
#include "zte_slibc.h"
#include "xdb_pfu_com.h"


void UpAssocAddrCfg::powerOnProc()
{
    BOOL bflag = FALSE;
    DM_UPF_GETR_UPASSOCADDR_REQ req = {0};
    DM_UPF_GETR_UPASSOCADDR_ACK ack = {0};

    req.msgType = MSG_CALL;
    req.dwgindex = 1;
    ack.retCODE = RC_DBM_ERROR;
    bflag = DBM_CALL(DM_UPF_GETR_UPASSOCADDR, (LPSTR)&req, (LPSTR)&ack);
    if(RC_OK != ack.retCODE  ||  TRUE != bflag)
    {
        zte_strncpy_s(g_upfConfig.daUpfIpCfg.upfName,         LEN_R_UPASSOCADDR_UPNAME_MAX+1,        "NCU_UPF",        LEN_R_UPASSOCADDR_UPNAME_MAX);
    }
    else
    {
        zte_strncpy_s(g_upfConfig.daUpfIpCfg.upfName,         LEN_R_UPASSOCADDR_UPNAME_MAX+1,       ack.upname,        LEN_R_UPASSOCADDR_UPNAME_MAX);
    }
    zte_printf_s("\n[UpAssocAddrCfg]recv UpAssocAddrCfg upfname is %s!\n", g_upfConfig.daUpfIpCfg.upfName);
    return;
}

void UpAssocAddrCfg::cfgNotifyProc(WORD16 operation, WORD16 msgLen, BYTE *msg)
{
    _DB_STATEMENT_TRUE_RTN_NONE(NULL == msg);
    zte_printf_s("\n[UpAssocAddrCfg]recv UpAssocAddrCfg msg is not NULL!\n");
    
    R_UPASSOCADDR_TUPLE *ptupleNew = NULL;
    switch (operation)
    {
        case _DB_CFGCHG_TUPLE_NOTIFY_INSERT:
        {
            zte_printf_s("\n[UpAssocAddrCfg]recv insert UPAssocAddr msg!\n");
            _DB_STATEMENT_TRUE_RTN_NONE(sizeof(R_UPASSOCADDR_TUPLE) > msgLen);
            ptupleNew = (R_UPASSOCADDR_TUPLE*)msg;
            break;
        }
        case _DB_CFGCHG_TUPLE_NOTIFY_MODIFY:
        {
            zte_printf_s("\n[UpAssocAddrCfg]recv modify UPAssocAddr msg!\n");
            _DB_STATEMENT_TRUE_RTN_NONE(2 * sizeof(R_UPASSOCADDR_TUPLE) > msgLen);
            ptupleNew = (R_UPASSOCADDR_TUPLE*)(msg + sizeof(R_UPASSOCADDR_TUPLE));
            break;
        }
        case _DB_CFGCHG_TUPLE_NOTIFY_DELETE:
        {
            zte_printf_s("\n[UpAssocAddrCfg]recv delete UPAssocAddr msg!\n");
            break;
        }
        default:
        {
            break;
        }
    }
    if(NULL != ptupleNew)
    {
        zte_strncpy_s(g_upfConfig.daUpfIpCfg.upfName,         LEN_R_UPASSOCADDR_UPNAME_MAX+1,       ptupleNew->upname,        LEN_R_UPASSOCADDR_UPNAME_MAX);
        zte_printf_s("\n[UpAssocAddrCfg]recv UpAssocAddrCfg upfname is %s!\n", g_upfConfig.daUpfIpCfg.upfName);
    }

    return;
}

void UpAssocAddrCfg::show()
{
    zte_printf_s("\n[UpAssocAddrCfg]recv UpAssocAddrCfg upfname is %s!\n", g_upfConfig.daUpfIpCfg.upfName);
}
/* Ended by AICoder, pid:z01c3n389894e011418b09d5509ed84da6e54118 */
