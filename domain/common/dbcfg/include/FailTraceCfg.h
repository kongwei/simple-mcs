/* Started by AICoder, pid:l4dab850c585dae1439a0b9880f5772cb1f61e53 */
/******************************************************************************
* 版权所有 (C)2016 深圳市中兴通讯股份有限公司*
* 模块名          : Ncu
* 文件名          : FailTraceCfg.h
* 相关文件        :
* 文件实现功能     : 失败观察配置
* 归属团队        : M6
* 版本           : V1.0
-------------------------------------------------------------------------------
* 修改记录:
* 修改日期            版本号           修改人           修改内容
* 2024-07-16        V7.24.30        zjw            create
******************************************************************************/

#include "SimpleCfg.h"
#include "psMcsGlobalCfg.h"
#include "r_failtracecfg.h"

class FailTraceCfg:public SimpleCfgTemplate
{
public:
    FailTraceCfg(string database,string configName,string tablename,DWORD eventid):SimpleCfgTemplate(database,configName,tablename,eventid){};
    virtual void show();
    virtual void powerOnProc();
    virtual void cfgNotifyProc(WORD16 operation, WORD16 msgLen, BYTE *msg);
};

extern T_UpfFailTraceCfg g_failtracecfg;
/* Ended by AICoder, pid:l4dab850c585dae1439a0b9880f5772cb1f61e53 */
