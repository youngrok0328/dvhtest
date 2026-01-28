//CPP_0_________________________________ Precompiled header
#include "stdAfx.h"

//CPP_1_________________________________ Main header
#include "VisionInspectionBgaBottom2D.h"

//CPP_2_________________________________ This project's headers
#include "DlgVisionInspectionBgaBottom2D.h"
#include "Result.h"
#include "VisionInspectionBgaBottom2DPara.h"

//CPP_3_________________________________ Other projects' headers
#include "../../AlgorithmModules/dPI_Blob/dPI_BlobLib.h"
#include "../../AlgorithmModules/dPI_EdgeDetect/dPI_EdgeDetectLib.h"
#include "../../AlgorithmModules/dPI_Geometry/dPI_GeometryLib.h"
#include "../../DefineModules/dA_Base/VisionScale.h"
#include "../../ImageCombineModules/dPI_ippModules/ippModules.h"
#include "../../InformationModule/dPI_DataBase/PackageSpec.h"
#include "../../MemoryModules/VisionReusableMemory/SurfaceLayerMask.h" //kircheis_MED3
#include "../../MemoryModules/VisionReusableMemory/SurfaceLayerRoi.h"
#include "../../MemoryModules/VisionReusableMemory/VisionReusableMemory.h"
#include "../../SharedCommunicationModules/VisionHostCommon/VisionHostBaseDef.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionAlignResult.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionImageLotInsp.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Algorithm/DataFitting.h>
#include <Ipvm/Algorithm/Geometry.h>
#include <Ipvm/Algorithm/ImageProcessing.h>
#include <Ipvm/Algorithm/TinyMatrix.h>
#include <Ipvm/Base/CircleEq32r.h>
#include <Ipvm/Base/Conversion.h>
#include <Ipvm/Base/EllipseEq32r.h>
#include <Ipvm/Base/Image32s.h> //kircheis_LKF
#include <Ipvm/Base/LineEq32r.h>
#include <Ipvm/Base/LineSeg32r.h>
#include <Ipvm/Base/TimeCheck.h>

//CPP_5_________________________________ Standard library headers
#include <algorithm>
#include <math.h>

//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif
//CPP_7_________________________________ Implementation body
//
BOOL VisionInspectionBgaBottom2D::OnInspection()
{
    m_pVisionInspDlg->OnBnClickedButtonInspect();
    return TRUE;
}

// 20140702 BGA_Inspection_BlobInfo : 여기서 사용하는 Blobinfo는 모두 삭제해버린다. 실제로 사용할 필요가 없다.
BOOL VisionInspectionBgaBottom2D::DoInspection(const bool detailSetupMode, const enSideVisionModule i_ModuleStatus)
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

    // ksy deeplearning
    m_bUse2ndInspection = m_pVisionPara->m_use2ndInspection;
    m_str2ndInspCode = m_pVisionPara->m_str2ndInspCode;

    Ipvm::TimeCheck time;

    ResetResult();

    // 패키지 스펙 변경으로 인해 볼 종류가 달라질 수 있으므로 갯수를 맞춰준다.
    m_pVisionPara->m_parameters.resize(m_packageSpec.m_originalballMap->m_ballTypes.size());

    BOOL bInsp = DoInsp(detailSetupMode);

    // Debug Info...
    SetDebugInfo(detailSetupMode);

    m_fCalcTime = CAST_FLOAT(time.Elapsed_ms());

    return bInsp;
}

BOOL VisionInspectionBgaBottom2D::DoInsp(const bool detailSetupMode)
{
    Ipvm::Image8u combineImage_normal;
    Ipvm::Image8u combineImage_quality;
    Ipvm::Image8u thresholdImage;
    Ipvm::Image8u bufferImage;

    if (!getReusableMemory().GetInspByteImage(combineImage_normal))
        return FALSE;
    if (!getReusableMemory().GetInspByteImage(combineImage_quality))
        return FALSE;
    if (!getReusableMemory().GetInspByteImage(thresholdImage))
        return FALSE;
    if (!getReusableMemory().GetInspByteImage(bufferImage))
        return FALSE;

    BOOL bInspResult = TRUE;

    // 영훈 20150107_Ball_BodyAlignMode : Ball 정보가 없는 경우에도 Body Size 검사항목을 사용할 수 있도록 수정
    PrepareInspection();

    if (m_packageSpec.m_ballMap->m_balls.size() <= 0)
    {
        return bInspResult;
    }

    // Ball 의 위치를 Body Center를 기준으로 계산한다.
    if (!GetBallCenterByBody())
        return FALSE;

    Ipvm::Rect32s rtInspectionROI(0, 0, 0, 0);
    rtInspectionROI.m_left = (long)(min(m_sEdgeAlignResult->fptLT.m_x, m_sEdgeAlignResult->fptLB.m_x) + 0.5f);
    rtInspectionROI.m_right = (long)(max(m_sEdgeAlignResult->fptRT.m_x, m_sEdgeAlignResult->fptRB.m_x) + 0.5f);
    rtInspectionROI.m_top = (long)(min(m_sEdgeAlignResult->fptLT.m_y, m_sEdgeAlignResult->fptRT.m_y) + 0.5f);
    rtInspectionROI.m_bottom = (long)(max(m_sEdgeAlignResult->fptLB.m_y, m_sEdgeAlignResult->fptRB.m_y) + 0.5f);
    rtInspectionROI.InflateRect(10, 10, 10, 10);

    // 불량 Ball에 정상 ROI를 추가하고 Surface Mask를 생성하기 위해 Stitch 관련 검사가 끝난 후 Sititch Image로 다시 Ball Align 해준다.
    if (!GetImageProcessing_Normal(combineImage_normal, bufferImage, rtInspectionROI))
    {
        return FALSE;
    }

    if (!GetImageProcessing_Qaulity(combineImage_quality, rtInspectionROI))
    {
        return FALSE;
    }

    if (!GetBallCenterAndWidth(detailSetupMode, combineImage_normal, bufferImage))
        return FALSE;

    if (!AlignBalls())
        return FALSE;

    if (!GetBallContrast(combineImage_normal))
    {
        return FALSE;
    }
    bInspResult &= DoBallContrast();

    Ipvm::Image8u auxFrame = bufferImage;

    if (m_pVisionPara->m_nImageCombineMode
        == VisionInspectionBgaBottom2DPara::enumCombineMode_InvertCoaxial) //kircheis_Shiny
    {
        Ipvm::ImageProcessing::Subtract(m_obliqueImage, m_coaxialImage, rtInspectionROI, 0, combineImage_normal);
    }
    else if (m_pVisionPara->m_nImageCombineMode
        == VisionInspectionBgaBottom2DPara::enumCombineMode_ReformedInvertCoaxial) //kircheis_InvertCoax
    {
        Ipvm::ImageProcessing::Copy(auxFrame, Ipvm::Rect32s(auxFrame), combineImage_normal);
    }

    long BallTypeCount = (long)m_packageSpec.m_originalballMap->m_ballTypes.size();

    m_vec2BallData.clear();
    m_vec2BallData.resize(BallTypeCount);

    for (auto& ball : m_packageSpec.m_ballMap->m_balls)
    {
        m_vec2BallData[ball.m_typeIndex].push_back(ball);
    }

    for (int i = 0; i < BallTypeCount; i++)
    {
        CString strInspName;

        if (BallTypeCount > 1)
            strInspName = m_pVisionPara->m_vecstrGroupInspName[i].BallQualityName;

        switch (m_pVisionPara->m_parameters[i].m_qualityCheckMethod) //이걸어떡하지...?
        {
            case VisionInspectionBgaBottom2DPara::enumNormalMethod:
                GetQualityInfo(strInspName, m_vec2BallData[i], combineImage_quality,
                    m_pVisionPara->m_parameters[i].m_qualityThresholdValue);
                break;
            case VisionInspectionBgaBottom2DPara::enumRingThickness:
                GetQualityInfoByRingThickness(strInspName, m_vec2BallData[i], combineImage_quality,
                    m_pVisionPara->m_parameters[i].m_qualityThresholdValue);
                break;
            default:
                return FALSE;
        }

        strInspName.Empty();
    }

    // Ball 관련 검사 수행
    bInspResult &= DoBallMissing();
    bInspResult &= DoBallOffsetX();
    bInspResult &= DoBallOffsetY();
    bInspResult &= DoBallOffsetR();
    bInspResult &= DoBallWidth();
    bInspResult &= DoBallGridOffsetXByKircheis();
    bInspResult &= DoBallGridOffsetYByKircheis();
    bInspResult &= DoBallQuality();
    bInspResult &= DoBallEllipticity(detailSetupMode); //kircheis_TMI

    return TRUE; //불량이 발생해도 검사 실패만 아니면 return TRUE을 날리자. 그래야 계속 검사 한다.
}

BOOL VisionInspectionBgaBottom2D::DoTeach(const bool detailSetupMode, long i_nBallGroupID)
{
    ResetResult();

    Ipvm::Image8u combineImage_noraml;
    Ipvm::Image8u combineImage_quality;
    Ipvm::Image8u thresholdImage;
    Ipvm::Image8u bufferImage;

    if (!getReusableMemory().GetInspByteImage(combineImage_noraml))
        return FALSE;
    if (!getReusableMemory().GetInspByteImage(combineImage_quality))
        return FALSE;
    if (!getReusableMemory().GetInspByteImage(thresholdImage))
        return FALSE;
    if (!getReusableMemory().GetInspByteImage(bufferImage))
        return FALSE;

    PrepareInspection();

    if (m_packageSpec.m_ballMap->m_balls.size() <= 0)
    {
        return FALSE;
    }

    // Ball 의 위치를 Body Center를 기준으로 계산한다.
    if (!GetBallCenterByBody())
        return FALSE;

    Ipvm::Rect32s rtInspectionROI(0, 0, 0, 0);
    rtInspectionROI.m_left = (long)(min(m_sEdgeAlignResult->fptLT.m_x, m_sEdgeAlignResult->fptLB.m_x) + 0.5f);
    rtInspectionROI.m_right = (long)(max(m_sEdgeAlignResult->fptRT.m_x, m_sEdgeAlignResult->fptRB.m_x) + 0.5f);
    rtInspectionROI.m_top = (long)(min(m_sEdgeAlignResult->fptLT.m_y, m_sEdgeAlignResult->fptRT.m_y) + 0.5f);
    rtInspectionROI.m_bottom = (long)(max(m_sEdgeAlignResult->fptLB.m_y, m_sEdgeAlignResult->fptRB.m_y) + 0.5f);
    rtInspectionROI.InflateRect(10, 10, 10, 10);

    if (!GetImageProcessing_Normal(combineImage_noraml, bufferImage, rtInspectionROI))
    {
        return FALSE;
    }

    if (!GetImageProcessing_Qaulity(combineImage_quality, rtInspectionROI))
    {
        return FALSE;
    }

    if (!GetBallCenterAndWidth(detailSetupMode, combineImage_noraml, bufferImage))
        return FALSE;

    if (m_pVisionPara->m_nImageCombineMode
        == VisionInspectionBgaBottom2DPara::enumCombineMode_InvertCoaxial) //kircheis_Shiny
    {
        Ipvm::ImageProcessing::Subtract(m_obliqueImage, m_coaxialImage, rtInspectionROI, 0, combineImage_noraml);
    }
    else if (m_pVisionPara->m_nImageCombineMode
        == VisionInspectionBgaBottom2DPara::enumCombineMode_ReformedInvertCoaxial) //kircheis_InvertCoax
    {
        Ipvm::ImageProcessing::Subtract(m_obliqueImage, m_coaxialImage, rtInspectionROI, 0, combineImage_noraml);
    }

    auto& GroupPara = m_pVisionPara->m_parameters[i_nBallGroupID];

    switch (GroupPara.m_qualityCheckMethod)
    {
        case VisionInspectionBgaBottom2DPara::enumNormalMethod:
            break;
        case VisionInspectionBgaBottom2DPara::enumRingThickness:
            if (!DoQualityTeach(combineImage_quality, GroupPara.m_qualityThresholdValue, i_nBallGroupID))
            {
                return FALSE;
            }
            break;
        default:
            return FALSE;
    }

    return TRUE;
}

BOOL VisionInspectionBgaBottom2D::GetImageProcessing_Normal(
    Ipvm::Image8u& combineImage, Ipvm::Image8u& auxFrame, Ipvm::Rect32s rtInspectionROI)
{
    m_obliqueImage = m_pVisionPara->m_obliqueImageIndex.getImage(false);
    if (m_obliqueImage.GetMem() == nullptr)
        return FALSE;

    m_coaxialImage = m_pVisionPara->m_coaxialImageIndex.getImage(false);
    if (m_coaxialImage.GetMem() == nullptr)
        return FALSE;

    Ipvm::Rect32s rtROI = Ipvm::Rect32s(rtInspectionROI.m_left + 2, rtInspectionROI.m_top + 2,
        rtInspectionROI.m_right - 2, rtInspectionROI.m_bottom - 2);

    switch (m_pVisionPara->m_nImageCombineMode)
    {
        case VisionInspectionBgaBottom2DPara::enumCombineMode_Default:
        case VisionInspectionBgaBottom2DPara::enumCombineMode_ImprovedDefault: //kircheis_ImproveBall2D
            Ipvm::ImageProcessing::Subtract(m_obliqueImage, m_coaxialImage, rtROI, 0, combineImage);
            break;
        case VisionInspectionBgaBottom2DPara::enumCombineMode_Oblique:
            Ipvm::ImageProcessing::Copy(m_obliqueImage, rtROI, combineImage);
            break;
        case VisionInspectionBgaBottom2DPara::enumCombineMode_Self:
            CippModules::EdgeDetect_Magnitude_Sobel(m_obliqueImage, rtROI, combineImage);
            break;
        case VisionInspectionBgaBottom2DPara::enumCombineMode_InvertCoaxial: //kircheis_Shiny
            Ipvm::ImageProcessing::BitwiseNot(m_coaxialImage, rtROI, combineImage);
            break;
        case VisionInspectionBgaBottom2DPara::enumCombineMode_ReformedInvertCoaxial: //kircheis_InvertCoax
        case VisionInspectionBgaBottom2DPara::enumCombineMode_ReformedCoaxial: //kircheis_BKShiny
            Ipvm::ImageProcessing::BitwiseNot(m_coaxialImage, rtROI, combineImage);
            Ipvm::ImageProcessing::Subtract(m_obliqueImage, m_coaxialImage, rtROI, 0, auxFrame);
            break;
    }

    return TRUE;
}

BOOL VisionInspectionBgaBottom2D::GetImageProcessing_Qaulity(Ipvm::Image8u& combineImage, Ipvm::Rect32s rtInspectionROI)
{
    m_obliqueImage = m_pVisionPara->m_qualityObliqueImageIndex.getImage(false);
    if (m_obliqueImage.GetMem() == nullptr)
        return FALSE;

    m_coaxialImage = m_pVisionPara->m_qualityCoaxialImageIndex.getImage(false);
    if (m_coaxialImage.GetMem() == nullptr)
        return FALSE;

    Ipvm::Rect32s rtROI = Ipvm::Rect32s(rtInspectionROI.m_left + 2, rtInspectionROI.m_top + 2,
        rtInspectionROI.m_right - 2, rtInspectionROI.m_bottom - 2);

    switch (m_pVisionPara->m_qualityCombineMode)
    {
        case VisionInspectionBgaBottom2DPara::enumQualityCombineMode_Default:
            Ipvm::ImageProcessing::Subtract(m_obliqueImage, m_coaxialImage, rtROI, 0, combineImage);
            break;
        case VisionInspectionBgaBottom2DPara::enumQualityCombineMode_Oblique:
            Ipvm::ImageProcessing::Copy(m_obliqueImage, rtROI, combineImage);
            break;
        case VisionInspectionBgaBottom2DPara::enumCombineMode_Self:
            CippModules::EdgeDetect_Magnitude_Sobel(m_obliqueImage, rtROI, combineImage);
            break;
    }

    return TRUE;
}

BOOL VisionInspectionBgaBottom2D::GetBallCenterByBody()
{
    m_vecBallSpec_Body.clear();
    m_vecBallSpec_Body.resize(m_packageSpec.m_ballMap->m_balls.size());

    Ipvm::Point32r2 center;

    if (m_pfptGetCenterPosbyPadAlign != NULL)
    {
        center.m_x = m_pfptGetCenterPosbyPadAlign->m_x;
        center.m_y = m_pfptGetCenterPosbyPadAlign->m_y;
    }
    else if (m_sEdgeAlignResult != NULL)
    {
        center = m_sEdgeAlignResult->m_center;
    }
    else
    {
        return FALSE;
    }

    for (const auto& ball : m_packageSpec.m_ballMap->m_balls)
    {
        Ipvm::Point32r2 pos(float(ball.m_posX_px + center.m_x), float(ball.m_posY_px + center.m_y));

        CPI_Geometry::RotatePoint(pos, m_sEdgeAlignResult->m_angle_rad, center);

        m_vecBallSpec_Body[ball.m_index].m_x = pos.m_x;
        m_vecBallSpec_Body[ball.m_index].m_y = pos.m_y;
        m_vecBallSpec_Body[ball.m_index].m_xradius = CAST_FLOAT(ball.m_radiusX_px);
        m_vecBallSpec_Body[ball.m_index].m_yradius = CAST_FLOAT(ball.m_radiusY_px);
    }

    return TRUE;
}

BOOL VisionInspectionBgaBottom2D::AlignBalls()
{
    std::vector<Ipvm::Point32r2> specBallPos_um;
    std::vector<Ipvm::Point32r2> realBallPos_um;

    specBallPos_um.reserve(m_packageSpec.m_ballMap->m_balls.size());
    realBallPos_um.reserve(m_packageSpec.m_ballMap->m_balls.size());

    for (const auto& ball : m_packageSpec.m_ballMap->m_balls)
    {
        if (ball.m_ignored)
        {
            continue;
        }

        specBallPos_um.emplace_back(ball.m_posX_um, ball.m_posY_um);

        // 실좌표계는 오른손 좌표계이므로 Y 좌표에 -1 을 곱해줌
        realBallPos_um.emplace_back(
            getScale().convert_pixelToUmX(m_sBallAlignResult->m_ballWidthCenterPos_px[ball.m_index].m_x),
            -getScale().convert_pixelToUmY(m_sBallAlignResult->m_ballWidthCenterPos_px[ball.m_index].m_y));
    }

    if (specBallPos_um.size() == 0)
    {
        return TRUE;
    }

    if (Ipvm::Status::e_ok
        != Ipvm::Geometry::GetAffineTransform(
            &specBallPos_um[0], &realBallPos_um[0], (long)specBallPos_um.size(), m_affineTransformSpecToReal))
    {
        return TRUE;
    }

    std::vector<float> errorScores(specBallPos_um.size());

    for (long idx = (long)specBallPos_um.size() - 1; idx >= 0; idx--)
    {
        const float x = (float)(specBallPos_um[idx].m_x * m_affineTransformSpecToReal[0][0]
            + specBallPos_um[idx].m_y * m_affineTransformSpecToReal[0][1] + m_affineTransformSpecToReal[0][2]);
        const float y = (float)(specBallPos_um[idx].m_x * m_affineTransformSpecToReal[1][0]
            + specBallPos_um[idx].m_y * m_affineTransformSpecToReal[1][1] + m_affineTransformSpecToReal[1][2]);

        const float dx = realBallPos_um[idx].m_x - x;
        const float dy = realBallPos_um[idx].m_y - y;

        errorScores[idx] = dx * dx + dy * dy;
    }

    Ipvm::Geometry::GetOptimalAffineTransform(&specBallPos_um[0], &realBallPos_um[0], &errorScores[0], 0.5f,
        (long)specBallPos_um.size(), m_affineTransformSpecToReal);

#ifdef _DEBUG
    for (long idx = (long)specBallPos_um.size() - 1; idx >= 0; idx--)
    {
        const float x = (float)(specBallPos_um[idx].m_x * m_affineTransformSpecToReal[0][0]
            + specBallPos_um[idx].m_y * m_affineTransformSpecToReal[0][1] + m_affineTransformSpecToReal[0][2]);
        const float y = (float)(specBallPos_um[idx].m_x * m_affineTransformSpecToReal[1][0]
            + specBallPos_um[idx].m_y * m_affineTransformSpecToReal[1][1] + m_affineTransformSpecToReal[1][2]);

        const float dx = realBallPos_um[idx].m_x - x;
        const float dy = realBallPos_um[idx].m_y - y;

        errorScores[idx] = dx * dx + dy * dy;
    }
#endif

    Ipvm::TinyMatrix<3, 3> matSpecToReal;

    matSpecToReal[0][0] = m_affineTransformSpecToReal[0][0];
    matSpecToReal[0][1] = m_affineTransformSpecToReal[0][1];
    matSpecToReal[0][2] = m_affineTransformSpecToReal[0][2];
    matSpecToReal[1][0] = m_affineTransformSpecToReal[1][0];
    matSpecToReal[1][1] = m_affineTransformSpecToReal[1][1];
    matSpecToReal[1][2] = m_affineTransformSpecToReal[1][2];
    matSpecToReal[2][0] = 0.;
    matSpecToReal[2][1] = 0.;
    matSpecToReal[2][2] = 1.;

    Ipvm::TinyMatrix<3, 3> matRealToSpec = !matSpecToReal;

    m_affineTransformRealToSpec[0][0] = matRealToSpec[0][0];
    m_affineTransformRealToSpec[0][1] = matRealToSpec[0][1];
    m_affineTransformRealToSpec[0][2] = matRealToSpec[0][2];
    m_affineTransformRealToSpec[1][0] = matRealToSpec[1][0];
    m_affineTransformRealToSpec[1][1] = matRealToSpec[1][1];
    m_affineTransformRealToSpec[1][2] = matRealToSpec[1][2];

#ifdef _DEBUG
    for (long idx = (long)specBallPos_um.size() - 1; idx >= 0; idx--)
    {
        const float x = (float)(realBallPos_um[idx].m_x * m_affineTransformRealToSpec[0][0]
            + realBallPos_um[idx].m_y * m_affineTransformRealToSpec[0][1] + m_affineTransformRealToSpec[0][2]);
        const float y = (float)(realBallPos_um[idx].m_x * m_affineTransformRealToSpec[1][0]
            + realBallPos_um[idx].m_y * m_affineTransformRealToSpec[1][1] + m_affineTransformRealToSpec[1][2]);

        const float dx = specBallPos_um[idx].m_x - x;
        const float dy = specBallPos_um[idx].m_y - y;

        errorScores[idx] = dx * dx + dy * dy;
    }
#endif

    return TRUE;
}

BOOL VisionInspectionBgaBottom2D::GetBallCenterAndWidth(
    const bool detailSetupMode, const Ipvm::Image8u& combineImage, const Ipvm::Image8u& auxFrame)
{
    // Ball Align 부분
    float fEdgeThesh
        = (m_pVisionPara->m_nImageCombineMode != VisionInspectionBgaBottom2DPara::enumCombineMode_InvertCoaxial
                ? 10.f
                : EDGE_THRESH_INVERT); //kircheis_ShinyTest
    float fDefaultEdgeTreshValue = m_pEdgeDetect->SetMininumThreshold(fEdgeThesh);

    if (m_pVisionPara->m_nImageCombineMode < VisionInspectionBgaBottom2DPara::enumCombineMode_ReformedInvertCoaxial
        || m_pVisionPara->m_nImageCombineMode
            == VisionInspectionBgaBottom2DPara::enumCombineMode_ImprovedDefault) //kircheis_InvertCoax
    {
        if (!GetBallWidthByEdgeDetect(detailSetupMode, combineImage))
        {
            m_pEdgeDetect->SetMininumThreshold(fDefaultEdgeTreshValue);
            return FALSE;
        }
    }
    else if (m_pVisionPara->m_nImageCombineMode
        == VisionInspectionBgaBottom2DPara::enumCombineMode_ReformedInvertCoaxial) //kircheis_InvertCoax
    {
        if (!GetBallWidthByEdgeDetect(detailSetupMode, combineImage, auxFrame)) //kircheis_InvertCoax
        {
            m_pEdgeDetect->SetMininumThreshold(fDefaultEdgeTreshValue);
            return FALSE;
        }
    }
    else //kircheis_BKShiny
    {
        if (!GetFindBallEdge(combineImage, m_coaxialImage)) // 도넛 영상으로 먼저 탐색.
        {
            m_pEdgeDetect->SetMininumThreshold(fDefaultEdgeTreshValue);
            return FALSE;
        }
    }
    m_pEdgeDetect->SetMininumThreshold(fDefaultEdgeTreshValue);

    return TRUE;
}

bool VisionInspectionBgaBottom2D::SkipMissing(const Package::Ball& ball, VisionInspectionSpec* spec,
    VisionInspectionResult* result, VisionInspectionSpec* missingSpec, VisionInspectionResult* missingResult,
    CString strCompID, const long nindex, const float nominalValue)
{
    if (spec == nullptr || result == nullptr || missingSpec == nullptr || missingResult == nullptr)
        return false;

    if (ball.m_ignored || (missingSpec->m_use && missingResult->m_objectErrorValues[ball.m_index] > 0))
    {
        result->SetValue_EachResult_AndTypicalValueBySingleSpec(nindex, ball.m_name, ball.m_groupID, strCompID, 0.f,
            *spec, (float)ball.m_posX_um, (float)ball.m_posY_um, nominalValue); //kircheis_MED2.5
        result->SetRect(nindex, missingResult->m_objectRects[ball.m_index]);
        return true;
    }
    else if (missingResult->m_objectErrorValues[ball.m_index] > 0)
    {
        if (missingSpec->m_use)
        {
            result->SetValue_EachResult_AndTypicalValueBySingleSpec(nindex, ball.m_name, ball.m_groupID, strCompID, 0.f,
                *spec, (float)ball.m_posX_um, (float)ball.m_posY_um, nominalValue); //kircheis_MED2.5
            result->SetRect(nindex, missingResult->m_objectRects[ball.m_index]);
            return true;
        }
        else
        {
            result->SetValue_EachResult_AndTypicalValueBySingleSpec(nindex, ball.m_name, ball.m_groupID, strCompID, 0.f,
                *spec, (float)ball.m_posX_um, (float)ball.m_posY_um, nominalValue, INVALID); //kircheis_MED2.5
            //result->SetInvalid_EachResult(nindex, ball.m_name, ball.m_groupID, (float)ball.m_posX_um, (float)ball.m_posY_um, nominalValue);//kircheis_MED2.5
            result->SetRect(nindex, missingResult->m_objectRects[ball.m_index]);
            return true;
        }
    }

    return false;
}

