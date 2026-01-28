//CPP_0_________________________________ Precompiled header
#include "stdAfx.h"

//CPP_1_________________________________ Main header
#include "VisionInspectionBgaBallBridge.h"

//CPP_2_________________________________ This project's headers
#include "DlgVisionInspectionBgaBallBridge.h"
#include "VisionInspectionBgaBallBridgePara.h"

//CPP_3_________________________________ Other projects' headers
#include "../../AlgorithmModules/dPI_Blob/dPI_BlobLib.h"
#include "../../AlgorithmModules/dPI_Geometry/dPI_GeometryLib.h"
#include "../../DefineModules/dA_Base/ConversionEx.h"
#include "../../DefineModules/dA_Base/PI_RECT.h"
#include "../../DefineModules/dA_Base/VisionScale.h"
#include "../../ImageCombineModules/dPI_ippModules/ippModules.h"
#include "../../InformationModule/dPI_DataBase/PackageSpec.h"
#include "../../MemoryModules/VisionReusableMemory/SurfaceLayerMask.h" //kircheis_MED3
#include "../../MemoryModules/VisionReusableMemory/VisionReusableMemory.h"
#include "../../SharedCommunicationModules/VisionHostCommon/VisionHostBaseDef.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionBaseDef.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionImageLotInsp.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionUnitAgent.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Algorithm/ImageProcessing.h>
#include <Ipvm/Base/Conversion.h>
#include <Ipvm/Base/EllipseEq32r.h>
#include <Ipvm/Base/Image32s.h>
#include <Ipvm/Base/LineSeg32r.h>
#include <Ipvm/Base/Quadrangle32r.h>
#include <Ipvm/Base/TimeCheck.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
BOOL VisionInspectionBgaBallBridge::OnInspection()
{
    m_pVisionInspDlg->OnBnClickedButtonInspect();
    return TRUE;
}

BOOL VisionInspectionBgaBallBridge::DoInspection(const bool detailSetupMode, const enSideVisionModule i_ModuleStatus)
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

    Ipvm::TimeCheck time;

    ResetResult();

    BOOL bInsp = DoInsp(detailSetupMode);

    // Debug Info...
    SetDebugInfo(detailSetupMode);

    m_fCalcTime = CAST_FLOAT(time.Elapsed_ms());

    return bInsp;
}

BOOL VisionInspectionBgaBallBridge::DoInsp(const bool detailSetupMode)
{
    BOOL bModuleInsp = false;
    for (auto& spec : m_fixedInspectionSpecs)
        bModuleInsp |= spec.m_use;

    if (!bModuleInsp)
        return FALSE;

    //Ball Link Data를 가져온다. Ball Align Result중 Circle Debug Info를 가져온다.  SetDebugInfoItem(detailSetupMode, _T("Ball Width Circle"), vecsEllipse, true);
    if (!MakeRefData(detailSetupMode))
        return FALSE;

    MakeBallBridgeMask(detailSetupMode); //kircheis_MED3

    SetDebugInfo(detailSetupMode);

    //검사 영상 가져오기, 이진화용 영상 버퍼 만들기
    Ipvm::Image8u image = GetInspectionFrameImage();
    if (image.GetMem() == nullptr)
        return false;

    Ipvm::Image8u combineImage;
    if (!getReusableMemory().GetInspByteImage(combineImage))
        return false;
    if (!CippModules::GrayImageProcessingManage(
            getReusableMemory(), &image, false, m_VisionPara->m_ImageProcManagePara, combineImage))
    {
        return FALSE;
    }

    BOOL bInspResult = InspBallBridge(detailSetupMode, combineImage);

    return bInspResult;
}

