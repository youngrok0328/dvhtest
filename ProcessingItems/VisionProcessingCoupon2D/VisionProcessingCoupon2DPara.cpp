//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionProcessingCoupon2DPara.h"

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
VisionProcessingCoupon2DPara::VisionProcessingCoupon2DPara(VisionProcessing& parent)
    : m_ImageProcManagePara(parent)
    , m_calcFrameIndex(parent, TRUE)
{
    Init();
}

VisionProcessingCoupon2DPara::~VisionProcessingCoupon2DPara(void)
{
}

BOOL VisionProcessingCoupon2DPara::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    long version = 1;
    if (!db[_T("Version")].Link(bSave, version))
        version = 0;

    m_ImageProcManagePara.LinkDataBase(bSave, db[_T("{4B20B17D-62B6-4485-9F91-45D901B9C491}")]);

    if (!db[_T("{312B0ED1-83D1-4AB4-BAC9-68E11F77CBCB}")].Link(bSave, m_CouponROI))
        m_CouponROI = Ipvm::Rect32s(-1, -1, -1, -1);
    if (!db[_T("{FD6E8140-3F22-46B8-B86C-41118AD5F9CE}")].Link(bSave, m_nMinBlobSize))
        m_nMinBlobSize = m_nMinBlobSize = 5000000;
    if (!db[_T("{AB9F02BB-83D0-419E-9997-774A4A20B784}")].Link(bSave, m_nMinBlobCount))
        m_nMinBlobCount = 0;
    if (!db[_T("{31B12F0F-5691-4E49-B3A1-6FC30345716B}")].Link(bSave, m_nCouponThreshold))
        m_nCouponThreshold = 200;

    return TRUE;
}

void VisionProcessingCoupon2DPara::Init()
{
    // 파라미터 초기화
    m_nCouponThreshold = 200;
    m_nMinBlobSize = 5000000;
    m_nMinBlobCount = 0;
    m_CouponROI = Ipvm::Rect32s(-1, -1, -1, -1); // 작동 확인 후에는 -1-1-1-1로 변경 필요
    m_calcFrameIndex.setFrameIndex(0);
}

std::vector<CString> VisionProcessingCoupon2DPara::ExportAlgoParaToText(
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
        strVisionName, strInspectionModuleName, _T("ImageSelect-Image Combine"));

    vecstrAlgorithmParameters.insert(
        vecstrAlgorithmParameters.end(), vecstrImageCombineParameters.begin(), vecstrImageCombineParameters.end());

    //Module 특성 변수
    //Algorithm Parameters
    strCategory.Format(_T("Algorithm Parameters"));

    //Main Parameters
    strGroup.Format(_T("Blob parameters"));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Image Threshold"), _T(""), m_nCouponThreshold));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Min blob size"), _T(""), m_nMinBlobSize));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Min blob count"), _T(""), m_nMinBlobCount));

    //ROI
    strGroup.Format(_T("ROI"));
    std::vector<CString> vecStrRoiInfoBuffer = CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Coupon Check ROI"), m_CouponROI);
    vecstrAlgorithmParameters.insert(
        vecstrAlgorithmParameters.end(), vecStrRoiInfoBuffer.begin(), vecStrRoiInfoBuffer.end());

    return vecstrAlgorithmParameters;
}