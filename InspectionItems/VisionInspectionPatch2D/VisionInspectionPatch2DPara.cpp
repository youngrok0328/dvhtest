//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionInspectionPatch2DPara.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/TxtRecipeParameter.h" //kircheis_TxtRecipe
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
VisionInspectionPatch2DPara::VisionInspectionPatch2DPara(VisionProcessing& parent)
    : m_ImageProcManagePara(parent)
{
}

VisionInspectionPatch2DPara::~VisionInspectionPatch2DPara(void)
{
}

BOOL VisionInspectionPatch2DPara::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    long version = 1;
    if (!db[_T("Version")].Link(bSave, version))
        version = 0;

    m_ImageProcManagePara.LinkDataBase(bSave, db[_T("{33D3A8D3-24CA-4E98-B68A-D5C846286AD4}")]);

    CString strTemp;

    if (!db[_T("m_nEdgeDirection")].Link(bSave, m_nEdgeDirection))
        m_nEdgeDirection = PI_ED_DIR_FALLING;
    if (!db[_T("m_nSearchDirection")].Link(bSave, m_nSearchDirection))
        m_nSearchDirection = PI_ED_DIR_OUTER;
    if (!db[_T("m_nEdgeDetectMode")].Link(bSave, m_nEdgeDetectMode))
        m_nEdgeDetectMode = PI_ED_MAX_EDGE;
    if (!db[_T("m_nFirstEdgeValue")].Link(bSave, m_nFirstEdgeValue))
        m_nFirstEdgeValue = 10;
    if (!db[_T("m_nEdgeSearchLength_um")].Link(bSave, m_nEdgeSearchLength_um))
        m_nEdgeSearchLength_um = 500;
    if (!db[_T("m_nEdgeSearchWidthRatio")].Link(bSave, m_nEdgeSearchWidthRatio))
        m_nEdgeSearchWidthRatio = 80;
    if (!db[_T("m_nEdgeDetectGap_um")].Link(bSave, m_nEdgeDetectGap_um))
        m_nEdgeDetectGap_um = 500;

    strTemp.Empty();

    return TRUE;
}

std::vector<CString> VisionInspectionPatch2DPara::ExportAlgoParaToText(
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
    std::vector<CString> vecstrImageCombineParameters(0);
    vecstrImageCombineParameters = m_ImageProcManagePara.ExportImageCombineParaToText(
        strVisionName, strInspectionModuleName, _T("Patch align - Image Combine"));

    vecstrAlgorithmParameters.insert(
        vecstrAlgorithmParameters.end(), vecstrImageCombineParameters.begin(), vecstrImageCombineParameters.end());

    //Module 특성 변수
    strCategory.Format(_T("Algorithm Parameter"));

    strGroup.Format(_T("Edge detect parameter"));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Search direction"), _T(""), m_nSearchDirection));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Edge direction"), _T(""), m_nEdgeDirection));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Edge detect mode"), _T(""), m_nEdgeDetectMode));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("First edge value"), _T(""), m_nFirstEdgeValue));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Edge search length (um)"), _T(""), m_nEdgeSearchLength_um));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Edge search width ratio (%)"), _T(""), m_nEdgeSearchWidthRatio));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Detected edge gap (um)"), _T(""), m_nEdgeDetectGap_um));

    return vecstrAlgorithmParameters;
}