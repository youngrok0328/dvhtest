//CPP_0_________________________________ Precompiled header
#include "stdAfx.h"

//CPP_1_________________________________ Main header
#include "VisionInspectionKOZ2DPara.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/semiinfo.h"
#include "../../SharedCommunicationModules/VisionHostCommon/DBObject.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
VisionInspectionKOZ2DPara::VisionInspectionKOZ2DPara(VisionProcessing& parent)
    : m_NotchImageProcManagePara(parent)
    , m_ImageProcManagePara(parent)
    , m_ChippingImageProcManagePara(parent)
    , m_NotchAlignFrameIndex(parent, FALSE)
    , m_KOZAlignFrameIndex(parent, FALSE)
    , m_ChippingInspFrameIndex(parent, FALSE)
{
}

VisionInspectionKOZ2DPara::~VisionInspectionKOZ2DPara(void)
{
}

BOOL VisionInspectionKOZ2DPara::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    long version = 1;
    if (!db[_T("Version")].Link(bSave, version))
        version = 0;

    CString strTemp;

    //{{//Notch Align
    if (!m_NotchAlignFrameIndex.LinkDataBase(bSave, db[GUID_KOZ2D_MODULE_NOTCH_ALIGN_FRAME_IDX]))
        m_NotchAlignFrameIndex.setFrameListIndex(0);
    m_NotchImageProcManagePara.LinkDataBase(bSave, db[GUID_KOZ2D_MODULE_IMAGECOMBINE_NOTCH]);

    if (!db[_T("m_nNotchEdgeSearchLengthRatio")].Link(bSave, m_nNotchEdgeSearchLengthRatio))
        m_nNotchEdgeSearchLengthRatio = 120;
    //}}

    //{{//KOZ Align
    if (!m_KOZAlignFrameIndex.LinkDataBase(bSave, db[GUID_KOZ2D_MODULE_KOZ_ALIGN_FRAME_IDX]))
        m_KOZAlignFrameIndex.setFrameListIndex(0);
    m_ImageProcManagePara.LinkDataBase(bSave, db[GUID_KOZ2D_MODULE_IMAGECOMBINE]);

    if (!db[_T("m_nEdgeSearchLength_um")].Link(bSave, m_nEdgeSearchLength_um))
        m_nEdgeSearchLength_um = 500;
    if (!db[_T("m_nEdgeSearchWidthRatio")].Link(bSave, m_nEdgeSearchWidthRatio))
        m_nEdgeSearchWidthRatio = 80;
    if (!db[_T("m_nEdgeDetectGap_um")].Link(bSave, m_nEdgeDetectGap_um))
        m_nEdgeDetectGap_um = 500;
    if (!db[_T("m_nSearchDirection")].Link(bSave, m_nSearchDirection))
        m_nSearchDirection = PI_ED_DIR_INNER;
    if (!db[_T("m_nEdgeDirection")].Link(bSave, m_nEdgeDirection))
        m_nEdgeDirection = PI_ED_DIR_FALLING;
    if (!db[_T("m_nEdgeDetectMode")].Link(bSave, m_nEdgeDetectMode))
        m_nEdgeDetectMode = PI_ED_MAX_EDGE;
    if (!db[_T("m_fEdgeThreshold")].Link(bSave, m_dEdgeThreshold))
        m_dEdgeThreshold = 5.0;
    //if (!db[_T("m_nEdgeThresholdLevel")].Link(bSave, m_nEdgeThresholdLevel)) m_nEdgeThresholdLevel = EDGE_THRESHOLD_LEVEL_HIGH;
    //}}

    //{{//KOZ Chipping
    if (!m_ChippingInspFrameIndex.LinkDataBase(bSave, db[GUID_KOZ2D_MODULE_CHIPPING_INSP_FRAME_IDX]))
        m_ChippingInspFrameIndex.setFrameListIndex(0);

    m_ChippingImageProcManagePara.LinkDataBase(bSave, db[GUID_KOZ2D_MODULE_IMAGECOMBINE_CHIPPING]);

    if (!db[_T("m_nChippingIgnoreOffset_um")].Link(bSave, m_nChippingIgnoreOffset_um))
        m_nChippingIgnoreOffset_um = 50;
    if (!db[_T("m_nChippingInspectionExpandOffset_um")].Link(bSave, m_nChippingInspectionExpandOffset_um))
        m_nChippingInspectionExpandOffset_um = 200;
    if (!db[_T("m_nUseIgnorePreprocessObj")].Link(bSave, m_nUseIgnorePreprocessObj))
        m_nUseIgnorePreprocessObj = OPT_NOT_USE;
    if (!db[_T("m_nIgnoreDilationCount")].Link(bSave, m_nIgnoreDilationCount))
        m_nIgnoreDilationCount = 10;
    if (!db[_T("m_nChippingMergeDistance_um")].Link(bSave, m_nChippingMergeDistance_um))
        m_nChippingMergeDistance_um = 50;
    if (!db[_T("m_nWindowSizeX_um")].Link(bSave, m_nWindowSizeX_um))
        m_nWindowSizeX_um = 20000;
    if (!db[_T("m_nWindowSizeY_um")].Link(bSave, m_nWindowSizeY_um))
        m_nWindowSizeY_um = 20000;
    if (!db[_T("m_nDefectColor")].Link(bSave, m_nDefectColor))
        m_nDefectColor = DEFECT_DARK;
    if (!db[_T("m_nMinDarkContrast")].Link(bSave, m_nMinDarkContrast))
        m_nMinDarkContrast = 35;
    if (!db[_T("m_nMinBrightContrast")].Link(bSave, m_nMinBrightContrast))
        m_nMinBrightContrast = 35;
    if (!db[_T("m_nMinimumBlobArea_um2")].Link(bSave, m_nMinimumBlobArea_um2))
        m_nMinimumBlobArea_um2 = 10000;
    if (!db[_T("m_nMinimumBlobWidth_um")].Link(bSave, m_nMinimumBlobWidth_um))
        m_nMinimumBlobWidth_um = 100;
    if (!db[_T("m_nMinimumBlobLenght_um")].Link(bSave, m_nMinimumBlobLenght_um))
        m_nMinimumBlobLenght_um = 200;
    //}}

    return TRUE;
}

