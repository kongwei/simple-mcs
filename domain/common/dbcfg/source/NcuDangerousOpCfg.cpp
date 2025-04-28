#include "NcuDangerousOpCfg.h"
#include "thdm_pub.h"
#include "zte_slibc.h"
#include "UPFLog.h"
#define USE_DM_ALL
#include "dbm_lib_upfncu.h"
#include "dbm_lib_upf.h"
#include "dbm_lib_comm_nf_cfg.h"
#include "dbmLibComm.h"
#include "ncuDangerOpAlarm.h"

R_NC_CAPACITY_TUPLE        g_tR_CAPACITY_TUPLE[R_NC_CAPACITY_CAPACITY]        = {0};
R_NC_THREADSIZE_TUPLE      g_tR_THREADSIZE_TUPLE[R_NC_THREADSIZE_CAPACITY]    = {0};
R_NC_THREADINFO_TUPLE      g_tR_THREADINFO_TUPLE[R_NC_THREADINFO_CAPACITY]    = {0};
DPDKSPECIALNFCFG_TUPLE  g_tDPDKSPECIALNFCFG_TUPLE                       = {0};
DPDKCOMMNFCFG_TUPLE     g_tDPDKCOMMNFCFG_TUPLE                          = {0};

/* Started by AICoder, pid:m862c6531e07ab814ea40a19e0e4fb2148765258 */
void Capacity::powerOnProc()
{
    WORD i = 0;
    
    for(i=1;i<R_NC_CAPACITY_CAPACITY;i++)
    {
        BOOLEAN bflag = FALSE;
        DM_UPFNCU_GETR_NC_CAPACITY_REQ req = {0};
        DM_UPFNCU_GETR_NC_CAPACITY_ACK ack = {0};

        req.msgType = MSG_CALL;
        req.ctxname = i;
        req.vrusize = getVruSizeCap();
        ack.retCODE = RC_DBM_ERROR;

        bflag = DBM_CALL(DM_UPFNCU_GETR_NC_CAPACITY, (LPSTR)&req, (LPSTR)&ack);
        if( (RC_DBM_OK != ack.retCODE) || !bflag)
        {
            req.vrusize = (BYTE)VRUSIZECAP_ALL;
            bflag = DBM_CALL(DM_UPFNCU_GETR_NC_CAPACITY, (LPSTR)&req, (LPSTR)&ack);
        }
        
        if( (RC_DBM_OK == ack.retCODE)&&bflag)
        {
            g_tR_CAPACITY_TUPLE[i].ctxname     = i;
            g_tR_CAPACITY_TUPLE[i].vrusize     = req.vrusize;
            g_tR_CAPACITY_TUPLE[i].capacity    = ack.capacity;
            g_tR_CAPACITY_TUPLE[i].valuetype   = ack.valuetype;
            g_tR_CAPACITY_TUPLE[i].rate        = ack.rate;
        }
    }
    return;
}

