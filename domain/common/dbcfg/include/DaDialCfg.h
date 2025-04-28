/* Started by AICoder, pid:1c4b93a777i732f1494509ebe0d4a1202b75bd5d */
/******************************************************************************
* 版权所有 (C)2024 深圳市中兴通讯股份有限公司*
* 模块名          : NCU
* 文件名          : DaDialCfg.h
* 相关文件        :
* 文件实现功能     : 数据分析拨测配置
* 归属团队        : M6
* 版本           : V7.24.40
-------------------------------------------------------------------------------
* 修改记录:
* 修改日期            版本号           修改人           修改内容
* 2024-10-10        V7.24.40        zjw            create
******************************************************************************/
#pragma once
#include "SimpleCfg.h"

class DaDialCfg:public SimpleCfgTemplate
{
public:
    DaDialCfg(string database,string configName,string tablename,DWORD eventid):SimpleCfgTemplate(database,configName,tablename,eventid){};
    virtual void show();
    virtual void powerOnProc();
    virtual void cfgNotifyProc(WORD16 operation, WORD16 msgLen, BYTE *msg);
};
// end of file
/* Ended by AICoder, pid:1c4b93a777i732f1494509ebe0d4a1202b75bd5d */
