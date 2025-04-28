CMAKE_MINIMUM_REQUIRED(VERSION 3.24)
PROJECT(UPF_NCU_MCS)

SET(UPF_PATH        ${CMAKE_CURRENT_SOURCE_DIR}/../..)
SET(UPF_COMMON      ${UPF_PATH}/common)
SET(UPF_COMMON_CORE ${UPF_COMMON}/core)
SET(NCU_PATH        ${UPF_PATH}/ncu)
SET(NCU_COMMON      ${NCU_PATH}/common)
SET(NCU_MCS_PATH    ${NCU_PATH}/mcs)
SET(NGC_LIB         ${NCU_PATH}/3rdparty/lib)
SET(NGC_INCLUDE     ${NCU_PATH}/3rdparty/include)
SET(NCU_DOMAIN_PATH ${NCU_MCS_PATH}/domain)
SET(NCU_APP_PATH    ${NCU_MCS_PATH}/application)
SET(NCU_UI_PATH     ${NCU_MCS_PATH}/ui)
SET(NCU_INSFRA_PATH ${NCU_MCS_PATH}/infrastructure)
SET(NCU_CTRL_PATH   ${NCU_MCS_PATH}/ctrl)
SET(NCU_CTRL_GDMM_PATH ${NCU_CTRL_PATH}/gdmm)
SET(NCU_MDMM_PATH   ${NCU_PATH}/oammerge/mdmm)
SET(DDMCODEC_PATH   ${NCU_PATH}/3rdparty/tools/ncu_oam_model/ddmcodec)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(UPF_NCU_DEFINED TRUE)

option(CCACHE "Compile with ccache" ON)
if(CCACHE)
    find_program(CCACHE_TOOL ccache HINTS /usr/local/bin)
    if(CCACHE_TOOL)
        MESSAGE("CCACHE_TOOL:" ${CCACHE_TOOL})
        set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ${CCACHE_TOOL})
        set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ${CCACHE_TOOL})
    endif(CCACHE_TOOL)
endif(CCACHE)

ADD_DEFINITIONS(-D_UPF_NCU)

MESSAGE(STATUS "ncu ARCH_TYPE is ${ARCH_TYPE}")
# INCLUDE(${UPF_PATH}/project/${ARCH_TYPE}.cmake)
INCLUDE(${NCU_PATH}/build/project/${ARCH_TYPE}.cmake)

MESSAGE(STATUS "ncu NIC_TYPE is ${NIC_TYPE}")

ADD_DEFINITIONS(" -D_SERVICE_TYPE=_UPF_NCU -D_SVR_NAME=_SN_SERVICE_FWD -D_INUPF_")
MESSAGE(STATUS "CMS_TYPE=${CMS_TYPE}")
if (CMS_TYPE)
    ADD_DEFINITIONS(-D_UPF_BM_CAAS)
ENDIF(CMS_TYPE)
MACRO(source_group_by_dir source_files)
    SET(sgbd_cur_dir ${CMAKE_CURRENT_SOURCE_DIR})
    FOREACH(sgbd_file ${${source_files}})
        STRING(REGEX REPLACE ${sgbd_cur_dir}/\(.*\) \\1 sgbd_fpath ${sgbd_file})
        STRING(REGEX REPLACE "\(.*\)/.*" \\1 sgbd_group_name ${sgbd_fpath})
        STRING(COMPARE EQUAL ${sgbd_fpath} ${sgbd_group_name} sgbd_nogroup)
        IF(MSVC)
            string(REPLACE "/" "\\" sgbd_group_name ${sgbd_group_name})
        ENDIF(MSVC)
        IF(sgbd_nogroup)
            SET(sgbd_group_name "\\")
        ENDIF(sgbd_nogroup)
        SOURCE_GROUP(${sgbd_group_name} FILES ${sgbd_file})
    ENDFOREACH(sgbd_file)
ENDMACRO(source_group_by_dir)

