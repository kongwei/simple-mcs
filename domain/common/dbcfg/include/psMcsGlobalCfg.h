#ifndef PS_MCS_GLOBAL_CFG_H_
#define PS_MCS_GLOBAL_CFG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "tulip.h"
// #include "r_danwdafcfg.h"
#include "r_dasystemcfg.h"
#include "flowageingtime.h"
#include "r_resourceage.h"
#include "r_upassocaddr.h"

#define PFU_SOFTPARA_ID_START       (WORD32)4000
#define PFU_SOFTPARA_MAX            (WORD32)2000
typedef struct
{
    CHAR upfName[LEN_R_UPASSOCADDR_UPNAME_MAX + 1];
    CHAR upfIpv4[LEN_R_DAUPFCFG_UPFIPV4_MAX + 1];
    CHAR upfIpv6[LEN_R_DAUPFCFG_UPFIPV6_MAX + 1];
    BYTE bPreferAddrType;
    BYTE bRsv[7];
    WORD16 wVrfV4;
    WORD16 wVrfV6;
    WORD32 rsv;
}T_DaUpfIpCfg;


typedef struct
{
    BYTE     bAppexpswitch;
    BYTE     bAnareportintf;
    BYTE     bExpreportintf;
    BYTE     bRptMulSwitch;
    WORD32   expreporttime;
}T_DataAnalysSysCfg;

// typedef struct

typedef struct
{
    WORD32  dwAgetime;
    BYTE    bScanstep;
    BYTE    usRsv[3]; 
}T_ResourceAgeData;

typedef struct
{
    CHAR    bCtxname[LEN_R_RESOURCEAGE_CTXNAME_MAX + 1];
    WORD32  dwAgetime;
    BYTE    bScanstep;
    BYTE    usRsv[3];
}T_ResourceAgeCfg;

typedef struct
{
    WORD16  longtime;
    WORD16  shorttime;
    BYTE    usRsv[4];
}T_FlowAgeingTimeCfg;

typedef struct
{
    BYTE         echoswitch;
    BYTE         echofailuretimes;
    BYTE         echofailurepolicy;
    BYTE         echoackpolicy;
    WORD32       echotime;

    BYTE         times;
    BYTE         rsv;
    WORD16       udpport;
    WORD16       rsv1;
}T_DaSynLinkCfg;

typedef struct
{
    BYTE     bSigStopOnOverLoad;
    BYTE     bSigRecoveryThreshold;
    BYTE     bSigStopThreshold;
    BYTE     usRsv[1];
    WORD16   bSigOverTimeThreshold_Hours;
    WORD16   WildTraceMaxNum;;
    
    WORD32   MaxReportPPS;
    BYTE     usRsv2[4];
}T_UpfSigTraceCfg;

typedef struct
{
    BYTE     FailStopOnOverLoad;
    BYTE     FailRecoveryThreshold;
    BYTE     FailStopThreshold;
    BYTE     usRsv[1];
    WORD16   FailOverTimeThreshold_Hours;
    BYTE     usRsv1[2];

    WORD32   MaxReportPPS;
    BYTE     usRsv2[4];
}T_UpfFailTraceCfg;

typedef struct {
    WORD32 dwKeyServQaSwitch ;
    WORD32 dwKeyUserDataColSwitch;
    WORD32 dwSampUserDataColSwitch;
    WORD32 dwKeyServQaCapacity;
    WORD32 dwKeyUserDataColCapacity;
    WORD32 dwSampUserDataColCapacity;
    BYTE   hasGetLicCfg;
}T_LicenseCfg;

typedef struct
{
    BYTE      bCheckSwitch;
    BYTE      bCheckFailurePolicy;
    BYTE      bReportPolicy;
    BYTE      bRsv;
    WORD32    dwCheckTime;

    WORD32    dwAgingTime;
    WORD32    dwRsv;
}T_DaHttpCfg;

typedef struct UPFGlobalConfig
{
    T_DaUpfIpCfg daUpfIpCfg;
    T_DataAnalysSysCfg dataSysCfg;
    T_FlowAgeingTimeCfg flowAgeingTimeCfg;
    T_ResourceAgeCfg resourceAgeCfg[R_RESOURCEAGE_CAPACITY];
    T_DaSynLinkCfg   daSyslinkCfg;
    T_LicenseCfg     ncuLicenseCfg;
    T_ResourceAgeData tSnAge;
    BYTE   bResourceAgeCfgScanSuccFlg;
    BYTE   bResourceAgeCfgAgeParaChanged;
    BYTE   bResourceAgeCfgAgeTableNums;
    BYTE   rsv;

    WORD16 wClientProfileV4ID;
    WORD16 wClientProfileV6ID;
    /* softpara start */
    WORD32  PfuSoftPara[PFU_SOFTPARA_MAX];
    /* softpara end */

    T_DaHttpCfg    daHttpCfg;
}UPFGlobalConfig;
extern UPFGlobalConfig g_upfConfig;

extern void psMcsGetNcuLicCfg();
extern  void psMcsShowNcuLicense();
#ifdef __cplusplus
}
#endif

#endif 
