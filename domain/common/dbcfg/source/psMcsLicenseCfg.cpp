#include "psMcsLicenseCfg.h"
#include "dbmLibComm.h" //DBMGetLicenseValueById
#include "UPFLog.h"
#include "psMcsGlobalCfg.h"
#include "zte_slibc.h"
#include "UPFHelp.h"

WORD32 g_dwKeyServQaSwitch = 2;
WORD32 g_dwKeyUserDataColSwitch = 2;
WORD32 g_dwSampUserDataColSwitch = 2;
WORD32 g_dwKeyServQaCapacity = 0xffffffff;
WORD32 g_dwKeyUserDataColCapacity = 0xffffffff;
WORD32 g_dwSampUserDataColCapacity = 0xffffffff;

void psNcuMcsQryLicIDVal(const CHAR *dwLicID, WORD32 *pbLicVal)
{

    if(RC_DBM_OK == DBMGetLicenseValueById(dwLicID,pbLicVal))
    {
        UPF_TRACE_INFO("\n[LicenseValue] DBMGetLicense %s is success!\n", dwLicID);
        return;
    }
    zte_printf_s("\n[LicenseValue] DBMGetLicense %s is fail!\n", dwLicID);
    return;
}

void fillNcuLicenseSwitch(WORD32 *pdwSwitch, WORD32 dwOldSwitch, WORD32 dwNewSwitch)
{
    *pdwSwitch = (dwNewSwitch == 0 || dwNewSwitch == 1) ? dwNewSwitch : dwOldSwitch;
}

void fillNcuLicenseCapacity(WORD32 *pdwCapacity, WORD32 dwOldCapacity, WORD32 dwNewCapacity)
{
    *pdwCapacity = (dwNewCapacity == 0xffffffff) ? dwOldCapacity : dwNewCapacity;
}

void psMcsGetNcuLicCfg()
{
    WORD32 dwKeyServQaSwitch =0;
    WORD32 dwKeyUserDataColSwitch =0;
    WORD32 dwSampUserDataColSwitch=0;
    WORD32 dwKeyServQaCapacity =0;
    WORD32 dwKeyUserDataColCapacity =0;
    WORD32 dwSampUserDataColCapacity=0;
    psNcuMcsQryLicIDVal("NFS_Nupf_NearComputing_10000001", &dwKeyServQaSwitch);
    psNcuMcsQryLicIDVal("NFS_Nupf_NearComputing_10000002", &dwKeyUserDataColSwitch);
    psNcuMcsQryLicIDVal("NFS_Nupf_NearComputing_10000003", &dwSampUserDataColSwitch);
    psNcuMcsQryLicIDVal("NFS_Nupf_NearComputing_10000004", &dwKeyServQaCapacity);
    psNcuMcsQryLicIDVal("NFS_Nupf_NearComputing_10000005", &dwKeyUserDataColCapacity);
    psNcuMcsQryLicIDVal("NFS_Nupf_NearComputing_10000006", &dwSampUserDataColCapacity);
    fillNcuLicenseSwitch(&g_upfConfig.ncuLicenseCfg.dwKeyServQaSwitch, dwKeyServQaSwitch, g_dwKeyServQaSwitch);
    fillNcuLicenseSwitch(&g_upfConfig.ncuLicenseCfg.dwKeyUserDataColSwitch, dwKeyUserDataColSwitch, g_dwKeyUserDataColSwitch);
    fillNcuLicenseSwitch(&g_upfConfig.ncuLicenseCfg.dwSampUserDataColSwitch, dwSampUserDataColSwitch, g_dwSampUserDataColSwitch);
    fillNcuLicenseCapacity(&g_upfConfig.ncuLicenseCfg.dwKeyServQaCapacity, dwKeyServQaCapacity, g_dwKeyServQaCapacity);
    fillNcuLicenseCapacity(&g_upfConfig.ncuLicenseCfg.dwKeyUserDataColCapacity, dwKeyUserDataColCapacity, g_dwKeyUserDataColCapacity);
    fillNcuLicenseCapacity(&g_upfConfig.ncuLicenseCfg.dwSampUserDataColCapacity, dwSampUserDataColCapacity, g_dwSampUserDataColCapacity);
    g_upfConfig.ncuLicenseCfg.hasGetLicCfg = 1;

    return ;
}

 void psMcsShowNcuLicense()
{
    zte_printf_s("\n show Ncu All License!");
    zte_printf_s("\n NFS_Nupf_NearComputing_10000001            = %u", g_upfConfig.ncuLicenseCfg.dwKeyServQaSwitch);
    zte_printf_s("\n NFS_Nupf_NearComputing_10000002            = %u", g_upfConfig.ncuLicenseCfg.dwKeyUserDataColSwitch);
    zte_printf_s("\n NFS_Nupf_NearComputing_10000003            = %u", g_upfConfig.ncuLicenseCfg.dwSampUserDataColSwitch);
    zte_printf_s("\n NFS_Nupf_NearComputing_10000004            = %u", g_upfConfig.ncuLicenseCfg.dwKeyServQaCapacity);
    zte_printf_s("\n NFS_Nupf_NearComputing_10000005            = %u", g_upfConfig.ncuLicenseCfg.dwKeyUserDataColCapacity);
    zte_printf_s("\n NFS_Nupf_NearComputing_10000006            = %u", g_upfConfig.ncuLicenseCfg.dwSampUserDataColCapacity);
	return ;
}

