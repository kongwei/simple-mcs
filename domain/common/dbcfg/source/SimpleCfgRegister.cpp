/*lint -e666 -e1773 -e837 -e528 -e1512*/
#include <string>
#include "SimpleCfg.h"
#include "SimpleCfgDef.h"
#include "psUpfJobTypes.h"
#include "psMcsGlobalCfg.h"
#include "SigTraceCfg.h"
#include "DataAnalysSysCfg.h"
// #include "DataNwdafCfg.h"
#include "DaUpfCfg.h"
#include "FlowAgeingTimeCfg.h"
#include "ResourceAgeCfg.h"
#include "DaSysLinkCfg.h"
#include "DaProfileCfg.h"
#include "SigTraceTaskCfg.h"
#include "psMcsLicenseCfg.h"
#include "NcuDangerousOpCfg.h"
#include "NcSystemLogCfg.h"
#include "ApplicationMapCfg.h"
#include "HttpProfileCfg.h"
#include "NcuSoftParaCfg.h"
#include "DaHttpCfg.h"
#include "FailTraceCfg.h"
#include "SubBindAppCfg.h"
#include "UpAssocAddrCfg.h"
#include "DaDialCfg.h"

UPFGlobalConfig g_upfConfig = {0};

extern "C" void SimpleCfgAddAllCfg()
{
    UPF_ADD_CFG(SigTraceCfg,                JOB_TYPE_MCS_MANAGE, "UpfNcu", TN_R_SIGTRACECFG,       EV_R_SIGTRACECFG_CHG);
    UPF_ADD_CFG(DataAnalysSysCfg,           JOB_TYPE_MCS_MANAGE, "UpfNcu", TN_R_DASYSTEMCFG,       EV_R_DASYSTEMCFG_CHG);
    UPF_ADD_CFG(DaUpfIpCfg,                 JOB_TYPE_MCS_MANAGE, "UpfNcu", TN_R_DAUPFCFG,          EV_R_DAUPFCFG_CHG);
    UPF_ADD_CFG(FlowAgeingTimeCfg,          JOB_TYPE_MCS_MANAGE, "UpfNcu", TN_NC_FLOWAGEINGTIME,   EV_NC_FLOWAGEINGTIME_CHG);
    UPF_ADD_CFG(ResourceAgeCfg,             JOB_TYPE_MCS_MANAGE, "UpfNcu", TN_R_NC_RESOURCEAGE,    EV_R_NC_RESOURCEAGE_CHG);
    UPF_ADD_CFG(DaSysLinkCfg,               JOB_TYPE_MCS_MANAGE, "UpfNcu", TN_R_DALINKCFG,         EV_R_DALINKCFG_CHG);
    UPF_ADD_CFG(DaProfileCfg,               JOB_TYPE_MCS_MANAGE, "UpfNcu", TN_R_DAPROFILE,         EV_R_DAPROFILE_CHG);
    UPF_ADD_CFG(SigTraceTaskCfg,            JOB_TYPE_MCS_MANAGE, "UpfNcu", TN_R_SIGTRACETASK,      EV_R_SIGTRACETASK_LONGCHG);
    UPF_ADD_CFG(NcuLicenseCfg,              JOB_TYPE_MCS_MANAGE, "UpfNcu", TN_R_DBMLICENSE,        EV_R_DBMLICENSE_CHG);
    UPF_ADD_CFG(Capacity,                   JOB_TYPE_MCS_MANAGE, "UpfNcu", TN_R_NC_CAPACITY,       EV_R_NC_CAPACITY_CHG);
    UPF_ADD_CFG(Threadsize,                 JOB_TYPE_MCS_MANAGE, "UpfNcu", TN_R_NC_THREADSIZE,     EV_R_NC_THREADSIZE_CHG);
    UPF_ADD_CFG(Threadinfo,                 JOB_TYPE_MCS_MANAGE, "UpfNcu", TN_R_NC_THREADINFO,     EV_R_NC_THREADINFO_CHG);
    UPF_ADD_CFG(DpdkCommCfg,                JOB_TYPE_MCS_MANAGE, "UpfNcu", TN_DPDKCOMMNFCFG,       EV_DPDKCOMMNFCFG_CHG);
    UPF_ADD_CFG(DpdkSpecialNfCfg,           JOB_TYPE_MCS_MANAGE, "UpfNcu", TN_DPDKSPECIALNFCFG,    EV_DPDKSPECIALNFCFG_CHG);
    UPF_ADD_CFG(NcSystemLogCfg,             JOB_TYPE_MCS_MANAGE, "UpfNcu", TN_R_NCSYSTEMLOGCFG,    EV_R_NCSYSTEMLOGCFG_CHG);
    UPF_ADD_CFG(ApplicationMapCfg,          JOB_TYPE_MCS_MANAGE, "UpfNcu", TN_APPLICATIONMAP,      EV_APPLICATIONMAP_CHG);
    UPF_ADD_CFG(NcHttpProfileCfg,           JOB_TYPE_MCS_MANAGE, "UpfNcu", TN_NC_HTTPPROFILECFG,   EV_NC_HTTPPROFILECFG_CHG);
    UPF_ADD_CFG(NcuSoftParaCfg,             JOB_TYPE_MCS_MANAGE, "UpfNcu", TN_DPIINNERSOFTPARA,    EV_DPIINNERSOFTPARA_CHG);
    UPF_ADD_CFG(DaHttpCfg,                  JOB_TYPE_MCS_MANAGE, "UpfNcu", TN_R_DAHTTPCFG,         EV_R_DAHTTPCFG_CHG);
    UPF_ADD_CFG(FailTraceCfg,               JOB_TYPE_MCS_MANAGE, "UpfNcu", TN_R_FAILTRACECFG,      EV_R_FAILTRACECFG_CHG);
    UPF_ADD_CFG(SubBindAppCfg,              JOB_TYPE_MCS_MANAGE, "UpfNcu", TN_R_SUBBINDAPP,        EV_R_SUBBINDAPP_CHG);
    UPF_ADD_CFG(UpAssocAddrCfg,             JOB_TYPE_MCS_MANAGE, "UpfNcu", TN_R_UPASSOCADDR,       EV_R_UPASSOCADDR_CHG);
    UPF_ADD_CFG(DaDialCfg,                  JOB_TYPE_MCS_MANAGE, "UpfNcu", TN_R_DADIALCFG,         EV_R_DADIALCFG_CHG);
}