void Capacity::cfgNotifyProc(WORD16 operation, WORD16 msgLen, BYTE *msg)
{
    PS_DB_TRUE_CHECK_RETNONE(msg == NULL);
    BOOL ret = FALSE;
    switch (operation)
    {
        case _DB_CFGCHG_TUPLE_NOTIFY_INSERT:
        case _DB_CFGCHG_TUPLE_NOTIFY_MODIFY:
        {
            UPF_TRACE_INFO("\n[ConfigNotify]recv modify Capacity msg!\n");
            g_qwDanageOpDelayLoop=0;

            _mcs_if(g_bDelayTimer<=POWER_ON_DELAY_TIMER)    /*上电过程中立即收到配置变更,不上报告警，只更新基础配置*/
            {
                powerOnProc();
                break;
            }
            
            WORD i = 0;
            R_NC_CAPACITY_TUPLE tmpcfg[R_NC_CAPACITY_CAPACITY] = {0};
            for(i=1;i<R_NC_CAPACITY_CAPACITY;i++)
            {
                BOOLEAN bflag = FALSE;
                DM_UPFNCU_GETR_NC_CAPACITY_REQ req = {0};
                DM_UPFNCU_GETR_NC_CAPACITY_ACK ack = {0};

                req.msgType = MSG_CALL;
                req.ctxname = i;
                req.vrusize = getVruSizeCap();
                ack.retCODE = RC_DBM_ERROR;
            
                bflag = DBM_CALL(DM_UPFNCU_GETR_NC_CAPACITY, (LPSTR)&req, (LPSTR)&ack);
                _mcs_if( (RC_DBM_OK != ack.retCODE) || !bflag)
                {
                    req.vrusize = (BYTE)VRUSIZECAP_ALL;
                    bflag = DBM_CALL(DM_UPFNCU_GETR_NC_CAPACITY, (LPSTR)&req, (LPSTR)&ack);
                }
                
                _mcs_if( (RC_DBM_OK == ack.retCODE)&&bflag)
                {
                    ret                   =TRUE;
                    tmpcfg[i].ctxname     = i;
                    tmpcfg[i].vrusize     = req.vrusize;
                    tmpcfg[i].capacity    = ack.capacity;
                    tmpcfg[i].valuetype   = ack.valuetype;
                    tmpcfg[i].rate        = ack.rate;
                    _mcs_if(0!=memcmp(&tmpcfg[i],&g_tR_CAPACITY_TUPLE[i],sizeof(R_NC_CAPACITY_TUPLE)))
                    {
                        g_qwDanageOpAlarm |= (1UL<<enumCAPACITY);
                        break;
                    }
                }
            }
            
            _mcs_if((TRUE==ret)&&(R_NC_CAPACITY_CAPACITY==i))
            {
                g_qwDanageOpAlarm &= (~(1UL<<enumCAPACITY));
            }
            break;
        }
        default:
        {
            break;
        }
    }
}
void Capacity::show()
{
    zte_printf_s("\n-------------initial value------------");

    int i=0;
    for(i=0;i<R_NC_CAPACITY_CAPACITY;i++)
    {
        if(0==g_tR_CAPACITY_TUPLE[i].ctxname)
        {
            continue;
        }
        zte_printf_s("\n ctxname = %u, vrusize = %u, capacity = %u, valuetype = %u, rate = %u",\
        g_tR_CAPACITY_TUPLE[i].ctxname,g_tR_CAPACITY_TUPLE[i].vrusize,g_tR_CAPACITY_TUPLE[i].capacity,g_tR_CAPACITY_TUPLE[i].valuetype, g_tR_CAPACITY_TUPLE[i].rate);
    }
    zte_printf_s("\n\n");
    return;
}
/* Ended by AICoder, pid:m862c6531e07ab814ea40a19e0e4fb2148765258 */

