//CPP_0_________________________________ Precompiled header
#include "stdAfx.h"

//CPP_1_________________________________ Main header
#include "VisionInspectionIntensityChecker2D.h"

//CPP_2_________________________________ This project's headers
#include "DlgVisionInspectionIntensityChecker2D.h"
#include "VisionInspectionIntensityChecker2DPara.h"

//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/FPI_RECT.h"
#include "../../DefineModules/dA_Base/iDataType.h"
#include "../../DefineModules/dA_Base/semiinfo.h"
#include "../../SharedCommunicationModules/VisionHostCommon/VisionHostBaseDef.h"
#include "../../UserInterfaceModules/ImageLotView/ImageLotView.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionAlignResult.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionUnitAgent.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Algorithm/ImageProcessing.h>
#include <Ipvm/Base/Conversion.h>
#include <Ipvm/Base/Image32u.h>
#include <Ipvm/Base/Image8u.h>
#include <Ipvm/Base/Point32s2.h>
#include <Ipvm/Base/Rect32r.h>
#include <Ipvm/Base/Rect32s.h>
#include <Ipvm/Base/TimeCheck.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define RESULT_SIZE 3
#define AVG_FRAME_INDEX 0

//CPP_7_________________________________ Implementation body
//
Ipvm::Rect32r VisionInspectionIntensityChecker2D::GetBodyRect()
{
    Ipvm::Rect32r frtBodyRect = Ipvm::Rect32r(0.f, 0.f, 0.f, 0.f);

    long nDataNum = 0;
    void* pData = m_visionUnit.GetVisionDebugInfo(
        m_visionUnit.GetVisionAlignProcessingModuleGUID(), _T("EDGE Align Result"), nDataNum);
    if (pData != nullptr && nDataNum > 0 && m_bUseBypassMode == false)
    {
        *m_sEdgeAlignResult = *(VisionAlignResult*)pData;
        frtBodyRect = m_sEdgeAlignResult->getBodyRect();
    }
    else
    {
        frtBodyRect = Ipvm::Rect32r(0.f, 0.f, 5120.f, 5120.f);
    }

    return frtBodyRect;
}

BOOL VisionInspectionIntensityChecker2D::OnInspection()
{
    m_pVisionInspDlg->OnBnClickedButtonInspect();

    return TRUE;
}

BOOL VisionInspectionIntensityChecker2D::DoInspection(
    const bool detailSetupMode, const enSideVisionModule i_ModuleStatus)
{
    //mc_Side Vision
    SetCurVisionModule_Status(i_ModuleStatus);
    //

    //{{ //kircheis_3DEmpty //검사 결과를 관리하는 민감한 변수니까 무조건 시작할 때 초기화 한다.
    m_bInvalid = FALSE;
    m_bEmpty = FALSE;
    m_bMarginal = FALSE;
    m_bDoubleDevice = FALSE;
    //}}

    Ipvm::TimeCheck time_Insp;

    BOOL bResult = TRUE;

    // Init...
    ResetResult();
    if (m_VisionPara->m_vecROI[0].m_bottom < 0 && m_bUseBypassMode == false)
    {
        // ROI 초기화
        ResetROI();
        m_rtPaneRect = Ipvm::Conversion::ToRect32s(GetBodyRect());
        m_ptPaneCenter = m_rtPaneRect.CenterPoint32r();
    }

    if (m_pVisionInspDlg != NULL && m_pVisionInspDlg->IsWindowVisible())
    {
        // Teaching
        if (detailSetupMode)
        {
            m_pVisionInspDlg->GetROI();
        }

        m_pVisionInspDlg->m_imageLotView->Overlay_RemoveAll();
    }

    m_vecbItemUsage.resize(0);
    m_vecIntensityAVG.resize(0);
    m_vecIntensityMin.resize(0);
    m_vecIntensityMax.resize(0);

    bResult = DoInsp(detailSetupMode);

    m_fCalcTime = CAST_FLOAT(time_Insp.Elapsed_ms());

    return bResult;
}

BOOL VisionInspectionIntensityChecker2D::DoInsp(const bool detailSetupMode)
{
    BOOL bResult = true;

    for (int nROInth = 0; nROInth < MAX_ROI_NUM; nROInth++)
    {
        for (int nFramenth = 0; nFramenth < MAX_FRAME_NUM; nFramenth++)
        {
            Ipvm::Image8u image;

            image = GetInspectionFrameImage(FALSE, m_VisionPara->m_vec2nROIFrameNum[nROInth][nFramenth]);

            bResult = InspIntensityCheck(detailSetupMode, (nROInth * MAX_FRAME_NUM) + nFramenth, image);
        }
    }

    SetDebugInfoItem(detailSetupMode, _T("Intensity Checker Usage"), m_vecbItemUsage, TRUE);
    SetDebugInfoItem(detailSetupMode, _T("Intensity AVG"), m_vecIntensityAVG, TRUE);
    SetDebugInfoItem(detailSetupMode, _T("Intensity Max"), m_vecIntensityMax, TRUE);
    SetDebugInfoItem(detailSetupMode, _T("Intensity Min"), m_vecIntensityMin, TRUE);

    //해당 검사는 항상 pass를 나타낼 것이므로 그냥 true를 리턴한다.
    return true;
}

