#ifndef PS_NC_REPORT_STRUCT_HEAD_H_
#define PS_NC_REPORT_STRUCT_HEAD_H_

#include "tulip.h"
typedef struct
{
    #ifdef _LITTLE_ENDIAN_
     BITS      AFlag:1;
     BITS      Spare:3;
     BITS      RFlag:1;
     BITS      Version:3;
    #else
     BITS      Version:3;   //没有规定，先设置为1
     BITS      RFlag:1;     //请求标识，置0表示不需要响应，当前置0
     BITS      Spare:3;     //保留
     BITS      AFlag:1;     //匿名处理标志，当前不做匿名处理，当前置0
    #endif
    BYTE       MessageType;  //当前定义3种报文类型
    WORD16     Length;       //载荷长度，不含头
    WORD16     SequenceNum;  //发送序列号，未明确，从零递增
    BYTE       NeTYpe;       //未明确，写upf
    BYTE       Resverd;      //保留
    WORD32     NeId;         //未明确？
}T_ExpDataHead;

typedef enum
{
    EXP_REPORT_TYPE = 1,
    HEART_BEART_REQUEST,
    HEART_BREAT_RESPONSE,
}enumExpHeadMsgType;

#define PS_FILL_EXP_HEAD_TO_NWDAF(ptHead, type, length) \
{ \
    ptHead->Version = 1;                         \
    ptHead->RFlag   = 0;                         \
    ptHead->Spare   = 0;                         \
    ptHead->AFlag   = 0;                         \
    ptHead->MessageType  = type;                 \
    ptHead->Length       = length;               \
    ptHead->SequenceNum  = mcs_ntohs(g_report_seq_num);     \
    ptHead->NeTYpe       = 0;                    \
    ptHead->Resverd      = 0;                    \
    ptHead->NeId         = 0;                    \
    g_report_seq_num++;                          \
}

#endif
