//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionProcessingSideDetailAlign.h"

//CPP_2_________________________________ This project's headers
#include "DlgVisionProcessingSideDetailAlign.h"
#include "VisionProcessingSideDetailAlignPara.h"

//CPP_3_________________________________ Other projects' headers
#include "../../AlgorithmModules/dPI_EdgeDetect/dPI_EdgeDetectLib.h"
#include "../../DefineModules/dA_Base/ConversionEx.h"
#include "../../DefineModules/dA_Base/FPI_RECT.h"
#include "../../DefineModules/dA_Base/VisionScale.h"
#include "../../DefineModules/dA_Base/iDataType.h"
#include "../../DefineModules/dA_Base/semiinfo.h"
#include "../../ImageCombineModules/dPI_ippModules/ippModules.h"
#include "../../InformationModule/dPI_DataBase/BodyInfo.h"
#include "../../InformationModule/dPI_DataBase/PackageSpec.h"
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../../MemoryModules/VisionReusableMemory/SurfaceLayerMask.h"
#include "../../MemoryModules/VisionReusableMemory/VisionReusableMemory.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionBaseDef.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionImageLot.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionUnitAgent.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Algorithm/DataFitting.h>
#include <Ipvm/Algorithm/Geometry.h>
#include <Ipvm/Algorithm/ImageProcessing.h>
#include <Ipvm/Base/Conversion.h>
#include <Ipvm/Base/Image8u.h>
#include <Ipvm/Base/Polygon32r.h>
#include <Ipvm/Base/Quadrangle32r.h>
#include <Ipvm/Base/Rect32s.h>
#include <Ipvm/Base/TimeCheck.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define SUBSTRATE_LR_EDGE_GAP 3
#define NORMAL_N_GLASS_LR_EDGE_GAP 5
#define LINE_FIT_NOISE_FILTER_SIGMA 1.f

//CPP_7_________________________________ Implementation body
//
BOOL VisionProcessingSideDetailAlign::OnInspection()
{
    // 검사 진행
    m_pVisionInspDlg->OnBnClickedButtonInspect();
    return TRUE;
}

BOOL VisionProcessingSideDetailAlign::DoInspection(const bool detailSetupMode, const enSideVisionModule i_ModuleStatus)
{
    //mc_Side Vision
    SetCurVisionModule_Status(i_ModuleStatus);
    //

    //{{ //kircheis_3DEmpty //검사 결과를 관리하는 민감한 변수니까 무조건 시작할 때 초기화 한다.
    m_bInvalid = FALSE;
    m_bEmpty = FALSE;
    m_bMarginal = FALSE;
    m_bDoubleDevice = FALSE;
    m_bCoupon = FALSE;
    //}}

    const static long nVisionType = ::SystemConfig::GetInstance().GetVisionType();
    if (nVisionType != VISIONTYPE_SIDE_INSP)
        return FALSE;

    Ipvm::TimeCheck time_Insp;

    BOOL bResult = TRUE;

    // Init...
    ResetResult();
    ReSetDebugInfoMemberVariable();

    if (m_pVisionInspDlg != NULL && m_pVisionInspDlg->IsWindowVisible())
    {
        // Teaching

        m_pVisionInspDlg->m_imageLotView->Overlay_RemoveAll();
        m_pVisionInspDlg->m_imageLotViewRearSide->Overlay_RemoveAll();
    }

    // 검사 시작 ////////////////////////////////////////////
    //{{//여기에 영상 처리 추가
    const auto& substrateFrameIdx = m_VisionPara->m_substrateAlignParam.m_alignFrameIndex;
    auto& substrateImageProcPara = m_VisionPara->m_substrateAlignParam.m_alignImageProcManagePara;

    const auto& glassFrameIdx = m_VisionPara->m_glassAlignParam.m_alignFrameIndex;
    const auto& glassFrameIdxLR = m_VisionPara->m_glassAlignParam.m_alignFrameIndexGlassLR;
    //}}

    Ipvm::Image8u SubstrateImage = substrateFrameIdx.getImage(false);
    Ipvm::Image8u GlassImage = glassFrameIdx.getImage(false);
    Ipvm::Image8u GlassLRImage = glassFrameIdxLR.getImage(false);

    if (SubstrateImage.GetMem() == nullptr || GlassImage.GetMem() == nullptr || GlassLRImage.GetMem() == nullptr)
        return FALSE;

    Ipvm::Image8u combineSubstrateImage; //Glass Core가 아닐경우 이영상으로 Align

    if (!getReusableMemory().GetInspByteImage(combineSubstrateImage))
        return FALSE;

    if (!CippModules::GrayImageProcessingManage(
            getReusableMemory(), &SubstrateImage, false, substrateImageProcPara, combineSubstrateImage))
    {
        return FALSE;
    }

    bResult = DoInsp(detailSetupMode, combineSubstrateImage, GlassImage, GlassLRImage);
    SetDebugInfoMemberVariable(detailSetupMode);
    /////////////////////////////////////////////////////////

    m_fCalcTime = (float)time_Insp.Elapsed_ms();

    if (bResult == FALSE)
        m_bInvalid = TRUE;

    return bResult;
}

BOOL VisionProcessingSideDetailAlign::DoInsp(const bool detailSetupMode, const Ipvm::Image8u& i_substrateImage,
    const Ipvm::Image8u& i_glassImage, const Ipvm::Image8u& i_glassImageLR)
{
    return DoAlign(detailSetupMode, i_substrateImage, i_glassImage, i_glassImageLR);
}

BOOL VisionProcessingSideDetailAlign::DoAlign(const bool detailSetupMode, const Ipvm::Image8u& i_substrateImage,
    const Ipvm::Image8u& i_glassImage, const Ipvm::Image8u& i_glassImageLR)
{
    if (GetBodyAlignResult(m_sEdgeAlignResult) == FALSE)
        return FALSE;

    if (m_sEdgeAlignResult->bAvailable == FALSE)
        return FALSE;

    const auto& um2px = getScale().umToPixel();

    static const long nSideVisionNumber = (long)SystemConfig::GetInstance().GetSideVisionNumber();
    const auto& bodyInfoMaster = m_packageSpec.m_bodyInfoMaster;

    //{{Package Spec 정보 가져오기 --> Side V1이면 Picker가 90도 돌아 찍으니까 PackSpec의 SizeY들이 Side Vision 1에선 Size X다.
    const float fBodySizeXum
        = (nSideVisionNumber == SIDE_VISIONNUMBER_1) ? bodyInfoMaster->fBodySizeX : bodyInfoMaster->fBodySizeY;
    const BOOL bIsGlassCore = bodyInfoMaster->m_bGlassCorePackage;
    const float fSubstrateSizeXum = (bIsGlassCore == TRUE)
        ? ((nSideVisionNumber == SIDE_VISIONNUMBER_1) ? bodyInfoMaster->m_fGlassCoreSubstrateSizeX
                                                      : bodyInfoMaster->m_fGlassCoreSubstrateSizeY)
        : 0.f;
    //}}

    //{{ Align X Position 계산
    const long nEdgeCount = m_VisionPara->m_nEdgeCount;
    const Ipvm::Rect32r frtBody = m_sEdgeAlignResult->getBodyRect();
    const Ipvm::Rect32s rtBody = Ipvm::Conversion::ToRect32s(frtBody);

    std::vector<float> vecfSubstrateEdgePositionX(0); //일반 유닛 및 Glass Core Substrate용
    std::vector<float> vecfGlassEdgePositionX(0); //Substrate 유무에 관계 없이 Galss Core 용

    //원래 지역 변수 였으나 결과 Overlay Display를 위해 멤버 변수화
    m_vecfptNormalAlignPoint.resize(0);
    m_vecfptGlassAlignPoint.resize(0);
    m_vecfptTopSubstrateAlignPoint.resize(0);
    m_vecfptBottomSubstrateAlignPoint.resize(0);

    if (bIsGlassCore == FALSE) //일반 유닛이면
    {
        vecfSubstrateEdgePositionX.resize(nEdgeCount);
        GetDefaultPositionX_EachROI(rtBody, nEdgeCount, vecfSubstrateEdgePositionX);

        if (DoNormalUnitSideAlign(
                detailSetupMode, i_substrateImage, rtBody, vecfSubstrateEdgePositionX, m_vecfptNormalAlignPoint)
            == FALSE)
            return FALSE;
    }
    else //Glass Core 유닛
    {
        vecfGlassEdgePositionX.resize(nEdgeCount);
        GetDefaultPositionX_EachROI(rtBody, nEdgeCount, vecfGlassEdgePositionX);

        vecfSubstrateEdgePositionX.resize(nEdgeCount);
        float fGapPxl = (fBodySizeXum - fSubstrateSizeXum) * .5f * um2px.m_x;
        Ipvm::Rect32r frtSubstrate = frtBody;
        if (fSubstrateSizeXum > 0.f)
            frtSubstrate.DeflateRect(fGapPxl, 0.f);
        Ipvm::Rect32s rtSubstrate = Ipvm::Conversion::ToRect32s(frtSubstrate);
        GetDefaultPositionX_EachROI(rtSubstrate, nEdgeCount, vecfSubstrateEdgePositionX);

        if (DoGlassCoreUnitSideAlign(detailSetupMode, i_substrateImage, i_glassImage, i_glassImageLR, rtBody,
                vecfSubstrateEdgePositionX, vecfGlassEdgePositionX, m_vecfptTopSubstrateAlignPoint,
                m_vecfptBottomSubstrateAlignPoint, m_vecfptGlassAlignPoint)
            == FALSE)
            return FALSE;
    }
    //}}

    return TRUE;
}

BOOL VisionProcessingSideDetailAlign::DoNormalUnitSideAlign(const bool detailSetupMode,
    const Ipvm::Image8u& i_substrateImage, const Ipvm::Rect32s& i_rtPackage, const std::vector<float>& i_vecfPosXpx,
    std::vector<Ipvm::Point32r2>& o_vecptAlignPoint)
{
    o_vecptAlignPoint.clear();

    static const long nSideVisionNumber = (long)SystemConfig::GetInstance().GetSideVisionNumber();
    const auto& bodyInfoMaster = m_packageSpec.m_bodyInfoMaster;

    const BOOL bIsGlassCore = bodyInfoMaster->m_bGlassCorePackage;

    if (bIsGlassCore == TRUE)
        return FALSE;

    std::vector<float> vecfOffsetFromTopOutLinePxl(0);
    std::vector<float> vecfOffsetFromBottomOutLinePxl(0);
    const auto& alignParams = m_VisionPara->m_substrateAlignParam;
    //{{//DebugInfo SpecROI
    std::vector<Ipvm::Rect32s> vecrtSpecROI(1);
    vecrtSpecROI[0] = i_rtPackage;
    SetDebugInfoItem(detailSetupMode, DEBUGINFO_SDA_SpecROI, vecrtSpecROI);
    //}}

    BOOL bResult = RunSingleObjectAlign(detailSetupMode, i_substrateImage, i_substrateImage, i_rtPackage, i_vecfPosXpx,
        vecfOffsetFromTopOutLinePxl, vecfOffsetFromBottomOutLinePxl, alignParams, FALSE, o_vecptAlignPoint);

    if (bResult == TRUE)
    {
        SetDebugInfoItem(detailSetupMode, DEBUGINFO_SDA_NormalUnitAlignResult, o_vecptAlignPoint, true);
        SetSurfaceMask(o_vecptAlignPoint, _T("Side Detail Align Result"));
    }

    return bResult;
}

