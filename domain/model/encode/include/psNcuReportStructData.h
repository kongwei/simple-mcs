#ifndef PS_NC_REPORT_STRUCT_DATA_H_
#define PS_NC_REPORT_STRUCT_DATA_H_
#include "tulip.h"
#include "ps_pub.h"


#pragma pack(1)    /* noalign for psosystem C */


typedef enum
{
    IETYPE_NotificationBase = 4,
    IETYPE_QosAnaInfo       = 5,
    IETYPE_ApplicationID    = 6,
    IETYPE_SubAppID         = 6,
    IETYPE_ReportType       = 7,
    IETYPE_QosMonReports    = 8,
    IETYPE_EventType        = 9,
    IETYPE_Snssai           = 10,
    IETYPE_FlowInfo         = 11,
    IETYPE_CorrelationId    = 12,
    IETYPE_UeIpv4Addr       = 14,
    IETYPE_UeIpv6Prefix     = 15,
    IETYPE_UeMacAddr        = 16,
    IETYPE_TimeStamp        = 17,
    IETYPE_StartTime        = 18,
    IETYPE_Dnn              = 19,
    IETYPE_Supi             = 20,
    IETYPE_Gpsi             = 21,
    IETYPE_RatType          = 22,
    IETYPE_UserLocationInfo = 23,
    
}enumNotificationIEType;

typedef struct
{
    WORD16  type;                  //4固定
    WORD16  length;                //去除type+lenth之外还有几个BYTE，此处为1+后续所有数据长度
    #ifdef _LITTLE_ENDIAN_
    BYTE   Instance:4;
    BYTE   Spare:4;
    #else
    BYTE  Spare:4;
    BYTE  Instance:4;
    #endif
}T_IE_Base;

typedef enum {
    EVENT_QOS_MONITORING,
    EVENT_USER_DATA_USAGE_MEASURES,
    EVENT_USER_DATA_USAGE_TRENDS,
    EVENT_TSC_MNGT_INFO,
    EVENT_QOS_ANA,  //质差数据
    EVENT_QOS_EXP   //体验数据
}enumEventType;

typedef struct
{
    T_IE_Base IEBase;
    BYTE   EventType;    //enumEventType
}T_IE_EventType;

typedef struct
{
    T_IE_Base IEBase;
    T_IPV4    Ipv4;
}T_IE_UeIpv4Addr;


typedef struct
{
    T_IE_Base       IEBase;
    BYTE          Ipv6Pre[16];
    BYTE          prelen;
}T_IE_UeIpv6Prefix;

typedef struct
{
    T_IE_Base       IEBase;
    BYTE            MacAddr[6];
}T_IE_UeMacAddr;

typedef struct
{
    T_IE_Base      IEBase;
    WORD32         dwTimeStamp;
    WORD16         milliisend;
}T_IE_TimeStamp;

typedef struct
{
    T_IE_Base      IEBase;
    WORD32         dwTime;
    WORD16         milliisend;
}T_IE_StartTime;

typedef struct
{
    T_IE_Base      IEBase;
    CHAR           dnn[64];
}T_IE_Dnn;

typedef struct
{
    T_IE_Base      IEBase;
    BYTE           sst;
    BYTE          sd[3];
}T_IE_Snssai;

typedef struct
{
    T_IE_Base      IEBase;
    BYTE           supi[8];
}T_IE_Supi;

typedef struct
{
    T_IE_Base      IEBase;
    BYTE           gpsi[8];
}T_IE_Gpsi;

typedef enum
{
    RATE_TYPE_UTRAN = 1,
    RATE_TYPE_GERAN,
    RATE_TYPE_WLAN,
    RATE_TYPE_GAN,
    RATE_TYPE_HSPA,
    RATE_TYPE_EUTRAN,
    RATE_TYPE_VIRTUAL,
    RATE_TYPE_EUTRAN_NB_IOT,
    RATE_TYPE_LTE_M,
    RATE_TYPE_NR,
    RATE_TYPE_WB_E_UTRAN_LEO,
    RATE_TYPE_WB_E_UTRAN_MEO,
    RATE_TYPE_WB_E_UTRAN_GEO,
    RATE_TYPE_WB_E_UTRAN_OTHERSAT,
    RATE_TYPE_EUTRAN_NB_IOT_LEO,
    RATE_TYPE_EUTRAN_NB_IOT_MEO,
    RATE_TYPE_EUTRAN_NB_IOT_GEO,
    RATE_TYPE_EUTRAN_NB_IOT_OTHERSAT,
    RATE_TYPE_LTE_M_LEO,
    RATE_TYPE_LTE_M_MEO,
    RATE_TYPE_LTE_M_GEO,
    RATE_TYPE_LTE_M_OTHERSAT
}enumRatType;

