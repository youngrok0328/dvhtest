//CPP_0_________________________________ Precompiled header
#include "stdAfx.h"

//CPP_1_________________________________ Main header
#include "VisionInspectionBgaBallPixelQuality.h"

//CPP_2_________________________________ This project's headers
#include "DlgVisionInspectionBgaBallPixelQuality.h"
#include "VisionInspectionBgaBallPixelQualityPara.h"

//CPP_3_________________________________ Other projects' headers
#include "../../ImageCombineModules/dPI_ippModules/ippModules.h"
#include "../../InformationModule/dPI_DataBase/PackageSpec.h"
#include "../../MemoryModules/VisionReusableMemory/VisionReusableMemory.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionBaseDef.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionUnitAgent.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Algorithm/ImageProcessing.h>
#include <Ipvm/Base/Conversion.h>
#include <Ipvm/Base/EllipseEq32r.h>
#include <Ipvm/Base/Enum.h>
#include <Ipvm/Base/Image32u.h>
#include <Ipvm/Base/Image8u.h>
#include <Ipvm/Base/TimeCheck.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#define ROI_MAX_COUNT 3

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
BOOL VisionInspectionBgaBallPixelQuality::OnInspection()
{
    m_pVisionInspDlg->OnBnClickedButtonInspect();
    return TRUE;
}

BOOL VisionInspectionBgaBallPixelQuality::DoInspection(
    const bool detailSetupMode, const enSideVisionModule i_ModuleStatus)
{
    //mc_Side Vision
    SetCurVisionModule_Status(i_ModuleStatus);
    //

    Ipvm::TimeCheck fTime;

    ResetResult();

    //{{ //kircheis_3DEmpty //검사 결과를 관리하는 민감한 변수니까 무조건 시작할 때 초기화 한다.
    m_bInvalid = FALSE;
    m_bEmpty = FALSE;
    m_bMarginal = FALSE;
    m_bDoubleDevice = FALSE;
    //}}

    // ksy deeplearning
    m_bUse2ndInspection = m_VisionPara->m_use2ndInspection;
    m_str2ndInspCode = m_VisionPara->m_str2ndInspCode;

    //{{ 검사 영역인 Ball ROI 획득 //kircheis_BPQDebug
    if (GetBallROI() == FALSE)
        return FALSE;
    //}}

    // 영훈 20150916 : 앞으로 영상 받는 부분은 통합하여 관리한다.
    Ipvm::Image8u image = GetInspectionFrameImage();
    if (image.GetMem() == nullptr)
        return FALSE;

    Ipvm::Image8u combineImage;

    if (!getReusableMemory().GetInspByteImage(combineImage))
        return FALSE;

    if (!CippModules::GrayImageProcessingManage(
            getReusableMemory(), &image, false, m_VisionPara->m_ImageProcMangePara, combineImage))
    {
        return FALSE;
    }

    BOOL bPass = DoBallPixelQualityInsp(detailSetupMode, combineImage);

    m_fCalcTime = CAST_FLOAT(fTime.Elapsed_ms());
    return bPass;
}

BOOL VisionInspectionBgaBallPixelQuality::GetBallROI() //m_vecrtBallCircleROI에 Ball 좌표 넣기//kircheis_BPQDebug
{
    m_vecrtBallCircleROI.clear();
    BOOL bGetROI = FALSE;

    long nDataNum = 0;
    void* pData = m_visionUnit.GetVisionDebugInfo(_VISION_INSP_GUID_BGA_BALL_2D, _T("Ball Width ROI"), nDataNum);
    if (pData != nullptr && nDataNum > 0)
    {
        m_vecrtBallCircleROI.resize(nDataNum);
        Ipvm::Rect32r* pfrtBallROI = (Ipvm::Rect32r*)pData;
        for (long j = 0; j < nDataNum; j++)
            m_vecrtBallCircleROI[j] = pfrtBallROI[j];
        bGetROI = TRUE;
    }

    return bGetROI;
}

void VisionInspectionBgaBallPixelQuality::SetDebugImageRingMask(
    const bool detailSetupMode, long nRingID, CString strRingID, const Ipvm::Image8u& i_image)
{
    if (nRingID < 0)
        return;
    const long nSize = i_image.GetSizeX() * i_image.GetSizeY();
    if (nSize < 1)
        return;

    CString strDebugInfoName;
    strDebugInfoName.Format(_T("%s Ring Mask"), (LPCTSTR)strRingID);

    SetDebugInfoItem(detailSetupMode, strDebugInfoName, i_image, true);

    strDebugInfoName.Empty();
}

