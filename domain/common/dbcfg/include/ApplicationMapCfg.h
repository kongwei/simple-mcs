/* Started by AICoder, pid:y1f64e2e167ff821478608ae10e54e209685b322 */
/******************************************************************************
* 版权所有 (C)2016 深圳市中兴通讯股份有限公司*
* 模块名          : NCU
* 文件名          : ApplicationMapCfg.h
* 相关文件        :
* 文件实现功能     : applicationmap配置变更
* 归属团队        : M6
* 版本           : V1.0
-------------------------------------------------------------------------------
* 修改记录:
* 修改日期            版本号           修改人           修改内容
* 2024-05-11        V7.24.11.B5        WYA            create
******************************************************************************/

#pragma once
#include "SimpleCfg.h"

class ApplicationMapCfg:public SimpleCfgTemplate
{
public:
    ApplicationMapCfg(string database,string configName,string tablename,DWORD eventid):SimpleCfgTemplate(database,configName,tablename,eventid){};
    virtual void show();
    virtual void powerOnProc();
    virtual void cfgNotifyProc(WORD16 operation, WORD16 msgLen, BYTE *msg);
};
/* Ended by AICoder, pid:y1f64e2e167ff821478608ae10e54e209685b322 */
