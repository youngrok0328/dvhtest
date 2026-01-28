//CPP_0_________________________________ Precompiled header
#include "stdAfx.h"

//CPP_1_________________________________ Main header
#include "VisionInspectionLid3D.h"

//CPP_2_________________________________ This project's headers
#include "DlgVisionInspectionLid3D.h"
#include "Result.h"
#include "VisionInspectionLid3DPara.h"

//CPP_3_________________________________ Other projects' headers
#include "../../AlgorithmModules/dPI_EdgeDetect/dPI_EdgeDetectLib.h"
#include "../../AlgorithmModules/dPI_Geometry/dPI_GeometryLib.h"
#include "../../DefineModules/dA_Base/FPI_RECT.h"
#include "../../DefineModules/dA_Base/VisionScale.h"
#include "../../DefineModules/dA_Base/iDataType.h"
#include "../../DefineModules/dA_Base/semiinfo.h"
#include "../../ImageCombineModules/dPI_ippModules/ippModules.h"
#include "../../InformationModule/dPI_DataBase/DMSHeatsink.h"
#include "../../InformationModule/dPI_DataBase/PackageSpec.h"
#include "../../MemoryModules/VisionReusableMemory/SurfaceLayerRoi.h"
#include "../../MemoryModules/VisionReusableMemory/VisionReusableMemory.h"
#include "../../SharedCommonUtilityModules/dPI_MsgDialog/SimpleMessage.h"
#include "../../SharedCommunicationModules/VisionHostCommon/VisionHostBaseDef.h"
#include "../../SharedCommunicationModules/VisionHostCommon/VisionWarpageShapeResult.h"
#include "../../SharedComponent/IntelSpecific/ShapeAlgorithm.h"
#include "../../UserInterfaceModules/ImageLotView/ImageLotView.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionAlignResult.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionImageLotInsp.h"
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

void VisionInspectionLid3D::AbsoluteRectToImageRectByBodyCenter(Ipvm::Rect32s& rtROI)
{
    Ipvm::Rect32r frtBodyRect = GetBodyRect();
    AbsoluteRectToImageRectByBodyCenter(frtBodyRect, rtROI);
}

void VisionInspectionLid3D::AbsoluteRectToImageRectByBodyCenter(Ipvm::Rect32r frtBodyRect, Ipvm::Rect32s& rtROI)
{
    Ipvm::Point32r2 fptCenter = frtBodyRect.CenterPoint();
    rtROI.m_left += (long)(fptCenter.m_x + .5f);
    rtROI.m_top += (long)(fptCenter.m_y + .5f);
    rtROI.m_right += (long)(fptCenter.m_x + .5f);
    rtROI.m_bottom += (long)(fptCenter.m_y + .5f);
}

void VisionInspectionLid3D::ImageRectToAbsoluteRectByBodyCenter(Ipvm::Rect32s& rtROI)
{
    Ipvm::Rect32r frtBodyRect = GetBodyRect();
    ImageRectToAbsoluteRectByBodyCenter(frtBodyRect, rtROI);
}

void VisionInspectionLid3D::ImageRectToAbsoluteRectByBodyCenter(Ipvm::Rect32r frtBodyRect, Ipvm::Rect32s& rtROI)
{
    Ipvm::Point32r2 fptCenter = frtBodyRect.CenterPoint();
    rtROI.m_left -= (long)(fptCenter.m_x + .5f);
    rtROI.m_top -= (long)(fptCenter.m_y + .5f);
    rtROI.m_right -= (long)(fptCenter.m_x + .5f);
    rtROI.m_bottom -= (long)(fptCenter.m_y + .5f);
}

Ipvm::Rect32r VisionInspectionLid3D::GetBodyRect()
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

BOOL VisionInspectionLid3D::OnInspection()
{
    m_pVisionInspDlg->OnBnClickedButtonInspect();

    return TRUE;
}

BOOL VisionInspectionLid3D::DoInspection(const bool detailSetupMode, const enSideVisionModule i_ModuleStatus)
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

    //Package Align 정보 가져오기
    Ipvm::Rect32r frtBody = GetBodyRect();
    if (frtBody.Width() <= 0.f || frtBody.Height() <= 0.f || m_sEdgeAlignResult->bAvailable == FALSE)
        return FALSE;

    if (!DoAlignLid(detailSetupMode))
        return FALSE;

    long nLidNum = (long)m_vecsLidInfoDB.size();
    std::vector<Ipvm::Rect32s> vecrtSubstrateROI(0);
    std::vector<std::vector<Ipvm::Rect32s>> vec2rtLidTiltROI(nLidNum);
    std::vector<std::vector<Ipvm::Rect32s>> vec2rtLidROI(nLidNum);

    std::vector<Ipvm::Rect32s> vecrtDebugLidTiltROI(0);
    std::vector<Ipvm::Rect32s> vecrtDebugLidROI(0);

    std::vector<Ipvm::Point32r3> vecfpt3SubstratePos(0);
    std::vector<std::vector<Ipvm::Point32r3>> vec2fpt3LidTiltPos(nLidNum);
    std::vector<std::vector<Ipvm::Point32r3>> vec2fpt3LidPos(nLidNum);

    Ipvm::PlaneEq64r planeSubstrate;
    std::vector<Ipvm::PlaneEq64r> vecPlaneLidTilt(nLidNum);
    std::vector<Ipvm::PlaneEq64r> vecPlaneLid(nLidNum);

    const float fSubstrateROI_OffsetX_um((float)m_VisionPara->m_nSubstrateROI_OffsetX_um);
    const float fSubstrateROI_OffsetY_um((float)m_VisionPara->m_nSubstrateROI_OffsetY_um);
    const float fSubstrateROI_size_um((float)m_VisionPara->m_nSubstrateROI_size_um);

    const float fLidTiltROI_OffsetX_um((float)m_VisionPara->m_nLidTiltROI_OffsetX_um);
    const float fLidTiltROI_OffsetY_um((float)m_VisionPara->m_nLidTiltROI_OffsetY_um);
    const float fLidTiltROI_size_um((float)m_VisionPara->m_nLidTiltROI_size_um);

    //Substrate ROI
    MakeBasicROI(*m_sEdgeAlignResult, fSubstrateROI_OffsetX_um, fSubstrateROI_OffsetY_um, fSubstrateROI_size_um,
        vecrtSubstrateROI);
    Get3DPosData(vecrtSubstrateROI, vecfpt3SubstratePos);
    Get3DPlane(vecfpt3SubstratePos, planeSubstrate);

    for (long nLid = 0; nLid < nLidNum; nLid++)
    {
        MakeBasicROI(m_vecsLidAlignResult[nLid], fLidTiltROI_OffsetX_um, fLidTiltROI_OffsetY_um, fLidTiltROI_size_um,
            vec2rtLidTiltROI[nLid]);
        vecrtDebugLidTiltROI.insert(
            vecrtDebugLidTiltROI.begin(), vec2rtLidTiltROI[nLid].begin(), vec2rtLidTiltROI[nLid].end());
        Get3DPosData(vec2rtLidTiltROI[nLid], vec2fpt3LidTiltPos[nLid]);
        Get3DPlane(vec2fpt3LidTiltPos[nLid], vecPlaneLidTilt[nLid]);

        MakeLidROI(nLid, vec2rtLidROI[nLid]);
        vecrtDebugLidROI.insert(vecrtDebugLidROI.begin(), vec2rtLidROI[nLid].begin(), vec2rtLidROI[nLid].end());
        Get3DPosData(vec2rtLidROI[nLid], vec2fpt3LidPos[nLid]);
        Get3DPlane(vec2fpt3LidPos[nLid], vecPlaneLid[nLid]);
    }

    bResult &= DoInspUnitLidTiltX(planeSubstrate, vecPlaneLidTilt);
    bResult &= DoInspUnitLidTiltY(planeSubstrate, vecPlaneLidTilt);
    bResult &= DoInspUnitLidHeight(planeSubstrate, vec2rtLidTiltROI, vec2fpt3LidTiltPos);
    bResult &= DoInspLidWarpageWithUnitWarpage(vecPlaneLid, vec2rtLidROI, vec2fpt3LidPos, detailSetupMode);

    //{{DebugInfo
    SetDebugInfoItem(detailSetupMode, _T("Substrate ROI"), vecrtSubstrateROI);
    SetDebugInfoItem(detailSetupMode, _T("Lid Tilt & Height ROI"), vecrtDebugLidTiltROI);
    SetDebugInfoItem(detailSetupMode, _T("Lid Warpage ROI"), vecrtDebugLidROI);
    //}}

    m_fCalcTime = CAST_FLOAT(time_Insp.Elapsed_ms());

    return bResult;
}