#3rdparty
INCLUDE_DIRECTORIES(${NGC_INCLUDE})
INCLUDE_DIRECTORIES(${NGC_INCLUDE}/cjson)
INCLUDE_DIRECTORIES(${NGC_INCLUDE}/DPDK/dpdk_adapt)
INCLUDE_DIRECTORIES(${NGC_INCLUDE}/DPDK/dpdk)
INCLUDE_DIRECTORIES(${NGC_INCLUDE}/json)
INCLUDE_DIRECTORIES(${NGC_INCLUDE}/V6PLAT/dbm)
INCLUDE_DIRECTORIES(${NGC_INCLUDE}/ZXDPI)
INCLUDE_DIRECTORIES(${NGC_INCLUDE}/V6PLAT/tulip)
INCLUDE_DIRECTORIES(${NGC_INCLUDE}/V6PLAT/tulip/oss)
INCLUDE_DIRECTORIES(${NGC_INCLUDE}/V6PLAT/tulip/scs)
INCLUDE_DIRECTORIES(${NGC_INCLUDE}/ddm)

#ncu Common
# INCLUDE_DIRECTORIES(${NCU_COMMON}/thdm/include)
# INCLUDE_DIRECTORIES(${NCU_COMMON}/thread/include)
# INCLUDE_DIRECTORIES(${NCU_COMMON}/env)
INCLUDE_DIRECTORIES(${NCU_COMMON}/xdb/include)
# INCLUDE_DIRECTORIES(${NCU_COMMON}/oam/alarm/include)
# INCLUDE_DIRECTORIES(${NCU_COMMON}/logHelp/include)
INCLUDE_DIRECTORIES(${NCU_COMMON}/cfgFrame/include)
# INCLUDE_DIRECTORIES(${NCU_COMMON}/pub)
# INCLUDE_DIRECTORIES(${NCU_COMMON}/headcap)
INCLUDE_DIRECTORIES(${NCU_COMMON}/trace)
INCLUDE_DIRECTORIES(${NCU_COMMON}/oam/pm/smgencode)
INCLUDE_DIRECTORIES(${NCU_COMMON}/oam/pm)
INCLUDE_DIRECTORIES(${NCU_COMMON}/oam/alarm/include)
INCLUDE_DIRECTORIES(${NCU_MDMM_PATH}/include)
INCLUDE_DIRECTORIES(${NCU_MDMM_PATH}/common/include)
INCLUDE_DIRECTORIES(${NCU_COMMON}/pub2)

#Common
INCLUDE_DIRECTORIES(${UPF_COMMON_CORE}/thread/entry/include)
INCLUDE_DIRECTORIES(${UPF_COMMON_CORE}/pub)
INCLUDE_DIRECTORIES(${UPF_COMMON_CORE}/com)
INCLUDE_DIRECTORIES(${UPF_COMMON_CORE}/env)
INCLUDE_DIRECTORIES(${UPF_COMMON_CORE}/headcap)
INCLUDE_DIRECTORIES(${UPF_COMMON_CORE}/thdm/include)
INCLUDE_DIRECTORIES(${UPF_COMMON_CORE}/cfg)
INCLUDE_DIRECTORIES(${UPF_COMMON_CORE}/oam/alarm/include)

if(ARCH_TYPE STREQUAL ARM)
INCLUDE_DIRECTORIES(${NGC_INCLUDE}/V6PLAT/zte_slibc/arm/aarch64_fp_el/usr/include)
else()
INCLUDE_DIRECTORIES(${NGC_INCLUDE}/V6PLAT/zte_slibc/x86/x86_64_el/usr/include)
endif()