void VisionInspectionBgaBallPixelQuality::SetDebugImageRingBrightPixel(
    const bool detailSetupMode, long nRingID, CString strRingID, const Ipvm::Image8u& i_image)
{
    if (nRingID < 0)
        return;
    const long nSize = i_image.GetSizeX() * i_image.GetSizeY();
    if (nSize < 1)
        return;

    CString strDebugInfoName;
    strDebugInfoName.Format(_T("%s Ring Bright Pixel"), (LPCTSTR)strRingID);

    SetDebugInfoItem(detailSetupMode, strDebugInfoName, i_image, true);

    strDebugInfoName.Empty();
}

void VisionInspectionBgaBallPixelQuality::SetDebugImageRingDarkPixel(
    const bool detailSetupMode, long nRingID, CString strRingID, const Ipvm::Image8u& i_image)
{
    if (nRingID < 0)
        return;
    const long nSize = i_image.GetSizeX() * i_image.GetSizeY();
    if (nSize < 1)
        return;

    CString strDebugInfoName;
    strDebugInfoName.Format(_T("%s Ring Dark Pixel"), (LPCTSTR)strRingID);

    SetDebugInfoItem(detailSetupMode, strDebugInfoName, i_image, true);

    strDebugInfoName.Empty();
}

BOOL VisionInspectionBgaBallPixelQuality::DoBallPixelQualityInsp(const bool detailSetupMode, const Ipvm::Image8u& image)
{
    //{{ 필요한 Data, Image Buffer 수집 및 만들기
    long nBallNum = (long)m_vecrtBallCircleROI.size();
    if (nBallNum <= 0)
        return FALSE;

    long nRingNum = (long)m_VisionPara->m_vecsPixelQualityParam.size();

    Ipvm::Image8u imageRingMask;
    Ipvm::Image8u imageRingGray;

    if (!getReusableMemory().GetInspByteImage(imageRingMask))
        return FALSE;
    if (!getReusableMemory().GetInspByteImage(imageRingGray))
        return FALSE;

    Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(imageRingMask), 0, imageRingMask);
    Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(imageRingGray), 0, imageRingGray);

    std::vector<Ipvm::Rect32s> vecrtRingROI(nBallNum);
    std::vector<Ipvm::EllipseEq32r> vecsBeginEllipse(nBallNum);
    std::vector<Ipvm::EllipseEq32r> vecsEndEllipse(nBallNum);
    std::vector<Ipvm::EllipseEq32r> vecsDebugRadius(0);

    std::vector<long> vecnRingArea(nBallNum);
    std::vector<long> vecnDarkPixelArea(nBallNum);
    long nDarkPixelArea;
    std::vector<Ipvm::Rect32s> vecrtDebugInfoBallArea(nBallNum);

    std::vector<float> vecfBallIntensity(nBallNum);
    std::vector<float> vecfBallFillRatio(nBallNum);
    std::vector<float> vecfBallSumSquare(nBallNum);
    std::vector<float> vecfBallDarkIntenAvr(nBallNum);

    long nLowThresh;
    long nHighThresh = 255;
    //}}

    //{{ 마스크 및 각 Ring 별 검사 영상 만들고 필요한 Data 추출/검사 까지 하자 (안그러면 버퍼를 너무 많이 만들어야 한다.
    Ipvm::Rect32s rtCurROI;
    Ipvm::EllipseEq32r sCurBeginEllipse, sCurEndEllipse;
    BOOL bResult = TRUE;
    BOOL bEraseBegin;
    BOOL bCurInsp;
    std::vector<BOOL> vecbSpecInsp(INSPECT_BALL_PIXEL_QUALITY_ITEM_END_);
    float fBallIntensityAvr = 0;
    float fCurDarkContrast;
    float fDarkConrastMinPxlRatio;
    long nMinPixelCount;
    CString strDebugItemName;
    CString strRingID;
    CString strInspSpecName;
    long nSizeOfLong = nBallNum * sizeof(long);
    long nSizeOfFloat = nBallNum * sizeof(float);

    for (long nRing = 0; nRing < nRingNum; nRing++)
    {
        strRingID = GetStringCount(nRing + 1);
        vecsDebugRadius.clear();
        strDebugItemName.Format(_T("%s Ring Radius"), (LPCTSTR)strRingID);
        //각 data 초기화
        memset(&vecnRingArea[0], 0, nSizeOfLong);
        memset(&vecnDarkPixelArea[0], 0, nSizeOfLong);
        memset(&vecfBallIntensity[0], 0, nSizeOfFloat);
        memset(&vecfBallFillRatio[0], 0, nSizeOfFloat);
        memset(&vecfBallSumSquare[0], 0, nSizeOfFloat);
        memset(&vecfBallDarkIntenAvr[0], 0, nSizeOfFloat);

        //{{검사 여부 학인
        bCurInsp = FALSE;
        for (long nInsp = 0; nInsp < INSPECT_BALL_PIXEL_QUALITY_ITEM_END_; nInsp++)
        {
            if (nInsp % nRingNum != nRing)
                continue;

            strInspSpecName.Format(_T("%s %s"), (LPCTSTR)strRingID, gl_szStrInspectPixelQualityItem[nInsp]);

            VisionInspectionResult* pResult = m_resultGroup.GetResultByName(gl_szStrInspectPixelQualityItem[nInsp]);
            if (pResult == NULL)
                break;
            VisionInspectionSpec* pSpec = GetSpecByName(pResult->m_resultName);
            if (pSpec == NULL)
                break;

            vecbSpecInsp[nInsp] = pSpec->m_use;
            bCurInsp |= vecbSpecInsp[nInsp];
        }

        if (!bCurInsp)
            continue;
        //}}

        fDarkConrastMinPxlRatio = (float)m_VisionPara->m_vecsPixelQualityParam[nRing].nDarkPixelMinRatio * 0.01f;
        MakeBallCircleROI(nBallNum, nRing, &vecrtRingROI[0]);
        MakeBallEllipse(nBallNum, nRing, &vecsBeginEllipse[0], &vecsEndEllipse[0]);
        bEraseBegin = (m_VisionPara->m_vecsPixelQualityParam[nRing].fRingWidthBeginRatio > 0.f);
        fBallIntensityAvr = 0;
        nLowThresh = m_VisionPara->m_vecsPixelQualityParam[nRing].nReferenceIntensity;
        Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(imageRingMask), 0, imageRingMask); //영상에서 검사할 영역 초기화 하고

        for (long nBall = 0; nBall < nBallNum; nBall++)
        {
            //Ref Data 만들기
            rtCurROI = vecrtRingROI[nBall];
            rtCurROI.InflateRect(2, 2);
            sCurBeginEllipse = vecsBeginEllipse[nBall];
            sCurEndEllipse = vecsEndEllipse[nBall];
            vecsDebugRadius.push_back(sCurBeginEllipse);
            vecsDebugRadius.push_back(sCurEndEllipse);

            //영상 만들기
            Ipvm::ImageProcessing::Fill(sCurEndEllipse, 255, imageRingMask); //외곽 링을 채우고
            if (bEraseBegin)
            {
                //내부 링의 Ratio가 0보다 클 때
                Ipvm::ImageProcessing::Fill(sCurBeginEllipse, 0, imageRingMask); //내부 링을 지우고
            }

            Ipvm::ImageProcessing::BitwiseAnd(imageRingMask, image, rtCurROI, imageRingGray); //그레이영상과 마스킹

            //Data 획득
            Ipvm::Image32u histogram(256, 1);

            if (Ipvm::ImageProcessing::MakeHistogram(imageRingMask, rtCurROI, histogram) != Ipvm::Status::e_ok)
            {
                return FALSE;
            }

            vecnRingArea[nBall] = histogram[0][255];
            Ipvm::ImageProcessing::GetMeanInRange(imageRingGray, rtCurROI, 1, 255, vecfBallIntensity[nBall]);
            fBallIntensityAvr += vecfBallIntensity[nBall];
        }
        fBallIntensityAvr /= (float)nBallNum;

        for (long nBall = 0; nBall < nBallNum; nBall++)
        {
            rtCurROI = vecrtRingROI[nBall];
            rtCurROI.InflateRect(2, 2);

            int counts = 0;

            Ipvm::ImageProcessing::GetCountInRange(
                imageRingGray, rtCurROI, CAST_UINT8T(nLowThresh), CAST_UINT8T(nHighThresh), counts);

            vecfBallFillRatio[nBall] = float(counts) / float(vecnRingArea[nBall]);

            nMinPixelCount = (long)((float)vecnRingArea[nBall] * fDarkConrastMinPxlRatio);
            fCurDarkContrast = CippModules::GetMeanInIntensityRange_BYTE(
                imageRingGray, rtCurROI, 1, BYTE(nLowThresh - 1), nMinPixelCount, nDarkPixelArea);
            vecnDarkPixelArea[nBall] = nDarkPixelArea;
            if (fCurDarkContrast == 0.f)
                vecfBallDarkIntenAvr[nBall] = 0.f;
            else
                vecfBallDarkIntenAvr[nBall] = fCurDarkContrast - (float)nLowThresh;
        }
        bResult &= DoFillRatioInsp(nRing, strRingID, nBallNum, &vecfBallFillRatio[0]);
        bResult &= DoContrastInsp(nRing, strRingID, nBallNum, &vecfBallDarkIntenAvr[0]);

        if (detailSetupMode) //순전히 Detail Setup의 Debug View를 위한 구간... 실제 Run 중에 들어오면 난리날거임... 속도 때문에...
        {
            Ipvm::Image8u imageDebug;
            Ipvm::Image8u imageTemp;

            getReusableMemory().GetInspByteImage(imageDebug);
            getReusableMemory().GetInspByteImage(imageTemp);

            Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(imageRingGray), 0, imageRingGray);
            Ipvm::ImageProcessing::Fill(
                Ipvm::Rect32s(imageDebug), 0, imageDebug); //Debug 결과용. 불량 Pixel 결과 View Buf
            Ipvm::ImageProcessing::Fill(
                Ipvm::Rect32s(imageTemp), 0, imageTemp); //Debug 결과용. 정상 Pixel 결과 View Buf

            for (long nBall = 0; nBall < nBallNum; nBall++)
            {
                rtCurROI = vecrtRingROI[nBall];
                rtCurROI.InflateRect(2, 2);

                Ipvm::ImageProcessing::BinarizeGreaterEqual(
                    image, rtCurROI, (BYTE)min(255, max(0, nLowThresh)), imageRingGray);
                Ipvm::ImageProcessing::BitwiseNot(rtCurROI, imageRingGray);

                Ipvm::ImageProcessing::BitwiseAnd(imageRingMask, imageRingGray, rtCurROI, imageDebug);
                Ipvm::ImageProcessing::Subtract(imageRingMask, imageDebug, rtCurROI, 0, imageTemp);
                if (vecsDebugRadius[nBall].m_xradius > 0)
                {
                    vecsDebugRadius[nBall].m_xradius += 1.f; //영상과의 괴리를 줄이기 위함.
                    vecsDebugRadius[nBall].m_yradius += 1.f;
                }
                vecrtDebugInfoBallArea[nBall].m_left = nBall;
                vecrtDebugInfoBallArea[nBall].m_right = vecnRingArea[nBall];
                vecrtDebugInfoBallArea[nBall].m_top = vecnDarkPixelArea[nBall];
                vecrtDebugInfoBallArea[nBall].m_bottom
                    = (long)((float)vecnDarkPixelArea[nBall] / (float)vecnRingArea[nBall] * 100.f + .5f);
            }
            SetDebugInfoItem(detailSetupMode, strDebugItemName, vecsDebugRadius);
            SetDebugImageRingMask(detailSetupMode, nRing, strRingID, imageRingMask);
            SetDebugImageRingBrightPixel(detailSetupMode, nRing, strRingID, imageTemp);
            SetDebugImageRingDarkPixel(detailSetupMode, nRing, strRingID, imageDebug);
            strDebugItemName.Format(
                _T("%s Ring BallID, Ring Area, Dark Pixel Area, Dark Ratio(%%)"), (LPCTSTR)strRingID);
            SetDebugInfoItem(detailSetupMode, strDebugItemName, vecrtDebugInfoBallArea);
        }
    }
    //}}

    strDebugItemName.Empty();
    strRingID.Empty();
    strInspSpecName.Empty();

    return bResult;
}