BOOL VisionInspectionLid3D::GetLidInfoDB()
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

BOOL VisionInspectionLid3D::CalcAlignedCenterLine(
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

BOOL VisionInspectionLid3D::DoAlignLid(const bool detailSetupMode)
{
    m_vecsLidAlignResult.clear();
    m_veclineLidCenterHor.clear();
    m_veclineLidCenterVer.clear();

    //검사 영상 준비
    Ipvm::Image8u image = GetInspectionFrameImage();
    if (image.GetMem() == nullptr)
        return -1;

    Ipvm::Image8u combineImage;
    if (!getReusableMemory().GetInspByteImage(combineImage))
        return false;
    if (!CippModules::GrayImageProcessingManage(
            getReusableMemory(), &image, false, m_VisionPara->m_ImageProcManagePara, combineImage))
    {
        return FALSE;
    }

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

    std::vector<float> vecfLidHeightUM(0);

    for (long nLid = 0; nLid < nLidNum; nLid++)
    {
        VisionAlignResult lidAlignResult;
        PI_RECT piAlignRect;
        lidAlignResult.bAvailable = FALSE;

        vecfLidHeightUM.push_back(m_vecsLidInfoDB[nLid].fHeight_um);

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

    SetDebugInfoItem(detailSetupMode, _T("Lid Spec Height (um)"), vecfLidHeightUM);
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

BOOL VisionInspectionLid3D::MakeEdgeSearchROI(
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

BOOL VisionInspectionLid3D::GetLidEdgePoint(
    Ipvm::Image8u image, long nDir, Ipvm::Rect32s rtSearchROI, std::vector<Ipvm::Point32r2>& o_vecfptEdge)
{
    const auto& um2px = getScale().umToPixel();

    o_vecfptEdge.clear();

    long nEdgeGap(0), nSearchLength(0), nSearchWidth(0);
    long nEdgeDirection = m_VisionPara->m_nEdgeDirection;
    long nSearchDir = nDir;
    BOOL bFirstEdge = m_VisionPara->m_nEdgeDetectMode == PI_ED_FIRST_EDGE;
    Ipvm::Point32r2 bestEdgeXY(0.f, 0.f);
    Ipvm::Point32s2 ptStart;

    if (nDir == LEFT || nDir == RIGHT) //Left/Right Edge용
    {
        nEdgeGap = (long)(m_VisionPara->m_nEdgeDetectGap_um * um2px.m_y + .5f);
        nSearchLength = (long)rtSearchROI.Width();
        if (nDir == LEFT && nEdgeDirection == PI_ED_DIR_INNER)
            nSearchDir = RIGHT;
        else if (nDir == RIGHT && nEdgeDirection == PI_ED_DIR_INNER)
            nSearchDir = LEFT;
    }
    else //Top/Bottom Edge용
    {
        nEdgeGap = (long)(m_VisionPara->m_nEdgeDetectGap_um * um2px.m_x + .5f);
        nSearchLength = (long)rtSearchROI.Height();
        if (nDir == UP && nEdgeDirection == PI_ED_DIR_INNER)
            nSearchDir = DOWN;
        else if (nDir == DOWN && nEdgeDirection == PI_ED_DIR_INNER)
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
                    nEdgeDirection, ptStart, nSearchDir, nSearchLength, nSearchWidth, image, bestEdgeXY, bFirstEdge))
            {
                o_vecfptEdge.push_back(bestEdgeXY);
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
                    nEdgeDirection, ptStart, nSearchDir, nSearchLength, nSearchWidth, image, bestEdgeXY, bFirstEdge))
            {
                o_vecfptEdge.push_back(bestEdgeXY);
            }
        }
    }
    m_pEdgeDetect->SetMininumThreshold(fEdgeThreshold_Old);

    return TRUE;
}

BOOL VisionInspectionLid3D::CalcLidAlignResult(std::vector<Ipvm::Point32r2> vecfptLeftEdge,
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

BOOL VisionInspectionLid3D::DoInspUnitLidTiltX(Ipvm::PlaneEq64r planeSub, std::vector<Ipvm::PlaneEq64r> vecPlaneLid)
{
    auto* result = m_resultGroup.GetResultByName(g_szLidInspection3DName[LID_INSPECTION_3D_UNIT_LID_TILT_X]);
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

    long nLidSpecNum = (long)m_vecsLidInfoDB.size();
    long nLidNum = (long)m_vecsLidAlignResult.size();
    if (nLidSpecNum != nLidNum)
    {
        return FALSE;
    }
    if (nLidNum == 0 || nLidSpecNum == 0)
        return TRUE;

    result->Resize(nLidNum);

    float fAngle;

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

        fAngle = (float)(GetDeltaTheta(Ipvm::Point64r2(planeSub.m_b, planeSub.m_c),
                             Ipvm::Point64r2(vecPlaneLid[nLid].m_b, vecPlaneLid[nLid].m_c))
            * DEF_RAD_TO_DEG);

        result->SetValue_EachResult_AndTypicalValueBySingleSpec(nLid, lidInfo.strName, lidInfo.strType, lidInfo.strType,
            fAngle, *spec, lidInfo.fOffsetX_um, lidInfo.fOffsetY_um); //kircheis_MED2.5
        result->SetRect(nLid, lidAlignResult.getBodyRect32s());
    }

    result->UpdateTypicalValue(spec);
    result->SetTotalResult();

    return TRUE;
}

