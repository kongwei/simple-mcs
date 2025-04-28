#pragma once
#include "SimpleCfg.h"
#include "dpiinnersoftpara.h"

#define USE_DM_UPFCOMM_GETALLDPIINNERSOFTPARA
#include "dbm_lib_upfcomm.h"

class NcuSoftParaCfg:public SimpleCfgTemplate
{
public:
    NcuSoftParaCfg(string database,string configName,string tablename,DWORD eventid):SimpleCfgTemplate(database,configName,tablename,eventid){};
    virtual void show();
    virtual void powerOnProc();
    virtual void cfgNotifyProc(WORD16 operation, WORD16 msgLen, BYTE *msg);

private:
    WORD32 getVal(DPIINNERSOFTPARA_TUPLE &tuple);
    WORD32 getVal(DM_UPFCOMM_GETALLDPIINNERSOFTPARA_ACK &ack, WORD32 i);
    void setConfig(WORD32 id, WORD32 val);
    void pfuInitSoftPara(void);
    void pfuGetSoftPara(void);
};


