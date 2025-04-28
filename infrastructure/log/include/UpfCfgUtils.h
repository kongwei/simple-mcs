#ifndef UPF_CFG_UTILS_H
#define UPF_CFG_UTILS_H

#ifdef __cplusplus
extern "C"{
#endif

#define CFG_LOG(level, ...)  \
    {\
        printf("[CFG][%s:%u]", __FUNCTION__, __LINE__);\
        printf(__VA_ARGS__);\
        printf("\n");\
    }while(0)

#define CFG_INFO(...)  CFG_LOG(1, __VA_ARGS__)
#define CFG_WARN(...)  CFG_LOG(2, __VA_ARGS__)
#define CFG_ERROR(...)  CFG_LOG(3, __VA_ARGS__)

#define PARAM_CHECK_RET(condition, ret) if(condition){return ret;}
#define PARAM_CHECK_RET_VOID(condition) if(condition){return;}
#define PARAM_CHECK_NULL_RET(ptr, ret) PARAM_CHECK_RET(NULL==(ptr), ret)
#define PARAM_CHECK_NULL_RET_FALSE(ptr) PARAM_CHECK_NULL_RET(ptr,FALSE)
#define PARAM_CHECK_NULLPTR_RET_4ARG(PTR1, PTR2, PTR3, PTR4, failRET)    \
    if(NULL == (PTR1) || NULL == (PTR2) || NULL == (PTR3) || NULL == (PTR4)) \
    {                                                             \
        return failRET;                                           \
    }


#ifdef __cplusplus
}
#endif

#endif
