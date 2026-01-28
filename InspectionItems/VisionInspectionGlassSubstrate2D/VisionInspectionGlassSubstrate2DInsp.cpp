//CPP_0_________________________________ Precompiled header
#include "stdAfx.h"

//CPP_1_________________________________ Main header
#include "VisionInspectionGlassSubstrate2D.h"

//CPP_2_________________________________ This project's headers
#include "DlgVisionInspectionGlassSubstrate2D.h"
#include "VisionInspectionGlassSubstrate2DPara.h"

//CPP_3_________________________________ Other projects' headers
#include "../../AlgorithmModules/dPI_EdgeDetect/dPI_EdgeDetectLib.h"
#include "../../DefineModules/dA_Base/VisionScale.h"
#include "../../DefineModules/dA_Base/iDataType.h"
#include "../../DefineModules/dA_Base/semiinfo.h"
#include "../../ImageCombineModules/dPI_ippModules/ippModules.h"
#include "../../InformationModule/dPI_DataBase/PackageSpec.h"
#include "../../MemoryModules/VisionReusableMemory/SurfaceLayerRoi.h"
#include "../../MemoryModules/VisionReusableMemory/VisionReusableMemory.h"
#include "../../SharedCommunicationModules/VisionHostCommon/VisionHostBaseDef.h"
#include "../../UserInterfaceModules/ImageLotView/ImageLotView.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionAlignResult.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionUnitAgent.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Algorithm/DataFitting.h>
#include <Ipvm/Algorithm/Geometry.h>
#include <Ipvm/Base/Conversion.h>
#include <Ipvm/Base/Enum.h>
#include <Ipvm/Base/Image8u.h>
#include <Ipvm/Base/Rect32r.h>
#include <Ipvm/Base/Rect32s.h>
#include <Ipvm/Base/TimeCheck.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
bool sort_toptobottom(Ipvm::Rect32s _F, Ipvm::Rect32s _S)
{
    return _F.m_top < _S.m_top;
};
bool sort_lefttoright(Ipvm::Rect32s _F, Ipvm::Rect32s _S)
{
    return _F.m_left < _S.m_left;
};

void VisionInspectionGlassSubstrate2D::AbsoluteRectToImageRectByBodyCenter(Ipvm::Rect32s& rtROI)
{
    Ipvm::Rect32r frtBodyRect = GetBodyRect();
    AbsoluteRectToImageRectByBodyCenter(frtBodyRect, rtROI);
}

void VisionInspectionGlassSubstrate2D::AbsoluteRectToImageRectByBodyCenter(
    Ipvm::Rect32r frtBodyRect, Ipvm::Rect32s& rtROI)
{
    Ipvm::Point32r2 fptCenter = frtBodyRect.CenterPoint();
    rtROI.m_left += (long)(fptCenter.m_x + .5f);
    rtROI.m_top += (long)(fptCenter.m_y + .5f);
    rtROI.m_right += (long)(fptCenter.m_x + .5f);
    rtROI.m_bottom += (long)(fptCenter.m_y + .5f);
}

void VisionInspectionGlassSubstrate2D::ImageRectToAbsoluteRectByBodyCenter(Ipvm::Rect32s& rtROI)
{
    Ipvm::Rect32r frtBodyRect = GetBodyRect();
    ImageRectToAbsoluteRectByBodyCenter(frtBodyRect, rtROI);
}

void VisionInspectionGlassSubstrate2D::ImageRectToAbsoluteRectByBodyCenter(
    Ipvm::Rect32r frtBodyRect, Ipvm::Rect32s& rtROI)
{
    Ipvm::Point32r2 fptCenter = frtBodyRect.CenterPoint();
    rtROI.m_left -= (long)(fptCenter.m_x + .5f);
    rtROI.m_top -= (long)(fptCenter.m_y + .5f);
    rtROI.m_right -= (long)(fptCenter.m_x + .5f);
    rtROI.m_bottom -= (long)(fptCenter.m_y + .5f);
}

Ipvm::Rect32r VisionInspectionGlassSubstrate2D::GetBodyRect()
{
    Ipvm::Rect32r frtBodyRect = Ipvm::Rect32r(0.f, 0.f, 0.f, 0.f);

    long nDataNum = 0;
    void* pData = m_visionUnit.GetVisionDebugInfo(
        m_visionUnit.GetVisionAlignProcessingModuleGUID(), _T("EDGE Align Result"), nDataNum);
    if (pData != nullptr && nDataNum > 0)
    {
        *m_sEdgeAlignResult = *(VisionAlignResult*)pData;
        frtBodyRect = m_sEdgeAlignResult->getBodyRect();
    }

    return frtBodyRect;
}

BOOL VisionInspectionGlassSubstrate2D::OnInspection()
{
    m_pVisionInspDlg->OnBnClickedButtonInspect();

    return TRUE;
}

BOOL VisionInspectionGlassSubstrate2D::DoInspection(const bool detailSetupMode, const enSideVisionModule i_ModuleStatus)
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
    if (m_pVisionInspDlg != NULL && m_pVisionInspDlg->IsWindowVisible())
    {
        // Teaching
        if (detailSetupMode)
        {
            m_pVisionInspDlg->GetROI();
        }

        m_pVisionInspDlg->m_imageLotView->Overlay_RemoveAll();
    }

    if (!DoAlignGlassSubstrate(detailSetupMode))
        return FALSE;

    bResult &= DoInspGlassSubstrateOffsetX();
    bResult &= DoInspGlassSubstrateOffsetY();
    bResult &= DoInspGlassSubstrateOffsetTheta();
    bResult &= DoInspGlassSubstrateSizeX();
    bResult &= DoInspGlassSubstrateSizeY();

    m_fCalcTime = CAST_FLOAT(time_Insp.Elapsed_ms());

    return bResult;
}

