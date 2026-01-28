//CPP_0_________________________________ Precompiled header
#include "stdAfx.h"

//CPP_1_________________________________ Main header
#include "VisionInspectionLid2D.h"

//CPP_2_________________________________ This project's headers
#include "DlgVisionInspectionLid2D.h"
#include "VisionInspectionLid2DPara.h"

//CPP_3_________________________________ Other projects' headers
#include "../../AlgorithmModules/dPI_EdgeDetect/dPI_EdgeDetectLib.h"
#include "../../DefineModules/dA_Base/FPI_RECT.h"
#include "../../DefineModules/dA_Base/VisionScale.h"
#include "../../DefineModules/dA_Base/iDataType.h"
#include "../../DefineModules/dA_Base/semiinfo.h"
#include "../../ImageCombineModules/dPI_ippModules/ippModules.h"
#include "../../InformationModule/dPI_DataBase/DMSHeatsink.h"
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

void VisionInspectionLid2D::AbsoluteRectToImageRectByBodyCenter(Ipvm::Rect32s& rtROI)
{
    Ipvm::Rect32r frtBodyRect = GetBodyRect();
    AbsoluteRectToImageRectByBodyCenter(frtBodyRect, rtROI);
}

void VisionInspectionLid2D::AbsoluteRectToImageRectByBodyCenter(Ipvm::Rect32r frtBodyRect, Ipvm::Rect32s& rtROI)
{
    Ipvm::Point32r2 fptCenter = frtBodyRect.CenterPoint();
    rtROI.m_left += (long)(fptCenter.m_x + .5f);
    rtROI.m_top += (long)(fptCenter.m_y + .5f);
    rtROI.m_right += (long)(fptCenter.m_x + .5f);
    rtROI.m_bottom += (long)(fptCenter.m_y + .5f);
}

void VisionInspectionLid2D::ImageRectToAbsoluteRectByBodyCenter(Ipvm::Rect32s& rtROI)
{
    Ipvm::Rect32r frtBodyRect = GetBodyRect();
    ImageRectToAbsoluteRectByBodyCenter(frtBodyRect, rtROI);
}

void VisionInspectionLid2D::ImageRectToAbsoluteRectByBodyCenter(Ipvm::Rect32r frtBodyRect, Ipvm::Rect32s& rtROI)
{
    Ipvm::Point32r2 fptCenter = frtBodyRect.CenterPoint();
    rtROI.m_left -= (long)(fptCenter.m_x + .5f);
    rtROI.m_top -= (long)(fptCenter.m_y + .5f);
    rtROI.m_right -= (long)(fptCenter.m_x + .5f);
    rtROI.m_bottom -= (long)(fptCenter.m_y + .5f);
}

Ipvm::Rect32r VisionInspectionLid2D::GetBodyRect()
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

BOOL VisionInspectionLid2D::OnInspection()
{
    m_pVisionInspDlg->OnBnClickedButtonInspect();

    return TRUE;
}

BOOL VisionInspectionLid2D::DoInspection(const bool detailSetupMode, const enSideVisionModule i_ModuleStatus)
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

    if (!DoAlignLid(detailSetupMode))
        return FALSE;

    bResult &= DoInspLidOffsetX();
    bResult &= DoInspLidOffsetY();
    bResult &= DoInspLidOffsetTheta();
    bResult &= DoInspLidSizeX();
    bResult &= DoInspLidSizeX();

    m_fCalcTime = CAST_FLOAT(time_Insp.Elapsed_ms());

    return bResult;
}

