/*
 *    PS统一规划巨页内存空间头文件
 */

/***************************** 头文件 *****************************/
#ifndef  _PS_MEMSCHE_H
#define  _PS_MEMSCHE_H

#include "ps_define.h"


#define VIR_MEM_CSS_SHARE_ID      0
#define VIR_MEM_DPI_SHARE_ID         1
#define VIR_MEM_MCS_LCOAL_ID      2
#define VIR_MEM_DB_SHARE_ID      3
#define VIR_MEM_DB_LOCAL_ID      4

/* XDPI内存ID */
#define VIR_MEM_XDPI_LSI_SHARE_ID         1
#define VIR_MEM_XDPI_DB_SHARE_ID         2
#define VIR_MEM_XDPI_MCS_SHARE_ID         3

#if (_PHY_BOARD == _BT_PC)||(_BT_VMB == _PHY_BOARD)
#define VIR_MEM_CSS_SHARE_SIZE    (120*1024*1024)
#elif (_PHY_BOARD != _BT_UFCD)

#if ((_SVR_NAME == _SN_SLBCTRL) || (_SVR_NAME == _SN_SLBMEDIA))
#define VIR_MEM_CSS_SHARE_SIZE    ((1500)*1024*1024)
#elif ((_SVR_NAME == _SN_STUCTRL) || (_SVR_NAME == _SN_STUMEDIA))
#define VIR_MEM_CSS_SHARE_SIZE    ((256 + 80 + 50 +60 + 50)*1024*1024)
#else
#define VIR_MEM_CSS_SHARE_SIZE    ((256 + 80 + 50 +60)*1024*1024)
#endif
#else
#if ((_SVR_NAME == _SN_SLBCTRL) || (_SVR_NAME == _SN_SLBMEDIA))
#define VIR_MEM_CSS_SHARE_SIZE    ((1600)*1024*1024)
#elif ((_SVR_NAME == _SN_STUCTRL) || (_SVR_NAME == _SN_STUMEDIA))
#define VIR_MEM_CSS_SHARE_SIZE    ((800 + 50)*1024*1024)
#else
#define VIR_MEM_CSS_SHARE_SIZE    ((800)*1024*1024)
#endif

#endif


#define VIR_MEM_IPSTACK_MEDIA_SHARE_SIZE     ((2048*2+1)*1024)
#define VIR_MEM_IPSTACK_UB_SHARE_SIZE     (4*1024*1024)
/*RFC内存大小*/
#define VIR_MEM_RFC_SHARE_SIZE  (56*1024*1024)

/*PCC内存大小*/
#define VIR_MEM_PCC_SHARE_SIZE  (30*1024*1024)

/*ACC内存大小*/
#if (_PHY_BOARD != _BT_PC)&&(_BT_VMB != _PHY_BOARD)
#define VIR_MEM_ACC_SHARE_SIZE  (24*1024*1024)
#else
#define VIR_MEM_ACC_SHARE_SIZE  (48*1024*1024)
#endif

/*vgsu MCS内存大小*/
#if (_PHY_BOARD != _BT_PC)&&(_BT_VMB != _PHY_BOARD)
#define VIR_MEM_MCS_SHARE_SIZE  (210*1024*1024)
#else
#define VIR_MEM_MCS_SHARE_SIZE    (64*1024*1024) /* 缩一下 */
#endif
#define SGW_VIR_MEM_MCS_SHARE_SIZE (32*1024*1024)

/*vpfu MCS内存大小 */
#ifdef FT_TEST
#define VIR_MEM_VPFUMCS_SHARE_SIZE    (1024UL*1024UL*1024UL)
#elif  (_CPU_TYPE == _CPU_ARM)
#define VIR_MEM_VPFUMCS_SHARE_SIZE    (1024UL*1024UL*1024UL)
#else
#define VIR_MEM_VPFUMCS_SHARE_SIZE    (562*1024*1024)
#endif

#define VIR_MEM_VPFUSFP_SHARE_SIZE    (256*1024*1024)

/*SEU上电测试，临时指定的内存大小，后续修改*/
#define VIR_MEM_VPFUSFSEU_SHARE_SIZE    (256*1024*1024)

/* TRIEsls */
#define VIR_MEM_TRIE_SHARE_SIZE 2*1024*1024
#define VIR_MEM_TRIE_IPPOOL_SHARE_SIZE 36*1024*1024
#define VIR_MEM_TRIE_BGP_ROUTE_SHARE_SIZE 300*1024*1024
#define VIR_MEM_BGP_SHARE_SIZE (32*1024*1024)