BOOL VisionInspectionLid3D::DoInspUnitLidTiltY(Ipvm::PlaneEq64r planeSub, std::vector<Ipvm::PlaneEq64r> vecPlaneLid)
{
    auto* result = m_resultGroup.GetResultByName(g_szLidInspection3DName[LID_INSPECTION_3D_UNIT_LID_TILT_Y]);
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

    long nLidSpecNum = (long)m_vecsLidInfoDB.size();
    long nLidNum = (long)m_vecsLidAlignResult.size();
    if (nLidSpecNum != nLidNum)
    {
        return FALSE;
    }
    if (nLidNum == 0 || nLidSpecNum == 0)
        return TRUE;

    result->Resize(nLidNum);

    float fAngle;

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

        fAngle = (float)(GetDeltaTheta(Ipvm::Point64r2(planeSub.m_a, planeSub.m_c),
                             Ipvm::Point64r2(vecPlaneLid[nLid].m_a, vecPlaneLid[nLid].m_c))
            * DEF_RAD_TO_DEG);

        result->SetValue_EachResult_AndTypicalValueBySingleSpec(nLid, lidInfo.strName, lidInfo.strType, lidInfo.strType,
            fAngle, *spec, lidInfo.fOffsetX_um, lidInfo.fOffsetY_um); //kircheis_MED2.5
        result->SetRect(nLid, lidAlignResult.getBodyRect32s());
    }

    result->UpdateTypicalValue(spec);
    result->SetTotalResult();

    return TRUE;
}

double VisionInspectionLid3D::GetDeltaTheta(const Ipvm::Point64r2& v1, const Ipvm::Point64r2& v2)
{
    return asin((v1.m_x * v2.m_y - v1.m_y * v2.m_x)
        / (sqrt(v1.m_x * v1.m_x + v1.m_y * v1.m_y) * sqrt(v2.m_x * v2.m_x + v2.m_y * v2.m_y)));
}

BOOL VisionInspectionLid3D::DoInspUnitLidHeight(Ipvm::PlaneEq64r planeSub,
    std::vector<std::vector<Ipvm::Rect32s>> vec2LidROI,
    std::vector<std::vector<Ipvm::Point32r3>> vec2fpt3Lid) //Unit Level 검사 Code
{
    auto* result = m_resultGroup.GetResultByName(g_szLidInspection3DName[LID_INSPECTION_3D_UNIT_LID_HIGHT]);
    if (result == NULL)
    {
        return FALSE;
    }
    auto* spec = GetSpecByName(result->m_resultName);
    if (spec == nullptr)
    {
        return FALSE;
    }

    auto* resultABS = m_resultGroup.GetResultByName(g_szLidInspection3DName[LID_INSPECTION_3D_UNIT_LID_HIGHT_ABS]);
    if (resultABS == NULL)
    {
        return FALSE;
    }
    auto* specABS = GetSpecByName(resultABS->m_resultName);

    if (specABS == nullptr)
    {
        return FALSE;
    }

    if (spec->m_use == FALSE && specABS->m_use == FALSE)
        return TRUE;

    long nLidSpecNum = (long)m_vecsLidInfoDB.size();
    long nLidNum = (long)m_vecsLidAlignResult.size();
    long nDataGroupNum = (long)vec2fpt3Lid.size();
    if (nLidSpecNum != nLidNum || nLidSpecNum != nDataGroupNum)
    {
        return FALSE;
    }
    if (nLidNum == 0 || nLidSpecNum == 0)
        return TRUE;

    long nTotalDataNum = 0;
    for (auto vecData : vec2fpt3Lid)
        for (auto data : vecData)
            nTotalDataNum++;

    if (spec->m_use)
        result->Resize(nLidNum);
    if (specABS->m_use)
        resultABS->Resize(nLidNum);

    const auto& px2um = getScale().pixelToUm();
    const float fDeadBugGain = 1.f; //m_packageSpec.m_deadBug ? -1.f : 1.f;
    const float px2um_y_new = px2um.m_y > 0.f ? (-px2um.m_y) : px2um.m_y;

    const float planeNorm
        = float(::sqrt(planeSub.m_a * planeSub.m_a + planeSub.m_b * planeSub.m_b + planeSub.m_c * planeSub.m_c));
    float fSpecHeight{};
    float fCurHeight{};
    float fError{};
    Ipvm::Point32r3 temp;
    long nLidDataNum(0);
    long nLidTotalID(0);
    float fMax(0.f);
    Ipvm::Rect32s rtMaxROI;

    for (long nLid = 0; nLid < nLidNum; nLid++)
    {
        auto lidAlignResult = m_vecsLidAlignResult[nLid];
        auto lidInfo = m_vecsLidInfoDB[nLid];
        auto vecLid3D = vec2fpt3Lid[nLid];

        fMax = 0.f;

        if (lidAlignResult.bAvailable == FALSE)
        {
            if (spec->m_use)
            {
                result->SetValue_EachResult_AndTypicalValueBySingleSpec(nLid, lidInfo.strName, lidInfo.strType,
                    lidInfo.strType, 0.f, *spec, lidInfo.fOffsetX_um, lidInfo.fOffsetY_um, fSpecHeight,
                    INVALID); //kircheis_VSV //kircheis_MED2.5
                //result->SetResult(nLidTotalID, INVALID);
                result->SetRect(nLidTotalID, lidInfo.rtSpecROI);
            }

            if (specABS->m_use)
            {
                resultABS->SetValue_EachResult_AndTypicalValueBySingleSpec(nLid, lidInfo.strName, lidInfo.strType,
                    lidInfo.strType, 0.f, *specABS, lidInfo.fOffsetX_um, lidInfo.fOffsetY_um, fSpecHeight,
                    INVALID); //kircheis_VSV //kircheis_MED2.5
                //result->SetResult(nLidTotalID, INVALID);
                resultABS->SetRect(nLidTotalID, lidInfo.rtSpecROI);
            }
            continue;
        }

        nLidDataNum = (long)vecLid3D.size();

        fSpecHeight = lidInfo.fHeight_um;
        long nMaxHeightROI_ID = 0;
        for (long nROI = 0; nROI < nLidDataNum; nROI++)
        {
            auto src = vecLid3D[nROI];
            temp.m_x = src.m_x * px2um.m_x * fDeadBugGain;
            temp.m_y = src.m_y * px2um_y_new;
            temp.m_z = src.m_z * fDeadBugGain;

            fCurHeight
                = float((planeSub.m_a * temp.m_x + planeSub.m_b * temp.m_y + planeSub.m_c * temp.m_z + planeSub.m_d)
                    / planeNorm);
            if (fMax <= fCurHeight)
            {
                fMax = fCurHeight;
                rtMaxROI = vec2LidROI[nLid][nROI];
                nMaxHeightROI_ID = nROI;
            }

            nLidTotalID++;
        }
        fError = fMax - fSpecHeight;

        CString strDir = _T("LT");
        switch (nMaxHeightROI_ID)
        {
            case 1:
                strDir = _T("RT");
                break;
            case 2:
                strDir = _T("RB");
                break;
            case 3:
                strDir = _T("LB");
                break;
        }

        if (spec->m_use)
        {
            result->SetValue_EachResult_AndTypicalValueBySingleSpec(nLid, strDir, lidInfo.strType, lidInfo.strType,
                fError, *spec, lidInfo.fOffsetX_um, lidInfo.fOffsetY_um, fSpecHeight); //kircheis_MED2.5
            result->SetRect(nLid, rtMaxROI);
        }
        if (specABS->m_use)
        {
            resultABS->SetValue_EachResult_AndTypicalValueBySingleSpec(nLid, strDir, lidInfo.strType, lidInfo.strType,
                fMax, *specABS, lidInfo.fOffsetX_um, lidInfo.fOffsetY_um, fSpecHeight); //kircheis_MED2.5
            resultABS->SetRect(nLid, rtMaxROI);
        }
    }

    if (spec->m_use)
    {
        result->UpdateTypicalValue(spec);
        result->SetTotalResult();
    }
    if (specABS->m_use)
    {
        resultABS->UpdateTypicalValue(specABS);
        resultABS->SetTotalResult();
    }

    return TRUE;
}

