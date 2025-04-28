#include "psNcuUliInfoFunc.h"
#include "ps_mcs_define.h"
#include "zte_slibc.h"

void psNcuFillNrLocation(T_NrLocation* ptNrLoc, NrLocation* ptNrLocation);
void psNcuFillEutraLocation(T_EutraLocation* ptEutraLoc, EutraLocation* ptEutraLocation);

void psNcuFillULiNormal(T_IE_UserLocationInfo* ptUliInfo, UserLocation* ptUliData)
{
    if(NULL == ptUliInfo || NULL== ptUliData)
    {
        return;
    }
    
    if (ptUliData->eutraLocFg)
    {
        ptUliInfo->btEutraLocation = 1;
        psNcuFillEutraLocation(&(ptUliInfo->eutraLocation), &ptUliData->eutraLocation);
        return;
    }
    if(ptUliData->nrLocFg)
    {
        ptUliInfo->btNrLocation = 1;
        psNcuFillNrLocation(&(ptUliInfo->nrLocation), &ptUliData->nrLocation);
        return;
    }

    if (ptUliData->geraLocFg)
    {
        ptUliInfo->btGeraLoction = 1;
        T_GeraLocation* ptGeraLocation = &ptUliInfo->geraLocation;
        
        if (ptUliData->geraLocation.btRaiFg)
        {
            ptGeraLocation->RAI = 1;
            ptGeraLocation->tRAI.LAC = *(WORD16*)&(ptUliData->geraLocation.rai.bLAC);
            ptGeraLocation->tRAI.RoutingAreaCode = *(WORD16*)&(ptUliData->geraLocation.rai.bRAC);
        }
        if(ptUliData->geraLocation.btCgiFg)
        {
            ptGeraLocation->CGI = 1;
            ptGeraLocation->tCGI.LAC = *(WORD16*)&(ptUliData->geraLocation.cgi.bLAC);
            ptGeraLocation->tCGI.CellIdentify = *(WORD16*)&(ptUliData->geraLocation.cgi.bCI);
        }
    }
}

#define encode_plmnid(ReportStruct, DataStruct) \
do{\
    T_psGTPMccMnc1* ptPlmn = (T_psGTPMccMnc1*)&(ReportStruct); \
    ptPlmn->btMCC1 = DataStruct.btMCC1;\
    ptPlmn->btMCC2 = DataStruct.btMCC2;\
    ptPlmn->btMCC3 = DataStruct.btMCC3;\
    ptPlmn->btMNC3 = DataStruct.btMNC3;\
    ptPlmn->btMNC2 = DataStruct.btMNC2;\
    ptPlmn->btMNC1 = DataStruct.btMNC1;\
}while(0)

#define encode_ulivalue_2(ReportStruct, DataStruct) \
do{\
    BYTE* pSrc = (BYTE*)&(ReportStruct);\
    BYTE* pDst = (BYTE*)&(DataStruct);\
    pSrc[0] = pDst[0]; \
    pSrc[1] = pDst[1];\
}while(0)
#define encode_ulivalue_3(ReportStruct, DataStruct) \
do{\
    BYTE* pSrc = (BYTE*)&(ReportStruct);\
    BYTE* pDst = (BYTE*)&(DataStruct);\
    pSrc[0] = pDst[0]; \
    pSrc[1] = pDst[1];\
    pSrc[2] = pDst[2]; \
}while(0)
#define encode_ulivalue_4(ReportStruct, DataStruct) \
do{\
    BYTE* pSrc = (BYTE*)&(ReportStruct);\
    BYTE* pDst = (BYTE*)&(DataStruct);\
    pSrc[0] = pDst[0]; \
    pSrc[1] = pDst[1];\
    pSrc[2] = pDst[2]; \
    pSrc[3] = pDst[3];\
}while(0)
#define encode_ulivalue_5(ReportStruct, DataStruct) \
do{\
    BYTE* pSrc = (BYTE*)&(ReportStruct);\
    BYTE* pDst = (BYTE*)&(DataStruct);\
    pSrc[0] = pDst[0]; \
    pSrc[1] = pDst[1];\
    pSrc[2] = pDst[2]; \
    pSrc[3] = pDst[3];\
    pSrc[4] = pDst[4];\
}while(0)

