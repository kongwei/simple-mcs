#include "SigTraceTaskCfg.h"
#include "ps_p4.h"
#include "UPFLog.h"
#include "psMcsGlobalCfg.h"
#include "ps_ncu_typedef.h"
#include "Task.h"
#include "r_sigtracetask.h"
#define USE_DM_UPFNCU_GETALLR_SIGTRACETASK
#define USE_DM_UPFNCU_GETR_SIGTRACETASK

#include "dbm_lib_upfncu.h"
#include "dbm_lib_upf.h"
#include "dbmLibComm.h"
#ifdef __cplusplus  
extern "C" {
#endif
    extern void Trace_AddTask(const R_SIGTRACETASK_TUPLE *pTask);
    extern void Trace_RmTask(WORD32 TraceID);
#ifdef __cplusplus  
}
#endif

void SigTraceTaskCfg::powerOnProc()
{
    WORD32 tuplenum = 0; 
    BOOLEAN bflag = FALSE; 
    DM_UPFNCU_GETALLR_SIGTRACETASK_REQ req = {0}; 
    DM_UPFNCU_GETALLR_SIGTRACETASK_ACK ack = {0}; 

    UPF_TRACE_INFO( "[getSigTraceTaskCfg] call DM_UPFNCU_GETALLR_SIGTRACETASK !\n" ); 

    do
    {
         req.msgType = MSG_CALL;
         req.dwTupleNo = ack.dwTupleNo;
         bflag = FALSE;
         ack.retCODE = RC_FAIL;
         bflag = DBM_CALL(DM_UPFNCU_GETALLR_SIGTRACETASK, (LPSTR)&req, (LPSTR)&ack);

         if((RC_OK != ack.retCODE) || (TRUE != bflag))
         {
             UPF_TRACE_INFO("\n pfu getSigTraceTaskCfg failed! \n ");
             return;
         }

         /* 此表比较大     ，一次自能获取UPF_LONG_TUPLE_MAX条记录 */
         if(0 != ack.dwValidNum)
         {
             R_SIGTRACETASK_TUPLE    tuple = {0};
             tuple.traceid   = ack.traceid[0];
             tuple.tasktype  = ack.tasktype[0];
             zte_memcpy_s(tuple.tcsaddr, sizeof(tuple.tcsaddr),ack.tcsaddr[0], sizeof(tuple.tcsaddr));
             tuple.modifytime= ack.modifytime[0];


             UPF_TRACE_INFO("\n===>Trace filter:(%s)\n", ack.filter[0]);

             zte_memcpy_s(tuple.filter, sizeof(tuple.filter),ack.filter[0], sizeof(tuple.filter));

             Trace_AddTask(&tuple);
             tuplenum++;

         }

         MCS_CHK_BREAK(tuplenum>=4096);
    }while( !ack.blEnd );

    UPF_TRACE_INFO( "[getSigTraceTaskCfg] call DM_UPF_GETALLR_SIGTRACETASK succ!\n" );

}

void SigTraceTaskCfg::cfgNotifyProc(WORD16 operation, WORD16 msgLen, BYTE *msg)
{
    if(msg == NULL)
    {
      return;
    }
    
    R_SIGTRACETASK_LONGNOTIFYCHG *notify = (R_SIGTRACETASK_LONGNOTIFYCHG *)msg;
    
    switch (operation)
    {
        case _DB_CFGCHG_TUPLE_NOTIFY_INSERT:
        case _DB_CFGCHG_TUPLE_NOTIFY_MODIFY:
        {
            UPF_TRACE_INFO("sigtrace tuple notify insert, traceid:%d\n", notify->traceid);
            if(sizeof(R_SIGTRACETASK_LONGNOTIFYCHG) > msgLen)
            {
                UPF_TRACE_INFO("\n[ConfigNotify]recv wrong notify msg!\n");
                return;
            }
            DM_UPFNCU_GETR_SIGTRACETASK_REQ req = {0};
            DM_UPFNCU_GETR_SIGTRACETASK_ACK ack = {0};

            req.msgType = MSG_CALL;
            req.traceid = notify->traceid;
            ack.retCODE = RC_DBM_ERROR;
            BOOL    bflag = DBM_CALL(DM_UPFNCU_GETR_SIGTRACETASK, (LPSTR)&req, (LPSTR)&ack);

            if (!bflag || RC_DBM_OK != ack.retCODE)
            {
                UPF_TRACE_ERROR("DBM_CALL DM_UPFNCU_GETR_SIGTRACETASK fail! ret:%d retCODE:%d\n", bflag, ack.retCODE);
                return;
            }
            UPF_TRACE_INFO("\n===>Trace filter:(%s)\n", ack.filter);
            R_SIGTRACETASK_TUPLE    tuple = {0};

            zte_memcpy_s(tuple.filter,sizeof(tuple.filter), ack.filter, sizeof(tuple.filter));


            tuple.traceid   = notify->traceid;
            tuple.tasktype  = ack.tasktype;
            memcpy(tuple.tcsaddr, ack.tcsaddr, sizeof(tuple.tcsaddr));
            tuple.modifytime= ack.modifytime;

            UPF_TRACE_INFO("\n===>Trace tasktype:(%u) filter:(%s)\n", tuple.tasktype, tuple.filter);
            Trace_AddTask(&tuple);
            break;
        }
        case _DB_CFGCHG_TUPLE_NOTIFY_DELETE:
        {
            UPF_TRACE_INFO("sigtrace tuple notify delete, traceid:%d\n", notify->traceid);

            Trace_RmTask(notify->traceid);
            break;
        }
        default:
        {
            break;
        }
    }
}

void SigTraceTaskCfg::show()
{
    printf("\n SigTraceTaskCfg \n");
}