BOOL VisionProcessingSideDetailAlign::DoGlassCoreUnitSideAlign(const bool detailSetupMode,
    const Ipvm::Image8u& i_substrateImage, const Ipvm::Image8u& i_glassImage, const Ipvm::Image8u& i_glassImageLR,
    const Ipvm::Rect32s& i_rtPackage, const std::vector<float>& i_vecfSubstratePosXpx,
    const std::vector<float>& i_vecfGlassPosXpx, std::vector<Ipvm::Point32r2>& o_vecptTopAlignPoint,
    std::vector<Ipvm::Point32r2>& o_vecptBottomAlignPoint, std::vector<Ipvm::Point32r2>& o_vecptGlassAlignPoint)
{
    o_vecptTopAlignPoint.clear();
    o_vecptBottomAlignPoint.clear();
    o_vecptGlassAlignPoint.clear();
    const auto& um2px = getScale().umToPixel();

    static const long nSideVisionNumber = (long)SystemConfig::GetInstance().GetSideVisionNumber();
    const auto& bodyInfoMaster = m_packageSpec.m_bodyInfoMaster;

    const float fSubstrateSizeXum = (nSideVisionNumber == SIDE_VISIONNUMBER_1)
        ? bodyInfoMaster->m_fGlassCoreSubstrateSizeX
        : bodyInfoMaster->m_fGlassCoreSubstrateSizeY;
    const float fTopSubstrateThicknessUM = bodyInfoMaster->m_fGlassCoreTopSubstrateThickness;
    const float fBottomSubstrateThicknessUM = bodyInfoMaster->m_fGlassCoreBottomSubstrateThickness;

    const BOOL bIsGlassCore = bodyInfoMaster->m_bGlassCorePackage;
    if (bIsGlassCore == FALSE)
        return FALSE;

    const BOOL bIsExistSubstrate
        = (fSubstrateSizeXum > 0.f && fTopSubstrateThicknessUM > 0.f && fBottomSubstrateThicknessUM > 0.f);

    std::vector<float> vecfOffsetFromOutLine(0);
    const auto& substrateAlignParams = m_VisionPara->m_substrateAlignParam;
    const auto& glassAlignParams = m_VisionPara->m_glassAlignParam;

    BOOL bResult = TRUE;
    if (bIsExistSubstrate == FALSE) //Glass만 있는 경우 Glass Core Align만 진행하고 끝낸다.
    {
        std::vector<Ipvm::Rect32s> vecrtSpecROI(1);
        vecrtSpecROI[0] = i_rtPackage;
        SetDebugInfoItem(detailSetupMode, DEBUGINFO_SDA_SpecROI, vecrtSpecROI);

        bResult = RunSingleObjectAlign(detailSetupMode, i_glassImage, i_glassImageLR, i_rtPackage, i_vecfGlassPosXpx,
            vecfOffsetFromOutLine, vecfOffsetFromOutLine, glassAlignParams, TRUE, o_vecptGlassAlignPoint);

        if (bResult == TRUE)
        {
            SetDebugInfoItem(detailSetupMode, DEBUGINFO_SDA_GlassAlignResult, o_vecptGlassAlignPoint, true);
            SetSurfaceMask(o_vecptGlassAlignPoint, _T("Side Detail Align - Glass Core Result"));
        }

        return bResult;
    }
    //이하는 Substrate도 있는 경우. 없는 경우는 위에서 Return 해버려서 별도의 Else는 무쓸모
    SetDebugInfoMultiLayerSpecROI(detailSetupMode, i_rtPackage);

    std::vector<Ipvm::Point32r2> vecptTopSubstrateTopAlignPoint(0);
    std::vector<Ipvm::Point32r2> vecptTopSubstrateBottomAlignPoint(0);
    std::vector<Ipvm::Point32r2> vecptTopSubstrateLeftAlignPoint(0);
    std::vector<Ipvm::Point32r2> vecptTopSubstrateRightAlignPoint(0);

    std::vector<Ipvm::Point32r2> vecptBottomSubstrateTopAlignPoint(0);
    std::vector<Ipvm::Point32r2> vecptBottomSubstrateBottomAlignPoint(0);
    std::vector<Ipvm::Point32r2> vecptBottomSubstrateLeftAlignPoint(0);
    std::vector<Ipvm::Point32r2> vecptBottomSubstrateRightAlignPoint(0);

    //Top Substrate의 Top면, Bottom Substrate의 Bottom면 Edge를 추출한다.
    bResult = GetGlassCoreSubstrateTopBottomOuterEdgePoint(detailSetupMode, i_substrateImage, i_rtPackage,
        i_vecfSubstratePosXpx, vecptTopSubstrateTopAlignPoint, vecptBottomSubstrateBottomAlignPoint);
    AddSubstrateEdgePoint_TB(detailSetupMode, vecptTopSubstrateTopAlignPoint);
    AddSubstrateEdgePoint_TB(detailSetupMode, vecptBottomSubstrateBottomAlignPoint);
    if (bResult == FALSE)
        return FALSE;

    //Substrate Position X 좌표 기준의 Substrate-Glass Edge를 추출한다. 이떄 Rough Align을 하는 Option 이면 이를 수행한다. 이때는 Glass Align Param을 참조한다.
    const long nSubstrateInnerAlignParam = substrateAlignParams.m_nSubstrateInnerParamType;
    const auto& imageInnerAlign = (nSubstrateInnerAlignParam == USE_GLASS_PARAM) ? i_glassImage : i_substrateImage;
    bResult = GetGlassCoreSubstrateTopBottomInnerEdgePoint(detailSetupMode, imageInnerAlign, i_rtPackage,
        i_vecfSubstratePosXpx, vecptTopSubstrateBottomAlignPoint, vecptBottomSubstrateTopAlignPoint);

    AddSubstrateEdgePoint_TB(detailSetupMode, vecptTopSubstrateBottomAlignPoint);
    AddSubstrateEdgePoint_TB(detailSetupMode, vecptBottomSubstrateTopAlignPoint);
    if (bResult == FALSE)
        return FALSE;

    //Top&Bottom Substrate의 좌우 측면 Edge를 추출한다.
    long nTB_PointNum = (long)vecptTopSubstrateTopAlignPoint.size();

    Ipvm::Quadrangle32r qrtSubstrate;
    //Top-Left/Right
    qrtSubstrate = ConversionEx::ToQuadrangle32r(vecptTopSubstrateTopAlignPoint[0],
        vecptTopSubstrateTopAlignPoint[nTB_PointNum - 1], vecptTopSubstrateBottomAlignPoint[0],
        vecptTopSubstrateBottomAlignPoint[nTB_PointNum - 1]);
    bResult = GetGlassCoreSubstrateLeftRightEdgePoint(detailSetupMode, i_substrateImage, true, qrtSubstrate,
        vecptTopSubstrateLeftAlignPoint, vecptTopSubstrateRightAlignPoint);
    AddSubstrateEdgePoint_LR(detailSetupMode, vecptTopSubstrateLeftAlignPoint);
    AddSubstrateEdgePoint_LR(detailSetupMode, vecptTopSubstrateRightAlignPoint);
    if (bResult == FALSE)
        return FALSE;

    //Bottom-Left/Right
    qrtSubstrate = ConversionEx::ToQuadrangle32r(vecptBottomSubstrateTopAlignPoint[0],
        vecptBottomSubstrateTopAlignPoint[nTB_PointNum - 1], vecptBottomSubstrateBottomAlignPoint[0],
        vecptBottomSubstrateBottomAlignPoint[nTB_PointNum - 1]);
    bResult = GetGlassCoreSubstrateLeftRightEdgePoint(detailSetupMode, i_substrateImage, false, qrtSubstrate,
        vecptBottomSubstrateLeftAlignPoint, vecptBottomSubstrateRightAlignPoint);
    AddSubstrateEdgePoint_LR(detailSetupMode, vecptBottomSubstrateLeftAlignPoint);
    AddSubstrateEdgePoint_LR(detailSetupMode, vecptBottomSubstrateRightAlignPoint);
    if (bResult == FALSE)
        return FALSE;

    //좌우 측면 Edge를 참조하여 Top&Bottom Substrate의 Align 최종 결과를 만든다.
    if (ReAlignOutermostPoint(vecptTopSubstrateLeftAlignPoint, vecptTopSubstrateRightAlignPoint,
            vecptTopSubstrateTopAlignPoint, vecptTopSubstrateBottomAlignPoint)
        == FALSE)
        return FALSE;
    if (MergeTopBottomAlignPoint(
            vecptTopSubstrateTopAlignPoint, vecptTopSubstrateBottomAlignPoint, o_vecptTopAlignPoint)
        == FALSE)
        return FALSE;
    SetDebugInfoItem(detailSetupMode, DEBUGINFO_SDA_TopSubstrateAlignResult, o_vecptTopAlignPoint, true);

    if (ReAlignOutermostPoint(vecptBottomSubstrateLeftAlignPoint, vecptBottomSubstrateRightAlignPoint,
            vecptBottomSubstrateTopAlignPoint, vecptBottomSubstrateBottomAlignPoint)
        == FALSE)
        return FALSE;
    if (MergeTopBottomAlignPoint(
            vecptBottomSubstrateTopAlignPoint, vecptBottomSubstrateBottomAlignPoint, o_vecptBottomAlignPoint)
        == FALSE)
        return FALSE;
    SetDebugInfoItem(detailSetupMode, DEBUGINFO_SDA_BottomSubstrateAlignResult, o_vecptBottomAlignPoint, true);

    //Glass Align을 진행한다. 이떄 Rough Align을 하는 Option 이면 이를 수행한다.
    std::vector<float> vecfOffsetFromTopOutLinePxl(1);
    std::vector<float> vecfOffsetFromBottomOutLinePxl(1);
    vecfOffsetFromTopOutLinePxl[0] = (fTopSubstrateThicknessUM * um2px.m_y);
    vecfOffsetFromBottomOutLinePxl[0] = (fBottomSubstrateThicknessUM * um2px.m_y);
    if (RunSingleObjectAlign(detailSetupMode, i_glassImage, i_glassImageLR, i_rtPackage, i_vecfGlassPosXpx,
            vecfOffsetFromTopOutLinePxl, vecfOffsetFromBottomOutLinePxl, glassAlignParams, TRUE, o_vecptGlassAlignPoint)
        == FALSE)
        return FALSE;
    SetDebugInfoItem(detailSetupMode, DEBUGINFO_SDA_GlassAlignResult, o_vecptGlassAlignPoint, true);

    //전체 Unit에 대한 Align Point를 재구성한다 (Substrate Top-Top & Bottom-Bottom  + Glass Left Right)
    std::vector<Ipvm::Point32r2> vecfptLeft(2);
    std::vector<Ipvm::Point32r2> vecfptRight(2);
    std::vector<Ipvm::Point32r2> vecptWholeUnitAlignPoint(0);
    vecfptLeft[0] = o_vecptGlassAlignPoint[0];
    vecfptLeft[1] = o_vecptGlassAlignPoint[o_vecptGlassAlignPoint.size() - 1];
    vecfptRight[0] = o_vecptGlassAlignPoint[nTB_PointNum - 1];
    vecfptRight[1] = o_vecptGlassAlignPoint[nTB_PointNum];

    if (ReAlignOutermostPoint(
            vecfptLeft, vecfptRight, vecptTopSubstrateTopAlignPoint, vecptBottomSubstrateBottomAlignPoint)
        == FALSE)
        return FALSE;
    if (MergeTopBottomAlignPoint(
            vecptTopSubstrateTopAlignPoint, vecptBottomSubstrateBottomAlignPoint, vecptWholeUnitAlignPoint)
        == FALSE)
        return FALSE;

    SetDebugInfoItem(detailSetupMode, DEBUGINFO_SDA_NormalUnitAlignResult, vecptWholeUnitAlignPoint, true);

    SetSurfaceMask(vecptWholeUnitAlignPoint, _T("Side Detail Align Result"));
    SetSurfaceMask(o_vecptTopAlignPoint, _T("Side Detail Align - Top Substrate Result"));
    SetSurfaceMask(o_vecptGlassAlignPoint, _T("Side Detail Align - Glass Core Result"));
    SetSurfaceMask(o_vecptBottomAlignPoint, _T("Side Detail Align - Bottom Substrate Result"));

    return bResult;
}

