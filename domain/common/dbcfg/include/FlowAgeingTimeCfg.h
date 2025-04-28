#include "SimpleCfg.h"
#include "nc_flowageingtime.h"


class FlowAgeingTimeCfg:public SimpleCfgTemplate
{
    public:
     FlowAgeingTimeCfg(string database,string configName,string tablename,DWORD eventid):SimpleCfgTemplate(database,configName,tablename,eventid){};
      virtual void show();
      virtual void powerOnProc();
      virtual void cfgNotifyProc(WORD16 operation, WORD16 msgLen, BYTE *msg);
};