BOOL VisionInspectionIntensityChecker2D::InspIntensityCheck(
    const bool detailSetupMode, const long InspectionID, const Ipvm::Image8u i_image)
{
    UNREFERENCED_PARAMETER(detailSetupMode);
    // 스펙 데이터 받아오기
    auto* result = m_resultGroup.GetResultByName(g_szIntensityCheckerInspection2DName[InspectionID]);
    if (result == NULL)
    {
        return FALSE;
    }

    auto* spec = GetSpecByName(result->m_resultName);
    if (spec == NULL)
    {
        return FALSE;
    }

    if (spec->m_use == FALSE)
    {
        m_vecbItemUsage.push_back(FALSE);
        m_vecIntensityAVG.push_back(-1);
        m_vecIntensityMin.push_back((long)-1);
        m_vecIntensityMax.push_back((long)-1);
        return TRUE;
    }

    // ROI 및 frame 정보 계산
    int nROInth = InspectionID / MAX_FRAME_NUM;
    //int nFramenth = InspectionID % MAX_FRAME_NUM;

    Ipvm::Rect32s Roi = m_VisionPara->m_vecROI[nROInth];

    // 검사항목 수만큼 결과 필요함
    result->Resize(RESULT_SIZE);

    // 영역 검출
    Ipvm::Rect32s validRoi = Roi & Ipvm::Rect32s(i_image);

    double dAvgValue = 0.;
    BYTE nMinValue = 0;
    BYTE nMaxValue = 0;

    // 유효한 영역이 없을 경우 에러코드 -1을 입력한다.
    if (validRoi.IsRectEmpty())
    {
        dAvgValue = -1;

        result->SetValue_EachResult_AndTypicalValueBySingleSpec(0, _T("AVG"), CAST_FLOAT(dAvgValue), *spec,
            (float)validRoi.CenterPoint().m_x, (float)validRoi.CenterPoint().m_y, CAST_FLOAT(dAvgValue), PASS);
        result->SetValue_EachResult_AndTypicalValueBySingleSpec(1, _T("Min"), CAST_FLOAT(dAvgValue), *spec,
            (float)validRoi.CenterPoint().m_x, (float)validRoi.CenterPoint().m_y, CAST_FLOAT(dAvgValue), PASS);
        result->SetValue_EachResult_AndTypicalValueBySingleSpec(2, _T("Max"), CAST_FLOAT(dAvgValue), *spec,
            (float)validRoi.CenterPoint().m_x, (float)validRoi.CenterPoint().m_y, CAST_FLOAT(dAvgValue), PASS);

        // ROI 설정
        for (long nObjNum = 0; nObjNum < RESULT_SIZE; nObjNum++)
        {
            result->SetRect(nObjNum, Roi);
        }

        result->UpdateTypicalValue(spec);
        result->SetTotalResult();

        return true;
    }

    Ipvm::ImageProcessing::GetMax(i_image, validRoi, nMaxValue);
    Ipvm::ImageProcessing::GetMin(i_image, validRoi, nMinValue);

    // 평균 방식으로 데이터 계산
    if (m_VisionPara->m_nROI_IntensityAlgorithmMode == INTENSITY_CHECK_MEAN)
    {
        Ipvm::ImageProcessing::GetMean(
            i_image, validRoi, dAvgValue); //Ipvm::ImageProcessing::GetMean() //평균 계산 > roi 사용함
    }

    // 캘리브레이션 방식
    else if (m_VisionPara->m_nROI_IntensityAlgorithmMode == INTENSITY_CHECK_CALIBRATION)
    {
        GetCalibration(i_image, validRoi, dAvgValue);
    }

    // 결과 전달
    result->SetValue_EachResult_AndTypicalValueBySingleSpec(0, _T("AVG"), CAST_FLOAT(dAvgValue), *spec,
        (float)validRoi.CenterPoint().m_x, (float)validRoi.CenterPoint().m_y, CAST_FLOAT(dAvgValue), PASS);
    result->SetValue_EachResult_AndTypicalValueBySingleSpec(1, _T("Min"), CAST_FLOAT(nMinValue), *spec,
        (float)validRoi.CenterPoint().m_x, (float)validRoi.CenterPoint().m_y, CAST_FLOAT(nMinValue), PASS);
    result->SetValue_EachResult_AndTypicalValueBySingleSpec(2, _T("Max"), CAST_FLOAT(nMaxValue), *spec,
        (float)validRoi.CenterPoint().m_x, (float)validRoi.CenterPoint().m_y, CAST_FLOAT(nMaxValue), PASS);

    m_vecbItemUsage.push_back(TRUE);
    m_vecIntensityAVG.push_back(dAvgValue);
    m_vecIntensityMin.push_back((long)nMinValue);
    m_vecIntensityMax.push_back((long)nMaxValue);

    // ROI 설정
    for (long nObjNum = 0; nObjNum < RESULT_SIZE; nObjNum++)
    {
        result->SetRect(nObjNum, validRoi);
    }

    //result->UpdateTypicalValue(spec);

    // AVG 값이 대푯값으로 표시되도록 하기 위해서 1번째 값이 Worst Value로 지정하였다.
    result->m_worstObjectIndex = AVG_FRAME_INDEX;
    result->SetTotalResult();

    return true;
}

