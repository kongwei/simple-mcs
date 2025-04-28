#include "psDbMcsDynTables.h"
#include "xdb_pfu_com.h"
#include "ps_ncu_session.h"
#include "ps_ncu_subscribe.h"
#include "ps_ncu_stream.h"
#include "ps_ncu_dataApp.h"
#include "r_ncu_sn.h"
#include "ps_ncu_httplink.h"

BOOLEAN create_ncu_mcsthread_dyn_tables(WORD32 dwDbHandle)
{
    _DB_STATEMENT_FALSE_RTN_FALSE(create_r_ncu_session(dwDbHandle),LOG_EMERGENCY);
    _DB_STATEMENT_FALSE_RTN_FALSE(create_r_ncu_subscribe(dwDbHandle),LOG_EMERGENCY);
    _DB_STATEMENT_FALSE_RTN_FALSE(create_r_flowctx(dwDbHandle), LOG_EMERGENCY);
    _DB_STATEMENT_FALSE_RTN_FALSE(create_r_ncu_dataApp(dwDbHandle), LOG_EMERGENCY);
    _DB_STATEMENT_FALSE_RTN_FALSE(create_r_ncu_sn(dwDbHandle), LOG_EMERGENCY);
    _DB_STATEMENT_FALSE_RTN_FALSE(create_r_ncu_httplink(dwDbHandle), LOG_EMERGENCY);
    return TRUE;
}