//BOOL VisionInspectionLid3D::DoInspUnitLidHeight(Ipvm::PlaneEq64r planeSub, std::vector<std::vector<Ipvm::Rect32s>> vec2LidROI, std::vector<std::vector<Ipvm::Point32r3>> vec2fpt3Lid)//개별 검사 Code
//{
//	auto *result = m_resultGroup.GetResultByName(g_szLidInspection3DName[LID_INSPECTION_3D_UNIT_LID_HIGHT]);
//	if (result == NULL)
//		return FALSE;
//	auto *spec = GetSpecByName(result->m_resultName);
//	if (spec->m_use == FALSE)
//		return TRUE;
//
//	long nLidSpecNum = (long)m_vecsLidInfoDB.size();
//	long nLidNum = (long)m_vecsLidAlignResult.size();
//	long nDataGroupNum = (long)vec2fpt3Lid.size();
//	if (nLidSpecNum != nLidNum || nLidSpecNum != nDataGroupNum)
//		return FALSE;
//	if (nLidNum == 0 || nLidSpecNum == 0)
//		return TRUE;
//
//	long nTotalDataNum = 0;
//	for (auto vecData : vec2fpt3Lid)
//		for (auto data : vecData)
//			nTotalDataNum++;
//
//	result->Resize(nTotalDataNum);
//
//	const float px2um_x = getScale().pixelToUm().m_x;
//	const float px2um_y = getScale().pixelToUm().m_y;
//	const float fDeadBugGain = m_packageSpec.m_deadBug ? -1.f : 1.f;
//	const float px2um_y_new = px2um_y > 0.f ? (-px2um_y) : px2um_y;
//
//	const float planeNorm = float(::sqrt(planeSub.m_a * planeSub.m_a + planeSub.m_b * planeSub.m_b + planeSub.m_c * planeSub.m_c));
//	float fSpecHeight, fCurHeight, fError;
//	Ipvm::Point32r3 temp;
//	long nLidDataNum(0);
//	long nLidTotalID(0);
//
//	for (long nLid = 0; nLid < nLidNum; nLid++)
//	{
//		auto lidAlignResult = m_vecsLidAlignResult[nLid];
//		auto lidInfo = m_vecsLidInfoDB[nLid];
//		auto vecLid3D = vec2fpt3Lid[nLid];
//		auto lid3DGroupInfo = m_VisionPara->m_vecsLid3DGroupInfo_pxl[nLid];
//
//		nLidDataNum = (long)vecLid3D.size();
//
//		fSpecHeight = lidInfo.fHeight_um;
//		for (long nROI = 0; nROI < nLidDataNum; nROI++)
//		{
//			auto lid3DInfo = lid3DGroupInfo.vecLidROI_Info[nROI];
//			if (lidAlignResult.bAvailable == FALSE || vecLid3D[nROI].m_z == Ipvm::k_noiseValue32r)
//			{
//				result->SetValue_EachResult_AndTypicalValueBySingleSpecForComponent(nLid, lid3DInfo.strID, lidInfo.strType, 0.f, *spec, lid3DInfo.fOffsetX, lid3DInfo.fOffsetY, fSpecHeight, INVALID);//kircheis_VSV
//																																																	  //result->SetResult(nLidTotalID, INVALID);
//				result->SetRect(nLidTotalID, vec2LidROI[nLid][nROI]);
//				nLidTotalID++;
//				continue;
//			}
//			auto src = vecLid3D[nROI];
//			temp.m_x = src.m_x * px2um_x * fDeadBugGain;
//			temp.m_y = src.m_y * px2um_y_new;
//			temp.m_z = src.m_z * fDeadBugGain;
//
//			fCurHeight = float((planeSub.m_a * temp.m_x + planeSub.m_b * temp.m_y + planeSub.m_c * temp.m_z + planeSub.m_d) / planeNorm);
//			fError = fCurHeight - fSpecHeight;
//
//			result->SetValue_EachResult_AndTypicalValueBySingleSpecForComponent(nLidTotalID, lid3DInfo.strID, lidInfo.strType, fError, *spec, lidInfo.fOffsetX_um, lidInfo.fOffsetY_um, fSpecHeight);
//			result->SetRect(nLidTotalID, vec2LidROI[nLid][nROI]);
//			nLidTotalID++;
//		}
//	}
//	result->UpdateTypicalValue(spec);
//	result->SetTotalResult();
//
//	return TRUE;
//}

