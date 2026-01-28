//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionInspectionPatch3D.h"

//CPP_2_________________________________ This project's headers
#include "DlgVisionInspectionPatch3D.h"
#include "Result.h"
#include "VisionInspectionPatch3DPara.h"

//CPP_3_________________________________ Other projects' headers
#include "../../AlgorithmModules/dPI_EdgeDetect/dPI_EdgeDetectLib.h"
#include "../../DefineModules/dA_Base/FPI_RECT.h"
#include "../../DefineModules/dA_Base/VisionScale.h"
#include "../../DefineModules/dA_Base/iDataType.h"
#include "../../DefineModules/dA_Base/semiinfo.h"
#include "../../ImageCombineModules/dPI_ippModules/ippModules.h"
#include "../../InformationModule/dPI_DataBase/DMSPatch.h"
#include "../../InformationModule/dPI_DataBase/PackageSpec.h"
#include "../../MemoryModules/VisionReusableMemory/SurfaceLayerRoi.h"
#include "../../MemoryModules/VisionReusableMemory/VisionReusableMemory.h"
#include "../../SharedCommunicationModules/VisionHostCommon/VisionHostBaseDef.h"
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

void VisionInspectionPatch3D::AbsoluteRectToImageRectByBodyCenter(Ipvm::Rect32s& rtROI)
{
    Ipvm::Rect32r frtBodyRect = GetBodyRect();
    AbsoluteRectToImageRectByBodyCenter(frtBodyRect, rtROI);
}

void VisionInspectionPatch3D::AbsoluteRectToImageRectByBodyCenter(Ipvm::Rect32r frtBodyRect, Ipvm::Rect32s& rtROI)
{
    Ipvm::Point32r2 fptCenter = frtBodyRect.CenterPoint();
    rtROI.m_left += (long)(fptCenter.m_x + .5f);
    rtROI.m_top += (long)(fptCenter.m_y + .5f);
    rtROI.m_right += (long)(fptCenter.m_x + .5f);
    rtROI.m_bottom += (long)(fptCenter.m_y + .5f);
}

void VisionInspectionPatch3D::ImageRectToAbsoluteRectByBodyCenter(Ipvm::Rect32s& rtROI)
{
    Ipvm::Rect32r frtBodyRect = GetBodyRect();
    ImageRectToAbsoluteRectByBodyCenter(frtBodyRect, rtROI);
}

void VisionInspectionPatch3D::ImageRectToAbsoluteRectByBodyCenter(Ipvm::Rect32r frtBodyRect, Ipvm::Rect32s& rtROI)
{
    Ipvm::Point32r2 fptCenter = frtBodyRect.CenterPoint();
    rtROI.m_left -= (long)(fptCenter.m_x + .5f);
    rtROI.m_top -= (long)(fptCenter.m_y + .5f);
    rtROI.m_right -= (long)(fptCenter.m_x + .5f);
    rtROI.m_bottom -= (long)(fptCenter.m_y + .5f);
}

Ipvm::Rect32r VisionInspectionPatch3D::GetBodyRect()
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

BOOL VisionInspectionPatch3D::OnInspection()
{
    m_pVisionInspDlg->OnBnClickedButtonInspect();

    return TRUE;
}

BOOL VisionInspectionPatch3D::DoInspection(const bool detailSetupMode, const enSideVisionModule i_ModuleStatus)
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

    if (!DoAlignPatch(detailSetupMode))
        return FALSE;

    long nPatchNum = (long)m_vecsPatchInfoDB.size();
    std::vector<Ipvm::Rect32s> vecrtInterposerROI(0);
    std::vector<std::vector<Ipvm::Rect32s>> vec2rtPatchHeightROI(nPatchNum);
    std::vector<std::vector<Ipvm::Rect32s>> vec2rtPatchROI(nPatchNum);

    std::vector<Ipvm::Rect32s> vecrtDebugPatchTiltROI(0);
    std::vector<Ipvm::Rect32s> vecrtDebugPatchROI(0);

    std::vector<Ipvm::Point32r3> vecfpt3InterposerPos(0);
    std::vector<std::vector<Ipvm::Point32r3>> vec2fpt3PatchHeightPos(nPatchNum);
    std::vector<std::vector<Ipvm::Point32r3>> vec2fpt3PatchPos(nPatchNum);

    Ipvm::PlaneEq64r planeInterposer;
    std::vector<Ipvm::PlaneEq64r> vecPlanePatchTilt(nPatchNum);
    std::vector<Ipvm::PlaneEq64r> vecPlanePatch(nPatchNum);

    const float fPatchHeightROI_OffsetX_um((float)m_VisionPara->m_nPatchHeightROI_OffsetX_um);
    const float fPatchHeightROI_OffsetY_um((float)m_VisionPara->m_nPatchHeightROI_OffsetY_um);
    const float fPatchHeightROI_size_um((float)m_VisionPara->m_nPatchHeightROI_size_um);

    //Interposer ROI
    MakeInterposerROI(m_result->m_vecsInterposerRefROI_Info_pxl, vecrtInterposerROI);
    Get3DPosData(vecrtInterposerROI, vecfpt3InterposerPos);
    Get3DPlane(vecfpt3InterposerPos, planeInterposer);

    for (long nPatch = 0; nPatch < nPatchNum; nPatch++)
    {
        MakePatchROI(m_vecsPatchAlignResult[nPatch], fPatchHeightROI_OffsetX_um, fPatchHeightROI_OffsetY_um,
            fPatchHeightROI_size_um, vec2rtPatchHeightROI[nPatch]);
        vecrtDebugPatchTiltROI.insert(
            vecrtDebugPatchTiltROI.begin(), vec2rtPatchHeightROI[nPatch].begin(), vec2rtPatchHeightROI[nPatch].end());
        Get3DPosData(vec2rtPatchHeightROI[nPatch], vec2fpt3PatchHeightPos[nPatch]);
        Get3DPlane(vec2fpt3PatchHeightPos[nPatch], vecPlanePatchTilt[nPatch]);
    }

    bResult &= DoInspPatchHeight(planeInterposer, vec2rtPatchHeightROI, vec2fpt3PatchHeightPos);

    //{{DebugInfo
    SetDebugInfoItem(detailSetupMode, _T("Interposer ROI"), vecrtInterposerROI);
    SetDebugInfoItem(detailSetupMode, _T("Patch Height ROI"), vecrtDebugPatchTiltROI);
    //SetDebugInfoItem(detailSetupMode, _T("Patch Warpage ROI"), vecrtDebugPatchROI);
    //}}

    m_fCalcTime = CAST_FLOAT(time_Insp.Elapsed_ms());

    return bResult;
}