extern "C"
{
    UPF_HELP_REG("ncu",
    "set ncusetnculicensecfg",
    void ncusetnculicensecfg(WORD32 dwKeyServQaSwitch, WORD32 dwKeyUserDataColSwitch, WORD32 dwSampUserDataColSwitch, WORD32 dwKeyServQaCapacity, WORD32 dwKeyUserDataColCapacity, WORD32 dwSampUserDataColCapacity))
    {
        g_dwKeyServQaSwitch         = dwKeyServQaSwitch;
        g_dwKeyUserDataColSwitch    = dwKeyUserDataColSwitch;
        g_dwSampUserDataColSwitch   = dwSampUserDataColSwitch;
        g_dwKeyServQaCapacity       = dwKeyServQaCapacity;
        g_dwKeyUserDataColCapacity  = dwKeyUserDataColCapacity;
        g_dwSampUserDataColCapacity = dwSampUserDataColCapacity;
        psMcsGetNcuLicCfg();
        return;
    }
}

void NcuLicenseCfg::powerOnProc()
{
  psMcsGetNcuLicCfg();
  return ; 
}

void NcuLicenseCfg::show()
{
    psMcsShowNcuLicense();
    return ;
}

void NcuLicenseCfg::cfgNotifyProc(WORD16 operation, WORD16 msgLen, BYTE *msg)
{
    if(msg == NULL)
    {
      return;
    }
    
    UPF_TRACE_INFO("\n[ConfigNotify]Receive LicenseNotifyProc process success!\n");
    switch(operation)
    {
        case _DB_CFGCHG_TUPLE_NOTIFY_INSERT:
            psMcsGetNcuLicCfg();
            UPF_TRACE_INFO("\n[ConfigNotify]recv insert LicenseNotifyProc msg!\n");
            break;
        case _DB_CFGCHG_TUPLE_NOTIFY_MODIFY:
            psMcsGetNcuLicCfg();
            UPF_TRACE_INFO("\n[ConfigNotify]recv modify LicenseNotifyProc msg!\n");
            break;
        case _DB_CFGCHG_TUPLE_NOTIFY_DELETE:
            psMcsGetNcuLicCfg();
            UPF_TRACE_INFO("\n[ConfigNotify]recv delete LicenseNotifyProc msg!\n");
            break;
        default:
            UPF_TRACE_INFO("\n[ConfigNotify]recv unknown LicenseNotifyProc msg!\n");
            break;
    }
    return;
}