BOOL VisionInspectionLid3D::DoInspLidWarpageWithUnitWarpage(std::vector<Ipvm::PlaneEq64r> vecPlaneLid,
    std::vector<std::vector<Ipvm::Rect32s>> vec2LidROI, std::vector<std::vector<Ipvm::Point32r3>> vec2fpt3Lid,
    const bool detailSetupMode)
{
    auto* result = m_resultGroup.GetResultByName(g_szLidInspection3DName[LID_INSPECTION_3D_LID_WARPAGE]);
    if (result == NULL)
    {
        return FALSE;
    }
    auto* spec = GetSpecByName(result->m_resultName);

    auto* resultUnit = m_resultGroup.GetResultByName(g_szLidInspection3DName[LID_INSPECTION_3D_UNIT_LID_WARPAGE]);
    if (resultUnit == NULL)
    {
        return FALSE;
    }
    auto* specUnit = GetSpecByName(resultUnit->m_resultName);

    auto* resultUnitAX = m_resultGroup.GetResultByName(g_szLidInspection3DName[LID_INSPECTION_3D_UNIT_LID_WARPAGE_AX]);
    if (resultUnitAX == NULL)
    {
        return FALSE;
    }
    auto* specUnitAX = GetSpecByName(resultUnitAX->m_resultName);

    auto* resultUnitAV = m_resultGroup.GetResultByName(g_szLidInspection3DName[LID_INSPECTION_3D_UNIT_LID_WARPAGE_AV]);
    if (resultUnitAV == NULL)
    {
        return FALSE;
    }
    auto* specUnitAV = GetSpecByName(resultUnitAV->m_resultName);

    auto* resultUnitAS = m_resultGroup.GetResultByName(g_szLidInspection3DName[LID_INSPECTION_3D_UNIT_LID_WARPAGE_AS]);
    if (resultUnitAS == NULL)
    {
        return FALSE;
    }
    auto* specUnitAS = GetSpecByName(resultUnitAS->m_resultName);

    auto* resultUnitAO = m_resultGroup.GetResultByName(g_szLidInspection3DName[LID_INSPECTION_3D_UNIT_LID_WARPAGE_AO]);
    if (resultUnitAO == NULL)
    {
        return FALSE;
    }
    auto* specUnitAO = GetSpecByName(resultUnitAO->m_resultName);

    if (spec == nullptr || specUnit == nullptr || specUnitAX == nullptr || specUnitAV == nullptr
        || specUnitAS == nullptr || specUnitAO == nullptr)
    {
        return FALSE;
    }

    BOOL bUseUnitAs
        = (specUnitAX->m_use && specUnitAV->m_use && specUnitAS->m_use && specUnitAO->m_use); //kircheis_MED2LW //
    BOOL bUseOrUnitAs
        = (specUnitAX->m_use || specUnitAV->m_use || specUnitAS->m_use || specUnitAO->m_use); //kircheis_MED2LW

    if (detailSetupMode && (!bUseUnitAs && bUseOrUnitAs))
    {
        ::SimpleMessage(_T("All items from [Unit Warpage AX] to [Unit Warpage AO] must be on."));
    }

    if (!spec->m_use && !specUnit->m_use && !bUseUnitAs)
        return TRUE;

    long nLidSpecNum = (long)m_vecsLidInfoDB.size();
    long nLidNum = (long)m_vecsLidAlignResult.size();
    if (nLidSpecNum != nLidNum)
    {
        return FALSE;
    }
    if (nLidNum == 0 || nLidSpecNum == 0)
        return TRUE;

    long nTotalDataNum = 0;
    for (auto vecData : vec2fpt3Lid)
        for (auto data : vecData)
            nTotalDataNum++;

    if (nTotalDataNum < 3)
        return true;

    if (spec->m_use)
        result->Resize(nTotalDataNum);
    if (specUnit->m_use)
        resultUnit->Resize(nLidNum);
    if (specUnitAX->m_use)
        resultUnitAX->Resize(nLidNum);
    if (specUnitAV->m_use)
        resultUnitAV->Resize(nLidNum);
    if (specUnitAS->m_use)
        resultUnitAS->Resize(nLidNum);
    if (specUnitAO->m_use)
        resultUnitAO->Resize(nLidNum);

    const auto& px2um = getScale().pixelToUm();
    const float fDeadBugGain = 1.f; //m_packageSpec.m_deadBug ? -1.f : 1.f;
    const float px2um_y_new = px2um.m_y > 0.f ? (-px2um.m_y) : px2um.m_y;

    CString strSign{};
    CString strShape{};
    double R{};
    double B4_um{};
    double B5_um{};
    double angle_deg{};
    std::vector<VisionWarpageShapeResult> vecWarpageShapeResult(nLidNum);
    std::vector<Ipvm::Point64r3> dataPoints;
    std::vector<std::vector<float>> vec2fWarpageValue(nLidNum);
    std::vector<float> vecfUnitWarpageValue(nLidNum);
    float fMax(-999999.f), fMin(999999.f);
    float fCurWarpageValue{};
    long nLidDataNum{};
    Ipvm::Point32r3 temp{};
    Ipvm::Point64r3 tempPoint{};

    ShapeAlgorithm shapeAlgorithm{};

    if (m_result->m_vecsLid3DGroupInfo_pxl.size() < 1)
        return false;

    for (long nLid = 0; nLid < nLidNum; nLid++)
    {
        dataPoints.clear();
        fMax = -999999.f;
        fMin = 999999.f;
        auto lidAlignResult = m_vecsLidAlignResult[nLid];
        if (lidAlignResult.bAvailable == FALSE)
            continue;

        auto lidInfo = m_vecsLidInfoDB[nLid];
        auto vecLid3D = vec2fpt3Lid[nLid];
        auto planeEq = vecPlaneLid[nLid];
        //const auto& lid3DGroupInfo = m_result->m_vecsLid3DGroupInfo_pxl[nLid];

        nLidDataNum = (long)vecLid3D.size();

        const float planeNorm
            = float(::sqrt(planeEq.m_a * planeEq.m_a + planeEq.m_b * planeEq.m_b + planeEq.m_c * planeEq.m_c));
        //{{기본 Warpage 계산
        for (long nROI = 0; nROI < nLidDataNum; nROI++)
        {
            //const auto& lid3DInfo = lid3DGroupInfo.vecLidROI_Info[nROI];
            auto src = vecLid3D[nROI];
            if (src.m_z == Ipvm::k_noiseValue32r)
            {
                vec2fWarpageValue[nLid].push_back(Ipvm::k_noiseValue32r);
                continue;
            }
            temp.m_x = src.m_x * px2um.m_x * fDeadBugGain;
            temp.m_y = src.m_y * px2um_y_new;
            temp.m_z = src.m_z * fDeadBugGain;

            tempPoint.Set((double)temp.m_x, (double)temp.m_y, (double)temp.m_z);

            temp.m_z = fCurWarpageValue = float(
                (planeEq.m_a * temp.m_x + planeEq.m_b * temp.m_y + planeEq.m_c * temp.m_z + planeEq.m_d) / planeNorm);
            vec2fWarpageValue[nLid].push_back(fCurWarpageValue);
            dataPoints.push_back(tempPoint);
            fMax = (float)max(fMax, fCurWarpageValue);
            fMin = (float)min(fMin, fCurWarpageValue);
        }
        vecfUnitWarpageValue[nLid] = fMax - fMin;
        //}}
        //{{Warpage Shape 계산
        if (shapeAlgorithm.GetShape(dataPoints, strSign, strShape, R, B4_um, B5_um, angle_deg))
            vecWarpageShapeResult[nLid].SetResult(strSign, strShape, R, B4_um, B5_um, angle_deg);
        else
        {
            vecWarpageShapeResult[nLid].Init();
            vecWarpageShapeResult[nLid].m_sign.Format(_T("X"));
        }
        //}}
    }

    //{{Debug Info
    auto* psDebugInfo = m_DebugInfoGroup.GetDebugInfo(_T("Lid Warpage Shape Info"));
    if (psDebugInfo != NULL)
    {
        if (psDebugInfo->pData != NULL)
        {
            psDebugInfo->Reset();
        }
        psDebugInfo->nDataNum = nLidNum;
        VisionWarpageShapeResult* pVisionWarpageShapeResult = new VisionWarpageShapeResult[psDebugInfo->nDataNum];
        for (long i = 0; i < psDebugInfo->nDataNum; i++)
        {
            pVisionWarpageShapeResult[i].SetResult(vecWarpageShapeResult[i]);
        }
        psDebugInfo->pData = pVisionWarpageShapeResult;
    }
    //}}

    if (spec->m_use)
    {
        long nLidTotalID(0);
        for (long nLid = 0; nLid < nLidNum; nLid++)
        {
            auto lidAlignResult = m_vecsLidAlignResult[nLid];
            auto lidInfo = m_vecsLidInfoDB[nLid];
            auto lidWarpPosSpec = m_VisionPara->m_vecsLid3DGroupInfo_um[nLid].vecLidROI_Info;
            long nROINum = (long)min(lidWarpPosSpec.size(), vec2fWarpageValue[nLid].size());
            const auto& lid3DGroupInfo = m_result->m_vecsLid3DGroupInfo_pxl[nLid];

            for (long nROI = 0; nROI < nROINum; nROI++)
            {
                auto lid3DInfo = lid3DGroupInfo.vecLidROI_Info[nROI];

                result->SetValue_EachResult_AndTypicalValueBySingleSpec(nLidTotalID, lidWarpPosSpec[nROI].strID,
                    lidInfo.strType, lidInfo.strType, vec2fWarpageValue[nLid][nROI], *spec,
                    lidWarpPosSpec[nROI].fOffsetX, lidWarpPosSpec[nROI].fOffsetY); //kircheis_MED2.5
                result->SetRect(nLidTotalID, vec2LidROI[nLid][nROI]);
                nLidTotalID++;
            }
        }
        result->UpdateTypicalValue(spec);
        result->SetTotalResult();
    }

    if (specUnit->m_use)
    {
        for (long nLid = 0; nLid < nLidNum; nLid++)
        {
            auto lidAlignResult = m_vecsLidAlignResult[nLid];
            auto lidInfo = m_vecsLidInfoDB[nLid];
            if (lidAlignResult.bAvailable == FALSE)
            {
                resultUnit->SetValue_EachResult_AndTypicalValueBySingleSpec(nLid, lidInfo.strName, lidInfo.strType,
                    lidInfo.strType, 0.f, *specUnit, lidInfo.fOffsetX_um, lidInfo.fOffsetY_um, 0.f,
                    INVALID); //kircheis_VSV //kircheis_MED2.5
                //resultUnit->SetResult(nLid, INVALID);
                resultUnit->SetRect(nLid, lidInfo.rtSpecROI);
                continue;
            }
            resultUnit->SetValue_EachResult_AndTypicalValueBySingleSpec(nLid, lidInfo.strName, lidInfo.strType,
                lidInfo.strType, vecfUnitWarpageValue[nLid], *specUnit, lidInfo.fOffsetX_um,
                lidInfo.fOffsetY_um); //kircheis_MED2.5
            resultUnit->SetRect(nLid, lidAlignResult.getBodyRect32s());
        }
        resultUnit->UpdateTypicalValue(specUnit);
        resultUnit->SetTotalResult();
    }

    if (bUseUnitAs) //kircheis_MED2.5
    {
        for (long nLid = 0; nLid < nLidNum; nLid++)
        {
            auto lidAlignResult = m_vecsLidAlignResult[nLid];
            auto lidInfo = m_vecsLidInfoDB[nLid];
            if (lidAlignResult.bAvailable == FALSE)
            {
                resultUnitAX->SetValue_EachResult_AndTypicalValueBySingleSpec(nLid, lidInfo.strName, lidInfo.strType,
                    lidInfo.strType, 0.f, *specUnitAX, lidInfo.fOffsetX_um, lidInfo.fOffsetY_um, 0.f, INVALID);
                resultUnitAV->SetValue_EachResult_AndTypicalValueBySingleSpec(nLid, lidInfo.strName, lidInfo.strType,
                    lidInfo.strType, 0.f, *specUnitAV, lidInfo.fOffsetX_um, lidInfo.fOffsetY_um, 0.f, INVALID);
                resultUnitAS->SetValue_EachResult_AndTypicalValueBySingleSpec(nLid, lidInfo.strName, lidInfo.strType,
                    lidInfo.strType, 0.f, *specUnitAS, lidInfo.fOffsetX_um, lidInfo.fOffsetY_um, 0.f, INVALID);
                resultUnitAO->SetValue_EachResult_AndTypicalValueBySingleSpec(nLid, lidInfo.strName, lidInfo.strType,
                    lidInfo.strType, 0.f, *specUnitAO, lidInfo.fOffsetX_um, lidInfo.fOffsetY_um, 0.f, INVALID);

                resultUnitAX->SetRect(nLid, lidInfo.rtSpecROI);
                resultUnitAV->SetRect(nLid, lidInfo.rtSpecROI);
                resultUnitAS->SetRect(nLid, lidInfo.rtSpecROI);
                resultUnitAO->SetRect(nLid, lidInfo.rtSpecROI);
                continue;
            }

            auto& warpageShapeResult = vecWarpageShapeResult[nLid];
            BOOL bIsSignPlus = (warpageShapeResult.m_sign == _T("+"));
            float fResultValue = vecfUnitWarpageValue[nLid] * (bIsSignPlus ? 1.f : -1.f);
            if (bIsSignPlus
                && (warpageShapeResult.m_shape == _T("Cylindrical")
                    || warpageShapeResult.m_shape == _T("Spherical"))) //AX
            {
                resultUnitAX->SetValue_EachResult_AndTypicalValueBySingleSpec(nLid, lidInfo.strName, lidInfo.strType,
                    lidInfo.strType, fResultValue, *specUnitAX, lidInfo.fOffsetX_um, lidInfo.fOffsetY_um);
                resultUnitAV->SetValue_EachResult_AndTypicalValueBySingleSpec(nLid, lidInfo.strName, lidInfo.strType,
                    lidInfo.strType, 0.f, *specUnitAV, lidInfo.fOffsetX_um, lidInfo.fOffsetY_um, 0.f, PASS);
                resultUnitAS->SetValue_EachResult_AndTypicalValueBySingleSpec(nLid, lidInfo.strName, lidInfo.strType,
                    lidInfo.strType, 0.f, *specUnitAS, lidInfo.fOffsetX_um, lidInfo.fOffsetY_um, 0.f, PASS);
                resultUnitAO->SetValue_EachResult_AndTypicalValueBySingleSpec(nLid, lidInfo.strName, lidInfo.strType,
                    lidInfo.strType, 0.f, *specUnitAO, lidInfo.fOffsetX_um, lidInfo.fOffsetY_um, 0.f, PASS);
            }
            else if (!bIsSignPlus
                && (warpageShapeResult.m_shape == _T("Cylindrical")
                    || warpageShapeResult.m_shape == _T("Spherical"))) //AV
            {
                resultUnitAX->SetValue_EachResult_AndTypicalValueBySingleSpec(nLid, lidInfo.strName, lidInfo.strType,
                    lidInfo.strType, 0.f, *specUnitAX, lidInfo.fOffsetX_um, lidInfo.fOffsetY_um, 0.f, PASS);
                resultUnitAV->SetValue_EachResult_AndTypicalValueBySingleSpec(nLid, lidInfo.strName, lidInfo.strType,
                    lidInfo.strType, fResultValue, *specUnitAV, lidInfo.fOffsetX_um, lidInfo.fOffsetY_um);
                resultUnitAS->SetValue_EachResult_AndTypicalValueBySingleSpec(nLid, lidInfo.strName, lidInfo.strType,
                    lidInfo.strType, 0.f, *specUnitAS, lidInfo.fOffsetX_um, lidInfo.fOffsetY_um, 0.f, PASS);
                resultUnitAO->SetValue_EachResult_AndTypicalValueBySingleSpec(nLid, lidInfo.strName, lidInfo.strType,
                    lidInfo.strType, 0.f, *specUnitAO, lidInfo.fOffsetX_um, lidInfo.fOffsetY_um, 0.f, PASS);
            }
            else if (warpageShapeResult.m_shape == _T("Saddle")) //AS
            {
                resultUnitAX->SetValue_EachResult_AndTypicalValueBySingleSpec(nLid, lidInfo.strName, lidInfo.strType,
                    lidInfo.strType, 0.f, *specUnitAX, lidInfo.fOffsetX_um, lidInfo.fOffsetY_um, 0.f, PASS);
                resultUnitAV->SetValue_EachResult_AndTypicalValueBySingleSpec(nLid, lidInfo.strName, lidInfo.strType,
                    lidInfo.strType, 0.f, *specUnitAV, lidInfo.fOffsetX_um, lidInfo.fOffsetY_um, 0.f, PASS);
                resultUnitAS->SetValue_EachResult_AndTypicalValueBySingleSpec(nLid, lidInfo.strName, lidInfo.strType,
                    lidInfo.strType, fResultValue, *specUnitAS, lidInfo.fOffsetX_um, lidInfo.fOffsetY_um);
                resultUnitAO->SetValue_EachResult_AndTypicalValueBySingleSpec(nLid, lidInfo.strName, lidInfo.strType,
                    lidInfo.strType, 0.f, *specUnitAO, lidInfo.fOffsetX_um, lidInfo.fOffsetY_um, 0.f, PASS);
            }
            else //AO
            {
                resultUnitAX->SetValue_EachResult_AndTypicalValueBySingleSpec(nLid, lidInfo.strName, lidInfo.strType,
                    lidInfo.strType, 0.f, *specUnitAX, lidInfo.fOffsetX_um, lidInfo.fOffsetY_um, 0.f, PASS);
                resultUnitAV->SetValue_EachResult_AndTypicalValueBySingleSpec(nLid, lidInfo.strName, lidInfo.strType,
                    lidInfo.strType, 0.f, *specUnitAV, lidInfo.fOffsetX_um, lidInfo.fOffsetY_um, 0.f, PASS);
                resultUnitAS->SetValue_EachResult_AndTypicalValueBySingleSpec(nLid, lidInfo.strName, lidInfo.strType,
                    lidInfo.strType, 0.f, *specUnitAS, lidInfo.fOffsetX_um, lidInfo.fOffsetY_um, 0.f, PASS);
                resultUnitAO->SetValue_EachResult_AndTypicalValueBySingleSpec(nLid, lidInfo.strName, lidInfo.strType,
                    lidInfo.strType, fResultValue, *specUnitAO, lidInfo.fOffsetX_um, lidInfo.fOffsetY_um);
            }
            resultUnitAX->SetRect(nLid, lidAlignResult.getBodyRect32s());
            resultUnitAV->SetRect(nLid, lidAlignResult.getBodyRect32s());
            resultUnitAS->SetRect(nLid, lidAlignResult.getBodyRect32s());
            resultUnitAO->SetRect(nLid, lidAlignResult.getBodyRect32s());
        }
        resultUnitAX->UpdateTypicalValue(specUnitAX);
        resultUnitAV->UpdateTypicalValue(specUnitAV);
        resultUnitAS->UpdateTypicalValue(specUnitAS);
        resultUnitAO->UpdateTypicalValue(specUnitAO);

        resultUnitAX->SetTotalResult();
        resultUnitAV->SetTotalResult();
        resultUnitAS->SetTotalResult();
        resultUnitAO->SetTotalResult();
    }

    strSign.Empty();
    strShape.Empty();

    return TRUE;
}