void psNcuFillEutraLocation(T_EutraLocation* ptEutraLoc, EutraLocation* ptEutraLocation)
{
    if(NULL == ptEutraLoc || NULL == ptEutraLocation)
    {
        return;
    }
    ptEutraLoc->TAI_PlmnLen = 3;
    encode_plmnid(ptEutraLoc->TAI_PlmnValue, ptEutraLocation->tai.plmn);

    ptEutraLoc->TAI_TacLen = 2;
    encode_ulivalue_2(ptEutraLoc->TAI_TacValue, ptEutraLocation->tai.bTAC);
    
    ptEutraLoc->ECGI_PlmnLen = 3;
    encode_plmnid(ptEutraLoc->ECGI_PlmnValue, ptEutraLocation->ecgi.plmnId);

    ptEutraLoc->ECGI_EutraCellLen = 4;
    encode_ulivalue_4(ptEutraLoc->ECGI_EutraCellValue, ptEutraLocation->ecgi.eutraCellId);

    ptEutraLoc->ALI = 0;
    ptEutraLoc->LTM = 0;
    ptEutraLoc->GGI = 0;
    ptEutraLoc->GDI = 0;
    ptEutraLoc->NGENB = 0;
    ptEutraLoc->head.gNbf = 0;
    ptEutraLoc->head.ngeNbf = 0;

}

void psNcuSetNrLocationCellId(T_NrLocation* ptNrLoc)
{
    if (NULL == ptNrLoc)
    {
        return;
    }

    WORD32 nrCellIdCtl = getNcuSoftPara(5011);


    if (nrCellIdCtl == 1) // 左移8bit
    {
        ptNrLoc->NCGI_NrCellValue[0] = ptNrLoc->NCGI_NrCellValue[1];
        ptNrLoc->NCGI_NrCellValue[1] = ptNrLoc->NCGI_NrCellValue[2];
        ptNrLoc->NCGI_NrCellValue[2] = ptNrLoc->NCGI_NrCellValue[3];
        ptNrLoc->NCGI_NrCellValue[3] = ptNrLoc->NCGI_NrCellValue[4];
        ptNrLoc->NCGI_NrCellValue[4] = 0;
        return;
    }
    if (nrCellIdCtl == 2) // 左移4bit
    {
        ptNrLoc->NCGI_NrCellValue[0] = ((ptNrLoc->NCGI_NrCellValue[0] & 0x0F) << 4) | ((ptNrLoc->NCGI_NrCellValue[1] & 0xF0) >> 4);
        ptNrLoc->NCGI_NrCellValue[1] = ((ptNrLoc->NCGI_NrCellValue[1] & 0x0F) << 4) | ((ptNrLoc->NCGI_NrCellValue[2] & 0xF0) >> 4);
        ptNrLoc->NCGI_NrCellValue[2] = ((ptNrLoc->NCGI_NrCellValue[2] & 0x0F) << 4) | ((ptNrLoc->NCGI_NrCellValue[3] & 0xF0) >> 4);
        ptNrLoc->NCGI_NrCellValue[3] = ((ptNrLoc->NCGI_NrCellValue[3] & 0x0F) << 4) | ((ptNrLoc->NCGI_NrCellValue[4] & 0xF0) >> 4);
        ptNrLoc->NCGI_NrCellValue[4] = ((ptNrLoc->NCGI_NrCellValue[4] & 0x0F) << 4);
        return;
    }


    return;
}

void psNcuFillNrLocation(T_NrLocation* ptNrLoc, NrLocation* ptNrLocation)
{
    if(NULL == ptNrLoc || NULL == ptNrLocation)
    {
        return;
    }
    ptNrLoc->TAI_PlmnLen = 3;
    encode_plmnid(ptNrLoc->TAI_PlmnValue, ptNrLocation->tai.plmn);

    ptNrLoc->TAI_TacLen = 3;
    encode_ulivalue_3(ptNrLoc->TAI_TacValue, ptNrLocation->tai.bTAC);

    ptNrLoc->NCGI_PlmnLen = 3;
    encode_plmnid(ptNrLoc->NCGI_PlmnValue, ptNrLocation->ncgi.plmnId);

    ptNrLoc->NCGI_NrCellLen = 5;
    encode_ulivalue_5(ptNrLoc->NCGI_NrCellValue, ptNrLocation->ncgi.nrCellId);
    psNcuSetNrLocationCellId(ptNrLoc);


    ptNrLoc->ALI = 0;
    ptNrLoc->LTM = 0;
    ptNrLoc->GGI = 0;
    ptNrLoc->GDI = 0;
    ptNrLoc->GNB = 0;
    ptNrLoc->head.gNbf = 0;
    ptNrLoc->head.ngeNbf = 0;
}
