//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionProcessingAlign3DPara.h"

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
CVisionProcessingAlign3DPara::CVisionProcessingAlign3DPara(void)
    : m_skipEdgeAlign(false)
    , m_eRoughAlignUsingDefine(eRoughAlignUsingDefine::eRoughAlignUsingDefine_H_Map)
{
    m_fPaneExtensionRatio = 20.f; // 20%

    m_fPatternSizeX = 2.f;
    m_fPatternSizeY = 5.f;
    m_fMovingErrorX = 2.f;
    m_fMovingErrorY = 3.f;
    m_fMatchingSize = 5.f;
    m_fMatchingReduceSizeX = 10.f;

    m_fMatchingPosOffset = 0.f;

    m_nTopStichRoiAmountX = 7;
    m_nTopStichRoiAmountY = 10;
    m_topStichRoiRadiusX_um = 250.f;
    m_topStichRoiRadiusX_um = 250.f;
}

CVisionProcessingAlign3DPara::~CVisionProcessingAlign3DPara(void)
{
}

BOOL CVisionProcessingAlign3DPara::LinkDataBase(
    BOOL bSave, float packageSizeX_um, float packageSizeY_um, CiDataBase& db)
{
    UNREFERENCED_PARAMETER(packageSizeX_um);
    UNREFERENCED_PARAMETER(packageSizeY_um);

    long version = 2;
    if (!db[_T("Version")].Link(bSave, version))
        version = 0;

    db[_T("PaneExtensionRatio")].Link(bSave, m_fPaneExtensionRatio);

    db[_T("fPatternSizeX")].Link(bSave, m_fPatternSizeX);
    db[_T("fPatternSizeY")].Link(bSave, m_fPatternSizeY);
    db[_T("fMovingErrorX")].Link(bSave, m_fMovingErrorX);
    db[_T("fMovingErrorY")].Link(bSave, m_fMovingErrorY);

    db[_T("MatchingSize")].Link(bSave, m_fMatchingSize);
    if (!db[_T("MatchingReduceSizeY")].Link(bSave, m_fMatchingReduceSizeX))
        m_fMatchingReduceSizeX = 10.f;
    db[_T("MatchingPosOffset")].Link(bSave, m_fMatchingPosOffset);

    if (!db[_T("TopStichRoiAmountX")].Link(bSave, m_nTopStichRoiAmountX))
        m_nTopStichRoiAmountX = 7;
    if (!db[_T("TopStichRoiAmountY")].Link(bSave, m_nTopStichRoiAmountY))
        m_nTopStichRoiAmountY = 10;

    if (version < 1)
    {
        // 과거 IMAGE 좌표계 기준이었던 옛날 버전 Recipe이다. 더이상 지원하지 않는다
        return FALSE;
    }

    if (!db[_T("TopStichRoiRadiusX")].Link(bSave, m_topStichRoiRadiusX_um))
        m_topStichRoiRadiusX_um = 250.f;
    if (!db[_T("TopStichRoiRadiusY")].Link(bSave, m_topStichRoiRadiusY_um))
        m_topStichRoiRadiusY_um = 250.f;

    //if (!db[_T("{6FF00A63-F258-4BE3-8726-1D7C472E5972}")].Link(bSave, m_autoPaneOffsetX)) m_autoPaneOffsetX = TRUE;

    if (!bSave
        && db[_T("{1F07A3A0-110F-4FBC-B754-62403FC7F8EB}")].Link(bSave, (long&)m_eRoughAlignUsingDefine)
            == FALSE) //Load이면서 추가한 Param의 값이 없을때
    {
        BOOL bExistautoPaneOffset(TRUE);
        if (!db[_T("{6FF00A63-F258-4BE3-8726-1D7C472E5972}")].Link(
                bSave, bExistautoPaneOffset)) //이전 Param의 값을 찾지못할때
            m_eRoughAlignUsingDefine = eRoughAlignUsingDefine::eRoughAlignUsingDefine_H_Map; //이전에 사용하던 Default값
        else //찾았다면 그값 그대로 넣어준다 FALSE : NotUse, TRUE : H-Map
            m_eRoughAlignUsingDefine = eRoughAlignUsingDefine(bExistautoPaneOffset);
    }
    else if (!db[_T("{1F07A3A0-110F-4FBC-B754-62403FC7F8EB}")].Link(bSave, (long&)m_eRoughAlignUsingDefine))
        m_eRoughAlignUsingDefine = eRoughAlignUsingDefine::eRoughAlignUsingDefine_H_Map;

    return TRUE;
}

std::vector<CString> CVisionProcessingAlign3DPara::ExportAlgoParaToText(
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

    //Module 특성 변수 //채워야 함
    CString strDirection{};

    CString arStrDirectionName[] = {_T("LEFT"), _T("TOP"), _T("RIGHT"), _T("BOTTOM")};

    //Algorithm Parameter 정리

    /********************************************************************* Stitch **********************************************************************/
    strCategory.Format(_T("Stitch"));

    // Run
    strGroup.Format(_T("Run"));

    // Stitchi information
    strGroup.Format(_T("Stitch information"));

    strParaName.Format(_T("Rough align"));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, strParaName, strParaNameAux, m_eRoughAlignUsingDefine));
    strParaName.Format(_T("Pane Extenstion Ratio"));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, strParaName, strParaNameAux, m_fPaneExtensionRatio));
    strParaName.Format(_T("Pattern Size X"));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, strParaName, strParaNameAux, m_fPatternSizeX));
    strParaName.Format(_T("Pattern Size Y"));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, strParaName, strParaNameAux, m_fPatternSizeY));
    strParaName.Format(_T("Moving Error X"));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, strParaName, strParaNameAux, m_fMovingErrorX));
    strParaName.Format(_T("Moving Error Y"));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, strParaName, strParaNameAux, m_fMovingErrorY));
    strParaName.Format(_T("Matching Size Y"));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, strParaName, strParaNameAux, m_fMatchingSize));
    strParaName.Format(_T("Matching Reduce Size X"));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, strParaName, strParaNameAux, m_fMatchingReduceSizeX));

    // Top Stitching Algorithm information
    strGroup.Format(_T("Top Stitching Algorithm information"));

    strParaName.Format(_T("Top Stitch ROI Amount X"));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, strParaName, strParaNameAux, m_nTopStichRoiAmountX));
    strParaName.Format(_T("Top Stitch ROI Amount Y"));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, strParaName, strParaNameAux, m_nTopStichRoiAmountY));
    strParaName.Format(_T("Top Stitch ROI Radius X (um)"));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, strParaName, strParaNameAux, m_topStichRoiRadiusX_um));
    strParaName.Format(_T("Top Stitch ROI Radius Y (um)"));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, strParaName, strParaNameAux, m_topStichRoiRadiusY_um));
    /***************************************************************************************************************************************************/

    return vecstrAlgorithmParameters;
}