#3rdparty
INCLUDE_DIRECTORIES(${NGC_INCLUDE})
INCLUDE_DIRECTORIES(${NGC_INCLUDE}/pfcp)
INCLUDE_DIRECTORIES(${NGC_INCLUDE}/dsl)
INCLUDE_DIRECTORIES(${NGC_INCLUDE}/ncu_oam_model)
INCLUDE_DIRECTORIES(${NGC_INCLUDE}/USYNC)
INCLUDE_DIRECTORIES(${NGC_INCLUDE}/UPF_Csslib)
INCLUDE_DIRECTORIES(${NGC_INCLUDE}/SCCU_Client)
INCLUDE_DIRECTORIES(${NGC_INCLUDE}/SCCU_Server)
INCLUDE_DIRECTORIES(${NGC_INCLUDE}/SCR_Client)
INCLUDE_DIRECTORIES(${NGC_INCLUDE}/VNFP_Pub)
INCLUDE_DIRECTORIES(${NGC_INCLUDE}/pub)
INCLUDE_DIRECTORIES(${NGC_INCLUDE}/V6PLAT/include)
INCLUDE_DIRECTORIES(${NGC_INCLUDE}/V6PLAT/dbs)
INCLUDE_DIRECTORIES(${NGC_INCLUDE}/V6PLAT/nff)
INCLUDE_DIRECTORIES(${NGC_INCLUDE}/V6PLAT/oam)
INCLUDE_DIRECTORIES(${NGC_INCLUDE}/V6PLAT/brs)
INCLUDE_DIRECTORIES(${NGC_INCLUDE}/V6PLAT/3rdparty/openssl)
INCLUDE_DIRECTORIES(${NGC_INCLUDE}/pugixml)
INCLUDE_DIRECTORIES(${NGC_INCLUDE}/LTM)
INCLUDE_DIRECTORIES(${NGC_INCLUDE}/ipm)
INCLUDE_DIRECTORIES(${NGC_INCLUDE}/LogAgent)
INCLUDE_DIRECTORIES(${NGC_INCLUDE}/ProtoBuf)
INCLUDE_DIRECTORIES(${NGC_INCLUDE}/HTTP_LB)
INCLUDE_DIRECTORIES(${NGC_INCLUDE}/HTTP_LB/sbigw )
INCLUDE_DIRECTORIES(${NGC_INCLUDE}/HTTP_LB/pbinc)
INCLUDE_DIRECTORIES(${NGC_INCLUDE}/TSNCodec)
INCLUDE_DIRECTORIES(${NGC_INCLUDE}/DAS)

INCLUDE(Include.cmake)

if (MAKE_PCLINT)
    get_property(inc_dirs DIRECTORY  PROPERTY INCLUDE_DIRECTORIES)
endif (MAKE_PCLINT)

ADD_SUBDIRECTORY(${UPF_COMMON_CORE}/com         ${NCU_PATH}/output/com)
ADD_SUBDIRECTORY(${UPF_COMMON_CORE}/thdm         ${NCU_PATH}/output/thdm)
ADD_SUBDIRECTORY(${UPF_COMMON_CORE}/thread       ${NCU_PATH}/output/thread)
ADD_SUBDIRECTORY(${UPF_COMMON_CORE}/env       ${NCU_PATH}/output/env)
ADD_SUBDIRECTORY(${UPF_COMMON_CORE}/headcap       ${NCU_PATH}/output/headcap)
# ADD_SUBDIRECTORY(${UPF_COMMON_CORE}/cfg       ${NCU_PATH}/output/ncu_sc/cfg)
# ADD_SUBDIRECTORY(${UPF_COMMON_CORE}/oam       ${NCU_PATH}/output/ncu_sc/oam)