BOOL VisionInspectionBgaBallPixelQuality::DoFillRatioInsp(
    long nRingID, CString strRingID, long nBallNum, float* pfValue)
{
    CString strInspSpecName;
    strInspSpecName = gl_szStrInspectPixelQualityItem[INSPECT_BALL_PIXEL_QUALITY_ITEM_BEGIN_FILLRATIO + nRingID];

    VisionInspectionResult* pResult = m_resultGroup.GetResultByName(strInspSpecName);
    if (pResult == NULL)
        return FALSE;
    VisionInspectionSpec* pSpec = GetSpecByName(pResult->m_resultName);
    if (pSpec == NULL)
        return FALSE;

    if (pSpec->m_use == FALSE)
        return TRUE;

    pResult->Clear();
    pResult->Resize(nBallNum);
    float fError, fValue;
    Ipvm::Rect32s rtBallROI;

    CString sObjectID = NULL;

    long nSpecBallNum = (long)m_packageSpec.m_ballMap->m_balls.size();

    CString strCompID = m_packageSpec.m_originalballMap->m_strAllGroupName; //kircheis_MED2.5

    for (long nBall = 0; nBall < nBallNum; nBall++)
    {
        auto& ball = m_packageSpec.m_ballMap->m_balls[nBall];
        rtBallROI = Ipvm::Conversion::ToRect32s(m_vecrtBallCircleROI[nBall]);
        fValue = pfValue[nBall] * 100.f;
        fError = fValue;
        fError = (float)min(fError, 100.f);
        fError = (float)max(fError, 0.f);

        if (nSpecBallNum > nBall)
            sObjectID = ball.m_name;
        else
            sObjectID.Format(_T("%d"), nBall);

        pResult->SetValue_EachResult_AndTypicalValueBySingleSpec(
            nBall, sObjectID, ball.m_groupID, strCompID, fError, *pSpec, (float)ball.m_posX_um, (float)ball.m_posY_um);
        pResult->SetRect(nBall, rtBallROI);
    }

    pResult->UpdateTypicalValue(pSpec);
    pResult->SetTotalResult();

    strInspSpecName.Empty();
    sObjectID.Empty();
    strCompID.Empty();

    return TRUE; //불량이 발생해도 검사 실패만 아니면 return TRUE을 날리자. 그래야 계속 검사 한다.
}