void VisionInspectionLid3D::MakeBasicROI(VisionAlignResult alignResult, float fOffsetX_um, float fOffsetY_um,
    float nSize_um, std::vector<Ipvm::Rect32s>& o_vecrtBasicROI)
{
    o_vecrtBasicROI.clear();

    if (alignResult.bAvailable == FALSE)
        return;

    const auto& um2px = getScale().umToPixel();

    const float fOffsetX_Pxl = (float)max(fOffsetX_um, 0.f) * um2px.m_x;
    const float fOffsetY_Pxl = (float)max(fOffsetY_um, 0.f) * um2px.m_y;

    const float fSizeX_Pxl = (float)max(nSize_um, 100.f) * um2px.m_x;
    const float fSizeY_Pxl = (float)max(nSize_um, 100.f) * um2px.m_y;

    o_vecrtBasicROI.resize(4);

    //LT : 0
    o_vecrtBasicROI[0].m_left = (long)(alignResult.fptLT.m_x + fOffsetX_Pxl + .5f);
    o_vecrtBasicROI[0].m_top = (long)(alignResult.fptLT.m_y + fOffsetY_Pxl + .5f);
    o_vecrtBasicROI[0].m_right = (long)(alignResult.fptLT.m_x + fOffsetX_Pxl + fSizeX_Pxl + .5f);
    o_vecrtBasicROI[0].m_bottom = (long)(alignResult.fptLT.m_y + fOffsetY_Pxl + fSizeY_Pxl + .5f);

    //RT : 1
    o_vecrtBasicROI[1].m_left = (long)(alignResult.fptRT.m_x - fOffsetX_Pxl - fSizeX_Pxl + .5f);
    o_vecrtBasicROI[1].m_top = (long)(alignResult.fptRT.m_y + fOffsetY_Pxl + .5f);
    o_vecrtBasicROI[1].m_right = (long)(alignResult.fptRT.m_x - fOffsetX_Pxl + .5f);
    o_vecrtBasicROI[1].m_bottom = (long)(alignResult.fptRT.m_y + fOffsetY_Pxl + fSizeY_Pxl + .5f);

    //RB : 2
    o_vecrtBasicROI[2].m_left = (long)(alignResult.fptRB.m_x - fOffsetX_Pxl - fSizeX_Pxl + .5f);
    o_vecrtBasicROI[2].m_top = (long)(alignResult.fptRB.m_y - fOffsetY_Pxl - fSizeY_Pxl + .5f);
    o_vecrtBasicROI[2].m_right = (long)(alignResult.fptRB.m_x - fOffsetX_Pxl + .5f);
    o_vecrtBasicROI[2].m_bottom = (long)(alignResult.fptRB.m_y - fOffsetY_Pxl + .5f);

    //LB : 3
    o_vecrtBasicROI[3].m_left = (long)(alignResult.fptLB.m_x + fOffsetX_Pxl + .5f);
    o_vecrtBasicROI[3].m_top = (long)(alignResult.fptLB.m_y - fOffsetY_Pxl - fSizeY_Pxl + .5f);
    o_vecrtBasicROI[3].m_right = (long)(alignResult.fptLB.m_x + fOffsetX_Pxl + fSizeX_Pxl + .5f);
    o_vecrtBasicROI[3].m_bottom = (long)(alignResult.fptLB.m_y - fOffsetY_Pxl + .5f);
}