typedef struct
{
    T_IE_Base      IEBase;
    BYTE           bRatType;
}T_IE_RatType;

typedef struct T_psGTPMccMnc1
{
#ifdef _LITTLE_ENDIAN_
    BITS btMCC1  : 4;  /* MCC digit 1 */
    BITS btMCC2  : 4;  /* MCC digit 2 */

    BITS btMCC3  : 4;  /* MCC digit 3 */
    BITS btMNC3  : 4;  /* MNC digit 3 */

    BITS btMNC1  : 4;  /* MNC digit 1 */
    BITS btMNC2  : 4;  /* MNC digit 2 */
#else
    BITS btMCC2  : 4;  /* MCC digit 2 */
    BITS btMCC1  : 4;  /* MCC digit 1 */

    BITS btMNC3  : 4;  /* MNC digit 3 */
    BITS btMCC3  : 4;  /* MCC digit 3 */

    BITS btMNC2  : 4;  /* MNC digit 2 */
    BITS btMNC1  : 4;  /* MNC digit 1 */
#endif
} T_psGTPMccMnc1;

typedef struct
{
    T_psGTPMccMnc1 mccmnc;
    WORD16   LAC;
    WORD16   CellIdentify;
}T_FIELD_CGI;

typedef struct
{
    T_psGTPMccMnc1 mccmnc;
    WORD16   LAC;
    WORD16   ServiceAreaCode;
}T_FIELD_SAI;

typedef struct
{
    T_psGTPMccMnc1 mccmnc;
    WORD16   LAC;
    WORD16   RoutingAreaCode;
}T_FIELD_RAI;

typedef struct
{
    T_psGTPMccMnc1 mccmnc;
    WORD16   TrackAreaCode;
}T_FIELD_TAI;

typedef struct
{
    T_psGTPMccMnc1 mccmnc;
    #ifdef _LITTLE_ENDIAN_
    BYTE   ECI:4;
    BYTE   Spare:4;
    #else
    BYTE   Spare:4;
    BYTE   ECI:4;
    #endif
    BYTE  LAC;
    WORD16   ECellIdentifer;
}T_FIELD_ECGI;


typedef struct
{
    T_psGTPMccMnc1 mccmnc;
    WORD16   LAC;
}T_FIELD_LAI;

typedef struct
{
    #ifdef _LITTLE_ENDIAN_
    BYTE           gNbf:1;
    BYTE           ngeNbf:1;
    BYTE           Spare2:7;
    #else
    BYTE           Spare2:7;
    BYTE           ngeNbf:1;
    BYTE           gNbf:1;
    #endif
}T_GNB_HEAD;
typedef struct
{
    #ifdef _LITTLE_ENDIAN_
    BYTE           ALI:1;
    BYTE           LTM:1;
    BYTE           GGI:1;
    BYTE           GDI:1;
    BYTE           NGENB:1;
    BYTE           Spare:3;
    #else
    BYTE           Spare:3;
    BYTE           NGENB:1;
    BYTE           GDI:1;
    BYTE           GGI:1;
    BYTE           LTM:1;
    BYTE           ALI:1;
    #endif
    BYTE          Spare1;
    BYTE          TAI_PlmnLen;
    BYTE          TAI_PlmnValue[5];
    BYTE          TAI_TacLen;
    BYTE          TAI_TacValue[5];
    BYTE          ECGI_PlmnLen;
    BYTE          ECGI_PlmnValue[5];
    BYTE          ECGI_EutraCellLen;
    BYTE          ECGI_EutraCellValue[5];
    BYTE          ALI_ageOfLIvalue[4];
    BYTE          LTMLen;
    BYTE          LTMLocationTimestampValue[5];
    BYTE          GGILen;
    BYTE          GGIgeographInfoValue[5];
    BYTE          GDILen;
    BYTE          GDIGendeticInfoValue[5];
    T_GNB_HEAD    head;
    BYTE          NgeNb_Plmn_len;
    BYTE          NgeNb_Plmn_value[5];
    BYTE          NgeNb_gNb_bit_len;
    BYTE          NgeNb_gNb_len;
    BYTE          NgeNb_gNb_value[5];
    BYTE          NgeNb_ngeNbID_len;
    BYTE          NgeNb_ngeNbID_value[5];
}T_EutraLocation;


