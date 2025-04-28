#ifndef PS_NEF_INTERFACE_H_
#define PS_NEF_INTERFACE_H_
#include "UpfNcuSynInfo.h"
#include "NFcodec/NupfEventExposure.mp.h"
#ifdef __cplusplus
extern "C" {
#endif
#include "tulip.h"
typedef struct
{
   CHAR   aucReportUri[AUCREPORTURI_MAXLEN];
   CHAR   aucCorid[AUCCORID_MAXLEN];
   QosAnalysisInfo tQosAnalysisInfo;
} T_NwdafInfoToUPM;
#ifdef __cplusplus
}
#endif


#endif