BOOL VisionInspectionBgaBallPixelQuality::DoContrastInsp(long nRingID, CString strRingID, long nBallNum, float* pfValue)
{
    CString strInspSpecName;
    strInspSpecName = gl_szStrInspectPixelQualityItem[INSPECT_BALL_PIXEL_QUALITY_ITEM_BEGIN_CONTRAST + nRingID];

    VisionInspectionResult* pResult = m_resultGroup.GetResultByName(strInspSpecName);
    if (pResult == NULL)
        return FALSE;
    VisionInspectionSpec* pSpec = GetSpecByName(pResult->m_resultName);
    if (pSpec == NULL)
        return FALSE;

    if (pSpec->m_use == FALSE)
        return TRUE;

    pResult->Clear();
    pResult->Resize(nBallNum);
    float fError, fValue;
    Ipvm::Rect32s rtBallROI;

    CString sObjectID = NULL;
    long nSpecBallNum = (long)m_packageSpec.m_ballMap->m_balls.size();

    CString strCompID = m_packageSpec.m_originalballMap->m_strAllGroupName; //kircheis_MED2.5

    for (long nBall = 0; nBall < nBallNum; nBall++)
    {
        auto& ball = m_packageSpec.m_ballMap->m_balls[nBall];
        rtBallROI = Ipvm::Conversion::ToRect32s(m_vecrtBallCircleROI[nBall]);
        fValue = pfValue[nBall];
        fError = fValue;

        if (nSpecBallNum > nBall)
            sObjectID = ball.m_name;
        else
            sObjectID.Format(_T("%d"), nBall);

        pResult->SetValue_EachResult_AndTypicalValueBySingleSpec(
            nBall, sObjectID, ball.m_groupID, strCompID, fError, *pSpec, (float)ball.m_posX_um, (float)ball.m_posY_um);
        pResult->SetRect(nBall, rtBallROI);
    }

    pResult->UpdateTypicalValue(pSpec);
    pResult->SetTotalResult();

    strInspSpecName.Empty();
    sObjectID.Empty();
    strCompID.Empty();

    return TRUE; //불량이 발생해도 검사 실패만 아니면 return TRUE을 날리자. 그래야 계속 검사 한다.
}