std::vector<CString> VisionInspectionKOZ2DPara::ExportAlgoParaToText(
    CString strVisionName, CString strInspectionModuleName) //kircheis_TxtRecipe
{
    //본 모듈의 공통 Parameter용 변수 (미리 값을 Fix)
    CString strHeader;
    strHeader.Format(_T("%s,%s"), (LPCTSTR)strVisionName, (LPCTSTR)strInspectionModuleName);

    //개별 Parameter용 변수
    CString strCategory;
    CString strGroup;
    CString strParaName;
    CString strParaNameAux;
    CString strValue;

    //Output Parameter 생성
    std::vector<CString> vecstrAlgorithmParameters(0);

    //Image Combine
    std::vector<CString> vecstrNotchCombineParameters(0);
    vecstrNotchCombineParameters = m_NotchImageProcManagePara.ExportImageCombineParaToText(
        strVisionName, strInspectionModuleName, _T("Notch Align Image Combine"));

    vecstrAlgorithmParameters.insert(
        vecstrAlgorithmParameters.end(), vecstrNotchCombineParameters.begin(), vecstrNotchCombineParameters.end());

    std::vector<CString> vecstrKozCombineParameters(0);
    vecstrKozCombineParameters = m_ImageProcManagePara.ExportImageCombineParaToText(
        strVisionName, strInspectionModuleName, _T("KOZ Align Image Combine"));

    vecstrAlgorithmParameters.insert(
        vecstrAlgorithmParameters.end(), vecstrKozCombineParameters.begin(), vecstrKozCombineParameters.end());

    std::vector<CString> vecstrImageCombineParameters(0);
    vecstrImageCombineParameters = m_ChippingImageProcManagePara.ExportImageCombineParaToText(
        strVisionName, strInspectionModuleName, _T("Chipping Image Combine"));

    vecstrAlgorithmParameters.insert(
        vecstrAlgorithmParameters.end(), vecstrImageCombineParameters.begin(), vecstrImageCombineParameters.end());

    //Module 특성 변수 //현재 DLL은 존재하나 객체 할당을 하지 않아 아직 활송화 되지 않는다. 추후 작업할 것!!!

    return vecstrAlgorithmParameters;
}