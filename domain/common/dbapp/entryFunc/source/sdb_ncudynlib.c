#include "sdb_ncudynlib.h"
#include "xdb_core_pfu.h"
#include "ps_db_define_ncu.h" //这两个pfu相关的定义需要再考虑，主要波及_NCU_DBHANDLE_COMM之类的句柄
#include "xdb_core_pfu_db.h" //xdb_Pfu_Create_Db
#include "xdb_core_pfu_tblimp.h" //xdb_pfu_create_tblimpList
#include "xdb_core_pfu_synobj.h" //xdb_pfu_create_synobjList
#include "thdm_pub.h"
#include "psUpfSCTypes.h" //SC_SIZE_E
#include "zte_slibc.h"

#include "psDbCommonDynTables.h"
#include "psDbMcsDynTables.h"
#define TYPEOFVRU_PACKETFORWARD 1 //模型文件中信息，需要做好模型后使用

enum SIZEOFVRU
{
    SIZEOFVRU_S = 1,
    SIZEOFVRU_L = 2,
}; //模型文件中信息，需要做好模型后使用

enum NAMEOFTHREAD
{
  NAMEOFTHREAD_DB_COMMON = 1,
  NAMEOFTHREAD_SCAN = 2,
  NAMEOFTHREAD_DISPATCH = 3,
  NAMEOFTHREAD_MEDIA = 4,
  NAMEOFTHREAD_LDBSYNC = 6,
  NAMEOFTHREAD_PFUC = 7,
  NAMEOFTHREAD_LI = 10,
  NAMEOFTHREAD_TCPO = 11,
}; //模型文件中信息，需要做好模型后使用
WORD64 xdb_Ncu_HugepageMemGetCommonSpareSize(WORD32 dwSthr);
WORD64 xdb_Ncu_HugepageMemGetThreadSpareSize();
WORD32 g_dwNcuDynLibInitOver = 0;
BYTE create_ncudynlibs(VOID)
{
    WORD64 dbMemSize = xdb_Ncu_HugepageMemGetCommonSpareSize(0);
    #ifdef FT_TEST
    dbMemSize = 24*1024*1024;
    #endif
    BYTE*  pDbpShareAddress = NULL;
    BOOL dbRet = FALSE;
    pDbpShareAddress = xdb_Pfu_Create_Db(_NCU_DBHANDLE_COMM, "NCU_DB_COMM", dbMemSize);
    zte_printf_s("line %d pDbpShareAddress %p\n",__LINE__,pDbpShareAddress);
    _DB_STATEMENT_TRUE_LOG_RTN_VALUE((NULL ==pDbpShareAddress),LOG_EMERGENCY,FALSE);

    xdb_pfu_create_tblimpList(_NCU_DBHANDLE_COMM);
    xdb_pfu_create_synobjList(_NCU_DBHANDLE_COMM);

    dbRet = create_ncu_common_dyn_tables(_NCU_DBHANDLE_COMM);
    zte_printf_s("line %d dbRet %d\n",__LINE__,dbRet);
    _DB_STATEMENT_TRUE_LOG_RTN_VALUE((FALSE ==dbRet),LOG_EMERGENCY,FALSE);
    XOS_SysLog(LOG_EMERGENCY, "DB: create_ncu_common_nopage_tbl success!\n");
    T_PSThreadInstAffinityPara   tInstPara = {0};
    WORD32 dwRet = psGetMediaThreadInfo(THDM_MEDIA_TYPE_PFU_MEDIA_PROC, &tInstPara);
    #ifdef FT_TEST
    tInstPara.bInstNum=1;
    tInstPara.atThreadParaList[0].bSoftThreadNo = 4;
    #endif
    zte_printf_s("line %d dwRet %d tInstPara.bInstNum %d\n",__LINE__,dwRet,tInstPara.bInstNum);
    _DB_STATEMENT_TRUE_LOG_RTN_VALUE((0 != dwRet), LOG_EMERGENCY, FALSE);
    _DB_STATEMENT_TRUE_LOG_RTN_VALUE((tInstPara.bInstNum > MAX_PS_THREAD_INST_NUM), LOG_EMERGENCY, FALSE);

    BYTE bInstNum = 0;
    for(; bInstNum < tInstPara.bInstNum; bInstNum++)
    {
        WORD64 ddwTime1 = XOS_GetCurStdSecs();
        BYTE bSoftThreadNo = tInstPara.atThreadParaList[bInstNum].bSoftThreadNo;

        dbMemSize = xdb_Ncu_HugepageMemGetThreadSpareSize(); 
        #ifdef FT_TEST
        dbMemSize = 56*1024*1024;
        #endif
        zte_printf_s("DB: create_pfudynlibs MemSize %llu! [%s,%d]\n", dbMemSize, __FILE__, __LINE__);

        CHAR   acName[XDB_NCU_DB_NAME_LEN]={0};
        sprintf(acName, "NCU_DB_%d", bInstNum);
        pDbpShareAddress = xdb_Pfu_Create_Db(_NCU_GET_DBHANDLE(bSoftThreadNo),acName,dbMemSize);
        _DB_STATEMENT_TRUE_LOG_RTN_VALUE((NULL ==pDbpShareAddress),LOG_EMERGENCY,FALSE);

        xdb_pfu_create_tblimpList(_NCU_GET_DBHANDLE(bSoftThreadNo));
        xdb_pfu_create_synobjList(_NCU_GET_DBHANDLE(bSoftThreadNo));


        dbRet = create_ncu_mcsthread_dyn_tables(_NCU_GET_DBHANDLE(bSoftThreadNo));
        _DB_STATEMENT_TRUE_LOG_RTN_VALUE((FALSE == dbRet),LOG_EMERGENCY,FALSE);
        XOS_SysLog(LOG_EMERGENCY, "DB: create_ncu_mcsthread_dyn_tables MEDIA_PROC success!\n");

        WORD64 ddwTime2 =  XOS_GetCurStdSecs();
        zte_printf_s("DB: create db(%s) time interval is %llu! bSoftThreadNo(%u) \n", acName, ddwTime2-ddwTime1, bSoftThreadNo);
        XOS_Delay(10);
    }
    
    g_dwNcuDynLibInitOver = 1;  /* DB动态库初始化完成 */
    return TRUE;
}