BOOL VisionInspectionBgaBallBridge::InspBallBridge(const bool detailSetupMode, Ipvm::Image8u& combineImage)
{
    //검사 준비 //Result가져오기, Spec 가져오기, Spec의 bUse 확인하기
    VisionInspectionResult* pResult = m_resultGroup.GetResultByName(m_strModuleName);
    if (pResult == NULL)
    {
        if (detailSetupMode == true)
            AfxMessageBox(_T("BallBridge Result is NULL"));
        return FALSE;
    }
    VisionInspectionSpec* pSpec = GetSpecByName(pResult->m_resultName);
    if (pSpec == NULL)
    {
        if (detailSetupMode == true)
            AfxMessageBox(_T("BallBridge Spec is NULL"));
        return FALSE;
    }
    if (!pSpec->m_use)
        return true;

    long nBallNum = (long)m_vecrtBallCircle.size();
    long nBallSpecNum = (long)m_packageSpec.m_ballMap->m_balls.size();

    const long nValidMinInspAreaWidth = (long)(getScale().convert_umToPixelX(50.f) + .5f); //kircheis_BallBidgeModified
    const long nInplateOffset = (long)(200.f / getScale().pixelToUmXY() + .5f); //항상 200um 만큼 Reject ROI를 키운다

    if (nBallNum <= 0 || nBallNum != nBallSpecNum)
        return FALSE;

    pResult->Clear();
    pResult->Resize(nBallNum);

    Ipvm::Image8u calcImage;
    if (!getReusableMemory().GetInspByteImage(calcImage))
        return false;
    Ipvm::ImageProcessing::Copy(combineImage, Ipvm::Rect32s(combineImage), calcImage);

    Ipvm::Image8u rotateImage;
    if (!getReusableMemory().GetInspByteImage(rotateImage))
        return false;
    Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(rotateImage), 0, rotateImage);

    Ipvm::Image8u thresholdImage;
    if (!getReusableMemory().GetInspByteImage(thresholdImage))
        return false;
    Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(thresholdImage), 0, thresholdImage);

    auto* pBlob = getReusableMemory().GetBlob();
    Ipvm::Image32s labelImage;
    if (!getReusableMemory().GetInspLongImage(labelImage))
        return FALSE;

    const long nMaxBlobNum = 50;
    int32_t nCurBlobNum(0);
    long nMaxBlobArea(0), nBlobMaxWidth(0);
    float fLinkAngle(0.f);
    float fWidthRatio(0.f), fWidthRatioMax(0.f);
    Ipvm::Rect32s rtRotateROI(0, 0, 0, 0), rtBlobROI(0, 0, 0, 0), rtObjROI(0, 0, 0, 0), rtResultROI(0, 0, 0, 0);
    Ipvm::Point32r2 fptCurBallCenter(0.f, 0.f);
    PI_RECT piRect;

    Ipvm::EllipseEq32r ballEllipseBuf;
    for (auto& ballEllipse : m_vecrtBallCircle)
    {
        ballEllipseBuf = ballEllipse;
        ballEllipseBuf.m_xradius -= 1.f;
        ballEllipseBuf.m_yradius -= 1.f;
        Ipvm::ImageProcessing::Fill(ballEllipseBuf, 0, calcImage);
    }

    CString strCompID = m_packageSpec.m_originalballMap->m_strAllGroupName; //kircheis_MED2.5
    for (long nBall = 0; nBall < nBallNum; nBall++)
    {
        auto& ball = m_packageSpec.m_ballMap->m_balls[nBall];
        auto& ballEllipse = m_vecrtBallCircle[nBall];
        rtResultROI = m_vecrtBallROI[nBall];
        fptCurBallCenter.Set(ballEllipse.m_x, ballEllipse.m_y);

        fWidthRatioMax = fWidthRatio = 0.f;

        if (ball.m_vecLinkBallID.size() == 0)
        {
            pResult->SetValue_EachResult_AndTypicalValueBySingleSpec(ball.m_index, ball.m_name, ball.m_groupID,
                strCompID, 0.f, *pSpec, (float)ball.m_posX_um, (float)ball.m_posY_um, 0.f, PASS); //kircheis_VSV
            pResult->SetRect(ball.m_index, rtResultROI);
            continue;
        }

        //{{//kircheis_BallBidgeModified //영상 회전부에 문제가 있었다. Ball Bridge 검사 영역을 회전을 시키는데 특정 각도 이상이면 영역이 이상하다. 일단 Ball 주변을 Ball 중심을 기준으로 다 돌려 버리자.
        //먼저 최대 링크 거리를 계산하고
        float fMaxDist = 0.f;
        float fDist;
        for (auto& linkBallID : ball.m_vecLinkBallID)
        {
            fDist = CPI_Geometry::GetDistance_PointToPoint(fptCurBallCenter.m_x, fptCurBallCenter.m_y,
                m_vecrtBallCircle[linkBallID].m_x, m_vecrtBallCircle[linkBallID].m_y);
            fMaxDist = (float)max(fMaxDist, fDist);
        }
        if (fMaxDist <= 0)
            continue;
        fMaxDist += CAST_FLOAT(ball.m_radiusX_px);
        //돌려 버릴 ROI을 생성
        Ipvm::Rect32s rtBallRotateROI;
        rtBallRotateROI.m_left = (long)(fptCurBallCenter.m_x - fMaxDist + .5f);
        rtBallRotateROI.m_top = (long)(fptCurBallCenter.m_y - fMaxDist + .5f);
        rtBallRotateROI.m_right = (long)(fptCurBallCenter.m_x + fMaxDist + .5f);
        rtBallRotateROI.m_bottom = (long)(fptCurBallCenter.m_y + fMaxDist + .5f);
        //}}

        for (auto& ballData : m_vec2BallCalcData[nBall])
        {
            fLinkAngle = ballData.fBallLinkAngle_Rad;
            rtBlobROI = ballData.rtBallBridgeROI;
            if (rtBlobROI.Width()
                < nValidMinInspAreaWidth) //kircheis_BallBidgeModified//검사영역의 폭이 nValidMinInspAreaWidth(50um) 보다 작으면 문제가 있는 Ball 이니까 Value를 100으로 할당.
            {
                rtObjROI = rtBlobROI;
                rtObjROI.InflateRect(3, 0);
                fWidthRatioMax = 100.f;
                CPI_Geometry::RotateRectToPI_RECT(rtObjROI, fLinkAngle, fptCurBallCenter, piRect);
                rtResultROI = piRect.CreateExternalMinimumRect();

                // ROI 가 너무 작아서 잘 안보여서 좀 키워줬다..
                rtResultROI.InflateRect(nInplateOffset, nInplateOffset);
                rtResultROI &= Ipvm::Rect32s(combineImage);
                continue;
            }

            rtRotateROI = ballData.pirtBallBridgeShowROI.CreateExternalMinimumRect();
            rtRotateROI |= rtBlobROI;
            rtRotateROI.InflateRect(3, 3);

            Ipvm::ImageProcessing::Fill(rtBallRotateROI, 0, rotateImage); //kircheis_BallBidgeModified
            Ipvm::ImageProcessing::Fill(rtBallRotateROI, 0, thresholdImage); //kircheis_BallBidgeModified
            Ipvm::ImageProcessing::RotateLinearInterpolation(calcImage, rtBallRotateROI, fptCurBallCenter,
                -fLinkAngle * ITP_RAD_TO_DEG, Ipvm::Point32r2(0.f, 0.f), rotateImage); //kircheis_BallBidgeModified
            //kirchies_MED3
            CippModules::Binarize(rotateImage, thresholdImage, rtBlobROI, m_VisionPara->m_nThresholdValue, false);

            BYTE bMax;
            Ipvm::Rect32s rtSlit = rtBlobROI;
            long nValidCnt = 0;
            long nLengthCnt = 0;
            for (long nIdx = rtBlobROI.m_left; nIdx < rtBlobROI.m_right; nIdx++)
            {
                nLengthCnt++;
                rtSlit.m_left = nIdx;
                rtSlit.m_right = nIdx + 1;
                Ipvm::ImageProcessing::GetMax(thresholdImage, rtSlit, bMax);
                if (bMax == 255)
                    nValidCnt++;
            }
            if (nValidCnt == 0)
                continue;

            //Blob
            Ipvm::ImageProcessing::Fill(rtBlobROI, 0, labelImage);
            std::vector<Ipvm::BlobInfo> vecObjBlob(nMaxBlobNum);
            pBlob->DoBlob(thresholdImage, rtBlobROI, nMaxBlobNum, labelImage, &vecObjBlob[0], nCurBlobNum);
            if (nCurBlobNum == 0)
                continue;

            //Area가 가장 큰 Blob 확인
            rtSlit = vecObjBlob[0].m_roi;
            nMaxBlobArea = 0;
            for (auto& blobInfo : vecObjBlob)
            {
                rtSlit |= blobInfo.m_roi;
                if (nMaxBlobArea < blobInfo.m_area)
                {
                    nMaxBlobArea = blobInfo.m_area;
                    rtObjROI = blobInfo.m_roi;
                    nBlobMaxWidth = rtObjROI.Width();
                }
            }
            fWidthRatio = (float)nValidCnt / (float)nLengthCnt * 100.f;

            if (fWidthRatioMax < fWidthRatio)
            {
                rtObjROI = rtSlit;
                fWidthRatioMax = fWidthRatio;
                CPI_Geometry::RotateRectToPI_RECT(rtObjROI, fLinkAngle, fptCurBallCenter, piRect);
                rtResultROI = piRect.CreateExternalMinimumRect();

                // ROI 가 너무 작아서 잘 안보여서 좀 키워줬다..
                rtResultROI.InflateRect(nInplateOffset, nInplateOffset);
                rtResultROI &= Ipvm::Rect32s(combineImage);
            }
        }
        //검사 결과 구성
        pResult->SetValue_EachResult_AndTypicalValueBySingleSpec(ball.m_index, ball.m_name, ball.m_groupID, strCompID,
            fWidthRatioMax, *pSpec, (float)ball.m_posX_um, (float)ball.m_posY_um);
        pResult->SetRect(ball.m_index, rtResultROI);
    }
    pResult->UpdateTypicalValue(pSpec);
    pResult->SetTotalResult();

    strCompID.Empty();

    return TRUE; //불량이 발생해도 검사 실패만 아니면 return TRUE을 날리자. 그래야 계속 검사 한다.
}

