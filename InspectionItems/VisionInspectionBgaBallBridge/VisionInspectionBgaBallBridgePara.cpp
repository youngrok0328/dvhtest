//CPP_0_________________________________ Precompiled header
#include "stdAfx.h"

//CPP_1_________________________________ Main header
#include "VisionInspectionBgaBallBridgePara.h"

//CPP_2_________________________________ This project's headers
#include "VisionInspectionBgaBallBridge.h"

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
VisionInspectionBgaBallBridgePara::VisionInspectionBgaBallBridgePara(VisionProcessing& parent)
    : m_ImageProcManagePara(parent)
{
    Init();
}

VisionInspectionBgaBallBridgePara::~VisionInspectionBgaBallBridgePara(void)
{
}

BOOL VisionInspectionBgaBallBridgePara::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    CString strTemp;
    CString strKey;

    if (!bSave)
    {
        Init();
    }

    strKey.Format(_T("VisionInspectionBgaBallBridgePara"));

    m_ImageProcManagePara.LinkDataBase(bSave, db[_T("{31BC1D83-D2E5-4D05-87AB-E9139E5403F4}")]);

    strTemp.Format(_T("BallLinkRangeRatio"));
    if (!db[strTemp].Link(bSave, m_fBallLinkRangeRatio))
        m_fBallLinkRangeRatio = 150.;

    if (!bSave && (m_fBallLinkRangeRatio < 80. || m_fBallLinkRangeRatio > 250.))
        m_fBallLinkRangeRatio = 150.;

    strTemp.Format(_T("BallIgnoreRatio"));
    if (!db[strTemp].Link(bSave, m_fBallIgnoreRatio))
        m_fBallIgnoreRatio = 110.;

    if (!bSave && (m_fBallIgnoreRatio < 90. || m_fBallIgnoreRatio > 150.))
        m_fBallIgnoreRatio = 110.;

    strTemp.Format(_T("RoiWidthRatio"));
    if (!db[strTemp].Link(bSave, m_fROI_WidthRatio))
        m_fROI_WidthRatio = 100.;
    if (!bSave && (m_fROI_WidthRatio < 50. || m_fROI_WidthRatio > 150.))
        m_fROI_WidthRatio = 100.;

    strTemp.Format(_T("ThresholdValue"));
    if (!db[strTemp].Link(bSave, m_nThresholdValue))
        m_nThresholdValue = 220;

    strTemp.Empty();
    strKey.Empty();

    return TRUE;
}

void VisionInspectionBgaBallBridgePara::Init()
{
    m_ImageProcManagePara.Init();
    m_fBallLinkRangeRatio = 150.;
    m_fBallIgnoreRatio = 110.;
    m_fROI_WidthRatio = 100.;
    m_nThresholdValue = 220;

    m_use2ndInspection = FALSE;
    m_str2ndInspCode = _T("");
}

std::vector<CString> VisionInspectionBgaBallBridgePara::ExportAlgoParaToText(
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
        strVisionName, strInspectionModuleName, _T("Image Combine"));

    vecstrAlgorithmParameters.insert(
        vecstrAlgorithmParameters.end(), vecstrImageCombineParameters.begin(), vecstrImageCombineParameters.end());

    //Module 특성 변수 //채워야 함
    //Algorithm Parameters
    strCategory.Format(_T("Algorithm Parameters"));

    //Main Parameters
    strGroup.Format(_T("Inspection Parameter"));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Link range ratio of the ball"), _T(""), m_fBallLinkRangeRatio));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Ball ignore ratio"), _T(""), m_fBallIgnoreRatio));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Inspection ROI width ratio"), _T(""), m_fROI_WidthRatio));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Threshold"), _T(""), m_nThresholdValue));

    //DL Parameters
    strCategory.Format(_T("Deep Learning Parameters"));
    strGroup.Format(_T("2nd Inspection Parameter (Deep Learning)"));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Use Inspection"), _T(""), (bool)m_use2ndInspection));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("DL Model Match Code"), _T(""), m_str2ndInspCode));

    return vecstrAlgorithmParameters;
}
