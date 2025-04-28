PROJECT(UPF_NCU_UTILS)

SET(NCU_DOMAIN_UTILS_PATH   ${NCU_DOMAIN_PATH}/utils)

SET(NCU_UTILS_ROUTE_PATH ${NCU_DOMAIN_UTILS_PATH}/route)

#route
INCLUDE_DIRECTORIES(${NCU_UTILS_ROUTE_PATH}/include)

AUX_SOURCE_DIRECTORY(${NCU_UTILS_ROUTE_PATH}/source           ncudomain_utils_files)

SET (ncudomain_files "${ncudomain_files};${ncudomain_utils_files}" PARENT_SCOPE)