void VisionInspectionBgaBallBridge::SetDebugInfo(const bool detailSetupMode)
{
    if (detailSetupMode)
    {
        std::vector<Ipvm::LineSeg32r> vecsLine(0);
        std::vector<PI_RECT> vecpirtShowROI(0);
        std::vector<Ipvm::Rect32s> vecpirtRawROI(0);

        for (auto& ppInspData : m_vec2BallCalcData)
        {
            for (auto& pInspData : ppInspData)
            {
                vecsLine.push_back(pInspData.lineBallLink);
                vecpirtShowROI.push_back(pInspData.pirtBallBridgeShowROI);
                vecpirtRawROI.push_back(pInspData.rtBallBridgeROI);
            }
        }
        SetDebugInfoItem(detailSetupMode, _T("Ball Link Map"), vecsLine);
        SetDebugInfoItem(detailSetupMode, _T("Ball Reference Circle"), m_vecrtBallOriginCircle); //kircheis_MED3
        SetDebugInfoItem(detailSetupMode, _T("Ball Ignore Circle"), m_vecrtBallCircle); //kircheis_MED3
        SetDebugInfoItem(detailSetupMode, _T("Ball Bridge Insp ROI"), vecpirtShowROI);
        SetDebugInfoItem(detailSetupMode, _T("Ball Bridge Raw ROI (for developer)"), vecpirtRawROI);
    }
}