/*10095930 xdpi adapt*/
#define VIR_MEM_XDPI_ADAPT_SHARE_SIZE (30*1024)


/* 内存分配方案暂定 */
#define PDSN_VIR_MEM_MCS_LOCAL_SIZE    4*128*1024*1024
#define PDSN_VIR_MEM_MCS_LOCAL_SIZE_LOW    2*128*1024*1024
#define PDSN_VIR_MEM_MCS_LOCAL_SIZE_HIGH    2*128*1024*1024

#define PDSN_VIR_MEM_DB_SHARE_SIZE          (WORD64)6*128*1024*1024     /* 控制面开启的DB巨页内存大小  */
#define PDSN_VIR_MEM_DB_SHARE_SIZE_MEDIA     (WORD64)3*128*1024*1024     /* 媒体面仅针对部分开启巨页共享 */
#define PDSN_VIR_MEM_DB_LOCAL_SIZE    (WORD64)1/* 2 modified by jchb 20101014 for 611001221826*/*300*1024*1024

#define HSGW_VIR_MEM_MCS_LOCAL_SIZE    4*128*1024*1024
#define HSGW_VIR_MEM_MCS_LOCAL_SIZE_LOW    2*128*1024*1024
#define HSGW_VIR_MEM_MCS_LOCAL_SIZE_HIGH    2*128*1024*1024

#define HSGW_VIR_MEM_DB_SHARE_SIZE          (WORD64)6*128*1024*1024     /* 控制面开启的DB巨页内存大小  */
#define HSGW_VIR_MEM_DB_SHARE_SIZE_MEDIA     (WORD64)3*128*1024*1024     /* 媒体面仅针对部分开启巨页共享 */
#define HSGW_VIR_MEM_DB_LOCAL_SIZE    (WORD64)1/* 2 modified by jchb 20101014 for 611001221826*/*300*1024*1024

#define HA_VIR_MEM_MCS_LOCAL_SIZE    3*128*1024*1024
#define HA_VIR_MEM_DB_SHARE_SIZE          (WORD64)3500/* 6*128 */*1024*1024  /*5*128*1024*1024*//* 751 */     /* 控制面开启的DB巨页内存大小  */
#define HA_VIR_MEM_DB_SHARE_SIZE_MEDIA    (WORD64)4*128*1024*1024 /*3*128*1024*1024 */    /* 媒体面仅针对部分开启巨页共享 */
#define HA_VIR_MEM_DB_LOCAL_SIZE    (WORD64)400/* 2*128 */*1024*1024  /* 3*128*1024*1024 */

#define GGSN_VIR_MEM_MCS_LOCAL_SIZE    0*128*1024*1024
#define GGSN_VIR_MEM_DB_SHARE_SIZE          (WORD64)6*128*1024*1024  /*5*128*1024*1024*//* 751 */     /* 控制面开启的DB巨页内存大小  */
#define GGSN_VIR_MEM_DB_SHARE_SIZE_MEDIA    (WORD64)4*128*1024*1024 /*3*128*1024*1024 */    /* 媒体面仅针对部分开启巨页共享 */
#define GGSN_VIR_MEM_DB_LOCAL_SIZE    (WORD64)4*128*1024*1024  /* 3*128*1024*1024 */

#define SGW_VIR_MEM_MCS_LOCAL_SIZE    3*128*1024*1024
#define SGW_VIR_MEM_DB_SHARE_SIZE          (WORD64)6*128*1024*1024  /*5*128*1024*1024*//* 751 */     /* 控制面开启的DB巨页内存大小  */
#define SGW_VIR_MEM_DB_SHARE_SIZE_MEDIA    (WORD64)4*128*1024*1024 /*3*128*1024*1024 */    /* 媒体面仅针对部分开启巨页共享 */
#define SGW_VIR_MEM_DB_LOCAL_SIZE    (WORD64)4*128*1024*1024  /* 3*128*1024*1024 */

