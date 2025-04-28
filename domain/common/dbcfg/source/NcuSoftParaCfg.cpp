#include "NcuSoftParaCfg.h"
#include "psMcsGlobalCfg.h"
#define USE_DM_ALL
#include "dbm_lib_upfcomm.h"
#include "dbmLibComm.h"
#include "zte_slibc.h"

#ifndef UPF_GETALL_MAX
#define UPF_GETALL_MAX 20
#endif
void NcuSoftParaCfg::powerOnProc()
{
    pfuInitSoftPara();
    pfuGetSoftPara();

}

void NcuSoftParaCfg::cfgNotifyProc(WORD16 operation, WORD16 msgLen, BYTE *msg)
{
    if (NULL == msg)
    {
        return;
    }
    
    DPIINNERSOFTPARA_TUPLE *ptupleNew = NULL;
    DPIINNERSOFTPARA_TUPLE *ptupleOld = NULL;
    
    switch (operation)
    {
        case _DB_CFGCHG_TUPLE_NOTIFY_INSERT:
        {
            if (sizeof(DPIINNERSOFTPARA_TUPLE) > msgLen)
            {
                return ;
            }
            ptupleNew = (DPIINNERSOFTPARA_TUPLE *)msg;
            break;
        }
        case _DB_CFGCHG_TUPLE_NOTIFY_MODIFY:
        {
            if (2*sizeof(DPIINNERSOFTPARA_TUPLE) > msgLen)
            {
                return ;
            }
            ptupleNew = (DPIINNERSOFTPARA_TUPLE *)(msg + sizeof(DPIINNERSOFTPARA_TUPLE));
            break;
        }
        case _DB_CFGCHG_TUPLE_NOTIFY_DELETE:
        {
            if (sizeof(DPIINNERSOFTPARA_TUPLE) > msgLen)
            {
                return ;
            }
            ptupleOld = (DPIINNERSOFTPARA_TUPLE *)msg;
            setConfig(ptupleOld->softparaid, 0);
            break;
        }
        default:
        {
            break;
        }
    }
    
    if (NULL != ptupleNew)
    {
        setConfig(ptupleNew->softparaid, getVal(*ptupleNew));
    }
}


void NcuSoftParaCfg::show()
{
    WORD32      i;

    for (i = 0; i < PFU_SOFTPARA_MAX; i ++)
    {
        if (g_upfConfig.PfuSoftPara[i] != 0)
        {
            zte_printf_s("SoftID:%4u, Val:%u\n", PFU_SOFTPARA_ID_START+i, g_upfConfig.PfuSoftPara[i]);
        }
    }
}

WORD32 NcuSoftParaCfg::getVal(DPIINNERSOFTPARA_TUPLE &tuple)
{
    WORD32  setVal = tuple.value;
    
    if (tuple.value > tuple.maxvalue
        || tuple.value < tuple.minvalue)
    {
        setVal  = tuple.defaultvalue;
    }

    return setVal;
}

WORD32 NcuSoftParaCfg::getVal(DM_UPFCOMM_GETALLDPIINNERSOFTPARA_ACK &ack, WORD32 i)
{
    if (i >= UPF_GETALL_MAX)
    {
        return 0;
    }

    WORD32  setVal = ack.value[i];
            
    if (ack.value[i] > ack.maxvalue[i]
        || ack.value[i] < ack.minvalue[i])
    {
        setVal  = ack.defaultvalue[i];
    }
    
    return setVal;
}

void NcuSoftParaCfg::setConfig(WORD32 id, WORD32 val)
{
    if (id < PFU_SOFTPARA_ID_START)
    {
        return;
    }
    
    if (id - PFU_SOFTPARA_ID_START >= PFU_SOFTPARA_MAX)
    {
        return;
    }
    
    g_upfConfig.PfuSoftPara[id - PFU_SOFTPARA_ID_START] = val;
}

void NcuSoftParaCfg::pfuInitSoftPara(void)
{
    zte_memset_s(g_upfConfig.PfuSoftPara, sizeof(g_upfConfig.PfuSoftPara), 0, sizeof(g_upfConfig.PfuSoftPara));
    /* 非0初始化 */
    
}

void NcuSoftParaCfg::pfuGetSoftPara(void)
{
    DM_UPFCOMM_GETALLDPIINNERSOFTPARA_REQ   req = {0};
    DM_UPFCOMM_GETALLDPIINNERSOFTPARA_ACK   ack = {0};
    WORD32          count = 0;

    do
    {
        req.msgType     = MSG_CALL;
        req.dwTupleNo   = ack.dwTupleNo;
        ack.retCODE     = RC_DBM_ERROR;
        
        BOOL    bRet = DBM_CALL(DM_UPFCOMM_GETALLDPIINNERSOFTPARA, (LPSTR)&req, (LPSTR)&ack);
        if ((RC_OK != ack.retCODE) || !bRet)
        {
            return;
        }

        WORD32      i;
        for (i = 0; (i < ack.dwValidNum) && (i < UPF_GETALL_MAX); i++)
        {
            setConfig(ack.softparaid[i], getVal(ack, i));
        }

        if (count++ > 10000)
        {
            return;
        }

    } while(!ack.blEnd);
}

