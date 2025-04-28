PROJECT(UPF_NCU_INFRA_STRUCTURE)

SET(NCU_INSFRA  ${NCU_BASE_PATH}/infrastructure)
SET(NCU_INSFRA_LOG   ${NCU_INSFRA}/log)

#log
INCLUDE_DIRECTORIES(${NCU_INSFRA}/log/include)
AUX_SOURCE_DIRECTORY(${NCU_INSFRA}/log/source           ncuapp_insfra_files)

ADD_LIBRARY(upf_ncu_insfra STATIC ${ncuapp_insfra_files})