BOOL VisionInspectionBgaBottom2D::DoBallContrast(
    const CString strBallContrastName, const std::vector<Package::Ball> BallData, BOOL bEachGroupInsp) //kircheis_MED2.5
{
    auto* result = m_resultGroup.GetResultByName(strBallContrastName);
    if (result == nullptr)
    {
        return FALSE;
    }
    auto* spec = GetSpecByName(result->m_resultName);
    if (spec == nullptr)
    {
        return FALSE;
    }

    if (spec->m_use == FALSE)
        return TRUE;

    const float fBallContrastAVGValue = GetBallContrastCompensation();

    auto* missingSpec = GetSpecByGuid(_T("{B0698F99-5390-40FB-AF77-1079512FA3BA}"));
    auto* missingResult = m_resultGroup.GetResultByGuid(_T("{B0698F99-5390-40FB-AF77-1079512FA3BA}"));
    if (missingSpec == nullptr || missingResult == nullptr)
    {
        return FALSE;
    }

    long idx = -1;
    BOOL CheckAllData = BallData.size() == m_packageSpec.m_ballMap->m_balls.size();

    if (!CheckAllData)
        result->Resize(CAST_LONG(BallData.size()));

    CString strCompID = m_packageSpec.m_originalballMap->m_strAllGroupName; //kircheis_MED2.5

    for (const auto& ball : BallData)
    {
        if (bEachGroupInsp) //kircheis_MED2.5
            strCompID = ball.m_groupID;

        if (CheckAllData)
            idx = ball.m_index;
        else
            idx++;

        if (SkipMissing(ball, spec, result, missingSpec, missingResult, strCompID,
                CAST_LONG(fBallContrastAVGValue + .5f))) //kircheis_MED2.5
        {
            continue;
        }

        const float fContrastValue = m_sBallAlignResult->m_ballConstrasts[ball.m_index];

        const float fError = fContrastValue - fBallContrastAVGValue;

        result->SetValue_EachResult_AndTypicalValueBySingleSpec(idx, ball.m_name, ball.m_groupID, strCompID, fError,
            *spec, (float)ball.m_posX_um, (float)ball.m_posY_um, fBallContrastAVGValue); //kircheis_MED2.5
        result->SetRect(idx, m_vecrtBallPosition_pixel[ball.m_index]);
    }

    result->UpdateTypicalValue(spec);
    result->SetTotalResult();

    strCompID.Empty();

    return TRUE; //불량이 발생해도 검사 실패만 아니면 return TRUE을 날리자. 그래야 계속 검사 한다.
}

BOOL VisionInspectionBgaBottom2D::DoBallContrast()
{
    DoBallContrast(g_szBallInspectionName[BALL_INSPECTION_BALL_CONTRAST], m_packageSpec.m_ballMap->m_balls);

    std::vector<std::vector<Package::Ball>> vec2BallData;
    long BallTypeCount = CAST_LONG(m_packageSpec.m_originalballMap->m_ballTypes.size());

    vec2BallData.resize(BallTypeCount);

    for (auto& ball : m_packageSpec.m_ballMap->m_balls)
    {
        vec2BallData[ball.m_typeIndex].push_back(ball);
    }

    //Group Object 구간
    long GroupSpecCount = CAST_LONG(m_pVisionPara->m_vecstrGroupInspName.size());

    if (GroupSpecCount != 0 && GroupSpecCount != m_packageSpec.m_originalballMap->m_ballTypes.size())
        return TRUE;

    for (int idx = 0; idx < GroupSpecCount; idx++)
    {
        if (!DoBallContrast(
                m_pVisionPara->m_vecstrGroupInspName[idx].BallContrastName, vec2BallData[idx], TRUE)) //kircheis_MED2.5
        {
            return TRUE;
        }
    }

    return TRUE; //불량이 발생해도 검사 실패만 아니면 return TRUE을 날리자. 그래야 계속 검사 한다.
}

//영훈 수정
BOOL VisionInspectionBgaBottom2D::DoBallOffsetX(
    const CString strBallContrastName, const std::vector<Package::Ball> BallData, BOOL bEachGroupInsp) //kircheis_MED2.5
{
    auto* result = m_resultGroup.GetResultByName(strBallContrastName);
    if (result == nullptr)
    {
        return FALSE;
    }
    auto* spec = GetSpecByName(result->m_resultName);
    if (spec == nullptr)
    {
        return FALSE;
    }

    if (spec->m_use == FALSE)
        return TRUE;

    auto* missingSpec = GetSpecByGuid(_T("{B0698F99-5390-40FB-AF77-1079512FA3BA}"));
    auto* missingResult = m_resultGroup.GetResultByGuid(_T("{B0698F99-5390-40FB-AF77-1079512FA3BA}"));
    if (missingSpec == nullptr || missingResult == nullptr)
    {
        return FALSE;
    }

    long idx = -1;
    BOOL CheckAllData = BallData.size() == m_packageSpec.m_ballMap->m_balls.size();

    if (!CheckAllData)
        result->Resize(CAST_LONG(BallData.size()));

    CString strCompID = m_packageSpec.m_originalballMap->m_strAllGroupName; //kircheis_MED2.5

    for (const auto& ball : BallData)
    {
        if (CheckAllData)
            idx = ball.m_index;
        else
            idx++;

        if (bEachGroupInsp) //kircheis_MED2.5
            strCompID = ball.m_groupID;

        if (SkipMissing(ball, spec, result, missingSpec, missingResult, strCompID, idx)) //kircheis_MED2.5
        {
            continue;
        }

        // 실좌표계는 오른손 좌표계이므로 Y 좌표에 -1 을 곱해줌
        const float curX_um
            = getScale().convert_pixelToUmX(m_sBallAlignResult->m_ballWidthCenterPos_px[ball.m_index].m_x);
        const float curY_um
            = -getScale().convert_pixelToUmY(m_sBallAlignResult->m_ballWidthCenterPos_px[ball.m_index].m_y);

        const float transformedX_um = (float)(curX_um * m_affineTransformRealToSpec[0][0]
            + curY_um * m_affineTransformRealToSpec[0][1] + m_affineTransformRealToSpec[0][2]);

        const float errorX_um = float(transformedX_um - ball.m_posX_um);

        result->SetValue_EachResult_AndTypicalValueBySingleSpec(idx, ball.m_name, ball.m_groupID, strCompID, errorX_um,
            *spec, (float)ball.m_posX_um, (float)ball.m_posY_um); //kircheis_MED2.5
        result->SetRect(idx, m_vecrtBallPosition_pixel[ball.m_index]);
    }

    result->UpdateTypicalValue(spec);
    result->SetTotalResult();

    strCompID.Empty();

    return TRUE; //불량이 발생해도 검사 실패만 아니면 return TRUE을 날리자. 그래야 계속 검사 한다.
}

BOOL VisionInspectionBgaBottom2D::DoBallOffsetX()
{
    DoBallOffsetX(g_szBallInspectionName[BALL_INSPECTION_BALL_OFFSET_X], m_packageSpec.m_ballMap->m_balls);

    //Group Object 구간
    long GroupSpecCount = CAST_LONG(m_pVisionPara->m_vecstrGroupInspName.size());

    if (GroupSpecCount != 0 && GroupSpecCount != m_packageSpec.m_originalballMap->m_ballTypes.size())
        return TRUE;

    for (int idx = 0; idx < GroupSpecCount; idx++)
    {
        if (!DoBallOffsetX(
                m_pVisionPara->m_vecstrGroupInspName[idx].BallOffsetXName, m_vec2BallData[idx], TRUE)) //kircheis_MED2.5
        {
            return TRUE;
        }
    }

    return TRUE; //불량이 발생해도 검사 실패만 아니면 return TRUE을 날리자. 그래야 계속 검사 한다.
}

BOOL VisionInspectionBgaBottom2D::DoBallOffsetY(
    const CString strBallContrastName, const std::vector<Package::Ball> BallData, BOOL bEachGroupInsp) //kircheis_MED2.5
{
    auto* result = m_resultGroup.GetResultByName(strBallContrastName);
    if (result == nullptr)
    {
        return FALSE;
    }
    auto* spec = GetSpecByName(result->m_resultName);
    if (spec == nullptr)
    {
        return FALSE;
    }

    if (spec->m_use == FALSE)
        return TRUE;

    auto* missingSpec = GetSpecByGuid(_T("{B0698F99-5390-40FB-AF77-1079512FA3BA}"));
    auto* missingResult = m_resultGroup.GetResultByGuid(_T("{B0698F99-5390-40FB-AF77-1079512FA3BA}"));
    if (missingSpec == nullptr || missingResult == nullptr)
    {
        return FALSE;
    }

    long idx = -1;
    BOOL CheckAllData = BallData.size() == m_packageSpec.m_ballMap->m_balls.size();

    if (!CheckAllData)
        result->Resize(CAST_LONG(BallData.size()));

    CString strCompID = m_packageSpec.m_originalballMap->m_strAllGroupName; //kircheis_MED2.5

    for (const auto& ball : BallData)
    {
        if (CheckAllData)
            idx = ball.m_index;
        else
            idx++;

        if (bEachGroupInsp) //kircheis_MED2.5
            strCompID = ball.m_groupID;

        if (SkipMissing(ball, spec, result, missingSpec, missingResult, strCompID, idx)) //kircheis_MED2.5
        {
            continue;
        }

        // 실좌표계는 오른손 좌표계이므로 Y 좌표에 -1 을 곱해줌
        const float curX_um
            = getScale().convert_pixelToUmX(m_sBallAlignResult->m_ballWidthCenterPos_px[ball.m_index].m_x);
        const float curY_um
            = -getScale().convert_pixelToUmY(m_sBallAlignResult->m_ballWidthCenterPos_px[ball.m_index].m_y);

        const float transformedY_um = (float)(curX_um * m_affineTransformRealToSpec[1][0]
            + curY_um * m_affineTransformRealToSpec[1][1] + m_affineTransformRealToSpec[1][2]);

        const float errorY_um = float(transformedY_um - ball.m_posY_um);

        result->SetValue_EachResult_AndTypicalValueBySingleSpec(idx, ball.m_name, ball.m_groupID, strCompID, errorY_um,
            *spec, (float)ball.m_posX_um, (float)ball.m_posY_um); //kircheis_MED2.5
        result->SetRect(idx, m_vecrtBallPosition_pixel[ball.m_index]);
    }

    result->UpdateTypicalValue(spec);
    result->SetTotalResult();

    strCompID.Empty();

    return TRUE; //불량이 발생해도 검사 실패만 아니면 return TRUE을 날리자. 그래야 계속 검사 한다.
}

BOOL VisionInspectionBgaBottom2D::DoBallOffsetY()
{
    DoBallOffsetY(g_szBallInspectionName[BALL_INSPECTION_BALL_OFFSET_Y], m_packageSpec.m_ballMap->m_balls);

    //Group Object 구간
    long GroupSpecCount = CAST_LONG(m_pVisionPara->m_vecstrGroupInspName.size());

    if (GroupSpecCount != 0 && GroupSpecCount != m_packageSpec.m_originalballMap->m_ballTypes.size())
        return TRUE;

    for (int idx = 0; idx < GroupSpecCount; idx++)
    {
        if (!DoBallOffsetY(
                m_pVisionPara->m_vecstrGroupInspName[idx].BallOffsetYName, m_vec2BallData[idx], TRUE)) //kircheis_MED2.5
        {
            return TRUE;
        }
    }

    return TRUE; //불량이 발생해도 검사 실패만 아니면 return TRUE을 날리자. 그래야 계속 검사 한다.
}

BOOL VisionInspectionBgaBottom2D::DoBallOffsetR(
    const CString strBallContrastName, const std::vector<Package::Ball> BallData, BOOL bEachGroupInsp) //kircheis_MED2.5
{
    auto* result = m_resultGroup.GetResultByName(strBallContrastName);
    if (result == nullptr)
    {
        return FALSE;
    }
    auto* spec = GetSpecByName(result->m_resultName);
    if (spec == nullptr)
    {
        return FALSE;
    }

    if (spec->m_use == FALSE)
        return TRUE;

    auto* missingSpec = GetSpecByGuid(_T("{B0698F99-5390-40FB-AF77-1079512FA3BA}"));
    auto* missingResult = m_resultGroup.GetResultByGuid(_T("{B0698F99-5390-40FB-AF77-1079512FA3BA}"));
    if (missingSpec == nullptr || missingResult == nullptr)
    {
        return FALSE;
    }

    long idx = -1;
    BOOL CheckAllData = BallData.size() == m_packageSpec.m_ballMap->m_balls.size();

    if (!CheckAllData)
        result->Resize(CAST_LONG(BallData.size()));

    CString strCompID = m_packageSpec.m_originalballMap->m_strAllGroupName; //kircheis_MED2.5

    for (const auto& ball : BallData)
    {
        if (CheckAllData)
            idx = ball.m_index;
        else
            idx++;

        if (bEachGroupInsp) //kircheis_MED2.5
            strCompID = ball.m_groupID;

        if (SkipMissing(ball, spec, result, missingSpec, missingResult, strCompID, idx)) //kircheis_MED2.5
        {
            continue;
        }

        // 실좌표계는 오른손 좌표계이므로 Y 좌표에 -1 을 곱해줌
        const float curX_um
            = getScale().convert_pixelToUmX(m_sBallAlignResult->m_ballWidthCenterPos_px[ball.m_index].m_x);
        const float curY_um
            = -getScale().convert_pixelToUmY(m_sBallAlignResult->m_ballWidthCenterPos_px[ball.m_index].m_y);

        const float transformedX_um = (float)(curX_um * m_affineTransformRealToSpec[0][0]
            + curY_um * m_affineTransformRealToSpec[0][1] + m_affineTransformRealToSpec[0][2]);
        const float transformedY_um = (float)(curX_um * m_affineTransformRealToSpec[1][0]
            + curY_um * m_affineTransformRealToSpec[1][1] + m_affineTransformRealToSpec[1][2]);

        const float errorX_um = float(transformedX_um - ball.m_posX_um);
        const float errorY_um = float(transformedY_um - ball.m_posY_um);

        const float errorR_um = CAST_FLOAT(sqrt(CAST_DOUBLE(errorX_um * errorX_um + errorY_um * errorY_um)));

        result->SetValue_EachResult_AndTypicalValueBySingleSpec(idx, ball.m_name, ball.m_groupID, strCompID, errorR_um,
            *spec, (float)ball.m_posX_um, (float)ball.m_posY_um); //kircheis_MED2.5
        result->SetRect(idx, m_vecrtBallPosition_pixel[ball.m_index]);
    }

    result->UpdateTypicalValue(spec);
    result->SetTotalResult();

    strCompID.Empty();

    return TRUE; //불량이 발생해도 검사 실패만 아니면 return TRUE을 날리자. 그래야 계속 검사 한다.
}

BOOL VisionInspectionBgaBottom2D::DoBallOffsetR()
{
    DoBallOffsetR(g_szBallInspectionName[BALL_INSPECTION_BALL_OFFSET_R], m_packageSpec.m_ballMap->m_balls);

    //Group Object 구간
    long GroupSpecCount = CAST_LONG(m_pVisionPara->m_vecstrGroupInspName.size());

    if (GroupSpecCount != 0 && GroupSpecCount != m_packageSpec.m_originalballMap->m_ballTypes.size())
        return TRUE;

    for (int idx = 0; idx < GroupSpecCount; idx++)
    {
        if (!DoBallOffsetR(
                m_pVisionPara->m_vecstrGroupInspName[idx].BallOffsetRName, m_vec2BallData[idx], TRUE)) //kircheis_MED2.5
        {
            return TRUE;
        }
    }

    return TRUE; //불량이 발생해도 검사 실패만 아니면 return TRUE을 날리자. 그래야 계속 검사 한다.
}

BOOL VisionInspectionBgaBottom2D::DoBallGridOffsetXByKircheis(
    const CString strBallContrastName, const std::vector<Package::Ball> BallData, BOOL bEachGroupInsp) //kircheis_MED2.5
{
    //검사의 접근 방식을 바꾼다. 먼저 Body Align의 x축 Center Line을 구하고, 모든 Ball을 그 Line에 Projection 한다.
    // 그리고 X Center Line의 시작 점을 구하고 모든 Ball이 그 시작 점과의 거리를 계산한 후 Spec상의 Ball 좌표에 Body Center X 좌표를 보상하고 그 차이를 구한다.
    // 즉 Image 원점 좌표계에서 Tilt를 보상한 후 Spec 좌표를 Image 좌표로 매칭해준다는 거임... 기존과 반대 방식이다.

    auto* result = m_resultGroup.GetResultByName(strBallContrastName);
    if (result == nullptr)
    {
        return FALSE;
    }
    auto* spec = GetSpecByName(result->m_resultName);
    if (spec == nullptr)
    {
        return FALSE;
    }

    if (spec->m_use == FALSE)
        return TRUE;

    const long imageSizeY = getImageLotInsp().GetImageSizeY();

    //{{ //kircheis_Nantong
    Ipvm::LineEq32r line;
    Ipvm::LineEq32r lineOrigin;
    Ipvm::Point32r2 fptImageOrigin;

    std::vector<Ipvm::Point32r2> vecfptBody(2);
    //Body Align의 x축 Center Line을 구하고
    vecfptBody[0].m_x = (m_sEdgeAlignResult->fptLT.m_x + m_sEdgeAlignResult->fptLB.m_x) * .5f;
    vecfptBody[0].m_y = (m_sEdgeAlignResult->fptLT.m_y + m_sEdgeAlignResult->fptLB.m_y) * .5f;
    vecfptBody[1].m_x = (m_sEdgeAlignResult->fptRT.m_x + m_sEdgeAlignResult->fptRB.m_x) * .5f;
    vecfptBody[1].m_y = (m_sEdgeAlignResult->fptRT.m_y + m_sEdgeAlignResult->fptRB.m_y) * .5f;
    Ipvm::DataFitting::FitToLine(2, &vecfptBody[0], line);

    //X Center Line의 시작 점을 구하고(Image 좌표 기준. 즉 Body Center Line의 좌측 끝 좌표)
    vecfptBody[0].m_x = 0.f;
    vecfptBody[0].m_y = 0.f;
    vecfptBody[1].m_x = 0.f;
    vecfptBody[1].m_y = (float)imageSizeY;
    Ipvm::DataFitting::FitToLine(2, &vecfptBody[0], lineOrigin);
    Ipvm::Geometry::GetCrossPoint(line, lineOrigin, fptImageOrigin);
    //}}

    float fCurDistX = 0.f;
    float fSpecDistX = 0.f;
    long nBallCount = 0;

    BOOL CheckAllData = BallData.size() == m_packageSpec.m_ballMap->m_balls.size();

    if (!CheckAllData)
        result->Resize(1);

    CString strCompID = m_packageSpec.m_originalballMap->m_strAllGroupName; //kircheis_MED2.5

    for (const auto& ball : BallData)
    {
        Ipvm::Point32r2 fptCalPos;

        //모든 Ball을 Body Center Line에 Projection.
        Ipvm::Geometry::GetFootOfPerpendicular(
            line, m_sBallAlignResult->m_ballWidthCenterPos_px[ball.m_index], fptCalPos);

        //시작 점과의 거리를 계산한 후 누적
        float distance = 0.f;
        Ipvm::Geometry::GetDistance(fptImageOrigin, fptCalPos, distance);
        fCurDistX += distance;

        //Spec상의 Ball 좌표에 Body Center X 좌표를 보상한 후 누적
        fSpecDistX += float(ball.m_posX_px + m_sEdgeAlignResult->m_center.m_x);

        nBallCount++;

        if (bEachGroupInsp) //kircheis_MED2.5
            strCompID = ball.m_groupID;
    }

    const float fAvg = (fCurDistX - fSpecDistX) / (float)nBallCount;
    const float fError = fAvg * m_f2DScale;

    result->SetValue_EachResult_AndTypicalValueBySingleSpec(
        0, _T("0"), strCompID, strCompID, fError, *spec, 0.f, 0.f); //kircheis_MED2.5
    result->UpdateTypicalValue(spec);
    result->SetTotalResult();

    strCompID.Empty();

    return TRUE; //불량이 발생해도 검사 실패만 아니면 return TRUE을 날리자. 그래야 계속 검사 한다.
}

BOOL VisionInspectionBgaBottom2D::DoBallGridOffsetXByKircheis()
{
    DoBallGridOffsetXByKircheis(
        g_szBallInspectionName[BALL_INSPECTION_BALL_GRID_OFFSET_X], m_packageSpec.m_ballMap->m_balls);

    //Group Object 구간
    long GroupSpecCount = CAST_LONG(m_pVisionPara->m_vecstrGroupInspName.size());

    if (GroupSpecCount != 0 && GroupSpecCount != m_packageSpec.m_originalballMap->m_ballTypes.size())
        return TRUE;

    for (int idx = 0; idx < GroupSpecCount; idx++)
    {
        if (!DoBallGridOffsetXByKircheis(m_pVisionPara->m_vecstrGroupInspName[idx].BallGridOffsetXName,
                m_vec2BallData[idx], TRUE)) //kircheis_MED2.5
        {
            return TRUE;
        }
    }

    return TRUE; //불량이 발생해도 검사 실패만 아니면 return TRUE을 날리자. 그래야 계속 검사 한다.
}

BOOL VisionInspectionBgaBottom2D::DoBallGridOffsetYByKircheis(
    const CString strBallContrastName, const std::vector<Package::Ball> BallData, BOOL bEachGroupInsp) //kircheis_MED2.5
{ //함수 내용에 대한 설명은 DoBallGridOffsetXByKircheis() 참조
    auto* result = m_resultGroup.GetResultByName(strBallContrastName);
    if (result == nullptr)
    {
        return FALSE;
    }
    auto* spec = GetSpecByName(result->m_resultName);
    if (spec == nullptr)
    {
        return FALSE;
    }

    if (spec->m_use == FALSE)
        return TRUE;

    const long imageSizeX = getImageLotInsp().GetImageSizeX();

    //{{ //kircheis_Nantong
    Ipvm::LineEq32r line;
    Ipvm::LineEq32r lineOrigin;
    Ipvm::Point32r2 fptImageOrigin;

    std::vector<Ipvm::Point32r2> vecfptBody(2);
    //Body Align의 Y축 Center Line을 구하고
    vecfptBody[0].m_x = (m_sEdgeAlignResult->fptLT.m_x + m_sEdgeAlignResult->fptRT.m_x) * .5f;
    vecfptBody[0].m_y = (m_sEdgeAlignResult->fptLT.m_y + m_sEdgeAlignResult->fptRT.m_y) * .5f;
    vecfptBody[1].m_x = (m_sEdgeAlignResult->fptLB.m_x + m_sEdgeAlignResult->fptRB.m_x) * .5f;
    vecfptBody[1].m_y = (m_sEdgeAlignResult->fptLB.m_y + m_sEdgeAlignResult->fptRB.m_y) * .5f;
    Ipvm::DataFitting::FitToLine(2, &vecfptBody[0], line);

    //Y Center Line의 시작 점을 구하고(Image 좌표 기준. 즉 Body Center Line의 상단 끝 좌표)
    vecfptBody[0].m_x = 0.f;
    vecfptBody[0].m_y = 0.f;
    vecfptBody[1].m_x = (float)imageSizeX;
    vecfptBody[1].m_y = 0.f;
    Ipvm::DataFitting::FitToLine(2, &vecfptBody[0], lineOrigin);
    Ipvm::Geometry::GetCrossPoint(line, lineOrigin, fptImageOrigin);
    //}}

    float fCurDistY = 0.f;
    float fSpecDistY = 0.f;
    long nBallCount = 0;

    BOOL CheckAllData = BallData.size() == m_packageSpec.m_ballMap->m_balls.size();

    if (!CheckAllData)
        result->Resize(1);

    CString strCompID = m_packageSpec.m_originalballMap->m_strAllGroupName; //kircheis_MED2.5

    for (const auto& ball : BallData)
    {
        Ipvm::Point32r2 fptCalPos;

        //모든 Ball을 Body Center Line에 Projection.
        Ipvm::Geometry::GetFootOfPerpendicular(
            line, m_sBallAlignResult->m_ballWidthCenterPos_px[ball.m_index], fptCalPos);

        //시작 점과의 거리를 계산한 후 누적
        float distance = 0.f;
        Ipvm::Geometry::GetDistance(fptImageOrigin, fptCalPos, distance);
        fCurDistY += distance;

        //Spec상의 Ball 좌표에 Body Center X 좌표를 보상한 후 누적
        fSpecDistY += (float)(ball.m_posY_px + m_sEdgeAlignResult->m_center.m_y);

        nBallCount++;

        if (bEachGroupInsp) //kircheis_MED2.5
            strCompID = ball.m_groupID;
    }

    const float fAvg = (fCurDistY - fSpecDistY) / (float)nBallCount;
    const float fError = fAvg * m_f2DScale;

    result->SetValue_EachResult_AndTypicalValueBySingleSpec(
        0, _T("0"), strCompID, strCompID, fError, *spec, 0.f, 0.f); //kircheis_MED2.5
    result->UpdateTypicalValue(spec);
    result->SetTotalResult();

    strCompID.Empty();

    return TRUE; //불량이 발생해도 검사 실패만 아니면 return TRUE을 날리자. 그래야 계속 검사 한다.
}