BOOL VisionInspectionIntensityChecker2D::GetCalibration(
    const Ipvm::Image8u& i_image, const Ipvm::Rect32s& validRoi, double& calibrationValue)
{
    // 히스토그램 추출
    m_histogram = new Ipvm::Image32u(256, 1);

    Ipvm::ImageProcessing::MakeHistogram(i_image, validRoi, *m_histogram);

    long nMaxCount = 0;
    long nMaxIndex = 0;

    for (long x = 0; x < 255; x++)
    {
        if (static_cast<long>((*m_histogram)[0][x]) > nMaxCount)
        {
            nMaxCount = (*m_histogram)[0][x];
            nMaxIndex = x;
        }
    }

    // 최대 군집에서 절반까지 수집한 뒤 해당 밝기로 평균
    double weightedIntensitySum = 0;
    double weightSum = 0;
    const long maxCountHalf = nMaxCount / 2;

    for (long nIntensity = nMaxIndex - 1; nIntensity >= 0; nIntensity--)
    {
        const auto weight = (*m_histogram)[0][nIntensity];

        if (static_cast<long>(weight) < maxCountHalf)
        {
            break;
        }

        weightedIntensitySum += nIntensity * weight;
        weightSum += weight;
    }

    for (long nIntensity = nMaxIndex; nIntensity < 256; nIntensity++)
    {
        const auto weight = (*m_histogram)[0][nIntensity];

        if (static_cast<long>(weight) < maxCountHalf)
        {
            break;
        }

        weightedIntensitySum += nIntensity * weight;
        weightSum += weight;
    }

    // 분모가 0일 경우에는 유효하지 않은 데이터가 남는다. 따라서 수정
    if (weightSum > 0)
    {
        calibrationValue = float(weightedIntensitySum / weightSum);
    }

    else
    {
        calibrationValue = 0;
    }

    delete m_histogram;

    return TRUE;
}

// 패키지 사이즈 정보를 기준으로 RESET ROI 수정 기본값이 삼각형이므로 3개마다 돌아가면서 발생하도록 만들었다.
void VisionInspectionIntensityChecker2D::ResetROI()
{
    for (int nROInth = 0; nROInth < MAX_ROI_NUM; nROInth++)
    {
        if ((nROInth % 3) == 0)
        {
            m_VisionPara->m_vecROI[nROInth]
                = Ipvm::Conversion::ToRect32s(Ipvm::Rect32r(m_rtPaneRect.m_left + (m_rtPaneRect.Width() * 0.5) - 150,
                    m_rtPaneRect.m_top + (m_rtPaneRect.Height() * 0.25) - 150,
                    m_rtPaneRect.m_left + (m_rtPaneRect.Width() * 0.5) + 150,
                    m_rtPaneRect.m_top + (m_rtPaneRect.Height() * 0.25) + 150));
        }

        else if ((nROInth % 3) == 1)
        {
            m_VisionPara->m_vecROI[nROInth]
                = Ipvm::Conversion::ToRect32s(Ipvm::Rect32r(m_rtPaneRect.m_left + (m_rtPaneRect.Width() * 0.25) - 150,
                    m_rtPaneRect.m_top + (m_rtPaneRect.Height() * 0.75) - 150,
                    m_rtPaneRect.m_left + (m_rtPaneRect.Width() * 0.25) + 150,
                    m_rtPaneRect.m_top + (m_rtPaneRect.Height() * 0.75) + 150));
        }

        else
        {
            m_VisionPara->m_vecROI[nROInth]
                = Ipvm::Conversion::ToRect32s(Ipvm::Rect32r(m_rtPaneRect.m_left + (m_rtPaneRect.Width() * 0.75) - 150,
                    m_rtPaneRect.m_top + (m_rtPaneRect.Height() * 0.75) - 150,
                    m_rtPaneRect.m_left + (m_rtPaneRect.Width() * 0.75) + 150,
                    m_rtPaneRect.m_top + (m_rtPaneRect.Height() * 0.75) + 150));
        }
    }
}

void VisionInspectionIntensityChecker2D::SetSameROI()
{
    if (MAX_ROI_NUM > 2)
    {
        for (int nROInth = 1; nROInth < MAX_ROI_NUM; nROInth++)
        {
            m_VisionPara->m_vecROI[nROInth] = m_VisionPara->m_vecROI[0];
        }
    }
}