BOOL VisionInspectionGlassSubstrate2D::CalcAlignedCenterLine(
    VisionAlignResult i_AlignResult, Ipvm::LineEq32r& o_HorLine, Ipvm::LineEq32r& o_VerLine)
{
    if (i_AlignResult.bAvailable == FALSE)
        return FALSE;

    Ipvm::LineEq32r lineLT, lineRB;
    std::vector<Ipvm::Point32r2> vecfptPoint(2);

    //Hor Line
    vecfptPoint[0] = i_AlignResult.fptLT;
    vecfptPoint[1] = i_AlignResult.fptRT;
    Ipvm::DataFitting::FitToLineRn(2, &vecfptPoint[0], 3.f, lineLT);

    vecfptPoint[0] = i_AlignResult.fptLB;
    vecfptPoint[1] = i_AlignResult.fptRB;
    Ipvm::DataFitting::FitToLineRn(2, &vecfptPoint[0], 3.f, lineRB);

    Ipvm::Geometry::GetBisection(lineLT, lineRB, o_HorLine);

    //Ver Line
    vecfptPoint[0] = i_AlignResult.fptLT;
    vecfptPoint[1] = i_AlignResult.fptLB;
    Ipvm::DataFitting::FitToLineRn(2, &vecfptPoint[0], 3.f, lineLT);

    vecfptPoint[0] = i_AlignResult.fptRT;
    vecfptPoint[1] = i_AlignResult.fptRB;
    Ipvm::DataFitting::FitToLineRn(2, &vecfptPoint[0], 3.f, lineRB);

    Ipvm::Geometry::GetBisection(lineLT, lineRB, o_VerLine);

    return TRUE;
}