BOOL VisionProcessingSideDetailAlign::GetGlassCoreSubstrateTopBottomOuterEdgePoint(const bool detailSetupMode,
    const Ipvm::Image8u& i_substrateImage, const Ipvm::Rect32s& i_rtPackage,
    const std::vector<float>& i_vecfSubstratePosXpx, std::vector<Ipvm::Point32r2>& o_vecptTopAlignPoint,
    std::vector<Ipvm::Point32r2>& o_vecptBottomAlignPoint)
{
    const auto& um2px = getScale().umToPixel();
    const auto& alignParams = m_VisionPara->m_substrateAlignParam;

    o_vecptTopAlignPoint.clear();
    o_vecptBottomAlignPoint.clear();

    //Params
    const long nSearchWidth = (long)(alignParams.m_nEdgeSearchWidthTB_um * um2px.m_x + .5f);
    const long nSearchLength = (long)(alignParams.m_nEdgeSearchLengthTB_um * um2px.m_y + .5f);
    const long nSearchDirectionTop = (long)(alignParams.m_nSearchDirection == PI_ED_DIR_INNER ? DOWN : UP);
    const long nSearchDirectionBottom = (long)(alignParams.m_nSearchDirection == PI_ED_DIR_OUTER ? DOWN : UP);
    const long nEdgeDirection = alignParams.m_nEdgeDirection;
    const float fEdgeThreshold = (float)alignParams.m_dEdgeThreshold;
    const BOOL bFirstEdge = (alignParams.m_nEdgeDetectMode == PI_ED_FIRST_EDGE);

    long nDataNum = (long)i_vecfSubstratePosXpx.size();
    BOOL bResult = FALSE;

    //ROI 구성
    std::vector<Ipvm::Rect32s> vecrtTopSearchROI(0);
    std::vector<Ipvm::Rect32s> vecrtBottomSearchROI(0);
    bResult = GetTopBottomEdgeSearchROI(
        i_rtPackage, i_vecfSubstratePosXpx, 0, 0, nSearchWidth, nSearchLength, vecrtTopSearchROI, vecrtBottomSearchROI);

    AddSubstrateEdgeSearchROI_TB(detailSetupMode, vecrtTopSearchROI);
    AddSubstrateEdgeSearchROI_TB(detailSetupMode, vecrtBottomSearchROI);
    //Edge 탐색
    std::vector<Ipvm::Point32r2> vecEdgePoint(0);
    for (long nIdx = 0; nIdx < nDataNum; nIdx++)
    {
        vecEdgePoint.clear();
        if (GetAllDirectionEdgePoints(i_substrateImage, vecrtTopSearchROI[nIdx], nSearchDirectionTop, nEdgeDirection,
                fEdgeThreshold, bFirstEdge, 0, FALSE, vecEdgePoint)
            == TRUE)
        {
            o_vecptTopAlignPoint.push_back(vecEdgePoint[0]);
        }
        else
            return FALSE;

        vecEdgePoint.clear();
        if (GetAllDirectionEdgePoints(i_substrateImage, vecrtBottomSearchROI[nIdx], nSearchDirectionBottom,
                nEdgeDirection, fEdgeThreshold, bFirstEdge, 0, FALSE, vecEdgePoint)
            == TRUE)
        {
            o_vecptBottomAlignPoint.push_back(vecEdgePoint[0]);
        }
        else
            return FALSE;
    }

    return TRUE;
}

BOOL VisionProcessingSideDetailAlign::GetGlassCoreSubstrateTopBottomInnerEdgePoint(const bool detailSetupMode,
    const Ipvm::Image8u& i_glassImage, const Ipvm::Rect32s& i_rtPackage,
    const std::vector<float>& i_vecfSubstratePosXpx, std::vector<Ipvm::Point32r2>& o_vecptTopAlignPoint,
    std::vector<Ipvm::Point32r2>& o_vecptBottomAlignPoint)
{
    const auto& um2px = getScale().umToPixel();
    const long nSubstrateInnerAlignParam = m_VisionPara->m_substrateAlignParam.m_nSubstrateInnerParamType;
    const auto& alignParams = (nSubstrateInnerAlignParam == USE_GLASS_PARAM) ? m_VisionPara->m_glassAlignParam
                                                                             : m_VisionPara->m_substrateAlignParam;

    o_vecptTopAlignPoint.clear();
    o_vecptBottomAlignPoint.clear();

    //Params
    const long nSearchWidth = (long)(alignParams.m_nEdgeSearchWidthTB_um * um2px.m_x + .5f);
    const long nSearchLength = (long)(alignParams.m_nEdgeSearchLengthTB_um * um2px.m_y + .5f);
    const long nSearchDirectionTop = (long)((nSubstrateInnerAlignParam == USE_GLASS_PARAM)
            ? (alignParams.m_nSearchDirection == PI_ED_DIR_INNER ? DOWN : UP)
            : (alignParams.m_nSearchDirection == PI_ED_DIR_INNER ? UP : DOWN));
    const long nSearchDirectionBottom = (long)((nSubstrateInnerAlignParam == USE_GLASS_PARAM)
            ? (alignParams.m_nSearchDirection == PI_ED_DIR_OUTER ? DOWN : UP)
            : (alignParams.m_nSearchDirection == PI_ED_DIR_OUTER ? UP : DOWN));
    const long nEdgeDirection = alignParams.m_nEdgeDirection;
    const float fEdgeThreshold = (float)alignParams.m_dEdgeThreshold;
    const BOOL bFirstEdge = (alignParams.m_nEdgeDetectMode == PI_ED_FIRST_EDGE);
    const BOOL bUseRoughAlign = (nSubstrateInnerAlignParam == USE_GLASS_PARAM) ? alignParams.m_bUseRoughAlign : FALSE;
    const long nRoughSearchWidth = (long)(alignParams.m_nRoughEdgeSearchWidthTB_um * um2px.m_x + .5f);
    const long nRoughSearchLength = (long)(alignParams.m_nRoughEdgeSearchLengthTB_um * um2px.m_y + .5f);

    const long nTopSubstrateThicknessPxl
        = (long)(m_packageSpec.m_bodyInfoMaster->m_fGlassCoreTopSubstrateThickness * um2px.m_y + .5f);
    const long nBottomSubstrateThicknessPxl
        = (long)(m_packageSpec.m_bodyInfoMaster->m_fGlassCoreBottomSubstrateThickness * um2px.m_y + .5f);

    long nDataNum = (long)i_vecfSubstratePosXpx.size();
    BOOL bResult = FALSE;

    std::vector<Ipvm::Rect32s> vecrtTopSearchROI(0);
    std::vector<Ipvm::Rect32s> vecrtBottomSearchROI(0);

    //ROI 구성
    //ROI 구성-Rough align
    if (bUseRoughAlign)
    {
        //ROI 구성-Rough align-ROI 구성
        bResult = GetTopBottomEdgeSearchROI(i_rtPackage, i_vecfSubstratePosXpx, (float)nTopSubstrateThicknessPxl,
            (float)nBottomSubstrateThicknessPxl, nRoughSearchWidth, nRoughSearchLength, vecrtTopSearchROI,
            vecrtBottomSearchROI);
        if (bResult == FALSE)
            return FALSE;
        AddSubstrateRoughEdgeSearchROI_TB(detailSetupMode, vecrtTopSearchROI);
        AddSubstrateRoughEdgeSearchROI_TB(detailSetupMode, vecrtBottomSearchROI);

        bResult = GetRoughAlignResultROI(detailSetupMode, i_glassImage, FALSE, vecrtTopSearchROI, vecrtBottomSearchROI);
    }
    else
        bResult = GetTopBottomEdgeSearchROI(i_rtPackage, i_vecfSubstratePosXpx, (float)nTopSubstrateThicknessPxl,
            (float)nBottomSubstrateThicknessPxl, nSearchWidth, nSearchLength, vecrtTopSearchROI, vecrtBottomSearchROI);

    AddSubstrateEdgeSearchROI_TB(detailSetupMode, vecrtTopSearchROI);
    AddSubstrateEdgeSearchROI_TB(detailSetupMode, vecrtBottomSearchROI);

    //Edge 탐색
    std::vector<Ipvm::Point32r2> vecEdgePoint(0);
    for (long nIdx = 0; nIdx < nDataNum; nIdx++)
    {
        vecEdgePoint.clear();
        if (GetAllDirectionEdgePoints(i_glassImage, vecrtTopSearchROI[nIdx], nSearchDirectionTop, nEdgeDirection,
                fEdgeThreshold, bFirstEdge, 0, FALSE, vecEdgePoint)
            == TRUE)
        {
            o_vecptTopAlignPoint.push_back(vecEdgePoint[0]);
        }
        else
            return FALSE;

        vecEdgePoint.clear();
        if (GetAllDirectionEdgePoints(i_glassImage, vecrtBottomSearchROI[nIdx], nSearchDirectionBottom, nEdgeDirection,
                fEdgeThreshold, bFirstEdge, 0, FALSE, vecEdgePoint)
            == TRUE)
        {
            o_vecptBottomAlignPoint.push_back(vecEdgePoint[0]);
        }
        else
            return FALSE;
    }

    return TRUE;
}

BOOL VisionProcessingSideDetailAlign::GetGlassCoreSubstrateLeftRightEdgePoint(const bool detailSetupMode,
    const Ipvm::Image8u& i_substrateImage, const bool i_bIsTop, const Ipvm::Quadrangle32r& i_qrtSubstrate,
    std::vector<Ipvm::Point32r2>& o_vecptLeftAlignPoint, std::vector<Ipvm::Point32r2>& o_vecptRightAlignPoint)
{
    const auto& um2px = getScale().umToPixel();
    const auto& alignParams = m_VisionPara->m_substrateAlignParam;

    o_vecptLeftAlignPoint.clear();
    o_vecptRightAlignPoint.clear();

    //Params
    const float fSearchWidthRatio = (float)((float)alignParams.m_nEdgeSearchWidthLR_Ratio * 0.01f);
    const long nSearchLengthHalf = (long)((float)alignParams.m_nEdgeSearchLengthLR_um * um2px.m_y * .5f + .5f);
    const long nSearchDirectionLeft = (long)(alignParams.m_nSearchDirection == PI_ED_DIR_INNER ? RIGHT : LEFT);
    const long nSearchDirectionRight = (long)(alignParams.m_nSearchDirection == PI_ED_DIR_INNER ? LEFT : RIGHT);
    const long nEdgeDirection = alignParams.m_nEdgeDirection;
    const float fEdgeThreshold = (float)alignParams.m_dEdgeThreshold;
    const BOOL bFirstEdge = (alignParams.m_nEdgeDetectMode == PI_ED_FIRST_EDGE);
    const float fTopSubstrateThicknessPxl
        = m_packageSpec.m_bodyInfoMaster->m_fGlassCoreTopSubstrateThickness * um2px.m_y;
    const float fBottomSubstrateThicknessPxl
        = m_packageSpec.m_bodyInfoMaster->m_fGlassCoreBottomSubstrateThickness * um2px.m_y;
    const float fSubstrateThicknessPxl = i_bIsTop ? fTopSubstrateThicknessPxl : fBottomSubstrateThicknessPxl;

    Ipvm::Rect32s rtSearchLeftROI{};
    Ipvm::Rect32s rtSearchRightROI{};

    //Left
    //이 구간은 Substrate의 상하 Align을 참조하여 측면 Align ROI를 만드는 알고리즘 --> 문제가 있는게 Substrate에 Damage가 있을 경우 Edge가 틀어져 Align ROI가 너무 작게 생성되는 문제가 있다.
    /*fptCenterPoint.m_x = (i_qrtSubstrate.m_ltX+ i_qrtSubstrate.m_lbX) * .5f;
	fptCenterPoint.m_y = (i_qrtSubstrate.m_ltY + i_qrtSubstrate.m_lbY) * .5f;
	MakeROI(fptCenterPoint, nSearchLengthHalf, nHalfWidth, rtSearchLeftROI);*/

    //이 구간은 Substrate의 Glass쪽 Align을 참조하여 Substrate Thickness로 측면 Align ROI를 만드는 알고리즘
    Ipvm::Point32r2 fptRefPoint = i_bIsTop ? Ipvm::Point32r2(i_qrtSubstrate.m_lbX, i_qrtSubstrate.m_lbY)
                                           : Ipvm::Point32r2(i_qrtSubstrate.m_ltX, i_qrtSubstrate.m_ltY);
    long nSubstrateThicknessPxl = CAST_LONG(fSubstrateThicknessPxl);
    MakeLeftRightROIByThickness(
        fptRefPoint, nSearchLengthHalf, nSubstrateThicknessPxl, fSearchWidthRatio, i_bIsTop, rtSearchLeftROI);

    AddSubstrateEdgeSearchROI_LR(detailSetupMode, rtSearchLeftROI, rtSearchLeftROI);
    if (GetAllDirectionEdgePoints(i_substrateImage, rtSearchLeftROI, nSearchDirectionLeft, nEdgeDirection,
            fEdgeThreshold, bFirstEdge, SUBSTRATE_LR_EDGE_GAP, FALSE, o_vecptLeftAlignPoint)
        == FALSE)
        return FALSE;

    //Right
    //이 구간은 Substrate의 상하 Align을 참조하여 측면 Align ROI를 만드는 알고리즘
    /*fptCenterPoint.m_x = (i_qrtSubstrate.m_rtX + i_qrtSubstrate.m_rbX) * .5f;
	fptCenterPoint.m_y = (i_qrtSubstrate.m_rtY + i_qrtSubstrate.m_rbY) * .5f;
	MakeROI(fptCenterPoint, nSearchLengthHalf, nHalfWidth, rtSearchRightROI);*/

    //이 구간은 Substrate의 Glass쪽 Align을 참조하여 Substrate Thickness로 측면 Align ROI를 만드는 알고리즘
    fptRefPoint = i_bIsTop ? Ipvm::Point32r2(i_qrtSubstrate.m_rbX, i_qrtSubstrate.m_rbY)
                           : Ipvm::Point32r2(i_qrtSubstrate.m_rtX, i_qrtSubstrate.m_rtY);
    nSubstrateThicknessPxl = CAST_LONG(fSubstrateThicknessPxl);
    MakeLeftRightROIByThickness(
        fptRefPoint, nSearchLengthHalf, nSubstrateThicknessPxl, fSearchWidthRatio, i_bIsTop, rtSearchRightROI);

    AddSubstrateEdgeSearchROI_LR(detailSetupMode, rtSearchLeftROI, rtSearchRightROI);
    if (GetAllDirectionEdgePoints(i_substrateImage, rtSearchRightROI, nSearchDirectionRight, nEdgeDirection,
            fEdgeThreshold, bFirstEdge, SUBSTRATE_LR_EDGE_GAP, FALSE, o_vecptRightAlignPoint)
        == FALSE)
        return FALSE;

    return TRUE;
}