WORD64 xdb_Ncu_HugepageMemGetThreadSpareSize()
{
    SC_SIZE size = (SC_SIZE)GetSCSize();
    WORD64 ddwThreadSize = 0;

    if(SC_SIZE_L == size)
    {
        ddwThreadSize = psGetMediaThreadSize(TYPEOFVRU_PACKETFORWARD, SIZEOFVRU_L, NAMEOFTHREAD_MEDIA);
        zte_printf_s("\n PACKETFORWARD MEDIA VRUSIZE_L wThreadSize = %llu\n", ddwThreadSize);
        return 0 != ddwThreadSize ? ddwThreadSize:1800*1024*1024UL;
    }

    if(SC_SIZE_S == size)
    {
        ddwThreadSize = psGetMediaThreadSize(TYPEOFVRU_PACKETFORWARD, SIZEOFVRU_S, NAMEOFTHREAD_MEDIA);
        zte_printf_s("\n PACKETFORWARD MEDIA VRUSIZE_S wThreadSize = %llu\n", ddwThreadSize);
        return 0 != ddwThreadSize ? ddwThreadSize:1800*1024*1024UL;
    }

    //SC_SIZE_S兜底
    ddwThreadSize = psGetMediaThreadSize(TYPEOFVRU_PACKETFORWARD, SIZEOFVRU_S, NAMEOFTHREAD_MEDIA);
    zte_printf_s("\n PACKETFORWARD MEDIA VRUSIZE_S wThreadSize = %llu\n", ddwThreadSize);
    return 0 != ddwThreadSize ? ddwThreadSize:1800*1024*1024UL;
}

WORD64 xdb_Ncu_HugepageMemGetCommonSpareSize(WORD32 dwSthr)
{
    SC_SIZE size = (SC_SIZE)GetSCSize();
    WORD64 ddwThreadSize = 0;

    if (SC_SIZE_L == size)
    {
        ddwThreadSize = psGetMediaThreadSize(TYPEOFVRU_PACKETFORWARD,SIZEOFVRU_L,NAMEOFTHREAD_DB_COMMON);
        zte_printf_s("\n PACKETFORWARD COMMON VRUSIZE_L wThreadSize = %llu\n", ddwThreadSize);
        return 0 != ddwThreadSize ? ddwThreadSize:3200*1024*1024UL;
    }

    if (SC_SIZE_S == size)
    {
        ddwThreadSize = psGetMediaThreadSize(TYPEOFVRU_PACKETFORWARD,SIZEOFVRU_S,NAMEOFTHREAD_DB_COMMON);
        zte_printf_s("\n PACKETFORWARD COMMON VRUSIZE_S wThreadSize = %llu\n", ddwThreadSize);
        return 0 != ddwThreadSize ? ddwThreadSize:1000*1024*1024UL;
    }

    ddwThreadSize = psGetMediaThreadSize(TYPEOFVRU_PACKETFORWARD,SIZEOFVRU_S,NAMEOFTHREAD_DB_COMMON);
    zte_printf_s("\n PACKETFORWARD COMMON VRUSIZE_S wThreadSize = %llu\n", ddwThreadSize);
    return 0 != ddwThreadSize ? ddwThreadSize:1000*1024*1024UL;
}