BOOL VisionInspectionGlassSubstrate2D::DoAlignGlassSubstrate(const bool detailSetupMode)
{
    m_vecsGlassSubstrateAlignResult.clear();
    m_veclineGlassSubstrateCenterHor.clear();
    m_veclineGlassSubstrateCenterVer.clear();

    if (IsValidGlassSubstrateSpecInfo() == FALSE)
        return FALSE;

    //검사 영상 준비
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

    //Package Align 정보 가져오기
    Ipvm::Rect32r frtBody = GetBodyRect();
    if (frtBody.Width() <= 0.f || frtBody.Height() <= 0.f)
        return FALSE;

    CalcAlignedCenterLine(*m_sEdgeAlignResult, m_linePackageCenterHor, m_linePackageCenterVer);

    //Debug Info 용 변수 확보
    std::vector<Ipvm::Rect32s> vecrtSpecROI(0);
    std::vector<Ipvm::Rect32s> vecrtSearchROI(0);
    std::vector<Ipvm::Point32r2> vecfptDebugLeftEdge(0);
    std::vector<Ipvm::Point32r2> vecfptDebugTopEdge(0);
    std::vector<Ipvm::Point32r2> vecfptDebugRightEdge(0);
    std::vector<Ipvm::Point32r2> vecfptDebugBottomEdge(0);
    std::vector<Ipvm::Rect32r> vecfrtAlignResult(0);
    std::vector<PI_RECT> vecprtAlignResult(0);

    //GlassSubstrate 정보 구성
    if (GetGlassSubstrateSpecROI(m_rtGlassSubstrateSpecROI) == FALSE)
        return FALSE;

    //실제 Align 구간
    std::vector<Ipvm::Rect32s> vecrtEdgeSearchROI(4);
    std::vector<Ipvm::Point32r2> vecfptLeftEdge(0);
    std::vector<Ipvm::Point32r2> vecfptTopEdge(0);
    std::vector<Ipvm::Point32r2> vecfptRightEdge(0);
    std::vector<Ipvm::Point32r2> vecfptBottomEdge(0);
    BOOL bAlignResult = TRUE;
    Ipvm::LineEq32r lineHor, lineVer;

    VisionAlignResult glassSubstrateAlignResult;
    PI_RECT piAlignRect;
    glassSubstrateAlignResult.bAvailable = FALSE;

    //Make Search ROI
    MakeEdgeSearchROI(m_rtGlassSubstrateSpecROI, vecrtEdgeSearchROI);
    vecrtSpecROI.push_back(m_rtGlassSubstrateSpecROI);
    vecrtSearchROI.push_back(vecrtEdgeSearchROI[LEFT]); //Align ROI의 순서를 맞추기 위해 insert를 안쓴거임
    vecrtSearchROI.push_back(vecrtEdgeSearchROI[UP]);
    vecrtSearchROI.push_back(vecrtEdgeSearchROI[RIGHT]);
    vecrtSearchROI.push_back(vecrtEdgeSearchROI[DOWN]);

    //Edge Detect Left
    GetGlassSubstrateEdgePoint(combineImage, LEFT, vecrtEdgeSearchROI[LEFT], vecfptLeftEdge);
    vecfptDebugLeftEdge.insert(vecfptDebugLeftEdge.end(), vecfptLeftEdge.begin(), vecfptLeftEdge.end());
    if (vecfptLeftEdge.size() <= 3)
        bAlignResult = FALSE;

    //Edge Detect Top
    GetGlassSubstrateEdgePoint(combineImage, UP, vecrtEdgeSearchROI[UP], vecfptTopEdge);
    vecfptDebugTopEdge.insert(vecfptDebugTopEdge.end(), vecfptTopEdge.begin(), vecfptTopEdge.end());
    if (vecfptTopEdge.size() <= 3)
        bAlignResult = FALSE;

    //Edge Detect Right
    GetGlassSubstrateEdgePoint(combineImage, RIGHT, vecrtEdgeSearchROI[RIGHT], vecfptRightEdge);
    vecfptDebugRightEdge.insert(vecfptDebugRightEdge.end(), vecfptRightEdge.begin(), vecfptRightEdge.end());
    if (vecfptRightEdge.size() <= 3)
        bAlignResult = FALSE;

    //Edge Detect bottom
    GetGlassSubstrateEdgePoint(combineImage, DOWN, vecrtEdgeSearchROI[DOWN], vecfptBottomEdge);
    vecfptDebugBottomEdge.insert(vecfptDebugBottomEdge.end(), vecfptBottomEdge.begin(), vecfptBottomEdge.end());
    if (vecfptBottomEdge.size() <= 3)
        bAlignResult = FALSE;

    //Line Fitting and Make Cross Point

    if (CalcGlassSubstrateAlignResult(
            vecfptLeftEdge, vecfptTopEdge, vecfptRightEdge, vecfptBottomEdge, glassSubstrateAlignResult))
    {
        m_vecsGlassSubstrateAlignResult.push_back(glassSubstrateAlignResult);

        vecfrtAlignResult.push_back(glassSubstrateAlignResult.getBodyRect());
        piAlignRect.SetLTPoint(Ipvm::Conversion::ToPoint32s2(glassSubstrateAlignResult.fptLT));
        piAlignRect.SetRTPoint(Ipvm::Conversion::ToPoint32s2(glassSubstrateAlignResult.fptRT));
        piAlignRect.SetRBPoint(Ipvm::Conversion::ToPoint32s2(glassSubstrateAlignResult.fptRB));
        piAlignRect.SetLBPoint(Ipvm::Conversion::ToPoint32s2(glassSubstrateAlignResult.fptLB));
        vecprtAlignResult.push_back(piAlignRect);

        CalcAlignedCenterLine(glassSubstrateAlignResult, lineHor, lineVer);
        m_veclineGlassSubstrateCenterHor.push_back(lineHor);
        m_veclineGlassSubstrateCenterVer.push_back(lineVer);
    }
    else
        bAlignResult = FALSE;

    vecfptLeftEdge.clear();
    vecfptTopEdge.clear();
    vecfptRightEdge.clear();
    vecfptBottomEdge.clear();

    SetDebugInfoItem(detailSetupMode, _T("Glass Substrate Spec"), vecrtSpecROI);
    SetDebugInfoItem(detailSetupMode, _T("Glass Substrate Edge Search ROI"), vecrtSearchROI);
    SetDebugInfoItem(detailSetupMode, _T("Glass Substrate Edge Point Left"), vecfptDebugLeftEdge);
    SetDebugInfoItem(detailSetupMode, _T("Glass Substrate Edge Point Top"), vecfptDebugTopEdge);
    SetDebugInfoItem(detailSetupMode, _T("Glass Substrate Edge Point Right"), vecfptDebugRightEdge);
    SetDebugInfoItem(detailSetupMode, _T("Glass Substrate Edge Point Bottom"), vecfptDebugBottomEdge);
    SetDebugInfoItem(detailSetupMode, _T("Detect Glass Substrate Rect"), vecfrtAlignResult, TRUE);

    //{{//kircheis_USI_OA
    auto* psDebugInfo = m_DebugInfoGroup.GetDebugInfo(_T("Detect Glass Substrate ROI"));
    if (psDebugInfo != NULL)
    {
        if (psDebugInfo->pData != NULL)
        {
            psDebugInfo->Reset();
        }
        long nLayoutCount = (long)vecprtAlignResult.size();
        psDebugInfo->nDataNum = nLayoutCount;
        PI_RECT* pPoint = new PI_RECT[psDebugInfo->nDataNum];
        for (long i = 0; i < psDebugInfo->nDataNum; i++)
        {
            pPoint[i] = vecprtAlignResult[i];
        }
        psDebugInfo->pData = pPoint;
    }

    for (long i = 0; i < long(vecprtAlignResult.size()); i++)
    {
        Ipvm::Quadrangle32r quadRangle;
        quadRangle.m_ltX = (float)vecprtAlignResult[i].ltX;
        quadRangle.m_ltY = (float)vecprtAlignResult[i].ltY;
        quadRangle.m_rtX = (float)vecprtAlignResult[i].rtX;
        quadRangle.m_rtY = (float)vecprtAlignResult[i].rtY;
        quadRangle.m_lbX = (float)vecprtAlignResult[i].blX;
        quadRangle.m_lbY = (float)vecprtAlignResult[i].blY;
        quadRangle.m_rbX = (float)vecprtAlignResult[i].brX;
        quadRangle.m_rbY = (float)vecprtAlignResult[i].brY;

        CString name;
        name.Format(_T("Glass Substrate Align Result"));

        if (auto* surfaceROI = getReusableMemory().AddSurfaceLayerRoiClass(name))
        {
            surfaceROI->Reset();
            surfaceROI->Add(quadRangle);
        }
        name.Empty();
    }

    //}}
    return bAlignResult;
}

BOOL VisionInspectionGlassSubstrate2D::IsValidGlassSubstrateSpecInfo()
{
    const auto& bodyInfoMaster = m_packageSpec.m_bodyInfoMaster;
    if (bodyInfoMaster->m_bGlassCorePackage == FALSE) //Glass Core Package가 아니면 할 필요가 없다
        return FALSE;
    if (bodyInfoMaster->m_fGlassCoreSubstrateSizeX <= 0.f
        || bodyInfoMaster->m_fGlassCoreSubstrateSizeY <= 0.f) //Glass Core여도 Substrate가 없으면 할 필요 없다
        return FALSE;

    return TRUE;
}