BOOL VisionProcessingSideDetailAlign::GetRoughAlignResultROI(const bool detailSetupMode,
    const Ipvm::Image8u& i_glassImage, const BOOL& bIsGlass, std::vector<Ipvm::Rect32s>& io_vecrtTopSearchROI,
    std::vector<Ipvm::Rect32s>& io_vecrtBottomSearchROI)
{
    long nTopDataNum = (long)io_vecrtTopSearchROI.size();
    long nBottomDataNum = (long)io_vecrtBottomSearchROI.size();
    if (nTopDataNum <= 0 || nBottomDataNum <= 0 || nTopDataNum != nBottomDataNum)
        return FALSE;

    const auto& alignParams = m_VisionPara->m_glassAlignParam;

    if (alignParams.m_bUseRoughAlign == FALSE) //함수를 잘못 호출한거임. 안해도 되는건데 호출한 상황.
        return TRUE;

    const auto& um2px = getScale().umToPixel();
    static long nEdgeGap = 3;

    const long nSearchWidth = (long)(alignParams.m_nEdgeSearchWidthTB_um * um2px.m_x + .5f);
    const long nSearchLength = (long)(alignParams.m_nEdgeSearchLengthTB_um * um2px.m_y + .5f);
    const long nSearchDirectionTop = (long)(alignParams.m_nSearchDirection == PI_ED_DIR_INNER ? DOWN : UP);
    const long nSearchDirectionBottom = (long)(alignParams.m_nSearchDirection == PI_ED_DIR_OUTER ? DOWN : UP);
    const long nEdgeDirection = alignParams.m_nEdgeDirection;
    const float fEdgeThreshold = (float)alignParams.m_dEdgeThreshold;
    const BOOL bFirstEdge = (alignParams.m_nEdgeDetectMode == PI_ED_FIRST_EDGE);

    //Edge 탐색
    std::vector<Ipvm::Point32r2> vecEdgePoint(0);
    Ipvm::LineEq32r lineEqVer;
    std::vector<Ipvm::Point32r2> vecfptCenter(2);

    for (long nIdx = 0; nIdx < nTopDataNum; nIdx++)
    {
        vecfptCenter[0] = io_vecrtTopSearchROI[nIdx].CenterPoint32r();
        vecfptCenter[1] = io_vecrtBottomSearchROI[nIdx].CenterPoint32r();
        Ipvm::DataFitting::FitToLine(2, &vecfptCenter[0], lineEqVer);

        vecEdgePoint.clear();
        if (GetAllDirectionEdgePoints(i_glassImage, io_vecrtTopSearchROI[nIdx], nSearchDirectionTop, nEdgeDirection,
                fEdgeThreshold, bFirstEdge, nEdgeGap, TRUE, vecEdgePoint)
            == TRUE)
        {
            AddRoughEdgePoint_TB(detailSetupMode, bIsGlass, vecEdgePoint);

            if (RemakeEdgeSearchRoiByRoughAlign(
                    lineEqVer, vecEdgePoint, nSearchWidth, nSearchLength, io_vecrtTopSearchROI[nIdx])
                == FALSE)
                return FALSE;
        }
        else
            return FALSE;

        vecEdgePoint.clear();
        if (GetAllDirectionEdgePoints(i_glassImage, io_vecrtBottomSearchROI[nIdx], nSearchDirectionBottom,
                nEdgeDirection, fEdgeThreshold, bFirstEdge, nEdgeGap, TRUE, vecEdgePoint)
            == TRUE)
        {
            AddRoughEdgePoint_TB(detailSetupMode, bIsGlass, vecEdgePoint);

            if (RemakeEdgeSearchRoiByRoughAlign(
                    lineEqVer, vecEdgePoint, nSearchWidth, nSearchLength, io_vecrtBottomSearchROI[nIdx])
                == FALSE)
                return FALSE;
        }
        else
            return FALSE;
    }
    return TRUE;
}

BOOL VisionProcessingSideDetailAlign::RemakeEdgeSearchRoiByRoughAlign(const Ipvm::LineEq32r& i_lineEqVer,
    const std::vector<Ipvm::Point32r2>& i_vecfptPonit, const long& nDestWidth, const long& nDestLength,
    Ipvm::Rect32s& io_rtSrcDstSearchROI)
{
    long nPointNum = (long)i_vecfptPonit.size();
    if (nPointNum < 2)
        return FALSE;

    Ipvm::LineEq32r lineEqEdge;
    int32_t nUsedDataNum = 0;
    std::vector<Ipvm::Point32r2> vecptUsedPoint(nPointNum);

    switch (m_VisionPara->m_nEdgePointNoiseFilter)
    {
        case ENF_USE_GENERAL_FILTER:
            Ipvm::DataFitting::FitToLineRn(nPointNum, &i_vecfptPonit[0], LINE_FIT_NOISE_FILTER_SIGMA, lineEqEdge);
            break;
        case ENF_USE_RANSAC_FILTER:
            Ipvm::DataFitting::FitToLineRansac(
                nPointNum, &i_vecfptPonit[0], &vecptUsedPoint[0], nUsedDataNum, lineEqEdge, 3.f);
            break;
        case ENF_NOT_USE:
        default:
            Ipvm::DataFitting::FitToLine(nPointNum, &i_vecfptPonit[0], lineEqEdge);
            break;
    }

    Ipvm::Point32r2 fptCrossPoint;
    Ipvm::Geometry::GetCrossPoint(i_lineEqVer, lineEqEdge, fptCrossPoint);

    const float fHalfWidth = (float)nDestWidth * .5f;
    const float fHalfLength = (float)nDestLength * .5f;

    io_rtSrcDstSearchROI.m_left = (int32_t)(fptCrossPoint.m_x - fHalfWidth + .5f);
    io_rtSrcDstSearchROI.m_top = (int32_t)(fptCrossPoint.m_y - fHalfLength + .5f);
    io_rtSrcDstSearchROI.m_right = (int32_t)(fptCrossPoint.m_x + fHalfWidth + .5f);
    io_rtSrcDstSearchROI.m_bottom = (int32_t)(fptCrossPoint.m_y + fHalfLength + .5f);

    return TRUE;
}

BOOL VisionProcessingSideDetailAlign::GetBodyAlignResult(VisionAlignResult* o_pEdgeAlignResult)
{
    long nDataNum = 0;
    void* pData = m_visionUnit.GetVisionDebugInfo(
        m_visionUnit.GetVisionAlignProcessingModuleGUID(), _T("EDGE Align Result"), nDataNum);
    if (pData != nullptr && nDataNum > 0)
    {
        *o_pEdgeAlignResult = *(VisionAlignResult*)pData;
    }
    else
        return FALSE;

    return TRUE;
}

BOOL VisionProcessingSideDetailAlign::GetDefaultPositionX_EachROI(
    const Ipvm::Rect32s& i_rtPackage, const long& i_nPointNum, std::vector<float>& o_vecfPosXpxl)
{
    o_vecfPosXpxl.clear();
    const long nWidth = i_rtPackage.Width();
    if (nWidth < 10 || nWidth < i_nPointNum || i_nPointNum < 3)
        return FALSE;

    o_vecfPosXpxl.resize(i_nPointNum);
    long nDivide = i_nPointNum - 1;

    float fGap = (float)nWidth / (float)nDivide;
    o_vecfPosXpxl[0] = (float)i_rtPackage.m_left;
    o_vecfPosXpxl[nDivide] = (float)i_rtPackage.m_right;

    for (long nIdx = 1; nIdx < nDivide; nIdx++)
        o_vecfPosXpxl[nIdx] = o_vecfPosXpxl[0] + (fGap * (float)nIdx);

    return TRUE;
}

BOOL VisionProcessingSideDetailAlign::GetTopBottomEdgeSearchROI(const Ipvm::Rect32s& i_rtPackage,
    const std::vector<float>& i_vecfPosXpxl, const float& i_fOffsetFromTopOutLinePxl,
    const float& i_fOffsetFromBottomOutLinePxl, const long& i_nSearchWidthPxl, const long& i_nSearchLengthPxl,
    std::vector<Ipvm::Rect32s>& o_vecrtTopSearchROI,
    std::vector<Ipvm::Rect32s>& o_vecrtBottomSearchROI) // 이함수는 Offset의 vector를 생성 후 오버로딩 함수를 호출
{
    long nPointNum = (long)i_vecfPosXpxl.size();
    if (nPointNum <= 0)
        return FALSE;

    std::vector<float> vecfOffsetFromTopOutLinePxl(nPointNum);
    std::vector<float> vecfOffsetFromBottomOutLinePxl(nPointNum);

    for (long nIdx = 0; nIdx < nPointNum; nIdx++)
    {
        vecfOffsetFromTopOutLinePxl[nIdx] = i_fOffsetFromTopOutLinePxl;
        vecfOffsetFromBottomOutLinePxl[nIdx] = i_fOffsetFromBottomOutLinePxl;
    }

    return GetTopBottomEdgeSearchROI(i_rtPackage, i_vecfPosXpxl, vecfOffsetFromTopOutLinePxl,
        vecfOffsetFromBottomOutLinePxl, i_nSearchWidthPxl, i_nSearchLengthPxl, o_vecrtTopSearchROI,
        o_vecrtBottomSearchROI);
}

BOOL VisionProcessingSideDetailAlign::GetTopBottomEdgeSearchROI(const Ipvm::Rect32s& i_rtPackage,
    const std::vector<float>& i_vecfPosXpxl, const std::vector<float>& i_vecfOffsetFromTopOutLinePxl,
    const std::vector<float>& i_vecfOffsetFromBottomOutLinePxl, const long& i_nSearchWidthPxl,
    const long& i_nSearchLengthPxl, std::vector<Ipvm::Rect32s>& o_vecrtTopSearchROI,
    std::vector<Ipvm::Rect32s>& o_vecrtBottomSearchROI)
{
    o_vecrtTopSearchROI.clear();
    o_vecrtBottomSearchROI.clear();

    const long nDataNum = (long)i_vecfPosXpxl.size();
    const long nTopOffsetNum = (long)i_vecfOffsetFromTopOutLinePxl.size();
    const long nBottomOffsetNum = (long)i_vecfOffsetFromBottomOutLinePxl.size();

    if (nDataNum != nTopOffsetNum || nDataNum != nBottomOffsetNum)
        return FALSE;

    const long nTopResPosY = i_rtPackage.m_top;
    const long nBottomResPosY = i_rtPackage.m_bottom;

    Ipvm::Rect32s rtROI;
    Ipvm::Point32r2 fpRoiCenter;
    long nWidthHalf = (long)((float)i_nSearchWidthPxl * .5f + .5f);
    long nLengthHalf = (long)((float)i_nSearchLengthPxl * .5f + .5f);

    o_vecrtTopSearchROI.resize(nDataNum);
    o_vecrtBottomSearchROI.resize(nDataNum);

    for (long nIdx = 0; nIdx < nDataNum; nIdx++)
    {
        fpRoiCenter.m_x = i_vecfPosXpxl[nIdx];
        fpRoiCenter.m_y = (float)nTopResPosY + i_vecfOffsetFromTopOutLinePxl[nIdx];
        MakeROI(fpRoiCenter, nWidthHalf, nLengthHalf, rtROI);
        o_vecrtTopSearchROI[nIdx] = rtROI;

        fpRoiCenter.m_y = (float)nBottomResPosY - i_vecfOffsetFromBottomOutLinePxl[nIdx];
        MakeROI(fpRoiCenter, nWidthHalf, nLengthHalf, rtROI);
        o_vecrtBottomSearchROI[nIdx] = rtROI;
    }

    return TRUE;
}