typedef struct
{
    #ifdef _LITTLE_ENDIAN_
    BYTE           ALI:1;
    BYTE           LTM:1;
    BYTE           GGI:1;
    BYTE           GDI:1;
    BYTE           GNB:1;
    BYTE           Spare:3;
    #else
    BYTE           Spare:3;
    BYTE           GNB:1;
    BYTE           GDI:1;
    BYTE           GGI:1;
    BYTE           LTM:1;
    BYTE           ALI:1;
    #endif
    BYTE          Spare1;
    BYTE          TAI_PlmnLen;
    BYTE          TAI_PlmnValue[5];
    BYTE          TAI_TacLen;
    BYTE          TAI_TacValue[5];
    BYTE          NCGI_PlmnLen;
    BYTE          NCGI_PlmnValue[5];
    BYTE          NCGI_NrCellLen;
    BYTE          NCGI_NrCellValue[5];
    BYTE          ALI_ageOfLIvalue[4];
    BYTE          LTMLen;
    BYTE          LTMLocationTimestampValue[5];
    BYTE          GGILen;
    BYTE          GGIgeographInfoValue[5];
    BYTE          GDILen;
    BYTE          GDIGendeticInfoValue[5];
    T_GNB_HEAD    head;
    BYTE          GNB_Plmn_len;
    BYTE          GNB_Plmn_value[5];
    BYTE          GNB_gNb_bit_len;
    BYTE          NGNB_gNb_len;
    BYTE          GNB_gNb_value[5];
    BYTE          GNB_ngeNbID_len;
    BYTE          GNB_ngeNbID_value[5];
}T_NrLocation;
typedef struct
{

}T_N3gaLocation;
typedef struct
{

}T_UtraLocation;
typedef struct
{
#ifdef _LITTLE_ENDIAN_
    BYTE           CGI : 1;
    BYTE           SAI : 1;
    BYTE           LAI : 1;
    BYTE           RAI : 1;
    BYTE           spare : 4;
#else
    BYTE           spare : 4;
    BYTE           RAI : 1;
    BYTE           LAI : 1;
    BYTE           SAI : 1;
    BYTE           CGI : 1;
    #endif

    T_FIELD_CGI tCGI;
    T_FIELD_SAI tSAI;
    T_FIELD_RAI tRAI;
    T_FIELD_LAI tLAI;
}T_GeraLocation;
// typedef struct
//     #ifdef _LITTLE_ENDIAN_
//     #else
//     #endif






typedef struct
{
    T_IE_Base      Base;
//     #ifdef _LITTLE_ENDIAN_
//     #else
//     #endif
    

#ifndef _LITTLE_ENDIAN_
    BYTE           btSpare : 3;
    BYTE           btGeraLoction : 1;
    BYTE           btUtraLocation : 1;
    BYTE           btN3gaLocation : 1;
    BYTE           btNrLocation : 1;
    BYTE           btEutraLocation : 1;
#else
    BYTE           btEutraLocation : 1;
    BYTE           btNrLocation : 1;
    BYTE           btN3gaLocation : 1;
    BYTE           btUtraLocation : 1;
    BYTE           btGeraLoction : 1;
    BYTE           btSpare : 3;
#endif
    T_EutraLocation eutraLocation;
    T_NrLocation nrLocation;
    T_N3gaLocation n3gaLocation;
    T_UtraLocation utraLocation;
    T_GeraLocation geraLocation;
}T_IE_UserLocationInfo;

typedef struct
{
    T_IE_Base           IEBase;
    CHAR              Appid[64];
}T_IE_ApplicationID;

typedef struct
{
    T_IE_Base           IEBase;
    CHAR              SubAppid[64];
}T_IE_SubAppID;