BOOL VisionInspectionGlassSubstrate2D::GetGlassSubstrateSpecROI(Ipvm::Rect32s& o_rtSpecROI)
{
    Ipvm::Rect32r frtBody = GetBodyRect();
    if (frtBody.Width() <= 0.f || frtBody.Height() <= 0.f)
        return FALSE;

    if (IsValidGlassSubstrateSpecInfo() == FALSE)
        return FALSE;

    const auto& bodyInfoMaster = m_packageSpec.m_bodyInfoMaster;

    const auto& um2px = getScale().umToPixel();
    const Ipvm::Point32r2 fptBodyCenter = frtBody.CenterPoint();

    const float fGlassSubstrateHalfSizeXpxl = (float)(bodyInfoMaster->m_fGlassCoreSubstrateSizeX * um2px.m_x * .5f);
    const float fGlassSubstrateHalfSizeYpxl = (float)(bodyInfoMaster->m_fGlassCoreSubstrateSizeY * um2px.m_y * .5f);

    o_rtSpecROI.m_left = (long)(fptBodyCenter.m_x - fGlassSubstrateHalfSizeXpxl + .5f);
    o_rtSpecROI.m_right = (long)(fptBodyCenter.m_x + fGlassSubstrateHalfSizeXpxl + .5f);
    o_rtSpecROI.m_top = (long)(fptBodyCenter.m_y - fGlassSubstrateHalfSizeYpxl + .5f);
    o_rtSpecROI.m_bottom = (long)(fptBodyCenter.m_y + fGlassSubstrateHalfSizeYpxl + .5f);

    return TRUE;
}

BOOL VisionInspectionGlassSubstrate2D::MakeEdgeSearchROI(
    const Ipvm::Rect32s i_rtSpecROI, std::vector<Ipvm::Rect32s>& o_vecrtSearchROI)
{
    const auto& um2px = getScale().umToPixel();
    const float fWidthRatio = (float)max(0.1f, min(1.f, m_VisionPara->m_nEdgeSearchWidthRatio * 0.01f));
    const long nSearchLengthHalfLR_Pxl = (long)(m_VisionPara->m_nEdgeSearchLength_um * um2px.m_x * .5f + .5f);
    const long nSearchLengthHalfTB_Pxl = (long)(m_VisionPara->m_nEdgeSearchLength_um * um2px.m_y * .5f + .5f);

    o_vecrtSearchROI.clear();
    o_vecrtSearchROI.resize(4);

    long nSearchWidthHalfLR_Pxl = (long)((float)i_rtSpecROI.Height() * (1.f - fWidthRatio) * .5f + .5f);
    long nSearchWidthHalfTB_Pxl = (long)((float)i_rtSpecROI.Width() * (1.f - fWidthRatio) * .5f + .5f);

    //LEFT
    o_vecrtSearchROI[LEFT] = i_rtSpecROI;
    o_vecrtSearchROI[LEFT].m_left -= nSearchLengthHalfLR_Pxl;
    o_vecrtSearchROI[LEFT].m_right = i_rtSpecROI.m_left + nSearchLengthHalfLR_Pxl;
    o_vecrtSearchROI[LEFT].m_top += nSearchWidthHalfLR_Pxl;
    o_vecrtSearchROI[LEFT].m_bottom -= nSearchWidthHalfLR_Pxl;

    //Top
    o_vecrtSearchROI[UP] = i_rtSpecROI;
    o_vecrtSearchROI[UP].m_left += nSearchWidthHalfTB_Pxl;
    o_vecrtSearchROI[UP].m_right -= nSearchWidthHalfTB_Pxl;
    o_vecrtSearchROI[UP].m_top -= nSearchLengthHalfTB_Pxl;
    o_vecrtSearchROI[UP].m_bottom = i_rtSpecROI.m_top + nSearchLengthHalfTB_Pxl;

    //RIGHT
    o_vecrtSearchROI[RIGHT] = i_rtSpecROI;
    o_vecrtSearchROI[RIGHT].m_left = i_rtSpecROI.m_right - nSearchLengthHalfLR_Pxl;
    o_vecrtSearchROI[RIGHT].m_right += nSearchLengthHalfLR_Pxl;
    o_vecrtSearchROI[RIGHT].m_top += nSearchWidthHalfLR_Pxl;
    o_vecrtSearchROI[RIGHT].m_bottom -= nSearchWidthHalfLR_Pxl;

    //BOTTOM
    o_vecrtSearchROI[DOWN] = i_rtSpecROI;
    o_vecrtSearchROI[DOWN].m_left += nSearchWidthHalfTB_Pxl;
    o_vecrtSearchROI[DOWN].m_right -= nSearchWidthHalfTB_Pxl;
    o_vecrtSearchROI[DOWN].m_top = i_rtSpecROI.m_bottom - nSearchLengthHalfTB_Pxl;
    o_vecrtSearchROI[DOWN].m_bottom += nSearchLengthHalfTB_Pxl;

    return TRUE;
}