BOOL VisionProcessingSideDetailAlign::MakeROI(
    const Ipvm::Point32r2& i_fptCenter, const long& i_nHalfX, const long& i_nHalfY, Ipvm::Rect32s& o_rtROI)
{
    o_rtROI = Ipvm::Rect32s(0, 0, 0, 0);
    if (i_nHalfX <= 0 || i_nHalfY <= 0)
        return FALSE;

    o_rtROI.m_left = (long)(i_fptCenter.m_x - (float)i_nHalfX + .5f);
    o_rtROI.m_top = (long)(i_fptCenter.m_y - (float)i_nHalfY + .5f);
    o_rtROI.m_right = (long)(i_fptCenter.m_x + (float)i_nHalfX + .5f);
    o_rtROI.m_bottom = (long)(i_fptCenter.m_y + (float)i_nHalfY + .5f);

    return TRUE;
}

BOOL VisionProcessingSideDetailAlign::MakeLeftRightROIByThickness(const Ipvm::Point32r2& i_fptRef,
    const long& i_nHalfSearchLengthPxl, const long& i_nThicknessPxl, const float& i_fSearchWidthRatio,
    const bool& i_bIsTop, Ipvm::Rect32s& o_rtROI) //Top Substrate에는 i_fptRef에 Bottom Point, Bottom에는 반대로
{
    //기준 위치에서 Thickness를 더하거나(Bottom), 빼고(Top) 중간 점을 구한다.
    const float fThicknessHalf = (i_bIsTop ? -i_nThicknessPxl : i_nThicknessPxl) * .5f;
    const float fRefY = i_fptRef.m_y;
    const float fRoiCenterY = fRefY + fThicknessHalf;

    //Thickness를 참조해서 ROI의 Height에 i_fSearchWidthRatio를 적용하여 실제 탐색 구간을 확인하고 이를 2등분하여 ROI 구성
    const float fSearchWidthHalf = CAST_FLOAT(fabs(fThicknessHalf) * i_fSearchWidthRatio);

    o_rtROI.m_left = (long)(i_fptRef.m_x - i_nHalfSearchLengthPxl + .5f);
    o_rtROI.m_top = (long)(fRoiCenterY - fSearchWidthHalf + .5f);
    o_rtROI.m_right = (long)(i_fptRef.m_x + i_nHalfSearchLengthPxl + .5f);
    o_rtROI.m_bottom = (long)(fRoiCenterY + fSearchWidthHalf + .5f);

    return TRUE;
}

BOOL VisionProcessingSideDetailAlign::GetAllDirectionEdgePoints(const Ipvm::Image8u& i_image,
    const Ipvm::Rect32s& i_rtSearchROI, const long& i_nSearchDirection, const long& i_nEdgeDirection,
    const float& i_fEdgeThreshold, const BOOL& i_bIsFirstEdge, const long& i_nEdgeGap,
    const BOOL& i_bNoNeedAllPointDetection, std::vector<Ipvm::Point32r2>& o_vecptEdgePoint)
{
    o_vecptEdgePoint.clear();
    long nSearchWidth(0);
    long nSearchLength(0);
    Ipvm::Point32s2 ptStart;

    if (GetEdgeSearchInfo(i_rtSearchROI, i_nSearchDirection, ptStart, nSearchWidth, nSearchLength) == FALSE)
        return FALSE;

    const BOOL bIsSinglePointDetection = (i_nEdgeGap <= 0) || (i_nEdgeGap >= (nSearchWidth) / 2);
    Ipvm::Point32r2 edgePoint;

    float fOldEdgeThreshold = m_pEdgeDetect->SetMininumThreshold(i_fEdgeThreshold);
    if (bIsSinglePointDetection) //단 한개의 Edge만을 찾는 경우
    {
        if (m_pEdgeDetect->PI_ED_CalcAvgEdgePointFromImage(i_nEdgeDirection, ptStart, i_nSearchDirection, nSearchLength,
                nSearchWidth, i_image, edgePoint, i_bIsFirstEdge)
            == TRUE)
        {
            o_vecptEdgePoint.push_back(edgePoint);
            m_pEdgeDetect->SetMininumThreshold(fOldEdgeThreshold);
        }
        else
        {
            m_pEdgeDetect->SetMininumThreshold(fOldEdgeThreshold);
            return FALSE;
        }
    }
    else //여려 개의 Edge를 찾는 경우
    {
        long nDetectedPointNum = 0;
        Ipvm::Point32s2 ptOffset;
        long nDetectNum = nSearchWidth / i_nEdgeGap;
        if (nDetectNum < 3)
            return FALSE;

        if (GetEdgeSearchInitInfo(i_rtSearchROI, i_nSearchDirection, i_nEdgeGap, ptStart, ptOffset) == FALSE)
        {
            m_pEdgeDetect->SetMininumThreshold(fOldEdgeThreshold);
            return FALSE;
        }

        for (long nIdx = 0; nIdx < nDetectNum; nIdx++)
        {
            if (m_pEdgeDetect->PI_ED_CalcAvgEdgePointFromImage(i_nEdgeDirection, ptStart, i_nSearchDirection,
                    nSearchLength, i_nEdgeGap, i_image, edgePoint, i_bIsFirstEdge)
                == TRUE)
            {
                o_vecptEdgePoint.push_back(edgePoint);
                nDetectedPointNum++;
            }
            else if (i_bNoNeedAllPointDetection == FALSE)
            {
                m_pEdgeDetect->SetMininumThreshold(fOldEdgeThreshold);
                return FALSE;
            }
            ptStart += ptOffset;
        }
        m_pEdgeDetect->SetMininumThreshold(fOldEdgeThreshold);
        if (nDetectedPointNum < 3)
            return FALSE;
    }

    return TRUE;
}

BOOL VisionProcessingSideDetailAlign::GetEdgeSearchInfo(const Ipvm::Rect32s& i_rtSearchROI,
    const long& i_nSearchDirection, Ipvm::Point32s2& o_ptStart, long& o_SearchWidth, long& o_SearchLength)
{
    const long nRoiWidth = (long)i_rtSearchROI.Width();
    const long nRoiHeight = (long)i_rtSearchROI.Height();

    if (nRoiWidth == 0 || nRoiHeight == 0)
        return FALSE;

    o_ptStart = i_rtSearchROI.CenterPoint();

    switch (i_nSearchDirection)
    {
        case LEFT:
            o_ptStart.m_x = i_rtSearchROI.m_left;
            o_SearchWidth = nRoiHeight;
            o_SearchLength = nRoiWidth;
            break;
        case RIGHT:
            o_ptStart.m_x = i_rtSearchROI.m_right;
            o_SearchWidth = nRoiHeight;
            o_SearchLength = nRoiWidth;
            break;
        case UP:
            o_ptStart.m_y = i_rtSearchROI.m_bottom;
            o_SearchWidth = nRoiWidth;
            o_SearchLength = nRoiHeight;
            break;
        case DOWN:
            o_ptStart.m_y = i_rtSearchROI.m_top;
            o_SearchWidth = nRoiWidth;
            o_SearchLength = nRoiHeight;
            break;
        default:
            return FALSE;
    }

    return TRUE;
}

BOOL VisionProcessingSideDetailAlign::GetEdgeSearchInitInfo(const Ipvm::Rect32s& i_rtSearchROI,
    const long& i_nSearchDirection, const long& i_nGapOffset, Ipvm::Point32s2& o_ptStart, Ipvm::Point32s2& o_ptOffset)
{
    switch (i_nSearchDirection)
    {
        case LEFT:
            o_ptStart = i_rtSearchROI.TopRight();
            break;
        case UP:
            o_ptStart = i_rtSearchROI.BottomLeft();
            break;
        case RIGHT:
        case DOWN:
            o_ptStart = i_rtSearchROI.TopLeft();
            break;
        default:
            return FALSE;
    }

    switch (i_nSearchDirection)
    {
        case LEFT:
        case RIGHT:
            o_ptOffset = Ipvm::Point32s2(0, i_nGapOffset);
            break;
        case UP:
        case DOWN:
            o_ptOffset = Ipvm::Point32s2(i_nGapOffset, 0);
            break;
        default:
            return FALSE;
    }

    return TRUE;
}

