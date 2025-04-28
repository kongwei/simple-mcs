#include "SimpleCfg.h"
#include "r_dasystemcfg.h"

class DataAnalysSysCfg :public SimpleCfgTemplate
{
    public:
     DataAnalysSysCfg(string database,string configName,string tablename,DWORD eventid):SimpleCfgTemplate(database,configName,tablename,eventid){};
      virtual void show();
      virtual void powerOnProc();
      virtual void cfgNotifyProc(WORD16 operation, WORD16 msgLen, BYTE *msg);
};
