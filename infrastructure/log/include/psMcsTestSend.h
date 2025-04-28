#ifndef PS_MCS_TEST_SEND_H_H
#define PS_MCS_TEST_SEND_H_H


#include "tulip.h"
#include "psNcuReportStructData.h"
WORD32 ps_make_ncu_subscribe(BYTE subscribeType, WORD64 upseid, WORD32 appid, BYTE subtype, BYTE threadID);
void ps_set_uli(WORD64 cgi, WORD64 sai, WORD64 rai, WORD64 tai, WORD64 ecgi, WORD64 ncgi);
void ps_set_correlation(CHAR* cord);
void ps_make_ncu_pkt_slowflowflg(BYTE slowflowflg, BYTE threadID);
void ps_make_ncu_pkt_setdata(WORD64 upseid, WORD32 subappid, BYTE iptype, BYTE dir, BYTE threadID, WORD16 payload, WORD32 pktTimsStamp);
void ps_make_ncu_pkt(WORD64 upseid, WORD32 subappid, BYTE iptype, BYTE dir, BYTE threadID);
void ps_make_ncu_trafficrpt(WORD64 upseid, WORD32 subappid, BYTE iptype, BYTE dir, BYTE threadID);
void ps_set_session(CHAR* imsi, CHAR* isdn, BYTE rateType, char* ipv4, char* ipv6, char* dnn);
void ps_set_appid_relate(WORD32 subappid1, WORD32 subappid2, WORD32 subappid3, WORD32 subappid4);
void psNcuFillDataTest(T_Notification* ptNotification, T_CorrelationId* ptCorrelationId);
void test_set_session_rattype(BYTE rattype);
WORD32 ps_set_appid_subappid(WORD32 appid, WORD32 subappid);
BOOL psUpfCommString2tBCD(BYTE *tBCD, BYTE *pbDigits, BYTE bDigitLen);
void test_set_stm_rel_info(char* srcip, char* dstip, WORD16 srcport, WORD16 dstPort,BYTE pro);
void test_set_del_stm_flg(BYTE flg);
void test_set_pkt(char* srcip, char* dstip, WORD16 srcport, WORD16 dstPort, WORD32 synnum, WORD32 acknum);
void ps_set_subscribe_nwdaf_ip(const char* ipv4, const char* ipv6);
void ps_make_ncu_ucom_msg(BYTE threadID, WORD32 dwMsgType, WORD32 dwGroupId, BYTE iptype);
void ps_make_heartbeat(BYTE ipType, BYTE threadID, BYTE msgtype);
#endif