BOOL VisionProcessingSideDetailAlign::RunSingleObjectAlign(const bool detailSetupMode, const Ipvm::Image8u& i_image,
    const Ipvm::Image8u& i_imageLR, const Ipvm::Rect32s& i_rtPackage, const std::vector<float>& i_vecfPosXpx,
    const std::vector<float>& i_vecfOffsetFromTopOutLinePxl, const std::vector<float>& i_vecfOffsetFromBottomOutLinePxl,
    const SideAlignParameters& alignParams, const BOOL& bIsGlass, std::vector<Ipvm::Point32r2>& o_vecptAlignPoint)
{
    const auto& um2px = getScale().umToPixel();
    o_vecptAlignPoint.clear();

    //Params
    const long nSearchWidth = (long)(alignParams.m_nEdgeSearchWidthTB_um * um2px.m_x + .5f);
    const long nSearchLength = (long)(alignParams.m_nEdgeSearchLengthTB_um * um2px.m_y + .5f);
    const long nSearchDirectionTop = (long)(alignParams.m_nSearchDirection == PI_ED_DIR_INNER ? DOWN : UP);
    const long nSearchDirectionBottom = (long)(alignParams.m_nSearchDirection == PI_ED_DIR_OUTER ? DOWN : UP);
    const long nEdgeDirection = alignParams.m_nEdgeDirection;
    const float fEdgeThreshold = (float)alignParams.m_dEdgeThreshold;
    const BOOL bFirstEdge = (alignParams.m_nEdgeDetectMode == PI_ED_FIRST_EDGE);
    const BOOL bUseRoughAlign = alignParams.m_bUseRoughAlign;

    long nDataNum = (long)i_vecfPosXpx.size();
    if (nDataNum <= 0)
        return FALSE;

    //Make T/B Edge Search ROI
    std::vector<Ipvm::Rect32s> vecrtTopSearchROI(0);
    std::vector<Ipvm::Rect32s> vecrtBottomSearchROI(0);

    BOOL bResult = FALSE;
    const long nTopOffsetNum = (long)i_vecfOffsetFromTopOutLinePxl.size();
    const long nBottomOffsetNum = (long)i_vecfOffsetFromBottomOutLinePxl.size();
    if (nTopOffsetNum == 0 && nBottomOffsetNum == 0)
        bResult = GetTopBottomEdgeSearchROI(
            i_rtPackage, i_vecfPosXpx, 0, 0, nSearchWidth, nSearchLength, vecrtTopSearchROI, vecrtBottomSearchROI);
    else if (nTopOffsetNum == 1 && nBottomOffsetNum == 1)
        bResult = GetTopBottomEdgeSearchROI(i_rtPackage, i_vecfPosXpx, i_vecfOffsetFromTopOutLinePxl[0],
            i_vecfOffsetFromTopOutLinePxl[0], nSearchWidth, nSearchLength, vecrtTopSearchROI, vecrtBottomSearchROI);
    else if (nTopOffsetNum == nBottomOffsetNum)
        bResult = GetTopBottomEdgeSearchROI(i_rtPackage, i_vecfPosXpx, i_vecfOffsetFromTopOutLinePxl,
            i_vecfOffsetFromTopOutLinePxl, nSearchWidth, nSearchLength, vecrtTopSearchROI, vecrtBottomSearchROI);

    if (bResult == FALSE)
        return FALSE;

    if (bUseRoughAlign)
    {
        const long nRoughSearchWidth = (long)(alignParams.m_nRoughEdgeSearchWidthTB_um * um2px.m_x + .5f);
        const long nRoughSearchLength = (long)(alignParams.m_nRoughEdgeSearchLengthTB_um * um2px.m_y + .5f);
        float fOffsetTop = nTopOffsetNum >= 1 ? i_vecfOffsetFromTopOutLinePxl[0] : 0.f;
        float fOffsetBottom = nBottomOffsetNum >= 1 ? i_vecfOffsetFromBottomOutLinePxl[0] : 0.f;
        //ROI 구성-Rough align-ROI 구성
        bResult = GetTopBottomEdgeSearchROI(i_rtPackage, i_vecfPosXpx, fOffsetTop, fOffsetBottom, nRoughSearchWidth,
            nRoughSearchLength, vecrtTopSearchROI, vecrtBottomSearchROI);
        AddRoughEdgeSearchROI_TB(detailSetupMode, bIsGlass, vecrtTopSearchROI);
        AddRoughEdgeSearchROI_TB(detailSetupMode, bIsGlass, vecrtBottomSearchROI);

        if (bResult == FALSE)
            return FALSE;
        bResult = GetRoughAlignResultROI(detailSetupMode, i_image, bIsGlass, vecrtTopSearchROI,
            vecrtBottomSearchROI); //Rough Align ROI를 기반으로 detection 후 이를 기반으로 새로운 ROI를 생성
    }

    AddEdgeSearchROI_TB(detailSetupMode, bIsGlass, vecrtTopSearchROI);
    AddEdgeSearchROI_TB(detailSetupMode, bIsGlass, vecrtBottomSearchROI);

    if (bUseRoughAlign == TRUE && bResult == FALSE)
        return FALSE;

    //Detect T/B Edge Point
    std::vector<Ipvm::Point32r2> vecEdgePoint(0);
    std::vector<Ipvm::Point32r2> vecptTopEdgePoint(0);
    std::vector<Ipvm::Point32r2> vecptBottomEdgePoint(0);
    Ipvm::Rect32s rtSearchROI;
    bool bIsFirst = false;
    bool bIsLast = false;
    long nLastIdx = nDataNum - 1;
    long nMarginX = (long)(vecrtTopSearchROI[0].Width() / 4);
    for (long nIdx = 0; nIdx < nDataNum; nIdx++)
    {
        bIsFirst = (nIdx == 0) ? true : false;
        bIsLast = (nIdx == nLastIdx) ? true : false;

        rtSearchROI = vecrtTopSearchROI[nIdx];
        if (bIsFirst)
        {
            rtSearchROI.m_left = i_rtPackage.m_left;
            rtSearchROI.m_right += nMarginX;
        }
        else if (bIsLast)
        {
            rtSearchROI.m_left -= nMarginX;
            rtSearchROI.m_right = i_rtPackage.m_right;
        }
        vecEdgePoint.clear();
        if (GetAllDirectionEdgePoints(i_image, rtSearchROI, nSearchDirectionTop, nEdgeDirection, fEdgeThreshold,
                bFirstEdge, 0, FALSE, vecEdgePoint)
            == TRUE)
        {
            if (bIsFirst)
                vecEdgePoint[0].m_x = (float)rtSearchROI.m_left;
            else if (bIsLast)
                vecEdgePoint[0].m_x = (float)rtSearchROI.m_right;
            vecptTopEdgePoint.push_back(vecEdgePoint[0]);
        }

        rtSearchROI = vecrtBottomSearchROI[nIdx];
        if (bIsFirst)
        {
            rtSearchROI.m_left = i_rtPackage.m_left;
            rtSearchROI.m_right += nMarginX;
        }
        else if (bIsLast)
        {
            rtSearchROI.m_left -= nMarginX;
            rtSearchROI.m_right = i_rtPackage.m_right;
        }
        vecEdgePoint.clear();
        if (GetAllDirectionEdgePoints(i_image, rtSearchROI, nSearchDirectionBottom, nEdgeDirection, fEdgeThreshold,
                bFirstEdge, 0, FALSE, vecEdgePoint)
            == TRUE)
        {
            if (bIsFirst)
                vecEdgePoint[0].m_x = (float)rtSearchROI.m_left;
            else if (bIsLast)
                vecEdgePoint[0].m_x = (float)rtSearchROI.m_right;
            vecptBottomEdgePoint.push_back(vecEdgePoint[0]);
        }
    }

    AddEdgePoint_TB(detailSetupMode, bIsGlass, vecptTopEdgePoint);
    AddEdgePoint_TB(detailSetupMode, bIsGlass, vecptBottomEdgePoint);

    if (vecptTopEdgePoint.size() != nDataNum || vecptBottomEdgePoint.size() != nDataNum)
        return FALSE;

    //Make L/R Edge Search ROI
    float fSearchWidthLRratio = (float)alignParams.m_nEdgeSearchWidthLR_Ratio;
    float fSearchLengthLRum = (float)alignParams.m_nEdgeSearchLengthLR_um;
    Ipvm::Rect32s rtSearchLeftROI;
    Ipvm::Rect32s rtSearchRightROI;
    if (GetLeftRightSearchROI(i_rtPackage, fSearchWidthLRratio, fSearchLengthLRum, rtSearchLeftROI, rtSearchRightROI)
        == FALSE)
        return FALSE;

    AddEdgeSearchROI_LR(detailSetupMode, bIsGlass, rtSearchLeftROI, rtSearchRightROI);

    //Detect L/R Edge Point
    const long nEdgeDirectionLR = (bIsGlass) ? alignParams.m_nEdgeDirectionGlassLR : alignParams.m_nEdgeDirection;
    const long nSearchDirection = (bIsGlass) ? alignParams.m_nSearchDirectionGlassLR : alignParams.m_nSearchDirection;

    const long nSearchDirectionLeft = (long)(nSearchDirection == PI_ED_DIR_INNER ? RIGHT : LEFT);
    const long nSearchDirectionRight = (long)(nSearchDirection == PI_ED_DIR_INNER ? LEFT : RIGHT);

    std::vector<Ipvm::Point32r2> vecptLeftEdgePoint(0);
    std::vector<Ipvm::Point32r2> vecptRightEdgePoint(0);

    bResult = GetAllDirectionEdgePoints(i_imageLR, rtSearchLeftROI, nSearchDirectionLeft, nEdgeDirectionLR,
        fEdgeThreshold, bFirstEdge, NORMAL_N_GLASS_LR_EDGE_GAP, TRUE, vecptLeftEdgePoint);
    AddEdgePoint_LR(detailSetupMode, bIsGlass, vecptLeftEdgePoint);
    if (bResult == FALSE)
        return FALSE;

    bResult = GetAllDirectionEdgePoints(i_imageLR, rtSearchRightROI, nSearchDirectionRight, nEdgeDirectionLR,
        fEdgeThreshold, bFirstEdge, NORMAL_N_GLASS_LR_EDGE_GAP, TRUE, vecptRightEdgePoint);
    AddEdgePoint_LR(detailSetupMode, bIsGlass, vecptRightEdgePoint);
    if (bResult == FALSE)
        return FALSE;

    //T/B의 좌우 최외곽 Point 위치 조정
    if (ReAlignOutermostPoint(vecptLeftEdgePoint, vecptRightEdgePoint, vecptTopEdgePoint, vecptBottomEdgePoint)
        == FALSE)
        return FALSE;

    if (bIsGlass == TRUE && m_VisionPara->m_bUseGlassCompensationLeftRight == TRUE)
        CompensateForOrthogonalOutermostPoints(vecptTopEdgePoint, vecptBottomEdgePoint);

    if (MergeTopBottomAlignPoint(vecptTopEdgePoint, vecptBottomEdgePoint, o_vecptAlignPoint) == FALSE)
        return FALSE;

    return TRUE;
}

BOOL VisionProcessingSideDetailAlign::CompensateForOrthogonalOutermostPoints(
    std::vector<Ipvm::Point32r2>& io_vecptTopPoint, std::vector<Ipvm::Point32r2>& io_vecptBottomPoint)
{
    Ipvm::Point32r2 ptCompensated;
    Ipvm::Point32r2 ptBottomProjection;
    Ipvm::LineEq32r eqLine;
    float fDist = 0.f;
    float fDistX = 0.f;
    float fDistY = 0.f;
    float fAngleDeg = 0.f;
    long nDataNumTop = (long)io_vecptTopPoint.size();
    long nDataNumBtm = (long)io_vecptBottomPoint.size();

    if (nDataNumTop != nDataNumBtm)
        return FALSE;

    //Left
    //좌상단 2개 Point로 직선 Fitting 후 좌하단 첫 Point를 Projection
    Ipvm::DataFitting::FitToLine(2, &io_vecptTopPoint[0], eqLine); //Top의 1,2번째 Point로 Line을 만들어
    Ipvm::Geometry::GetFootOfPerpendicular(
        eqLine, io_vecptBottomPoint[0], ptBottomProjection); //Btm 첫 포인트를 Top Line에 Projection하고
    fDist = io_vecptTopPoint[0].m_x - ptBottomProjection.m_x; // 이 두 Point의 거리를 계산
    Ipvm::Geometry::GetDistance(io_vecptTopPoint[0], ptBottomProjection, fDistX);
    Ipvm::Geometry::GetDistance(io_vecptBottomPoint[0], ptBottomProjection, fDistY);
    fAngleDeg = (float)atan2f(fDistX, fDistY) / (float)PI * 180.f;
    fDist *= getScale().pixelToUm().m_x; //Pixel 단위를 um 단위로
    if (fAngleDeg > 2.f) //틀어진 각도가 2도 이상일 때
    {
        if (fDist > 0) //Top의 Outermost가 Bottom보다 안쪽이면
        {
            Ipvm::Geometry::GetFootOfPerpendicular(
                eqLine, io_vecptBottomPoint[0], ptCompensated); //더 바깥 쪽인 Btm Point를 Top Line에 Projection해서
            io_vecptTopPoint[0] = ptCompensated; //만들어진 Point를 새로운 Top 첫 포인트로 삼는다.
        }
        else //Top의 Outermost가 Bottom보다 바깥쪽이면
        {
            Ipvm::DataFitting::FitToLine(
                2, &io_vecptBottomPoint[0], eqLine); //Btm Line에 Projectoin 해야하니까 Btm Line 만들고
            Ipvm::Geometry::GetFootOfPerpendicular(
                eqLine, io_vecptTopPoint[0], ptCompensated); //더 바깥 쪽인 Top Point를 Btm Line에 Projection해서
            io_vecptBottomPoint[0] = ptCompensated; //만들어진 Point를 새로운 Btm 첫 포인트로 삼는다.
        }
    }

    //Right
    long nRightStarter = nDataNumBtm - 2;
    long nRightOuter = nDataNumBtm - 1;
    Ipvm::DataFitting::FitToLine(2, &io_vecptTopPoint[nRightStarter], eqLine); //Top의 1,2번째 Point로 Line을 만들어
    Ipvm::Geometry::GetFootOfPerpendicular(
        eqLine, io_vecptBottomPoint[nRightOuter], ptBottomProjection); //Btm 첫 포인트를 Top Line에 Projection하고
    fDist = io_vecptTopPoint[nRightOuter].m_x - ptBottomProjection.m_x; // 이 두 Point의 거리를 계산
    Ipvm::Geometry::GetDistance(io_vecptTopPoint[nRightOuter], ptBottomProjection, fDistX);
    Ipvm::Geometry::GetDistance(io_vecptBottomPoint[nRightOuter], ptBottomProjection, fDistY);
    fAngleDeg = (float)atan2f(fDistX, fDistY) / (float)PI * 180.f;
    fDist *= getScale().pixelToUm().m_x; //Pixel 단위를 um 단위로
    if (fAngleDeg > 2.f) //틀어진 각도가 2도 이상일 때
    {
        if (fDist < 0) //Top의 Outermost가 Bottom보다 안쪽이면
        {
            Ipvm::Geometry::GetFootOfPerpendicular(eqLine, io_vecptBottomPoint[nRightOuter],
                ptCompensated); //더 바깥 쪽인 Btm Point를 Top Line에 Projection해서
            io_vecptTopPoint[nRightOuter] = ptCompensated; //만들어진 Point를 새로운 Top 첫 포인트로 삼는다.
        }
        else //Top의 Outermost가 Bottom보다 바깥쪽이면
        {
            Ipvm::DataFitting::FitToLine(
                2, &io_vecptBottomPoint[nRightStarter], eqLine); //Btm Line에 Projectoin 해야하니까 Btm Line 만들고
            Ipvm::Geometry::GetFootOfPerpendicular(eqLine, io_vecptTopPoint[nRightOuter],
                ptCompensated); //더 바깥 쪽인 Top Point를 Btm Line에 Projection해서
            io_vecptBottomPoint[nRightOuter] = ptCompensated; //만들어진 Point를 새로운 Btm 첫 포인트로 삼는다.
        }
    }

    return TRUE;
}