void Threadsize::powerOnProc()
{
    BYTE i=0;
    BOOLEAN bflag = FALSE;
    DM_UPFNCU_GETR_NC_THREADSIZE_BY_VRUTYPE_VRUSIZE_THREADNAME_REQ req = {0};
    DM_UPFNCU_GETR_NC_THREADSIZE_BY_VRUTYPE_VRUSIZE_THREADNAME_ACK ack = {0};
    req.msgType = MSG_CALL;
    req.vrutype = getVruType_Threadsize();
    req.vrusize = getVruSizeCap();
    
    for(i=1;i<R_NC_THREADSIZE_CAPACITY;i++)
    {
        req.threadname = i;
        ack.retCODE = RC_DBM_ERROR;

        bflag = DBM_CALL(DM_UPFNCU_GETR_NC_THREADSIZE_BY_VRUTYPE_VRUSIZE_THREADNAME, (LPSTR)&req, (LPSTR)&ack);

        if( (RC_DBM_OK == ack.retCODE) && bflag)
        {
            g_tR_THREADSIZE_TUPLE[i].vrusize      = req.vrusize;
            g_tR_THREADSIZE_TUPLE[i].vrutype      = req.vrutype;
            g_tR_THREADSIZE_TUPLE[i].threadname   = req.threadname;
            g_tR_THREADSIZE_TUPLE[i].threaddbsize = ack.threaddbsize;
        }
    }

    return;
}
/* Started by AICoder, pid:w4fdeba561m5ca414b220a1710c2217799582c01 */
void Threadsize::cfgNotifyProc(WORD16 operation, WORD16 msgLen, BYTE *msg)
{
    PS_DB_TRUE_CHECK_RETNONE(msg == NULL);

    BOOL ret = FALSE;
    switch (operation)
    {
        case _DB_CFGCHG_TUPLE_NOTIFY_INSERT:
        {
            _mcs_if(sizeof(R_NC_THREADSIZE_TUPLE) > msgLen)
            {
                UPF_TRACE_INFO("\n[ConfigNotify]recv wrong add notify msg!\n");
                return ;
            }
            break;
        }
        case _DB_CFGCHG_TUPLE_NOTIFY_DELETE:
        {
            _mcs_if(sizeof(R_NC_THREADSIZE_TUPLE) > msgLen)
            {
                UPF_TRACE_INFO("\n[ConfigNotify]recv wrong del notify msg!\n");
                return ;
            }
            break;
        }
        case _DB_CFGCHG_TUPLE_NOTIFY_MODIFY:
        {
            UPF_TRACE_INFO("\n[ConfigNotify]recv modify Threadsize msg!\n");
            _mcs_if(2*sizeof(R_NC_THREADSIZE_TUPLE) > msgLen)
            {
                UPF_TRACE_INFO("\n[ConfigNotify]recv wrong notify msg!\n");
                return ;
            }
            break;
        }
        default:
        {
            return;
        }
    }
    g_qwDanageOpDelayLoop=0;

    _mcs_if(g_bDelayTimer<=POWER_ON_DELAY_TIMER)    /*上电过程中立即收到配置变更,不上报告警，只更新基础配置*/
    {
        powerOnProc();
        return;
    }

    R_NC_THREADSIZE_TUPLE tmpcfg[R_NC_THREADSIZE_CAPACITY] = {0};
    BYTE i=0;
    BOOLEAN bflag = FALSE;
    DM_UPFNCU_GETR_NC_THREADSIZE_BY_VRUTYPE_VRUSIZE_THREADNAME_REQ req = {0};
    DM_UPFNCU_GETR_NC_THREADSIZE_BY_VRUTYPE_VRUSIZE_THREADNAME_ACK ack = {0};
    req.msgType = MSG_CALL;
    req.vrutype = getVruType_Threadsize();
    req.vrusize = getVruSizeCap();
    for(i=1;i<R_NC_THREADSIZE_CAPACITY;i++)
    {
        req.threadname = i;
        ack.retCODE = RC_DBM_ERROR;
    
        bflag = DBM_CALL(DM_UPFNCU_GETR_NC_THREADSIZE_BY_VRUTYPE_VRUSIZE_THREADNAME, (LPSTR)&req, (LPSTR)&ack);
    
        _mcs_if( (RC_DBM_OK == ack.retCODE) && bflag)
        {
            ret                    =TRUE;
            tmpcfg[i].vrusize      = req.vrusize;
            tmpcfg[i].vrutype      = req.vrutype;
            tmpcfg[i].threadname   = req.threadname;
            tmpcfg[i].threaddbsize = ack.threaddbsize;
            
            _mcs_if(0!=memcmp(&tmpcfg[i],&g_tR_THREADSIZE_TUPLE[i],sizeof(R_NC_THREADSIZE_TUPLE)))
            {
                g_qwDanageOpAlarm |= (1UL<<enumTHREADSIZE);
                break;
            }
        }
    }
    _mcs_if((TRUE==ret)&&(R_NC_THREADSIZE_CAPACITY==i))
    {
        g_qwDanageOpAlarm &= (~(1UL<<enumTHREADSIZE));
    }
    return;
}
void Threadsize::show()
{
    zte_printf_s("\n-------------initial value------------");
    int index = 0;
    for(index=0;index<R_NC_THREADSIZE_CAPACITY;index++)
    {
        if(g_tR_THREADSIZE_TUPLE[index].vrusize)
        {
            zte_printf_s("\n index = %-4u, vrutype = %2u, vrusize = %2u, threadname = %2u, threaddbsize = %2u",\
                index,g_tR_THREADSIZE_TUPLE[index].vrutype,g_tR_THREADSIZE_TUPLE[index].vrusize,g_tR_THREADSIZE_TUPLE[index].threadname,g_tR_THREADSIZE_TUPLE[index].threaddbsize);
        }
    }
    zte_printf_s("\n\n");
    return;
}

