
#include "tulip.h"
#include "ps_gdmm_ncu_entry.h"
#include "ps_define.h"
#include "ps_gdmm_upf_def.h"

extern VOID psUpfGdmm_StartupFunc();
extern VOID psUpfGdmm_MasterFunc(WORD32 msgId, VOID *msg, T_psUpmGdmmPrivData *ptGdmmPrivData);

VOID psUpfGdmmEntry(WORD16 wState, WORD32 dwMsgId, VOID *pMsgBody, VOID *pPData, BOOLEAN bSame)
{
    T_psUpmGdmmPrivData         *ptGdmmPrivData;
    if ((NULL == pPData)||(NULL == pMsgBody))
    {
        XOS_SetDefaultNextState();
        return;
    }

    ptGdmmPrivData = (T_psUpmGdmmPrivData *)XOS_GetSelfPriDataPtr();
    switch (wState)
    {
        case S_START_UP:
        {
            switch (dwMsgId)
            {
                case EV_MASTER_POWER_ON:
                case EV_SLAVE_POWER_ON:
                {
                    RTE_PER_LCORE(_lcore_id) = CSS_CTRL_THREAD_NO;
                    psUpfGdmm_StartupFunc();
                    XOS_PowerOnAck(EV_POWER_ON_SUCCESS, NULL, 0, 0);
                    XOS_SetNextState(S_MASTER);
                    break;
                }
                default:
                {
                    XOS_SetDefaultNextState();
                    break;
                }
            }

            break;
        }

        case S_MASTER:
        {
            psUpfGdmm_MasterFunc(dwMsgId, pMsgBody, ptGdmmPrivData);
            break;
        }

        default:
        {
            break;
        }
    }

    return;
}