#define PGW_VIR_MEM_MCS_LOCAL_SIZE    3*128*1024*1024
#define PGW_VIR_MEM_DB_SHARE_SIZE          (WORD64)6*128*1024*1024  /*5*128*1024*1024*//* 751 */     /* 控制面开启的DB巨页内存大小  */
#define PGW_VIR_MEM_DB_SHARE_SIZE_MEDIA    (WORD64)5*128*1024*1024 /*3*128*1024*1024 */    /* 媒体面仅针对部分开启巨页共享 */
#define PGW_VIR_MEM_DB_LOCAL_SIZE    (WORD64)5*128*1024*1024  /* 3*128*1024*1024 */

#define TWAG_VIR_MEM_MCS_LOCAL_SIZE    3*128*1024*1024
#define TWAG_VIR_MEM_DB_SHARE_SIZE          (WORD64)6*128*1024*1024  /*5*128*1024*1024*//* 751 */     /* 控制面开启的DB巨页内存大小  */
#define TWAG_VIR_MEM_DB_SHARE_SIZE_MEDIA    (WORD64)4*128*1024*1024 /*3*128*1024*1024 */    /* 媒体面仅针对部分开启巨页共享 */
#define TWAG_VIR_MEM_DB_LOCAL_SIZE    (WORD64)4*128*1024*1024  /* 3*128*1024*1024 */

#define SLB_VIR_MEM_MCS_LOCAL_SIZE    3*128*1024*1024
#define SLB_VIR_MEM_DB_SHARE_SIZE          (WORD64)8*128*1024*1024  /*5*128*1024*1024*//* 751 */     /* 控制面开启的DB巨页内存大小  */
#define SLB_VIR_MEM_DB_SHARE_SIZE_MEDIA    (WORD64)8*128*1024*1024 /*3*128*1024*1024 */    /* 媒体面仅针对部分开启巨页共享 */
#define SLB_VIR_MEM_DB_LOCAL_SIZE    (WORD64)4*128*1024*1024  /* 3*128*1024*1024 */

/* XDPI只需要承载和XDPI两块内存 */
#define XDPI_VIR_MEM_DB_SHARE_SIZE      (WORD64)7*128*1024*1024
#define XDPI_VIR_MEM_MCS_SHARE_SIZE      (5*128*1024*1024-30*1024*1024)
#define XDPI_VIR_MEM_LSI_SHARE_SIZE      5*128*1024*1024

#define STU_VIR_MEM_DB_LOCAL_SIZE    (WORD64)2*128*1024*1024
#define VIR_MEM_STU_DAP_SHARE_SIZE   (128*1024*1024)

#define VIR_MEM_STU_SCTP_SHARE_SIZE   (56*1024*1024)

#define VIR_MEM_VPFUUDR_SHARE_SIZE    (20*1024*1024)

/* DBS全局变量使用的共享内存 */
#define DBS_MEM_GLOBAL_VAR_SHARE_SIZE (WORD64)30*1024*1024



/*流程中需要使用的全局变量，使用该宏申明，保证所有全局变量在代码段中的空间连续性 */
#define __PS_GVAL_SPEC__




extern WORD64 ptMcsPhyMemAddr;        /* MCS physical memory address  */

extern WORD64  gptDPIPhyMemAddr;        /*DPI physical memory address  */




typedef struct Tag_psGlobalVarDef{
    ULONG vir_css_addr;
    ULONG vir_msc_share_addr;
    ULONG vir_msc_sgw_share_addr;
    ULONG vir_msc_local_addr;
    ULONG vir_msc_local_addr_low ;
    ULONG vir_msc_local_addr_high ;
    ULONG vir_sfp_share_addr;
    ULONG vir_db_share_addr;
    ULONG vir_db_local_addr;
    ULONG vir_dpi_share_addr;
    ULONG vir_xdpi_db_share_addr;
    ULONG vir_xdpi_mcs_share_addr;
    ULONG vir_xdpi_lsi_share_addr;
    ULONG vir_rfc_addr;
    ULONG vir_pcc_addr;
    ULONG vir_ipstack_media_share_addr;
    ULONG vir_acc_addr;
    ULONG vir_trie_addr; /* TRIEsls*/
    ULONG vir_trie_ippool_addr; /* TRIEsls*/
    ULONG vir_bgp_addr; /* TRIEsls*/
    ULONG vir_trie_bgp_route_addr; /* TRIEsls*/
    ULONG vir_xdpi_adapt_share_addr; /*10095930*/
    ULONG vir_tcp_addr;
    ULONG vir_stu_dap_addr;
    ULONG vir_psm_share_addr;
    ULONG vir_db_core_mem_addr;
    ULONG vir_flowcache_addr;
    ULONG vir_sctp_addr;
    ULONG vir_sfseu_share_addr;
    ULONG vir_udr_share_addr;
}T_psGlobalVar;

