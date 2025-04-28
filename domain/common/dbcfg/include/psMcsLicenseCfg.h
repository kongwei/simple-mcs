#ifndef PS_MCS_LICENSE_CFG_H_H
#define PS_MCS_LICENSE_CFG_H_H


#include "SimpleCfg.h"
#include "dbmLibComm.h"




class NcuLicenseCfg :public SimpleCfgTemplate
{
    public:
      NcuLicenseCfg(string database,string configName,string tablename,DWORD eventid):SimpleCfgTemplate(database,configName,tablename,eventid){};
      virtual void show();
      virtual void powerOnProc();
      virtual void cfgNotifyProc(WORD16 operation, WORD16 msgLen, BYTE *msg);
};

#endif
