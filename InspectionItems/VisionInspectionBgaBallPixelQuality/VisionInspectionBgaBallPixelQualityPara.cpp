//CPP_0_________________________________ Precompiled header
#include "stdAfx.h"

//CPP_1_________________________________ Main header
#include "VisionInspectionBgaBallPixelQualityPara.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/SimpleFunction.h"
#include "../../DefineModules/dA_Base/TxtRecipeParameter.h" //kircheis_TxtRecipe
#include "../../SharedCommunicationModules/VisionHostCommon/DBObject.h"

//CPP_4_________________________________ External library headers
#include <map>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
//CPP_7_________________________________ Implementation body
//
VisionInspectionBgaBallPixelQualityPara::VisionInspectionBgaBallPixelQualityPara(VisionProcessing& parent)
    : m_vecsPixelQualityParam(5)
    , m_ImageProcMangePara(parent)
{
    Init();
}

VisionInspectionBgaBallPixelQualityPara::~VisionInspectionBgaBallPixelQualityPara(void)
{
}

BOOL VisionInspectionBgaBallPixelQualityPara::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    CString strTemp;
    CString strKey;

    if (!bSave)
    {
        Init();
    }

    strKey.Format(_T("VisionInspectionBgaBallPixelQualityPara"));

    CString strRing;
    for (long nRing = 0; nRing < m_vecsPixelQualityParam.size(); nRing++)
    {
        strRing.Format(_T("Ring%d"), nRing);

        strTemp.Format(_T("%s_WidthBeginRatio"), (LPCTSTR)strRing);
        if (!db[strTemp].Link(bSave, m_vecsPixelQualityParam[nRing].fRingWidthBeginRatio))
            m_vecsPixelQualityParam[nRing].fRingWidthBeginRatio = 30.f;

        strTemp.Format(_T("%s_WidthEndRatio"), (LPCTSTR)strRing);
        if (!db[strTemp].Link(bSave, m_vecsPixelQualityParam[nRing].fRingWidthEndRatio))
            m_vecsPixelQualityParam[nRing].fRingWidthEndRatio = 90.f;

        if (m_vecsPixelQualityParam[nRing].fRingWidthBeginRatio == 0.f
            && m_vecsPixelQualityParam[nRing].fRingWidthEndRatio == 0.f) //일단 초기값은 정해 놓자
        {
            m_vecsPixelQualityParam[nRing].fRingWidthBeginRatio = 30.f;
            m_vecsPixelQualityParam[nRing].fRingWidthEndRatio = 90.f;
        }

        strTemp.Format(_T("%s_Reference Intensity"), (LPCTSTR)strRing);
        if (!db[strTemp].Link(bSave, m_vecsPixelQualityParam[nRing].nReferenceIntensity))
            m_vecsPixelQualityParam[nRing].nReferenceIntensity = 200;

        strTemp.Format(_T("%s_Dark Pixel Minimum Effective Ratio"), (LPCTSTR)strRing);
        if (!db[strTemp].Link(bSave, m_vecsPixelQualityParam[nRing].nDarkPixelMinRatio))
            m_vecsPixelQualityParam[nRing].nDarkPixelMinRatio = 5;
    }

    m_ImageProcMangePara.LinkDataBase(bSave, db[_T("{6F4EB198-D633-426A-80DE-0F285C36FAA8}")]);

    if (!db[_T("Use2ndInspection")].Link(bSave, m_use2ndInspection))
        m_use2ndInspection = FALSE;
    if (!db[_T("str2ndInspCode")].Link(bSave, m_str2ndInspCode))
        m_str2ndInspCode = _T("No Code");

    strTemp.Empty();
    strKey.Empty();
    strRing.Empty();

    return TRUE;
}

void VisionInspectionBgaBallPixelQualityPara::Init()
{
    for (auto& param : m_vecsPixelQualityParam)
    {
        param.Init();
    }

    m_use2ndInspection = FALSE;
    m_str2ndInspCode = _T("");
}

std::vector<CString> VisionInspectionBgaBallPixelQualityPara::ExportAlgoParaToText(
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
    vecstrImageCombineParameters = m_ImageProcMangePara.ExportImageCombineParaToText(
        strVisionName, strInspectionModuleName, _T("Image Combine"));

    vecstrAlgorithmParameters.insert(
        vecstrAlgorithmParameters.end(), vecstrImageCombineParameters.begin(), vecstrImageCombineParameters.end());

    //Module 특성 변수
    strCategory.Format(_T("Algorithm Parameters"));

    //Main Parameters
    long nNumberOfRings = (long)m_vecsPixelQualityParam.size();

    for (long nRing = 0; nRing < nNumberOfRings; nRing++)
    {
        strGroup.Format(_T("%s Ring Option"), (LPCTSTR)(SimpleFunction::GetStringCount(nRing + 1)));

        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
            _T("Ring Width Begin Ratio"), _T(""), m_vecsPixelQualityParam[nRing].fRingWidthBeginRatio));
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
            _T("Ring Width End Ratio"), _T(""), m_vecsPixelQualityParam[nRing].fRingWidthEndRatio));
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
            _T("Reference Threshold Value"), _T(""), m_vecsPixelQualityParam[nRing].nReferenceIntensity));
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
            _T("Dark Pixel Min. Effective Ratio"), _T(""), m_vecsPixelQualityParam[nRing].nDarkPixelMinRatio));
    }

    //DL Parameters
    strCategory.Format(_T("Deep Learning Parameters"));
    strGroup.Format(_T("2nd Inspection Parameter (Deep Learning)"));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Use Inspection"), _T(""), (bool)m_use2ndInspection));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("DL Model Match Code"), _T(""), m_str2ndInspCode));

    return vecstrAlgorithmParameters;
}
