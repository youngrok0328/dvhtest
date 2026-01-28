#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
#include "../../VisionNeedLibrary/VisionCommon/ImageProcPara.h"

//HDR_3_________________________________ External library headers
#include <Ipvm/Base/Point32r2.h>
#include <Ipvm/Base/Rect32r.h>

//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class CiDataBase;

//HDR_6_________________________________ Header body
//
class CVisionProcessingOtherAlignPara
{
public:
    CVisionProcessingOtherAlignPara(VisionProcessing& parent);
    ~CVisionProcessingOtherAlignPara(void);

    BOOL LinkDataBase(BOOL bSave, CiDataBase& db);

    std::vector<CString> ExportAlgoParaToText(
        CString strVisionName, CString strInspectionModuleName); //kircheis_TxtRecipe

    void Init();

    //Align Para
    std::vector<Ipvm::Rect32r> m_vecrtSearchROI_BCU;

    long m_nSelectDirection;
    long m_nEdgeDirection[4]; //LTRB 순 저장
    long m_nSearchDirection[4];
    long m_nEdgeDetectMode[4];
    long m_nFirstEdgeValue[4];
    long m_nEdgeSearchLength[4];

    long m_nSearchROICount;

    //{{//kircheis_USI_OA
    BOOL m_arbUseEdgeAlignResult[4]; //FALSE: Edge 추출   //TRUE:Body align 사용
    Ipvm::Point32r2 m_fptBodyCenterOffset_um;
    //}}

    ImageProcPara m_ImageProcMangePara;

    bool m_bIsValidateRecipe; //Recipe가 유효한지 체크하기 위한 변수(저장 안함)
};
