#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
#include "../../VisionNeedLibrary/VisionCommon/ImageProcPara.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionInspFrameIndex.h"

//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class CiDataBase;

//HDR_6_________________________________ Header body
//
class VisionInspectionKOZ2DPara
{
public:
    VisionInspectionKOZ2DPara(VisionProcessing& parent);
    ~VisionInspectionKOZ2DPara(void);

    BOOL LinkDataBase(BOOL bSave, CiDataBase& db);
    std::vector<CString> ExportAlgoParaToText(
        CString strVisionName, CString strInspectionModuleName); //kircheis_TxtRecipe

    //{{//Notch Align
    VisionInspFrameIndex m_NotchAlignFrameIndex;
    ImageProcPara m_NotchImageProcManagePara; //Btn
    long m_nNotchEdgeSearchLengthRatio;
    //**** Re Align 기능은 Test 해보니 필요 없을 것 같아 제외했음. Notch Hole이 충분히 크고 Edge가 명확해서 굳이 안써도 될듯 하다 ****//
    //}}

    //{{//KOZ Align
    VisionInspFrameIndex m_KOZAlignFrameIndex;
    ImageProcPara m_ImageProcManagePara; //Btn
    long m_nEdgeSearchLength_um;
    long m_nEdgeSearchWidthRatio;
    long m_nEdgeDetectGap_um;
    long m_nSearchDirection; //Combo List
    long m_nEdgeDirection; //Combo List
    long m_nEdgeDetectMode; //Combo List
    double m_dEdgeThreshold;
    long m_nEdgeThresholdLevel; //0:(Low->1.f), 1:(Middle->3.f), 2:(High->5.f), 3:(Highest->10.f)//Combo List
    //}}

    //{{//KOZ Chipping
    VisionInspFrameIndex m_ChippingInspFrameIndex;
    ImageProcPara m_ChippingImageProcManagePara; //Btn
    long m_nChippingIgnoreOffset_um;
    long m_nChippingInspectionExpandOffset_um;
    long m_nUseIgnorePreprocessObj; //Combo List
    long m_nIgnoreDilationCount;
    long m_nChippingMergeDistance_um;
    long m_nWindowSizeX_um;
    long m_nWindowSizeY_um;
    long m_nDefectColor; //0:Dark, 1:Bright, 2:All
    long m_nMinDarkContrast; //Slider
    long m_nMinBrightContrast; //Slider
    long m_nMinimumBlobArea_um2;
    long m_nMinimumBlobWidth_um;
    long m_nMinimumBlobLenght_um;
    //}}
};