BOOL VisionProcessingSideDetailAlign::GetLeftRightSearchROI(const Ipvm::Rect32s& i_rtPackage,
    const float& i_fSearchWidthLRratio, const float& i_fSearchLengthLRum, Ipvm::Rect32s& o_rtLeftSearchROI,
    Ipvm::Rect32s& o_rtRightSearchROI)
{
    const auto& um2px = getScale().umToPixel();

    Ipvm::Point32r2 fptRef;
    Ipvm::Point32s2 ptPackageCenter = i_rtPackage.CenterPoint();
    float fSearchWidthRatio = i_fSearchWidthLRratio * 0.01f;
    long nPackageHeight = i_rtPackage.Height();
    long nSearchHalfLength = (long)((i_fSearchLengthLRum * um2px.m_x * .5f) + .5f);
    long nSearchHalfWidth = (long)(((float)nPackageHeight * fSearchWidthRatio * .5f) + .5f);

    //Left ROI
    fptRef.m_x = (float)i_rtPackage.m_left;
    fptRef.m_y = (float)ptPackageCenter.m_y;
    if (MakeROI(fptRef, nSearchHalfLength, nSearchHalfWidth, o_rtLeftSearchROI) == FALSE)
        return FALSE;

    //Right ROI
    fptRef.m_x = (float)i_rtPackage.m_right;
    return MakeROI(fptRef, nSearchHalfLength, nSearchHalfWidth, o_rtRightSearchROI);
}

BOOL VisionProcessingSideDetailAlign::ReAlignOutermostPoint(const Ipvm::LineEq32r& lineEqLeft,
    const Ipvm::LineEq32r& lineEqRight, std::vector<Ipvm::Point32r2>& io_vecptTopPoint,
    std::vector<Ipvm::Point32r2>& io_vecptBottomPoint)
{
    Ipvm::LineEq32r lineEqTop, lineEqBottom;

    //Top&Bottom의 좌측 2개 Point로 Fitting하고
    Ipvm::DataFitting::FitToLine(2, &io_vecptTopPoint[0], lineEqTop);
    Ipvm::DataFitting::FitToLine(2, &io_vecptBottomPoint[0], lineEqBottom);

    //앞서 만든 Top&Bottom Line과 Left Line과의 교점을 찾아 좌측 첫번째 Point의 위치를 바꿔준다
    Ipvm::Geometry::GetCrossPoint(lineEqTop, lineEqLeft, io_vecptTopPoint[0]);
    Ipvm::Geometry::GetCrossPoint(lineEqBottom, lineEqLeft, io_vecptBottomPoint[0]);

    //Top&Bottom의 우측 2개 Point로 Fitting하고
    long nTopRightEndIdx = (long)io_vecptTopPoint.size() - 1;
    long nBottomRightEndIdx = (long)io_vecptBottomPoint.size() - 1;
    Ipvm::DataFitting::FitToLine(2, &io_vecptTopPoint[nTopRightEndIdx - 1], lineEqTop);
    Ipvm::DataFitting::FitToLine(2, &io_vecptBottomPoint[nBottomRightEndIdx - 1], lineEqBottom);

    //앞서 만든 Top&Bottom Line과 Right Line과의 교점을 찾아 좌측 첫번째 Point의 위치를 바꿔준다
    Ipvm::Geometry::GetCrossPoint(lineEqTop, lineEqRight, io_vecptTopPoint[nTopRightEndIdx]);
    Ipvm::Geometry::GetCrossPoint(lineEqBottom, lineEqRight, io_vecptBottomPoint[nBottomRightEndIdx]);

    return TRUE;
}

BOOL VisionProcessingSideDetailAlign::ReAlignOutermostPoint(const std::vector<Ipvm::Point32r2>& i_vecfptLeft,
    const std::vector<Ipvm::Point32r2>& i_vecfptRight, std::vector<Ipvm::Point32r2>& io_vecptTopPoint,
    std::vector<Ipvm::Point32r2>& io_vecptBottomPoint)
{
    Ipvm::LineEq32r lineEqLeft, lineEqRight;
    const int32_t nLeftSize = (int32_t)i_vecfptLeft.size();
    const int32_t nRightSize = (int32_t)i_vecfptRight.size();

    std::vector<Ipvm::Point32r2> vecptLeftUsedPoint(nLeftSize);
    std::vector<Ipvm::Point32r2> vecptRightUsedPoint(nRightSize);
    int32_t nUsedDataNum = 0;

    switch (m_VisionPara->m_nEdgePointNoiseFilter)
    {
        case ENF_USE_GENERAL_FILTER:
            Ipvm::DataFitting::FitToLineRn(nLeftSize, &i_vecfptLeft[0], LINE_FIT_NOISE_FILTER_SIGMA, lineEqLeft);
            Ipvm::DataFitting::FitToLineRn(nRightSize, &i_vecfptRight[0], LINE_FIT_NOISE_FILTER_SIGMA, lineEqRight);
            break;
        case ENF_USE_RANSAC_FILTER:
            Ipvm::DataFitting::FitToLineRansac(
                nLeftSize, &i_vecfptLeft[0], &vecptLeftUsedPoint[0], nUsedDataNum, lineEqLeft, 3.f);
            Ipvm::DataFitting::FitToLineRansac(
                nRightSize, &i_vecfptRight[0], &vecptRightUsedPoint[0], nUsedDataNum, lineEqRight, 3.f);
            break;
        case ENF_NOT_USE:
        default:
            Ipvm::DataFitting::FitToLine(nLeftSize, &i_vecfptLeft[0], lineEqLeft);
            Ipvm::DataFitting::FitToLine(nRightSize, &i_vecfptRight[0], lineEqRight);
            break;
    }

    if (ReAlignOutermostPoint(lineEqLeft, lineEqRight, io_vecptTopPoint, io_vecptBottomPoint) == FALSE)
        return FALSE;

    return TRUE;
}

BOOL VisionProcessingSideDetailAlign::MergeTopBottomAlignPoint(const std::vector<Ipvm::Point32r2>& i_vecptTopAlignPoint,
    const std::vector<Ipvm::Point32r2>& i_vecptBottomAlignPoint, std::vector<Ipvm::Point32r2>& o_vecptTotalAlignPoint)
{
    o_vecptTotalAlignPoint.clear();

    long nDataNum = (long)i_vecptTopAlignPoint.size();
    if (nDataNum != i_vecptBottomAlignPoint.size())
        return FALSE;

    for (long nIdx = 0; nIdx < nDataNum; nIdx++)
        o_vecptTotalAlignPoint.push_back(i_vecptTopAlignPoint[nIdx]);

    for (long nIdx = nDataNum - 1; nIdx >= 0; nIdx--)
        o_vecptTotalAlignPoint.push_back(i_vecptBottomAlignPoint[nIdx]);

    return TRUE;
}

void VisionProcessingSideDetailAlign::SetDebugInfoMultiLayerSpecROI(
    const bool detailSetupMode, const Ipvm::Rect32s& i_rtPackage)
{
    const auto& um2px = getScale().umToPixel();

    static const long nSideVisionNumber = (long)SystemConfig::GetInstance().GetSideVisionNumber();
    const auto& bodyInfoMaster = m_packageSpec.m_bodyInfoMaster;

    const float fBodySizeXum
        = (nSideVisionNumber == SIDE_VISIONNUMBER_1) ? bodyInfoMaster->fBodySizeX : bodyInfoMaster->fBodySizeY;
    const float fSubstrateSizeXum = (nSideVisionNumber == SIDE_VISIONNUMBER_1)
        ? bodyInfoMaster->m_fGlassCoreSubstrateSizeX
        : bodyInfoMaster->m_fGlassCoreSubstrateSizeY;
    const float fTopSubstrateThicknessUM = bodyInfoMaster->m_fGlassCoreTopSubstrateThickness;
    const float fBottomSubstrateThicknessUM = bodyInfoMaster->m_fGlassCoreBottomSubstrateThickness;

    std::vector<Ipvm::Rect32s> vecrtSpecROI(3);
    const long nTopSubstrateThickness = (long)(fTopSubstrateThicknessUM * um2px.m_y + .5f);
    const long nBottomSubstrateThickness = (long)(fBottomSubstrateThicknessUM * um2px.m_y + .5f);
    const long nEdgeGap = (long)((fBodySizeXum - fSubstrateSizeXum) * .5f * um2px.m_y + .5f);

    //vecrtSpecROI[0] = i_rtPackage;//일반적인 Glass Core Package 일때는 전체 Unit Spec ROI는 그리지 말자 오히려 헷갈린다.

    vecrtSpecROI[0] = i_rtPackage;
    vecrtSpecROI[0].DeflateRect(0, nTopSubstrateThickness, 0, nBottomSubstrateThickness);

    vecrtSpecROI[1] = i_rtPackage;
    vecrtSpecROI[1].DeflateRect(nEdgeGap, 0);
    vecrtSpecROI[1].m_bottom = vecrtSpecROI[1].m_top + nTopSubstrateThickness;

    vecrtSpecROI[2] = i_rtPackage;
    vecrtSpecROI[2].DeflateRect(nEdgeGap, 0);
    vecrtSpecROI[2].m_top = vecrtSpecROI[2].m_bottom - nBottomSubstrateThickness;

    SetDebugInfoItem(detailSetupMode, DEBUGINFO_SDA_SpecROI, vecrtSpecROI);
}

void VisionProcessingSideDetailAlign::ReSetDebugInfoMemberVariable()
{
    m_vecrtSubstrateRoughEdgeSearchROI_TB.clear();
    m_vecptSubstrateRoughEdgePoint_TB.clear();
    m_vecrtSubstrateEdgeSearchROI_TB.clear();
    m_vecptSubstrateEdgePoint_TB.clear();
    m_vecrtSubstrateEdgeSearchROI_LR.clear();
    m_vecptSubstrateEdgePoint_LR.clear();

    m_vecrtGlassRoughEdgeSearchROI_TB.clear();
    m_vecptGlassRoughEdgePoint_TB.clear();
    m_vecrtGlassEdgeSearchROI_TB.clear();
    m_vecptGlassEdgePoint_TB.clear();
    m_vecrtGlassEdgeSearchROI_LR.clear();
    m_vecptGlassEdgePoint_LR.clear();
}

