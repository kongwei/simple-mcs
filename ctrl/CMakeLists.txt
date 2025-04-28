PROJECT(UPF_NCU_CTRL)

SET(NCU_CTRL_PATH ${NCU_MCS_PATH}/ctrl)
SET(NCU_CTRL_JOB_PATH ${NCU_CTRL_PATH}/job)
SET(NCU_CTRL_GDMM_PATH ${NCU_CTRL_PATH}/gdmm)

# 包含job模块
INCLUDE_DIRECTORIES(${NCU_CTRL_JOB_PATH}/include)
# 包含gdmm模块
INCLUDE_DIRECTORIES(${NCU_CTRL_GDMM_PATH}/include)

# 收集源文件
AUX_SOURCE_DIRECTORY(${NCU_CTRL_JOB_PATH}/source ncuctrl_files)
AUX_SOURCE_DIRECTORY(${NCU_CTRL_GDMM_PATH}/source ncuctrl_files)

# 创建静态库
ADD_LIBRARY(upf_ncu_ctrl STATIC ${ncuctrl_files})

SET(ncudomain_files "${ncudomain_files};${ncuctrl_files}" PARENT_SCOPE) 