BOOL VisionInspectionBgaBottom2D::DoBallGridOffsetYByKircheis()
{
    DoBallGridOffsetYByKircheis(
        g_szBallInspectionName[BALL_INSPECTION_BALL_GRID_OFFSET_Y], m_packageSpec.m_ballMap->m_balls);

    //Group Object 구간
    long GroupSpecCount = CAST_LONG(m_pVisionPara->m_vecstrGroupInspName.size());

    if (GroupSpecCount != 0 && GroupSpecCount != m_packageSpec.m_originalballMap->m_ballTypes.size())
        return TRUE;

    for (int idx = 0; idx < GroupSpecCount; idx++)
    {
        if (!DoBallGridOffsetYByKircheis(m_pVisionPara->m_vecstrGroupInspName[idx].BallGridOffsetYName,
                m_vec2BallData[idx], TRUE)) //kircheis_MED2.5
        {
            return TRUE;
        }
    }

    return TRUE; //불량이 발생해도 검사 실패만 아니면 return TRUE을 날리자. 그래야 계속 검사 한다.
}

BOOL VisionInspectionBgaBottom2D::DoBallWidth(
    const CString strBallContrastName, const std::vector<Package::Ball> BallData, BOOL bEachGroupInsp) //kircheis_MED2.5
{
    auto* result = m_resultGroup.GetResultByName(strBallContrastName);
    if (result == nullptr)
    {
        return FALSE;
    }
    auto* spec = GetSpecByName(result->m_resultName);
    if (spec == nullptr)
    {
        return FALSE;
    }

    if (spec->m_use == FALSE)
        return TRUE;

    auto* missingSpec = GetSpecByGuid(_T("{B0698F99-5390-40FB-AF77-1079512FA3BA}"));
    auto* missingResult = m_resultGroup.GetResultByGuid(_T("{B0698F99-5390-40FB-AF77-1079512FA3BA}"));
    if (missingSpec == nullptr || missingResult == nullptr)
    {
        return FALSE;
    }

    long idx = -1;
    BOOL CheckAllData = BallData.size() == m_packageSpec.m_ballMap->m_balls.size();

    if (!CheckAllData)
        result->Resize(CAST_LONG(BallData.size()));

    CString strCompID = m_packageSpec.m_originalballMap->m_strAllGroupName; //kircheis_MED2.5

    for (const auto& ball : BallData)
    {
        if (CheckAllData)
            idx = ball.m_index;
        else
            idx++;

        if (bEachGroupInsp) //kircheis_MED2.5
            strCompID = ball.m_groupID;

        if (SkipMissing(ball, spec, result, missingSpec, missingResult, strCompID, idx,
                float(ball.m_diameter_um))) //kircheis_MED2.5
        {
            continue;
        }

        const float ballWidth_px = m_sBallAlignResult->m_ballWidths_px[ball.m_index];

        const float fError = float(ballWidth_px * m_f2DScale - ball.m_diameter_um);

        result->SetValue_EachResult_AndTypicalValueBySingleSpec(idx, ball.m_name, ball.m_groupID, strCompID, fError,
            *spec, (float)ball.m_posX_um, (float)ball.m_posY_um, float(ball.m_diameter_um)); //kircheis_MED2.5
        result->SetRect(idx, m_vecrtBallPosition_pixel[ball.m_index]);
    }

    result->UpdateTypicalValue(spec);
    result->SetTotalResult();

    strCompID.Empty();

    return TRUE; //불량이 발생해도 검사 실패만 아니면 return TRUE을 날리자. 그래야 계속 검사 한다.
}

BOOL VisionInspectionBgaBottom2D::DoBallWidth()
{
    DoBallWidth(g_szBallInspectionName[BALL_INSPECTION_BALL_WIDTH], m_packageSpec.m_ballMap->m_balls);

    //Group Object 구간
    long GroupSpecCount = CAST_LONG(m_pVisionPara->m_vecstrGroupInspName.size());

    if (GroupSpecCount != 0 && GroupSpecCount != m_packageSpec.m_originalballMap->m_ballTypes.size())
        return TRUE;

    for (int idx = 0; idx < GroupSpecCount; idx++)
    {
        if (!DoBallWidth(
                m_pVisionPara->m_vecstrGroupInspName[idx].BallWidthName, m_vec2BallData[idx], TRUE)) //kircheis_MED2.5
        {
            return TRUE;
        }
    }

    return TRUE; //불량이 발생해도 검사 실패만 아니면 return TRUE을 날리자. 그래야 계속 검사 한다.
}

BOOL VisionInspectionBgaBottom2D::DoBallEllipticity(const bool detailSetupMode) //kircheis_TMI
{
    //{{//Spec & Result
    auto* result = m_resultGroup.GetResultByName(g_szBallInspectionName[BALL_INSPECTION_BALL_ELLIPTICITY]);
    if (result == nullptr)
    {
        return FALSE;
    }
    auto* spec = GetSpecByName(result->m_resultName);
    if (spec == nullptr)
    {
        return FALSE;
    }

    //kk GroupInspection으로 만들기 위한 Spec 및 Result 정의
    long nballTypeCount = CAST_LONG(m_pVisionPara->m_vecstrGroupInspName.size());
    BOOL GroupInspectionUse = FALSE;

    if (nballTypeCount != 0 && nballTypeCount != m_packageSpec.m_originalballMap->m_ballTypes.size())
        return TRUE;

    std::vector<VisionInspectionResult*> GroupResult;
    std::vector<VisionInspectionSpec*> GroupSpec;
    std::vector<int> GroupIdx;

    GroupResult.resize(nballTypeCount);
    GroupSpec.resize(nballTypeCount);
    GroupIdx.resize(nballTypeCount);

    for (int i = 0; i < nballTypeCount; i++)
    {
        GroupResult[i] = m_resultGroup.GetResultByName(m_pVisionPara->m_vecstrGroupInspName[i].BallEllipticityName);
        if (GroupResult[i] == nullptr)
        {
            return FALSE;
        }
        GroupSpec[i] = GetSpecByName(result->m_resultName);
        if (GroupSpec[i] == nullptr)
        {
            return FALSE;
        }
        GroupIdx[i] = 0;

        BOOL UseInsp = GroupSpec[i]->m_use;

        GroupInspectionUse = GroupInspectionUse || UseInsp;

        if (UseInsp)
            GroupResult[i]->Resize(CAST_LONG(m_vec2BallData[i].size()));
    }

    if (spec->m_use == FALSE && GroupInspectionUse == FALSE)
        return TRUE;

    auto* missingSpec = GetSpecByGuid(_T("{B0698F99-5390-40FB-AF77-1079512FA3BA}"));
    auto* missingResult = m_resultGroup.GetResultByGuid(_T("{B0698F99-5390-40FB-AF77-1079512FA3BA}"));
    if (missingSpec == nullptr || missingResult == nullptr)
    {
        return FALSE;
    }
    //}}//Spec & Result

    Ipvm::Image8u invertCoaxial;
    if (!getReusableMemory().GetInspByteImage(invertCoaxial))
        return FALSE;
    Ipvm::ImageProcessing::BitwiseNot(
        m_pVisionPara->m_coaxialImageIndex.getImage(false), Ipvm::Rect32s(m_coaxialImage), invertCoaxial);

    //{{//알고리즘
    std::vector<BYTE> vbyLineValue(0);
    std::vector<BYTE> vbyLineValue_Reverse(0);
    std::vector<Ipvm::Point32r2> vfCirclePointXY(0);
    std::vector<Ipvm::Point32r2> vfCirclePointXYfiltered(0);
    std::vector<Ipvm::Point32r2> vfCirclePointXYforDebug(0);
    std::vector<Ipvm::LineSeg32r> vfCircleSweepLine(0);
    std::vector<Ipvm::EllipseEq32r> vecEllipse(0);

    std::vector<float> vecfBallCircularityValue(m_packageSpec.m_ballMap->m_balls.size()); //Result

    float fUMToPixel = getScale().umToPixelXY();

    float fSpecBallWidth(0.f), fBallHalfWidth(0.f), fInX(0.f), fInY(0.f), fEdgeThreshOld(0.f), fEdgeValue(0.f),
        fEdgeRadius(0.f);
    float fDistX(0.f), fDistY(0.f);
    long nNewValidEdgeCount(0), nSectionNumber(0), nLineValue(0), nRoughLineNum(0), nValidEdgeCount(0);
    BOOL bSubPixelingDone(TRUE);
    CPoint ptEnd;
    Ipvm::CircleEq32r CircleCoeff;

    for (const auto& ball : m_packageSpec.m_ballMap->m_balls)
    {
        const auto& ballparameter = m_pVisionPara->m_parameters[ball.m_typeIndex];

        float* pfCos = &m_vecfCosForBallCenter[ball.m_typeIndex][0];
        float* pfSin = &m_vecfSinForBallCenter[ball.m_typeIndex][0];

        vfCirclePointXY.clear();
        vfCirclePointXYfiltered.clear();
        nSectionNumber = ballparameter.m_nBallEdgeCountMode == enumBallEdgeFastMode
            ? SPEC_WIDTH_SWEEP_LINE_COUNT_FAST
            : SPEC_WIDTH_SWEEP_LINE_COUNT_DETAIL;
        fSpecBallWidth = float(ball.m_diameter_um * fUMToPixel);
        fBallHalfWidth = fSpecBallWidth * 0.5f;
        const float fSweepLength = float(fSpecBallWidth * ballparameter.m_ballWidthRoughSearchLengthRatio);
        const long nBallIndex = ball.m_index;

        // 이 검사 항목의 알고리즘은 기본적인 Align을 한 다음에 동작하니까 기준 위치는 Spec Pos가 아닌 Align Result를 쓴다.
        fInX = m_sBallAlignResult->m_ballWidthCenterPos_px[nBallIndex].m_x;
        fInY = m_sBallAlignResult->m_ballWidthCenterPos_px[nBallIndex].m_y;

        if (fInX <= 0 || fInY <= 0)
        {
            vecfBallCircularityValue[nBallIndex] = Ipvm::k_noiseValue32r;
            continue;
        }

        vfCirclePointXY.clear();
        vfCirclePointXY.resize(nSectionNumber, Ipvm::Point32r2(0.f, 0.f));

        for (long nSection = 0; nSection < nSectionNumber; nSection++)
        {
            const float fOutX = fInX + fSweepLength * pfCos[nSection];
            const float fOutY = fInY + fSweepLength * pfSin[nSection];

            nLineValue = CippModules::GetLineData(
                invertCoaxial, fOutX, fOutY, fInX, fInY, pfCos[nSection], pfSin[nSection], NULL);
            nRoughLineNum = nLineValue;
            vbyLineValue.clear();
            vbyLineValue.resize(nLineValue);
            CippModules::GetLineData(
                invertCoaxial, fInX, fInY, fOutX, fOutY, pfCos[nSection], pfSin[nSection], &vbyLineValue[0]);

            vbyLineValue_Reverse = vbyLineValue;
            for (long n = 0; n < nLineValue; n++)
            {
                vbyLineValue_Reverse[nLineValue - n - 1] = vbyLineValue[n];
            }
            vfCircleSweepLine.emplace_back(fInX, fInY, fOutX, fOutY);

            fEdgeThreshOld = m_pEdgeDetect->SetMininumThreshold(3.f);
            if (!m_pEdgeDetect->PI_ED_CalcEdgePointFromLine(PI_ED_DIR_RISING, nRoughLineNum, &vbyLineValue_Reverse[0],
                    fEdgeRadius, fEdgeValue, bSubPixelingDone, TRUE)) //FALSE ))//kircheis_ShinyTest//kircheis_NantongBO
            {
                m_pEdgeDetect->SetMininumThreshold(fEdgeThreshOld);
                continue;
            }
            m_pEdgeDetect->SetMininumThreshold(fEdgeThreshOld);

            fEdgeRadius = nLineValue - fEdgeRadius;

            vfCirclePointXY[nSection].m_x = fInX + fEdgeRadius * pfCos[nSection];
            vfCirclePointXY[nSection].m_y = fInY + fEdgeRadius * pfSin[nSection];
            vfCirclePointXYforDebug.push_back(vfCirclePointXY[nSection]);
        }
        for (long n = (long)vfCirclePointXY.size() - 1; n >= 0; n--)
        {
            if (vfCirclePointXY[n].m_x <= 0.f || vfCirclePointXY[n].m_y <= 0.f)
            {
                vfCirclePointXY.erase(vfCirclePointXY.begin() + n);
            }
        }
        nValidEdgeCount = (long)vfCirclePointXY.size();

        if (nValidEdgeCount <= 0)
        {
            vecfBallCircularityValue[nBallIndex] = Ipvm::k_noiseValue32r;
            continue;
        }

        Ipvm::DataFitting::FitToCircle(nValidEdgeCount, &vfCirclePointXY[0], CircleCoeff);

        // 구한 원의 원주에 대해서 일정 거리 이상 떨어진 놈들을 없애고 다시 피팅.
        // 얘를 안하면 노이즈가 존재할 경우 서클 피팅시 원이 이동해서 정상적인 Point들도 오차 성분이 있는걸로 계산된다.
        const float fSpec_Temp = SPEC_BALL_EDGE_FILTERING * CircleCoeff.m_radius;
        const float fRadiusSpec_Min = CircleCoeff.m_radius - fSpec_Temp;
        const float fRadiusSpec_Max = CircleCoeff.m_radius + fSpec_Temp;

        nNewValidEdgeCount = 0;
        for (long nSection = 0; nSection < nValidEdgeCount; nSection++)
        {
            fDistX = vfCirclePointXY[nSection].m_x - CircleCoeff.m_x;
            fDistY = vfCirclePointXY[nSection].m_y - CircleCoeff.m_y;
            fEdgeRadius = CAST_FLOAT(sqrt(CAST_DOUBLE(fDistX * fDistX + fDistY * fDistY)));

            // 찾은 Ball의 반지름이 입력한 값보다 크게 튀는 Edge는 필터링하여 다시 피팅한다.
            if (fEdgeRadius > fRadiusSpec_Min && fEdgeRadius < fRadiusSpec_Max)
            {
                vfCirclePointXYfiltered.push_back(vfCirclePointXY[nSection]);
                nNewValidEdgeCount++;
            }
        }

        if (nNewValidEdgeCount <= 0)
        {
            vecfBallCircularityValue[nBallIndex] = Ipvm::k_noiseValue32r;
            continue;
        }

        Ipvm::DataFitting::FitToCircle(nNewValidEdgeCount, &vfCirclePointXYfiltered[0], CircleCoeff);

        vecEllipse.emplace_back(CircleCoeff.m_x, CircleCoeff.m_y, CircleCoeff.m_radius, CircleCoeff.m_radius);

        vecfBallCircularityValue[nBallIndex] = GetBallEllipticity(vfCirclePointXY, CircleCoeff, TRUE, 0.f);
    }
    SetDebugInfoItem(detailSetupMode, _T("Ball Ellipticity Sweep Line"), vfCircleSweepLine);
    SetDebugInfoItem(detailSetupMode, _T("Ball Ellipticity Edge Point"), vfCirclePointXYforDebug);
    SetDebugInfoItem(detailSetupMode, _T("Ball Ellipticity Edge Point Circle"), vecEllipse);
    //}}//알고리즘

    //{{//판정부
    const CString strCompID = m_packageSpec.m_originalballMap->m_strAllGroupName; //kircheis_MED2.5
    CString strCompEachGroupID; //kircheis_MED2.5
    for (const auto& ball : m_packageSpec.m_ballMap->m_balls)
    {
        const long nBallIndex = ball.m_index;
        if (SkipMissing(ball, spec, result, missingSpec, missingResult, strCompID, nBallIndex,
                float(ball.m_diameter_um))) //kircheis_MED2.5
        {
            if (nballTypeCount > 0 && GroupSpec[ball.m_typeIndex]->m_use)
            {
                strCompEachGroupID
                    = m_packageSpec.m_originalballMap->m_ballTypes[ball.m_typeIndex].m_groupID; //kircheis_MED2.5
                SkipMissing(ball, GroupSpec[ball.m_typeIndex], GroupResult[ball.m_typeIndex], missingSpec,
                    missingResult, strCompEachGroupID, GroupIdx[ball.m_typeIndex],
                    float(ball.m_diameter_um)); //kircheis_MED2.5
                GroupIdx[ball.m_typeIndex]++;
            }

            continue;
        }

        if (spec->m_use)
        {
            result->SetValue_EachResult_AndTypicalValueBySingleSpec(nBallIndex, ball.m_name, ball.m_groupID, strCompID,
                vecfBallCircularityValue[nBallIndex], *spec, (float)ball.m_posX_um, (float)ball.m_posY_um,
                0.f); //kircheis_MED2.5
            result->SetRect(nBallIndex, m_vecrtBallPosition_pixel[nBallIndex]);
        }
        //kk GroupResult
        if (nballTypeCount > 0 && GroupSpec[ball.m_typeIndex]->m_use)
        {
            GroupResult[ball.m_typeIndex]->SetValue_EachResult_AndTypicalValueBySingleSpec(GroupIdx[ball.m_typeIndex],
                ball.m_name, ball.m_groupID, ball.m_groupID, vecfBallCircularityValue[nBallIndex],
                *GroupSpec[ball.m_typeIndex], (float)ball.m_posX_um, (float)ball.m_posY_um, 0.f); //kircheis_MED2.5
            GroupResult[ball.m_typeIndex]->SetRect(GroupIdx[ball.m_typeIndex], m_vecrtBallPosition_pixel[nBallIndex]);

            GroupIdx[ball.m_typeIndex]++;
        }
    }

    if (spec->m_use)
    {
        result->UpdateTypicalValue(spec);
        result->SetTotalResult();
    }

    //kk GroupResult
    for (int i = 0; i < nballTypeCount; i++)
    {
        if (GroupSpec[i]->m_use)
        {
            GroupResult[i]->UpdateTypicalValue(GroupSpec[i]);
            GroupResult[i]->SetTotalResult();
        }
    }
    //}}//판정부

    strCompEachGroupID.Empty();

    return TRUE;
}

BOOL VisionInspectionBgaBottom2D::DoBallQuality(
    const CString strBallContrastName, const std::vector<Package::Ball> BallData, BOOL bEachGroupInsp) //kircheis_MED2.5
{
    auto* result = m_resultGroup.GetResultByName(strBallContrastName);
    if (result == nullptr)
    {
        return FALSE;
    }
    auto* spec = GetSpecByName(result->m_resultName);
    if (spec == nullptr)
    {
        return FALSE;
    }

    if (spec->m_use == FALSE)
        return TRUE;

    float* pfValue = &result->m_objectErrorValues[0];

    auto* missingSpec = GetSpecByGuid(_T("{B0698F99-5390-40FB-AF77-1079512FA3BA}"));
    auto* missingResult = m_resultGroup.GetResultByGuid(_T("{B0698F99-5390-40FB-AF77-1079512FA3BA}"));
    if (missingSpec == nullptr || missingResult == nullptr)
    {
        return FALSE;
    }

    long idx = -1;
    BOOL CheckAllData = BallData.size() == m_packageSpec.m_ballMap->m_balls.size();

    CString strCompID = m_packageSpec.m_originalballMap->m_strAllGroupName; //kircheis_MED2.5

    for (const auto& ball : BallData)
    {
        if (CheckAllData)
            idx = ball.m_index;
        else
            idx++;

        if (bEachGroupInsp) //kircheis_MED2.5
            strCompID = ball.m_groupID;

        if (SkipMissing(ball, spec, result, missingSpec, missingResult, strCompID, idx, 100.f)) //kircheis_MED2.5
        {
            continue;
        }

        // 각각의 검사한 Value를 Result에...
        const float fError = min(100.f, pfValue[idx]);

        result->SetValue_EachResult_AndTypicalValueBySingleSpec(idx, ball.m_name, ball.m_groupID, strCompID, fError,
            *spec, (float)ball.m_posX_um, (float)ball.m_posY_um, 100.f); //kircheis_MED2.5
        result->SetRect(idx, m_vecrtBallPosition_pixel[ball.m_index]);
    }

    result->UpdateTypicalValue(spec);
    result->SetTotalResult();

    strCompID.Empty();

    return TRUE; //불량이 발생해도 검사 실패만 아니면 return TRUE을 날리자. 그래야 계속 검사 한다.
}

BOOL VisionInspectionBgaBottom2D::DoBallQuality()
{
    DoBallQuality(g_szBallInspectionName[BALL_INSPECTION_BALL_QUALITY], m_packageSpec.m_ballMap->m_balls);

    //Group Object 구간
    long GroupSpecCount = CAST_LONG(m_pVisionPara->m_vecstrGroupInspName.size());

    if (GroupSpecCount != 0 && GroupSpecCount != m_packageSpec.m_originalballMap->m_ballTypes.size())
        return TRUE;

    for (int idx = 0; idx < GroupSpecCount; idx++)
    {
        if (!DoBallQuality(
                m_pVisionPara->m_vecstrGroupInspName[idx].BallQualityName, m_vec2BallData[idx], TRUE)) //kircheis_MED2.5
        {
            return TRUE;
        }
    }

    return TRUE; //불량이 발생해도 검사 실패만 아니면 return TRUE을 날리자. 그래야 계속 검사 한다.
}

BOOL VisionInspectionBgaBottom2D::Qualtiy_BallAreaPixelValidCount(
    const Ipvm::Image8u& combine, long nQualityThresh, std::vector<long>& vecnOverPixelCount)
{
    Ipvm::Rect32s rtROI;
    std::vector<long> vecnValidPixelCount;

    BYTE byGrayValue;
    long nInnerPixelCount = 0;
    long nTotalValidPixelCount = 0;
    long nValidPixelAVG = 0;

    for (const auto& ball : m_packageSpec.m_ballMap->m_balls)
    {
        float fWidth = m_sBallAlignResult->m_ballWidths_px[ball.m_index];

        float fCenX = m_sBallAlignResult->m_ballWidthCenterPos_px[ball.m_index].m_x;
        float fCenY = m_sBallAlignResult->m_ballWidthCenterPos_px[ball.m_index].m_y;
        float fRadius = fWidth * 0.5f;

        rtROI.m_left = (long)(fCenX - fRadius + 0.5f);
        rtROI.m_right = (long)(fCenX + fRadius + 0.5f);
        rtROI.m_top = (long)(fCenY - fRadius + 0.5f);
        rtROI.m_bottom = (long)(fCenY + fRadius + 0.5f);

        nInnerPixelCount = 0;

        for (long y = rtROI.m_top; y < rtROI.m_bottom; y++)
        {
            auto* combine_y = combine.GetMem(0, y);
            for (long x = rtROI.m_left; x < rtROI.m_right; x++)
            {
                // 20141103 영훈 : Ball 내부의 면적을 알아온다.
                float fDistToCenterSqr
                    = CAST_FLOAT(sqrt(CAST_DOUBLE((x - fCenX) * (x - fCenX) + (y - fCenY) * (y - fCenY))));

                if (fDistToCenterSqr <= fRadius)
                {
                    byGrayValue = combine_y[x];
                    if (nQualityThresh < byGrayValue)
                    {
                        nInnerPixelCount++;
                    }
                }
            }
        }

        if (nInnerPixelCount == 0)
            return FALSE;

        nTotalValidPixelCount += nInnerPixelCount;
        vecnValidPixelCount.push_back(nInnerPixelCount);
    }

    long nSize = (long)vecnValidPixelCount.size();
    nValidPixelAVG = nTotalValidPixelCount / nSize;
    nValidPixelAVG += long(nValidPixelAVG * 0.15f);

    vecnOverPixelCount.resize(nSize);

    for (long n = 0; n < nSize; n++)
    {
        if (vecnValidPixelCount[n] > nValidPixelAVG)
        {
            vecnOverPixelCount[n] = vecnValidPixelCount[n] - nValidPixelAVG;
        }
    }

    return TRUE;
}

BOOL VisionInspectionBgaBottom2D::GetBallContrast(const Ipvm::Image8u& image)
{
    const long imageSizeX = getImageLotInsp().GetImageSizeX();
    const long imageSizeY = getImageLotInsp().GetImageSizeY();

    std::vector<Ipvm::Rect32s> vecDebugInfoRect;

    const float fOuterRadiusParam = (float)(0.25f * 0.9f * 0.9f);

    for (const auto& ball : m_packageSpec.m_ballMap->m_balls)
    {
        const float fCenX = m_sBallAlignResult->m_ballWidthCenterPos_px[ball.m_index].m_x;
        const float fCenY = m_sBallAlignResult->m_ballWidthCenterPos_px[ball.m_index].m_y;
        const float fRadius = m_sBallAlignResult->m_ballWidths_px[ball.m_index] * 0.5f;

        long nLeft = (long)(fCenX - fRadius + 0.5f);
        long nTop = (long)(fCenY - fRadius + 0.5f);
        long nRight = (long)(fCenX + fRadius + 1.5f);
        long nBottom = (long)(fCenY + fRadius + 1.5f);

        if (nLeft < 0 || nRight < 0 || nTop < 0 || nBottom < 0 || nLeft >= imageSizeX || nRight >= imageSizeX
            || nTop >= imageSizeY || nBottom >= imageSizeY)
        {
            return FALSE;
        }

        m_sBallAlignResult->m_ballConstrastROIs[ball.m_index].m_left = nLeft;
        m_sBallAlignResult->m_ballConstrastROIs[ball.m_index].m_top = nTop;
        m_sBallAlignResult->m_ballConstrastROIs[ball.m_index].m_right = nRight;
        m_sBallAlignResult->m_ballConstrastROIs[ball.m_index].m_bottom = nBottom;

        long nWidthSqr = min((nRight - nLeft), (nBottom - nTop));
        nWidthSqr *= nWidthSqr;

        float fOuterRadiusSqr = fOuterRadiusParam * nWidthSqr;

        long nOuterPixelCount = 0;
        long nOuterIntensitySum = 0;

        for (long y = nTop; y < nBottom; y++)
        {
            auto* image_y = image.GetMem(0, y);
            for (long x = nLeft; x < nRight; x++)
            {
                // 20141103 영훈 : BGA Width의 내부까지 계산하는 코드
                float fDistToCenterSqr = (x - fCenX) * (x - fCenX) + (y - fCenY) * (y - fCenY);

                if (/*fDistToCenterSqr > fInnerRadiusSqr && */ fDistToCenterSqr <= fOuterRadiusSqr)
                {
                    nOuterIntensitySum += image_y[x];
                    nOuterPixelCount++;
                }
            }
        }

        if (nOuterPixelCount == 0)
            return FALSE;

        float fOuterAverageIntensity = float(nOuterIntensitySum) / nOuterPixelCount;
        float fBallContrast = fOuterAverageIntensity;

        m_vecDebugInfoValue.push_back(fBallContrast);
        m_sBallAlignResult->m_ballConstrasts[ball.m_index] = fOuterAverageIntensity;
    }

    return TRUE;
}

