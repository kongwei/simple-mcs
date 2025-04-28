#include "SimpleCfg.h"
#include "r_sigtracecfg.h"
#include "psMcsGlobalCfg.h"



class SigTraceCfg:public SimpleCfgTemplate
{
    public:
     SigTraceCfg(string database,string configName,string tablename,DWORD eventid):SimpleCfgTemplate(database,configName,tablename,eventid){};
      virtual void show();
      virtual void powerOnProc();
      virtual void cfgNotifyProc(WORD16 operation, WORD16 msgLen, BYTE *msg);
};

extern T_UpfSigTraceCfg g_sigtracecfg;