BOOL VisionInspectionGlassSubstrate2D::GetGlassSubstrateEdgePoint(
    Ipvm::Image8u image, long nDir, Ipvm::Rect32s rtSearchROI, std::vector<Ipvm::Point32r2>& o_vecfptEdge)
{
    const auto& um2px = getScale().umToPixel();

    o_vecfptEdge.clear();

    long nEdgeGap(0), nSearchLength(0), nSearchWidth(0);
    long nSearchDirection = m_VisionPara->m_nSearchDirection;
    long nEdgeDirection = m_VisionPara->m_nEdgeDirection;
    long nSearchDir = nDir;
    BOOL bFirstEdge = m_VisionPara->m_nEdgeDetectMode == PI_ED_FIRST_EDGE;

    Ipvm::Point32r2 bestEdge;
    Ipvm::Point32s2 ptStart;

    if (nDir == LEFT || nDir == RIGHT) //Left/Right Edge용
    {
        nEdgeGap = (long)(m_VisionPara->m_nEdgeDetectGap_um * um2px.m_y + .5f);
        nSearchLength = (long)rtSearchROI.Width();
        if (nDir == LEFT && nSearchDirection == PI_ED_DIR_INNER)
            nSearchDir = RIGHT;
        else if (nDir == RIGHT && nSearchDirection == PI_ED_DIR_INNER)
            nSearchDir = LEFT;
    }
    else //Top/Bottom Edge용
    {
        nEdgeGap = (long)(m_VisionPara->m_nEdgeDetectGap_um * um2px.m_x + .5f);
        nSearchLength = (long)rtSearchROI.Height();
        if (nDir == UP && nSearchDirection == PI_ED_DIR_INNER)
            nSearchDir = DOWN;
        else if (nDir == DOWN && nSearchDirection == PI_ED_DIR_INNER)
            nSearchDir = UP;
    }
    nEdgeGap = (long)max(2, nEdgeGap);
    nSearchLength = (long)max(5, nSearchLength);
    nSearchWidth = nEdgeGap / 2;
    nSearchWidth = (long)max(2, nSearchWidth);

    switch (nSearchDir)
    {
        case LEFT:
            ptStart = Ipvm::Point32s2(rtSearchROI.TopRight().m_x, rtSearchROI.TopRight().m_y + nSearchWidth);
            break;
        case UP:
            ptStart = Ipvm::Point32s2(rtSearchROI.BottomLeft().m_x + nSearchWidth, rtSearchROI.BottomLeft().m_y);
            break;
        case RIGHT:
            ptStart = Ipvm::Point32s2(rtSearchROI.TopLeft().m_x, rtSearchROI.TopLeft().m_y + nSearchWidth);
            break;
        case DOWN:
            ptStart = Ipvm::Point32s2(rtSearchROI.TopLeft().m_x + nSearchWidth, rtSearchROI.TopLeft().m_y);
            break;
        default:
            return FALSE;
    }

    float fEdgeThreshold_Old = m_pEdgeDetect->SetMininumThreshold((float)m_VisionPara->m_nFirstEdgeValue);
    if (nDir == LEFT || nDir == RIGHT)
    {
        long nEnd = rtSearchROI.BottomRight().m_y;
        for (long nY = ptStart.m_y; nY < nEnd; nY += nEdgeGap)
        {
            ptStart.m_y = nY;
            if (m_pEdgeDetect->PI_ED_CalcAvgEdgePointFromImage(
                    nEdgeDirection, ptStart, nSearchDir, nSearchLength, nSearchWidth, image, bestEdge, bFirstEdge))
            {
                o_vecfptEdge.push_back(bestEdge);
            }
        }
    }
    else
    {
        long nEnd = rtSearchROI.BottomRight().m_x;
        for (long nX = ptStart.m_x; nX < nEnd; nX += nEdgeGap)
        {
            ptStart.m_x = nX;
            if (m_pEdgeDetect->PI_ED_CalcAvgEdgePointFromImage(
                    nEdgeDirection, ptStart, nSearchDir, nSearchLength, nSearchWidth, image, bestEdge, bFirstEdge))
            {
                o_vecfptEdge.push_back(bestEdge);
            }
        }
    }
    m_pEdgeDetect->SetMininumThreshold(fEdgeThreshold_Old);

    return TRUE;
}