/* Ended by AICoder, pid:w4fdeba561m5ca414b220a1710c2217799582c01 */

void Threadinfo::powerOnProc()
{
    BYTE i=0;
    BOOLEAN bflag = FALSE;
    DM_UPFNCU_GETR_NC_THREADINFO_BY_VRUTYPE_VRUSIZE_THREADNAME_REQ req = {0};
    DM_UPFNCU_GETR_NC_THREADINFO_BY_VRUTYPE_VRUSIZE_THREADNAME_ACK ack = {0};
    req.msgType = MSG_CALL;
    req.vrutype = getVruType();
    req.vrusize = getVruSizeCap();
    for(i=1;i<R_NC_THREADINFO_CAPACITY;i++)
    {
        req.threadname = i;
        ack.retCODE = RC_DBM_ERROR;

        bflag = DBM_CALL(DM_UPFNCU_GETR_NC_THREADINFO_BY_VRUTYPE_VRUSIZE_THREADNAME, (LPSTR)&req, (LPSTR)&ack);

        if( (RC_DBM_OK == ack.retCODE) && bflag)
        {
            g_tR_THREADINFO_TUPLE[i].vrusize      = req.vrusize;
            g_tR_THREADINFO_TUPLE[i].vrutype      = req.vrutype;
            g_tR_THREADINFO_TUPLE[i].threadname   = req.threadname;
            g_tR_THREADINFO_TUPLE[i].threadnum    = ack.threadnum;
            g_tR_THREADINFO_TUPLE[i].isexclusive  = ack.isexclusive;
            g_tR_THREADINFO_TUPLE[i].priority     = ack.priority;
        }
    }
    return;
}
/* Started by AICoder, pid:jceed0fb10n1b2e142880a4720212e87b391b226 */
void Threadinfo::cfgNotifyProc(WORD16 operation, WORD16 msgLen, BYTE *msg)
{
    PS_DB_TRUE_CHECK_RETNONE(msg == NULL);
    BOOL ret = FALSE;
    switch (operation)
    {
        case _DB_CFGCHG_TUPLE_NOTIFY_INSERT:
        {
            _mcs_if(sizeof(R_NC_THREADINFO_TUPLE) > msgLen)
            {
                UPF_TRACE_INFO("\n[ConfigNotify]recv wrong add notify msg!\n");
                return ;
            }
            break;
        }
        case _DB_CFGCHG_TUPLE_NOTIFY_DELETE:
        {
            _mcs_if(sizeof(R_NC_THREADINFO_TUPLE) > msgLen)
            {
                UPF_TRACE_INFO("\n[ConfigNotify]recv wrong del notify msg!\n");
                return ;
            }
            break;
        }
        case _DB_CFGCHG_TUPLE_NOTIFY_MODIFY:
        {
            UPF_TRACE_INFO("\n[ConfigNotify]recv modify Threadinfo msg!\n");
            _mcs_if(2*sizeof(R_NC_THREADINFO_TUPLE) > msgLen)
            {
                UPF_TRACE_INFO("\n[ConfigNotify]recv wrong notify msg!\n");
                return ;
            }

            break;
        }
        default:
        {
            return;
        }
    }
    
    g_qwDanageOpDelayLoop=0;

    _mcs_if(g_bDelayTimer<=POWER_ON_DELAY_TIMER)    /*上电过程中立即收到配置变更,不上报告警，只更新基础配置*/
    {
        powerOnProc();
        return;
    }
    
    R_NC_THREADINFO_TUPLE tmpcfg[R_NC_THREADINFO_CAPACITY] = {0};
    BYTE i=0;
    BOOLEAN bflag = FALSE;
    DM_UPFNCU_GETR_NC_THREADINFO_BY_VRUTYPE_VRUSIZE_THREADNAME_REQ req = {0};
    DM_UPFNCU_GETR_NC_THREADINFO_BY_VRUTYPE_VRUSIZE_THREADNAME_ACK ack = {0};
    req.msgType = MSG_CALL;
    req.vrutype = getVruType();
    req.vrusize = getVruSizeCap();
    for(i=1;i<R_NC_THREADINFO_CAPACITY;i++)
    {
        req.threadname = i;
        ack.retCODE = RC_DBM_ERROR;
    
        bflag = DBM_CALL(DM_UPFNCU_GETR_NC_THREADINFO_BY_VRUTYPE_VRUSIZE_THREADNAME, (LPSTR)&req, (LPSTR)&ack);
    
        _mcs_if( (RC_DBM_OK == ack.retCODE) && bflag)
        {
            ret                    =TRUE;
            tmpcfg[i].vrusize      = req.vrusize;
            tmpcfg[i].vrutype      = req.vrutype;
            tmpcfg[i].threadname   = req.threadname;
            tmpcfg[i].threadnum    = ack.threadnum;
            tmpcfg[i].isexclusive  = ack.isexclusive;
            tmpcfg[i].priority     = ack.priority;
            _mcs_if(0!=memcmp(&tmpcfg[i],&g_tR_THREADINFO_TUPLE[i],sizeof(R_NC_THREADINFO_TUPLE)))
            {
                g_qwDanageOpAlarm |= (1UL<<enumTHREADINFO);
                break;
            }
        }
    }
    
    _mcs_if((TRUE==ret)&&(R_NC_THREADINFO_CAPACITY==i))
    {
        g_qwDanageOpAlarm &= (~(1UL<<enumTHREADINFO));
    }
    return;
}
void Threadinfo::show()
{
    zte_printf_s("\n-------------initial value------------");
    int index = 0;
    for(index=0;index<R_NC_THREADINFO_CAPACITY;index++)
    {
        if(g_tR_THREADINFO_TUPLE[index].vrusize)
        {
            zte_printf_s("\n index = %-4u, vrutype = %2u, vrusize = %2u, threadname = %2u, threadnum = %2u, isexclusive = %2u, priority = %2u",\
                index,g_tR_THREADINFO_TUPLE[index].vrutype,g_tR_THREADINFO_TUPLE[index].vrusize,g_tR_THREADINFO_TUPLE[index].threadname,\
                g_tR_THREADINFO_TUPLE[index].threadnum,g_tR_THREADINFO_TUPLE[index].isexclusive,g_tR_THREADINFO_TUPLE[index].priority);
        }
    }
    zte_printf_s("\n\n");
    return;
}