# ADD_SUBDIRECTORY(${NCU_COMMON}/thdm         ${NCU_PATH}/output/thdm)
# ADD_SUBDIRECTORY(${NCU_COMMON}/thread       ${NCU_PATH}/output/thread)
# ADD_SUBDIRECTORY(${NCU_COMMON}/env       ${NCU_PATH}/output/env)
ADD_SUBDIRECTORY(${NCU_COMMON}/xdb       ${NCU_PATH}/output/xdb)
ADD_SUBDIRECTORY(${NCU_COMMON}/oam       ${NCU_PATH}/output/oam)
#ADD_SUBDIRECTORY(${NCU_COMMON}/logHelp       ${NCU_PATH}/output/logHelp)
ADD_SUBDIRECTORY(${NCU_COMMON}/cfgFrame       ${NCU_PATH}/output/ncu_sc/cfg)
ADD_SUBDIRECTORY(${NCU_COMMON}/trace       ${NCU_PATH}/output/trace)
ADD_SUBDIRECTORY(${NCU_APP_PATH}           ${NCU_PATH}/output/ncu_sc/application)
ADD_SUBDIRECTORY(${NCU_DOMAIN_PATH}           ${NCU_PATH}/output/ncu_sc/domain)
ADD_SUBDIRECTORY(${NCU_UI_PATH}           ${NCU_PATH}/output/ncu_sc/ui)
ADD_SUBDIRECTORY(${NCU_INSFRA_PATH}       ${NCU_PATH}/output/ncu_sc/infrastructure)
ADD_SUBDIRECTORY(${NCU_CTRL_PATH}         ${NCU_PATH}/output/ncu_sc/ctrl)
LIST (APPEND all_files ${NCU_MCS_PATH}/entry/main.c)
source_group_by_dir(all_files)
LIST(APPEND all_files ${NCU_PATH}/script/upf_plat_weakfun.c)
LINK_DIRECTORIES(${NGC_LIB})
#LINK_DIRECTORIES(${UPF_COMMON}/opensource/cryptoex_lib/lib)
#LINK_DIRECTORIES(${UPF_COMMON}/dsl/generate/build)

# 添加gdmm包含目录
INCLUDE_DIRECTORIES(${NCU_CTRL_GDMM_PATH}/include)

IF(MAKE_KLOCWORK)
  ADD_LIBRARY(objs ${all_files})
ELSE(MAKE_KLOCWORK)
  SET(TARGET_NAME "UPF_NCU_${ARCH_TYPE}.exe")
  ADD_EXECUTABLE(${TARGET_NAME} ${all_files})

if(ARCH_TYPE STREQUAL ARM)
  TARGET_LINK_LIBRARIES(${TARGET_NAME}
    -Wl,-z,noexecstack
    -Wl,--start-group
    upf_ncuapp upf_ncu_ui upf_ncu_insfra upf_ncu_ctrl upfthdm pfuthread  upfenv ncudb ncutrace ncucfg upfcom upfheadcap STATIC_dsl
    xDPI_AARCH64_SOFT kmsclient css pzdb_intf_job smf_disp fm dpa dpdk-cryptops dpp_arm_aarch64_lit_64_rel pcap das_agent 
    json_linux-gcc-4.8_libmt sync_hcdb log_agent boost_thread ncu_rs
    nrfclient_proto.a httppub.a pub_proto.a httplink.a nfcodec.a nfcodec_nrf.a nfcodec_base.a nghttp2codec.a nghttp2.a
    oamagent  tt_oamagent dbm "-Wl,--whole-archive" upf_ncudomain dbconfig commcfg ncuoam "-Wl,--no-whole-archive"
    xml2pdll z ssl ssh2 sccu_client cjson
    sync_hcdb common_job common_task common_all usyncpbcodec usync ulog udb ipmproto protobuf-lite protobuf vnfp_pub crypto dl usksdpdk dpdktipc uskcdpdk dpdktipcdispatch pthread crypt pugixml  rt util protobuf
    -Wl,--end-group
    ${tsnClk_lib} _tipc_dll _adm_dll _alarm_dll _com_dll zte_slibc _dbms _dbscoredll _tulip_dll _alarm_dll
    _tool_telnet_dll _nfs_dll _tool_ftp_dll _tool_tftp_dll _hot_patch_dll _dlib_dll
    numa mlx5 -lnl-3 -lnl-route-3 -libverbs -rdynamic -luskc scs elf
    vnet vppinit brssocket svm svmdb vlib vlibmemory vppinfra vlibmemoryclientbuiltin das_agent das_proxy das_agt_comm
  )
