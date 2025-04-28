#include "psMcsRegComm.h"

/*全局协议消息入口变量*/
T_GwVpfuMcsRegFunc g_atGwVpfuMcsRegFunc[MES_NUM_MAX] = {{0}};

/*协议入口函数注册*/
void psGwVpfuMcsIntfRegMsg(WORD16 wPktAttr, pfType_psGwVpfuMcsPktProc psGwVpfuMcsProc)
{
    g_atGwVpfuMcsRegFunc[wPktAttr].dwUsedFlg = 1;
    g_atGwVpfuMcsRegFunc[wPktAttr].dwMsgType = (WORD32)wPktAttr;
    g_atGwVpfuMcsRegFunc[wPktAttr].psGwVpfuMcsProc = psGwVpfuMcsProc;
}

/*协议入口函数返回指针*/
inline pfType_psGwVpfuMcsPktProc psGwVpfuMcsGetRegFuc(WORD16 wPktAttr)
{
    return g_atGwVpfuMcsRegFunc[wPktAttr].psGwVpfuMcsProc;
}
/*lint -e559*/
void psGwVpfuMcsShowRegInfo()
{
    WORD32 dwNum = 0;
    WORD32 i = 0;

    printf("/*NORMAL ATTR*/ \n");
    for(i = 0; i < MES_NUM_MAX; i++)
    {
        if(/*(g_atGwVpfuMcsRegFunc[i].dwUsedFlg == 1)||(*/NULL != g_atGwVpfuMcsRegFunc[i].psGwVpfuMcsProc/*)*/)
        {
            printf("ATTR = %u ,FUC = %p\n", g_atGwVpfuMcsRegFunc[i].dwMsgType, g_atGwVpfuMcsRegFunc[i].psGwVpfuMcsProc);
            dwNum++;
        }
        
    }
    printf("NUMBER = %u \n",dwNum);
    printf("/***********/ \n");
}
/*lint +e559*/


