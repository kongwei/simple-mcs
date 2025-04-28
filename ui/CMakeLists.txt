PROJECT(UPF_NCU_UI)

SET(NCU_UI_PATH   ${NCU_BASE_PATH}/ui)

INCLUDE_DIRECTORIES(${NCU_UI_PATH}/include)
AUX_SOURCE_DIRECTORY(${NCU_UI_PATH}/source           ncuapp_ui_files)

SET (ncu_ui_files "${ncu_ui_files};${ncuapp_ui_files}")

ADD_LIBRARY(upf_ncu_ui STATIC ${ncu_ui_files})