typedef struct
{
    T_IE_Base           IEBase;
    WORD32              dwFlow;
}T_IE_FlowInfo;

typedef struct
{
    T_IE_Base           IEBase;
    BYTE                type;
}T_IE_ReportType;

typedef struct
{
    T_IE_Base           IEBase;
    WORD32              delay_an;
    WORD32              delay_dn;
    WORD32              bandwidth_ul;
    WORD32              bandwidth_dl;
    WORD32              lostpackets_ul;
    WORD32              totalpackets_ul;
    WORD32              lostpackets_dl;
    WORD32               totalpackets_dl;
}T_IE_QosMonReports;

typedef struct
{
    BYTE has_ApplicationID_flg:1;
    BYTE has_SubAppID_flg:1;
    BYTE has_FlowInfo_flg:1;
    BYTE has_ReportType_flg:1;
    BYTE has_QosMonReports_flg:1;
    T_IE_Base           Base;
    T_IE_ApplicationID  ApplicationID;
    T_IE_SubAppID       SubAppID;
    T_IE_FlowInfo       FlowInfo;
    T_IE_ReportType     ReportType;
    T_IE_QosMonReports   QosMonReports;
}T_IE_QosAnaInfo;

typedef struct
{
    BYTE has_EventType_flg:1;
    BYTE has_UeIpv4Addr_flg:1;
    BYTE has_UeIpv6Prefix_flg:1;
    BYTE has_UeMacAddr_flg:1;
    BYTE has_TimeStamp_flg:1;
    BYTE has_StartTime_flg:1;
    BYTE has_Dnn_flg:1;
    BYTE has_Snssai_flg:1;
    BYTE has_Supi_flg:1;
    BYTE has_Gpsi_flg:1;
    BYTE has_RatType_flg:1;
    BYTE has_UserLocationInfo_flg:1;
    BYTE has_QosAnaInfo_flg:1;
    T_IE_Base             Base;
    T_IE_EventType        EventType;
    T_IE_UeIpv4Addr       UeIpv4Addr;
    T_IE_UeIpv6Prefix     UeIpv6Prefix;
    T_IE_UeMacAddr        UeMacAddr;
    T_IE_TimeStamp        TimeStamp;
    T_IE_StartTime        StartTime;
    T_IE_Dnn              Dnn;
    T_IE_Snssai           Snssai;
    T_IE_Supi             Supi;
    T_IE_Gpsi             Gpsi;
    T_IE_RatType          RatType;
    T_IE_UserLocationInfo UserLocationInfo;
    T_IE_QosAnaInfo       QosAnaInfo;
}T_Notification;

typedef struct
{
    T_IE_Base   Base;
    CHAR correlationId[256];
}T_CorrelationId;

#define NCU_ENCODE_IE_FIELD(IETOP, FLAG, FIELD) \
do{ \
    if(IETOP->FLAG) \
    { \
        wIElen = sizeof(T_FIELD_##FLAG);\
        zte_memcpy_s(bufferCur, wIElen, (BYTE*)&IETOP->FIELD, wIElen);\
        bufferCur+= wIElen;\
        *ptDataLen+=wIElen;\
    }\
}while(0)

#define NCU_ENCODE_IE(IETopPointer, IEInfo) \
do{ \
    if(IETopPointer->has_##IEInfo##_flg) \
    { \
        if(0 != IETopPointer->IEInfo.IEBase.length){\
            wIElen = mcs_htons(IETopPointer->IEInfo.IEBase.length) + sizeof(T_IE_Base);\
        }\
        else{\
            wIElen = sizeof(T_IE_##IEInfo);\
            IETopPointer->IEInfo.IEBase.length = mcs_htons(wIElen - sizeof(T_IE_Base)); \
        }\
        IETopPointer->IEInfo.IEBase.type   = mcs_htons(IETYPE_##IEInfo); \
        IETopPointer->IEInfo.IEBase.Spare  = 0; \
        zte_memcpy_s(bufferCur, wIElen, (BYTE*)&IETopPointer->IEInfo, wIElen); /*rte_hexdump(stdout,"data "#IEInfo,bufferCur, wIElen);*/ \
        bufferCur+= wIElen;\
        *ptDataLen+=wIElen;\
    }\
}while(0)

#pragma pack()     /* align for psosystem C */
#endif