// 얘는 정보 수집 차원에서 무조건 검사하지만, 최종 결과는 검사 항목이 켜져있을 때에만 갱신한다.
BOOL VisionInspectionBgaBottom2D::DoBallMissing()
{
    auto* result = m_resultGroup.GetResultByName(g_szBallInspectionName[BALL_INSPECTION_BALL_MISSING]);
    if (result == nullptr)
    {
        return FALSE;
    }
    auto* spec = GetSpecByName(result->m_resultName);
    if (spec == nullptr)
    {
        return FALSE;
    }

    CString strCompID = m_packageSpec.m_originalballMap->m_strAllGroupName; //kircheis_MED2.5

    for (const auto& ball : m_packageSpec.m_ballMap->m_balls)
    {
        result->SetValue_EachResult_AndTypicalValueBySingleSpec(ball.m_index, ball.m_name, ball.m_groupID, strCompID,
            0.f, *spec, (float)ball.m_posX_um, (float)ball.m_posY_um); //kircheis_MED
        result->SetRect(ball.m_index, Ipvm::Conversion::ToRect32s(m_vecrtBallCircleROI[ball.m_index]));
    }

    long nMissingBallCount(1);
    for (const auto& missing : m_missingBalls)
    {
        const auto& ball = m_packageSpec.m_ballMap->m_balls[missing.m_ballIndex];

        result->SetValue_EachResult_AndTypicalValueBySingleSpec(ball.m_index, ball.m_name, ball.m_groupID, strCompID,
            CAST_FLOAT(nMissingBallCount), *spec, (float)ball.m_posX_um, (float)ball.m_posY_um); //kircheis_MED2.5
        result->SetRect(ball.m_index, missing.m_ballSpecROI);

        nMissingBallCount++;
    }

    if (spec->m_use)
    {
        result->UpdateTypicalValue(spec);
        result->SetTotalResult();
    }

    strCompID.Empty();

    return TRUE;
}

float VisionInspectionBgaBottom2D::GetBallContrastCompensation()
{
    long nSpecBallNum = (long)m_packageSpec.m_ballMap->m_balls.size();

    long nBegin = long(nSpecBallNum * SEPC_QUALITY_COMPENSATION_RANGE_MIN + 0.5f);
    long nEnd = long(nSpecBallNum * SEPC_QUALITY_COMPENSATION_RANGE_MAX + 0.5f) + 1;

    if (nEnd - nBegin < 1)
        return 0.f;

    std::vector<float> vecfCalcContrast(
        m_sBallAlignResult->m_ballConstrasts.begin(), m_sBallAlignResult->m_ballConstrasts.end());

    std::sort(vecfCalcContrast.begin(), vecfCalcContrast.end());

    float fBallContrastSum = 0.f;
    for (long idx = nBegin; idx < nEnd; idx++)
    {
        fBallContrastSum += vecfCalcContrast[idx];
    }

    return fBallContrastSum / (nEnd - nBegin);
}

BOOL VisionInspectionBgaBottom2D::GetBallRoughCenterByCoaxImageBallPeak(const bool detailSetupMode,
    const Ipvm::Image8u& coaxImage, const Ipvm::Point32r2 i_ptSpecBallCenter, const float i_fSearchRadius,
    Ipvm::Point32r2& o_ptRoughCenter, Ipvm::Image8u& o_imageBallPeakDebugThresh) //kircheis_LKF
{
    UNREFERENCED_PARAMETER(detailSetupMode);

    Ipvm::Rect32s rtROI{};
    Ipvm::Rect32s rtMaxBlobROI{};
    Ipvm::CircleEq32r ballSearchAreaCircle;
    BYTE bMaxIntensity(0);
    long nThreshold(0), nMaxBlobArea(0), nMaxBlobLabel(0);
    int32_t nCurBlobNum(0), nMaxBlobNum(100);
    long nSumX(0), nSumY(0), nSumCnt(0), nImageSizeX(0);

    //검사 영역 만들고... 실제 연산 함수들에 사용할 사각형과 마스크만들데 쓸 써클
    rtROI.m_left = (long)(i_ptSpecBallCenter.m_x - i_fSearchRadius + .5f) - 2;
    rtROI.m_top = (long)(i_ptSpecBallCenter.m_y - i_fSearchRadius + .5f) - 2;
    rtROI.m_right = (long)(i_ptSpecBallCenter.m_x + i_fSearchRadius + .5f) + 2;
    rtROI.m_bottom = (long)(i_ptSpecBallCenter.m_y + i_fSearchRadius + .5f) + 2;

    ballSearchAreaCircle.m_x = i_ptSpecBallCenter.m_x;
    ballSearchAreaCircle.m_y = i_ptSpecBallCenter.m_y;
    ballSearchAreaCircle.m_radius = i_fSearchRadius;

    //이미지 버퍼 만들고.. 이진 영상 및 마스크하고 마스크랑 원본하고 &&연산할거.. 마스크는 미리 만들자
    Ipvm::Image8u calcImage;
    if (!getReusableMemory().GetInspByteImage(calcImage))
        return false;
    Ipvm::ImageProcessing::Fill(rtROI, 0, calcImage);
    nImageSizeX = calcImage.GetSizeX();
    ;

    Ipvm::Image8u maskImage;
    if (!getReusableMemory().GetInspByteImage(maskImage))
        return false;
    Ipvm::ImageProcessing::Fill(rtROI, 0, maskImage);
    Ipvm::ImageProcessing::Fill(ballSearchAreaCircle, 255, maskImage);

    Ipvm::Image8u threshImage;
    if (!getReusableMemory().GetInspByteImage(threshImage))
        return false;
    Ipvm::ImageProcessing::Fill(rtROI, 0, threshImage);

    Ipvm::Image32s labelImage;
    if (!getReusableMemory().GetInspLongImage(labelImage))
        return false;
    Ipvm::ImageProcessing::Fill(rtROI, 0, labelImage);

    //원본 영상에 마스크 적용한 영상을 만들고 Max Intensity를 계산한 후 그 값의 90%에 해당하는 값으로 이진화
    Ipvm::ImageProcessing::BitwiseAnd(coaxImage, maskImage, rtROI, calcImage);
    Ipvm::ImageProcessing::GetMax(calcImage, rtROI, bMaxIntensity);
    nThreshold = (long)((float)bMaxIntensity * .9f + .5f);
    CippModules::Binarize(calcImage, threshImage, rtROI, nThreshold, false);
    Ipvm::ImageProcessing::BitwiseOr(threshImage, rtROI, o_imageBallPeakDebugThresh);

    //이진 영상으로 Blob 추출한 후 가장 큰 Blob의 무게 중심을 Rough Center로 한다. Blob이 없으면 Return false
    auto* pBlob = getReusableMemory().GetBlob();
    std::vector<Ipvm::BlobInfo> vecObjBlob(nMaxBlobNum);
    pBlob->DoBlob(threshImage, rtROI, nMaxBlobNum, labelImage, &vecObjBlob[0], nCurBlobNum);
    if (nCurBlobNum == 0)
        return false;

    for (long nBlob = 0; nBlob < nCurBlobNum; nBlob++)
    {
        if (nMaxBlobArea < vecObjBlob[nBlob].m_area)
        {
            nMaxBlobArea = vecObjBlob[nBlob].m_area;
            nMaxBlobLabel = vecObjBlob[nBlob].m_label;
            rtMaxBlobROI = vecObjBlob[nBlob].m_roi;
        }
    }

    for (long nY = rtMaxBlobROI.m_top; nY <= rtMaxBlobROI.m_bottom; nY++)
    {
        auto* label_y = labelImage.GetMem(0, nY);
        for (long nX = rtMaxBlobROI.m_left; nX <= rtMaxBlobROI.m_right; nX++)
        {
            if (label_y[nX] == nMaxBlobLabel)
            {
                nSumCnt++;
                nSumX += nX;
                nSumY += nY;
            }
        }
    }

    o_ptRoughCenter.m_x = (float)((float)nSumX / (float)nSumCnt);
    o_ptRoughCenter.m_y = (float)((float)nSumY / (float)nSumCnt);

    return true;
}

BOOL VisionInspectionBgaBottom2D::GetBallWidthByEdgeDetect(const bool detailSetupMode, const Ipvm::Image8u& image)
{
    float fCurEdgeThresh = m_pEdgeDetect->SetMininumThreshold(10.f); //kircheis_ShinyTest
    m_pEdgeDetect->SetMininumThreshold(fCurEdgeThresh);
    BOOL bFirstEdge = (fCurEdgeThresh == 5.f);

    //{{//kircheis_NantongBO
    long nRoughLineNum(0);
    float fEdgeThreshOld(0.f);
    //}}

    std::vector<Ipvm::Point32r2> vfCirclePointXY;

    std::vector<BYTE> vbyLineValue;
    std::vector<BYTE> vbyLineValue_Revers;

    Ipvm::CircleEq32r CircleCoeff;
    Ipvm::EllipseEq32r roughAlignCircle; //kircheis_MED3

    float fDistX(0.f), fDistY(0.f);
    float fEdgeRadius(0.f);
    float fSpec_Temp(0.f);
    float fRadiusSpec_Min(0.f);
    float fRadiusSpec_Max(0.f);

    BOOL bSubPixelingDone;

    std::vector<float> vecfBallEdgeDist(0); //kircheis_FootTip_BW

    // 영훈 : Ball Missing의 정보를 기억하도록 한다.
    m_missingBalls.clear();
    m_vecrtBallCircleROI.clear();

    float fUMToPixel = getScale().umToPixelXY();

    Ipvm::Rect32r rtfSpecROI;

    CPoint ptEnd;
    float fEdgeValue;

    m_vecrtBallPosition_pixel.resize(m_packageSpec.m_ballMap->m_balls.size());

    //{{//kircheis_BallFlux
    std::vector<Ipvm::Point32r2> vecfptReEdge(0);
    std::vector<Ipvm::Point32r2> vecfptReEdgeFinal(0);
    std::vector<Ipvm::EllipseEq32r> vecEllipseReSearchArea(0);
    //}}

    //{{//kircheis_ImproveBall2D
    Ipvm::Image8u invertCoaxial;
    if (!getReusableMemory().GetInspByteImage(invertCoaxial))
        return FALSE;
    if (m_pVisionPara->m_nImageCombineMode == VisionInspectionBgaBottom2DPara::enumCombineMode_ImprovedDefault)
        Ipvm::ImageProcessing::BitwiseNot(
            m_pVisionPara->m_coaxialImageIndex.getImage(false), Ipvm::Rect32s(m_coaxialImage), invertCoaxial);
    //}}

    //{{//kircheis_LKF
    std::vector<Ipvm::EllipseEq32r> vecEllipseRoughBallPeakArea(0);
    Ipvm::EllipseEq32r ellipseRoughBallPeakArea;
    std::vector<Ipvm::Point32r2> vecfptRoughBallCenter(0);
    Ipvm::Image8u imageRoughBallPeakDebug;
    if (!getReusableMemory().GetInspByteImage(imageRoughBallPeakDebug))
        return FALSE;
    Ipvm::ImageProcessing::Fill((Ipvm::Rect32s)imageRoughBallPeakDebug, 0, imageRoughBallPeakDebug);
    //}}

    for (const auto& ball : m_packageSpec.m_ballMap->m_balls)
    {
        auto& GroupParas = m_pVisionPara->m_parameters[ball.m_typeIndex];

        // 각도에 따른 삼각함수 테이블을 얻어온다.
        float* pfCos = &m_vecfCosForBallCenter[ball.m_typeIndex][0];
        float* pfSin = &m_vecfSinForBallCenter[ball.m_typeIndex][0];

        //{{//kircheis_BallFlux
        long nSectionNumber = GroupParas.m_nBallEdgeCountMode == enumBallEdgeFastMode
            ? SPEC_WIDTH_SWEEP_LINE_COUNT_FAST
            : SPEC_WIDTH_SWEEP_LINE_COUNT_DETAIL;
        const BOOL bIsBallEdgeReSearch = GroupParas.m_nBallEdgeReSearchMode == enumBallEdgeReSearchUse;
        const float fReSearchRatioInner = CAST_FLOAT(GroupParas.m_ballReSearchLengthRatioInner);
        const float fReSearchRatioOuter = CAST_FLOAT(GroupParas.m_ballReSearchLengthRatioOuter);
        //}}

        BOOL bRoughFirstEdge = (GroupParas.m_nBallWidthMethod == enumGetMaxValue);

        float fSpecBallWidth = float(ball.m_diameter_um * fUMToPixel);

        float fBallWidth = fSpecBallWidth;
        float fBallHalfWidth = fBallWidth * 0.5f;

        // 무게중심을 구할 라인 정의
        const float fSweepLength = float(fSpecBallWidth * GroupParas.m_ballWidthRoughSearchLengthRatio);

        // 무게중심을 구할 영역의 볼 센터
        float fInX = m_vecBallSpec_Body[ball.m_index].m_x;
        float fInY = m_vecBallSpec_Body[ball.m_index].m_y;

        if (fInX <= 0 || fInY <= 0)
            return FALSE;

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////
        ////////////////////////////////////러프 센터를 먼저 찾는다/////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////
        CPoint ptStart((long)(fInX + 0.5f), (long)(fInY + 0.5f));
        long nValidEdgeCount(0); //kircheis_LKF

        if (GroupParas.m_nRoughAlignMethod == enumBallRoughAlignMethod_UsePeakCenterOfBallInCoaxialImage) //kircheis_LKF
        {
            Ipvm::Point32r2 fptSpecBall(fInX, fInY);
            Ipvm::Point32r2 fptResult;

            ellipseRoughBallPeakArea.m_x = fInX;
            ellipseRoughBallPeakArea.m_y = fInY;
            ellipseRoughBallPeakArea.m_xradius = ellipseRoughBallPeakArea.m_yradius = fSweepLength;
            vecEllipseRoughBallPeakArea.push_back(ellipseRoughBallPeakArea);

            if (GetBallRoughCenterByCoaxImageBallPeak(detailSetupMode,
                    m_pVisionPara->m_coaxialImageIndex.getImage(false), fptSpecBall, fSweepLength, fptResult,
                    imageRoughBallPeakDebug))
            {
                fInX = fptResult.m_x;
                fInY = fptResult.m_y;
                vecfptRoughBallCenter.push_back(fptResult);
            }
        }
        else
        {
            vfCirclePointXY.clear();
            vfCirclePointXY.resize(nSectionNumber, Ipvm::Point32r2(0.f, 0.f));

            for (long nSection = 0; nSection < nSectionNumber; nSection++)
            {
                const float fOutX = fInX + fSweepLength * pfCos[nSection];
                const float fOutY = fInY + fSweepLength * pfSin[nSection];

                // 스윕 라인 상의 인센서티 샘플링
                long nLineValue
                    = CippModules::GetLineData(image, fOutX, fOutY, fInX, fInY, pfCos[nSection], pfSin[nSection], NULL);
                nRoughLineNum = bRoughFirstEdge ? nLineValue / 2 : nLineValue; //kircheis_NantongBO
                vbyLineValue.clear();
                vbyLineValue.resize(nLineValue);

                if (m_pVisionPara->m_nImageCombineMode
                    == VisionInspectionBgaBottom2DPara::enumCombineMode_ImprovedDefault) //kircheis_ImproveBall2D
                    CippModules::GetLineData(
                        invertCoaxial, fInX, fInY, fOutX, fOutY, pfCos[nSection], pfSin[nSection], &vbyLineValue[0]);
                else
                    CippModules::GetLineData(
                        image, fInX, fInY, fOutX, fOutY, pfCos[nSection], pfSin[nSection], &vbyLineValue[0]);

                vbyLineValue_Revers = vbyLineValue;
                for (long n = 0; n < nLineValue; n++)
                {
                    vbyLineValue_Revers[nLineValue - n - 1] = vbyLineValue[n];
                }
                // 스윕 라인 상의 상승 에지 구하기.
                //{{//kircheis_NantongBO
                float fEdgeT = bRoughFirstEdge ? 3.f : fCurEdgeThresh;
                fEdgeThreshOld = m_pEdgeDetect->SetMininumThreshold(fEdgeT);
                //}}
                m_result->m_rough_vecSweepLine.emplace_back(fInX, fInY, fOutX, fOutY);

                if (!m_pEdgeDetect->PI_ED_CalcEdgePointFromLine(PI_ED_DIR_RISING, nRoughLineNum,
                        &vbyLineValue_Revers[0], fEdgeRadius, fEdgeValue, bSubPixelingDone,
                        bRoughFirstEdge)) //FALSE ))//kircheis_ShinyTest//kircheis_NantongBO
                {
                    m_pEdgeDetect->SetMininumThreshold(fEdgeT * 0.5f);
                    if (!m_pEdgeDetect->PI_ED_CalcEdgePointFromLine(PI_ED_DIR_RISING, nRoughLineNum,
                            &vbyLineValue_Revers[0], fEdgeRadius, fEdgeValue, bSubPixelingDone,
                            bRoughFirstEdge)) //FALSE ))//kircheis_ShinyTest//kircheis_NantongBO
                    {
                        m_pEdgeDetect->SetMininumThreshold(fEdgeThreshOld); //kircheis_NantongBO
                        continue;
                    }
                }
                m_pEdgeDetect->SetMininumThreshold(fEdgeThreshOld); //kircheis_NantongBO

                fEdgeRadius = nLineValue - fEdgeRadius;

                vfCirclePointXY[nSection].m_x = fInX + fEdgeRadius * pfCos[nSection];
                vfCirclePointXY[nSection].m_y = fInY + fEdgeRadius * pfSin[nSection];
                m_result->m_rough_vecfptBallWidthEdgeTotalPoint.push_back(vfCirclePointXY[nSection]);
            }

            // 영훈 20131119 : 0의 값은 버리도록 하자.
            for (long n = (long)vfCirclePointXY.size() - 1; n >= 0; n--)
            {
                if (vfCirclePointXY[n].m_x <= 0.f || vfCirclePointXY[n].m_y <= 0.f)
                {
                    vfCirclePointXY.erase(vfCirclePointXY.begin() + n);
                }
            }

            rtfSpecROI.m_left = fInX - (fSpecBallWidth * 0.5f);
            rtfSpecROI.m_right = fInX + (fSpecBallWidth * 0.5f);
            rtfSpecROI.m_top = fInY - (fSpecBallWidth * 0.5f);
            rtfSpecROI.m_bottom = fInY + (fSpecBallWidth * 0.5f);

            // 영훈 20150210_Ball_Missing : Edge를 하나도 못잡을 경우 invalid처리가 되던 것을 missing으로 처리되도록 수정
            nValidEdgeCount = (long)vfCirclePointXY.size();
            if (nValidEdgeCount < SPEC_BALL_EDGE_COUNT_MIN)
            {
                m_missingBalls.emplace_back(ball.m_index, Ipvm::Conversion::ToRect32s(rtfSpecROI));
                m_vecrtBallCircleROI.push_back(rtfSpecROI);

                continue;
            }

            Ipvm::DataFitting::FitToCircle(nValidEdgeCount, &vfCirclePointXY[0], CircleCoeff);

            //{{//kircheis_MED3
            roughAlignCircle.m_x = CircleCoeff.m_x;
            roughAlignCircle.m_y = CircleCoeff.m_y;
            roughAlignCircle.m_xradius = CircleCoeff.m_radius;
            roughAlignCircle.m_yradius = CircleCoeff.m_radius;
            m_result->m_rough_vecellipseAlignCircle.push_back(roughAlignCircle);
            //}}

            float f100umToPxl = 100.f * fUMToPixel;
            float fMaxAcceptableBallWidthGap = f100umToPxl;

            if (m_pVisionPara->m_nImageCombineMode
                == VisionInspectionBgaBottom2DPara::enumCombineMode_ImprovedDefault) //kircheis_ImproveBall2D
                fMaxAcceptableBallWidthGap = fSpecBallWidth * .5f; //동축 반전 영상에서는 Ball이 훨씬 크게 보일 수 있다.
            fMaxAcceptableBallWidthGap = (float)max(fMaxAcceptableBallWidthGap, 10.f);

            if (fabs(CircleCoeff.m_x - fInX) < f100umToPxl && fabs(CircleCoeff.m_y - fInY) < f100umToPxl
                && fabs((CircleCoeff.m_radius * 2) - fSpecBallWidth) < fMaxAcceptableBallWidthGap)
            {
                fInX = CircleCoeff.m_x;
                fInY = CircleCoeff.m_y;

                vecfptRoughBallCenter.push_back(Ipvm::Point32r2(fInX, fInY)); //kircheis_LKF
            }
        }

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        const long nSweepLineIgnoreDist = (long)(fSpecBallWidth * GroupParas.m_ballWidthSearchIgnoreLengthRatio + 0.5f);

        vfCirclePointXY.clear();
        vfCirclePointXY.resize(nSectionNumber, Ipvm::Point32r2(0.f, 0.f));

        const float fSweepLengthDetail
            = float(fSpecBallWidth * GroupParas.m_ballWidthDetailSearchLengthRatio); //kircheis_BallFlux

        for (long nSection = 0; nSection < nSectionNumber; nSection++)
        {
            // 무게중심을 구할 라인 정의

            const float fInX_Debug = fInX + (float)nSweepLineIgnoreDist * pfCos[nSection];
            const float fInY_Debug = fInY + (float)nSweepLineIgnoreDist * pfSin[nSection];
            const float fOutX = fInX + fSweepLengthDetail * pfCos[nSection]; //kircheis_BallFlux
            const float fOutY = fInY + fSweepLengthDetail * pfSin[nSection]; //kircheis_BallFlux

            // 스윕 라인 상의 인센서티 샘플링
            long nLineValue
                = CippModules::GetLineData(image, fOutX, fOutY, fInX, fInY, pfCos[nSection], pfSin[nSection], NULL);
            vbyLineValue.clear();
            vbyLineValue.resize(nLineValue);
            CippModules::GetLineData(
                image, fInX, fInY, fOutX, fOutY, pfCos[nSection], pfSin[nSection], &vbyLineValue[0]);

            for (long nIgnoreSOP = 0; nIgnoreSOP < nSweepLineIgnoreDist; nIgnoreSOP++)
            {
                vbyLineValue[nIgnoreSOP] = 0; // // 20140616 영훈 : 여긴 어둡게 한다.
            }

            m_result->m_vecSweepLine.emplace_back(fInX_Debug, fInY_Debug, fOutX, fOutY);

            // 스윕 라인 상의 상승 에지 구하기.
            if (!m_pEdgeDetect->PI_ED_CalcEdgePointFromLine(PI_ED_DIR_FALLING, nLineValue, &vbyLineValue[0],
                    fEdgeRadius, fEdgeValue, bSubPixelingDone, bFirstEdge)) //FALSE ))//kircheis_ShinyTest
            {
                float fEdgeReThresh = fCurEdgeThresh * .3f;
                if (fEdgeReThresh < fEdgeValue)
                {
                    fEdgeThreshOld = m_pEdgeDetect->SetMininumThreshold(fEdgeReThresh);
                    if (!m_pEdgeDetect->PI_ED_CalcEdgePointFromLine(PI_ED_DIR_FALLING, nLineValue, &vbyLineValue[0],
                            fEdgeRadius, fEdgeValue, bSubPixelingDone, bFirstEdge)) //FALSE ))//kircheis_ShinyTest
                    {
                        m_pEdgeDetect->SetMininumThreshold(fEdgeThreshOld);
                        continue;
                    }
                    m_pEdgeDetect->SetMininumThreshold(fEdgeThreshOld);
                }
                else
                    continue;
            }

            vfCirclePointXY[nSection].m_x = fInX + fEdgeRadius * pfCos[nSection];
            vfCirclePointXY[nSection].m_y = fInY + fEdgeRadius * pfSin[nSection];
        }

        std::vector<Ipvm::Point32r2> vecfptBallEdgePoint(
            0); //kircheis_20160524 //일종의 치트. 불량이 의심되면 상하좌우의 Max 길이를 구한다.
        // 영훈 20131119 : 0의 값은 버리도록 하자.
        for (long n = (long)vfCirclePointXY.size() - 1; n >= 0; n--)
        {
            vecfptBallEdgePoint.push_back(
                vfCirclePointXY[n]); //kircheis_20160524 //일종의 치트. 불량이 의심되면 상하좌우의 Max 길이를 구한다.

            if (vfCirclePointXY[n].m_x <= 0.f || vfCirclePointXY[n].m_y <= 0.f)
            {
                vfCirclePointXY.erase(vfCirclePointXY.begin() + n);
            }
        }

        if ((long)vfCirclePointXY.size() <= 0)
        {
            m_missingBalls.emplace_back(ball.m_index, Ipvm::Conversion::ToRect32s(rtfSpecROI));
            m_vecrtBallCircleROI.push_back(rtfSpecROI);

            continue;
        }

        nValidEdgeCount = (long)vfCirclePointXY.size();
        Ipvm::DataFitting::FitToCircle(nValidEdgeCount, &vfCirclePointXY[0], CircleCoeff);

        // 구한 원의 원주에 대해서 일정 거리 이상 떨어진 놈들을 없애고 다시 피팅
        long nNewValidEdgeCount = 0;
        for (long nSection = 0; nSection < nValidEdgeCount; nSection++)
        {
            fDistX = vfCirclePointXY[nSection].m_x - CircleCoeff.m_x;
            fDistY = vfCirclePointXY[nSection].m_y - CircleCoeff.m_y;
            fEdgeRadius = CAST_FLOAT(sqrt(CAST_DOUBLE(fDistX * fDistX + fDistY * fDistY)));

            // 영훈 20140616 : 찾은 Ball의 반지름이 입력한 값보다 크게 튀는 Edge는 필터링하여 다시 피팅한다.
            float fNoiseFilter = bIsBallEdgeReSearch ? CAST_FLOAT(GroupParas.m_ballReSearchNoiseFilterThresholdRatio)
                                                     : SPEC_BALL_EDGE_FILTERING;
            fSpec_Temp = fNoiseFilter * CircleCoeff.m_radius;
            fRadiusSpec_Min = CircleCoeff.m_radius - fSpec_Temp;
            fRadiusSpec_Max = CircleCoeff.m_radius + fSpec_Temp;

            if (detailSetupMode)
                m_result->m_vecfptBallWidthEdgeTotalPoint.push_back(vfCirclePointXY[nSection]);

            if (fEdgeRadius > fRadiusSpec_Min && fEdgeRadius < fRadiusSpec_Max)
            {
                vfCirclePointXY[nNewValidEdgeCount] = vfCirclePointXY[nSection];

                if (detailSetupMode)
                    m_result->m_vecfptBallWidthEdgeValidPoint.push_back(vfCirclePointXY[nNewValidEdgeCount]);

                nNewValidEdgeCount++;
            }
        }

        if (nNewValidEdgeCount == 0 || nNewValidEdgeCount < SPEC_BALL_EDGE_COUNT_MIN)
        {
            m_missingBalls.emplace_back(ball.m_index, Ipvm::Conversion::ToRect32s(rtfSpecROI));
            m_vecrtBallCircleROI.push_back(rtfSpecROI);

            continue;
        }

        // 새로 정한 무게중심 포인트를 이용해 원 피팅을 실시.
        Ipvm::DataFitting::FitToCircle(nNewValidEdgeCount, &vfCirclePointXY[0], CircleCoeff);

        if (bIsBallEdgeReSearch)
        {
            std::vector<Ipvm::EllipseEq32r> vecReSearchArea(0);
            std::vector<Ipvm::Point32r2> vecfptTotal(0);
            std::vector<Ipvm::Point32r2> vecfptFinal(0);
            for (long i = 0; i < 2; i++)
            {
                float fReSearchStart = CircleCoeff.m_radius * (1.f - fReSearchRatioInner);
                float fReSearchEnd = CircleCoeff.m_radius * (1.f + fReSearchRatioOuter);
                Ipvm::EllipseEq32r circleReSearch(
                    CircleCoeff.m_x, CircleCoeff.m_y, CircleCoeff.m_radius, CircleCoeff.m_radius);
                vecReSearchArea.push_back(circleReSearch);
                circleReSearch.m_xradius = circleReSearch.m_yradius = fReSearchStart;
                vecReSearchArea.push_back(circleReSearch);
                circleReSearch.m_xradius = circleReSearch.m_yradius = fReSearchEnd;
                vecReSearchArea.push_back(circleReSearch);

                vecfptTotal.clear();

                GetReSearchBallWidthEdgeDetectByOblique(CircleCoeff, CircleCoeff.m_radius * 2.f, fReSearchRatioInner,
                    fReSearchRatioOuter, ball.m_typeIndex, vecfptTotal, vecfptFinal, CircleCoeff);
                if ((float)vecfptFinal.size() / (float)vecfptTotal.size() > 0.6f)
                    break;
            }
            vecfptReEdge.insert(vecfptReEdge.end(), vecfptTotal.begin(), vecfptTotal.end());
            vecfptReEdgeFinal.insert(vecfptReEdgeFinal.end(), vecfptFinal.begin(), vecfptFinal.end());
            vecEllipseReSearchArea.insert(vecEllipseReSearchArea.end(), vecReSearchArea.begin(), vecReSearchArea.end());
        }

        if (detailSetupMode)
            m_result->m_vecfptBallWidthEdgeValidPoint.push_back(Ipvm::Point32r2(CircleCoeff.m_x, CircleCoeff.m_y));

        // 20140616 영훈 : Ball Center는 여기서 넣어준다.
        m_sBallAlignResult->m_ballWidthCenterPos_px[ball.m_index].m_x = CircleCoeff.m_x;
        m_sBallAlignResult->m_ballWidthCenterPos_px[ball.m_index].m_y = CircleCoeff.m_y;

        //{{ //kircheis_20160524 //일종의 치트. 불량이 의심되면 상하좌우의 Max 길이를 구한다.
        float fMinX = 1000000.f;
        float fMinY = 1000000.f;
        float fMaxX = -1000000.f;
        float fMaxY = -1000000.f;
        float fCurDistX, fCurDistY;
        long nMinX_ID(0), nMinY_ID(0), nMaxX_ID(0), nMaxY_ID(0);
        long nLeftID(0), nRightID(0);
        long nIdNum = (long)vecfptBallEdgePoint.size();
        for (long nID = 0; nID < nIdNum; nID++)
        {
            if (vecfptBallEdgePoint[nID].m_x <= 0.f || vecfptBallEdgePoint[nID].m_y <= 0.f)
                continue;

            fCurDistX = vecfptBallEdgePoint[nID].m_x - CircleCoeff.m_x;
            fCurDistY = vecfptBallEdgePoint[nID].m_y - CircleCoeff.m_y;
            if (fMinX > fCurDistX)
            {
                fMinX = fCurDistX;
                nMinX_ID = nID;
            }
            if (fMinY > fCurDistY)
            {
                fMinY = fCurDistY;
                nMinY_ID = nID;
            }
            if (fMaxX < fCurDistX)
            {
                fMaxX = fCurDistX;
                nMaxX_ID = nID;
            }
            if (fMaxY < fCurDistY)
            {
                fMaxY = fCurDistY;
                nMaxY_ID = nID;
            }
        }

        //Top
        float fTopResult = 0.f;
        nLeftID = nMinY_ID - 1;
        nLeftID = nLeftID < 0 ? nIdNum - 1 : nLeftID;
        nRightID = nMinY_ID + 1;
        nRightID = nRightID >= nIdNum ? 0 : nRightID;
        float fLeft = (float)fabs(fMinY - (vecfptBallEdgePoint[nLeftID].m_y - CircleCoeff.m_y));
        float fRight = (float)fabs(fMinY - (vecfptBallEdgePoint[nRightID].m_y - CircleCoeff.m_y));
        if (fLeft < 2.f || fRight < 2.f)
            fTopResult = fMinY;

        //Btm
        float fBtmResult = 0.f;
        nLeftID = nMaxY_ID - 1;
        nLeftID = nLeftID < 0 ? nIdNum - 1 : nLeftID;
        nRightID = nMaxY_ID + 1;
        nRightID = nRightID >= nIdNum ? 0 : nRightID;
        fLeft = (float)fabs(fMaxY - (vecfptBallEdgePoint[nLeftID].m_y - CircleCoeff.m_y));
        fRight = (float)fabs(fMaxY - (vecfptBallEdgePoint[nRightID].m_y - CircleCoeff.m_y));
        if (fLeft < 2.f || fRight < 2.f)
            fBtmResult = fMaxY;

        //Left
        float fLeftResult = 0.f;
        nLeftID = nMinX_ID - 1;
        nLeftID = nLeftID < 0 ? nIdNum - 1 : nLeftID;
        nRightID = nMinX_ID + 1;
        nRightID = nRightID >= nIdNum ? 0 : nRightID;
        fLeft = (float)fabs(fMinX - (vecfptBallEdgePoint[nLeftID].m_x - CircleCoeff.m_x));
        fRight = (float)fabs(fMinX - (vecfptBallEdgePoint[nRightID].m_x - CircleCoeff.m_x));
        if (fLeft < 2.f || fRight < 2.f) //kircheis_20160525
        {
            fLeftResult = fMinX;
            if (fLeft < 0.5f || fRight < 0.5f)
                fLeftResult = (float)min(fLeft, fRight) + fMinX;
        }

        //Right
        float fRightResult = 0.f;
        nLeftID = nMaxX_ID - 1;
        nLeftID = nLeftID < 0 ? nIdNum - 1 : nLeftID;
        nRightID = nMaxX_ID + 1;
        nRightID = nRightID >= nIdNum ? 0 : nRightID;
        fLeft = (float)fabs(fMaxX - (vecfptBallEdgePoint[nLeftID].m_x - CircleCoeff.m_x));
        fRight = (float)fabs(fMaxX - (vecfptBallEdgePoint[nRightID].m_x - CircleCoeff.m_x));
        if (fLeft < 2.f || fRight < 2.f)
        {
            fRightResult = fMaxX;
            if (fLeft < 0.5f || fRight < 0.5f) //kircheis_20160525
                fRightResult = fMaxX - (float)min(fLeft, fRight);
        }

        //float fResultX = -fLeftResult + fRightResult;
        //float fResultY = -fTopResult + fBtmResult;

        //}}

        //{{ //kircheis_FootTip_BW 개선 각 Edge Point에서 가장 먼 Point간의 거리를 누적한 다음 상위 3개의 거리를 평균 내서 쓰자
        static BOOL bTest = TRUE;
        if (nIdNum > 10 && bTest)
        {
            float fMaxDist = 0.f;
            long nLengthNum = nIdNum / 2;
            vecfBallEdgeDist.resize(nLengthNum);
            memset(&vecfBallEdgeDist[0], 0, nLengthNum * sizeof(float));
            for (long nID = 0; nID < nLengthNum; nID++)
            {
                long nDestID = nID + nLengthNum;
                auto& cur = vecfptBallEdgePoint[nID];
                auto& dst = vecfptBallEdgePoint[nDestID];

                if (cur.m_x > 0.f && cur.m_y > 0.f && dst.m_x > 0.f && dst.m_y > 0.f)
                {
                    float distance = 0.f;
                    Ipvm::Geometry::GetDistance(cur, dst, distance);

                    vecfBallEdgeDist[nID] = distance;
                    fMaxDist = (float)max(fMaxDist, distance);
                }
            }
            vecfBallEdgeDist.clear();
        }
        //}}
        Ipvm::Rect32r frtBallWidthROI;
        frtBallWidthROI.m_left = CircleCoeff.m_x - CircleCoeff.m_radius;
        frtBallWidthROI.m_right = CircleCoeff.m_x + CircleCoeff.m_radius;
        frtBallWidthROI.m_top = CircleCoeff.m_y - CircleCoeff.m_radius;
        frtBallWidthROI.m_bottom = CircleCoeff.m_y + CircleCoeff.m_radius;

        m_vecrtBallPosition_pixel[ball.m_index] = Ipvm::Conversion::ToRect32s(frtBallWidthROI);
        m_vecrtBallCircleROI.push_back(frtBallWidthROI);

        const float ballDiameter_px = 2 * CircleCoeff.m_radius;

        m_sBallAlignResult->m_ballWidths_px[ball.m_index] = ballDiameter_px;

        if ((fBallWidth + fBallWidth) <= ballDiameter_px || fBallHalfWidth >= ballDiameter_px)
        {
            m_missingBalls.emplace_back(ball.m_index, Ipvm::Conversion::ToRect32s(rtfSpecROI));
            continue;
        }
    }

    if (vecfptReEdge.size() > 0) //kircheis_BallFlux
    {
        SetDebugInfoItem(detailSetupMode, _T("Ball Width Re-Search Area"), vecEllipseReSearchArea);
        SetDebugInfoItem(detailSetupMode, _T("Ball Width Re-Edge Total Point"), vecfptReEdge);
        SetDebugInfoItem(detailSetupMode, _T("Ball Width Re-Edge Valid Point"), vecfptReEdgeFinal);
    }

    if (vecEllipseRoughBallPeakArea.size() > 0) //kircheis_LKF
    {
        SetDebugInfoItem(detailSetupMode, _T("Ball: Rough Ball Peak Search Circle"), vecEllipseRoughBallPeakArea);
        SetDebugInfoItem(detailSetupMode, _T("Ball: Rough Ball Peak Threshold Image"), imageRoughBallPeakDebug);
    }
    if (vecfptRoughBallCenter.size() > 0) //kircheis_LKF
        SetDebugInfoItem(detailSetupMode, _T("Ball: Rough Ball Peak Center"), vecfptRoughBallCenter);

    return TRUE;
}