void VisionProcessingSideDetailAlign::SetDebugInfoMemberVariable(const bool detailSetupMode)
{
    if (detailSetupMode == FALSE) //어차피 멤버로 선언된 DebugInfo용 변수들은 Setup UI가 아니면 쓸일이 없는 애들 이다.
        return;

    SetDebugInfoItem(detailSetupMode, DEBUGINFO_SDA_SubstrateRoughSearchROI_TB, m_vecrtSubstrateRoughEdgeSearchROI_TB);
    SetDebugInfoItem(detailSetupMode, DEBUGINFO_SDA_SubstrateRoughEdgePoint_TB, m_vecptSubstrateRoughEdgePoint_TB);
    SetDebugInfoItem(detailSetupMode, DEBUGINFO_SDA_SubstrateSearchROI_TB, m_vecrtSubstrateEdgeSearchROI_TB);
    SetDebugInfoItem(detailSetupMode, DEBUGINFO_SDA_SubstrateEdgePoint_TB, m_vecptSubstrateEdgePoint_TB);
    SetDebugInfoItem(detailSetupMode, DEBUGINFO_SDA_SubstrateSearchROI_LR, m_vecrtSubstrateEdgeSearchROI_LR);
    SetDebugInfoItem(detailSetupMode, DEBUGINFO_SDA_SubstrateEdgePoint_LR, m_vecptSubstrateEdgePoint_LR);

    SetDebugInfoItem(detailSetupMode, DEBUGINFO_SDA_GlassRoughSearchROI_TB, m_vecrtGlassRoughEdgeSearchROI_TB);
    SetDebugInfoItem(detailSetupMode, DEBUGINFO_SDA_GlassRoughEdgePoint_TB, m_vecptGlassRoughEdgePoint_TB);
    SetDebugInfoItem(detailSetupMode, DEBUGINFO_SDA_GlassSearchROI_TB, m_vecrtGlassEdgeSearchROI_TB);
    SetDebugInfoItem(detailSetupMode, DEBUGINFO_SDA_GlassEdgePoint_TB, m_vecptGlassEdgePoint_TB);
    SetDebugInfoItem(detailSetupMode, DEBUGINFO_SDA_GlassSearchROI_LR, m_vecrtGlassEdgeSearchROI_LR);
    SetDebugInfoItem(detailSetupMode, DEBUGINFO_SDA_GlassEdgePoint_LR, m_vecptGlassEdgePoint_LR);
}

void VisionProcessingSideDetailAlign::AddSubstrateRoughEdgeSearchROI_TB(
    const bool detailSetupMode, const std::vector<Ipvm::Rect32s>& vecrtSubstrateRoughEdgeSearchROI_TB)
{
    if (detailSetupMode == FALSE || vecrtSubstrateRoughEdgeSearchROI_TB.size() < 1)
        return;

    m_vecrtSubstrateRoughEdgeSearchROI_TB.insert(m_vecrtSubstrateRoughEdgeSearchROI_TB.end(),
        vecrtSubstrateRoughEdgeSearchROI_TB.begin(), vecrtSubstrateRoughEdgeSearchROI_TB.end());
}

void VisionProcessingSideDetailAlign::AddSubstrateRoughEdgePoint_TB(
    const bool detailSetupMode, const std::vector<Ipvm::Point32r2>& vecptSubstrateRoughEdgePoint_TB)
{
    if (detailSetupMode == FALSE || vecptSubstrateRoughEdgePoint_TB.size() < 1)
        return;

    m_vecptSubstrateRoughEdgePoint_TB.insert(m_vecptSubstrateRoughEdgePoint_TB.end(),
        vecptSubstrateRoughEdgePoint_TB.begin(), vecptSubstrateRoughEdgePoint_TB.end());
}

void VisionProcessingSideDetailAlign::AddSubstrateEdgeSearchROI_TB(
    const bool detailSetupMode, const std::vector<Ipvm::Rect32s>& vecrtSubstrateEdgeSearchROI_TB)
{
    if (detailSetupMode == FALSE || vecrtSubstrateEdgeSearchROI_TB.size() < 1)
        return;

    m_vecrtSubstrateEdgeSearchROI_TB.insert(m_vecrtSubstrateEdgeSearchROI_TB.end(),
        vecrtSubstrateEdgeSearchROI_TB.begin(), vecrtSubstrateEdgeSearchROI_TB.end());
}

void VisionProcessingSideDetailAlign::AddSubstrateEdgePoint_TB(
    const bool detailSetupMode, const std::vector<Ipvm::Point32r2>& vecptSubstrateEdgePoint_TB)
{
    if (detailSetupMode == FALSE || vecptSubstrateEdgePoint_TB.size() < 1)
        return;

    m_vecptSubstrateEdgePoint_TB.insert(
        m_vecptSubstrateEdgePoint_TB.end(), vecptSubstrateEdgePoint_TB.begin(), vecptSubstrateEdgePoint_TB.end());
}

void VisionProcessingSideDetailAlign::AddSubstrateEdgeSearchROI_LR(
    const bool detailSetupMode, const Ipvm::Rect32s& rtSubstrateLeftROI, const Ipvm::Rect32s& rtSubstrateRightROI)
{
    if (detailSetupMode == FALSE)
        return;

    m_vecrtSubstrateEdgeSearchROI_LR.push_back(rtSubstrateLeftROI);
    m_vecrtSubstrateEdgeSearchROI_LR.push_back(rtSubstrateRightROI);
}

void VisionProcessingSideDetailAlign::AddSubstrateEdgePoint_LR(
    const bool detailSetupMode, const std::vector<Ipvm::Point32r2>& vecptSubstrateEdgePoint_LR)
{
    if (detailSetupMode == FALSE || vecptSubstrateEdgePoint_LR.size() < 1)
        return;

    m_vecptSubstrateEdgePoint_LR.insert(
        m_vecptSubstrateEdgePoint_LR.end(), vecptSubstrateEdgePoint_LR.begin(), vecptSubstrateEdgePoint_LR.end());
}

void VisionProcessingSideDetailAlign::AddGlassRoughEdgeSearchROI_TB(
    const bool detailSetupMode, const std::vector<Ipvm::Rect32s>& vecrtGlassRoughEdgeSearchROI_TB)
{
    if (detailSetupMode == FALSE || vecrtGlassRoughEdgeSearchROI_TB.size() < 1)
        return;

    m_vecrtGlassRoughEdgeSearchROI_TB.insert(m_vecrtGlassRoughEdgeSearchROI_TB.end(),
        vecrtGlassRoughEdgeSearchROI_TB.begin(), vecrtGlassRoughEdgeSearchROI_TB.end());
}

void VisionProcessingSideDetailAlign::AddGlassRoughEdgePoint_TB(
    const bool detailSetupMode, const std::vector<Ipvm::Point32r2>& vecptGlassRoughEdgePoint_TB)
{
    if (detailSetupMode == FALSE || vecptGlassRoughEdgePoint_TB.size() < 1)
        return;

    m_vecptGlassRoughEdgePoint_TB.insert(
        m_vecptGlassRoughEdgePoint_TB.end(), vecptGlassRoughEdgePoint_TB.begin(), vecptGlassRoughEdgePoint_TB.end());
}

void VisionProcessingSideDetailAlign::AddGlassEdgeSearchROI_TB(
    const bool detailSetupMode, const std::vector<Ipvm::Rect32s>& vecrtGlassEdgeSearchROI_TB)
{
    if (detailSetupMode == FALSE || vecrtGlassEdgeSearchROI_TB.size() < 1)
        return;

    m_vecrtGlassEdgeSearchROI_TB.insert(
        m_vecrtGlassEdgeSearchROI_TB.end(), vecrtGlassEdgeSearchROI_TB.begin(), vecrtGlassEdgeSearchROI_TB.end());
}

void VisionProcessingSideDetailAlign::AddGlassEdgePoint_TB(
    const bool detailSetupMode, const std::vector<Ipvm::Point32r2>& vecptGlassEdgePoint_TB)
{
    if (detailSetupMode == FALSE || vecptGlassEdgePoint_TB.size() < 1)
        return;

    m_vecptGlassEdgePoint_TB.insert(
        m_vecptGlassEdgePoint_TB.end(), vecptGlassEdgePoint_TB.begin(), vecptGlassEdgePoint_TB.end());
}

void VisionProcessingSideDetailAlign::AddGlassEdgeSearchROI_LR(
    const bool detailSetupMode, const Ipvm::Rect32s& rtGlassLeftROI, const Ipvm::Rect32s& rtGlassRightROI)
{
    if (detailSetupMode == FALSE)
        return;

    m_vecrtGlassEdgeSearchROI_LR.push_back(rtGlassLeftROI);
    m_vecrtGlassEdgeSearchROI_LR.push_back(rtGlassRightROI);
}

void VisionProcessingSideDetailAlign::AddGlassEdgePoint_LR(
    const bool detailSetupMode, const std::vector<Ipvm::Point32r2>& vecptGlassEdgePoint_LR)
{
    if (detailSetupMode == FALSE || vecptGlassEdgePoint_LR.size() < 1)
        return;

    m_vecptGlassEdgePoint_LR.insert(
        m_vecptGlassEdgePoint_LR.end(), vecptGlassEdgePoint_LR.begin(), vecptGlassEdgePoint_LR.end());
}

void VisionProcessingSideDetailAlign::AddRoughEdgeSearchROI_TB(
    const bool detailSetupMode, const bool bIsGlass, const std::vector<Ipvm::Rect32s>& vecrtRoughEdgeSearchROI_TB)
{
    if (bIsGlass)
        AddGlassRoughEdgeSearchROI_TB(detailSetupMode, vecrtRoughEdgeSearchROI_TB);
    else
        AddSubstrateRoughEdgeSearchROI_TB(detailSetupMode, vecrtRoughEdgeSearchROI_TB);
}

void VisionProcessingSideDetailAlign::AddRoughEdgePoint_TB(
    const bool detailSetupMode, const bool bIsGlass, const std::vector<Ipvm::Point32r2>& vecptRoughEdgePoint_TB)
{
    if (bIsGlass)
        AddGlassRoughEdgePoint_TB(detailSetupMode, vecptRoughEdgePoint_TB);
    else
        AddSubstrateRoughEdgePoint_TB(detailSetupMode, vecptRoughEdgePoint_TB);
}

void VisionProcessingSideDetailAlign::AddEdgeSearchROI_TB(
    const bool detailSetupMode, const bool bIsGlass, const std::vector<Ipvm::Rect32s>& vecrtEdgeSearchROI_TB)
{
    if (bIsGlass)
        AddGlassEdgeSearchROI_TB(detailSetupMode, vecrtEdgeSearchROI_TB);
    else
        AddSubstrateEdgeSearchROI_TB(detailSetupMode, vecrtEdgeSearchROI_TB);
}

void VisionProcessingSideDetailAlign::AddEdgePoint_TB(
    const bool detailSetupMode, const bool bIsGlass, const std::vector<Ipvm::Point32r2>& vecptEdgePoint_TB)
{
    if (bIsGlass)
        AddGlassEdgePoint_TB(detailSetupMode, vecptEdgePoint_TB);
    else
        AddSubstrateEdgePoint_TB(detailSetupMode, vecptEdgePoint_TB);
}

void VisionProcessingSideDetailAlign::AddEdgeSearchROI_LR(
    const bool detailSetupMode, const bool bIsGlass, const Ipvm::Rect32s& rtLeftROI, const Ipvm::Rect32s& rtRightROI)
{
    if (bIsGlass)
        AddGlassEdgeSearchROI_LR(detailSetupMode, rtLeftROI, rtRightROI);
    else
        AddSubstrateEdgeSearchROI_LR(detailSetupMode, rtLeftROI, rtRightROI);
}

void VisionProcessingSideDetailAlign::AddEdgePoint_LR(
    const bool detailSetupMode, const bool bIsGlass, const std::vector<Ipvm::Point32r2>& vecptEdgePoint_LR)
{
    if (bIsGlass)
        AddGlassEdgePoint_LR(detailSetupMode, vecptEdgePoint_LR);
    else
        AddSubstrateEdgePoint_LR(detailSetupMode, vecptEdgePoint_LR);
}

BOOL VisionProcessingSideDetailAlign::SetSurfaceMask(
    const std::vector<Ipvm::Point32r2>& i_vecptAlignPoint, const CString& i_strMaskName)
{
    long nPointNum = (long)i_vecptAlignPoint.size();
    if (nPointNum < 3)
        return FALSE;

    Ipvm::Image8u imageMask;
    if (!getReusableMemory().GetInspByteImage(imageMask))
        return FALSE;
    imageMask.FillZero();

    Ipvm::Polygon32r polygon;
    polygon.SetVertices(&i_vecptAlignPoint[0], nPointNum); //필히 확인 필요
    Ipvm::ImageProcessing::Fill(polygon, 255, imageMask);

    auto* alignMask = getReusableMemory().AddSurfaceLayerMaskClass(i_strMaskName);
    if (alignMask != nullptr)
    {
        alignMask->Reset();
        alignMask->Add(imageMask);

        return TRUE;
    }

    return FALSE;
}

BOOL VisionProcessingSideDetailAlign::IsGlassCorePackage()
{
    const auto& bodyInfoMaster = m_packageSpec.m_bodyInfoMaster;

    return (bodyInfoMaster->m_bGlassCorePackage);
}