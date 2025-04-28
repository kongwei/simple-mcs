PROJECT(UPF_NCU_APP)

SET(NCU_APP_PATH   ${NCU_BASE_PATH}/application)

INCLUDE_DIRECTORIES(${NCU_APP_PATH}/pktsolve/include)
INCLUDE_DIRECTORIES(${NCU_APP_PATH}/httplink/include)
INCLUDE_DIRECTORIES(${NCU_APP_PATH}/subscribe/include)
INCLUDE_DIRECTORIES(${NCU_APP_PATH}/report/include)
INCLUDE_DIRECTORIES(${NCU_APP_PATH}/trace/include)
INCLUDE_DIRECTORIES(${NCU_APP_PATH}/heart/include)
INCLUDE_DIRECTORIES(${NCU_APP_PATH}/UcomMsg/include)
INCLUDE_DIRECTORIES(${NCU_CTRL_PATH}/job/include)

AUX_SOURCE_DIRECTORY(${NCU_APP_PATH}/pktsolve/source           ncu_app_files)
AUX_SOURCE_DIRECTORY(${NCU_APP_PATH}/httplink/source            ncu_app_files)
AUX_SOURCE_DIRECTORY(${NCU_APP_PATH}/subscribe/source           ncu_app_files)
AUX_SOURCE_DIRECTORY(${NCU_APP_PATH}/report/source           ncu_app_files)
AUX_SOURCE_DIRECTORY(${NCU_APP_PATH}/trace/source           ncu_app_files)
AUX_SOURCE_DIRECTORY(${NCU_APP_PATH}/heart/source            ncu_app_files)
AUX_SOURCE_DIRECTORY(${NCU_APP_PATH}/UcomMsg/source            ncu_app_files)

SET (ncuapp_files "${ncuapp_files};${ncu_app_files}")

ADD_LIBRARY(upf_ncuapp STATIC ${ncuapp_files})
