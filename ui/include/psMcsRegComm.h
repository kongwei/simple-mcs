#ifndef _PS_MCS_REGCOMMON_H_
#define _PS_MCS_REGCOMMON_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "ps_mcs_define.h" //T_MediaProcThreadPara


/*表驱动提升性能相关*/
#define MES_NUM_MAX 65536 /*定义最大个数*/

/*表驱动-函数指针定义*/
typedef VOID (*pfType_psGwVpfuMcsPktProc)(T_MediaProcThreadPara *ptMediaProcThreadPara);

/*表驱动-结构定义*/
typedef struct
{
    WORD32  dwUsedFlg; /*是否可用*/
    WORD32  dwMsgType;
    pfType_psGwVpfuMcsPktProc psGwVpfuMcsProc; /*函数指针*/
}T_GwVpfuMcsRegFunc;


/*int Reg fun*/
void psGwVpfuMcsIntfRegMsg(WORD16 wPktAttr, pfType_psGwVpfuMcsPktProc    psGwVpfuMcsProc);

/*reg */
#define MF_Mcs_RegMsg(wPktAttr,pFunc )      \
    static void MF_Mcs_RegMsg##wPktAttr##pFunc() \
    __attribute__((constructor)); \
    static void MF_Mcs_RegMsg##wPktAttr##pFunc() { \
        psGwVpfuMcsIntfRegMsg(wPktAttr, pFunc); \
        }

        /*get address for function*/
inline pfType_psGwVpfuMcsPktProc psGwVpfuMcsGetRegFuc(WORD16 wPktAttr);

#ifdef __cplusplus
}
#endif
#endif
/*lint +e528*/