/* Ended by AICoder, pid:jceed0fb10n1b2e142880a4720212e87b391b226 */

void DpdkSpecialNfCfg::powerOnProc()
{
    BOOLEAN bflag = FALSE;
    DM_UPF_GETDPDKSPECIALNFCFG_REQ req = {0};
    DM_UPF_GETDPDKSPECIALNFCFG_ACK ack = {0};
    req.msgType = MSG_CALL;
    req.dwgindex = (WORD32)1;
    ack.retCODE = RC_DBM_OK;
    bflag = DBM_CALL(DM_UPF_GETDPDKSPECIALNFCFG, (LPSTR)&req, (LPSTR)&ack);
    PS_DB_TRUE_CHECK_RETNONE((RC_OK != ack.retCODE) || (TRUE != bflag));
    
    g_tDPDKSPECIALNFCFG_TUPLE.dwgindex          = 1;
    g_tDPDKSPECIALNFCFG_TUPLE.benchmarkmbufnum  = ack.benchmarkmbufnum;
    g_tDPDKSPECIALNFCFG_TUPLE.pfumbufnum        = ack.pfumbufnum;
    return;
}

void DpdkSpecialNfCfg::cfgNotifyProc(WORD16 operation, WORD16 msgLen, BYTE *msg)
{
    if(msg == NULL)
    {
      return;
    }
    
    switch (operation)
    {
        case _DB_CFGCHG_TUPLE_NOTIFY_MODIFY:
        {
            UPF_TRACE_INFO("\n[ConfigNotify]recv modify DpdkSpecialNfCfg msg!\n");
            if(2*sizeof(DPDKSPECIALNFCFG_TUPLE) > msgLen)
            {
                UPF_TRACE_INFO("\n[ConfigNotify]recv wrong notify msg!\n");
                return ;
            }
            
            g_qwDanageOpDelayLoop=0;

            _mcs_if(g_bDelayTimer<=POWER_ON_DELAY_TIMER)    /*上电过程中立即收到配置变更,不上报告警，只更新基础配置*/
            {
                powerOnProc();
                break;
            }

            DPDKSPECIALNFCFG_TUPLE *ptupleNew = (DPDKSPECIALNFCFG_TUPLE *)(msg + sizeof(DPDKSPECIALNFCFG_TUPLE));
            if((ptupleNew->benchmarkmbufnum==g_tDPDKSPECIALNFCFG_TUPLE.benchmarkmbufnum)&&(ptupleNew->pfumbufnum==g_tDPDKSPECIALNFCFG_TUPLE.pfumbufnum))
            {
                g_qwDanageOpAlarm &= (~(1UL<<enumDPDKSPECIALCFG));
                break;
            }
            
            g_qwDanageOpAlarm |= (1UL<<enumDPDKSPECIALCFG);
            break;
        }
        default:
        {
            break;
        }
    }
}