BOOL VisionInspectionPatch3D::GetPatchInfoDB()
{
    m_vecsPatchInfoDB.clear();

    static const float fMMtoum(1000.f);
    const auto& um2px = getScale().umToPixel();

    Ipvm::Point32r2 fptPackageCenter_Pxl = m_sEdgeAlignResult->m_center;

    for (auto PatchInfo_MapData : m_packageSpec.vecDMSPatch)
    {
        SDieLidPatchInfoDB sPatchInfo;
        sPatchInfo.strName = PatchInfo_MapData.strCompID;
        sPatchInfo.strType = PatchInfo_MapData.strCompType;
        sPatchInfo.fOffsetX_um = PatchInfo_MapData.fChipOffsetX * fMMtoum;
        sPatchInfo.fOffsetY_um = -PatchInfo_MapData.fChipOffsetY * fMMtoum;
        sPatchInfo.fAngle_Deg = PatchInfo_MapData.fChipAngle;

        sPatchInfo.fWidth_um = PatchInfo_MapData.fChipWidth * fMMtoum;
        sPatchInfo.fLength_um = PatchInfo_MapData.fChipLength * fMMtoum;
        sPatchInfo.fHeight_um = PatchInfo_MapData.fChipThickness * fMMtoum;

        float fPatchOffsetX_Pxl = sPatchInfo.fOffsetX_um * um2px.m_x;
        float fPatchOffsetY_Pxl = sPatchInfo.fOffsetY_um * um2px.m_y;
        float fPatchSizeX_Pxl = sPatchInfo.fWidth_um * um2px.m_x;
        float fPatchSizeY_Pxl = sPatchInfo.fLength_um * um2px.m_y;
        float fPatchSizeHalfX_Pxl = fPatchSizeX_Pxl * .5f;
        float fPatchSizeHalfY_Pxl = fPatchSizeY_Pxl * .5f;

        sPatchInfo.rtSpecROI.m_left = (long)(fptPackageCenter_Pxl.m_x + fPatchOffsetX_Pxl - fPatchSizeHalfX_Pxl + .5f);
        sPatchInfo.rtSpecROI.m_right = (long)(fptPackageCenter_Pxl.m_x + fPatchOffsetX_Pxl + fPatchSizeHalfX_Pxl + .5f);
        sPatchInfo.rtSpecROI.m_top = (long)(fptPackageCenter_Pxl.m_y + fPatchOffsetY_Pxl - fPatchSizeHalfY_Pxl + .5f);
        sPatchInfo.rtSpecROI.m_bottom
            = (long)(fptPackageCenter_Pxl.m_y + fPatchOffsetY_Pxl + fPatchSizeHalfY_Pxl + .5f);

        m_vecsPatchInfoDB.push_back(sPatchInfo);
    }

    return (m_vecsPatchInfoDB.size() > 0);
}