BOOL VisionInspectionBgaBottom2D::GetReSearchBallWidthEdgeDetectByOblique(Ipvm::CircleEq32r i_circleEq,
    float i_fSecBallWidth, float i_fReSearchRatioInner, float i_fReSearchRatioOuter, long i_nBallType,
    std::vector<Ipvm::Point32r2>& o_vecTotalEdgePoint, std::vector<Ipvm::Point32r2>& o_vecFinalEdgePoint,
    Ipvm::CircleEq32r& o_circleEq)
{
    o_vecTotalEdgePoint.clear();
    o_vecFinalEdgePoint.clear();
    if (m_pVisionPara->m_nImageCombineMode != VisionInspectionBgaBottom2DPara::enumCombineMode_Default
        && m_pVisionPara->m_nImageCombineMode
            != VisionInspectionBgaBottom2DPara::enumCombineMode_ImprovedDefault //kircheis_ImproveBall2D
        && m_pVisionPara->m_nImageCombineMode != VisionInspectionBgaBottom2DPara::enumCombineMode_Oblique)
        return false;

    auto& GroupParas = m_pVisionPara->m_parameters[i_nBallType]; //kircheis_BallFluxModify
    long nSectionNumber = m_pVisionPara->m_parameters[i_nBallType].m_nBallEdgeCountMode == enumBallEdgeFastMode
        ? SPEC_WIDTH_SWEEP_LINE_COUNT_FAST
        : SPEC_WIDTH_SWEEP_LINE_COUNT_DETAIL; //kircheis_BallFlux

    float* pfCos = &m_vecfCosForBallCenter[i_nBallType][0];
    float* pfSin = &m_vecfSinForBallCenter[i_nBallType][0];
    const long nSweepLineIgnoreDist = (long)(i_fSecBallWidth * 0.25f + 0.5f);
    const float fSweepLength
        = float((float)max(i_circleEq.m_radius, i_fSecBallWidth) * (1.f + i_fReSearchRatioOuter) + 2.f);
    std::vector<BYTE> vbyLineValue;
    float fEdgeValue, fEdgeRadius;
    BOOL bSubPixelingDone = true;

    float fInX = i_circleEq.m_x;
    float fInY = i_circleEq.m_y;

    std::vector<Ipvm::Point32r2> vfCirclePointXY(0);
    vfCirclePointXY.resize(nSectionNumber, Ipvm::Point32r2(0.f, 0.f));

    static float fEdgeThresh = 3.f;
    float fOldEdgeThresh = m_pEdgeDetect->SetMininumThreshold(fEdgeThresh);

    long nStart = (long)(i_circleEq.m_radius * (1.f - i_fReSearchRatioInner));
    long nEnd = (long)(i_circleEq.m_radius * (1.f + i_fReSearchRatioOuter));
    long nLength = nEnd - nStart;

    Ipvm::Image8u obliqueImage; //kircheis_TMI
    if (!getReusableMemory().GetInspByteImage(obliqueImage))
        return FALSE;
    obliqueImage = m_pVisionPara->m_obliqueImageIndex.getImage(false);

    for (long nSection = 0; nSection < nSectionNumber; nSection++)
    {
        const float fOutX = fInX + fSweepLength * pfCos[nSection];
        const float fOutY = fInY + fSweepLength * pfSin[nSection];

        // 스윕 라인 상의 인센서티 샘플링
        long nLineValue
            = CippModules::GetLineData(obliqueImage, fOutX, fOutY, fInX, fInY, pfCos[nSection], pfSin[nSection], NULL);
        vbyLineValue.clear();
        vbyLineValue.resize(nLineValue);
        CippModules::GetLineData(
            obliqueImage, fInX, fInY, fOutX, fOutY, pfCos[nSection], pfSin[nSection], &vbyLineValue[0]);

        for (long nIgnoreSOP = 0; nIgnoreSOP < nSweepLineIgnoreDist; nIgnoreSOP++)
        {
            vbyLineValue[nIgnoreSOP] = 0; // // 20140616 영훈 : 여긴 어둡게 한다.
        }

        // 스윕 라인 상의 하강 에지 구하기.
        if (!m_pEdgeDetect->PI_ED_CalcEdgePointFromLine(PI_ED_DIR_FALLING, nLength, &vbyLineValue[nStart], fEdgeRadius,
                fEdgeValue, bSubPixelingDone, true)) //FALSE ))//kircheis_ShinyTest
        {
            continue;
        }

        vfCirclePointXY[nSection].m_x = fInX + ((float)nStart + fEdgeRadius) * pfCos[nSection];
        vfCirclePointXY[nSection].m_y = fInY + ((float)nStart + fEdgeRadius) * pfSin[nSection];
        o_vecTotalEdgePoint.push_back(vfCirclePointXY[nSection]);
    }
    m_pEdgeDetect->SetMininumThreshold(fOldEdgeThresh);
    if ((float)o_vecTotalEdgePoint.size() / (float)nSectionNumber < 0.4f)
        return false;

    Ipvm::DataFitting::FitToCircle((long)o_vecTotalEdgePoint.size(), &o_vecTotalEdgePoint[0], o_circleEq);

    std::vector<float> vecfptSort(0);
    std::vector<std::vector<float>> vecfptGroupSort(0);
    std::vector<float> vecfptBuf(0);
    std::vector<float> vecDist(0);
    long nSize = (long)o_vecTotalEdgePoint.size();
    float fDistX, fDistY;
    //각 Point의 원점과의 거리 계산 후 저장
    for (long n = 0; n < nSize; n++)
    {
        fDistX = o_vecTotalEdgePoint[n].m_x - o_circleEq.m_x;
        fDistY = o_vecTotalEdgePoint[n].m_y - o_circleEq.m_y;
        fEdgeRadius = CAST_FLOAT(sqrt(CAST_DOUBLE(fDistX * fDistX + fDistY * fDistY)));
        vecfptSort.push_back(fEdgeRadius);
        vecDist.push_back(fEdgeRadius);
    }

    vecfptBuf.push_back(vecfptSort[0]);
    float fSpec_Temp
        = (float)GroupParas.m_ballReSearchNoiseFilterThresholdRatio * o_circleEq.m_radius; //kircheis_BallFluxModify
    float fSpec_Half = fSpec_Temp * .5f;

    float fCut = fSpec_Temp;
    for (long n = 1; n < nSize; n++)
    {
        if ((float)fabs(vecfptSort[n] - vecfptSort[n - 1]) > fCut)
        {
            vecfptGroupSort.push_back(vecfptBuf);
            vecfptBuf.clear();
            vecfptBuf.push_back(vecfptSort[n]);
        }
        else
            vecfptBuf.push_back(vecfptSort[n]);
    }
    vecfptGroupSort.push_back(vecfptBuf);

    long nGroupNum = (long)vecfptGroupSort.size();
    if (nGroupNum > 1)
    {
        float fGap = (float)fabs(vecDist[0] - vecDist[nSize - 1]);
        if (fGap < fCut)
        {
            vecfptGroupSort[0].insert(
                vecfptGroupSort[0].end(), vecfptGroupSort[nGroupNum - 1].begin(), vecfptGroupSort[nGroupNum - 1].end());
            vecfptGroupSort.resize(nGroupNum - 1);
        }
    }

    long nReGroupNum = (long)vecfptGroupSort.size();
    if (nReGroupNum == 1)
    {
        o_vecFinalEdgePoint.insert(o_vecFinalEdgePoint.end(), o_vecTotalEdgePoint.begin(), o_vecTotalEdgePoint.end());
        return true;
    }

    long nMaxSize(0), nMaxID(0), nCurSize(0);
    std::vector<float> vecfAvr(0);
    float fMaxGroupAvr;
    for (long nGroup = 0; nGroup < nReGroupNum; nGroup++)
    {
        nCurSize = (long)vecfptGroupSort[nGroup].size();
        vecfAvr.push_back(GetVecterAvr(vecfptGroupSort[nGroup]));
        if (nMaxSize < nCurSize)
        {
            nMaxSize = nCurSize;
            nMaxID = nGroup;
            fMaxGroupAvr = vecfAvr[nGroup];
        }
    }

    std::vector<float> vecfValidDist(0);
    vecfValidDist.insert(vecfValidDist.end(), vecfptGroupSort[nMaxID].begin(), vecfptGroupSort[nMaxID].end());
    for (long nGroup = 0; nGroup < nReGroupNum; nGroup++)
    {
        if (nGroup == nMaxID)
            continue;

        if ((float)fabs(vecfAvr[nGroup] - vecfAvr[nMaxID]) < fSpec_Half)
            vecfValidDist.insert(vecfValidDist.end(), vecfptGroupSort[nGroup].begin(), vecfptGroupSort[nGroup].end());
    }

    std::vector<float> vecfValidDist2(0);
    for (long nGroup = 0; nGroup < nReGroupNum; nGroup++)
    {
        if ((float)fabs(vecfAvr[nGroup] - i_circleEq.m_radius) < fSpec_Half)
            vecfValidDist2.insert(vecfValidDist2.end(), vecfptGroupSort[nGroup].begin(), vecfptGroupSort[nGroup].end());
    }

    if (vecfValidDist.size() < o_vecTotalEdgePoint.size() / 2 && vecfValidDist2.size() > vecfValidDist.size())
    {
        vecfValidDist.clear();
        vecfValidDist.insert(vecfValidDist.end(), vecfValidDist2.begin(), vecfValidDist2.end());
    }
    long nValidDataNum = (long)vecfValidDist.size();
    for (long n = 0; n < nValidDataNum; n++)
    {
        for (long i = 0; i < nSize; i++)
        {
            if (vecfValidDist[n] == vecDist[i])
            {
                o_vecFinalEdgePoint.push_back(o_vecTotalEdgePoint[i]);
                vecDist[i] = 0;
                break;
            }
        }
    }

    Ipvm::DataFitting::FitToCircle((long)o_vecFinalEdgePoint.size(), &o_vecFinalEdgePoint[0], o_circleEq);

    return true;
}

float VisionInspectionBgaBottom2D::GetVecterAvr(std::vector<float> vecfData)
{
    float fAvr = 0.f;
    long nSize = (long)vecfData.size();
    if (nSize == 0)
        return fAvr;

    for (long n = 0; n < nSize; n++)
        fAvr += vecfData[n];

    fAvr /= (float)nSize;
    return fAvr;
}

