PROJECT(UPF_NCU_DOMAIN)

SET(NCU_DOMAIN_COMMON_PATH   ${NCU_DOMAIN_PATH}/common)
SET(NCU_DOMAIN_CFG_PATH   ${NCU_DOMAIN_COMMON_PATH}/dbcfg)
SET(NCU_DOMAIN_MEMCFG_PATH ${NCU_DOMAIN_COMMON_PATH}/memcfg)
SET(NCU_DOMAIN_DBAPP_PATH ${NCU_DOMAIN_COMMON_PATH}/dbapp)

#dbcfg
INCLUDE_DIRECTORIES(${NCU_DOMAIN_COMMON_PATH}/dbcfg/include)
#memcfg
INCLUDE_DIRECTORIES(${NCU_DOMAIN_COMMON_PATH}/memcfg/include)
#dbapp
INCLUDE_DIRECTORIES(${NCU_DOMAIN_COMMON_PATH}/dbapp/entryFunc/include)
INCLUDE_DIRECTORIES(${NCU_DOMAIN_COMMON_PATH}/dbapp/commTable/include)
INCLUDE_DIRECTORIES(${NCU_DOMAIN_COMMON_PATH}/dbapp/mcsTable/include)
INCLUDE_DIRECTORIES(${NCU_DOMAIN_COMMON_PATH}/dbapp/dbaccess/include)
#pub
INCLUDE_DIRECTORIES(${NCU_DOMAIN_COMMON_PATH}/pub)
#sccuclient
INCLUDE_DIRECTORIES(${NCU_DOMAIN_COMMON_PATH}/sccuclient/include)
#pm
INCLUDE_DIRECTORIES(${NCU_DOMAIN_COMMON_PATH}/pmReport/include)

AUX_SOURCE_DIRECTORY(${NCU_DOMAIN_CFG_PATH}/source           ncudomain_cfg_files)
AUX_SOURCE_DIRECTORY(${NCU_DOMAIN_MEMCFG_PATH}/source           ncudomain_cfg_files)
AUX_SOURCE_DIRECTORY(${NCU_DOMAIN_DBAPP_PATH}/entryFunc/source           ncudomain_cfg_files)
AUX_SOURCE_DIRECTORY(${NCU_DOMAIN_DBAPP_PATH}/commTable/source           ncudomain_cfg_files)
AUX_SOURCE_DIRECTORY(${NCU_DOMAIN_DBAPP_PATH}/mcsTable/source           ncudomain_cfg_files)
AUX_SOURCE_DIRECTORY(${NCU_DOMAIN_DBAPP_PATH}/dbaccess/source           ncudomain_cfg_files)
AUX_SOURCE_DIRECTORY(${NCU_DOMAIN_COMMON_PATH}/sccuclient/source           ncudomain_cfg_files)
AUX_SOURCE_DIRECTORY(${NCU_DOMAIN_COMMON_PATH}/pmReport/source           ncudomain_cfg_files)

SET (ncudomain_files "${ncudomain_files};${ncudomain_cfg_files}" PARENT_SCOPE)
