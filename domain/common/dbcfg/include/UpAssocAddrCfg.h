/* Started by AICoder, pid:96337350a4g4b7614f9e0a831019712c8bb5180e */
/******************************************************************************
* 版权所有 (C)2016 深圳市中兴通讯股份有限公司*
* 模块名          : UpAssocAddrCfg
* 文件名          : UpAssocAddrCfg.h
* 相关文件        :
* 文件实现功能     : UpAssocAddrCfg配置
* 归属团队        : M6
* 版本           : V7.24.40
-------------------------------------------------------------------------------
* 修改记录:
* 修改日期            版本号           修改人           修改内容
*                             create
******************************************************************************/

#pragma once
#include "SimpleCfg.h"

class UpAssocAddrCfg : public SimpleCfgTemplate
{
public:
    UpAssocAddrCfg(string database, string configName, string tablename, DWORD eventid) : SimpleCfgTemplate(database, configName, tablename, eventid) {};
    virtual void show();
    virtual void powerOnProc();
    virtual void cfgNotifyProc(WORD16 operation, WORD16 msgLen, BYTE *msg);
};
/* Ended by AICoder, pid:96337350a4g4b7614f9e0a831019712c8bb5180e */