extern T_psGlobalVar g_psGlobalVar;

#define Vir_Mem_CSS_SHARE_Addr (g_psGlobalVar.vir_css_addr)         /*   承载使用的巨页内存，占用第一片映射出的内存 */
#define Vir_Mem_MSC_Share_Addr (g_psGlobalVar.vir_msc_share_addr)   /*  MCS使用的共享内存映射  */
#define Vir_Mem_MSC_SGWShare_Addr (g_psGlobalVar.vir_msc_sgw_share_addr)   /*  MCS使用的共享内存映射  */
#define Vir_Mem_MSC_Local_Addr (g_psGlobalVar.vir_msc_local_addr)   /*  MCS使用的本地内存映射  */
#define Vir_Mem_MSC_Local_Addr_low (g_psGlobalVar.vir_msc_local_addr_low)   /*  MCS使用的本地内存映射  */
#define Vir_Mem_MSC_Local_Addr_high (g_psGlobalVar.vir_msc_local_addr_high)   /*  MCS使用的本地内存映射  */

#define Vir_Mem_SFP_Share_Addr (g_psGlobalVar.vir_sfp_share_addr)   /*  SPF使用的共享内存映射  */

#define Vir_Mem_SFSEU_Share_Addr (g_psGlobalVar.vir_sfseu_share_addr)   /*  SFSEU使用的共享内存映射  */

#define Vir_Mem_Ipstack_Media_Share_Addr  (g_psGlobalVar.vir_ipstack_media_share_addr)    /* 分发线程上送ipstack共享内存地址映射段 */
#define Vir_Mem_DB_Local_Addr  (g_psGlobalVar.vir_db_local_addr)    /* DB使用的LOCAL库共享内存地址映射段，控制面和媒体面共同使用  */

#define Vir_Mem_DPI_Share_Addr  (g_psGlobalVar.vir_dpi_share_addr)    /*  DPI使用的共享内存映射  */
#define Vir_Mem_XDPI_DB_Share_Addr  (g_psGlobalVar.vir_xdpi_db_share_addr)    /*  xDPI使用的共享内存映射  */
#define Vir_Mem_XDPI_MCS_Share_Addr  (g_psGlobalVar.vir_xdpi_mcs_share_addr)    /*  xDPI使用的共享内存映射  */
#define Vir_Mem_XDPI_LSI_Share_Addr  (g_psGlobalVar.vir_xdpi_lsi_share_addr)    /*  xDPI使用的共享内存映射  */

#define Vir_Mem_RFC_SHARE_Addr (g_psGlobalVar.vir_rfc_addr)
#define Vir_Mem_PCC_SHARE_Addr       (g_psGlobalVar.vir_pcc_addr)
#define Vir_Mem_ACC_SHARE_Addr   (g_psGlobalVar.vir_acc_addr)
#define Vir_Mem_FLOWCACHE_SHARE_Addr   (g_psGlobalVar.vir_flowcache_addr)

/* TRIEsls */
#define Vir_Mem_TRIE_SHARE_Addr (g_psGlobalVar.vir_trie_addr)
#define Vir_Mem_TRIE_IPPOOL_SHARE_Addr (g_psGlobalVar.vir_trie_ippool_addr)
#define Vir_Mem_BGP_SHARE_Addr (g_psGlobalVar.vir_bgp_addr)
#define Vir_Mem_TRIE_BGP_ROUTE_SHARE_Addr (g_psGlobalVar.vir_trie_bgp_route_addr)

#define Vir_Mem_XDPI_ADAPT_Share_Addr (g_psGlobalVar.vir_xdpi_adapt_share_addr) /*10095930*/
#define Vir_Mem_TCP_SHARE_Addr   (g_psGlobalVar.vir_tcp_addr)         /*   tcp协议栈使用的巨页内存 */
#define Vir_Mem_STU_DAP_SHARE_Addr   (g_psGlobalVar.vir_stu_dap_addr)         /*   zbh stu dap addr */
#define Vir_Mem_PSM_Share_Addr (g_psGlobalVar.vir_psm_share_addr)
/* sctp add begin */
#define Vir_Mem_SCTP_SHARE_Addr (g_psGlobalVar.vir_sctp_addr)
/* sctp add end */

