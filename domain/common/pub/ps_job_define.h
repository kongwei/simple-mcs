#ifndef PS_JOB_DEFINE_H_
#define PS_JOB_DEFINE_H_

#ifdef __cplusplus
extern "C" {
#endif
#include "tulip.h"

/*JOB处理返回值*/
#define JOB_RET_SUCCESS         (WORD32)(0) /*成功*/
#define JOB_RET_CONTINUE        (WORD32)(1) /*继续当前流程*/
#define JOB_RET_BREAK           (WORD32)(2) /*中断当前流程*/
#define JOB_RET_FAIL            (WORD32)(0xffffffff) /*失败*/

#ifndef S_POWER_OFF
#define S_POWER_OFF (WORD16)(S_StartUp+4)
#endif



#define JOB_CHECK_NULL_RET(x, y) \
{\
    if (NULL == (x))\
    {\
        return y;\
    }\
}

#define JOB_CHECK_FALSE_RET(x, y) \
{\
    if (!(x))\
    {\
        return y;\
    }\
}


#ifdef __cplusplus
}
#endif

#endif