extern "C" void DpdkSpecialNfCfg_show()
{
    zte_printf_s("\n-------------initial value------------");
    zte_printf_s("\n benchmarkmbufnum            =%u ",g_tDPDKSPECIALNFCFG_TUPLE.benchmarkmbufnum);
    zte_printf_s("\n pfumbufnum                  =%u \n",g_tDPDKSPECIALNFCFG_TUPLE.pfumbufnum);

    BOOLEAN bflag = FALSE;
    DM_UPF_GETDPDKSPECIALNFCFG_REQ req = {0};
    DM_UPF_GETDPDKSPECIALNFCFG_ACK ack = {0};
    req.msgType = MSG_CALL;
    req.dwgindex = (WORD32)1;
    ack.retCODE = RC_DBM_OK;
    bflag = DBM_CALL(DM_UPF_GETDPDKSPECIALNFCFG, (LPSTR)&req, (LPSTR)&ack);
    PS_DB_TRUE_CHECK_RETNONE((RC_OK != ack.retCODE) || (TRUE != bflag));
    zte_printf_s("\n-------------current value------------");
    zte_printf_s("\n benchmarkmbufnum            =%u ",ack.benchmarkmbufnum);
    zte_printf_s("\n pfumbufnum                  =%u \n",ack.pfumbufnum);
    return;
}

void DpdkSpecialNfCfg::show()
{
    DpdkSpecialNfCfg_show();
    return;
}


void DpdkCommCfg::powerOnProc()
{
    BOOLEAN bflag = FALSE;
    DM_COMM_NF_CFG_GETDPDKCOMMNFCFG_REQ req = {0};
    DM_COMM_NF_CFG_GETDPDKCOMMNFCFG_ACK ack = {0};
    req.msgType = MSG_CALL;
    req.dwgindex = (WORD32)1;
    ack.retCODE = RC_DBM_OK;
    bflag = DBM_CALL(DM_COMM_NF_CFG_GETDPDKCOMMNFCFG, (LPSTR)&req, (LPSTR)&ack);
    PS_DB_TRUE_CHECK_RETNONE((RC_OK != ack.retCODE) || (TRUE != bflag));
    
    g_tDPDKCOMMNFCFG_TUPLE.dwgindex          = 1;
    g_tDPDKCOMMNFCFG_TUPLE.supportqinq       = ack.supportqinq;
    g_tDPDKCOMMNFCFG_TUPLE.multiqueue        = ack.multiqueue;
    g_tDPDKCOMMNFCFG_TUPLE.scaleradio        = ack.scaleradio;
    g_tDPDKCOMMNFCFG_TUPLE.rawpktmtu         = ack.rawpktmtu;
    g_tDPDKCOMMNFCFG_TUPLE.eiopktmtu         = ack.eiopktmtu;

    return;
}