BOOL VisionInspectionGlassSubstrate2D::CalcGlassSubstrateAlignResult(std::vector<Ipvm::Point32r2> vecfptLeftEdge,
    std::vector<Ipvm::Point32r2> vecfptTopEdge, std::vector<Ipvm::Point32r2> vecfptRightEdge,
    std::vector<Ipvm::Point32r2> vecfptBottomEdge, VisionAlignResult& o_glassSubstrateAlignResult)
{
    o_glassSubstrateAlignResult.bAvailable = FALSE;

    long nLeftEdgeNum = (long)vecfptLeftEdge.size();
    long nTopEdgeNum = (long)vecfptTopEdge.size();
    long nRightEdgeNum = (long)vecfptRightEdge.size();
    long nBottomEdgeNum = (long)vecfptBottomEdge.size();

    if ((nLeftEdgeNum * nTopEdgeNum * nRightEdgeNum * nBottomEdgeNum) == 0)
        return FALSE;

    std::vector<Ipvm::LineEq32r> vecsLineEq(4);

    if (Ipvm::DataFitting::FitToLineRn(nLeftEdgeNum, &vecfptLeftEdge[0], 3.f, vecsLineEq[LEFT]) != Ipvm::Status::e_ok)
        return FALSE;
    if (Ipvm::DataFitting::FitToLineRn(nTopEdgeNum, &vecfptTopEdge[0], 3.f, vecsLineEq[UP]) != Ipvm::Status::e_ok)
        return FALSE;
    if (Ipvm::DataFitting::FitToLineRn(nRightEdgeNum, &vecfptRightEdge[0], 3.f, vecsLineEq[RIGHT])
        != Ipvm::Status::e_ok)
        return FALSE;
    if (Ipvm::DataFitting::FitToLineRn(nBottomEdgeNum, &vecfptBottomEdge[0], 3.f, vecsLineEq[DOWN])
        != Ipvm::Status::e_ok)
        return FALSE;

    m_vecLineGlassSubstrate[LEFT] = vecsLineEq[LEFT];
    m_vecLineGlassSubstrate[UP] = vecsLineEq[UP];
    m_vecLineGlassSubstrate[RIGHT] = vecsLineEq[RIGHT];
    m_vecLineGlassSubstrate[DOWN] = vecsLineEq[DOWN];

    // 직선 방정식으로부터 네 모서리를 계산한다.
    Ipvm::Point32r2& ptLeftTop = o_glassSubstrateAlignResult.fptLT;
    Ipvm::Point32r2& ptRightTop = o_glassSubstrateAlignResult.fptRT;
    Ipvm::Point32r2& ptRightBottom = o_glassSubstrateAlignResult.fptRB;
    Ipvm::Point32r2& ptLeftBottom = o_glassSubstrateAlignResult.fptLB;

    if (Ipvm::Geometry::GetCrossPoint(vecsLineEq[LEFT], vecsLineEq[UP], ptLeftTop) != Ipvm::Status::e_ok)
        return FALSE;
    if (Ipvm::Geometry::GetCrossPoint(vecsLineEq[UP], vecsLineEq[RIGHT], ptRightTop) != Ipvm::Status::e_ok)
        return FALSE;
    if (Ipvm::Geometry::GetCrossPoint(vecsLineEq[RIGHT], vecsLineEq[DOWN], ptRightBottom) != Ipvm::Status::e_ok)
        return FALSE;
    if (Ipvm::Geometry::GetCrossPoint(vecsLineEq[DOWN], vecsLineEq[LEFT], ptLeftBottom) != Ipvm::Status::e_ok)
        return FALSE;

    // 네 모서리로부터 바디 센터를 얻는다.
    o_glassSubstrateAlignResult.m_center.m_x
        = (ptLeftTop.m_x + ptRightTop.m_x + ptRightBottom.m_x + ptLeftBottom.m_x) * 0.25f;
    o_glassSubstrateAlignResult.m_center.m_y
        = (ptLeftTop.m_y + ptRightTop.m_y + ptRightBottom.m_y + ptLeftBottom.m_y) * 0.25f;

    // 네 모서리로부터 바디 앵글을 얻는다.
    const float angleLower = (float)atan2(ptRightBottom.m_y - ptLeftBottom.m_y, ptRightBottom.m_x - ptLeftBottom.m_x);
    const float angleUpper = (float)atan2(ptRightTop.m_y - ptLeftTop.m_y, ptRightTop.m_x - ptLeftTop.m_x);

    float fTopBottomAngle = (angleLower + angleUpper) * 0.5f;

    float fAngleLeft = CAST_FLOAT(
        atan2(ptLeftBottom.m_y - ptLeftTop.m_y, ptLeftBottom.m_x - ptLeftTop.m_x) - (90. * 0.017453292519943));
    float fAngleRight = CAST_FLOAT(
        atan2(ptRightBottom.m_y - ptRightTop.m_y, ptRightBottom.m_x - ptRightTop.m_x) - (90. * 0.017453292519943));
    float fLeftRightAngle = (fAngleLeft + fAngleRight) * .5f;
    o_glassSubstrateAlignResult.SetAngle((fTopBottomAngle + fLeftRightAngle) * 0.5f);

    o_glassSubstrateAlignResult.bAvailable = TRUE;

    return TRUE;
}

BOOL VisionInspectionGlassSubstrate2D::DoInspGlassSubstrateOffsetX()
{
    auto* result
        = m_resultGroup.GetResultByName(g_szGlassSubstrateInspection2DName[GLASS_SUBSTRATE_INSPECTION_2D_OFFSET_X]);
    if (result == NULL)
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

    const auto& px2um = getScale().pixelToUm();

    long nAlignedNum = (long)m_vecsGlassSubstrateAlignResult.size();
    if (nAlignedNum < 1)
    {
        return FALSE;
    }

    result->Resize(1);

    Ipvm::Point32r2 fptPackageCenter = m_sEdgeAlignResult->m_center;
    Ipvm::Point32r2 fptGlassSubstrateCenter;
    Ipvm::Point32r2 fptProjPackageCenter, fptProjGlassSubstrateCenter;
    float fDistance;
    float fSpecOffset(0.f);
    float fError;

    Ipvm::LineEq32r linePackageCenter = m_linePackageCenterHor;

    auto glassSubstrateAlignResult = m_vecsGlassSubstrateAlignResult[0];

    if (glassSubstrateAlignResult.bAvailable == FALSE)
    {
        result->SetValue_EachResult_AndTypicalValueBySingleSpec(
            0, _T("Substrate"), _T(""), _T(""), 0.f, *spec, 0.f, 0.f, 0.f, INVALID); //kircheis_VSV //kircheis_MED2.5
        result->SetRect(0, m_rtGlassSubstrateSpecROI);
        result->UpdateTypicalValue(spec);
        result->SetTotalResult();

        return FALSE;
    }

    fptGlassSubstrateCenter = glassSubstrateAlignResult.m_center;

    Ipvm::Geometry::GetFootOfPerpendicular(linePackageCenter, fptPackageCenter, fptProjPackageCenter);
    Ipvm::Geometry::GetFootOfPerpendicular(linePackageCenter, fptGlassSubstrateCenter, fptProjGlassSubstrateCenter);
    Ipvm::Geometry::GetDistance(fptProjPackageCenter, fptProjGlassSubstrateCenter, fDistance);
    fDistance *= px2um.m_x;
    if (fptProjPackageCenter.m_x > fptProjGlassSubstrateCenter.m_x)
        fDistance *= -1.f;
    fError = fDistance - fSpecOffset;

    result->SetValue_EachResult_AndTypicalValueBySingleSpec(
        0, _T("Substrate"), _T(""), _T(""), fError, *spec, 0.f, 0.f, fSpecOffset); //kircheis_MED2.5
    result->SetRect(0, glassSubstrateAlignResult.getBodyRect32s());

    result->UpdateTypicalValue(spec);
    result->SetTotalResult();

    return TRUE;
}

