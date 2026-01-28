//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionInspectionOTITRPara.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/TxtRecipeParameter.h" //kircheis_TxtRecipe
#include "../../SharedCommunicationModules/VisionHostCommon/DBObject.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
VisionInspectionOTITRPara::VisionInspectionOTITRPara(VisionProcessing& parent)
    : m_coverTapeImageFrameIndex(parent, false)
    , m_coverTapeImageProc(parent)
    , m_sealingImageFrameIndex(parent, false)
    , m_sealingImageProc(parent)
{
    
    Init();
}

VisionInspectionOTITRPara::~VisionInspectionOTITRPara(void)
{
}

BOOL VisionInspectionOTITRPara::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    long version = 1;
    if (!db[_T("OTR_PARAM_Version")].Link(bSave, version))
        version = 1;

    if (!bSave)
    {
        Init();
    }

    m_coverTapeImageFrameIndex.LinkDataBase(bSave, db[_T("{3AE9502C-6E91-4E32-B6A6-3FCCF8A3198F}")]);
    m_coverTapeImageProc.LinkDataBase(bSave, db[_T("{CF0EC272-F988-419D-BB9D-C86961D7C07C}")]);
    if (!db[_T("{7EE7C4E0-B0CF-49C9-A590-1A388AB93101}")].Link(bSave, m_coverTapeEdgeDetectMode))
        m_coverTapeEdgeDetectMode = EdgeDetectMode_BestEdge;
    if (!db[_T("{FE3693C1-293F-43C1-B36F-D217D780F44D}")].Link(bSave, m_coverTapeEdgeThreshold))
        m_coverTapeEdgeThreshold = 10.;
    if (!db[_T("{4C2A1B0A-8D7E-4F6C-9A5B-3E2F1D0C9B8A}")].Link(bSave, m_coverTapeAlignSearchLength_um))
        m_coverTapeAlignSearchLength_um = 500;
    if (!db[_T("{6B7EC844-2AD3-4B97-B318-1C77FCC6D62B}")].Link(bSave, m_coverTapeDetectGap_um))
        m_coverTapeDetectGap_um = 100;

    m_sealingImageFrameIndex.LinkDataBase(bSave, db[_T("{E1D3C8D4-1F2A-4C7D-8D6E-3A9F0B5C6D7E}")]);
    m_sealingImageProc.LinkDataBase(bSave, db[_T("{D4F5A6B7-C8D9-4E0F-9A1B-2C3D4E5F6071}")]);
    if (!db[_T("{8F9E0D1C-2B3A-4C5D-6E7F-8091A2B3C4D5}")].Link(bSave, m_sealingEdgeDetectMode))
        m_sealingEdgeDetectMode = EdgeDetectMode_BestEdge;
    if (!db[_T("{1A2B3C4D-5E6F-7081-92A3-B4C5D6E7F809}")].Link(bSave, m_sealingEdgeThreshold))
        m_sealingEdgeThreshold = 10.;
    if (!db[_T("{0F1E2D3C-4B5A-6978-8091-A2B3C4D5E6F7}")].Link(bSave, m_sealingAlignSearchLength_um))
        m_sealingAlignSearchLength_um = 500;
    if (!db[_T("{9F8E7D6C-5B4A-3928-1706-050403020100}")].Link(bSave, m_sealingDetectGap_um))
        m_sealingDetectGap_um = 100;
    if (!db[_T("{ABCDEF12-3456-7890-ABCD-EF1234567890}")].Link(bSave, m_sealingBoundaryType))
        m_sealingBoundaryType = enumSealingBoundaryType_Outer;


    return TRUE;
}

void VisionInspectionOTITRPara::Init()
{
    m_coverTapeImageFrameIndex.setFrameIndex(0);
    m_coverTapeImageProc.Init();
    m_coverTapeEdgeDetectMode = EdgeDetectMode_BestEdge;
    m_coverTapeEdgeThreshold = 10.;
    m_coverTapeAlignSearchLength_um = 500;
    m_coverTapeDetectGap_um = 100;

    m_sealingImageFrameIndex.setFrameIndex(0);
    m_sealingImageProc.Init();
    m_sealingEdgeDetectMode = EdgeDetectMode_BestEdge;
    m_sealingEdgeThreshold = 10.;
    m_sealingAlignSearchLength_um = 500;
    m_sealingDetectGap_um = 100;
    m_sealingBoundaryType = enumSealingBoundaryType_Outer;
}

std::vector<CString> VisionInspectionOTITRPara::ExportAlgoParaToText(
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

    //Module 특성 변수 
    //Algorithm Parameters
    strCategory.Format(_T("Algorithm Parameters"));

    //Common Parameters
    strGroup.Format(_T("Cover tape align parameters"));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Cover Tpae Edge Detection Mode"), _T(""), m_coverTapeEdgeDetectMode));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Cover Tape Edge Threshold"), _T(""), m_coverTapeEdgeThreshold));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
        _T("Cover Tape Align Search Length"), _T("um"), m_coverTapeAlignSearchLength_um));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Cover Tape Edge Gap"), _T(""), m_coverTapeDetectGap_um));

    strGroup.Format(_T("Seaning align parameters"));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Sealing Edge Detection Mode"), _T(""), m_sealingEdgeDetectMode));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Sealing Edge Threshold"), _T(""), m_sealingEdgeThreshold));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Sealing Align Search Length"), _T("um"), m_sealingAlignSearchLength_um));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Sealing Edge Gap"), _T(""), m_sealingDetectGap_um));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Sealing Boundary Type"), _T(""), m_sealingBoundaryType));

    return vecstrAlgorithmParameters;
}