BOOL VisionInspectionBgaBallBridge::MakeRefData(BOOL detailSetupMode)
{
    ResetBufferResult();

    if (!GetBallCircle())
        return false;

    CalcBallLink();
    MakeBallBridgeInspROI();
    MakeBallBridgeShowpROI();

    if (detailSetupMode && m_imageMaskInspROI->GetMem())
    {
        long ImageBufferSize = m_imageMaskInspROI->GetSizeX() * m_imageMaskInspROI->GetSizeY();
        long OriginalBufferSize = getImageLotInsp().GetImageSizeX() * getImageLotInsp().GetImageSizeY();

        if (ImageBufferSize != OriginalBufferSize)
            return false;

        Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(*m_imageMaskInspROI), 0, *m_imageMaskInspROI);
        Ipvm::Quadrangle32r qRect;
        for (auto& ppInspData : m_vec2BallCalcData)
        {
            for (auto& pInspData : ppInspData)
            {
                auto& ballBridgeShowROI = pInspData.pirtBallBridgeShowROI;
                qRect = ConversionEx::ToQuadrangle32r(ballBridgeShowROI);
                Ipvm::ImageProcessing::Fill(qRect, 255, *m_imageMaskInspROI);
            }
        }

        Ipvm::EllipseEq32r ballEllipseBuf;
        for (auto& pBallCircle : m_vecrtBallCircle)
        {
            ballEllipseBuf = pBallCircle;
            ballEllipseBuf.m_xradius -= 1.f;
            ballEllipseBuf.m_yradius -= 1.f;
            Ipvm::ImageProcessing::Fill(ballEllipseBuf, 0, *m_imageMaskInspROI);
        }
    }

    return true;
}

