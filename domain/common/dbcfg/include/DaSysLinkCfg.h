#include "SimpleCfg.h"
#include "r_dalinkcfg.h"



class DaSysLinkCfg:public SimpleCfgTemplate
{
    public:
      DaSysLinkCfg(string database,string configName,string tablename,DWORD eventid):SimpleCfgTemplate(database,configName,tablename,eventid){};
      virtual void show();
      virtual void powerOnProc();
      virtual void cfgNotifyProc(WORD16 operation, WORD16 msgLen, BYTE *msg);
};