#define Vir_Mem_UDR_SHARE_Addr (g_psGlobalVar.vir_udr_share_addr)




/****************************** 巨页内存改造2.0版本*************************************/
/* 向BSP申请内存的ID */
#define	PS_HUGE_MEM_PS_SHARE_ID          0

/* 最大申请内存块总数,包括隔离带内存 */
#define	PS_HUGE_MEM_BLOCK_NUM          128

/* 内存块名称最大字符数 */
#define	PS_HUGE_MEM_BLOCK_NAME_SIZE   28

/* 隔离带内存字节数 ,8字节的倍数*/
#define	PS_HUGE_MEM_SEPA_BLOCK_SIZE    64

/* 总的虚拟内存起始地址，刘强推荐值 */
#define	PS_HUGE_MEM_VIR_ADDR    0xa000000000ULL
#define	PS_HUGE_TCAM_VIR_ADDR   0x9000000000

/* 重复申请内存记录总数 */
#define	PS_HUGE_MEM_REAPPLY_INFO_NUM    100

/* 成研使用的内存size */
#define	PS_HUGE_MEM_KERNEL_SIZE    512*1024*1024

/* 保序的计数信号量每次释放的信号个数 */
#define	PS_HUGE_MEM_SEM_NUM           2
#define PS_HUGEMEM_OK                 0
#define PS_HUGEMEM_FAIL              1
#define pMemBlockInfo                      ptMemBlockTable->g_tMemBlockInfo

#define CSS_MEMNAME_MAXLEN 23
typedef struct __ps_mem_apply_info
{
    T_Tick tMemBlockApplyTick;
    WORD32 dwMemBlockApplyJno;
    WORD16 wGetXOSTickErr;
    WORD16 wGetXOSJIDErr;
} T_HUGE_MEM_APPLY_INFO;



#if (_PHY_BOARD == _BT_PC)||(_BT_VMB == _PHY_BOARD)

typedef struct __ps_men_block_info
{
    BYTE abMemBlockName[PS_HUGE_MEM_BLOCK_NAME_SIZE];
    WORD32 dwApplyTimes;
    ULONG  ulMemSize;
    ULONG  ulPhyMemAddr;
    ULONG  ulVirMemAddr;
	ULONG  ulAddrOffset;
    T_HUGE_MEM_APPLY_INFO tMemBlockApplyInfo[PS_HUGE_MEM_REAPPLY_INFO_NUM];
} T_HUGE_MEM_BLOCK_INFO;


typedef struct __ps_mem_table
{
    T_HUGE_MEM_BLOCK_INFO g_tMemBlockInfo[PS_HUGE_MEM_BLOCK_NUM];
    ULONG  g_ulAssignMemSize;
    ULONG  g_ulTotalHugMemSize;
    ULONG  g_ulBSPPhyMemAddr;
    ULONG  g_ulBSPVirMemAddr;
    ULONG  g_ulBSPCtrlVirMemAddr;
	ULONG  g_ulBSPMediaVirMemAddr;
	ULONG  g_ulAddrOffset;
    ULONG  g_ulHugeMemBlockNum;
    WORD32 g_dwHugeMemSemID;
    WORD32 dwIpstackMemInitFlg;
} T_HUGE_MEM_ASSIGN_TABLE;
#else
typedef struct __ps_men_block_info
{
    BYTE abMemBlockName[PS_HUGE_MEM_BLOCK_NAME_SIZE];
    WORD32 dwApplyTimes;
    ULONG  ulMemSize;
    ULONG  ulPhyMemAddr;
    ULONG  ulVirMemAddr;
    T_HUGE_MEM_APPLY_INFO tMemBlockApplyInfo[PS_HUGE_MEM_REAPPLY_INFO_NUM];
} T_HUGE_MEM_BLOCK_INFO;

typedef struct __ps_mem_table
{
    T_HUGE_MEM_BLOCK_INFO g_tMemBlockInfo[PS_HUGE_MEM_BLOCK_NUM + 1];
    ULONG  g_ulAssignMemSize;
    ULONG  g_ulTotalHugMemSize;
    ULONG  g_ulBSPPhyMemAddr;
    ULONG  g_ulBSPVirMemAddr;
    ULONG  g_ulHugeMemBlockNum;
    WORD32 g_dwHugeMemSemID;
    WORD32 dwIpstackMemInitFlg;
} T_HUGE_MEM_ASSIGN_TABLE;
#endif

#endif