void VisionInspectionBgaBallBridge::ResetBufferResult()
{
    m_vecrtBallROI.clear();
    m_vecrtBallCircle.clear();
    m_vecrtBallOriginCircle.clear(); //kircheis_MED3

    long nBallNum = (long)m_vec2BallCalcData.size();
    for (long nBall = 0; nBall < nBallNum; nBall++)
        m_vec2BallCalcData[nBall].clear();
    m_vec2BallCalcData.clear();
}

BOOL VisionInspectionBgaBallBridge::GetBallCircle() //m_vecrtBallCircle에 Ball 좌표 넣기
{
    BOOL bGetROI = false;

    float fBallIgnoreRatio = (float)(m_VisionPara->m_fBallIgnoreRatio * .01);

    long nDataNumCircle(0), nDataNumROI(0);
    void* pDataCircle
        = m_visionUnit.GetVisionDebugInfo(_VISION_INSP_GUID_BGA_BALL_2D, _T("Ball Width Circle"), nDataNumCircle);
    void* pDataROI = m_visionUnit.GetVisionDebugInfo(_VISION_INSP_GUID_BGA_BALL_2D, _T("Ball Width ROI"), nDataNumROI);
    if (pDataCircle != nullptr && nDataNumCircle > 0 && pDataROI != nullptr && nDataNumROI > 0
        && nDataNumCircle == nDataNumROI)
    {
        m_vecrtBallCircle.resize(nDataNumCircle);
        m_vecrtBallOriginCircle.resize(nDataNumCircle); //kircheis_MED3
        m_vecrtBallROI.resize(nDataNumROI);
        Ipvm::EllipseEq32r* pBallEllipse = (Ipvm::EllipseEq32r*)pDataCircle;
        Ipvm::Rect32r* pROI = (Ipvm::Rect32r*)pDataROI;
        for (long j = 0; j < nDataNumCircle; j++)
        {
            m_vecrtBallCircle[j] = m_vecrtBallOriginCircle[j] = pBallEllipse[j]; //kircheis_MED3
            m_vecrtBallCircle[j].m_xradius
                *= fBallIgnoreRatio; //모든 계산에 Ball Ignore Ratio가 적용된 상황에서 계산하니까 미리 적용해버린다.
            m_vecrtBallCircle[j].m_yradius *= fBallIgnoreRatio;
            m_vecrtBallROI[j] = Ipvm::Conversion::ToRect32s(pROI[j]);
        }
        bGetROI = true;
    }
    return bGetROI;
}