BOOL VisionInspectionGlassSubstrate2D::DoInspGlassSubstrateOffsetY()
{
    auto* result
        = m_resultGroup.GetResultByName(g_szGlassSubstrateInspection2DName[GLASS_SUBSTRATE_INSPECTION_2D_OFFSET_Y]);
    if (result == NULL)
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

    const auto& px2um = getScale().pixelToUm();

    long nAlignedNum = (long)m_vecsGlassSubstrateAlignResult.size();
    if (nAlignedNum < 1)
    {
        return FALSE;
    }

    result->Resize(1);

    Ipvm::Point32r2 fptPackageCenter = m_sEdgeAlignResult->m_center;
    Ipvm::Point32r2 fptGlassSubstrateCenter;
    Ipvm::Point32r2 fptProjPackageCenter, fptProjGlassSubstrateCenter;
    float fDistance;
    float fSpecOffset(0.f);
    float fError;

    Ipvm::LineEq32r linePackageCenter = m_linePackageCenterVer;

    auto glassSubstrateAlignResult = m_vecsGlassSubstrateAlignResult[0];

    if (glassSubstrateAlignResult.bAvailable == FALSE)
    {
        result->SetValue_EachResult_AndTypicalValueBySingleSpec(
            0, _T("Substrate"), _T(""), _T(""), 0.f, *spec, 0.f, 0.f, 0.f, INVALID); //kircheis_VSV //kircheis_MED2.5
        result->SetRect(0, m_rtGlassSubstrateSpecROI);
        result->UpdateTypicalValue(spec);
        result->SetTotalResult();

        return FALSE;
    }

    fptGlassSubstrateCenter = glassSubstrateAlignResult.m_center;

    Ipvm::Geometry::GetFootOfPerpendicular(linePackageCenter, fptPackageCenter, fptProjPackageCenter);
    Ipvm::Geometry::GetFootOfPerpendicular(linePackageCenter, fptGlassSubstrateCenter, fptProjGlassSubstrateCenter);
    Ipvm::Geometry::GetDistance(fptProjPackageCenter, fptProjGlassSubstrateCenter, fDistance);
    fDistance *= px2um.m_y;
    if (fptProjPackageCenter.m_y > fptProjGlassSubstrateCenter.m_y)
        fDistance *= -1.f;
    fError = fDistance - fSpecOffset;

    result->SetValue_EachResult_AndTypicalValueBySingleSpec(
        0, _T("Substrate"), _T(""), _T(""), fError, *spec, 0.f, 0.f, fSpecOffset); //kircheis_MED2.5
    result->SetRect(0, glassSubstrateAlignResult.getBodyRect32s());

    result->UpdateTypicalValue(spec);
    result->SetTotalResult();

    return TRUE;
}

BOOL VisionInspectionGlassSubstrate2D::DoInspGlassSubstrateOffsetTheta()
{
    auto* result
        = m_resultGroup.GetResultByName(g_szGlassSubstrateInspection2DName[GLASS_SUBSTRATE_INSPECTION_2D_OFFSET_THETA]);
    if (result == NULL)
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

    long nAlignedNum = (long)m_vecsGlassSubstrateAlignResult.size();
    if (nAlignedNum < 1)
    {
        return FALSE;
    }

    result->Resize(1);

    Ipvm::LineEq32r linePackageHor = m_linePackageCenterHor;
    Ipvm::LineEq32r lineGlassSubstrateHor;
    float fError;

    auto glassSubstrateAlignResult = m_vecsGlassSubstrateAlignResult[0];

    if (glassSubstrateAlignResult.bAvailable == FALSE)
    {
        result->SetValue_EachResult_AndTypicalValueBySingleSpec(
            0, _T("Substrate"), _T(""), _T(""), 0.f, *spec, 0.f, 0.f, 0.f, INVALID); //kircheis_VSV //kircheis_MED2.5
        result->SetRect(0, m_rtGlassSubstrateSpecROI);
        result->UpdateTypicalValue(spec);
        result->SetTotalResult();

        return FALSE;
    }

    lineGlassSubstrateHor = m_veclineGlassSubstrateCenterHor[0];
    fError = (float)(atan(-lineGlassSubstrateHor.m_a / lineGlassSubstrateHor.m_b)
                 - atan(-linePackageHor.m_a / linePackageHor.m_b))
        * ITP_RAD_TO_DEG;

    result->SetValue_EachResult_AndTypicalValueBySingleSpec(
        0, _T("Substrate"), _T(""), _T(""), fError, *spec, 0.f, 0.f); //kircheis_MED2.5
    result->SetRect(0, glassSubstrateAlignResult.getBodyRect32s());

    result->UpdateTypicalValue(spec);
    result->SetTotalResult();

    return TRUE;
}