void DpdkCommCfg::cfgNotifyProc(WORD16 operation, WORD16 msgLen, BYTE *msg)
{
    if(msg == NULL)
    {
      return;
    }
    
    switch (operation)
    {
        case _DB_CFGCHG_TUPLE_NOTIFY_MODIFY:
        {
            UPF_TRACE_INFO("\n[ConfigNotify]recv modify DpdkCommCfg msg!\n");
            if(2*sizeof(DPDKCOMMNFCFG_TUPLE) > msgLen)
            {
                UPF_TRACE_INFO("\n[ConfigNotify]recv wrong notify msg!\n");
                return ;
            }
            g_qwDanageOpDelayLoop=0;

            _mcs_if(g_bDelayTimer<=POWER_ON_DELAY_TIMER)    /*上电过程中立即收到配置变更,不上报告警，只更新基础配置*/
            {
                powerOnProc();
                break;
            }

            DPDKCOMMNFCFG_TUPLE *ptupleNew = (DPDKCOMMNFCFG_TUPLE *)(msg + sizeof(DPDKCOMMNFCFG_TUPLE));
            
            DPDKCOMMNFCFG_TUPLE tmpcfg = {0};
            tmpcfg.dwgindex          = 1;
            tmpcfg.supportqinq       = ptupleNew->supportqinq;
            tmpcfg.multiqueue        = ptupleNew->multiqueue;
            tmpcfg.scaleradio        = ptupleNew->scaleradio;
            tmpcfg.rawpktmtu         = ptupleNew->rawpktmtu;
            tmpcfg.eiopktmtu         = ptupleNew->eiopktmtu;
            
            if(0==memcmp(&tmpcfg,&g_tDPDKCOMMNFCFG_TUPLE,sizeof(DPDKCOMMNFCFG_TUPLE)))
            {
                g_qwDanageOpAlarm &= (~(1UL<<enumDPDKCOMMCFG));
                break;
            }

            g_qwDanageOpAlarm |= (1UL<<enumDPDKCOMMCFG);
            break;
        }
        default:
        {
            break;
        }
    }
}
void DpdkCommCfg::show()
{
    zte_printf_s("\n-------------initial value------------");
    zte_printf_s("\n supportqinq            =%u ",g_tDPDKCOMMNFCFG_TUPLE.supportqinq);
    zte_printf_s("\n multiqueue             =%u ",g_tDPDKCOMMNFCFG_TUPLE.multiqueue);
    zte_printf_s("\n scaleradio             =%u ",g_tDPDKCOMMNFCFG_TUPLE.scaleradio);
    zte_printf_s("\n rawpktmtu              =%u ",g_tDPDKCOMMNFCFG_TUPLE.rawpktmtu);
    zte_printf_s("\n eiopktmtu              =%u \n",g_tDPDKCOMMNFCFG_TUPLE.eiopktmtu);
    BOOLEAN bflag = FALSE;
    DM_COMM_NF_CFG_GETDPDKCOMMNFCFG_REQ req = {0};
    DM_COMM_NF_CFG_GETDPDKCOMMNFCFG_ACK ack = {0};
    req.msgType = MSG_CALL;
    req.dwgindex = (WORD32)1;
    ack.retCODE = RC_DBM_OK;
    bflag = DBM_CALL(DM_COMM_NF_CFG_GETDPDKCOMMNFCFG, (LPSTR)&req, (LPSTR)&ack);
    PS_DB_TRUE_CHECK_RETNONE((RC_OK != ack.retCODE) || (TRUE != bflag));
    zte_printf_s("\n-------------current value------------");
    zte_printf_s("\n supportqinq            =%u ",ack.supportqinq);
    zte_printf_s("\n multiqueue             =%u ",ack.multiqueue);
    zte_printf_s("\n scaleradio             =%u ",ack.scaleradio);
    zte_printf_s("\n rawpktmtu              =%u ",ack.rawpktmtu);
    zte_printf_s("\n eiopktmtu              =%u \n",ack.eiopktmtu);
    return;
}