BOOL VisionInspectionBgaBottom2D::GetBallWidthByEdgeDetect(
    const bool detailSetupMode, const Ipvm::Image8u& i_ballImage, const Ipvm::Image8u& i_combineImage)
{
    if (i_ballImage.GetMem() == NULL)
    {
        return GetBallWidthByEdgeDetect(detailSetupMode, i_combineImage);
    }

    float fCurEdgeThresh = m_pEdgeDetect->SetMininumThreshold(10.f);

    std::vector<Ipvm::Point32r2> vfCirclePointXY;

    std::vector<BYTE> vbyLineValue;
    std::vector<BYTE> vbyLineValue_Revers;

    Ipvm::CircleEq32r CircleCoeff;

    float fEdgeValue;
    BOOL bSubPixelingDone;

    std::vector<float> vecfBallEdgeDist(0); //kircheis_FootTip_BW

    // 영훈 : Ball Missing의 정보를 기억하도록 한다.
    m_missingBalls.clear();
    m_vecrtBallCircleROI.clear();

    float fUMToPixel = getScale().umToPixelXY();

    Ipvm::Rect32r rtfSpecROI;

    m_vecrtBallPosition_pixel.resize(m_packageSpec.m_ballMap->m_balls.size());

    for (const auto& ball : m_packageSpec.m_ballMap->m_balls)
    {
        auto& GroupParas = m_pVisionPara->m_parameters[ball.m_typeIndex];

        // 각도에 따른 삼각함수 테이블을 얻어온다.
        float* pfCos = &m_vecfCosForBallCenter[ball.m_typeIndex][0];
        float* pfSin = &m_vecfSinForBallCenter[ball.m_typeIndex][0];

        long nSectionNumber = GroupParas.m_nBallEdgeCountMode == enumBallEdgeFastMode
            ? SPEC_WIDTH_SWEEP_LINE_COUNT_FAST
            : SPEC_WIDTH_SWEEP_LINE_COUNT_DETAIL; //kircheis_BallFlux

        long nSectionNumberHalf = nSectionNumber / 2; //kircheis_ShinyNoise

        float fSpecBallWidth = float(ball.m_diameter_um * fUMToPixel);

        float fBallWidth = fSpecBallWidth;
        float fBallHalfWidth = fBallWidth * 0.5f;

        float fBallEdgeOffset = fSpecBallWidth / 20.f;

        float fBallEdgeThreshold = fSpecBallWidth * 0.08f; //kircheis_ShinyNoise

        // 무게중심을 구할 영역의 볼 센터
        float fInX = m_vecBallSpec_Body[ball.m_index].m_x;
        float fInY = m_vecBallSpec_Body[ball.m_index].m_y;

        if (fInX <= 0 || fInY <= 0)
            return FALSE;

        // 무게중심을 구할 라인 정의
        const float fSweepLength = float(fSpecBallWidth * GroupParas.m_ballWidthRoughSearchLengthRatio);

        vfCirclePointXY.clear();
        vfCirclePointXY.resize(nSectionNumber, Ipvm::Point32r2(0.f, 0.f));

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////
        ////////////////////////////////////러프 센터를 먼저 찾는다/////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////
        fCurEdgeThresh = m_pEdgeDetect->SetMininumThreshold(10.f);
        for (long nSection = 0; nSection < nSectionNumber; nSection++)
        {
            const float fOutX = fInX + fSweepLength * pfCos[nSection];
            const float fOutY = fInY + fSweepLength * pfSin[nSection];

            // 스윕 라인 상의 인센서티 샘플링
            long nLineValue = CippModules::GetLineData(
                i_combineImage, fOutX, fOutY, fInX, fInY, pfCos[nSection], pfSin[nSection], NULL);
            vbyLineValue.clear();
            vbyLineValue.resize(nLineValue);
            CippModules::GetLineData(
                i_combineImage, fInX, fInY, fOutX, fOutY, pfCos[nSection], pfSin[nSection], &vbyLineValue[0]);

            vbyLineValue_Revers = vbyLineValue;
            for (long n = 0; n < nLineValue; n++)
            {
                vbyLineValue_Revers[nLineValue - n - 1] = vbyLineValue[n];
            }

            float fEdgeRadius = 0.f;
            m_result->m_rough_vecSweepLine.emplace_back(fInX, fInY, fOutX, fOutY);

            // 스윕 라인 상의 상승 에지 구하기.
            if (!m_pEdgeDetect->PI_ED_CalcEdgePointFromLine(PI_ED_DIR_RISING, nLineValue, &vbyLineValue_Revers[0],
                    fEdgeRadius, fEdgeValue, bSubPixelingDone, FALSE))
                continue;

            fEdgeRadius = nLineValue - fEdgeRadius;

            vfCirclePointXY[nSection].m_x = fInX + fEdgeRadius * pfCos[nSection];
            vfCirclePointXY[nSection].m_y = fInY + fEdgeRadius * pfSin[nSection];

            m_result->m_rough_vecfptBallWidthEdgeTotalPoint.push_back(vfCirclePointXY[nSection]);
        }

        // 영훈 20131119 : 0의 값은 버리도록 하자.
        for (long n = (long)vfCirclePointXY.size() - 1; n >= 0; n--)
        {
            if (vfCirclePointXY[n].m_x <= 0.f || vfCirclePointXY[n].m_y <= 0.f)
            {
                vfCirclePointXY.erase(vfCirclePointXY.begin() + n);
            }
        }

        rtfSpecROI.m_left = fInX - (fSpecBallWidth * 0.5f);
        rtfSpecROI.m_right = fInX + (fSpecBallWidth * 0.5f);
        rtfSpecROI.m_top = fInY - (fSpecBallWidth * 0.5f);
        rtfSpecROI.m_bottom = fInY + (fSpecBallWidth * 0.5f);

        // 영훈 20150210_Ball_Missing : Edge를 하나도 못잡을 경우 invalid처리가 되던 것을 missing으로 처리되도록 수정
        long nValidEdgeCount = (long)vfCirclePointXY.size();
        if (nValidEdgeCount <= 2)
        {
            m_missingBalls.emplace_back(ball.m_index, Ipvm::Conversion::ToRect32s(rtfSpecROI));
            m_vecrtBallCircleROI.push_back(rtfSpecROI);

            continue;
        }

        Ipvm::DataFitting::FitToCircle(nValidEdgeCount, &vfCirclePointXY[0], CircleCoeff);

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        if (fabs(CircleCoeff.m_x - fInX) < 10.f && fabs(CircleCoeff.m_y - fInY) < 10.f
            && fabs((CircleCoeff.m_radius * 2) - fSpecBallWidth) < 10)
        {
            fInX = CircleCoeff.m_x;
            fInY = CircleCoeff.m_y;
        }

        const long nSweepLineIgnoreDist = (long)(fSpecBallWidth * 0.25f + 0.5f);

        vfCirclePointXY.clear();
        vfCirclePointXY.resize(nSectionNumber, Ipvm::Point32r2(0.f, 0.f));

        //{{//kircheis_ShinyNoise
        std::vector<float> vecfEdgeRadius(nSectionNumber);
        memset(&vecfEdgeRadius[0], 0, nSectionNumber * sizeof(float));
        //}}

        const float fSweepLengthDetail
            = float(fSpecBallWidth * GroupParas.m_ballWidthDetailSearchLengthRatio); //kircheis_BallFlux

        for (long nSection = 0; nSection < nSectionNumber; nSection++)
        {
            const float fOutX = fInX + fSweepLengthDetail * pfCos[nSection]; //kircheis_BallFlux
            const float fOutY = fInY + fSweepLengthDetail * pfSin[nSection]; //kircheis_BallFlux

            // 스윕 라인 상의 인센서티 샘플링
            long nLineValue = CippModules::GetLineData(
                i_ballImage, fOutX, fOutY, fInX, fInY, pfCos[nSection], pfSin[nSection], NULL);
            vbyLineValue.clear();
            vbyLineValue.resize(nLineValue);
            CippModules::GetLineData(
                i_ballImage, fInX, fInY, fOutX, fOutY, pfCos[nSection], pfSin[nSection], &vbyLineValue[0]);

            for (long nIgnoreSOP = 0; nIgnoreSOP < nSweepLineIgnoreDist; nIgnoreSOP++)
            {
                vbyLineValue[nIgnoreSOP] = 0; // // 20140616 영훈 : 여긴 어둡게 한다.
            }

            m_result->m_vecSweepLine.emplace_back(fInX, fInY, fOutX, fOutY);

            float fEdgeRadiusInner = 0.f;

            fCurEdgeThresh = m_pEdgeDetect->SetMininumThreshold(EDGE_THRESH_INVERT);
            if (!m_pEdgeDetect->PI_ED_CalcEdgePointFromLine(PI_ED_DIR_FALLING, nLineValue, &vbyLineValue[0],
                    fEdgeRadiusInner, fEdgeValue, bSubPixelingDone, FALSE))
                continue;
            m_pEdgeDetect->SetMininumThreshold(EDGE_THRESH_INVERT);

            // 스윕 라인 상의 인센서티 샘플링
            vbyLineValue.clear();
            vbyLineValue.resize(nLineValue);
            CippModules::GetLineData(
                i_ballImage, fInX, fInY, fOutX, fOutY, pfCos[nSection], pfSin[nSection], &vbyLineValue[0]);

            for (long nIgnoreSOP = 0; nIgnoreSOP < nSweepLineIgnoreDist; nIgnoreSOP++)
            {
                vbyLineValue[nIgnoreSOP] = 0; // // 20140616 영훈 : 여긴 어둡게 한다.
            }

            //{{

            BYTE* pbLineValue = new BYTE[nLineValue];
            for (long idx = 0; idx < nLineValue; idx++)
                pbLineValue[idx] = vbyLineValue[nLineValue - idx - 1];

            long nLineLimit = (long)(nLineValue - fEdgeRadiusInner) + 1; //한 픽셀만 더보자.

            float fEdgeRadius = 0.f;

            if (!m_pEdgeDetect->PI_ED_CalcEdgePointFromLine(PI_ED_DIR_RISING, nLineLimit, pbLineValue, fEdgeRadius,
                    fEdgeValue, bSubPixelingDone, TRUE)) //FALSE ))//kircheis_ShinyTest
            {
                delete[] pbLineValue;
                continue;
            }
            delete[] pbLineValue;

            fEdgeRadius = (float)nLineValue - fEdgeRadius
                - fBallEdgeOffset; //이 함수 중 유일한 꼼수.  - fBallEdgeOffset 로 실제 Ball 보다 약간 크게 잡는거를 보상한다.
            vecfEdgeRadius[nSection] = fEdgeRadius; //kircheis_ShinyNoise

            vfCirclePointXY[nSection].m_x = fInX + fEdgeRadius * pfCos[nSection];
            vfCirclePointXY[nSection].m_y = fInY + fEdgeRadius * pfSin[nSection];
        }

        std::vector<Ipvm::Point32r2> vecfptBallEdgePoint(
            0); //kircheis_20160524 //일종의 치트. 불량이 의심되면 상하좌우의 Max 길이를 구한다.

        //{{//kircheis_ShinyNoise
        BOOL bTryIgnoreNoise = TRUE;
        while (bTryIgnoreNoise)
        {
            std::vector<Ipvm::Point32r2> vecfEdgeData(0);

            Ipvm::LineEq32r line(0.f, 0.f, 0.f);
            float fDist = 0.f;
            long nZeroPoint = 0;
            long nNoisePoint = 0;

            //정상적으로 Edge를 찾은 Point 만으로 Ipvm::Point32r2 Array를 구성하고
            for (long nIdx = 0; nIdx < nSectionNumber; nIdx++)
            {
                if (vecfEdgeRadius[nIdx] > 0.f)
                    vecfEdgeData.push_back(Ipvm::Point32r2((float)nIdx, vecfEdgeRadius[nIdx]));
                else
                    nZeroPoint++;
            }

            if (vecfEdgeData.size() == 0)
            {
                bTryIgnoreNoise = FALSE;
                continue;
            }

            //그걸로 라인 피팅을 한다.
            Ipvm::DataFitting::FitToLine(long(vecfEdgeData.size()), &vecfEdgeData[0], line);

            //라인과 각 Point의 거리를 계산하여 그 거리가 fBallEdgeThreshold보다 크면 그 Edge는 날린다. fBallEdgeThreshold는 Ball Width의 8%로 정의
            for (long nIdx = 0; nIdx < nSectionNumber; nIdx++)
            {
                if (vecfEdgeRadius[nIdx] > 0.f)
                {
                    fDist = CPI_Geometry::GetDistance_PointToLine((float)nIdx, vecfEdgeRadius[nIdx], line);
                    if (fDist > fBallEdgeThreshold)
                    {
                        vecfEdgeRadius[nIdx] = 0.f;
                        nNoisePoint++;
                    }
                }
            }

            if ((nZeroPoint + nNoisePoint) > nSectionNumberHalf || nNoisePoint == 0)
            {
                //남은 Point가 찾아야하는 Point보다 절반 이하이거나 찾은 Noise Point가 없으면 그만
                bTryIgnoreNoise = FALSE;
                continue;
            }

            //남은 Point가 찾을 Point보다 절반 이상이고 찾은 Noise Point가 있으면 해당 Noise Point의 Data를 날려버려~~

            for (long nIdx = 0; nIdx < nSectionNumber; nIdx++)
            {
                if (vecfEdgeRadius[nIdx] <= 0.f && vfCirclePointXY[nIdx].m_x != 0.f)
                {
                    vfCirclePointXY[nIdx] = Ipvm::Point32r2(0.f, 0.f);
                }
            }
        }
        //}}

        // 영훈 20131119 : 0의 값은 버리도록 하자.
        for (long n = (long)vfCirclePointXY.size() - 1; n >= 0; n--)
        {
            vecfptBallEdgePoint.push_back(
                vfCirclePointXY[n]); //kircheis_20160524 //일종의 치트. 불량이 의심되면 상하좌우의 Max 길이를 구한다.

            if (vfCirclePointXY[n].m_x <= 0.f || vfCirclePointXY[n].m_y <= 0.f)
            {
                vfCirclePointXY.erase(vfCirclePointXY.begin() + n);
            }
        }

        if ((long)vfCirclePointXY.size() <= 0)
        {
            m_missingBalls.emplace_back(ball.m_index, Ipvm::Conversion::ToRect32s(rtfSpecROI));
            m_vecrtBallCircleROI.push_back(rtfSpecROI);

            continue;
        }

        nValidEdgeCount = (long)vfCirclePointXY.size();
        Ipvm::DataFitting::FitToCircle(nValidEdgeCount, &vfCirclePointXY[0], CircleCoeff);

        // 구한 원의 원주에 대해서 일정 거리 이상 떨어진 놈들을 없애고 다시 피팅
        long nNewValidEdgeCount = 0;
        for (long nSection = 0; nSection < nValidEdgeCount; nSection++)
        {
            const float fDistX = vfCirclePointXY[nSection].m_x - CircleCoeff.m_x;
            const float fDistY = vfCirclePointXY[nSection].m_y - CircleCoeff.m_y;
            const float fEdgeRadius = CAST_FLOAT(sqrt(CAST_DOUBLE(fDistX * fDistX + fDistY * fDistY)));

            // 영훈 20140616 : 찾은 Ball의 반지름이 입력한 값보다 크게 튀는 Edge는 필터링하여 다시 피팅한다.
            const float fSpec_Temp = SPEC_BALL_EDGE_FILTERING * CircleCoeff.m_radius;
            const float fRadiusSpec_Min = CircleCoeff.m_radius - fSpec_Temp;
            const float fRadiusSpec_Max = CircleCoeff.m_radius + fSpec_Temp;

            m_result->m_vecfptBallWidthEdgeTotalPoint.push_back(vfCirclePointXY[nSection]);

            if (fEdgeRadius > fRadiusSpec_Min && fEdgeRadius < fRadiusSpec_Max)
            {
                vfCirclePointXY[nNewValidEdgeCount] = vfCirclePointXY[nSection];

                m_result->m_vecfptBallWidthEdgeValidPoint.push_back(vfCirclePointXY[nNewValidEdgeCount]);
                nNewValidEdgeCount++;
            }
        }

        if (nNewValidEdgeCount == 0 || nNewValidEdgeCount < SPEC_BALL_EDGE_COUNT_MIN)
        {
            m_missingBalls.emplace_back(ball.m_index, Ipvm::Conversion::ToRect32s(rtfSpecROI));
            m_vecrtBallCircleROI.push_back(rtfSpecROI);

            continue;
        }

        // 새로 정한 무게중심 포인트를 이용해 원 피팅을 실시.
        Ipvm::DataFitting::FitToCircle(nNewValidEdgeCount, &vfCirclePointXY[0], CircleCoeff);
        m_result->m_vecfptBallWidthEdgeValidPoint.push_back(Ipvm::Point32r2(CircleCoeff.m_x, CircleCoeff.m_y));

        // 20140616 영훈 : Ball Center는 여기서 넣어준다.
        m_sBallAlignResult->m_ballWidthCenterPos_px[ball.m_index].m_x = CircleCoeff.m_x;
        m_sBallAlignResult->m_ballWidthCenterPos_px[ball.m_index].m_y = CircleCoeff.m_y;

        //{{ //kircheis_20160524 //일종의 치트. 불량이 의심되면 상하좌우의 Max 길이를 구한다.
        float fMinX = 1000000.f;
        float fMinY = 1000000.f;
        float fMaxX = -1000000.f;
        float fMaxY = -1000000.f;
        long nMinX_ID(0), nMinY_ID(0), nMaxX_ID(0), nMaxY_ID(0);
        long nLeftID(0), nRightID(0);
        long nIdNum = (long)vecfptBallEdgePoint.size();
        for (long nID = 0; nID < nIdNum; nID++)
        {
            if (vecfptBallEdgePoint[nID].m_x <= 0.f || vecfptBallEdgePoint[nID].m_y <= 0.f)
                continue;

            const float fDistX = vecfptBallEdgePoint[nID].m_x - CircleCoeff.m_x;
            const float fDistY = vecfptBallEdgePoint[nID].m_y - CircleCoeff.m_y;
            if (fMinX > fDistX)
            {
                fMinX = fDistX;
                nMinX_ID = nID;
            }
            if (fMinY > fDistY)
            {
                fMinY = fDistY;
                nMinY_ID = nID;
            }
            if (fMaxX < fDistX)
            {
                fMaxX = fDistX;
                nMaxX_ID = nID;
            }
            if (fMaxY < fDistY)
            {
                fMaxY = fDistY;
                nMaxY_ID = nID;
            }
        }

        //Top
        float fTopResult = 0.f;
        nLeftID = nMinY_ID - 1;
        nLeftID = nLeftID < 0 ? nIdNum - 1 : nLeftID;
        nRightID = nMinY_ID + 1;
        nRightID = nRightID >= nIdNum ? 0 : nRightID;
        float fLeft = (float)fabs(fMinY - (vecfptBallEdgePoint[nLeftID].m_y - CircleCoeff.m_y));
        float fRight = (float)fabs(fMinY - (vecfptBallEdgePoint[nRightID].m_y - CircleCoeff.m_y));
        if (fLeft < 2.f || fRight < 2.f)
            fTopResult = fMinY;

        //Btm
        float fBtmResult = 0.f;
        nLeftID = nMaxY_ID - 1;
        nLeftID = nLeftID < 0 ? nIdNum - 1 : nLeftID;
        nRightID = nMaxY_ID + 1;
        nRightID = nRightID >= nIdNum ? 0 : nRightID;
        fLeft = (float)fabs(fMaxY - (vecfptBallEdgePoint[nLeftID].m_y - CircleCoeff.m_y));
        fRight = (float)fabs(fMaxY - (vecfptBallEdgePoint[nRightID].m_y - CircleCoeff.m_y));
        if (fLeft < 2.f || fRight < 2.f)
            fBtmResult = fMaxY;

        //Left
        float fLeftResult = 0.f;
        nLeftID = nMinX_ID - 1;
        nLeftID = nLeftID < 0 ? nIdNum - 1 : nLeftID;
        nRightID = nMinX_ID + 1;
        nRightID = nRightID >= nIdNum ? 0 : nRightID;
        fLeft = (float)fabs(fMinX - (vecfptBallEdgePoint[nLeftID].m_x - CircleCoeff.m_x));
        fRight = (float)fabs(fMinX - (vecfptBallEdgePoint[nRightID].m_x - CircleCoeff.m_x));
        if (fLeft < 2.f || fRight < 2.f) //kircheis_20160525
        {
            fLeftResult = fMinX;
            if (fLeft < 0.5f || fRight < 0.5f)
                fLeftResult = (float)min(fLeft, fRight) + fMinX;
        }

        //Right
        float fRightResult = 0.f;
        nLeftID = nMaxX_ID - 1;
        nLeftID = nLeftID < 0 ? nIdNum - 1 : nLeftID;
        nRightID = nMaxX_ID + 1;
        nRightID = nRightID >= nIdNum ? 0 : nRightID;
        fLeft = (float)fabs(fMaxX - (vecfptBallEdgePoint[nLeftID].m_x - CircleCoeff.m_x));
        fRight = (float)fabs(fMaxX - (vecfptBallEdgePoint[nRightID].m_x - CircleCoeff.m_x));
        if (fLeft < 2.f || fRight < 2.f)
        {
            fRightResult = fMaxX;
            if (fLeft < 0.5f || fRight < 0.5f) //kircheis_20160525
                fRightResult = fMaxX - (float)min(fLeft, fRight);
        }

        //float fResultX = -fLeftResult + fRightResult;
        //float fResultY = -fTopResult + fBtmResult;

        //}}

        //{{ //kircheis_FootTip_BW 개선 각 Edge Point에서 가장 먼 Point간의 거리를 누적한 다음 상위 3개의 거리를 평균 내서 쓰자
        static BOOL bTest = TRUE;
        if (nIdNum > 10 && bTest)
        {
            float fMaxDist = 0.f;
            long nLengthNum = nIdNum / 2;
            vecfBallEdgeDist.resize(nLengthNum);
            memset(&vecfBallEdgeDist[0], 0, nLengthNum * sizeof(float));
            for (long nID = 0; nID < nLengthNum; nID++)
            {
                long nDestID = nID + nLengthNum;
                auto& cur = vecfptBallEdgePoint[nID];
                auto& dst = vecfptBallEdgePoint[nDestID];

                if (cur.m_x > 0.f && cur.m_y > 0.f && dst.m_x > 0.f && dst.m_y > 0.f)
                {
                    float distance = 0.f;
                    Ipvm::Geometry::GetDistance(cur, dst, distance);

                    vecfBallEdgeDist[nID] = distance;
                    fMaxDist = (float)max(fMaxDist, distance);
                }
            }
            vecfBallEdgeDist.clear();
        }
        //}}

        Ipvm::Rect32r frtBallWidthROI;
        frtBallWidthROI.m_left = CircleCoeff.m_x - CircleCoeff.m_radius;
        frtBallWidthROI.m_right = CircleCoeff.m_x + CircleCoeff.m_radius;
        frtBallWidthROI.m_top = CircleCoeff.m_y - CircleCoeff.m_radius;
        frtBallWidthROI.m_bottom = CircleCoeff.m_y + CircleCoeff.m_radius;

        m_vecrtBallPosition_pixel[ball.m_index] = Ipvm::Conversion::ToRect32s(frtBallWidthROI);
        m_vecrtBallCircleROI.push_back(frtBallWidthROI);

        const float ballDiameter_px = 2 * CircleCoeff.m_radius;

        m_sBallAlignResult->m_ballWidths_px[ball.m_index] = ballDiameter_px;

        if ((fBallWidth + fBallWidth) <= ballDiameter_px || fBallHalfWidth >= ballDiameter_px)
        {
            m_missingBalls.emplace_back(ball.m_index, Ipvm::Conversion::ToRect32s(rtfSpecROI));

            continue;
        }
    }

    return TRUE;
}

void VisionInspectionBgaBottom2D::UpdateSweepAngleTable(
    long nSectionNum, std::vector<float>& vecfCos, std::vector<float>& vecfSin)
{
    vecfCos.resize(nSectionNum);
    vecfSin.resize(nSectionNum);

    float* pfCos = &vecfCos[0];
    float* pfSin = &vecfSin[0];

    for (long nSection = 0; nSection < nSectionNum; nSection++)
    {
        float fCurAngle = float(nSection) / (float)nSectionNum * (float)ITP_TwoPI;
        pfCos[nSection] = float(cos(fCurAngle));
        pfSin[nSection] = float(sin(fCurAngle));
    }
}

float VisionInspectionBgaBottom2D::GetBallEllipticity(std::vector<Ipvm::Point32r2> i_vecfptEdgePoints,
    Ipvm::CircleEq32r i_circleEqBall, BOOL bUseInnerCirclePoint, float fSpecRadiusPxl) //kircheis_TMI
{
    UNREFERENCED_PARAMETER(fSpecRadiusPxl);

    long nPointNum = (long)i_vecfptEdgePoints.size();
    const float fBallRadius = i_circleEqBall.m_radius;
    float fResult(0.f), fSum(0.f), fDist(0.f);

    Ipvm::Point32r2 fptCenter = Ipvm::Point32r2(i_circleEqBall.m_x, i_circleEqBall.m_y);
    for (long nPoint = 0; nPoint < nPointNum; nPoint++)
    {
        auto& dst = i_vecfptEdgePoints[nPoint];
        Ipvm::Geometry::GetDistance(fptCenter, dst, fDist);

        if (!bUseInnerCirclePoint && fDist < fBallRadius)
            continue;

        fSum += (float)fabs(fDist - fBallRadius);
    }

    fResult = (fSum / (float)nPointNum) / fBallRadius * 100.f;

    return fResult;
}