BOOL VisionInspectionGlassSubstrate2D::DoInspGlassSubstrateSizeX()
{
    auto* result
        = m_resultGroup.GetResultByName(g_szGlassSubstrateInspection2DName[GLASS_SUBSTRATE_INSPECTION_2D_SIZE_X]);
    if (result == NULL)
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

    long nAlignedNum = (long)m_vecsGlassSubstrateAlignResult.size();
    if (nAlignedNum < 1)
    {
        return FALSE;
    }

    result->Resize(1);

    auto glassSubstrateAlignResult = m_vecsGlassSubstrateAlignResult[0];
    const auto& pixelToUm = getScale().pixelToUm();

    if (glassSubstrateAlignResult.bAvailable == FALSE)
    {
        result->SetValue_EachResult_AndTypicalValueBySingleSpec(
            0, _T("Substrate"), _T(""), _T(""), 0.f, *spec, 0.f, 0.f, 0.f, INVALID); //kircheis_VSV //kircheis_MED2.5
        result->SetRect(0, m_rtGlassSubstrateSpecROI);
        result->UpdateTypicalValue(spec);
        result->SetTotalResult();

        return FALSE;
    }
    float fSpecSize = m_packageSpec.m_bodyInfoMaster->m_fGlassCoreSubstrateSizeX;
    float fError(0.f);
    Ipvm::LineEq32r lineLT, lineRB;
    float fDist(0.f), fDistMax(0.f);
    Ipvm::Point32r2 fptDest;

    lineLT = m_vecLineGlassSubstrate[LEFT];
    lineRB = m_vecLineGlassSubstrate[RIGHT];

    //LT Line to RT
    fDist = 0.f;
    fptDest = glassSubstrateAlignResult.fptRT;
    Ipvm::Geometry::GetDistance(lineLT, fptDest, fDist);
    fDistMax = (float)max(fDistMax, fDist);

    //LT Line to RB
    fDist = 0.f;
    fptDest = glassSubstrateAlignResult.fptRB;
    Ipvm::Geometry::GetDistance(lineLT, fptDest, fDist);
    fDistMax = (float)max(fDistMax, fDist);

    //RB Line to LT
    fDist = 0.f;
    fptDest = glassSubstrateAlignResult.fptLT;
    Ipvm::Geometry::GetDistance(lineRB, fptDest, fDist);
    fDistMax = (float)max(fDistMax, fDist);

    //RB Line to LB
    fDist = 0.f;
    fptDest = glassSubstrateAlignResult.fptLB;
    Ipvm::Geometry::GetDistance(lineRB, fptDest, fDist);
    fDistMax = (float)max(fDistMax, fDist);

    fDistMax *= pixelToUm.m_y;
    fError = fDistMax - fSpecSize;

    result->SetValue_EachResult_AndTypicalValueBySingleSpec(
        0, _T("Substrate"), _T(""), _T(""), fError, *spec, 0.f, 0.f, fSpecSize); //kircheis_MED2.5
    result->SetRect(0, glassSubstrateAlignResult.getBodyRect32s());

    result->UpdateTypicalValue(spec);
    result->SetTotalResult();

    return TRUE;
}

BOOL VisionInspectionGlassSubstrate2D::DoInspGlassSubstrateSizeY()
{
    auto* result
        = m_resultGroup.GetResultByName(g_szGlassSubstrateInspection2DName[GLASS_SUBSTRATE_INSPECTION_2D_SIZE_Y]);
    if (result == NULL)
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

    long nAlignedNum = (long)m_vecsGlassSubstrateAlignResult.size();
    if (nAlignedNum < 1)
    {
        return FALSE;
    }

    result->Resize(1);

    auto glassSubstrateAlignResult = m_vecsGlassSubstrateAlignResult[0];
    const auto& pixelToUm = getScale().pixelToUm();

    if (glassSubstrateAlignResult.bAvailable == FALSE)
    {
        result->SetValue_EachResult_AndTypicalValueBySingleSpec(
            0, _T("Substrate"), _T(""), _T(""), 0.f, *spec, 0.f, 0.f, 0.f, INVALID); //kircheis_VSV //kircheis_MED2.5
        result->SetRect(0, m_rtGlassSubstrateSpecROI);
        result->UpdateTypicalValue(spec);
        result->SetTotalResult();

        return FALSE;
    }
    float fSpecSize = m_packageSpec.m_bodyInfoMaster->m_fGlassCoreSubstrateSizeY;
    float fError(0.f);
    Ipvm::LineEq32r lineLT, lineRB;
    float fDist(0.f), fDistMax(0.f);
    Ipvm::Point32r2 fptDest;

    lineLT = m_vecLineGlassSubstrate[UP];
    lineRB = m_vecLineGlassSubstrate[DOWN];

    //LT Line to LB
    fDist = 0.f;
    fptDest = glassSubstrateAlignResult.fptLB;
    Ipvm::Geometry::GetDistance(lineLT, fptDest, fDist);
    fDistMax = (float)max(fDistMax, fDist);

    //LT Line to RB
    fDist = 0.f;
    fptDest = glassSubstrateAlignResult.fptRB;
    Ipvm::Geometry::GetDistance(lineLT, fptDest, fDist);
    fDistMax = (float)max(fDistMax, fDist);

    //RB Line to LT
    fDist = 0.f;
    fptDest = glassSubstrateAlignResult.fptLT;
    Ipvm::Geometry::GetDistance(lineRB, fptDest, fDist);
    fDistMax = (float)max(fDistMax, fDist);

    //RB Line to RT
    fDist = 0.f;
    fptDest = glassSubstrateAlignResult.fptRT;
    Ipvm::Geometry::GetDistance(lineRB, fptDest, fDist);
    fDistMax = (float)max(fDistMax, fDist);

    fDistMax *= pixelToUm.m_x;
    fError = fDistMax - fSpecSize;

    result->SetValue_EachResult_AndTypicalValueBySingleSpec(
        0, _T("Substrate"), _T(""), _T(""), fError, *spec, 0.f, 0.f, fSpecSize); //kircheis_MED2.5
    result->SetRect(0, glassSubstrateAlignResult.getBodyRect32s());

    result->UpdateTypicalValue(spec);
    result->SetTotalResult();

    return TRUE;
}
