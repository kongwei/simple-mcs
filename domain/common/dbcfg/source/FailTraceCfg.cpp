/* Started by AICoder, pid:kb37az7438ze46f149aa083cc0159a6b5d908c0e */ 
/******************************************************************************
* 版权所有 (C)2016 深圳市中兴通讯股份有限公司*
* 模块名          : Ncu
* 文件名          : FailTraceCfg.cpp
* 相关文件        :
* 文件实现功能     : 失败观察配置
* 归属团队        : M6
* 版本           : V1.0
-------------------------------------------------------------------------------
* 修改记录:
* 修改日期            版本号           修改人           修改内容
* 2024-07-16        V7.24.30        zjw            create
******************************************************************************/

#include "FailTraceCfg.h"
#define USE_DM_UPF_GETR_FAILTRACECFG
#include "dbm_lib_upf.h"
#include "dbmLibComm.h"
#include "zte_slibc.h"

T_UpfFailTraceCfg g_failtracecfg = {0};

/* Started by AICoder, pid:scb121da722fac514d9c0bcbb02ec68e57293adb */ 
void FailTraceCfg::powerOnProc()
{
    BOOLEAN bflag = FALSE;
    DM_UPF_GETR_FAILTRACECFG_REQ req = {0};
    DM_UPF_GETR_FAILTRACECFG_ACK ack = {0};

    req.msgType = MSG_CALL;
    req.dwgindex= 1;
    ack.retCODE = RC_DBM_ERROR;

    bflag = DBM_CALL(DM_UPF_GETR_FAILTRACECFG, (LPSTR)&req, (LPSTR)&ack);
    if( RC_DBM_OK != ack.retCODE || (TRUE != bflag))
    {
        g_failtracecfg.FailRecoveryThreshold          = 55;
        g_failtracecfg.FailStopThreshold              = 80;
        g_failtracecfg.FailStopOnOverLoad             = 1;
        g_failtracecfg.FailOverTimeThreshold_Hours    = 0;
        g_failtracecfg.MaxReportPPS                   = 2000;
    }
    else
    {
        g_failtracecfg.FailRecoveryThreshold           = ack.recoverythreshold;
        g_failtracecfg.FailStopThreshold               = ack.stopthreshold;
        g_failtracecfg.FailStopOnOverLoad              = ack.stoponoverload;
        g_failtracecfg.FailOverTimeThreshold_Hours     = ack.overtimethreshold;
        g_failtracecfg.MaxReportPPS                    = ack.maxreportpps;
    }
}

void FailTraceCfg::cfgNotifyProc(WORD16 operation, WORD16 msgLen, BYTE *msg)
{
    if(NULL == msg)
    {
        return;
    }

    R_FAILTRACECFG_TUPLE *ptupleNew = NULL;
    switch(operation)
    {
        case _DB_CFGCHG_TUPLE_NOTIFY_INSERT:
        {
            if(sizeof(R_FAILTRACECFG_TUPLE) > msgLen) 
            {
                return ;
            }
            ptupleNew = (R_FAILTRACECFG_TUPLE*)msg;
            break;
        }
        case _DB_CFGCHG_TUPLE_NOTIFY_MODIFY:
        {
            if(2*sizeof(R_FAILTRACECFG_TUPLE) > msgLen)
            {
                return ;
            }
            ptupleNew = (R_FAILTRACECFG_TUPLE*)(msg + sizeof(R_FAILTRACECFG_TUPLE));
            break;
        }
        default:
        {
            break;
        }
    }

    if( NULL != ptupleNew )
    {
        g_failtracecfg.FailRecoveryThreshold         = ptupleNew->recoverythreshold;
        g_failtracecfg.FailStopThreshold             = ptupleNew->stopthreshold;
        g_failtracecfg.FailStopOnOverLoad            = ptupleNew->stoponoverload;
        g_failtracecfg.FailOverTimeThreshold_Hours   = ptupleNew->overtimethreshold;
        g_failtracecfg.MaxReportPPS                  = ptupleNew->maxreportpps;
    }
}

/* Started by AICoder, pid:64bbbqbfb5e0b86146e108d790ecab051be8e6d1 */
void FailTraceCfg::show()
{
    zte_printf_s("\n FailRecoveryThreshold         = %u", g_failtracecfg.FailRecoveryThreshold);
    zte_printf_s("\n FailStopThreshold             = %u", g_failtracecfg.FailStopThreshold);
    zte_printf_s("\n FailStopOnOverLoad            = %u", g_failtracecfg.FailStopOnOverLoad);
    zte_printf_s("\n FailOverTimeThreshold_Hours   = %u", g_failtracecfg.FailOverTimeThreshold_Hours);
    zte_printf_s("\n MaxReportPPS                 = %u", g_failtracecfg.MaxReportPPS);
}
/* Ended by AICoder, pid:64bbbqbfb5e0b86146e108d790ecab051be8e6d1 */
/* Ended by AICoder, pid:scb121da722fac514d9c0bcbb02ec68e57293adb */ 
/* Ended by AICoder, pid:kb37az7438ze46f149aa083cc0159a6b5d908c0e */ 
