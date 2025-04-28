/* Started by AICoder, pid:h13bdx500dwe72d14161080af017c62921259c75 */
/******************************************************************************
* 版权所有 (C)2016 深圳市中兴通讯股份有限公司*
* 模块名          : SubBindAppCfg
* 文件名          : SubBindAppCfg.h
* 相关文件        :
* 文件实现功能     : 子绑定应用程序
* 归属团队        : M6
* 版本           : V7.24.30.B11
-------------------------------------------------------------------------------
* 修改记录:
* 修改日期            版本号           修改人           修改内容
* 2024-08-22        V7.24.30.B11       wya            create
******************************************************************************/
#pragma once
#include "SimpleCfg.h"

class  SubBindAppCfg:public SimpleCfgTemplate
{
public:
    SubBindAppCfg(string database,string configName,string tablename,DWORD eventid):SimpleCfgTemplate(database,configName,tablename,eventid){};
    virtual void show();
    virtual void powerOnProc();
    virtual void cfgNotifyProc(WORD16 operation, WORD16 msgLen, BYTE *msg);
};
// end of file
/* Ended by AICoder, pid:h13bdx500dwe72d14161080af017c62921259c75 */