else()
    if(NIC_TYPE STREQUAL IAASMLX43)
        TARGET_LINK_LIBRARIES(${TARGET_NAME}
          -Wl,-z,noexecstack
          -Wl,--start-group
          upf_ncuapp upf_ncu_ui upf_ncu_insfra upf_ncu_ctrl upfthdm pfuthread  upfenv ncudb ncutrace ncucfg upfcom upfheadcap pcrez STATIC_dsl
          xDPI_X86_64_SOFT kmsclient css pzdb_intf_job smf_disp fm dpa dpdk-cryptops dpp_x86_64_lit_64_rel pcap das_agent
          json_linux-gcc-4.8_libmt sync_hcdb log_agent boost_thread ncu_rs
          nrfclient_proto.a httppub.a pub_proto.a httplink.a nfcodec.a nfcodec_nrf.a nfcodec_base.a nghttp2codec.a nghttp2.a
          oamagent  tt_oamagent dbm "-Wl,--whole-archive" upf_ncudomain ligservice dbconfig commcfg ncuoam "-Wl,--no-whole-archive"
          xml2pdll z ssl ssh2 sccu_client libcjson.a
          sync_hcdb common_job common_task common_all usyncpbcodec usync ulog udb ipmproto
          protobuf-lite protobuf vnfp_pub crypto dl usksdpdk dpdktipc uskcdpdk dpdktipcdispatch pthread crypt pugixml  rt util protobuf
          -Wl,--end-group
          ${tsnClk_lib} _tipc_dll _adm_dll _alarm_dll _com_dll ssp zte_slibc _dbms _dbscoredll _tulip_dll _alarm_dll
          _tool_telnet_dll _nfs_dll _tool_ftp_dll _tool_tftp_dll _hot_patch_dll _dlib_dll
          mnl numa mlx4 mlx5 -lnl-3 -lnl-route-3 -libverbs -rdynamic -luskc scs elf
          vnet vppinit brssocket svm svmdb vlib vlibmemory vppinfra vlibmemoryclientbuiltin das_agent das_proxy das_agt_comm
        )
    else()
        TARGET_LINK_LIBRARIES(${TARGET_NAME}
          -Wl,-z,noexecstack
          -Wl,--start-group
          STATIC_dsl upf_ncuapp upf_ncu_ui upf_ncu_insfra upf_ncu_ctrl upfthdm  pfuthread upfenv ncudb ncutrace ncucfg upfcom upfheadcap 
          xDPI_X86_64_SOFT kmsclient css pzdb_intf_job smf_disp fm dpa dpdk-cryptops dpp_x86_64_lit_64_rel pcap das_agent
          json_linux-gcc-4.8_libmt sync_hcdb log_agent boost_thread ncu_rs
          nrfclient_proto.a httppub.a pub_proto.a httplink.a nfcodec.a nfcodec_nrf.a nfcodec_base.a nghttp2codec.a nghttp2.a
          oamagent  tt_oamagent dbm "-Wl,--whole-archive" upf_ncudomain dbconfig commcfg ncuoam "-Wl,--no-whole-archive"
          xml2pdll z ssl ssh2 sccu_client libcjson.a
          sync_hcdb common_job common_task common_all usyncpbcodec usync ulog udb ipmproto
          protobuf-lite protobuf vnfp_pub crypto dl usksdpdk dpdktipc uskcdpdk dpdktipcdispatch pthread crypt pugixml  rt util protobuf
          -Wl,--end-group
          ${tsnClk_lib} _tipc_dll _adm_dll _alarm_dll _com_dll ssp zte_slibc _dbms _dbscoredll _tulip_dll _alarm_dll
          _tool_telnet_dll _nfs_dll _tool_ftp_dll _tool_tftp_dll _hot_patch_dll _dlib_dll
          mnl numa mlx5 -lnl-3 -lnl-route-3 -libverbs -rdynamic -luskc scs elf
          vnet vppinit brssocket svm svmdb vlib vlibmemory vppinfra vlibmemoryclientbuiltin das_agent das_proxy das_agt_comm
        )
    endif()
endif()
  SET(TARGET_PATH ${NCU_PATH}/output/ncu_sc/bin)
  INSTALL(CODE "MESSAGE(\"install ${TARGET_NAME} to ${TARGET_PATH}\")")
  INSTALL(TARGETS ${TARGET_NAME} RUNTIME DESTINATION ${TARGET_PATH})
  INCLUDE(Install.cmake)
ENDIF(MAKE_KLOCWORK)