BOOL VisionInspectionBgaBottom2D::GetQualityInfo(const CString strBallQualityName,
    const std::vector<Package::Ball> i_BallData, const Ipvm::Image8u& i_ballImage, long nQualityThresh)
{
    BOOL CheckGroupInsp = !strBallQualityName.IsEmpty();
    BOOL AllDataSpecUsse = FALSE;
    BOOL GroupDataSpecUsse = FALSE;

    auto* result = m_resultGroup.GetResultByName(g_szBallInspectionName[BALL_INSPECTION_BALL_QUALITY]);
    if (result == nullptr)
    {
        return FALSE;
    }
    auto* spec = GetSpecByName(result->m_resultName);
    if (spec == nullptr)
    {
        return FALSE;
    }

    AllDataSpecUsse = spec->m_use;

    float* pfValue{};

    if (AllDataSpecUsse)
        pfValue = &result->m_objectErrorValues[0];

    auto* Groupresult = result;
    auto* Groupspec = spec;
    float* pfGroupValue{};

    if (CheckGroupInsp)
    {
        Groupresult = m_resultGroup.GetResultByName(strBallQualityName);
        if (Groupresult == nullptr)
        {
            return FALSE;
        }
        Groupspec = GetSpecByName(Groupresult->m_resultName);
        if (Groupspec == nullptr)
        {
            return FALSE;
        }

        GroupDataSpecUsse = Groupspec->m_use;

        if (GroupDataSpecUsse)
        {
            Groupresult->Resize(CAST_LONG(i_BallData.size()));

            pfGroupValue = &Groupresult->m_objectErrorValues[0];
        }
    }

    if (!AllDataSpecUsse && !GroupDataSpecUsse)
        return TRUE;

    const long imageSizeX = getImageLotInsp().GetImageSizeX();
    const long imageSizeY = getImageLotInsp().GetImageSizeY();

    long nSectionNum = SPEC_QUALITY_SWEEP_LINE_COUNT;

    // 각도에 따른 삼각함수 테이블을 얻어온다.
    float* pfCos = &m_vecfCosForQuality[0];
    float* pfSin = &m_vecfSinForQuality[0];

    Ipvm::LineSeg32r LineTemp;
    Ipvm::Point32r2 pftOuterPoint = Ipvm::Point32r2(0.f, 0.f);
    Ipvm::Point32r2 pftInnerPoint = Ipvm::Point32r2(0.f, 0.f);

    long idx = 0;

    for (const auto& ball : i_BallData)
    {
        auto& GroupParas = m_pVisionPara->m_parameters[ball.m_typeIndex];
        float fSamplingInterval = float(GroupParas.m_qualityCheckSamplingInterval);

        BOOL bInnerInvalid;
        float fValidCount(0);

        // 2004.10.18 이현민
        const float fCenX = m_sBallAlignResult->m_ballWidthCenterPos_px[ball.m_index].m_x;
        const float fCenY = m_sBallAlignResult->m_ballWidthCenterPos_px[ball.m_index].m_y;

        const float fCurRadius = m_sBallAlignResult->m_ballWidths_px[ball.m_index] * 0.5f;

        float fSweepBegin = float(fCurRadius * GroupParas.m_qualityCheckRangeMin);
        float fSweepEnd = float(fCurRadius * GroupParas.m_qualityCheckRangeMax);

        for (long nSection = 0; nSection < nSectionNum; nSection++)
        {
            // 바깥 부분 Quality 불량 계산
            if (GroupParas.m_qualityCheckOuterVoid > 0.f) // 영훈 : 만약 Outer void가 값이 입력 되었다면
            {
                // 영훈 : Outer void 값만큼 센터로부터 입력된 값의 Pixel까지 이동한 후에
                float fCurX = (float)(fCenX + fCurRadius * GroupParas.m_qualityCheckOuterVoid * pfCos[nSection]);
                float fCurY = (float)(fCenY + fCurRadius * GroupParas.m_qualityCheckOuterVoid * pfSin[nSection]);
                long nCurX = long(fCurX + 0.5f);
                long nCurY = long(fCurY + 0.5f);

                if (nCurX < 0 || nCurY < 0 || nCurX >= imageSizeX || nCurY >= imageSizeY)
                {
                    return FALSE;
                }

                // 영훈 : 그 Pixel의 값이 >= 설정된 Ball Thresold값보다 크면 Reject
                BYTE byValue = i_ballImage[nCurY][nCurX];
                if (byValue >= nQualityThresh)
                {
                    continue;
                }

                pftOuterPoint = Ipvm::Point32r2(fCurX, fCurY);
            }

            // Hole 부분 Quality 불량 계산
            // 영훈 : 여기도 동일하지만 outer에서 Pass가 되었다면
            bInnerInvalid = FALSE;
            pftInnerPoint = Ipvm::Point32r2(fCenX, fCenY);
            if (GroupParas.m_qualityCheckInnerVoid > 0.f)
            {
                // 영훈: 위와 같은 방식으로 계산한 후에
                float fCurX = (float)(fCenX + fCurRadius * GroupParas.m_qualityCheckInnerVoid * pfCos[nSection]);
                float fCurY = (float)(fCenY + fCurRadius * GroupParas.m_qualityCheckInnerVoid * pfSin[nSection]);
                long nCurX = long(fCurX + 0.5f);
                long nCurY = long(fCurY + 0.5f);

                // 영훈 : 그 부분의 Pixel값이 >= Ball Threshold값 보다 크면 Reject이지만 다음으로 넘기지 않고 +0.5점만 준다
                BYTE byValue = i_ballImage[nCurY][nCurX];
                if (byValue >= nQualityThresh)
                {
                    bInnerInvalid = TRUE;
                }

                pftInnerPoint = Ipvm::Point32r2(fCurX, fCurY);
            }

            // 영훈 20140627_BallQuality_SweeqpLine : Ball Quality 검사 Sweep Line도 화면에 그려줘서 Teaching이 편하도록 한다.
            if (GroupParas.m_qualityCheckOuterVoid > 0.f)
            {
                LineTemp.m_sx = pftOuterPoint.m_x;
                LineTemp.m_sy = pftOuterPoint.m_y;
                LineTemp.m_ex = pftInnerPoint.m_x;
                LineTemp.m_ey = pftInnerPoint.m_y;
                m_vecQualitySweepLine.push_back(LineTemp);
            }

            // 영훈: 위에서 선택된 Void Pixel 부분의 값이 입력이 됐다면 Range Min으로붙 max까지 Pixel을 확인한다.
            // 확인 하는 Pixel은 Check Sampleing interval에 입력된 값 만큼 점프하며 확인한다 (2라고 적히면 2픽셀씩 뛰어넘어가며 확인)
            for (float fSweep = fSweepBegin; fSweep < fSweepEnd; fSweep += fSamplingInterval)
            {
                float fCurX = fCenX + fSweep * pfCos[nSection];
                float fCurY = fCenY + fSweep * pfSin[nSection];
                long nCurX = long(fCurX + 0.5f);
                long nCurY = long(fCurY + 0.5f);

                // 영훈 : 확인하는 Pixel의 값이 Ball Threhold값 보다
                // 높으면 +0.5(위에서 Outer는 Pass, Inner가 불량일때) 나 +1(Outerm Inner 둘다 패스일때) 점을 준다.
                BYTE byValue = i_ballImage[nCurY][nCurX];
                if (byValue >= nQualityThresh)
                {
                    m_vecfptQualityCheckPairPoint.push_back(Ipvm::Point32r2((float)nCurX, (float)nCurY));
                    if (bInnerInvalid)
                        fValidCount += 0.5f;
                    else
                        fValidCount += 1;
                    break;
                }
            }
        }

        float fValue = fValidCount / nSectionNum * 100;

        if (AllDataSpecUsse)
            pfValue[ball.m_index] = fValue;

        if (GroupDataSpecUsse)
        {
            pfGroupValue[idx] = fValue;
            idx++;
        }
    }

    return TRUE;
}

BOOL VisionInspectionBgaBottom2D::GetQualityInfoByRingThickness(const CString strBallQualityName,
    const std::vector<Package::Ball> i_BallData, const Ipvm::Image8u& i_ballImage, long nQualityThresh)
{
    BOOL CheckGroupInsp = !strBallQualityName.IsEmpty();
    BOOL AllDataSpecUsse = FALSE;
    BOOL GroupDataSpecUsse = FALSE;

    auto* result = m_resultGroup.GetResultByName(g_szBallInspectionName[BALL_INSPECTION_BALL_QUALITY]);
    if (result == nullptr)
    {
        return FALSE;
    }
    auto* spec = GetSpecByName(result->m_resultName);
    if (spec == nullptr)
    {
        return FALSE;
    }

    AllDataSpecUsse = spec->m_use;

    float* pfValue{};

    if (AllDataSpecUsse)
        pfValue = &result->m_objectErrorValues[0];

    auto* Groupresult = result;
    auto* Groupspec = spec;
    float* pfGroupValue{};

    if (CheckGroupInsp)
    {
        Groupresult = m_resultGroup.GetResultByName(strBallQualityName);
        if (Groupresult == nullptr)
        {
            return FALSE;
        }
        Groupspec = GetSpecByName(Groupresult->m_resultName);
        if (Groupspec == nullptr)
        {
            return FALSE;
        }

        GroupDataSpecUsse = Groupspec->m_use;

        if (GroupDataSpecUsse)
        {
            Groupresult->Resize(CAST_LONG(i_BallData.size()));

            pfGroupValue = &Groupresult->m_objectErrorValues[0];
        }
    }

    if (!AllDataSpecUsse && !GroupDataSpecUsse)
        return TRUE;

    long nSectionNum = SPEC_QUALITY_SWEEP_LINE_COUNT;
    const float fUMtoPixels = getScale().umToPixelXY();

    // 각도에 따른 삼각함수 테이블을 얻어온다.
    float* pfCos = &m_vecfCosForQuality[0];
    float* pfSin = &m_vecfSinForQuality[0];

    Ipvm::LineSeg32r LineTemp;
    Ipvm::Point32r2 pftOuterPoint = Ipvm::Point32r2(0.f, 0.f);
    Ipvm::Point32r2 pftInnerPoint = Ipvm::Point32r2(0.f, 0.f);

    float fCurX(0.f), fCurY(0.f);
    long nCurX(0), nCurY(0);
    float fQualityThickTotal(0.f);
    BOOL bVoidCheck;
    float fError(0.f);

    std::vector<long> vecnOverPixelCount;

    Qualtiy_BallAreaPixelValidCount(i_ballImage, nQualityThresh, vecnOverPixelCount);

    long idx = 0;

    for (const auto& ball : i_BallData)
    {
        auto& GroupParas = m_pVisionPara->m_parameters[ball.m_typeIndex];
        float fQualityRingThicknessRate_Min = (float)(GroupParas.m_qualityRingRadius_px
            - (GroupParas.m_qualityRingRadius_px * GroupParas.m_qualityRingThicknessRate));
        float fQualityRingThicknessRate_Max = (float)(GroupParas.m_qualityRingRadius_px
            + (GroupParas.m_qualityRingRadius_px * GroupParas.m_qualityRingThicknessRate));

        const float fSpecRadius = float(ball.m_diameter_um * fUMtoPixels);

        const float fCurRadius = m_sBallAlignResult->m_ballWidths_px[ball.m_index] * 0.5f;

        float fValidCount = 0;

        const float fCenX = m_sBallAlignResult->m_ballWidthCenterPos_px[ball.m_index].m_x;
        const float fCenY = m_sBallAlignResult->m_ballWidthCenterPos_px[ball.m_index].m_y;

        float fSweepBegin = float(fSpecRadius * GroupParas.m_qualityCheckInnerVoid);
        float fSweepEnd = float(fSpecRadius * GroupParas.m_qualityCheckOuterVoid);

        for (long nSection = 0; nSection < nSectionNum; nSection++)
        {
            float fValidCount_Current = 0;
            bVoidCheck = TRUE;

            // 바깥 부분 Quality 불량 계산
            if (GroupParas.m_qualityCheckOuterVoid > 0.f) // 영훈 : 만약 Outer void가 값이 입력 되었다면
            {
                // 영훈 : Outer void 값만큼 센터로부터 입력된 값의 Pixel까지 이동한 후에
                fCurX = (float)(fCenX + fCurRadius * GroupParas.m_qualityCheckOuterVoid * pfCos[nSection]);
                fCurY = (float)(fCenY + fCurRadius * GroupParas.m_qualityCheckOuterVoid * pfSin[nSection]);
                nCurX = long(fCurX + 0.5f);
                nCurY = long(fCurY + 0.5f);
                // 영훈 : 그 Pixel의 값이 >= 설정된 Ball Thresold값보다 크면 Reject
                BYTE byValue = i_ballImage[nCurY][nCurX];

                if (byValue > nQualityThresh)
                {
                    bVoidCheck = FALSE;
                }

                pftOuterPoint = Ipvm::Point32r2(fCurX, fCurY);
            }

            // Hole 부분 Quality 불량 계산
            pftInnerPoint = Ipvm::Point32r2(fCenX, fCenY);
            if (GroupParas.m_qualityCheckInnerVoid > 0.f)
            {
                fCurX = (float)(fCenX + fCurRadius * GroupParas.m_qualityCheckInnerVoid * pfCos[nSection]);
                fCurY = (float)(fCenY + fCurRadius * GroupParas.m_qualityCheckInnerVoid * pfSin[nSection]);
                nCurX = long(fCurX + 0.5f);
                nCurY = long(fCurY + 0.5f);

                BYTE byValue = i_ballImage[nCurY][nCurX];
                if (byValue > nQualityThresh)
                {
                    bVoidCheck = FALSE;
                }

                pftInnerPoint = Ipvm::Point32r2(fCurX, fCurY);
            }
            else
            {
                fCurX = fCenX + pfCos[nSection];
                fCurY = fCenY + pfSin[nSection];
                pftInnerPoint = Ipvm::Point32r2(fCurX, fCurY);
            }

            // 영훈 20141119_BallQuality_RingThickness : Inner void와 outer void를 체크할 경우 둘중 하나라도 불량이면 다음 계산으로 넘어간다. 점수제 변경
            if (bVoidCheck)
            {
                fValidCount += 0.5f;
                fValidCount_Current += 0.5f;
            }

            // 영훈 20140627_BallQuality_SweeqpLine : Ball Quality 검사 Sweep Line도 화면에 그려줘서 Teaching이 편하도록 한다.
            if (GroupParas.m_qualityCheckOuterVoid > 0.f)
            {
                LineTemp.m_sx = pftOuterPoint.m_x;
                LineTemp.m_sy = pftOuterPoint.m_y;
                LineTemp.m_ex = pftInnerPoint.m_x;
                LineTemp.m_ey = pftInnerPoint.m_y;
                m_vecQualitySweepLine.push_back(LineTemp);
            }

            fQualityThickTotal = 0.f;

            for (float fSweep = fSweepBegin; fSweep < fSweepEnd; fSweep++)
            {
                fCurX = fCenX + fSweep * pfCos[nSection];
                fCurY = fCenY + fSweep * pfSin[nSection];
                nCurX = long(fCurX + 0.5f);
                nCurY = long(fCurY + 0.5f);

                // 영훈 20150213_BGA_Quality_Edge_Error : Quality Edge Detect 시 엉뚱한 영역을 잡으면 -값이 들어가 Index가 꼬여 Crash 발생
                if (nCurX < 0 || nCurY < 0)
                    continue;

                // 영훈 20150116_BGA_RingThinkness : Ball 두께 검사 시 위치 정보를 함께 사용하게 되면 값이 누적되어 실제 두께보다 큰 값이 나오게 되어 위치 정보 삭제
                if (i_ballImage[nCurY][nCurX] >= nQualityThresh)
                {
                    fQualityThickTotal++;
                }
            }

            if (fQualityRingThicknessRate_Min < fQualityThickTotal
                && fQualityRingThicknessRate_Max > fQualityThickTotal)
            {
                fValidCount += 0.5f;
                fValidCount_Current += 0.5f;
            }

            if (fValidCount_Current > 0.5f)
            {
                float fSweep = fSweepEnd * 0.5f;
                fCurX = fCenX + fSweep * pfCos[nSection];
                fCurY = fCenY + fSweep * pfSin[nSection];

                m_vecfptQualityCheckPairPoint.push_back(Ipvm::Point32r2(fCurX, fCurY));
            }
        }

        fError = fValidCount / nSectionNum * 100.f;

        if ((long)vecnOverPixelCount.size() <= 0)
            return FALSE;

        float fValue
            = (fError - vecnOverPixelCount[ball.m_index]) < 0.f ? 0.f : fError - vecnOverPixelCount[ball.m_index];

        if (AllDataSpecUsse)
            pfValue[ball.m_index] = fValue;

        if (GroupDataSpecUsse)
        {
            pfGroupValue[idx] = fValue;
            idx++;
        }
    }

    return TRUE;
}

BOOL VisionInspectionBgaBottom2D::DoQualityTeach(
    const Ipvm::Image8u& i_ballImage, long nQualityThresh, long i_nBallGroupID)
{
    long nSectionNum = SPEC_QUALITY_SWEEP_LINE_COUNT;
    const float fUMtoPixels = getScale().umToPixelXY();

    // 각도에 따른 삼각함수 테이블을 얻어온다.
    float* pfCos = &m_vecfCosForQuality[0];
    float* pfSin = &m_vecfSinForQuality[0];

    std::vector<float> vecfRingThinkness;

    auto& GroupParas = m_pVisionPara->m_parameters[i_nBallGroupID];
    for (const auto& ball : m_packageSpec.m_ballMap->m_balls)
    {
        if (ball.m_typeIndex != i_nBallGroupID)
            continue;

        const float fSpecBallWidth = float(ball.m_diameter_um * fUMtoPixels);

        // 2004.10.18 이현민
        const float fCenX = m_sBallAlignResult->m_ballWidthCenterPos_px[ball.m_index].m_x;
        const float fCenY = m_sBallAlignResult->m_ballWidthCenterPos_px[ball.m_index].m_y;

        // fBallRadius = ( [Ball Width] + [Ball Height] ) / 4
        float fBallRadius = fSpecBallWidth * 0.5f;
        float fBeginSweep = float(fBallRadius * GroupParas.m_qualityCheckInnerVoid);
        float fEndSweep = float(fBallRadius * GroupParas.m_qualityCheckOuterVoid);

        // 환형의 평균두께 및 평균 반지름 계산
        float fQualityThickTotal = 0.f;

        for (long nSectionID = 0; nSectionID < nSectionNum; nSectionID++)
        {
            for (float fSweep = fBeginSweep; fSweep < fEndSweep; fSweep++)
            {
                long nCurX = long(fCenX + fSweep * pfCos[nSectionID] + 0.5f);
                long nCurY = long(fCenY + fSweep * pfSin[nSectionID] + 0.5f);

                if (i_ballImage[nCurY][nCurX] >= nQualityThresh)
                {
                    fQualityThickTotal++;
                }
            }
        }

        fQualityThickTotal /= nSectionNum;

        vecfRingThinkness.push_back(fQualityThickTotal);
    }

    long nBallTotalNum = (long)vecfRingThinkness.size();
    long nBegin = long(nBallTotalNum * SEPC_QUALITY_COMPENSATION_RANGE_MIN + 0.5f);
    long nEnd = long(nBallTotalNum * SEPC_QUALITY_COMPENSATION_RANGE_MAX + 0.5f) + 1;

    std::sort(vecfRingThinkness.begin(), vecfRingThinkness.end());

    if (nEnd - nBegin < 1)
        return FALSE;

    long nCount(0);
    float fSum(0.f);

    for (long n = nBegin; n < nEnd; n++)
    {
        fSum += vecfRingThinkness[n];
        nCount++;
    }

    GroupParas.m_qualityRingRadius_px = fSum / (float)nCount;

    return TRUE;
}

// 20140710_BGA_Align : debuginfo 작성 시 Size Check를 하여 0보다 크면 입력되도록 한다. (Crash 수정)
void VisionInspectionBgaBottom2D::SetDebugInfo(const bool detailSetupMode)
{
    SetDebugInfoItem(detailSetupMode, _T("Ball Spec Point"), m_vecBallSpec_Body);
    SetDebugInfoItem(detailSetupMode, _T("Ball Missing ROI"), m_vecrtDebugMissingBallROI);
    SetDebugInfoItem(detailSetupMode, _T("Ball Contrast Value"), m_vecDebugInfoValue);
    SetDebugInfoItem(detailSetupMode, _T("Ball Width ROI"), m_vecrtBallCircleROI, TRUE); //kircheis_BPQ
    SetDebugInfoItem(detailSetupMode, _T("Ball Rough Width Edge Sweep Line"), m_result->m_rough_vecSweepLine);
    SetDebugInfoItem(
        detailSetupMode, _T("Ball Rough Width Edge Total Point"), m_result->m_rough_vecfptBallWidthEdgeTotalPoint);
    SetDebugInfoItem(
        detailSetupMode, _T("Ball Rough Align Circle"), m_result->m_rough_vecellipseAlignCircle, TRUE); //kircheis_MED3
    SetDebugInfoItem(detailSetupMode, _T("Ball Width Edge Sweep Line"), m_result->m_vecSweepLine);
    SetDebugInfoItem(detailSetupMode, _T("Ball Width Edge Total Point"), m_result->m_vecfptBallWidthEdgeTotalPoint);
    SetDebugInfoItem(detailSetupMode, _T("Ball Width Edge Valid Point"), m_result->m_vecfptBallWidthEdgeValidPoint);
    SetDebugInfoItem(detailSetupMode, _T("Ball Quality Sweep Line"), m_vecQualitySweepLine);
    SetDebugInfoItem(detailSetupMode, _T("Ball Quality Check Pair Point"), m_vecfptQualityCheckPairPoint);

    auto* psDebugInfo = m_DebugInfoGroup.GetDebugInfo(_T("Ball Align Result by Ball Insp"));
    if (psDebugInfo != NULL)
    {
        if (psDebugInfo->pData != NULL)
        {
            psDebugInfo->Reset();
        }
        psDebugInfo->nDataNum = 1;
        VisionAlignResult* psBallAlignResult = new VisionAlignResult[psDebugInfo->nDataNum];
        for (long i = 0; i < psDebugInfo->nDataNum; i++)
        {
            psBallAlignResult[i] = *m_sBallAlignResult;
        }
        psDebugInfo->pData = psBallAlignResult;
    }

    long nBallNum = (long)m_vecrtBallCircleROI.size();
    std::vector<Ipvm::EllipseEq32r> vecsEllipse(nBallNum);
    for (long i = 0; i < nBallNum; i++)
    {
        vecsEllipse[i].m_x = (m_vecrtBallCircleROI[i].m_left + m_vecrtBallCircleROI[i].m_right) * .5f;
        vecsEllipse[i].m_y = (m_vecrtBallCircleROI[i].m_top + m_vecrtBallCircleROI[i].m_bottom) * .5f;
        vecsEllipse[i].m_xradius = (m_vecrtBallCircleROI[i].m_right - m_vecrtBallCircleROI[i].m_left) * .5f;
        vecsEllipse[i].m_yradius = (m_vecrtBallCircleROI[i].m_bottom - m_vecrtBallCircleROI[i].m_top) * .5f;
    }
    SetDebugInfoItem(detailSetupMode, _T("Ball Width Circle"), vecsEllipse, true);

    Ipvm::Image8u BallMaskImage;
    if (!getReusableMemory().GetInspByteImage(BallMaskImage))
        return;
    Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(BallMaskImage), 0, BallMaskImage);

    for (auto BallEllipse : vecsEllipse)
        Ipvm::ImageProcessing::Fill(BallEllipse, 255, BallMaskImage);

    SetDebugInfoItem(detailSetupMode, _T("Ball Mask Image"), BallMaskImage, true);

    //{{//kircheis_MED3 //Rough Align Mask도 만들자
    Ipvm::Image8u BallRoughAlignMaskImage;
    if (!getReusableMemory().GetInspByteImage(BallRoughAlignMaskImage))
        return;
    Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(BallRoughAlignMaskImage), 0, BallRoughAlignMaskImage);

    for (auto BallRoughEllipse : m_result->m_rough_vecellipseAlignCircle)
        Ipvm::ImageProcessing::Fill(BallRoughEllipse, 255, BallRoughAlignMaskImage);
    //}}

    long nBallTypeCount = CAST_LONG(m_packageSpec.m_originalballMap->m_ballTypes.size());
    std::vector<SurfaceLayerRoi*> groupSurfaceRoi;

    groupSurfaceRoi.resize(nBallTypeCount);

    for (int i = 0; i < nBallTypeCount; i++)
    {
        CString str;

        str.Format(_T("Measured - Ball Group%s"), (LPCTSTR)m_packageSpec.m_originalballMap->m_ballTypes[i].m_groupID);

        groupSurfaceRoi[i] = getReusableMemory().AddSurfaceLayerRoiClass(str);
        groupSurfaceRoi[i]->Reset();

        str.Empty();
    }

    if (auto* surfaceRoi = getReusableMemory().AddSurfaceLayerRoiClass(_T("Measured - Ball")))
    {
        surfaceRoi->Reset();
        int idx = 0;

        for (auto& ball : vecsEllipse)
        {
            if (nBallTypeCount > 1)
                groupSurfaceRoi[m_packageSpec.m_ballMap->m_balls[idx].m_typeIndex]->Add(ball);

            surfaceRoi->Add(ball);

            idx++;
        }
    }

    auto* alignMask = getReusableMemory().AddSurfaceLayerMaskClass(_T("Ball Rough Align Mask Image")); //kircheis_MED3
    if (alignMask)
    {
        alignMask->Reset();
        alignMask->Add(BallRoughAlignMaskImage);
    }
}

void VisionInspectionBgaBottom2D::PrepareInspection()
{
    long nCount = (long)m_packageSpec.m_ballMap->m_balls.size();

    for (long nInsp = BALL_INSPECTION_START; nInsp < BALL_INSPECTION_END; nInsp++)
    {
        auto* result = &m_resultGroup.m_vecResult[nInsp];

        if (!m_fixedInspectionSpecs[nInsp].m_use)
        {
            continue;
        }

        if (nInsp == BALL_INSPECTION_BALL_GRID_OFFSET_X || nInsp == BALL_INSPECTION_BALL_GRID_OFFSET_Y) //kircheis_BGABS
        {
            if (nCount > 0)
            {
                result->Resize(1);
            }
        }
        else
        {
            result->Resize(nCount);
        }
    }

    // Missing 은 검사 항목이 켜져있는 것과 무관하게 무조건 볼 별 결과를 수집할 것이므로, 결과 버퍼를 만들어 놓는다.
    m_resultGroup.GetResultByGuid(_T("{B0698F99-5390-40FB-AF77-1079512FA3BA}"))->Resize(nCount);

    return;
}