void VisionInspectionLid3D::MakeLidROI(long nLidID, std::vector<Ipvm::Rect32s>& o_vecrtLidROI)
{
    o_vecrtLidROI.clear();
    if (nLidID >= m_vecsLidAlignResult.size() || nLidID >= m_result->m_vecsLid3DGroupInfo_pxl.size()
        || nLidID >= m_veclineLidCenterHor.size())
        return;

    auto alignResult = m_vecsLidAlignResult[nLidID];
    auto lidCenterHor = m_veclineLidCenterHor[nLidID];
    const auto& lid3DGroupInfo = m_result->m_vecsLid3DGroupInfo_pxl[nLidID];

    if (alignResult.bAvailable == FALSE || lid3DGroupInfo.vecLidROI_Info.size() <= 0)
        return;

    float fLidAngle = (float)atan(-lidCenterHor.m_a / lidCenterHor.m_b);

    long nROINum = (long)lid3DGroupInfo.vecLidROI_Info.size();
    Ipvm::Point32r2 fptLidCenter = alignResult.m_center;

    Ipvm::Rect32s rtROI;
    long nHalfSizeX, nHalfSizeY;
    long nOffsetX, nOffsetY;

    FPI_RECT frtLid;
    frtLid.fptLT = alignResult.fptLT;
    frtLid.fptLB = alignResult.fptLB;
    frtLid.fptRT = alignResult.fptRT;
    frtLid.fptRB = alignResult.fptRB;
    Ipvm::Rect32s rtLid = frtLid.GetCRect();
    Ipvm::Rect32s rtIntersect;

    for (long nROI = 0; nROI < nROINum; nROI++)
    {
        rtIntersect.SetRectEmpty();
        nOffsetX = (long)(lid3DGroupInfo.vecLidROI_Info[nROI].fOffsetX + fptLidCenter.m_x + .5f);
        nOffsetY = (long)(lid3DGroupInfo.vecLidROI_Info[nROI].fOffsetY + fptLidCenter.m_y + .5f);
        nHalfSizeX = (long)(lid3DGroupInfo.vecLidROI_Info[nROI].fWidth * .5f + .5f);
        nHalfSizeY = (long)(lid3DGroupInfo.vecLidROI_Info[nROI].fLength * .5f + .5f);
        rtROI = Ipvm::Rect32s(int32_t(nOffsetX - nHalfSizeX), int32_t(nOffsetY - nHalfSizeY),
            int32_t(nOffsetX + nHalfSizeX), int32_t(nOffsetY + nHalfSizeY));
        CPI_Geometry::RotateRect(rtROI, fLidAngle, Ipvm::Conversion::ToPoint32s2(fptLidCenter));

        if (rtIntersect.IntersectRect(rtLid, rtROI))
        {
            if (!rtIntersect.IsRectEmpty() && rtIntersect == rtROI)
                o_vecrtLidROI.push_back(rtROI);
        }
    }
}