void VisionInspectionBgaBallBridge::
    CalcBallLink() //m_vec2BallCalcData Size할당 및 lineBallLink & fBallLinkAngle_Rad 구성
{
    long nBallNum = (long)m_vecrtBallCircle.size();
    long nBallSpecNum = (long)m_packageSpec.m_ballMap->m_balls.size();

    if (nBallNum != nBallSpecNum || nBallNum == 0 || nBallSpecNum == 0)
        return;

    m_vec2BallCalcData.resize(nBallNum);
    Ipvm::LineSeg32r LineTemp;
    long nLinkNum(0), nDestLinkID(0);

    for (long nBall = 0; nBall < nBallNum; nBall++)
    {
        auto& ball = m_packageSpec.m_ballMap->m_balls[nBall];
        LineTemp.m_sx = m_vecrtBallCircle[nBall].m_x;
        LineTemp.m_sy = m_vecrtBallCircle[nBall].m_y;
        nLinkNum = (long)ball.m_vecLinkBallID.size();

        m_vec2BallCalcData[nBall].resize(nLinkNum);

        for (long nLink = 0; nLink < nLinkNum; nLink++)
        {
            nDestLinkID = ball.m_vecLinkBallID[nLink];
            LineTemp.m_ex = m_vecrtBallCircle[nDestLinkID].m_x;
            LineTemp.m_ey = m_vecrtBallCircle[nDestLinkID].m_y;
            m_vec2BallCalcData[nBall][nLink].lineBallLink = LineTemp;
            m_vec2BallCalcData[nBall][nLink].fBallLinkAngle_Rad
                = CPI_Geometry::Get1LineAngleRad(LineTemp.m_sx, LineTemp.m_sy, LineTemp.m_ex, LineTemp.m_ey);
        }
    }
}

void VisionInspectionBgaBallBridge::MakeBallBridgeInspROI() //m_vec2rtBallBridgeROI구성
{
    long nBallNum = (long)m_vec2BallCalcData.size();

    if (nBallNum == 0)
        return;

    float fRoiWidthRatio = (float)(m_VisionPara->m_fROI_WidthRatio * 0.01);
    float fBallIgnoreRatio = (float)(m_VisionPara->m_fBallIgnoreRatio * 0.01);

    long nLinkNum(0), nCurBallWidth(0), nDestBallWidth(0), nROI_Height(0), nDestLinkID(0);
    long nBallPosY(0), nRoiRightOffset(0);
    float fSrcBallR(0.f), fDstBallR(0.f), fDist(0.f);
    Ipvm::Rect32s rtROI;

    for (long nBall = 0; nBall < nBallNum; nBall++)
    {
        auto& ball = m_packageSpec.m_ballMap->m_balls[nBall];
        auto& srcBallCircle = m_vecrtBallCircle[nBall];

        fSrcBallR = srcBallCircle.m_xradius;
        nCurBallWidth = (long)(fSrcBallR + .5f);
        rtROI.m_left = (long)(srcBallCircle.m_x + fSrcBallR + .5f);
        nLinkNum = (long)m_vec2BallCalcData[nBall].size();

        for (long nLink = 0; nLink < nLinkNum; nLink++)
        {
            nDestLinkID = ball.m_vecLinkBallID[nLink];
            auto& dstBallCircle = m_vecrtBallCircle[nDestLinkID];

            nBallPosY = (long)(srcBallCircle.m_y + .5f);
            fDstBallR = dstBallCircle.m_xradius;
            nDestBallWidth = (long)(fDstBallR + .5f);
            nROI_Height = (long)(((float)max(nCurBallWidth, nDestBallWidth) / fBallIgnoreRatio) * fRoiWidthRatio + .5f);
            fDist = CPI_Geometry::GetDistance_PointToPoint(
                srcBallCircle.m_x, srcBallCircle.m_y, dstBallCircle.m_x, dstBallCircle.m_y);

            rtROI.m_top = nBallPosY - nROI_Height;
            rtROI.m_bottom = nBallPosY + nROI_Height;
            nRoiRightOffset = (long)max(0, fDist - fSrcBallR - fDstBallR + .5f);
            rtROI.m_right = rtROI.m_left + nRoiRightOffset;

            m_vec2BallCalcData[nBall][nLink].rtBallBridgeROI = rtROI;
        }
    }
}