BOOL VisionInspectionLid2D::GetLidInfoDB()
{
    m_vecsLidInfoDB.clear();

    static const float fMMtoum(1000.f);
    const auto& um2px = getScale().umToPixel();

    Ipvm::Point32r2 fptPackageCenter_Pxl = m_sEdgeAlignResult->m_center;

    for (auto LidInfo_MapData : m_packageSpec.vecDMSHeatsink)
    {
        SDieLidPatchInfoDB sLidInfo;
        sLidInfo.strName = LidInfo_MapData.strCompID;
        sLidInfo.strType = LidInfo_MapData.strCompType;
        sLidInfo.fOffsetX_um = LidInfo_MapData.fChipOffsetX * fMMtoum;
        sLidInfo.fOffsetY_um = -LidInfo_MapData.fChipOffsetY * fMMtoum;
        sLidInfo.fAngle_Deg = LidInfo_MapData.fChipAngle;

        sLidInfo.fWidth_um = LidInfo_MapData.fChipWidth * fMMtoum;
        sLidInfo.fLength_um = LidInfo_MapData.fChipLength * fMMtoum;
        sLidInfo.fHeight_um = LidInfo_MapData.fChipThickness * fMMtoum;

        float fLidOffsetX_Pxl = sLidInfo.fOffsetX_um * um2px.m_x;
        float fLidOffsetY_Pxl = sLidInfo.fOffsetY_um * um2px.m_y;
        float fLidSizeX_Pxl = sLidInfo.fWidth_um * um2px.m_x;
        float fLidSizeY_Pxl = sLidInfo.fLength_um * um2px.m_y;
        float fLidSizeHalfX_Pxl = fLidSizeX_Pxl * .5f;
        float fLidSizeHalfY_Pxl = fLidSizeY_Pxl * .5f;

        sLidInfo.rtSpecROI.m_left = (long)(fptPackageCenter_Pxl.m_x + fLidOffsetX_Pxl - fLidSizeHalfX_Pxl + .5f);
        sLidInfo.rtSpecROI.m_right = (long)(fptPackageCenter_Pxl.m_x + fLidOffsetX_Pxl + fLidSizeHalfX_Pxl + .5f);
        sLidInfo.rtSpecROI.m_top = (long)(fptPackageCenter_Pxl.m_y + fLidOffsetY_Pxl - fLidSizeHalfY_Pxl + .5f);
        sLidInfo.rtSpecROI.m_bottom = (long)(fptPackageCenter_Pxl.m_y + fLidOffsetY_Pxl + fLidSizeHalfY_Pxl + .5f);

        m_vecsLidInfoDB.push_back(sLidInfo);
    }

    return (m_vecsLidInfoDB.size() > 0);
}