void VisionInspectionBgaBallPixelQuality::MakeBallEllipse(
    long nBallNum, long nRingID, Ipvm::EllipseEq32r* o_psRingBeginEllipse, Ipvm::EllipseEq32r* psRingEndEllipse)
{
    long nRingNum = (long)m_VisionPara->m_vecsPixelQualityParam.size();
    if (nRingID < 0 || nRingID >= nRingNum || o_psRingBeginEllipse == NULL || psRingEndEllipse == NULL)
        return;

    float fRingWidthBeginRatio = m_VisionPara->m_vecsPixelQualityParam[nRingID].fRingWidthBeginRatio * 0.01f;
    float fRingWidthEndRatio = m_VisionPara->m_vecsPixelQualityParam[nRingID].fRingWidthEndRatio * 0.01f;

    float fWidth, fHeight, fHalfWidth, fHalfHeight, fCalcWidth, fCalcHeight;
    Ipvm::Rect32r frtRefBallROI;
    Ipvm::Point32r2 fptCenter;
    for (long nBall = 0; nBall < nBallNum; nBall++)
    {
        frtRefBallROI = m_vecrtBallCircleROI[nBall];
        fptCenter = frtRefBallROI.CenterPoint();
        fWidth = frtRefBallROI.Width();
        fHeight = frtRefBallROI.Height();
        fHalfWidth = fWidth * .5f;
        fHalfHeight = fHeight * .5f;

        //Ring Begin
        if (m_VisionPara->m_fBallWidthRatio1st != 100.f)
        {
            fCalcWidth = fHalfWidth * fRingWidthBeginRatio;
            fCalcHeight = fHalfHeight * fRingWidthBeginRatio;
        }
        else
        {
            fCalcWidth = fHalfWidth;
            fCalcHeight = fHalfHeight;
        }
        o_psRingBeginEllipse[nBall] = Ipvm::EllipseEq32r(fptCenter.m_x, fptCenter.m_y, fCalcWidth, fCalcHeight);

        //Ring End
        if (m_VisionPara->m_fBallWidthRatio2nd != 100.f)
        {
            fCalcWidth = fHalfWidth * fRingWidthEndRatio;
            fCalcHeight = fHalfHeight * fRingWidthEndRatio;
        }
        else
        {
            fCalcWidth = fHalfWidth;
            fCalcHeight = fHalfHeight;
        }
        psRingEndEllipse[nBall] = Ipvm::EllipseEq32r(fptCenter.m_x, fptCenter.m_y, fCalcWidth, fCalcHeight);
    }
}