void VisionInspectionLid3D::Get3DPosData(std::vector<Ipvm::Rect32s> vecrtROI,
    std::vector<Ipvm::Point32r3>& o_vecfpt3Pos) //출력 변수 구조체 xyz 중 xy는 Pxl Rect Center, z는 실좌표를 쓴다.
{
    //const long nROINum = (long)vecrtROI.size();
    o_vecfpt3Pos.clear();
    Ipvm::Point32r3 fpt3Result;

    for (auto rtROI : vecrtROI)
    {
        CalcLidOrSubstrateZ(rtROI, fpt3Result);
        o_vecfpt3Pos.push_back(fpt3Result);
    }
}

void VisionInspectionLid3D::CalcLidOrSubstrateZ(
    Ipvm::Rect32s rtROI, Ipvm::Point32r3& o_fpt3Pos) //출력 변수 구조체 xyz 중 xy는 Pxl Rect Center, z는 실좌표를 쓴다.
{
    const auto& px2um = getScale().pixelToUm();

    o_fpt3Pos = Ipvm::Point32r3((float)rtROI.CenterPoint().m_x, (float)rtROI.CenterPoint().m_y, Ipvm::k_noiseValue32r);

    const long allPixelCount = rtROI.Width() * rtROI.Height();
    const long desiredMaxPixelCount = 4000;

    // 소팅시 계산량을 줄이기 위해서 샘플링을 시도한다.
    const long samplingStep = max(1, long(sqrt((allPixelCount + desiredMaxPixelCount - 1) / desiredMaxPixelCount)));

    long dataCount = 0;

    std::vector<Ipvm::Point32r3> vecfpt3Points(0);
    Ipvm::Point32r3 fpt3Point;

    for (long y = rtROI.m_top; y < rtROI.m_bottom; y += samplingStep)
    {
        const float* zmap = getImageLotInsp().m_zmapImage.GetMem(0, y);

        const float y_um = y * px2um.m_y;

        for (long x = rtROI.m_left; x < rtROI.m_right; x += samplingStep)
        {
            if (zmap[x] == Ipvm::k_noiseValue32r)
                continue;

            fpt3Point.m_x = x * px2um.m_x;
            fpt3Point.m_y = y_um;
            fpt3Point.m_z = zmap[x];
            vecfpt3Points.push_back(fpt3Point);
            dataCount++;
        }
    }

    if (dataCount < 3)
        return;

    Ipvm::PlaneEq64r planeEq = {0., 0., 0., 0.};

    if (Ipvm::Status::e_ok != Ipvm::DataFitting::FitToPlane(dataCount, &vecfpt3Points[0], planeEq))
        return;

    const float centerx_um = 0.5f * (rtROI.m_right - 1 + rtROI.m_left) * px2um.m_x;
    const float centery_um = 0.5f * (rtROI.m_bottom - 1 + rtROI.m_top) * px2um.m_y;

    o_fpt3Pos.m_z = (float)planeEq.GetPositionZ((double)centerx_um, (double)centery_um);

    //{{
    float fSum = 0;
    for (auto fData : vecfpt3Points)
        fSum += fData.m_z;

    fSum /= (float)vecfpt3Points.size();

    static BOOL bTest = FALSE;
    if (bTest)
        o_fpt3Pos.m_z = fSum;
    //}}
}

BOOL VisionInspectionLid3D::Get3DPlane(std::vector<Ipvm::Point32r3> vecfpt3Pos, Ipvm::PlaneEq64r& o_plane)
{
    const auto& px2um = getScale().pixelToUm();

    const long pointCount = (long)vecfpt3Pos.size();

    o_plane = {0., 0., 0., 0.};

    if (pointCount <= 0)
        return FALSE;

    // 오른손 좌표계를 사용하기 위해서 Y 에 -1 곱해줌
    const float px2um_y_new = px2um.m_y > 0.f ? (-px2um.m_y) : px2um.m_y;

    std::vector<Ipvm::Point32r3> vecPoints;
    vecPoints.reserve(pointCount);

    Ipvm::Point32r3 temp;

    const float fDeadBugGain = 1.f; //m_packageSpec.m_deadBug ? -1.f : 1.f;

    for (int idx = 0; idx < pointCount; idx++)
    {
        const auto& src = vecfpt3Pos[idx];

        if (src.m_z == Ipvm::k_noiseValue32r)
        {
            continue;
        }

        // 단위계를 실 단위계로 바꿔 주고...
        temp.m_x = src.m_x * px2um.m_x * fDeadBugGain;
        temp.m_y = src.m_y * px2um_y_new;
        temp.m_z = src.m_z * fDeadBugGain;

        vecPoints.push_back(temp);
    }

    if (vecPoints.size() < 3)
        return FALSE;

    // 기준 평면을 계산한다.
    Ipvm::PlaneEq64r planeEq = {0., 0., 0., 0.};

    if (Ipvm::Status::e_ok != Ipvm::DataFitting::FitToPlane((long)vecPoints.size(), &vecPoints[0], planeEq))
        return FALSE;

    o_plane = planeEq;

    return TRUE;
}
