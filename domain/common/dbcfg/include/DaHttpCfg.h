#include "SimpleCfg.h"
#include "r_dahttpcfg.h"

class DaHttpCfg:public SimpleCfgTemplate
{
    public:
      DaHttpCfg(string database,string configName,string tablename,DWORD eventid):SimpleCfgTemplate(database,configName,tablename,eventid){};
      virtual void show();
      virtual void powerOnProc();
      virtual void cfgNotifyProc(WORD16 operation, WORD16 msgLen, BYTE *msg);
};