void VisionInspectionBgaBallPixelQuality::MakeBallCircleROI(long nBallNum, long nRingID, Ipvm::Rect32s* o_prtEndRingROI)
{
    long nRingNum = (long)m_VisionPara->m_vecsPixelQualityParam.size();
    if (nRingID < 0 || nRingID >= nRingNum || o_prtEndRingROI == NULL)
        return;

    float fRingWidthEndRatio = m_VisionPara->m_vecsPixelQualityParam[nRingID].fRingWidthEndRatio * 0.01f;

    float fWidth, fHeight, fHalfWidth, fHalfHeight, fCalcWidth, fCalcHeight;
    Ipvm::Rect32r frtRefBallROI;
    Ipvm::Point32r2 fptCenter;
    for (long nBall = 0; nBall < nBallNum; nBall++)
    {
        frtRefBallROI = m_vecrtBallCircleROI[nBall];
        fptCenter = frtRefBallROI.CenterPoint();
        fWidth = frtRefBallROI.Width();
        fHeight = frtRefBallROI.Height();
        fHalfWidth = fWidth * .5f;
        fHalfHeight = fHeight * .5f;

        //1st Ring
        if (m_VisionPara->m_fBallWidthRatio1st != 100.f)
        {
            fCalcWidth = fHalfWidth * fRingWidthEndRatio;
            fCalcHeight = fHalfHeight * fRingWidthEndRatio;
        }
        else
        {
            fCalcWidth = fHalfWidth;
            fCalcHeight = fHalfHeight;
        }
        o_prtEndRingROI[nBall] = GetRectFromFloatData(fptCenter, fCalcWidth, fCalcHeight);
    }
}

Ipvm::Rect32s VisionInspectionBgaBallPixelQuality::GetRectFromFloatData(
    Ipvm::Point32r2 fptCenter, float fHalfWidth, float fHalfHeight)
{
    Ipvm::Rect32s rtRes;
    fHalfWidth = (float)max(fHalfWidth, -fHalfWidth); //양수 화
    fHalfHeight = (float)max(fHalfHeight, -fHalfHeight);

    rtRes.m_left = (long)(fptCenter.m_x - fHalfWidth + .5f);
    rtRes.m_right = (long)(fptCenter.m_x + fHalfWidth + .5f);

    rtRes.m_top = (long)(fptCenter.m_y - fHalfHeight + .5f);
    rtRes.m_bottom = (long)(fptCenter.m_y + fHalfHeight + .5f);

    return rtRes;
}
