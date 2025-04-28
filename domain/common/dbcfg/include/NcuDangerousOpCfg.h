#pragma once
#include "SimpleCfg.h"
#include "r_nc_capacity.h"
#include "r_nc_threadsize.h"
#include "r_nc_threadinfo.h"
#include "dpdkspecialnfcfg.h"
#include "dpdkcommnfcfg.h"

#define PS_DB_TRUE_CHECK_RETNONE(x)\
    if( x )\
    {\
        return ;\
    }


class Capacity:public SimpleCfgTemplate
{
    public:
      Capacity(string database,string configName,string tablename,DWORD eventid):SimpleCfgTemplate(database,configName,tablename,eventid){};
      virtual void show();
      virtual void powerOnProc();
      virtual void cfgNotifyProc(WORD16 operation, WORD16 msgLen, BYTE *msg);
};


class Threadsize:public SimpleCfgTemplate
{
    public:
      Threadsize(string database,string configName,string tablename,DWORD eventid):SimpleCfgTemplate(database,configName,tablename,eventid){};
      virtual void show();
      virtual void powerOnProc();
      virtual void cfgNotifyProc(WORD16 operation, WORD16 msgLen, BYTE *msg);
};

class Threadinfo:public SimpleCfgTemplate
{
    public:
      Threadinfo(string database,string configName,string tablename,DWORD eventid):SimpleCfgTemplate(database,configName,tablename,eventid){};
      virtual void show();
      virtual void powerOnProc();
      virtual void cfgNotifyProc(WORD16 operation, WORD16 msgLen, BYTE *msg);
};

class DpdkSpecialNfCfg:public SimpleCfgTemplate
{
    public:
      DpdkSpecialNfCfg(string database,string configName,string tablename,DWORD eventid):SimpleCfgTemplate(database,configName,tablename,eventid){};
      virtual void show();
      virtual void powerOnProc();
      virtual void cfgNotifyProc(WORD16 operation, WORD16 msgLen, BYTE *msg);
};

class DpdkCommCfg:public SimpleCfgTemplate
{
    public:
      DpdkCommCfg(string database,string configName,string tablename,DWORD eventid):SimpleCfgTemplate(database,configName,tablename,eventid){};
      virtual void show();
      virtual void powerOnProc();
      virtual void cfgNotifyProc(WORD16 operation, WORD16 msgLen, BYTE *msg);
};
