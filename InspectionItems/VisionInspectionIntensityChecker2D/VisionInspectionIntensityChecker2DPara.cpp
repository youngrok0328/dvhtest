//CPP_0_________________________________ Precompiled header
#include "stdAfx.h"

//CPP_1_________________________________ Main header
#include "VisionInspectionIntensityChecker2DPara.h"

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
VisionInspectionIntensityChecker2DPara::VisionInspectionIntensityChecker2DPara(VisionProcessing& parent)
    : m_calcFrameIndex(parent, TRUE)
{
    Init();
}

VisionInspectionIntensityChecker2DPara::~VisionInspectionIntensityChecker2DPara(void)
{
    // 벡터 클리어
    for (int nROInth = 0; nROInth < MAX_ROI_NUM; nROInth++)
    {
        m_vec2nROIFrameNum[nROInth].clear();
    }
    m_vec2nROIFrameNum.clear();
    m_vecROI.clear();
}

BOOL VisionInspectionIntensityChecker2DPara::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    long version = 1;
    if (!db[_T("Version")].Link(bSave, version))
        version = 0;

    if (!db[_T("m_nROI1_IntensityAlgorithmMode")].Link(bSave, m_nROI_IntensityAlgorithmMode))
        m_nROI_IntensityAlgorithmMode = INTENSITY_CHECK_MEAN; // enum으로 사용 > 모드 파라미터 헤더에 넣으면 됨

    CString strROITemp;
    CString strGUIDTemp;
    for (int nROInth = 0; nROInth < MAX_ROI_NUM; nROInth++)
    {
        for (int nFramenth = 0; nFramenth < MAX_FRAME_NUM; nFramenth++)
        {
            strGUIDTemp.Format(_T("{02862DD6-BF6E-4C0C-AE35-8AD920DDC5FC}%d_%d"), nROInth, nFramenth);
            if (!db[(strGUIDTemp)].Link(bSave, m_vec2nROIFrameNum[nROInth][nFramenth]))
                m_vec2nROIFrameNum[nROInth][nFramenth] = (nFramenth % 10);
        }

        strGUIDTemp.Format(_T("{3AA40B82-774E-4A79-B3D2-1CE6ECA8590B}%d"), nROInth);
        if (!db[(strGUIDTemp)].Link(bSave, m_vecROI[nROInth]))
            m_vecROI[nROInth] = Ipvm::Rect32s(-1, -1, -1, -1);
    }

    strROITemp.Empty();
    strGUIDTemp.Empty();

    return TRUE;
}

void VisionInspectionIntensityChecker2DPara::Init()
{
    // 파라미터 초기화
    m_vec2nROIFrameNum.resize(MAX_ROI_NUM);
    m_vecROI.resize(MAX_ROI_NUM);

    m_Max_Frame_Num = MAX_FRAME_NUM;
    m_Max_ROI_Num = MAX_ROI_NUM;

    for (int nROInth = 0; nROInth < MAX_ROI_NUM; nROInth++)
    {
        m_vec2nROIFrameNum[nROInth].resize(MAX_FRAME_NUM);

        for (int nFramenth = 0; nFramenth < MAX_FRAME_NUM; nFramenth++)
        {
            m_vec2nROIFrameNum[nROInth][nFramenth] = (nFramenth % 10);
        }
        m_vecROI[nROInth] = Ipvm::Rect32s(-1, -1, -1, -1);
    }
}

std::vector<CString> VisionInspectionIntensityChecker2DPara::ExportAlgoParaToText(
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
    strCategory.Format(_T("Algorithm Parameter"));

    strGroup.Format(_T("ROI intensity algorithm"));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Intensity algorithm mode"), _T(""), m_nROI_IntensityAlgorithmMode));

    long nIdx = 0;
    long nFrameID = -1;
    strGroup.Format(_T("Set Image Frame for ROI %d"), nIdx);
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("First Image Frame ID"), _T(""), m_vec2nROIFrameNum[nIdx][++nFrameID]));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Second Image Frame ID"), _T(""), m_vec2nROIFrameNum[nIdx][++nFrameID]));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Third Image Frame ID"), _T(""), m_vec2nROIFrameNum[nIdx][++nFrameID]));

    nFrameID = -1;
    strGroup.Format(_T("Set Image Frame for ROI %d"), ++nIdx);
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("First Image Frame ID"), _T(""), m_vec2nROIFrameNum[nIdx][++nFrameID]));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Second Image Frame ID"), _T(""), m_vec2nROIFrameNum[nIdx][++nFrameID]));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Third Image Frame ID"), _T(""), m_vec2nROIFrameNum[nIdx][++nFrameID]));

    nFrameID = -1;
    strGroup.Format(_T("Set Image Frame for ROI %d"), ++nIdx);
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("First Image Frame ID"), _T(""), m_vec2nROIFrameNum[nIdx][++nFrameID]));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Second Image Frame ID"), _T(""), m_vec2nROIFrameNum[nIdx][++nFrameID]));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Third Image Frame ID"), _T(""), m_vec2nROIFrameNum[nIdx][++nFrameID]));

    strGroup.Format(_T("ROI"));
    std::vector<CString> vecstrROIs(0);
    for (int nROInth = 0; nROInth < MAX_ROI_NUM; nROInth++)
    {
        strParaName.Format(_T("ROI%d"), nROInth);
        vecstrROIs = CTxtRecipeParameter::GetTextFromParmeter(
            strHeader, strCategory, strGroup, strParaName, m_vecROI[nROInth]);
        vecstrAlgorithmParameters.insert(vecstrAlgorithmParameters.end(), vecstrROIs.begin(), vecstrROIs.end());
    }

    return vecstrAlgorithmParameters;
}