BOOL VisionInspectionBgaBottom2D::GetFindBallEdge(const Ipvm::Image8u& oblique, const Ipvm::Image8u& coax)
{
    Ipvm::Rect32s rtBody(0, 0, 0, 0);
    rtBody.m_left = (long)(min(m_sEdgeAlignResult->fptLT.m_x, m_sEdgeAlignResult->fptLB.m_x) + 0.5f);
    rtBody.m_right = (long)(max(m_sEdgeAlignResult->fptRT.m_x, m_sEdgeAlignResult->fptRB.m_x) + 0.5f);
    rtBody.m_top = (long)(min(m_sEdgeAlignResult->fptLT.m_y, m_sEdgeAlignResult->fptRT.m_y) + 0.5f);
    rtBody.m_bottom = (long)(max(m_sEdgeAlignResult->fptLB.m_y, m_sEdgeAlignResult->fptRB.m_y) + 0.5f);
    rtBody.InflateRect(10, 10, 10, 10);

    Ipvm::Image8u roughAlignImage = oblique;
    Ipvm::Image8u coaxImage = coax;

    std::vector<Ipvm::Point32r2> vfCirclePointXY;
    std::vector<Ipvm::Point32r2> vfValidCirclePointXY;

    Ipvm::CircleEq32r CircleCoeff;

    float fDistX, fDistY;
    float fEdgeRadius;
    float fSpec_Temp;
    float fRadiusSpec_Min;
    float fRadiusSpec_Max;

    long nFilteringMethod = 0;
    //long nFilteringMethod = 2;

    // 영훈 : Ball Missing의 정보를 기억하도록 한다.
    m_missingBalls.clear();
    m_vecrtBallCircleROI.clear();

    m_vecrtRoughBall.clear();

    float fUmToPixel = getScale().umToPixelXY();

    Ipvm::Rect32r rtfSpecROI;

    m_vecrtBallPosition_pixel.resize(m_packageSpec.m_ballMap->m_balls.size());

    for (const auto& ball : m_packageSpec.m_ballMap->m_balls)
    {
        long nSectionNumber = m_nWidthSweepLineCount[ball.m_typeIndex];

        // 각도에 따른 삼각함수 테이블을 얻어온다.
        float* pfCos = &m_vecfCosForBallCenter[ball.m_typeIndex][0];
        float* pfSin = &m_vecfSinForBallCenter[ball.m_typeIndex][0];

        float fSpecBallWidth = float(ball.m_diameter_um * fUmToPixel);
        float fSpecBallHalfWidth = float(0.5f * fSpecBallWidth);

        float fBallWidth = fSpecBallWidth;
        float fBallHalfWidth = fBallWidth * 0.5f;

        float fBallEdgeThreshold = fSpecBallWidth * 0.08f; //kircheis_ShinyNoise
        float fRoughSearchLength = fSpecBallWidth * 0.3f; // iCOS는 볼 Width에 따라 Search Length를 결정한다.
        float fRadius = fBallHalfWidth;

        Ipvm::Rect32s rtRoughSpecBall
            = Ipvm::Rect32s((int32_t)(m_vecBallSpec_Body[ball.m_index].m_x - (fSpecBallWidth * 0.5f) + 0.5f),
                (int32_t)(m_vecBallSpec_Body[ball.m_index].m_y - (fSpecBallWidth * 0.5f) + 0.5f),
                (int32_t)(m_vecBallSpec_Body[ball.m_index].m_x + (fSpecBallWidth * 0.5f) + 0.5f),
                (int32_t)(m_vecBallSpec_Body[ball.m_index].m_y + (fSpecBallWidth * 0.5f) + 0.5f));

        float fIntenResult;
        Ipvm::Rect32s rtRoughBall{};

        // 반지름 정도만 탐색한다.
        IntensityYPos(roughAlignImage, rtRoughSpecBall, (long)fSpecBallHalfWidth, rtRoughBall, fIntenResult, TRUE);
        IntensityXPos(roughAlignImage, rtRoughBall, (long)fSpecBallHalfWidth, rtRoughBall, fIntenResult, TRUE);
        m_vecrtRoughBall.push_back(rtRoughBall);

        // 볼 센터
        float fInX = (float)rtRoughBall.CenterPoint().m_x;
        float fInY = (float)rtRoughBall.CenterPoint().m_y;

        if (fInX <= 0 || fInY <= 0)
            return FALSE;

        rtfSpecROI.m_left = fInX - (fSpecBallWidth * 0.5f);
        rtfSpecROI.m_right = fInX + (fSpecBallWidth * 0.5f);
        rtfSpecROI.m_top = fInY - (fSpecBallWidth * 0.5f);
        rtfSpecROI.m_bottom = fInY + (fSpecBallWidth * 0.5f);
        Ipvm::Rect32s rtSpecBall = Ipvm::Conversion::ToRect32s(rtfSpecROI);

        vfCirclePointXY.clear();
        vfCirclePointXY.resize(nSectionNumber, Ipvm::Point32r2(0.f, 0.f));

        // Auto Threshold : 바닥면 기준
        std::vector<float> vecfEdgeRadius;
        vecfEdgeRadius.clear();
        for (long nSection = 0; nSection < nSectionNumber; nSection++)
        {
            Ipvm::Point32r2 fptSearchIn = Ipvm::Point32r2(fInX + (fRadius - fRoughSearchLength) * pfCos[nSection],
                fInY + (fRadius - fRoughSearchLength) * pfSin[nSection]);
            Ipvm::Point32r2 fptSearchOut = Ipvm::Point32r2(fInX + (fRadius + fRoughSearchLength) * pfCos[nSection],
                fInY + (fRadius + fRoughSearchLength) * pfSin[nSection]);

            m_result->m_vecSweepLine.emplace_back(fptSearchIn.m_x, fptSearchIn.m_y, fptSearchOut.m_x, fptSearchOut.m_y);

            Ipvm::Point32r2 fptEdge;
            BOOL bFirstEdge = FALSE;

            fEdgeRadius = 0.f;
            float fDefaultEdgeTreshValue = m_pEdgeDetect->SetMininumThreshold(10.f);
            if (FindEdge(
                    roughAlignImage, fptSearchIn, fptSearchOut, FALSE, bFirstEdge, fptEdge, fEdgeRadius)) // In -> Out
            {
                m_result->m_vecfptBallWidthEdgeTotalPoint.push_back(fptEdge);

                float fDetailSearch = 5.f; // 5pixel
                fDetailSearch = min(fDetailSearch, fRoughSearchLength);
                Ipvm::Point32r2 fptDetailSearchIn
                    = Ipvm::Point32r2(fptEdge.m_x - (fDetailSearch * 0.3f) * pfCos[nSection],
                        fptEdge.m_y - (fDetailSearch * 0.3f) * pfSin[nSection]);
                ;
                Ipvm::Point32r2 fptDetailSearchOut
                    = Ipvm::Point32r2(fptEdge.m_x + (fDetailSearch * 0.7f) * pfCos[nSection],
                        fptEdge.m_y + (fDetailSearch * 0.7f) * pfSin[nSection]);

                Ipvm::Point32r2 fptEdge2;
                float fEdgeRadius2 = 0.f;
                m_pEdgeDetect->SetMininumThreshold(1.f);
                if (FindEdge(coaxImage, fptDetailSearchIn, fptDetailSearchOut, TRUE, bFirstEdge, fptEdge2,
                        fEdgeRadius2)) // In -> Out
                {
                    vfCirclePointXY[nSection] = fptEdge2;
                    vecfEdgeRadius.push_back(fEdgeRadius2);
                }
            }

            m_pEdgeDetect->SetMininumThreshold(fDefaultEdgeTreshValue);
        }

        // 영훈 20131119 : 0의 값은 버리도록 하자.
        for (long n = (long)vfCirclePointXY.size() - 1; n >= 0; n--)
        {
            if (vfCirclePointXY[n].m_x <= 0.f || vfCirclePointXY[n].m_y <= 0.f)
            {
                vfCirclePointXY.erase(vfCirclePointXY.begin() + n);
            }
        }

        if ((long)vfCirclePointXY.size() <= 0)
        {
            m_missingBalls.emplace_back(ball.m_index, Ipvm::Conversion::ToRect32s(rtfSpecROI));
            m_vecrtBallCircleROI.push_back(rtfSpecROI);

            continue;
        }

        long nValidEdgeCount = (long)vfCirclePointXY.size();
        // 반 이하이면 다시 찾아 보자.

        BOOL bRemovePoint = FALSE;
        if (bRemovePoint)
        {
            // 가장 max로 튄넘들 5개 지우고 시작
            if (nValidEdgeCount > 30)
            {
                //long nDeletNum = 5;
                long nDeletNum = (long)((float)nValidEdgeCount * 0.2f + 0.5f);
                std::vector<float> vecfRadius(nValidEdgeCount);
                for (long n = 0; n < nValidEdgeCount; n++)
                {
                    fDistX = vfCirclePointXY[n].m_x - fInX;
                    fDistY = vfCirclePointXY[n].m_y - fInY;
                    fEdgeRadius = CAST_FLOAT(sqrt(fDistX * fDistX + fDistY * fDistY));
                    vecfRadius[n] = fEdgeRadius;
                }

                sort(vecfRadius.begin(), vecfRadius.end());
                float fLimitVal = vecfRadius[vecfRadius.size() - nDeletNum];
                for (long n = 0; n < vfCirclePointXY.size(); n++)
                {
                    fDistX = vfCirclePointXY[n].m_x - fInX;
                    fDistY = vfCirclePointXY[n].m_y - fInY;
                    fEdgeRadius = CAST_FLOAT(sqrt(fDistX * fDistX + fDistY * fDistY));
                    if (fEdgeRadius >= fLimitVal)
                    {
                        vfCirclePointXY.erase(vfCirclePointXY.begin() + n);
                        n--;
                    }
                }
            }
            nValidEdgeCount = (long)vfCirclePointXY.size();
        }

        BOOL bRemoveDist = TRUE;
        if (bRemoveDist)
        {
            long nNewValidEdgeCount = 0;
            vfValidCirclePointXY.clear();

            if (nFilteringMethod == 0)
            {
                Ipvm::DataFitting::FitToCircle(nValidEdgeCount, &vfCirclePointXY[0], CircleCoeff);

                // 구한 원의 원주에 대해서 일정 거리 이상 떨어진 놈들을 없애고 다시 피팅
                for (long nSection = 0; nSection < nValidEdgeCount; nSection++)
                {
                    fDistX = vfCirclePointXY[nSection].m_x - CircleCoeff.m_x;
                    fDistY = vfCirclePointXY[nSection].m_y - CircleCoeff.m_y;
                    fEdgeRadius = CAST_FLOAT(sqrt(fDistX * fDistX + fDistY * fDistY));

                    // 영훈 20140616 : 찾은 Ball의 반지름이 입력한 값보다 크게 튀는 Edge는 필터링하여 다시 피팅한다.
                    float fValidPercentage = 0.1f;
                    fSpec_Temp = fValidPercentage * CircleCoeff.m_radius;
                    fRadiusSpec_Min = CircleCoeff.m_radius - fSpec_Temp;
                    fRadiusSpec_Max = CircleCoeff.m_radius + fSpec_Temp;

                    if (fRadiusSpec_Min < fEdgeRadius && fEdgeRadius < fRadiusSpec_Max)
                    {
                        vfValidCirclePointXY.push_back(vfCirclePointXY[nSection]);

                        m_result->m_vecfptBallWidthEdgeValidPoint.push_back(vfCirclePointXY[nSection]);
                        nNewValidEdgeCount++;
                    }
                }
            }
            else if (nFilteringMethod == 1)
            {
                // Spec 기준으로 벗어난 넘들 제거 후 써클 피팅.
                for (long nSection = 0; nSection < nValidEdgeCount; nSection++)
                {
                    fDistX = vfCirclePointXY[nSection].m_x - fInX;
                    fDistY = vfCirclePointXY[nSection].m_y - fInY;
                    fEdgeRadius = CAST_FLOAT(sqrt(fDistX * fDistX + fDistY * fDistY));

                    float fValidPercentage = 0.12f;
                    fSpec_Temp = fValidPercentage * fSpecBallHalfWidth;
                    fRadiusSpec_Min = fSpecBallHalfWidth - fSpec_Temp;
                    fRadiusSpec_Max = fSpecBallHalfWidth + fSpec_Temp;

                    if (fRadiusSpec_Min < fEdgeRadius && fEdgeRadius < fRadiusSpec_Max)
                    {
                        vfValidCirclePointXY.push_back(vfCirclePointXY[nSection]);

                        m_result->m_vecfptBallWidthEdgeValidPoint.push_back(vfCirclePointXY[nSection]);
                        nNewValidEdgeCount++;
                    }
                }
            }
            else if (nFilteringMethod == 2)
            {
                Ipvm::LineEq32r line(0.f, 0.f, 0.f);
                float fTolerence = 1.f;

                std::vector<Ipvm::Point32r2> vecfptEdgeData;
                for (long n = 0; n < nValidEdgeCount; n++)
                {
                    if (vecfEdgeRadius[n] > 0.f)
                        vecfptEdgeData.push_back(Ipvm::Point32r2((float)n, vecfEdgeRadius[n]));
                }

                if (vecfptEdgeData.size())
                {
                    Ipvm::DataFitting::FitToLine(long(vecfptEdgeData.size()), &vecfptEdgeData[0], line);

                    // 피팅라인에서 벗어난 넘 Filtering
                    for (long i = 0; i < (long)vecfptEdgeData.size(); i++)
                    {
                        float det = CAST_FLOAT(sqrt(line.m_a * line.m_a + line.m_b * line.m_b));
                        if (det < FLT_MIN)
                            return FALSE;
                        float fDist = CAST_FLOAT(fabs(line.m_a * vecfptEdgeData[i].m_x
                                          + line.m_b * vecfptEdgeData[i].m_y + line.m_c))
                            / det;
                        if (fabs(fDist) < fTolerence)
                        {
                            vfValidCirclePointXY.push_back(vfCirclePointXY[i]);
                            m_result->m_vecfptBallWidthEdgeValidPoint.push_back(vfCirclePointXY[i]);
                            nNewValidEdgeCount++;
                        }
                    }
                }
            }
            else if (nFilteringMethod == 3)
            {
                //{{//kircheis_ShinyNoise
                BOOL bTryIgnoreNoise = TRUE;
                while (bTryIgnoreNoise)
                {
                    std::vector<Ipvm::Point32r2> vecfptEdgeData(0);
                    Ipvm::LineEq32r line(0.f, 0.f, 0.f);
                    float fDist = 0.f;
                    long nZeroPoint = 0;
                    long nNoisePoint = 0;

                    //정상적으로 Edge를 찾은 Point 만으로 Ipvm::Point32r2 Array를 구성하고
                    long nDataNum = (long)vecfEdgeRadius.size();
                    long nDataHalfNum = (long)((float)nDataNum * 0.5f + 0.5f);
                    for (long nIdx = 0; nIdx < nDataNum; nIdx++)
                    {
                        if (vecfEdgeRadius[nIdx] > 0.f)
                            vecfptEdgeData.push_back(Ipvm::Point32r2((float)nIdx, vecfEdgeRadius[nIdx]));
                        else
                            nZeroPoint++;
                    }

                    if (vecfptEdgeData.size() == 0)
                    {
                        bTryIgnoreNoise = FALSE;
                        continue;
                    }

                    //그걸로 라인 피팅을 한다.
                    Ipvm::DataFitting::FitToLine(long(vecfptEdgeData.size()), &vecfptEdgeData[0], line);

                    //라인과 각 Point의 거리를 계산하여 그 거리가 fBallEdgeThreshold보다 크면 그 Edge는 날린다. fBallEdgeThreshold는 Ball Width의 8%로 정의
                    for (long nIdx = 0; nIdx < nDataNum; nIdx++)
                    {
                        if (vecfEdgeRadius[nIdx] > 0.f)
                        {
                            fDist = CPI_Geometry::GetDistance_PointToLine((float)nIdx, vecfEdgeRadius[nIdx], line);
                            if (fDist > fBallEdgeThreshold)
                            {
                                vecfEdgeRadius[nIdx] = 0.f;
                                nNoisePoint++;
                            }
                        }
                    }

                    if ((nZeroPoint + nNoisePoint) > nDataHalfNum || nNoisePoint == 0)
                    {
                        //남은 Point가 찾아야하는 Point보다 절반 이하이거나 찾은 Noise Point가 없으면 그만
                        bTryIgnoreNoise = FALSE;
                        continue;
                    }

                    //남은 Point가 찾을 Point보다 절반 이상이고 찾은 Noise Point가 있으면 해당 Noise Point의 Data를 날려버려~~

                    for (long nIdx = 0; nIdx < nDataNum; nIdx++)
                    {
                        if (vecfEdgeRadius[nIdx] <= 0.f && vfCirclePointXY[nIdx].m_x != 0.f)
                        {
                            vfCirclePointXY[nIdx] = Ipvm::Point32r2(0.f, 0.f);
                        }
                    }
                }
                //}}

                // 영훈 20131119 : 0의 값은 버리도록 하자.
                for (long n = (long)vfCirclePointXY.size() - 1; n >= 0; n--)
                {
                    if (vfCirclePointXY[n].m_x <= 0.f || vfCirclePointXY[n].m_y <= 0.f)
                    {
                        vfCirclePointXY.erase(vfCirclePointXY.begin() + n);
                    }
                }

                nValidEdgeCount = (long)vfCirclePointXY.size();
                Ipvm::DataFitting::FitToCircle(nValidEdgeCount, &vfCirclePointXY[0], CircleCoeff);

                // 구한 원의 원주에 대해서 일정 거리 이상 떨어진 놈들을 없애고 다시 피팅
                for (long nSection = 0; nSection < nValidEdgeCount; nSection++)
                {
                    fDistX = vfCirclePointXY[nSection].m_x - CircleCoeff.m_x;
                    fDistY = vfCirclePointXY[nSection].m_y - CircleCoeff.m_y;
                    fEdgeRadius = CAST_FLOAT(sqrt(fDistX * fDistX + fDistY * fDistY));

                    // 영훈 20140616 : 찾은 Ball의 반지름이 입력한 값보다 크게 튀는 Edge는 필터링하여 다시 피팅한다.
                    float fValidPercentage = 0.1f;
                    fSpec_Temp = fValidPercentage * CircleCoeff.m_radius;
                    fRadiusSpec_Min = CircleCoeff.m_radius - fSpec_Temp;
                    fRadiusSpec_Max = CircleCoeff.m_radius + fSpec_Temp;

                    if (fRadiusSpec_Min < fEdgeRadius && fEdgeRadius < fRadiusSpec_Max)
                    {
                        vfValidCirclePointXY.push_back(vfCirclePointXY[nSection]);

                        m_result->m_vecfptBallWidthEdgeValidPoint.push_back(vfCirclePointXY[nSection]);
                        nNewValidEdgeCount++;
                    }
                }
            }
            else
            {
                return FALSE;
            }

            if (nNewValidEdgeCount < SPEC_BALL_EDGE_COUNT_MIN)
            {
                m_missingBalls.emplace_back(ball.m_index, Ipvm::Conversion::ToRect32s(rtfSpecROI));
                m_vecrtBallCircleROI.push_back(rtfSpecROI);

                continue;
            }

            // 새로 정한 무게중심 포인트를 이용해 원 피팅을 실시.
            Ipvm::DataFitting::FitToCircle(nNewValidEdgeCount, &vfValidCirclePointXY[0], CircleCoeff);
            m_result->m_vecfptBallWidthEdgeValidPoint.push_back(Ipvm::Point32r2(CircleCoeff.m_x, CircleCoeff.m_y));
        }
        else
        {
            Ipvm::DataFitting::FitToCircle(nValidEdgeCount, &vfCirclePointXY[0], CircleCoeff);

            for (long nSection = 0; nSection < nValidEdgeCount; nSection++)
            {
                m_result->m_vecfptBallWidthEdgeTotalPoint.push_back(vfCirclePointXY[nSection]);
                m_result->m_vecfptBallWidthEdgeValidPoint.push_back(vfCirclePointXY[nSection]);
            }
        }

        //// 20140616 영훈 : Ball Center는 여기서 넣어준다.frtBallWidthROI
        m_sBallAlignResult->m_ballWidthCenterPos_px[ball.m_index].m_x = CircleCoeff.m_x;
        m_sBallAlignResult->m_ballWidthCenterPos_px[ball.m_index].m_y = CircleCoeff.m_y;

        Ipvm::Rect32r frtBallWidthROI;
        frtBallWidthROI.m_left = CircleCoeff.m_x - CircleCoeff.m_radius;
        frtBallWidthROI.m_right = CircleCoeff.m_x + CircleCoeff.m_radius;
        frtBallWidthROI.m_top = CircleCoeff.m_y - CircleCoeff.m_radius;
        frtBallWidthROI.m_bottom = CircleCoeff.m_y + CircleCoeff.m_radius;

        m_vecrtBallPosition_pixel[ball.m_index] = Ipvm::Conversion::ToRect32s(frtBallWidthROI);
        m_vecrtBallCircleROI.push_back(frtBallWidthROI);

        const float ballDiameter_px = 2 * CircleCoeff.m_radius;

        m_sBallAlignResult->m_ballWidths_px[ball.m_index] = ballDiameter_px;

        if ((fSpecBallWidth + fSpecBallWidth) <= ballDiameter_px || fSpecBallHalfWidth >= ballDiameter_px)
        {
            m_missingBalls.emplace_back(ball.m_index, Ipvm::Conversion::ToRect32s(rtfSpecROI));
            continue;
        }
    }

    return TRUE;
}

BOOL VisionInspectionBgaBottom2D::IntensityYPos(const Ipvm::Image8u& image, Ipvm::Rect32s rtRough, long nSearchLength,
    Ipvm::Rect32s& o_rtResult, float& o_fResult, BOOL bMaxPos)
{
    o_fResult = FLT_MIN;

    long nHalfSearchLength = static_cast<long>((float)nSearchLength / 2.f + .5f);
    float fMaxMean = FLT_MIN;
    float fMinMean = FLT_MAX;
    long nMaxPos = LONG_MIN;
    long nMinPos = LONG_MAX;

    Ipvm::Rect32s rtROI = rtRough;
    rtROI.InflateRect(0, nHalfSearchLength);
    std::vector<BYTE> vecbyVal(rtROI.Height());

    if (Ipvm::Status::e_ok != Ipvm::ImageProcessing::MakeProjectionProfileAxisY(image, rtROI, &vecbyVal[0]))
    {
        return FALSE;
    }

    std::vector<double> vecdAvg(nHalfSearchLength * 2);
    long nRectHeight = rtRough.Height();
    long nDataNum = (long)vecbyVal.size() - nRectHeight;

    long i = 0;
    for (long nPos = -nHalfSearchLength; nPos < nHalfSearchLength; nPos++)
    {
        if (i > nDataNum)
        {
            ASSERT(0);
            return FALSE;
        }

        double dAvg = 0.f;
        for (long j = 0; j < nRectHeight; j++)
        {
            dAvg += (double)vecbyVal[i + j];
        }
        dAvg /= (double)nRectHeight;
        vecdAvg[i] = dAvg;

        if (dAvg > fMaxMean)
        {
            fMaxMean = (float)dAvg;
            nMaxPos = nPos;
        }

        if (dAvg < fMinMean)
        {
            fMinMean = (float)dAvg;
            nMinPos = nPos;
        }

        i++;
    }

    if (bMaxPos)
    {
        if (fMaxMean > LONG_MIN)
            o_rtResult = rtRough + Ipvm::Point32s2(0, nMaxPos);
        else
            return FALSE;

        o_fResult = fMaxMean;
    }
    else
    {
        if (fMinMean < LONG_MAX)
            o_rtResult = rtRough + Ipvm::Point32s2(0, nMinPos);
        else
            return FALSE;

        o_fResult = fMinMean;
    }

    return TRUE;
}

BOOL VisionInspectionBgaBottom2D::IntensityXPos(const Ipvm::Image8u& image, Ipvm::Rect32s rtRough, long nSearchLength,
    Ipvm::Rect32s& o_rtResult, float& o_fResult, BOOL bMaxPos)
{
    o_fResult = FLT_MIN;

    float fSearchLength = (float)nSearchLength;
    long nHalfSearchLength = (long)(fSearchLength * 0.5f + 0.5f);
    float fMaxMean = FLT_MIN;
    float fMinMean = FLT_MAX;
    long nMaxPos = LONG_MIN;
    long nMinPos = LONG_MAX;

    Ipvm::Rect32s rtROI = rtRough;
    rtROI.InflateRect(nHalfSearchLength, 0);
    std::vector<BYTE> vecbyVal(rtROI.Width());

    if (Ipvm::Status::e_ok != Ipvm::ImageProcessing::MakeProjectionProfileAxisX(image, rtROI, &vecbyVal[0]))
    {
        return FALSE;
    }

    std::vector<double> vecdAvg(nHalfSearchLength * 2);
    long nRectWidth = rtRough.Width();
    long nDataNum = (long)vecbyVal.size() - nRectWidth;

    long i = 0;
    for (long nPos = -nHalfSearchLength; nPos < nHalfSearchLength; nPos++)
    {
        if (i > nDataNum)
        {
            ASSERT(0);
            return FALSE;
        }

        double dAvg = 0.f;
        for (long j = 0; j < nRectWidth; j++)
        {
            dAvg += (double)vecbyVal[i + j];
        }
        dAvg /= (double)nRectWidth;
        vecdAvg[i] = dAvg;

        if (dAvg > fMaxMean)
        {
            fMaxMean = (float)dAvg;
            nMaxPos = nPos;
        }

        if (dAvg < fMinMean)
        {
            fMinMean = (float)dAvg;
            nMinPos = nPos;
        }

        i++;
    }

    if (bMaxPos)
    {
        if (fMaxMean > LONG_MIN)
            o_rtResult = rtRough + Ipvm::Point32s2(nMaxPos, 0);
        else
            return FALSE;

        o_fResult = fMaxMean;
    }
    else
    {
        if (fMinMean < LONG_MAX)
            o_rtResult = rtRough + Ipvm::Point32s2(nMinPos, 0);
        else
            return FALSE;

        o_fResult = fMinMean;
    }

    return TRUE;
}

BOOL VisionInspectionBgaBottom2D::FindEdge(const Ipvm::Image8u& image, Ipvm::Point32r2 fptStart, Ipvm::Point32r2 fptEnd,
    BOOL bRisingEdge, BOOL bFirstEdge, Ipvm::Point32r2& o_fptEdge, float& o_fEdgeRadius)
{
    o_fEdgeRadius = 0.f;
    o_fptEdge = Ipvm::Point32r2(0.f, 0.f);
    double lfDX = fptEnd.m_x - fptStart.m_x;
    double lfDY = fptEnd.m_y - fptStart.m_y;
    double lfRadian = atan2(lfDY, lfDX);
    double lfDist = sqrt(lfDX * lfDX + lfDY * lfDY);

    long nDist = (long)(lfDist + .5f);
    float fCos = (float)cos(lfRadian);
    float fSin = (float)sin(lfRadian);

    CPoint ptFirstEdge = CPoint(0, 0);
    std::vector<BYTE> vecbyData(nDist);
    for (long n = 0; n < nDist; n++)
    {
        long x = (long)((float)n * fCos + fptStart.m_x + .5f);
        long y = (long)((float)n * fSin + fptStart.m_y + .5f);
        vecbyData[n] = image[y][x];
    }

    long nEdgeDir = PI_ED_DIR_FALLING;
    if (bRisingEdge)
        nEdgeDir = PI_ED_DIR_RISING;

    BOOL bSubPixelingDone;
    float fEdge, fEdgeVal;
    if (!m_pEdgeDetect->PI_ED_CalcEdgePointFromLine(
            nEdgeDir, nDist, &vecbyData[0], fEdge, fEdgeVal, bSubPixelingDone, bFirstEdge))
        return FALSE;
    o_fptEdge.m_x = fEdge * fCos + fptStart.m_x;
    o_fptEdge.m_y = fEdge * fSin + fptStart.m_y;
    o_fEdgeRadius = fEdge;

    return TRUE;
}