BOOL VisionInspectionPatch3D::CalcAlignedCenterLine(
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

BOOL VisionInspectionPatch3D::DoAlignPatch(const bool detailSetupMode)
{
    m_vecsPatchAlignResult.clear();
    m_veclinePatchCenterHor.clear();
    m_veclinePatchCenterVer.clear();

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

    //Patch 정보 가져오기
    if (GetPatchInfoDB() == FALSE)
        return FALSE;
    long nPatchNum = (long)m_vecsPatchInfoDB.size();

    //실제 Align 구간
    std::vector<Ipvm::Rect32s> vecrtEdgeSearchROI(4);
    std::vector<Ipvm::Point32r2> vecfptLeftEdge(0);
    std::vector<Ipvm::Point32r2> vecfptTopEdge(0);
    std::vector<Ipvm::Point32r2> vecfptRightEdge(0);
    std::vector<Ipvm::Point32r2> vecfptBottomEdge(0);
    BOOL bAlignResult = TRUE;
    Ipvm::LineEq32r lineHor, lineVer;

    std::vector<float> vecfPatchHeightUM(0);

    for (long nPatch = 0; nPatch < nPatchNum; nPatch++)
    {
        VisionAlignResult patchAlignResult;
        PI_RECT piAlignRect;
        patchAlignResult.bAvailable = FALSE;

        vecfPatchHeightUM.push_back(m_vecsPatchInfoDB[nPatch].fHeight_um);

        //Make Search ROI
        MakeEdgeSearchROI(m_vecsPatchInfoDB[nPatch], vecrtEdgeSearchROI);
        vecrtSpecROI.push_back(m_vecsPatchInfoDB[nPatch].rtSpecROI);
        vecrtSearchROI.push_back(vecrtEdgeSearchROI[LEFT]); //Align ROI의 순서를 맞추기 위해 insert를 안쓴거임
        vecrtSearchROI.push_back(vecrtEdgeSearchROI[UP]);
        vecrtSearchROI.push_back(vecrtEdgeSearchROI[RIGHT]);
        vecrtSearchROI.push_back(vecrtEdgeSearchROI[DOWN]);

        //Edge Detect Left
        GetPatchEdgePoint(combineImage, LEFT, vecrtEdgeSearchROI[LEFT], vecfptLeftEdge);
        vecfptDebugLeftEdge.insert(vecfptDebugLeftEdge.end(), vecfptLeftEdge.begin(), vecfptLeftEdge.end());
        if (vecfptLeftEdge.size() <= 3)
            bAlignResult = FALSE;

        //Edge Detect Top
        GetPatchEdgePoint(combineImage, UP, vecrtEdgeSearchROI[UP], vecfptTopEdge);
        vecfptDebugTopEdge.insert(vecfptDebugTopEdge.end(), vecfptTopEdge.begin(), vecfptTopEdge.end());
        if (vecfptTopEdge.size() <= 3)
            bAlignResult = FALSE;

        //Edge Detect Right
        GetPatchEdgePoint(combineImage, RIGHT, vecrtEdgeSearchROI[RIGHT], vecfptRightEdge);
        vecfptDebugRightEdge.insert(vecfptDebugRightEdge.end(), vecfptRightEdge.begin(), vecfptRightEdge.end());
        if (vecfptRightEdge.size() <= 3)
            bAlignResult = FALSE;

        //Edge Detect bottom
        GetPatchEdgePoint(combineImage, DOWN, vecrtEdgeSearchROI[DOWN], vecfptBottomEdge);
        vecfptDebugBottomEdge.insert(vecfptDebugBottomEdge.end(), vecfptBottomEdge.begin(), vecfptBottomEdge.end());
        if (vecfptBottomEdge.size() <= 3)
            bAlignResult = FALSE;

        //Line Fitting and Make Cross Point

        if (CalcPatchAlignResult(vecfptLeftEdge, vecfptTopEdge, vecfptRightEdge, vecfptBottomEdge, patchAlignResult))
        {
            m_vecsPatchAlignResult.push_back(patchAlignResult);

            vecfrtAlignResult.push_back(patchAlignResult.getBodyRect());
            piAlignRect.SetLTPoint(Ipvm::Conversion::ToPoint32s2(patchAlignResult.fptLT));
            piAlignRect.SetRTPoint(Ipvm::Conversion::ToPoint32s2(patchAlignResult.fptRT));
            piAlignRect.SetRBPoint(Ipvm::Conversion::ToPoint32s2(patchAlignResult.fptRB));
            piAlignRect.SetLBPoint(Ipvm::Conversion::ToPoint32s2(patchAlignResult.fptLB));
            vecprtAlignResult.push_back(piAlignRect);

            CalcAlignedCenterLine(patchAlignResult, lineHor, lineVer);
            m_veclinePatchCenterHor.push_back(lineHor);
            m_veclinePatchCenterVer.push_back(lineVer);
        }
        else
            bAlignResult = FALSE;

        vecfptLeftEdge.clear();
        vecfptTopEdge.clear();
        vecfptRightEdge.clear();
        vecfptBottomEdge.clear();
    }

    SetDebugInfoItem(detailSetupMode, _T("Patch Spec Height (um)"), vecfPatchHeightUM);
    SetDebugInfoItem(detailSetupMode, _T("Patch Spec"), vecrtSpecROI);
    SetDebugInfoItem(detailSetupMode, _T("Patch Edge Search ROI"), vecrtSearchROI);
    SetDebugInfoItem(detailSetupMode, _T("Patch Edge Point Left"), vecfptDebugLeftEdge);
    SetDebugInfoItem(detailSetupMode, _T("Patch Edge Point Top"), vecfptDebugTopEdge);
    SetDebugInfoItem(detailSetupMode, _T("Patch Edge Point Right"), vecfptDebugRightEdge);
    SetDebugInfoItem(detailSetupMode, _T("Patch Edge Point Bottom"), vecfptDebugBottomEdge);
    SetDebugInfoItem(detailSetupMode, _T("Detect Patch Rect"), vecfrtAlignResult, TRUE);

    //{{//kircheis_USI_OA
    auto* psDebugInfo = m_DebugInfoGroup.GetDebugInfo(_T("Detect Patch ROI"));
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
        name.Format(_T("Patch Align Result [%s]"), (LPCTSTR)m_vecsPatchInfoDB[i].strName);

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

BOOL VisionInspectionPatch3D::MakeEdgeSearchROI(
    SDieLidPatchInfoDB patchInfoDB, std::vector<Ipvm::Rect32s>& o_vecrtSearchROI)
{
    const auto& um2px = getScale().umToPixel();
    const float fWidthRatio = (float)max(0.1f, min(1.f, m_VisionPara->m_nEdgeSearchWidthRatio * 0.01f));
    const long nSearchLengthHalfLR_Pxl = (long)(m_VisionPara->m_nEdgeSearchLength_um * um2px.m_x * .5f + .5f);
    const long nSearchLengthHalfTB_Pxl = (long)(m_VisionPara->m_nEdgeSearchLength_um * um2px.m_y * .5f + .5f);

    o_vecrtSearchROI.clear();
    o_vecrtSearchROI.resize(4);

    Ipvm::Rect32s rtPatchSpecROI = patchInfoDB.rtSpecROI;

    long nSearchWidthHalfLR_Pxl = (long)((float)rtPatchSpecROI.Height() * (1.f - fWidthRatio) * .5f + .5f);
    long nSearchWidthHalfTB_Pxl = (long)((float)rtPatchSpecROI.Width() * (1.f - fWidthRatio) * .5f + .5f);

    //LEFT
    o_vecrtSearchROI[LEFT] = rtPatchSpecROI;
    o_vecrtSearchROI[LEFT].m_left -= nSearchLengthHalfLR_Pxl;
    o_vecrtSearchROI[LEFT].m_right = rtPatchSpecROI.m_left + nSearchLengthHalfLR_Pxl;
    o_vecrtSearchROI[LEFT].m_top += nSearchWidthHalfLR_Pxl;
    o_vecrtSearchROI[LEFT].m_bottom -= nSearchWidthHalfLR_Pxl;

    //Top
    o_vecrtSearchROI[UP] = rtPatchSpecROI;
    o_vecrtSearchROI[UP].m_left += nSearchWidthHalfTB_Pxl;
    o_vecrtSearchROI[UP].m_right -= nSearchWidthHalfTB_Pxl;
    o_vecrtSearchROI[UP].m_top -= nSearchLengthHalfTB_Pxl;
    o_vecrtSearchROI[UP].m_bottom = rtPatchSpecROI.m_top + nSearchLengthHalfTB_Pxl;

    //RIGHT
    o_vecrtSearchROI[RIGHT] = rtPatchSpecROI;
    o_vecrtSearchROI[RIGHT].m_left = rtPatchSpecROI.m_right - nSearchLengthHalfLR_Pxl;
    o_vecrtSearchROI[RIGHT].m_right += nSearchLengthHalfLR_Pxl;
    o_vecrtSearchROI[RIGHT].m_top += nSearchWidthHalfLR_Pxl;
    o_vecrtSearchROI[RIGHT].m_bottom -= nSearchWidthHalfLR_Pxl;

    //BOTTOM
    o_vecrtSearchROI[DOWN] = rtPatchSpecROI;
    o_vecrtSearchROI[DOWN].m_left += nSearchWidthHalfTB_Pxl;
    o_vecrtSearchROI[DOWN].m_right -= nSearchWidthHalfTB_Pxl;
    o_vecrtSearchROI[DOWN].m_top = rtPatchSpecROI.m_bottom - nSearchLengthHalfTB_Pxl;
    o_vecrtSearchROI[DOWN].m_bottom += nSearchLengthHalfTB_Pxl;

    return TRUE;
}

BOOL VisionInspectionPatch3D::GetPatchEdgePoint(
    Ipvm::Image8u image, long nDir, Ipvm::Rect32s rtSearchROI, std::vector<Ipvm::Point32r2>& o_vecfptEdge)
{
    const auto& um2px = getScale().umToPixel();

    o_vecfptEdge.clear();

    long nEdgeGap(0), nSearchLength(0), nSearchWidth(0);
    long nEdgeDirection = m_VisionPara->m_nEdgeDirection;
    long nSearchDir = nDir;
    BOOL bFirstEdge = m_VisionPara->m_nEdgeDetectMode == PI_ED_FIRST_EDGE;
    Ipvm::Point32r2 bestEdgeXY;
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

BOOL VisionInspectionPatch3D::CalcPatchAlignResult(std::vector<Ipvm::Point32r2> vecfptLeftEdge,
    std::vector<Ipvm::Point32r2> vecfptTopEdge, std::vector<Ipvm::Point32r2> vecfptRightEdge,
    std::vector<Ipvm::Point32r2> vecfptBottomEdge, VisionAlignResult& o_patchAlignResult)
{
    o_patchAlignResult.bAvailable = FALSE;

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
    Ipvm::Point32r2& ptLeftTop = o_patchAlignResult.fptLT;
    Ipvm::Point32r2& ptRightTop = o_patchAlignResult.fptRT;
    Ipvm::Point32r2& ptRightBottom = o_patchAlignResult.fptRB;
    Ipvm::Point32r2& ptLeftBottom = o_patchAlignResult.fptLB;

    if (Ipvm::Geometry::GetCrossPoint(vecsLineEq[LEFT], vecsLineEq[UP], ptLeftTop) != Ipvm::Status::e_ok)
        return FALSE;
    if (Ipvm::Geometry::GetCrossPoint(vecsLineEq[UP], vecsLineEq[RIGHT], ptRightTop) != Ipvm::Status::e_ok)
        return FALSE;
    if (Ipvm::Geometry::GetCrossPoint(vecsLineEq[RIGHT], vecsLineEq[DOWN], ptRightBottom) != Ipvm::Status::e_ok)
        return FALSE;
    if (Ipvm::Geometry::GetCrossPoint(vecsLineEq[DOWN], vecsLineEq[LEFT], ptLeftBottom) != Ipvm::Status::e_ok)
        return FALSE;

    // 네 모서리로부터 바디 센터를 얻는다.
    o_patchAlignResult.m_center.m_x = (ptLeftTop.m_x + ptRightTop.m_x + ptRightBottom.m_x + ptLeftBottom.m_x) * 0.25f;
    o_patchAlignResult.m_center.m_y = (ptLeftTop.m_y + ptRightTop.m_y + ptRightBottom.m_y + ptLeftBottom.m_y) * 0.25f;

    // 네 모서리로부터 바디 앵글을 얻는다.
    const float angleLower = (float)atan2(ptRightBottom.m_y - ptLeftBottom.m_y, ptRightBottom.m_x - ptLeftBottom.m_x);
    const float angleUpper = (float)atan2(ptRightTop.m_y - ptLeftTop.m_y, ptRightTop.m_x - ptLeftTop.m_x);

    float fTopBottomAngle = (angleLower + angleUpper) * 0.5f;

    float fAngleLeft = CAST_FLOAT(
        atan2(ptLeftBottom.m_y - ptLeftTop.m_y, ptLeftBottom.m_x - ptLeftTop.m_x) - (90. * 0.017453292519943));
    float fAngleRight = CAST_FLOAT(
        atan2(ptRightBottom.m_y - ptRightTop.m_y, ptRightBottom.m_x - ptRightTop.m_x) - (90. * 0.017453292519943));
    float fLeftRightAngle = (fAngleLeft + fAngleRight) * .5f;
    o_patchAlignResult.SetAngle((fTopBottomAngle + fLeftRightAngle) * 0.5f);

    o_patchAlignResult.bAvailable = TRUE;

    return TRUE;
}

BOOL VisionInspectionPatch3D::DoInspPatchHeight(Ipvm::PlaneEq64r planeInterposer,
    std::vector<std::vector<Ipvm::Rect32s>> vec2PatchROI,
    std::vector<std::vector<Ipvm::Point32r3>> vec2fpt3Patch) //Unit Level 검사 Code
{
    auto* result = m_resultGroup.GetResultByName(g_szPatchInspection3DName[PATCH_INSPECTION_3D_PATCH_HEIGHT]);
    if (result == nullptr)
        return FALSE;
    auto* spec = GetSpecByName(result->m_resultName);

    auto* resultUnit = m_resultGroup.GetResultByName(g_szPatchInspection3DName[PATCH_INSPECTION_3D_UNIT_PATCH_HIGHT]);
    if (resultUnit == nullptr)
        return FALSE;
    auto* specUnit = GetSpecByName(resultUnit->m_resultName);

    auto* resultUnitABS
        = m_resultGroup.GetResultByName(g_szPatchInspection3DName[PATCH_INSPECTION_3D_UNIT_PATCH_HIGHT_ABS]);
    if (resultUnitABS == nullptr)
        return FALSE;
    auto* specUnitABS = GetSpecByName(resultUnitABS->m_resultName);

    if (spec == nullptr || specUnit == nullptr || specUnitABS == nullptr)
        return FALSE;

    if (spec->m_use == FALSE && specUnit->m_use == FALSE && specUnitABS->m_use == FALSE)
        return TRUE;

    long nPatchSpecNum = (long)m_vecsPatchInfoDB.size();
    long nPatchNum = (long)m_vecsPatchAlignResult.size();
    long nDataGroupNum = (long)vec2fpt3Patch.size();
    if (nPatchSpecNum != nPatchNum || nPatchSpecNum != nDataGroupNum)
        return FALSE;
    if (nPatchNum == 0 || nPatchSpecNum == 0)
        return TRUE;

    long nTotalDataNum = 0;
    for (auto vecData : vec2fpt3Patch)
        for (auto data : vecData)
            nTotalDataNum++;

    if (spec->m_use)
        result->Resize(nTotalDataNum);
    if (specUnit->m_use)
        resultUnit->Resize(nPatchNum);
    if (specUnitABS->m_use)
        resultUnitABS->Resize(nPatchNum);

    std::vector<CString> vecstrID(8);
    if (vec2PatchROI[0].size() != 8)
        return TRUE;

    vecstrID[0].Format(_T("LT"));
    vecstrID[1].Format(_T("Top"));
    vecstrID[2].Format(_T("RT"));
    vecstrID[3].Format(_T("Right"));
    vecstrID[4].Format(_T("RB"));
    vecstrID[5].Format(_T("BTM"));
    vecstrID[6].Format(_T("LB"));
    vecstrID[7].Format(_T("Left"));
    CString strMaxID;

    const float px2um_x = getScale().pixelToUm().m_x;
    const float px2um_y = getScale().pixelToUm().m_y;
    const float fDeadBugGain = m_packageSpec.m_deadBug ? -1.f : 1.f;
    const float px2um_y_new = px2um_y > 0.f ? (-px2um_y) : px2um_y;

    const float planeNorm = float(::sqrt(planeInterposer.m_a * planeInterposer.m_a
        + planeInterposer.m_b * planeInterposer.m_b + planeInterposer.m_c * planeInterposer.m_c));
    float fSpecHeight{};
    float fCurHeight{};
    float fError{};
    Ipvm::Point32r3 temp;
    long nPatchDataNum(0);
    long nPatchTotalID(0);
    float fMax(0.f);
    Ipvm::Rect32s rtMaxROI;

    for (long nPatch = 0; nPatch < nPatchNum; nPatch++)
    {
        auto patchAlignResult = m_vecsPatchAlignResult[nPatch];
        auto patchInfo = m_vecsPatchInfoDB[nPatch];
        auto vecPatch3D = vec2fpt3Patch[nPatch];

        fMax = 0.f;

        if (patchAlignResult.bAvailable == FALSE)
        {
            if (spec->m_use)
            {
                result->SetValue_EachResult_AndTypicalValueBySingleSpec(nPatch, patchInfo.strName, patchInfo.strType,
                    patchInfo.strType, 0.f, *spec, patchInfo.fOffsetX_um, patchInfo.fOffsetY_um, fSpecHeight, INVALID);
            }
            if (specUnit->m_use)
            {
                resultUnit->SetValue_EachResult_AndTypicalValueBySingleSpec(nPatch, patchInfo.strName,
                    patchInfo.strType, patchInfo.strType, 0.f, *spec, patchInfo.fOffsetX_um, patchInfo.fOffsetY_um,
                    fSpecHeight, INVALID); //kircheis_VSV //kircheis_MED2.5
                //result->SetResult(nPatchTotalID, INVALID);
                resultUnit->SetRect(nPatchTotalID, patchInfo.rtSpecROI);
            }

            if (specUnitABS->m_use)
            {
                resultUnitABS->SetValue_EachResult_AndTypicalValueBySingleSpec(nPatch, patchInfo.strName,
                    patchInfo.strType, patchInfo.strType, 0.f, *specUnitABS, patchInfo.fOffsetX_um,
                    patchInfo.fOffsetY_um, fSpecHeight, INVALID); //kircheis_VSV //kircheis_MED2.5
                //result->SetResult(nPatchTotalID, INVALID);
                resultUnitABS->SetRect(nPatchTotalID, patchInfo.rtSpecROI);
            }
            continue;
        }

        nPatchDataNum = (long)vecPatch3D.size();

        if (nPatchDataNum != 8)
            return TRUE;

        fSpecHeight = patchInfo.fHeight_um;
        long nMaxHeightROI_ID = 0;
        for (long nROI = 0; nROI < nPatchDataNum; nROI++)
        {
            auto src = vecPatch3D[nROI];
            temp.m_x = src.m_x * px2um_x * fDeadBugGain;
            temp.m_y = src.m_y * px2um_y_new;
            temp.m_z = src.m_z * fDeadBugGain;

            fCurHeight = float((planeInterposer.m_a * temp.m_x + planeInterposer.m_b * temp.m_y
                                   + planeInterposer.m_c * temp.m_z + planeInterposer.m_d)
                / planeNorm);
            if (fMax <= fCurHeight)
            {
                fMax = fCurHeight;
                rtMaxROI = vec2PatchROI[nPatch][nROI];
                nMaxHeightROI_ID = nROI;
                strMaxID = vecstrID[nROI];
            }

            if (spec->m_use)
            {
                result->SetValue_EachResult_AndTypicalValueBySingleSpec(nPatchTotalID, vecstrID[nROI],
                    patchInfo.strType, patchInfo.strType, fCurHeight - fSpecHeight, *spec, temp.m_x, temp.m_y,
                    fSpecHeight); //kircheis_MED2.5
                result->SetRect(nPatchTotalID, vec2PatchROI[nPatch][nROI]);
            }

            nPatchTotalID++;
        }
        fError = fMax - fSpecHeight;

        if (specUnit->m_use)
        {
            resultUnit->SetValue_EachResult_AndTypicalValueBySingleSpec(nPatch, strMaxID, patchInfo.strType,
                patchInfo.strType, fError, *specUnit, patchInfo.fOffsetX_um, patchInfo.fOffsetY_um,
                fSpecHeight); //kircheis_MED2.5
            resultUnit->SetRect(nPatch, rtMaxROI);
        }
        if (specUnitABS->m_use)
        {
            resultUnitABS->SetValue_EachResult_AndTypicalValueBySingleSpec(nPatch, strMaxID, patchInfo.strType,
                patchInfo.strType, fMax, *specUnitABS, patchInfo.fOffsetX_um, patchInfo.fOffsetY_um,
                fSpecHeight); //kircheis_MED2.5
            resultUnitABS->SetRect(nPatch, rtMaxROI);
        }
    }

    if (spec->m_use)
    {
        result->UpdateTypicalValue(spec);
        result->SetTotalResult();
    }
    if (specUnit->m_use)
    {
        resultUnit->UpdateTypicalValue(specUnit);
        resultUnit->SetTotalResult();
    }
    if (specUnitABS->m_use)
    {
        resultUnitABS->UpdateTypicalValue(specUnitABS);
        resultUnitABS->SetTotalResult();
    }

    for (int nIdx = 0; nIdx < vecstrID.size(); nIdx++)
    {
        vecstrID[nIdx].Empty();
    }

    strMaxID.Empty();
    return TRUE;
}

void VisionInspectionPatch3D::MakeInterposerROI(
    std::vector<SInterposerRefROI_Info> i_vecsInterposerROI_Pxl, std::vector<Ipvm::Rect32s>& o_vecrtInterposerROI)
{
    o_vecrtInterposerROI.clear();
    long nROINum = (long)i_vecsInterposerROI_Pxl.size();
    if (0 >= nROINum)
        return;

    auto alignResult = *m_sEdgeAlignResult;
    if (alignResult.bAvailable == FALSE)
        return;

    Ipvm::Rect32s rtROI;
    long nHalfSizeX, nHalfSizeY;
    long nOffsetX, nOffsetY;

    FPI_RECT frtBody;
    frtBody.fptLT = alignResult.fptLT;
    frtBody.fptLB = alignResult.fptLB;
    frtBody.fptRT = alignResult.fptRT;
    frtBody.fptRB = alignResult.fptRB;
    Ipvm::Rect32s rtBody = frtBody.GetCRect();
    Ipvm::Rect32s rtIntersect;

    for (long nROI = 0; nROI < nROINum; nROI++)
    {
        rtIntersect.SetRectEmpty();
        nOffsetX = (long)(i_vecsInterposerROI_Pxl[nROI].fOffsetX + alignResult.m_center.m_x + .5f);
        nOffsetY = (long)(i_vecsInterposerROI_Pxl[nROI].fOffsetY + alignResult.m_center.m_y + .5f);
        nHalfSizeX = (long)(i_vecsInterposerROI_Pxl[nROI].fWidth * .5f + .5f);
        nHalfSizeY = (long)(i_vecsInterposerROI_Pxl[nROI].fLength * .5f + .5f);
        rtROI = Ipvm::Rect32s(int32_t(nOffsetX - nHalfSizeX), int32_t(nOffsetY - nHalfSizeY),
            int32_t(nOffsetX + nHalfSizeX), int32_t(nOffsetY + nHalfSizeY));

        if (rtIntersect.IntersectRect(rtBody, rtROI))
        {
            if (!rtIntersect.IsRectEmpty() && rtIntersect == rtROI)
                o_vecrtInterposerROI.push_back(rtROI);
        }
    }
}

void VisionInspectionPatch3D::MakePatchROI(VisionAlignResult alignResult, float fOffsetX_um, float fOffsetY_um,
    float nSize_um, std::vector<Ipvm::Rect32s>& o_vecrtPatchROI)
{
    const auto& um2px = getScale().umToPixel();

    o_vecrtPatchROI.clear();

    if (alignResult.bAvailable == FALSE)
        return;

    const float fOffsetX_Pxl = (float)max(fOffsetX_um, 0.f) * um2px.m_x;
    const float fOffsetY_Pxl = (float)max(fOffsetY_um, 0.f) * um2px.m_y;

    const float fSizeX_Pxl = (float)max(nSize_um, 100.f) * um2px.m_x;
    const float fSizeY_Pxl = (float)max(nSize_um, 100.f) * um2px.m_y;

    const float fSizeX_PxlHalf = fSizeX_Pxl * .5f;
    const float fSizeY_PxlHalf = fSizeY_Pxl * .5f;

    const float fTopX = (alignResult.fptLT.m_x + alignResult.fptRT.m_x) * .5f;
    const float fTopY = (alignResult.fptLT.m_y + alignResult.fptRT.m_y) * .5f;

    const float fLeftX = (alignResult.fptLT.m_x + alignResult.fptLB.m_x) * .5f;
    const float fLeftY = (alignResult.fptLT.m_y + alignResult.fptLB.m_y) * .5f;

    const float fBottomX = (alignResult.fptLB.m_x + alignResult.fptRB.m_x) * .5f;
    const float fBottomY = (alignResult.fptLB.m_y + alignResult.fptRB.m_y) * .5f;

    const float fRightX = (alignResult.fptRT.m_x + alignResult.fptRB.m_x) * .5f;
    const float fRightY = (alignResult.fptRT.m_y + alignResult.fptRB.m_y) * .5f;

    o_vecrtPatchROI.resize(8);

    //LT : 0
    o_vecrtPatchROI[0].m_left = (long)(alignResult.fptLT.m_x + fOffsetX_Pxl + .5f);
    o_vecrtPatchROI[0].m_top = (long)(alignResult.fptLT.m_y + fOffsetY_Pxl + .5f);
    o_vecrtPatchROI[0].m_right = (long)(alignResult.fptLT.m_x + fOffsetX_Pxl + fSizeX_Pxl + .5f);
    o_vecrtPatchROI[0].m_bottom = (long)(alignResult.fptLT.m_y + fOffsetY_Pxl + fSizeY_Pxl + .5f);

    //Top : 1
    o_vecrtPatchROI[1].m_left = (long)(fTopX - fSizeX_PxlHalf + .5f);
    o_vecrtPatchROI[1].m_top = (long)(fTopY + fOffsetY_Pxl + .5f);
    o_vecrtPatchROI[1].m_right = (long)(fTopX + fSizeX_PxlHalf + .5f);
    o_vecrtPatchROI[1].m_bottom = (long)(fTopY + fOffsetY_Pxl + fSizeY_Pxl + .5f);

    //RT : 2
    o_vecrtPatchROI[2].m_left = (long)(alignResult.fptRT.m_x - fOffsetX_Pxl - fSizeX_Pxl + .5f);
    o_vecrtPatchROI[2].m_top = (long)(alignResult.fptRT.m_y + fOffsetY_Pxl + .5f);
    o_vecrtPatchROI[2].m_right = (long)(alignResult.fptRT.m_x - fOffsetX_Pxl + .5f);
    o_vecrtPatchROI[2].m_bottom = (long)(alignResult.fptRT.m_y + fOffsetY_Pxl + fSizeY_Pxl + .5f);

    //Right : 3
    o_vecrtPatchROI[3].m_left = (long)(fRightX - fOffsetX_Pxl - fSizeX_Pxl + .5f);
    o_vecrtPatchROI[3].m_top = (long)(fRightY - fSizeY_PxlHalf + .5f);
    o_vecrtPatchROI[3].m_right = (long)(fRightX - fOffsetX_Pxl + .5f);
    o_vecrtPatchROI[3].m_bottom = (long)(fRightY + fSizeY_PxlHalf + .5f);

    //RB : 4
    o_vecrtPatchROI[4].m_left = (long)(alignResult.fptRB.m_x - fOffsetX_Pxl - fSizeX_Pxl + .5f);
    o_vecrtPatchROI[4].m_top = (long)(alignResult.fptRB.m_y - fOffsetY_Pxl - fSizeY_Pxl + .5f);
    o_vecrtPatchROI[4].m_right = (long)(alignResult.fptRB.m_x - fOffsetX_Pxl + .5f);
    o_vecrtPatchROI[4].m_bottom = (long)(alignResult.fptRB.m_y - fOffsetY_Pxl + .5f);

    //Bottom : 5
    o_vecrtPatchROI[5].m_left = (long)(fBottomX - fSizeX_PxlHalf + .5f);
    o_vecrtPatchROI[5].m_top = (long)(fBottomY - fOffsetY_Pxl - fSizeY_Pxl + .5f);
    o_vecrtPatchROI[5].m_right = (long)(fBottomX + fSizeX_PxlHalf + .5f);
    o_vecrtPatchROI[5].m_bottom = (long)(fBottomY - fOffsetY_Pxl + .5f);

    //LB : 6
    o_vecrtPatchROI[6].m_left = (long)(alignResult.fptLB.m_x + fOffsetX_Pxl + .5f);
    o_vecrtPatchROI[6].m_top = (long)(alignResult.fptLB.m_y - fOffsetY_Pxl - fSizeY_Pxl + .5f);
    o_vecrtPatchROI[6].m_right = (long)(alignResult.fptLB.m_x + fOffsetX_Pxl + fSizeX_Pxl + .5f);
    o_vecrtPatchROI[6].m_bottom = (long)(alignResult.fptLB.m_y - fOffsetY_Pxl + .5f);

    //Left : 7
    o_vecrtPatchROI[7].m_left = (long)(fLeftX + fOffsetX_Pxl + .5f);
    o_vecrtPatchROI[7].m_top = (long)(fLeftY - fSizeY_PxlHalf + .5f);
    o_vecrtPatchROI[7].m_right = (long)(fLeftX + fOffsetX_Pxl + fSizeX_Pxl + .5f);
    o_vecrtPatchROI[7].m_bottom = (long)(fLeftY + fSizeY_PxlHalf + .5f);
}

void VisionInspectionPatch3D::Get3DPosData(std::vector<Ipvm::Rect32s> vecrtROI,
    std::vector<Ipvm::Point32r3>& o_vecfpt3Pos) //출력 변수 구조체 xyz 중 xy는 Pxl Rect Center, z는 실좌표를 쓴다.
{
    o_vecfpt3Pos.clear();
    Ipvm::Point32r3 fpt3Result;

    for (auto rtROI : vecrtROI)
    {
        CalcPatchOrInterposerZ(rtROI, fpt3Result);
        o_vecfpt3Pos.push_back(fpt3Result);
    }
}

void VisionInspectionPatch3D::CalcPatchOrInterposerZ(
    Ipvm::Rect32s rtROI, Ipvm::Point32r3& o_fpt3Pos) //출력 변수 구조체 xyz 중 xy는 Pxl Rect Center, z는 실좌표를 쓴다.
{
    o_fpt3Pos = Ipvm::Point32r3((float)rtROI.CenterPoint().m_x, (float)rtROI.CenterPoint().m_y, Ipvm::k_noiseValue32r);

    const long allPixelCount = rtROI.Width() * rtROI.Height();
    const long desiredMaxPixelCount = 4000;

    // 소팅시 계산량을 줄이기 위해서 샘플링을 시도한다.
    const long samplingStep = max(1, long(sqrt((allPixelCount + desiredMaxPixelCount - 1) / desiredMaxPixelCount)));

    long dataCount = 0;

    std::vector<Ipvm::Point32r3> vecfpt3Points(0);
    Ipvm::Point32r3 fpt3Point;
    const auto& px2um = getScale().pixelToUm();

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

BOOL VisionInspectionPatch3D::Get3DPlane(std::vector<Ipvm::Point32r3> vecfpt3Pos, Ipvm::PlaneEq64r& o_plane)
{
    const auto& pixelToUm = getScale().pixelToUm();

    const long pointCount = (long)vecfpt3Pos.size();

    o_plane = {0., 0., 0., 0.};

    if (pointCount <= 0)
        return FALSE;

    // 오른손 좌표계를 사용하기 위해서 Y 에 -1 곱해줌
    const float px2um_y_new = pixelToUm.m_y > 0.f ? (-pixelToUm.m_y) : pixelToUm.m_y;

    std::vector<Ipvm::Point32r3> vecPoints;
    vecPoints.reserve(pointCount);

    Ipvm::Point32r3 temp;

    const float fDeadBugGain = m_packageSpec.m_deadBug ? -1.f : 1.f;

    for (int idx = 0; idx < pointCount; idx++)
    {
        const auto& src = vecfpt3Pos[idx];

        if (src.m_z == Ipvm::k_noiseValue32r)
        {
            continue;
        }

        // 단위계를 실 단위계로 바꿔 주고...
        temp.m_x = src.m_x * pixelToUm.m_x * fDeadBugGain;
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