void VisionInspectionBgaBallBridge::MakeBallBridgeShowpROI() //m_vec2prtBallBridgeShowROI구성
{
    long nBallNum = (long)m_vec2BallCalcData.size();

    if (nBallNum == 0)
        return;

    Ipvm::Point32r2 fptCurBallCenter;
    PI_RECT piRect;

    for (long nBall = 0; nBall < nBallNum; nBall++)
    {
        auto& srcBallCircle = m_vecrtBallCircle[nBall];
        fptCurBallCenter.Set(srcBallCircle.m_x, srcBallCircle.m_y);

        for (auto& realROI : m_vec2BallCalcData[nBall])
        {
            CPI_Geometry::RotateRectToPI_RECT(
                realROI.rtBallBridgeROI, realROI.fBallLinkAngle_Rad, fptCurBallCenter, piRect);
            realROI.pirtBallBridgeShowROI = piRect;
        }
    }
}

void VisionInspectionBgaBallBridge::MakeBallBridgeMask(
    const BOOL detailSetupMode) //kircheis_MED3 //Surface용 Mask를 만든다.
{
    long nBallNum = (long)m_vec2BallCalcData.size();
    if (nBallNum == 0)
        return;

    Ipvm::Image8u imageMask;
    if (!getReusableMemory().GetInspByteImage(imageMask))
        return;
    Ipvm::ImageProcessing::Fill((Ipvm::Rect32s)imageMask, 0, imageMask);

    Ipvm::Image8u imageInspMask;
    if (!getReusableMemory().GetInspByteImage(imageInspMask))
        return;
    Ipvm::ImageProcessing::Fill((Ipvm::Rect32s)imageInspMask, 0, imageInspMask);

    Ipvm::Quadrangle32r qrtBallBridgeHor; //개별 영역을 각도 무시하고 수평으로 만드는 버퍼
    Ipvm::Quadrangle32r qrtBallBridge; //개별 영역을 회전시켜 벡터에 넣기 위한 버퍼
    std::vector<Ipvm::Quadrangle32r> vecqrtBallBridge(0); //모든 영역을 저장할 버퍼

    PI_RECT prtBallBridgeDebug; //Debug Info용 버퍼
    std::vector<PI_RECT> vecprtBallBridgeDebug(0); //Debug Info용 버퍼

    //링크 라인과 오리지널 Ball로 사다리꼴 박스(수평)를 만들고 회전 후 버퍼에 채우기까지 한 루프에서 처리 한다.
    long nLinkNum(0), nDestLinkID(0);
    float fDist(0.f), fRotateAngle(0.f);
    Ipvm::Point32r2 fptCurBallCenter;

    for (long nBall = 0; nBall < nBallNum; nBall++)
    {
        auto& ball = m_packageSpec.m_ballMap->m_balls[nBall];
        auto& srcBallCircle = m_vecrtBallOriginCircle[nBall];
        fptCurBallCenter.Set(srcBallCircle.m_x, srcBallCircle.m_y);

        //먼저 수평 사다리꼴부터 만드는데 좌측 Point를 만든다.
        qrtBallBridgeHor.m_ltX = qrtBallBridgeHor.m_lbX = srcBallCircle.m_x;
        qrtBallBridgeHor.m_ltY = srcBallCircle.m_y - srcBallCircle.m_yradius;
        qrtBallBridgeHor.m_lbY = srcBallCircle.m_y + srcBallCircle.m_yradius;

        nLinkNum = (long)m_vec2BallCalcData[nBall].size();

        for (long nLink = 0; nLink < nLinkNum; nLink++)
        {
            nDestLinkID = ball.m_vecLinkBallID[nLink];
            fRotateAngle = m_vec2BallCalcData[nBall][nLink].fBallLinkAngle_Rad;
            auto& dstBallCircle = m_vecrtBallOriginCircle[nDestLinkID];

            //각도의 문제가 있어 먼저 Ball간 거리를 구한다
            fDist = CPI_Geometry::GetDistance_PointToPoint(
                srcBallCircle.m_x, srcBallCircle.m_y, dstBallCircle.m_x, dstBallCircle.m_y);

            //구한 거리로 수평 사다리꼴의 우측 Point를 만든다.
            qrtBallBridgeHor.m_rtX = qrtBallBridgeHor.m_rbX = (srcBallCircle.m_x + fDist);
            qrtBallBridgeHor.m_rtY = srcBallCircle.m_y - dstBallCircle.m_yradius;
            qrtBallBridgeHor.m_rbY = srcBallCircle.m_y + dstBallCircle.m_yradius;

            //수평 Quad Rect를 실제 Ball 위치에 맞게 회전
            CPI_Geometry::RotateQuadRect(qrtBallBridgeHor, fRotateAngle, fptCurBallCenter, qrtBallBridge);
            vecqrtBallBridge.push_back(qrtBallBridge);

            //회전된 영역을 255로 채우자.
            Ipvm::ImageProcessing::Fill(qrtBallBridge, 255, imageMask);

            if (detailSetupMode)
            {
                prtBallBridgeDebug.Set(CAST_LONG(qrtBallBridge.m_ltX + .5f), CAST_LONG(qrtBallBridge.m_ltY + .5f),
                    CAST_LONG(qrtBallBridge.m_rtX + .5f), CAST_LONG(qrtBallBridge.m_rtY + .5f),
                    CAST_LONG(qrtBallBridge.m_lbX + .5f), CAST_LONG(qrtBallBridge.m_lbY + .5f),
                    CAST_LONG(qrtBallBridge.m_rbX + .5f), CAST_LONG(qrtBallBridge.m_rbY + .5f));
                vecprtBallBridgeDebug.push_back(prtBallBridgeDebug);
            }
        }
    }
    //vecpirtShowROI
    Ipvm::Quadrangle32r qrtBallBridgeInspROI;
    for (auto& ppInspData : m_vec2BallCalcData)
    {
        for (auto& pInspData : ppInspData)
        {
            qrtBallBridgeInspROI = ConversionEx::ToQuadrangle32r(pInspData.pirtBallBridgeShowROI);
            Ipvm::ImageProcessing::Fill(qrtBallBridgeInspROI, 255, imageInspMask);
        }
    }

    //Dialate이 적용된 Ball 영역을 지운다
    nBallNum = (long)m_vecrtBallCircle.size();
    if (nBallNum > 0)
    {
        for (auto& ballCircle : m_vecrtBallCircle)
        {
            Ipvm::ImageProcessing::Fill(ballCircle, 0, imageMask);
            Ipvm::ImageProcessing::Fill(ballCircle, 0, imageInspMask);
        }
    }

    //Surface Mask 등록

    auto* alignMask = getReusableMemory().AddSurfaceLayerMaskClass(_T("Ball Bridge Insp Mask"));
    if (alignMask)
    {
        alignMask->Reset();
        alignMask->Add(imageMask);
    }
    auto* alignMaskInspROI = getReusableMemory().AddSurfaceLayerMaskClass(_T("Ball Bridge Insp Mask [Rect]"));
    if (alignMaskInspROI)
    {
        alignMaskInspROI->Reset();
        alignMaskInspROI->Add(imageInspMask);
    }
    if (detailSetupMode)
    {
        SetDebugInfoItem(detailSetupMode, _T("Ball Bridge Insp Mask [Rect]"), imageInspMask, true);
        SetDebugInfoItem(detailSetupMode, _T("Ball Bridge Insp Mask"), imageMask, true);
        SetDebugInfoItem(detailSetupMode, _T("Ball Bridge Insp Mask ROI"), vecprtBallBridgeDebug, true);
    }
}