BOOL VisionInspectionLid2D::CalcAlignedCenterLine(
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

BOOL VisionInspectionLid2D::DoAlignLid(const bool detailSetupMode)
{
    m_vecsLidAlignResult.clear();
    m_veclineLidCenterHor.clear();
    m_veclineLidCenterVer.clear();

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

    //Lid 정보 가져오기
    if (GetLidInfoDB() == FALSE)
        return FALSE;
    long nLidNum = (long)m_vecsLidInfoDB.size();

    //실제 Align 구간
    std::vector<Ipvm::Rect32s> vecrtEdgeSearchROI(4);
    std::vector<Ipvm::Point32r2> vecfptLeftEdge(0);
    std::vector<Ipvm::Point32r2> vecfptTopEdge(0);
    std::vector<Ipvm::Point32r2> vecfptRightEdge(0);
    std::vector<Ipvm::Point32r2> vecfptBottomEdge(0);
    BOOL bAlignResult = TRUE;
    Ipvm::LineEq32r lineHor, lineVer;

    for (long nLid = 0; nLid < nLidNum; nLid++)
    {
        VisionAlignResult lidAlignResult;
        PI_RECT piAlignRect;
        lidAlignResult.bAvailable = FALSE;

        //Make Search ROI
        MakeEdgeSearchROI(m_vecsLidInfoDB[nLid], vecrtEdgeSearchROI);
        vecrtSpecROI.push_back(m_vecsLidInfoDB[nLid].rtSpecROI);
        vecrtSearchROI.push_back(vecrtEdgeSearchROI[LEFT]); //Align ROI의 순서를 맞추기 위해 insert를 안쓴거임
        vecrtSearchROI.push_back(vecrtEdgeSearchROI[UP]);
        vecrtSearchROI.push_back(vecrtEdgeSearchROI[RIGHT]);
        vecrtSearchROI.push_back(vecrtEdgeSearchROI[DOWN]);

        //Edge Detect Left
        GetLidEdgePoint(combineImage, LEFT, vecrtEdgeSearchROI[LEFT], vecfptLeftEdge);
        vecfptDebugLeftEdge.insert(vecfptDebugLeftEdge.end(), vecfptLeftEdge.begin(), vecfptLeftEdge.end());
        if (vecfptLeftEdge.size() <= 3)
            bAlignResult = FALSE;

        //Edge Detect Top
        GetLidEdgePoint(combineImage, UP, vecrtEdgeSearchROI[UP], vecfptTopEdge);
        vecfptDebugTopEdge.insert(vecfptDebugTopEdge.end(), vecfptTopEdge.begin(), vecfptTopEdge.end());
        if (vecfptTopEdge.size() <= 3)
            bAlignResult = FALSE;

        //Edge Detect Right
        GetLidEdgePoint(combineImage, RIGHT, vecrtEdgeSearchROI[RIGHT], vecfptRightEdge);
        vecfptDebugRightEdge.insert(vecfptDebugRightEdge.end(), vecfptRightEdge.begin(), vecfptRightEdge.end());
        if (vecfptRightEdge.size() <= 3)
            bAlignResult = FALSE;

        //Edge Detect bottom
        GetLidEdgePoint(combineImage, DOWN, vecrtEdgeSearchROI[DOWN], vecfptBottomEdge);
        vecfptDebugBottomEdge.insert(vecfptDebugBottomEdge.end(), vecfptBottomEdge.begin(), vecfptBottomEdge.end());
        if (vecfptBottomEdge.size() <= 3)
            bAlignResult = FALSE;

        //Line Fitting and Make Cross Point

        if (CalcLidAlignResult(vecfptLeftEdge, vecfptTopEdge, vecfptRightEdge, vecfptBottomEdge, lidAlignResult))
        {
            m_vecsLidAlignResult.push_back(lidAlignResult);

            vecfrtAlignResult.push_back(lidAlignResult.getBodyRect());
            piAlignRect.SetLTPoint(Ipvm::Conversion::ToPoint32s2(lidAlignResult.fptLT));
            piAlignRect.SetRTPoint(Ipvm::Conversion::ToPoint32s2(lidAlignResult.fptRT));
            piAlignRect.SetRBPoint(Ipvm::Conversion::ToPoint32s2(lidAlignResult.fptRB));
            piAlignRect.SetLBPoint(Ipvm::Conversion::ToPoint32s2(lidAlignResult.fptLB));
            vecprtAlignResult.push_back(piAlignRect);

            CalcAlignedCenterLine(lidAlignResult, lineHor, lineVer);
            m_veclineLidCenterHor.push_back(lineHor);
            m_veclineLidCenterVer.push_back(lineVer);
        }
        else
            bAlignResult = FALSE;

        vecfptLeftEdge.clear();
        vecfptTopEdge.clear();
        vecfptRightEdge.clear();
        vecfptBottomEdge.clear();
    }

    SetDebugInfoItem(detailSetupMode, _T("Lid Spec"), vecrtSpecROI);
    SetDebugInfoItem(detailSetupMode, _T("Lid Edge Search ROI"), vecrtSearchROI);
    SetDebugInfoItem(detailSetupMode, _T("Lid Edge Point Left"), vecfptDebugLeftEdge);
    SetDebugInfoItem(detailSetupMode, _T("Lid Edge Point Top"), vecfptDebugTopEdge);
    SetDebugInfoItem(detailSetupMode, _T("Lid Edge Point Right"), vecfptDebugRightEdge);
    SetDebugInfoItem(detailSetupMode, _T("Lid Edge Point Bottom"), vecfptDebugBottomEdge);
    SetDebugInfoItem(detailSetupMode, _T("Detect Lid Rect"), vecfrtAlignResult, TRUE);

    //{{//kircheis_USI_OA
    auto* psDebugInfo = m_DebugInfoGroup.GetDebugInfo(_T("Detect Lid ROI"));
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
        name.Format(_T("Lid Align Result [%s]"), (LPCTSTR)m_vecsLidInfoDB[i].strName);

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

BOOL VisionInspectionLid2D::MakeEdgeSearchROI(
    SDieLidPatchInfoDB lidInfoDB, std::vector<Ipvm::Rect32s>& o_vecrtSearchROI)
{
    const auto& um2px = getScale().umToPixel();
    const float fWidthRatio = (float)max(0.1f, min(1.f, m_VisionPara->m_nEdgeSearchWidthRatio * 0.01f));
    const long nSearchLengthHalfLR_Pxl = (long)(m_VisionPara->m_nEdgeSearchLength_um * um2px.m_x * .5f + .5f);
    const long nSearchLengthHalfTB_Pxl = (long)(m_VisionPara->m_nEdgeSearchLength_um * um2px.m_y * .5f + .5f);

    o_vecrtSearchROI.clear();
    o_vecrtSearchROI.resize(4);

    Ipvm::Rect32s rtLidSpecROI = lidInfoDB.rtSpecROI;

    long nSearchWidthHalfLR_Pxl = (long)((float)rtLidSpecROI.Height() * (1.f - fWidthRatio) * .5f + .5f);
    long nSearchWidthHalfTB_Pxl = (long)((float)rtLidSpecROI.Width() * (1.f - fWidthRatio) * .5f + .5f);

    //LEFT
    o_vecrtSearchROI[LEFT] = rtLidSpecROI;
    o_vecrtSearchROI[LEFT].m_left -= nSearchLengthHalfLR_Pxl;
    o_vecrtSearchROI[LEFT].m_right = rtLidSpecROI.m_left + nSearchLengthHalfLR_Pxl;
    o_vecrtSearchROI[LEFT].m_top += nSearchWidthHalfLR_Pxl;
    o_vecrtSearchROI[LEFT].m_bottom -= nSearchWidthHalfLR_Pxl;

    //Top
    o_vecrtSearchROI[UP] = rtLidSpecROI;
    o_vecrtSearchROI[UP].m_left += nSearchWidthHalfTB_Pxl;
    o_vecrtSearchROI[UP].m_right -= nSearchWidthHalfTB_Pxl;
    o_vecrtSearchROI[UP].m_top -= nSearchLengthHalfTB_Pxl;
    o_vecrtSearchROI[UP].m_bottom = rtLidSpecROI.m_top + nSearchLengthHalfTB_Pxl;

    //RIGHT
    o_vecrtSearchROI[RIGHT] = rtLidSpecROI;
    o_vecrtSearchROI[RIGHT].m_left = rtLidSpecROI.m_right - nSearchLengthHalfLR_Pxl;
    o_vecrtSearchROI[RIGHT].m_right += nSearchLengthHalfLR_Pxl;
    o_vecrtSearchROI[RIGHT].m_top += nSearchWidthHalfLR_Pxl;
    o_vecrtSearchROI[RIGHT].m_bottom -= nSearchWidthHalfLR_Pxl;

    //BOTTOM
    o_vecrtSearchROI[DOWN] = rtLidSpecROI;
    o_vecrtSearchROI[DOWN].m_left += nSearchWidthHalfTB_Pxl;
    o_vecrtSearchROI[DOWN].m_right -= nSearchWidthHalfTB_Pxl;
    o_vecrtSearchROI[DOWN].m_top = rtLidSpecROI.m_bottom - nSearchLengthHalfTB_Pxl;
    o_vecrtSearchROI[DOWN].m_bottom += nSearchLengthHalfTB_Pxl;

    return TRUE;
}

BOOL VisionInspectionLid2D::GetLidEdgePoint(
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

BOOL VisionInspectionLid2D::CalcLidAlignResult(std::vector<Ipvm::Point32r2> vecfptLeftEdge,
    std::vector<Ipvm::Point32r2> vecfptTopEdge, std::vector<Ipvm::Point32r2> vecfptRightEdge,
    std::vector<Ipvm::Point32r2> vecfptBottomEdge, VisionAlignResult& o_lidAlignResult)
{
    o_lidAlignResult.bAvailable = FALSE;

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

    // 직선 방정식으로부터 네 모서리를 계산한다.
    Ipvm::Point32r2& ptLeftTop = o_lidAlignResult.fptLT;
    Ipvm::Point32r2& ptRightTop = o_lidAlignResult.fptRT;
    Ipvm::Point32r2& ptRightBottom = o_lidAlignResult.fptRB;
    Ipvm::Point32r2& ptLeftBottom = o_lidAlignResult.fptLB;

    if (Ipvm::Geometry::GetCrossPoint(vecsLineEq[LEFT], vecsLineEq[UP], ptLeftTop) != Ipvm::Status::e_ok)
        return FALSE;
    if (Ipvm::Geometry::GetCrossPoint(vecsLineEq[UP], vecsLineEq[RIGHT], ptRightTop) != Ipvm::Status::e_ok)
        return FALSE;
    if (Ipvm::Geometry::GetCrossPoint(vecsLineEq[RIGHT], vecsLineEq[DOWN], ptRightBottom) != Ipvm::Status::e_ok)
        return FALSE;
    if (Ipvm::Geometry::GetCrossPoint(vecsLineEq[DOWN], vecsLineEq[LEFT], ptLeftBottom) != Ipvm::Status::e_ok)
        return FALSE;

    // 네 모서리로부터 바디 센터를 얻는다.
    o_lidAlignResult.m_center.m_x = (ptLeftTop.m_x + ptRightTop.m_x + ptRightBottom.m_x + ptLeftBottom.m_x) * 0.25f;
    o_lidAlignResult.m_center.m_y = (ptLeftTop.m_y + ptRightTop.m_y + ptRightBottom.m_y + ptLeftBottom.m_y) * 0.25f;

    // 네 모서리로부터 바디 앵글을 얻는다.
    const float angleLower = (float)atan2(ptRightBottom.m_y - ptLeftBottom.m_y, ptRightBottom.m_x - ptLeftBottom.m_x);
    const float angleUpper = (float)atan2(ptRightTop.m_y - ptLeftTop.m_y, ptRightTop.m_x - ptLeftTop.m_x);

    float fTopBottomAngle = (angleLower + angleUpper) * 0.5f;

    float fAngleLeft = CAST_FLOAT(
        atan2(ptLeftBottom.m_y - ptLeftTop.m_y, ptLeftBottom.m_x - ptLeftTop.m_x) - (90. * 0.017453292519943));
    float fAngleRight = CAST_FLOAT(
        atan2(ptRightBottom.m_y - ptRightTop.m_y, ptRightBottom.m_x - ptRightTop.m_x) - (90. * 0.017453292519943));
    float fLeftRightAngle = (fAngleLeft + fAngleRight) * .5f;
    o_lidAlignResult.SetAngle((fTopBottomAngle + fLeftRightAngle) * 0.5f);

    o_lidAlignResult.bAvailable = TRUE;

    return TRUE;
}

BOOL VisionInspectionLid2D::DoInspLidOffsetX()
{
    auto* result = m_resultGroup.GetResultByName(g_szLidInspection2DName[LID_INSPECTION_2D_OFFSET_X]);
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

    const auto& px2um = getScale().pixelToUm();

    long nLidSpecNum = (long)m_vecsLidInfoDB.size();
    long nLidNum = (long)m_vecsLidAlignResult.size();
    if (nLidSpecNum != nLidNum)
    {
        return FALSE;
    }
    if (nLidNum == 0 || nLidSpecNum == 0)
        return TRUE;

    result->Resize(nLidNum);

    Ipvm::Point32r2 fptPackageCenter = m_sEdgeAlignResult->m_center;
    Ipvm::Point32r2 fptLidCenter;
    Ipvm::Point32r2 fptProjPackageCenter, fptProjLidCenter;
    float fDistance;
    float fSpecOffset;
    float fError;

    Ipvm::LineEq32r linePackageCenter = m_linePackageCenterHor;

    for (long nLid = 0; nLid < nLidNum; nLid++)
    {
        auto lidAlignResult = m_vecsLidAlignResult[nLid];
        auto lidInfo = m_vecsLidInfoDB[nLid];
        fSpecOffset = lidInfo.fOffsetX_um;
        if (lidAlignResult.bAvailable == FALSE)
        {
            result->SetValue_EachResult_AndTypicalValueBySingleSpec(nLid, lidInfo.strName, lidInfo.strType,
                lidInfo.strType, 0.f, *spec, lidInfo.fOffsetX_um, lidInfo.fOffsetY_um, fSpecOffset,
                INVALID); //kircheis_VSV //kircheis_MED2.5
            //result->SetResult(nLid, INVALID);
            result->SetRect(nLid, lidInfo.rtSpecROI);
            continue;
        }

        fptLidCenter = lidAlignResult.m_center;

        Ipvm::Geometry::GetFootOfPerpendicular(linePackageCenter, fptPackageCenter, fptProjPackageCenter);
        Ipvm::Geometry::GetFootOfPerpendicular(linePackageCenter, fptLidCenter, fptProjLidCenter);
        Ipvm::Geometry::GetDistance(fptProjPackageCenter, fptProjLidCenter, fDistance);
        fDistance *= px2um.m_x;
        if (fptProjPackageCenter.m_x > fptProjLidCenter.m_x)
            fDistance *= -1.f;
        fError = fDistance - fSpecOffset;

        result->SetValue_EachResult_AndTypicalValueBySingleSpec(nLid, lidInfo.strName, lidInfo.strType, lidInfo.strType,
            fError, *spec, lidInfo.fOffsetX_um, lidInfo.fOffsetY_um, fSpecOffset); //kircheis_MED2.5
        result->SetRect(nLid, lidAlignResult.getBodyRect32s());
    }

    result->UpdateTypicalValue(spec);
    result->SetTotalResult();

    return TRUE;
}

BOOL VisionInspectionLid2D::DoInspLidOffsetY()
{
    auto* result = m_resultGroup.GetResultByName(g_szLidInspection2DName[LID_INSPECTION_2D_OFFSET_Y]);
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

    const auto& px2um = getScale().pixelToUm();

    long nLidSpecNum = (long)m_vecsLidInfoDB.size();
    long nLidNum = (long)m_vecsLidAlignResult.size();
    if (nLidSpecNum != nLidNum)
    {
        return FALSE;
    }
    if (nLidNum == 0 || nLidSpecNum == 0)
        return TRUE;

    result->Resize(nLidNum);

    Ipvm::Point32r2 fptPackageCenter = m_sEdgeAlignResult->m_center;
    Ipvm::Point32r2 fptLidCenter;
    Ipvm::Point32r2 fptProjPackageCenter, fptProjLidCenter;
    float fDistance;
    float fSpecOffset;
    float fError;

    Ipvm::LineEq32r linePackageCenter = m_linePackageCenterVer;

    for (long nLid = 0; nLid < nLidNum; nLid++)
    {
        auto lidAlignResult = m_vecsLidAlignResult[nLid];
        auto lidInfo = m_vecsLidInfoDB[nLid];
        fSpecOffset = lidInfo.fOffsetY_um;
        if (lidAlignResult.bAvailable == FALSE)
        {
            result->SetValue_EachResult_AndTypicalValueBySingleSpec(nLid, lidInfo.strName, lidInfo.strType,
                lidInfo.strType, 0.f, *spec, lidInfo.fOffsetX_um, lidInfo.fOffsetY_um, fSpecOffset,
                INVALID); //kircheis_VSV //kircheis_MED2.5
            //result->SetResult(nLid, INVALID);
            result->SetRect(nLid, lidInfo.rtSpecROI);
            continue;
        }

        fptLidCenter = lidAlignResult.m_center;

        Ipvm::Geometry::GetFootOfPerpendicular(linePackageCenter, fptPackageCenter, fptProjPackageCenter);
        Ipvm::Geometry::GetFootOfPerpendicular(linePackageCenter, fptLidCenter, fptProjLidCenter);
        Ipvm::Geometry::GetDistance(fptProjPackageCenter, fptProjLidCenter, fDistance);
        fDistance *= px2um.m_y;
        if (fptProjPackageCenter.m_y > fptProjLidCenter.m_y)
            fDistance *= -1.f;
        fError = fDistance - fSpecOffset;

        result->SetValue_EachResult_AndTypicalValueBySingleSpec(nLid, lidInfo.strName, lidInfo.strType, lidInfo.strType,
            fError, *spec, lidInfo.fOffsetX_um, lidInfo.fOffsetY_um, fSpecOffset); //kircheis_MED2.5
        result->SetRect(nLid, lidAlignResult.getBodyRect32s());
    }

    result->UpdateTypicalValue(spec);
    result->SetTotalResult();

    return TRUE;
}

BOOL VisionInspectionLid2D::DoInspLidOffsetTheta()
{
    auto* result = m_resultGroup.GetResultByName(g_szLidInspection2DName[LID_INSPECTION_2D_OFFSET_THETA]);
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

    long nLidSpecNum = (long)m_vecsLidInfoDB.size();
    long nLidNum = (long)m_vecsLidAlignResult.size();
    if (nLidSpecNum != nLidNum)
    {
        return FALSE;
    }
    if (nLidNum == 0 || nLidSpecNum == 0)
        return TRUE;

    result->Resize(nLidNum);

    Ipvm::LineEq32r linePackageHor = m_linePackageCenterHor;
    Ipvm::LineEq32r lineLidHor;
    float fError;

    for (long nLid = 0; nLid < nLidNum; nLid++)
    {
        auto lidAlignResult = m_vecsLidAlignResult[nLid];
        auto lidInfo = m_vecsLidInfoDB[nLid];
        if (lidAlignResult.bAvailable == FALSE)
        {
            result->SetValue_EachResult_AndTypicalValueBySingleSpec(nLid, lidInfo.strName, lidInfo.strType,
                lidInfo.strType, 0.f, *spec, lidInfo.fOffsetX_um, lidInfo.fOffsetY_um, 0.f,
                INVALID); //kircheis_VSV //kircheis_MED2.5
            //result->SetResult(nLid, INVALID);
            result->SetRect(nLid, lidInfo.rtSpecROI);
            continue;
        }

        lineLidHor = m_veclineLidCenterHor[nLid];
        fError = (float)(atan(-lineLidHor.m_a / lineLidHor.m_b) - atan(-linePackageHor.m_a / linePackageHor.m_b))
            * ITP_RAD_TO_DEG;

        result->SetValue_EachResult_AndTypicalValueBySingleSpec(nLid, lidInfo.strName, lidInfo.strType, lidInfo.strType,
            fError, *spec, lidInfo.fOffsetX_um, lidInfo.fOffsetY_um); //kircheis_MED2.5
        result->SetRect(nLid, lidAlignResult.getBodyRect32s());
    }
    result->UpdateTypicalValue(spec);
    result->SetTotalResult();

    return TRUE;
}

BOOL VisionInspectionLid2D::DoInspLidSizeX()
{
    //auto *result = m_resultGroup.GetResultByName(g_szLidInspection2DName[LID_INSPECTION_2D_SIZE_X]);
    //if (result == NULL)
    //	return FALSE;
    //auto *spec = GetSpecByName(result->m_resultName);
    //if (spec->m_use == FALSE)
    //	return TRUE;

    /*long nLidSpecNum = (long)m_vecsLidInfoDB.size();
	long nLidNum = (long)m_vecsLidAlignResult.size();
	if (nLidSpecNum != nLidNum)
		return FALSE;
	if (nLidNum == 0 || nLidSpecNum == 0)
		return TRUE;

	result->Resize(nLidNum);

	for (long nLid = 0; nLid < nLidNum; nLid++)
	{
		auto lidAlignResult = m_vecsLidAlignResult[nLid];
		auto lidInfo = m_vecsLidInfoDB[nLid];
		if (lidAlignResult.bAvailable == FALSE)
		{
			result->SetValue_EachResult_AndTypicalValueBySingleSpecForComponent(nLid, lidInfo.strName, lidInfo.strType, 0.f, *spec, lidInfo.fLidOffsetX_um, lidInfo.fLidOffsetY_um, 0.f, INVALID);//kircheis_VSV
			//result->SetResult(nLid, INVALID);
			result->SetRect(nLid, lidInfo.rtLidSpecROI);
			continue;
		}


		result->SetValue_EachResult_AndTypicalValueBySingleSpecForComponent(nLid, lidInfo.strName, lidInfo.strType, fError, *spec, lidInfo.fLidOffsetX_um, lidInfo.fLidOffsetY_um);
		result->SetRect(nLid, lidAlignResult.getBodyRect());
	}
	result->UpdateTypicalValue(spec);
	result->SetTotalResult();*/

    return TRUE;
}

BOOL VisionInspectionLid2D::DoInspLidSizeY()
{
    //auto *result = m_resultGroup.GetResultByName(g_szLidInspection2DName[LID_INSPECTION_2D_SIZE_Y);
    //if (result == NULL)
    //	return FALSE;
    //auto *spec = GetSpecByName(result->m_resultName);
    //if (spec->m_use == FALSE)
    //	return TRUE;

    return TRUE;
}
