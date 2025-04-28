#ifndef PS_GDMM_UPF_DEF_H_NCU
#define PS_GDMM_UPF_DEF_H_NCU

#include "tulip.h"
#define UPM_GDMM_PROTOBUF_MAX_LEN 2000
typedef struct
{

    BYTE abProtobuf[UPM_GDMM_PROTOBUF_MAX_LEN];
}T_psUpmGdmmPrivData;

typedef enum {
   NCUSCTYPE_ALL = 0,
   NCUSCTYPE_NEARCOMPUTING = 1,
}SCTYPE_NCUVCPU;

#define GDMM_UPF_NULL_CHECK_ACTION(x, ACTION)       if(unlikely((x) == NULL)) {ACTION; return;}
#define GDMM_UPF_VALID_CHECK_ACTION(x, ACTION)       if(unlikely(x)) {ACTION; return;}
#endif
