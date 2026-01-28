//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "Inspection_Impl.h"

//CPP_2_________________________________ This project's headers
#include "Para.h"

//CPP_3_________________________________ Other projects' headers
#include "../../AlgorithmModules/dPI_Blob/dPI_BlobLib.h"
#include "../../AlgorithmModules/dPI_EdgeDetect/dPI_EdgeDetectLib.h"
#include "../../AlgorithmModules/dPI_Geometry/dPI_GeometryLib.h"
#include "../../DefineModules/dA_Base/PI_RECT.h"
#include "../../ImageCombineModules/dPI_ippModules/ippModules.h"
#include "../../InformationModule/dPI_DataBase/PackageSpec.h"
#include "../../InformationModule/dPI_DataBase/VisionTrayScanSpec.h"
#include "../../InformationModule/dPI_SystemIni/Config.h"
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../../MemoryModules/VisionReusableMemory/SurfaceLayerRoi.h"
#include "../../MemoryModules/VisionReusableMemory/VisionReusableMemory.h"
#include "../VisionCommon/VisionAlignResult.h"
#include "../VisionCommon/VisionBaseDef.h"
#include "../VisionCommon/VisionDebugInfo.h"
#include "../VisionCommon/VisionImageLot.h"
#include "../VisionCommon/VisionInspectionOverlayResult.h"
#include "../VisionCommon/VisionProcessing.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Algorithm/DataFitting.h>
#include <Ipvm/Algorithm/Geometry.h>
#include <Ipvm/Algorithm/ImageProcessing.h>
#include <Ipvm/Base/Conversion.h>
#include <Ipvm/Base/Enum.h>
#include <Ipvm/Base/Image32s.h>
#include <Ipvm/Base/Image8u.h>
#include <Ipvm/Base/LineSeg32r.h>
#include <Ipvm/Base/Quadrangle32r.h>
#include <Ipvm/Base/Rect32r.h>
#include <Ipvm/Base/Size32s2.h>
#include <Ipvm/Base/TimeCheck.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
using namespace VisionEdgeAlign;

Inspection_Impl::Inspection_Impl(VisionProcessing& processor)
    : m_processor(processor)
    , m_packageSpec(processor.m_packageSpec)
    , m_fBodySizeX_Pixel(0.f)
    , m_fBodySizeY_Pixel(0.f)
    , m_fBodySizeX_Check_Offset_LowLimit(0.f)
    , m_fBodySizeY_Check_Offset_LowLimit(0.f)
    , m_fCalcTime(0.f)
    , m_teachModeForHeightMoldToWall(false)
{
    auto& debugInfoGroup = m_processor.m_DebugInfoGroup;

    debugInfoGroup.Add(_T("EDGE Align Result"), enumDebugInfoType::None, 1);
    debugInfoGroup.Add(_T("Current Empty Check Value"), enumDebugInfoType::Float, 1);
    debugInfoGroup.Add(_T("Tray Wall ROI"), enumDebugInfoType::Rect, 1);
    debugInfoGroup.Add(_T("Left Search ROI"), enumDebugInfoType::Rect, 1);
    debugInfoGroup.Add(_T("Top Search ROI"), enumDebugInfoType::Rect, 1);
    debugInfoGroup.Add(_T("Right Search ROI"), enumDebugInfoType::Rect, 1);
    debugInfoGroup.Add(_T("Bottom Search ROI"), enumDebugInfoType::Rect, 1);
    debugInfoGroup.Add(_T("LowTop Search ROI"), enumDebugInfoType::Rect, 1);
    debugInfoGroup.Add(_T("Edge Point Left Total"), enumDebugInfoType::Point_32f_C2, 1);
    debugInfoGroup.Add(_T("Edge Point Left Final"), enumDebugInfoType::Point_32f_C2, 1);
    debugInfoGroup.Add(_T("Edge Point Top Total"), enumDebugInfoType::Point_32f_C2, 1);
    debugInfoGroup.Add(_T("Edge Point Top Final"), enumDebugInfoType::Point_32f_C2, 1);
    debugInfoGroup.Add(_T("Edge Point Right Total"), enumDebugInfoType::Point_32f_C2, 1);
    debugInfoGroup.Add(_T("Edge Point Right Final"), enumDebugInfoType::Point_32f_C2, 1);
    debugInfoGroup.Add(_T("Edge Point Bottom Total"), enumDebugInfoType::Point_32f_C2, 1);
    debugInfoGroup.Add(_T("Edge Point Bottom Final"), enumDebugInfoType::Point_32f_C2, 1);
    debugInfoGroup.Add(_T("Edge Point LowTop Total"), enumDebugInfoType::Point_32f_C2, 1);
    debugInfoGroup.Add(_T("Edge Point LowTop Final"), enumDebugInfoType::Point_32f_C2, 1);
    debugInfoGroup.Add(_T("Body Rect"), enumDebugInfoType::Rect_32f, 1);
    debugInfoGroup.Add(_T("Body Center"), enumDebugInfoType::Point_32f_C2, 1);
    debugInfoGroup.Add(_T("Round Ellipse"), enumDebugInfoType::EllipseEq_32f, 1);
    debugInfoGroup.Add(_T("Round Spec Ellipse"), enumDebugInfoType::EllipseEq_32f, 1);
    debugInfoGroup.Add(_T("Round Edge Research Range"), enumDebugInfoType::LineSeg_32f, 1);
    debugInfoGroup.Add(_T("Round Edge Research Result"), enumDebugInfoType::Point_32f_C2, 1);
    debugInfoGroup.Add(_T("Round Edge Research Result Final"), enumDebugInfoType::Point_32f_C2, 1);
    debugInfoGroup.Add(_T("Round Research Ellipse"), enumDebugInfoType::EllipseEq_32f, 1);
    debugInfoGroup.Add(_T("Side Detail Align FPoint"), enumDebugInfoType::Point_32f_C2, 1);
    debugInfoGroup.Add(_T("Side Detail Align Point"), enumDebugInfoType::Point, 1);

    if (Config::getVisionType() == VISIONTYPE_2D_INSP
        || Config::getVisionType() == VISIONTYPE_SWIR_INSP) //kircheis_SideVision고려 //kircheis_SWIR
    {
        debugInfoGroup.Add(_T("Empty Blob Rect"), enumDebugInfoType::Rect_32f, 2);
        debugInfoGroup.Add(_T("Empty Blob Count"), enumDebugInfoType::Long, 2);
    }
}

Inspection_Impl::~Inspection_Impl()
{
}

void Inspection_Impl::resetResult()
{
    m_errorLogText.Empty();
    m_result_edgeAlign.Init(Ipvm::Point32r2(0.f, 0.f));
    m_result_2DEmpty.Reset();

    // Get Algorithm...
    auto& memory = m_processor.getReusableMemory();
    auto& scale = m_processor.getScale();

    m_pEdgeDetect = memory.GetEdgeDetect();
    m_processor.GetPackageSize(true, m_fBodySizeX_Pixel, m_fBodySizeY_Pixel);
    //	m_fBodySizeX_Pixel = scale.convert_umToPixelX(m_packageSpec.m_bodyInfoMaster->GetBodySizeX());
    //	m_fBodySizeY_Pixel = scale.convert_umToPixelY(m_packageSpec.m_bodyInfoMaster->GetBodySizeY());

    m_fBodySizeX_Check_Offset_LowLimit = scale.convert_umToPixelX(300.f);
    m_fBodySizeY_Check_Offset_LowLimit = scale.convert_umToPixelY(300.f);
}

bool Inspection_Impl::run(const Para& para, const bool detailSetupMode, const bool teachModeForHeightMoldToWall)
{
    Ipvm::TimeCheck time;

    m_teachModeForHeightMoldToWall = teachModeForHeightMoldToWall;
    m_fHeightMoldToWall = 0.f;

    Ipvm::Image8u mainImageFor3D;

    if (Config::getVisionType() == VISIONTYPE_3D_INSP)
    {
        Ipvm::Image8u mainImageRawFor3D = m_processor.getImageLot().GetImageFrame(
            0, m_processor.GetCurVisionModule_Status()); //m_processor.GetInspectionFrameImage(FALSE, 0);
        if (mainImageRawFor3D.GetMem() == nullptr)
        {
            m_errorLogText.AppendFormat(_T("  Not enough Resuable Memory\r\n"));
            m_processor.m_bInvalid = FALSE;
            return false;
        }

        // 3D Empty 체크는 No Stitch 영역에서 진행하자. 비었는데 Stitch 할수 없다

        Ipvm::Rect32s rtPane = getImageSourceRoiForNoStitch();
        Ipvm::Rect32s rtImageArea = Ipvm::Rect32s(mainImageRawFor3D);
        rtPane &= rtImageArea;
        mainImageFor3D = Ipvm::Image8u(mainImageRawFor3D, rtPane);
    }

    if (Config::getVisionType() == VISIONTYPE_3D_INSP)
    {
        if (para.m_emptyInsp || para.m_3D_doubleInsp)
        {
            m_result_2DEmpty.m_inspectioned = true;

            float fCurPackageHeight = 0.f;
            BOOL bIsEmpty = FALSE;
            BOOL bEmptyCheckResult = CheckEmptyPocket3D(para, mainImageFor3D, fCurPackageHeight, bIsEmpty);

            static long nEmptyCnt = 0;
            static long nDoubleCnt = 0;
            static long nWallFailCnt = 0;

            if (bEmptyCheckResult && bIsEmpty && para.m_emptyInsp)
            {
                m_processor.m_bEmpty = TRUE;
                m_fCalcTime = CAST_FLOAT(time.Elapsed_ms());
                nEmptyCnt++;
                return false;
            }

            m_result_2DEmpty.m_success = true;
        }
    }
    else
    {
        Ipvm::Image8u emptyChekImage = para.m_emptyFrameIndex.getImage(true);
        if (emptyChekImage.GetMem() == nullptr)
            return false;

        if (para.m_emptyInsp)
        {
            m_result_2DEmpty.m_inspectioned = true;

            // 검사
            auto retValue = emptyCheck2D(para);

            std::vector<Ipvm::Rect32r> blobRois;
            m_result_2DEmpty.getBlobRois(blobRois);
            long blobCount = long(blobRois.size());

            m_processor.SetDebugInfoItem(detailSetupMode, _T("Empty Blob Rect"), blobRois);
            m_processor.SetDebugInfoItem(detailSetupMode, _T("Empty Blob Count"), blobCount, TRUE);

            if (!retValue)
            {
                m_processor.m_bEmpty = TRUE;
                m_fCalcTime = CAST_FLOAT(time.Elapsed_ms());
                return false;
            }

            m_result_2DEmpty.m_success = true;
        }
    }

    m_result_edgeAlign.m_inspectioned = true;
    m_result_edgeAlign.m_success = DoAlign(para, detailSetupMode);

    if (!m_result_edgeAlign.m_success)
    {
        m_processor.m_bInvalid = TRUE;
    }

    setDebugInfo(detailSetupMode);

    BOOL bIsGullwing = (m_packageSpec.nDeviceType == enDeviceType::PACKAGE_GULLWING);

    if (Config::getVisionType() == VISIONTYPE_3D_INSP && !bIsGullwing
        && (para.m_emptyInsp || para.m_3D_doubleInsp)) //kircheis_20160713
    {
        BOOL bIsEmpty = FALSE;
        BOOL bIsDouble = FALSE;

        BOOL bEmptyCheckResult = EmptyDoubleCheckUseTrayWallForLeadless(
            para, detailSetupMode, mainImageFor3D, bIsEmpty, bIsDouble); //kircheis_20160713

        if (bEmptyCheckResult && bIsEmpty && para.m_emptyInsp)
        {
            m_processor.m_bEmpty = TRUE;
            m_fCalcTime = CAST_FLOAT(time.Elapsed_ms());
            return false;
        }
        if (bEmptyCheckResult && bIsDouble && para.m_3D_doubleInsp)
        {
            m_processor.m_bDoubleDevice = TRUE;
            m_fCalcTime = CAST_FLOAT(time.Elapsed_ms());
            return false;
        }
    }

    if (!detailSetupMode)
    {
        getResult_EdgeAlign()->FreeMemoryForCalculation();
        getResult_2DEmpty()->FreeMemoryForCalculation();
    }

    m_fCalcTime = CAST_FLOAT(time.Elapsed_ms());

    return m_result_edgeAlign.m_success;
}

Ipvm::Rect32s Inspection_Impl::getImageSourceRoiForNoStitch()
{
    Ipvm::Point32r2 ptImageCenter(
        m_processor.getImageLot().GetImageSizeX() * 0.5f, m_processor.getImageLot().GetImageSizeY() * 0.5f);

    long paneID = m_processor.GetCurrentPaneID();
    auto& scale = m_processor.getImageLot().GetScale();

    //Ipvm::Rect32s MaxROI(0, 0, m_processor.getImageLot().GetImageSizeX(), m_processor.getImageLot().GetImageSizeY()); // SDY ROI 최댓값은 raw 이미지의 크기를 최댓값으로 한다.
    Ipvm::Rect32s roi(0, 0, 0, 0);
    const auto& inspectionAreaInfo = m_processor.getInspectionAreaInfo();

    for (auto& fovIndex : inspectionAreaInfo.m_fovList)
    {
        if (paneID >= 0 && paneID < (long)inspectionAreaInfo.m_unitIndexList.size())
        {
            long unitID = inspectionAreaInfo.m_unitIndexList[paneID];
            auto pocketRegion
                = scale.convert_mmToPixel(inspectionAreaInfo.m_parent->GetUnitPocketRegionInFOV(fovIndex, unitID))
                + Ipvm::Conversion::ToPoint32s2(ptImageCenter);
            roi |= pocketRegion;
        }
    }

    // SDY ROI가 최대 이미지 이상의 크기를 갖지 못하도록 한다.
    //roi &= MaxROI;

    return roi;
}

bool Inspection_Impl::getImageForAlign(Para& para, bool useFullImage, Ipvm::Image8u& dstImage)
{
    auto& memory = m_processor.getReusableMemory();

    //------------------------------------------------------------------------------------------
    // Align 용 이미지 만들기
    //------------------------------------------------------------------------------------------

    Ipvm::Image8u mainImage;

    auto& imageLot = m_processor.getImageLot();

    if (Config::getVisionType() == VISIONTYPE_3D_INSP)
    {
        if (imageLot.GetImageFrameCount() == 0)
            return false;
        mainImage = imageLot.GetImageFrame(0, enSideVisionModule::SIDE_VISIONMODULE_FRONT);
    }
    else
    {
        mainImage = para.m_alignImageFrameIndex.getImage(true);
        if (mainImage.GetMem() == nullptr)
            return false;
    }

    if (mainImage.GetMem() == nullptr)
    {
        m_errorLogText.AppendFormat(_T("  Not enough Resuable Memory\r\n"));
        m_processor.m_bInvalid = FALSE;
        return false;
    }

    if (useFullImage)
    {
        // 이미지 원래 사이즈 크기로 계산
        if (!memory.GetByteImage(dstImage, mainImage.GetSizeX(), mainImage.GetSizeY()))
        {
            m_errorLogText.AppendFormat(_T("  Not enough Resuable Memory\r\n"));
            return false;
        }

        auto paneRoi = Ipvm::Rect32s(mainImage);
        return CippModules::GrayImageProcessingManage(
            memory, &mainImage, true, paneRoi, para.m_ImageProcMangePara, dstImage);
    }

    Ipvm::Rect32s paneRoi = getImageSourceRoiForNoStitch();
    Ipvm::Image8u paneSource;

    if (!memory.GetByteImage(paneSource, paneRoi.Width(), paneRoi.Height()))
    {
        m_errorLogText.AppendFormat(_T("  Not enough Resuable Memory\r\n"));
        return false;
    }

    // Pane Center 중심의 이미지를 만든다
    paneSource.FillZero();
    CippModules::CopyValid(mainImage, paneRoi.TopLeft(), Ipvm::Point32s2(0, 0),
        Ipvm::Size32s2(paneRoi.Width(), paneRoi.Height()), paneSource);

    if (!memory.GetByteImage(dstImage, paneRoi.Width(), paneRoi.Height()))
    {
        m_errorLogText.AppendFormat(_T("  Not enough Resuable Memory\r\n"));
        return false;
    }

    return CippModules::GrayImageProcessingManageForAlignUseRawInPane(
        memory, &paneSource, paneRoi, para.m_ImageProcMangePara, dstImage);
}

bool Inspection_Impl::getImageFor2DEmpty(Para& para, bool useFullImage, Ipvm::Image8u& dstImage)
{
    auto& memory = m_processor.getReusableMemory();

    //------------------------------------------------------------------------------------------
    // Align 용 이미지 만들기
    //------------------------------------------------------------------------------------------

    Ipvm::Image8u mainImage;

    if (Config::getVisionType() == VISIONTYPE_3D_INSP)
    {
        return false;
    }

    mainImage = para.m_emptyFrameIndex.getImage(true);
    if (mainImage.GetMem() == nullptr)
        return false;

    if (mainImage.GetMem() == nullptr)
    {
        m_errorLogText.AppendFormat(_T("  Not enough Resuable Memory\r\n"));
        m_processor.m_bInvalid = FALSE;
        return false;
    }

    if (useFullImage)
    {
        dstImage = mainImage;
        return true;
    }

    Ipvm::Rect32s paneRoi = getImageSourceRoiForNoStitch();

    if (!memory.GetByteImage(dstImage, paneRoi.Width(), paneRoi.Height()))
    {
        m_errorLogText.AppendFormat(_T("  Not enough Resuable Memory\r\n"));
        return false;
    }

    // Pane Center 중심의 이미지를 만든다
    dstImage.FillZero();
    CippModules::CopyValid(mainImage, paneRoi.TopLeft(), Ipvm::Point32s2(0, 0),
        Ipvm::Size32s2(paneRoi.Width(), paneRoi.Height()), dstImage);

    return true;
}

void Inspection_Impl::setGlobalAlignResult()
{
    auto& memory = m_processor.getReusableMemory();
    auto& debugInfoGroup = m_processor.m_DebugInfoGroup;
    auto* psDebugInfo = debugInfoGroup.GetDebugInfo(_T("EDGE Align Result"));
    if (psDebugInfo != NULL)
    {
        if (psDebugInfo->pData != NULL)
        {
            psDebugInfo->Reset();
        }

        psDebugInfo->nDataNum = 1;
        VisionAlignResult* psBodyAlignResult = new VisionAlignResult[1];
        *psBodyAlignResult = m_result_edgeAlign;
        psDebugInfo->pData = psBodyAlignResult;

        // Align 정보를 기록해 준다
        memory.SetAlignInfo(psBodyAlignResult->m_center, float(psBodyAlignResult->m_angle_rad * IV_RAD_TO_DEG));
        if (auto* surfaceRoi = memory.AddSurfaceLayerRoiClass(_T("Measured - Body")))
        {
            Ipvm::Quadrangle32r body;
            body.m_ltX = psBodyAlignResult->fptLT.m_x;
            body.m_ltY = psBodyAlignResult->fptLT.m_y;
            body.m_rtX = psBodyAlignResult->fptRT.m_x;
            body.m_rtY = psBodyAlignResult->fptRT.m_y;
            body.m_lbX = psBodyAlignResult->fptLB.m_x;
            body.m_lbY = psBodyAlignResult->fptLB.m_y;
            body.m_rbX = psBodyAlignResult->fptRB.m_x;
            body.m_rbY = psBodyAlignResult->fptRB.m_y;

            surfaceRoi->Reset();
            surfaceRoi->Add(body);
        }
    }
}

void Inspection_Impl::setGlobalAlignResult_ZeroAngle(const Ipvm::Point32r2& shift)
{
    auto& memory = m_processor.getReusableMemory();
    auto& debugInfoGroup = m_processor.m_DebugInfoGroup;
    auto* psDebugInfo = debugInfoGroup.GetDebugInfo(_T("EDGE Align Result"));
    if (psDebugInfo != NULL)
    {
        if (psDebugInfo->pData != NULL)
        {
            psDebugInfo->Reset();
        }

        psDebugInfo->nDataNum = 1;
        VisionAlignResult* psBodyAlignResult = new VisionAlignResult[1];
        *psBodyAlignResult = m_result_edgeAlign;

        psBodyAlignResult->SetBodyAlign_AngleZero(shift);

        psDebugInfo->pData = psBodyAlignResult;

        // Align 정보를 기록해 준다
        memory.SetAlignInfo(psBodyAlignResult->m_center, float(psBodyAlignResult->m_angle_rad * IV_RAD_TO_DEG));
        if (auto* surfaceRoi = memory.AddSurfaceLayerRoiClass(_T("Measured - Body")))
        {
            Ipvm::Quadrangle32r body;
            body.m_ltX = psBodyAlignResult->fptLT.m_x;
            body.m_ltY = psBodyAlignResult->fptLT.m_y;
            body.m_rtX = psBodyAlignResult->fptRT.m_x;
            body.m_rtY = psBodyAlignResult->fptRT.m_y;
            body.m_lbX = psBodyAlignResult->fptLB.m_x;
            body.m_lbY = psBodyAlignResult->fptLB.m_y;
            body.m_rbX = psBodyAlignResult->fptRB.m_x;
            body.m_rbY = psBodyAlignResult->fptRB.m_y;

            surfaceRoi->Reset();
            surfaceRoi->Add(body);
        }
    }
}

void Inspection_Impl::getOverlayResult(
    const Para& para, VisionInspectionOverlayResult* overlayResult, long overlayMode, const bool detailSetupMode)
{
    UNREFERENCED_PARAMETER(detailSetupMode);

    if (overlayMode == OverlayType_ShowAll)
    {
        auto& debugInfoGroup = m_processor.m_DebugInfoGroup;
        auto* psDebugInfo = debugInfoGroup.GetDebugInfo(_T("EDGE Align Result"));
        if (psDebugInfo != NULL && psDebugInfo->nDataNum == 1)
        {
            auto* psBodyAlignResult = (VisionAlignResult*)psDebugInfo->pData;

            overlayResult->AddCrPoint(psBodyAlignResult->m_center, RGB(0, 255, 0));
            overlayResult->AddLine(psBodyAlignResult->fptLT, psBodyAlignResult->fptRT, RGB(0, 255, 0));
            overlayResult->AddLine(psBodyAlignResult->fptRT, psBodyAlignResult->fptRB, RGB(0, 255, 0));
            overlayResult->AddLine(psBodyAlignResult->fptRB, psBodyAlignResult->fptLB, RGB(0, 255, 0));
            overlayResult->AddLine(psBodyAlignResult->fptLB, psBodyAlignResult->fptLT, RGB(0, 255, 0));

            if (para.m_nLowTopCount > 0 && psBodyAlignResult->fptLowtopLT.m_x > 0
                && psBodyAlignResult->fptLowtopLT.m_y > 0 && psBodyAlignResult->fptLowtopRT.m_x > 0
                && psBodyAlignResult->fptLowtopRT.m_y > 0)
            {
                overlayResult->AddLine(psBodyAlignResult->fptLowtopLT, psBodyAlignResult->fptLowtopRT, RGB(0, 255, 0));
            }
        }

        if (m_processor.m_bEmpty)
        {
            std::vector<Ipvm::Rect32r> blobRois;
            m_result_2DEmpty.getBlobRois(blobRois);

            for (auto& roi : blobRois)
            {
                overlayResult->AddRectangle(roi, RGB(0, 255, 0));
            }
        }
    }
}

void Inspection_Impl::appendTextResult(CString& textResult)
{
    textResult.AppendFormat(_T("  [ Align Inspection Time : %.2f ]\r\n"), getCalculationTime());
    textResult.AppendFormat(_T("    Body Left Top Point(X,Y) : %.2f, %.2f\r\n"), m_result_edgeAlign.fptLT.m_x,
        m_result_edgeAlign.fptLT.m_y);
    textResult.AppendFormat(_T("    Body Right Top Point(X,Y) : %.2f, %.2f\r\n"), m_result_edgeAlign.fptRT.m_x,
        m_result_edgeAlign.fptRT.m_y);
    textResult.AppendFormat(_T("    Body Left Bottom Point(X,Y) : %.2f, %.2f\r\n"), m_result_edgeAlign.fptLB.m_x,
        m_result_edgeAlign.fptLB.m_y);
    textResult.AppendFormat(_T("    Body Right Bottom Point(X,Y) : %.2f, %.2f\r\n"), m_result_edgeAlign.fptRB.m_x,
        m_result_edgeAlign.fptRB.m_y);
    textResult.AppendFormat(_T("    Body Center Point(X,Y) : %.2f, %.2f\r\n"), m_result_edgeAlign.m_center.m_x,
        m_result_edgeAlign.m_center.m_y);
    textResult.AppendFormat(_T("    Body Angle : %.2f\r\n\r\n"), m_result_edgeAlign.m_angle_rad * ITP_RAD_TO_DEG);

    if (m_processor.m_bEmpty)
    {
        textResult.AppendFormat(_T("  [ Alin Empty ]\r\n"));
        textResult += getErrorLogText();
    }

    if (m_processor.m_bInvalid)
    {
        textResult.AppendFormat(_T("  [ Edge Alin Invalid ]\r\n"));
        textResult += getErrorLogText();
    }
}

Result_EdgeAlign* Inspection_Impl::getResult_EdgeAlign()
{
    return &m_result_edgeAlign;
}

Result_2DEmpty* Inspection_Impl::getResult_2DEmpty()
{
    return &m_result_2DEmpty;
}

float Inspection_Impl::getCalculationTime() const
{
    return m_fCalcTime;
}

LPCTSTR Inspection_Impl::getErrorLogText() const
{
    return m_errorLogText;
}

void Inspection_Impl::setDebugInfo(const bool detailSetupMode)
{
    std::vector<Ipvm::Rect32r> vecfrtTemp(1);
    std::vector<Ipvm::Point32r2> vecfptTemp(1);

    vecfrtTemp[0] = m_result_edgeAlign.getBodyRect();
    m_processor.SetDebugInfoItem(detailSetupMode, _T("Body Rect"), vecfrtTemp);

    vecfptTemp[0] = m_result_edgeAlign.m_center;
    m_processor.SetDebugInfoItem(detailSetupMode, _T("Body Center"), vecfptTemp);

    m_processor.SetDebugInfoItem(detailSetupMode, _T("Left Search ROI"), m_result_edgeAlign.vecLeftSearchROI);
    m_processor.SetDebugInfoItem(detailSetupMode, _T("Top Search ROI"), m_result_edgeAlign.vecTopSearchROI);
    m_processor.SetDebugInfoItem(detailSetupMode, _T("Right Search ROI"), m_result_edgeAlign.vecRightSearchROI);
    m_processor.SetDebugInfoItem(detailSetupMode, _T("Bottom Search ROI"), m_result_edgeAlign.vecBottomSearchROI);
    m_processor.SetDebugInfoItem(detailSetupMode, _T("LowTop Search ROI"), m_result_edgeAlign.vecLowTopSearchROI);

    m_processor.SetDebugInfoItem(
        detailSetupMode, _T("Edge Point Left Total"), m_result_edgeAlign.vecLeftEdgeAlignPoints_Total);
    m_processor.SetDebugInfoItem(
        detailSetupMode, _T("Edge Point Left Final"), m_result_edgeAlign.vecLeftEdgeAlignPoints);

    m_processor.SetDebugInfoItem(
        detailSetupMode, _T("Edge Point Top Total"), m_result_edgeAlign.vecTopEdgeAlignPoints_Total);
    m_processor.SetDebugInfoItem(detailSetupMode, _T("Edge Point Top Final"), m_result_edgeAlign.vecTopEdgeAlignPoints);

    m_processor.SetDebugInfoItem(
        detailSetupMode, _T("Edge Point Right Total"), m_result_edgeAlign.vecRightEdgeAlignPoints_Total);
    m_processor.SetDebugInfoItem(
        detailSetupMode, _T("Edge Point Right Final"), m_result_edgeAlign.vecRightEdgeAlignPoints);

    m_processor.SetDebugInfoItem(
        detailSetupMode, _T("Edge Point Bottom Total"), m_result_edgeAlign.vecBottomEdgeAlignPoints_Total);
    m_processor.SetDebugInfoItem(
        detailSetupMode, _T("Edge Point Bottom Final"), m_result_edgeAlign.vecBottomEdgeAlignPoints);

    m_processor.SetDebugInfoItem(
        detailSetupMode, _T("Edge Point LowTop Total"), m_result_edgeAlign.vecLowTopEdgeAlignPoints_Total);
    m_processor.SetDebugInfoItem(
        detailSetupMode, _T("Edge Point LowTop Final"), m_result_edgeAlign.vecLowTopEdgeAlignPoints);
}

bool Inspection_Impl::DoAlign(const Para& para, const bool detailSetupMode)
{
    static const bool bIsSideVision = (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_SIDE_INSP);

    if (para.m_nLowTopCount)
    {
        m_packageSpec.m_bLowtopEdge = TRUE;
    }
    else
    {
        m_packageSpec.m_bLowtopEdge = FALSE;
    }

    // 영훈 20150121_FirstEdge_Min_Threshold : First edge로 찾을 경우 Min Edge Value를 설정해주도록 한다.
    float fOldThresholdValue = 0.f;
    if (para.m_nEdgeDetectMode == EdgeDetectMode_FirstEdge)
    {
        fOldThresholdValue = m_pEdgeDetect->SetMininumThreshold(para.m_fFirstEdgeMinThreshold);
    }

    float fCurEdgeThresh = m_pEdgeDetect->GetMininumThreshold();
    if (fCurEdgeThresh < 1.f)
    {
        float fDefaultEdgeThresh = 1.f;
        m_pEdgeDetect->SetMininumThreshold(fDefaultEdgeThresh);
    }

    auto paneCenter = m_result_edgeAlign.getPaneCenter();

    if (bIsSideVision)
    {
        BodyEdgeDetect_TopbyMatching(para, m_result_edgeAlign.getImage(), paneCenter);
        BodyEdgeDetect_BottombyMatching(para, m_result_edgeAlign.getImage(), paneCenter);

        //Top & Bottom Point 정보를 가져와서
        const auto& vecptTopPoint = m_result_edgeAlign.vecTopEdgeAlignPoints_Total;
        const auto& vecptBottomPoint = m_result_edgeAlign.vecBottomEdgeAlignPoints_Total;
        long nTopPointSize = (long)vecptTopPoint.size();
        long nBottomPointSize = (long)vecptTopPoint.size();
        Ipvm::LineEq32r eqLineTop, eqLineBottom;
        Ipvm::LineEq32r eqLineCenter(0.f, 0.f,
            0.f); //얘는 초기화 필수. BodyEdgeDetect_Left/RightbyMatching에서 이 초기 값을 확인하여 알고리즘 적용 여부를 결정한다.

        if (nTopPointSize > 5 && nBottomPointSize > 5) //상하 Point 개수가 충분하면
        {
            //각각 피팅을 하고
            Ipvm::DataFitting::FitToLine(nTopPointSize, &vecptTopPoint[0], eqLineTop);
            Ipvm::DataFitting::FitToLine(nBottomPointSize, &vecptBottomPoint[0], eqLineBottom);

            //두 피팅라인의 중간선을 구한다.
            Ipvm::Geometry::GetBisection(eqLineTop, eqLineBottom, eqLineCenter);
        }

        //연산한 피팅라인을 Left/Right Edge따는 함수에 전달해서 이를 참조하여 ROI를 이동하게 한다.
        BodyEdgeDetect_LeftbyMatching(para, m_result_edgeAlign.getImage(), paneCenter, eqLineCenter);
        BodyEdgeDetect_RightbyMatching(para, m_result_edgeAlign.getImage(), paneCenter, eqLineCenter);
    }
    else
    {
        BodyEdgeDetect_LeftbyMatching(para, m_result_edgeAlign.getImage(), paneCenter);
        BodyEdgeDetect_RightbyMatching(para, m_result_edgeAlign.getImage(), paneCenter);
        BodyEdgeDetect_TopbyMatching(para, m_result_edgeAlign.getImage(), paneCenter);
        BodyEdgeDetect_BottombyMatching(para, m_result_edgeAlign.getImage(), paneCenter);
    }

    if (para.m_nLowTopCount >= 1)
    {
        BodyEdgeDetect_Lowtop(para, m_result_edgeAlign.getImage(), paneCenter);
    }

    if (para.m_nEdgeDetectMode == EdgeDetectMode_FirstEdge)
    {
        m_pEdgeDetect->SetMininumThreshold(fOldThresholdValue);
    }

    // 영훈 예외처리 : 엣지를 찾지 못할 경우 그냥 빠져나가도록 한다.
    long nSize = 0;
    nSize = (long)(m_result_edgeAlign.vecLeftEdgeAlignPoints.size());
    if (nSize < 2)
    {
        m_errorLogText.AppendFormat(_T("  Edge not found for Left side (%d)\r\n"), nSize);
        return false;
    }

    nSize = (long)(m_result_edgeAlign.vecTopEdgeAlignPoints.size());
    if (nSize < 2)
    {
        m_errorLogText.AppendFormat(_T("  Edge not found for Top side (%d)\r\n"), nSize);
        return false;
    }

    nSize = (long)(m_result_edgeAlign.vecRightEdgeAlignPoints.size());
    if (nSize < 2)
    {
        m_errorLogText.AppendFormat(_T("  Edge not found for Right side (%d)\r\n"), nSize);
        return false;
    }

    nSize = (long)(m_result_edgeAlign.vecBottomEdgeAlignPoints.size());
    if (nSize < 2)
    {
        m_errorLogText.AppendFormat(_T("  Edge not found for Bottom side (%d)\r\n"), nSize);
        return false;
    }

    // LowTop
    if (para.m_nLowTopCount >= 1)
    {
        nSize = (long)(m_result_edgeAlign.vecLowTopEdgeAlignPoints.size());
        if (nSize < 2)
            return false;
    }

    //SetResult(m_packageSpec.m_bLowtopEdge);//kircheis_RoundPKG
    long nRoundPackageOption = m_packageSpec.m_bodyInfoMaster->m_nRoundPackageOption;
    if (nRoundPackageOption == Round_UpDown)
        SetResultForRound_UpDown(detailSetupMode, m_result_edgeAlign.getImage(), nRoundPackageOption);
    else
        SetResult(m_packageSpec.m_bLowtopEdge);

    calcParallelism();

    // Lead Type은 Body align이 정확하지 않으므로 Size를 체크하지 않는다.
    if (m_packageSpec.nPackageType <= enPackageType::QFP)
        return true;

    return true;
}

bool Inspection_Impl::MoveEdgeROIforSide(const Ipvm::LineEq32r& i_eqLineCenter, Ipvm::Rect32s& io_rtEdgeROI)
{
    static const bool bIsSideVision = (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_SIDE_INSP);

    if (bIsSideVision == false) //Side Vision이 아니면 이 함수 연산 안하고
        return true;
    if (i_eqLineCenter.m_a == 0.f && i_eqLineCenter.m_b == 0.f
        && i_eqLineCenter.m_c == 0.f) //Line Fitting 안했어도 안한다.
        return true;

    long nRoiWidthHalf = (io_rtEdgeROI.Width() / 2);
    long nRoiHeightHalf = (io_rtEdgeROI.Height() / 2);

    Ipvm::Point32r2 ptRoiCenter = io_rtEdgeROI.CenterPoint32r();
    Ipvm::Point32r2 ptRoiDst;

    Ipvm::Geometry::GetFootOfPerpendicular(
        i_eqLineCenter, ptRoiCenter, ptRoiDst); //원래의 ROI Center를 Fitting Line으로 이동시키고

    //이동한 Center Point 기준으로 ROI를 재구성한다.
    io_rtEdgeROI.m_left = (long)(ptRoiDst.m_x - (float)nRoiWidthHalf + .5f);
    io_rtEdgeROI.m_right = (long)(ptRoiDst.m_x + (float)nRoiWidthHalf + .5f);
    io_rtEdgeROI.m_top = (long)(ptRoiDst.m_y - (float)nRoiHeightHalf + .5f);
    io_rtEdgeROI.m_bottom = (long)(ptRoiDst.m_y + (float)nRoiHeightHalf + .5f);

    return true;
}

long Inspection_Impl::BodyEdgeDetect_LeftbyMatching(
    const Para& para, const Ipvm::Image8u& image, const Ipvm::Point32r2& paneCenter, Ipvm::LineEq32r eqLineCenter)
{
    float nSearchROIWidth = 0.f;
    Ipvm::Point32s2 ptStart(0, 0);
    long nSize = 0;

    long nEdgeDirection = para.m_nEdgeDirection ? PI_ED_DIR_RISING : PI_ED_DIR_FALLING;
    long nSearchDirection(0);

    std::vector<Ipvm::Point32r2> vecLeftNewpoint;
    std::vector<Ipvm::Point32r2> vecFirstEdge;
    std::vector<Ipvm::Point32r2> vecBestEdge;

    std::vector<BYTE> vecfProfile;
    std::vector<BYTE> vecfProfile_Temp;

    m_result_edgeAlign.vecLeftSearchROI.resize(para.m_searchPara[LEFT].size());

    nSize = (long)m_result_edgeAlign.vecLeftSearchROI.size();

    auto& scale = m_processor.getScale();
    float bodySizeX_px; // = scale.convert_umToPixelX(m_packageSpec.m_bodyInfoMaster->GetBodySizeX());
    float bodySizeY_px; // = scale.convert_umToPixelY(m_packageSpec.m_bodyInfoMaster->GetBodySizeY());
    m_processor.GetPackageSize(true, bodySizeX_px, bodySizeY_px);

    for (long nCount = 0; nCount < nSize; nCount++)
    {
        m_result_edgeAlign.vecLeftSearchROI[nCount]
            = para.m_searchPara[LEFT][nCount].ToRect(scale, LEFT, paneCenter, bodySizeX_px, bodySizeY_px);
        MoveEdgeROIforSide(eqLineCenter,
            m_result_edgeAlign.vecLeftSearchROI
                [nCount]); //이 함수 내부에서 피팅 파라미터가 초기 값이거나 Side Vision이 아니면 ROI는 그대로 나온다

        Ipvm::Rect32s rtROI = m_result_edgeAlign.vecLeftSearchROI[nCount];
        Ipvm::Rect32s rtTempROI = rtROI;
        rtTempROI.InflateRect(5, 5, 5, 5);

        rtROI = ModifiedROIbyImage(image, rtROI);

        m_result_edgeAlign.vecLeftSearchROI[nCount] = rtROI;
        nSearchROIWidth = (rtROI.m_bottom - rtROI.m_top) * 0.1f;

        BOOL bFirstEdgeContinue = TRUE;
        BOOL bBestEdgeContinue = TRUE;

        //float fFullSearchLength = (float)rtROI.Height();
        //long nJump = fFullSearchLength / (fFullSearchLength * para.m_fEdgeSamplerate / 100.f) + .5f;
        float fpx2um_x = scale.pixelToUm().m_x;
        /*float fpx2um_y = scale.pixelToUm().m_y;
		float fpx2um = (fpx2um_x + fpx2um_y) * 0.5f;*/
        long nJump = (long)((para.m_fEdgeAlignGap_um / fpx2um_x) + 0.5f);
        long nROISize = rtROI.Height();
        for (long nEdgeCount = 0; nEdgeCount < nROISize; nEdgeCount += nJump)
        {
            bFirstEdgeContinue = TRUE;
            bBestEdgeContinue = TRUE;

            if (para.m_nSearchDirection == PI_ED_DIR_OUTER) //In->Out)
            {
                nSearchDirection = LEFT;
                ptStart = Ipvm::Point32s2(rtROI.m_right, rtROI.m_top + nEdgeCount);
            }
            else
            {
                nSearchDirection = RIGHT;
                ptStart = Ipvm::Point32s2(rtROI.m_left, rtROI.m_top + nEdgeCount);
            }

            if (para.m_nEdgeDetectMode == EdgeDetectMode_FirstEdge
                || para.m_nEdgeDetectMode == EdgeDetectMode_AutoDetect)
            {
                Ipvm::Point32r2 firstEdge{};
                if (m_pEdgeDetect->PI_ED_CalcAvgEdgePointFromImage(nEdgeDirection, ptStart, nSearchDirection,
                        rtROI.Width(), CAST_INT(nSearchROIWidth), image, firstEdge, TRUE))
                {
                    m_result_edgeAlign.vecLeftEdgeAlignPoints_Total.push_back(firstEdge);
                    if (rtTempROI.PtInRect(Ipvm::Point32s2((long)firstEdge.m_x, (long)firstEdge.m_y))
                        && bFirstEdgeContinue)
                    {
                        long nEdgeSize = (long)vecFirstEdge.size();
                        if (nEdgeSize > 0)
                        {
                            if (vecFirstEdge[nEdgeSize - 1].m_y == firstEdge.m_y)
                                continue;
                        }
                        vecLeftNewpoint.push_back(firstEdge); // 하나의 공간에 first와 Best의 Edge를 모두 넣는다.
                        vecFirstEdge.push_back(firstEdge);
                    }
                }
            }

            if (para.m_nEdgeDetectMode == EdgeDetectMode_BestEdge
                || para.m_nEdgeDetectMode == EdgeDetectMode_AutoDetect)
            {
                Ipvm::Point32r2 bestEdge{};
                if (m_pEdgeDetect->PI_ED_CalcAvgEdgePointFromImage(nEdgeDirection, ptStart, nSearchDirection,
                        rtROI.Width(), CAST_INT(nSearchROIWidth), image, bestEdge, FALSE))
                {
                    m_result_edgeAlign.vecLeftEdgeAlignPoints_Total.push_back(bestEdge);
                    if (rtTempROI.PtInRect(Ipvm::Point32s2((long)bestEdge.m_x, (long)bestEdge.m_y))
                        && bBestEdgeContinue)
                    {
                        long nEdgeSize = (long)vecBestEdge.size();
                        if (nEdgeSize > 0)
                        {
                            if (vecBestEdge[nEdgeSize - 1].m_y == bestEdge.m_y)
                                continue;
                        }
                        vecLeftNewpoint.push_back(bestEdge);
                        vecBestEdge.push_back(bestEdge);
                    }
                }
            }
        }

        if (vecLeftNewpoint.size() <= 0)
        {
            continue;
        }
    }

    if (RoughAlign_LineAlign(para, enum_Left, vecLeftNewpoint, vecFirstEdge, vecBestEdge))
        return -1;

    long nFinalEdgeNum = (long)m_result_edgeAlign.vecLeftEdgeAlignPoints.size(); //kircheis_USI_Side
    if (nFinalEdgeNum <= 0)
        return -1;

    return 0;
}

long Inspection_Impl::BodyEdgeDetect_TopbyMatching(
    const Para& para, const Ipvm::Image8u& image, const Ipvm::Point32r2& paneCenter)
{
    float nSearchROIWidth = 0.f;
    Ipvm::Point32s2 ptStart(0, 0);
    long nSize = 0;

    long nEdgeDirection = para.m_nEdgeDirection ? PI_ED_DIR_RISING : PI_ED_DIR_FALLING;
    long nSearchDirection(0);

    std::vector<Ipvm::Point32r2> vecTopNewpoint;
    std::vector<Ipvm::Point32r2> vecFirstEdge;
    std::vector<Ipvm::Point32r2> vecBestEdge;

    std::vector<BYTE> vecfProfile_Temp;
    std::vector<BYTE> vecfProfile;

    const auto& scale = m_processor.getScale();

    float bodySizeX_px; // = scale.convert_umToPixelX(m_packageSpec.m_bodyInfoMaster->GetBodySizeX());
    float bodySizeY_px; // = scale.convert_umToPixelY(m_packageSpec.m_bodyInfoMaster->GetBodySizeY());
    m_processor.GetPackageSize(true, bodySizeX_px, bodySizeY_px);

    long nRoiNum = (long)para.m_searchPara[UP].size();

    m_result_edgeAlign.vecTopSearchROI.resize(para.m_searchPara[UP].size());
    for (long i = 0; i < nRoiNum; i++)
    {
        m_result_edgeAlign.vecTopSearchROI[i]
            = para.m_searchPara[UP][i].ToRect(scale, UP, paneCenter, bodySizeX_px, bodySizeY_px);
    }

    nSize = (long)m_result_edgeAlign.vecTopSearchROI.size();
    for (long nCount = 0; nCount < nSize; nCount++)
    {
        Ipvm::Rect32s rtROI = m_result_edgeAlign.vecTopSearchROI[nCount];
        Ipvm::Rect32s rtTempROI = rtROI;
        rtTempROI.InflateRect(5, 5, 5, 5);

        rtROI = ModifiedROIbyImage(image, rtROI);

        m_result_edgeAlign.vecTopSearchROI[nCount] = rtROI;
        nSearchROIWidth = (rtROI.m_right - rtROI.m_left) * 0.1f;

        if (m_packageSpec.m_bodyInfoMaster->m_nRoundPackageOption == Round_UpDown) //kircheis_RoundPKG
            nSearchROIWidth = 3;

        BOOL bFirstEdgeContinue = TRUE;
        BOOL bBestEdgeContinue = TRUE;

        long nROISize = rtROI.Width();
        /*float fFullSearchLength = (float)rtROI.Width();
		long nJump = fFullSearchLength / (fFullSearchLength * para.m_fEdgeSamplerate / 100.f) + .5f; */
        float fpx2um_x = scale.pixelToUm().m_x;
        /*float fpx2um_y = scale.pixelToUm().m_y;
		float fpx2um = (fpx2um_x + fpx2um_y) * 0.5f;*/
        long nJump = (long)((para.m_fEdgeAlignGap_um / fpx2um_x) + 0.5f);
        for (long nEdgeCount = 0; nEdgeCount < nROISize; nEdgeCount += nJump)
        {
            bFirstEdgeContinue = TRUE;
            bBestEdgeContinue = TRUE;

            if (para.m_nSearchDirection == PI_ED_DIR_OUTER) //In->Out)
            {
                nSearchDirection = UP;
                ptStart = Ipvm::Point32s2(rtROI.m_left + nEdgeCount, rtROI.m_bottom);
            }
            else
            {
                nSearchDirection = DOWN;
                ptStart = Ipvm::Point32s2(rtROI.m_left + nEdgeCount, rtROI.m_top);
            }

            if (para.m_nEdgeDetectMode == EdgeDetectMode_FirstEdge
                || para.m_nEdgeDetectMode == EdgeDetectMode_AutoDetect)
            {
                Ipvm::Point32r2 firstEdge{};
                if (m_pEdgeDetect->PI_ED_CalcAvgEdgePointFromImage(nEdgeDirection, ptStart, nSearchDirection,
                        rtROI.Height(), CAST_INT(nSearchROIWidth), image, firstEdge, TRUE))
                {
                    m_result_edgeAlign.vecTopEdgeAlignPoints_Total.push_back(firstEdge);

                    if (rtTempROI.PtInRect(Ipvm::Point32s2(CAST_INT32T(firstEdge.m_x), CAST_INT32T(firstEdge.m_y)))
                        && bFirstEdgeContinue)
                    {
                        long nEdgeSize = (long)vecFirstEdge.size();
                        if (nEdgeSize > 0)
                        {
                            if (vecFirstEdge[nEdgeSize - 1].m_x == firstEdge.m_x)
                                continue;
                        }
                        vecTopNewpoint.push_back(firstEdge);
                        vecFirstEdge.push_back(firstEdge);
                    }
                }
            }

            if (para.m_nEdgeDetectMode == EdgeDetectMode_BestEdge
                || para.m_nEdgeDetectMode == EdgeDetectMode_AutoDetect)
            {
                Ipvm::Point32r2 bestEdge{};
                if (m_pEdgeDetect->PI_ED_CalcAvgEdgePointFromImage(nEdgeDirection, ptStart, nSearchDirection,
                        rtROI.Height(), CAST_INT(nSearchROIWidth), image, bestEdge, FALSE))
                {
                    m_result_edgeAlign.vecTopEdgeAlignPoints_Total.push_back(bestEdge);

                    if (rtTempROI.PtInRect(Ipvm::Point32s2((long)bestEdge.m_x, (long)bestEdge.m_y))
                        && bBestEdgeContinue)
                    {
                        long nEdgeSize = (long)vecBestEdge.size();
                        if (nEdgeSize > 0)
                        {
                            if (vecBestEdge[nEdgeSize - 1].m_x == bestEdge.m_x)
                                continue;
                        }
                        vecTopNewpoint.push_back(bestEdge); // 하나의 공간에 first와 Best의 Edge를 모두 넣는다.
                        vecBestEdge.push_back(bestEdge);
                    }
                }
            }
        }

        if (vecTopNewpoint.size() <= 0)
        {
            continue;
        }
    }

    if (RoughAlign_LineAlign(para, enum_Top, vecTopNewpoint, vecFirstEdge, vecBestEdge))
        return -1;

    if (m_result_edgeAlign.vecTopEdgeAlignPoints.size() <= 0)
        return -1;

    return 0;
}

long Inspection_Impl::BodyEdgeDetect_RightbyMatching(
    const Para& para, const Ipvm::Image8u& image, const Ipvm::Point32r2& paneCenter, Ipvm::LineEq32r eqLineCenter)
{
    float nSearchROIWidth = 0.f;
    Ipvm::Point32s2 ptStart(0, 0);

    long nEdgeDirection = para.m_nEdgeDirection ? PI_ED_DIR_RISING : PI_ED_DIR_FALLING;
    long nSearchDirection(0);

    std::vector<Ipvm::Point32r2> vecRightNewpoint;
    std::vector<Ipvm::Point32r2> vecFirstEdge;
    std::vector<Ipvm::Point32r2> vecBestEdge;

    std::vector<BYTE> vecfProfile_Temp;
    std::vector<BYTE> vecfProfile;

    m_result_edgeAlign.vecRightSearchROI.resize(para.m_searchPara[RIGHT].size());

    const auto& scale = m_processor.getScale();

    float bodySizeX_px; // = scale.convert_umToPixelX(m_packageSpec.m_bodyInfoMaster->GetBodySizeX());
    float bodySizeY_px; // = scale.convert_umToPixelY(m_packageSpec.m_bodyInfoMaster->GetBodySizeY());
    m_processor.GetPackageSize(true, bodySizeX_px, bodySizeY_px);

    for (long nCount = 0; nCount < (long)para.m_searchPara[RIGHT].size(); nCount++)
    {
        m_result_edgeAlign.vecRightSearchROI[nCount]
            = para.m_searchPara[RIGHT][nCount].ToRect(scale, RIGHT, paneCenter, bodySizeX_px, bodySizeY_px);
        MoveEdgeROIforSide(eqLineCenter,
            m_result_edgeAlign.vecRightSearchROI
                [nCount]); //이 함수 내부에서 피팅 파라미터가 초기 값이거나 Side Vision이 아니면 ROI는 그대로 나온다
        Ipvm::Rect32s rtROI = m_result_edgeAlign.vecRightSearchROI[nCount];
        Ipvm::Rect32s rtTempROI = rtROI;
        rtTempROI.InflateRect(5, 5, 5, 5);

        rtROI = ModifiedROIbyImage(image, rtROI);

        m_result_edgeAlign.vecRightSearchROI[nCount] = rtROI;
        nSearchROIWidth = (rtROI.m_bottom - rtROI.m_top) * 0.1f;

        BOOL bFirstEdgeContinue = TRUE;
        BOOL bBestEdgeContinue = TRUE;

        long nROISize = rtROI.Height();
        /*float fFullSearchLength = (float)rtROI.Height();
		long nJump = fFullSearchLength / (fFullSearchLength * para.m_fEdgeSamplerate / 100.f) + .5f;*/
        float fpx2um_x = scale.pixelToUm().m_x;
        /*float fpx2um_y = scale.pixelToUm().m_y;
		float fpx2um = (fpx2um_x + fpx2um_y) * 0.5f;*/
        long nJump = (long)((para.m_fEdgeAlignGap_um / fpx2um_x) + 0.5f);
        for (long nEdgeCount = 0; nEdgeCount < nROISize; nEdgeCount += nJump)
        {
            bFirstEdgeContinue = TRUE;
            bBestEdgeContinue = TRUE;

            if (para.m_nSearchDirection == PI_ED_DIR_OUTER) //In->Out)
            {
                nSearchDirection = RIGHT;
                ptStart = Ipvm::Point32s2(rtROI.m_left, rtROI.m_top + nEdgeCount);
            }
            else
            {
                nSearchDirection = LEFT;
                ptStart = Ipvm::Point32s2(rtROI.m_right, rtROI.m_top + nEdgeCount);
            }

            if (para.m_nEdgeDetectMode == EdgeDetectMode_FirstEdge
                || para.m_nEdgeDetectMode == EdgeDetectMode_AutoDetect)
            {
                Ipvm::Point32r2 firstEdge{};
                if (m_pEdgeDetect->PI_ED_CalcAvgEdgePointFromImage(nEdgeDirection, ptStart, nSearchDirection,
                        rtROI.Width(), CAST_INT(nSearchROIWidth), image, firstEdge, TRUE))
                {
                    m_result_edgeAlign.vecRightEdgeAlignPoints_Total.push_back(firstEdge);

                    if (rtTempROI.PtInRect(Ipvm::Point32s2((long)firstEdge.m_x, (long)firstEdge.m_y))
                        && bFirstEdgeContinue)
                    {
                        long nEdgeSize = (long)vecFirstEdge.size();
                        if (nEdgeSize > 0)
                        {
                            if (vecFirstEdge[nEdgeSize - 1].m_y == firstEdge.m_y)
                                continue;
                        }
                        vecRightNewpoint.push_back(firstEdge);
                        vecFirstEdge.push_back(firstEdge);
                    }
                }
            }

            if (para.m_nEdgeDetectMode == EdgeDetectMode_BestEdge
                || para.m_nEdgeDetectMode == EdgeDetectMode_AutoDetect)
            {
                Ipvm::Point32r2 bestEdge{};
                if (m_pEdgeDetect->PI_ED_CalcAvgEdgePointFromImage(nEdgeDirection, ptStart, nSearchDirection,
                        rtROI.Width(), CAST_INT(nSearchROIWidth), image, bestEdge, FALSE))
                {
                    m_result_edgeAlign.vecRightEdgeAlignPoints_Total.push_back(bestEdge);

                    if (rtTempROI.PtInRect(Ipvm::Point32s2((long)bestEdge.m_x, (long)bestEdge.m_y))
                        && bBestEdgeContinue)
                    {
                        long nEdgeSize = (long)vecBestEdge.size();
                        if (nEdgeSize > 0)
                        {
                            if (vecBestEdge[nEdgeSize - 1].m_y == bestEdge.m_y)
                                continue;
                        }
                        vecRightNewpoint.push_back(bestEdge); // 하나의 공간에 first와 Best의 Edge를 모두 넣는다.
                        vecBestEdge.push_back(bestEdge);
                    }
                }
            }
        }

        if (vecRightNewpoint.size() <= 0)
        {
            continue;
        }
    }

    if (RoughAlign_LineAlign(para, enum_Right, vecRightNewpoint, vecFirstEdge, vecBestEdge))
        return -1;

    long nFinalEdgeNum = (long)m_result_edgeAlign.vecRightEdgeAlignPoints.size(); //kircheis_USI_Side
    if (nFinalEdgeNum <= 0)
        return -1;

    return 0;
}

long Inspection_Impl::BodyEdgeDetect_BottombyMatching(
    const Para& para, const Ipvm::Image8u& image, const Ipvm::Point32r2& paneCenter)
{
    float nSearchROIWidth = 0.f;
    Ipvm::Point32s2 ptStart(0, 0);
    long nSize = 0;

    long nEdgeDirection = para.m_nEdgeDirection ? PI_ED_DIR_RISING : PI_ED_DIR_FALLING;
    long nSearchDirection(0);

    std::vector<Ipvm::Point32r2> vecBottomNewpoint;
    std::vector<Ipvm::Point32r2> vecFirstEdge;
    std::vector<Ipvm::Point32r2> vecBestEdge;

    std::vector<BYTE> vecfProfile_Temp;
    std::vector<BYTE> vecfProfile;

    const auto& scale = m_processor.getScale();

    float bodySizeX_px; // = scale.convert_umToPixelX(m_packageSpec.m_bodyInfoMaster->GetBodySizeX());
    float bodySizeY_px; // = scale.convert_umToPixelY(m_packageSpec.m_bodyInfoMaster->GetBodySizeY());
    m_processor.GetPackageSize(true, bodySizeX_px, bodySizeY_px);

    long nRoiNum = (long)para.m_searchPara[DOWN].size();

    m_result_edgeAlign.vecBottomSearchROI.resize(nRoiNum);
    for (long i = 0; i < nRoiNum; i++)
    {
        m_result_edgeAlign.vecBottomSearchROI[i]
            = para.m_searchPara[DOWN][i].ToRect(scale, DOWN, paneCenter, bodySizeX_px, bodySizeY_px);
    }

    nSize = (long)m_result_edgeAlign.vecBottomSearchROI.size();
    for (long nCount = 0; nCount < nSize; nCount++)
    {
        Ipvm::Rect32s rtROI = m_result_edgeAlign.vecBottomSearchROI[nCount];
        Ipvm::Rect32s rtTempROI = rtROI;
        rtTempROI.InflateRect(5, 5, 5, 5);

        rtROI = ModifiedROIbyImage(image, rtROI);

        m_result_edgeAlign.vecBottomSearchROI[nCount] = rtROI;
        nSearchROIWidth = (rtROI.m_right - rtROI.m_left) * 0.1f;
        if (m_packageSpec.m_bodyInfoMaster->m_nRoundPackageOption == Round_UpDown) //kircheis_RoundPKG
            nSearchROIWidth = 3;

        BOOL bFirstEdgeContinue = TRUE;
        BOOL bBestEdgeContinue = TRUE;

        long nROISize = rtROI.Width();
        /*float fFullSearchLength = (float)rtROI.Width();
		long nJump = fFullSearchLength / (fFullSearchLength * para.m_fEdgeSamplerate / 100.f) + .5f; */
        float fpx2um_x = scale.pixelToUm().m_x;
        //float fpx2um_y = scale.pixelToUm().m_y;
        //float fpx2um = (fpx2um_x + fpx2um_y) * 0.5f;
        long nJump = (long)((para.m_fEdgeAlignGap_um / fpx2um_x) + 0.5f);
        for (long nEdgeCount = 0; nEdgeCount < nROISize; nEdgeCount += nJump)
        {
            bFirstEdgeContinue = TRUE;
            bBestEdgeContinue = TRUE;

            if (para.m_nSearchDirection == PI_ED_DIR_OUTER) //In->Out)
            {
                nSearchDirection = DOWN;
                ptStart = Ipvm::Point32s2(rtROI.m_left + nEdgeCount, rtROI.m_top);
            }
            else
            {
                nSearchDirection = UP;
                ptStart = Ipvm::Point32s2(rtROI.m_left + nEdgeCount, rtROI.m_bottom);
            }

            if (para.m_nEdgeDetectMode == EdgeDetectMode_FirstEdge
                || para.m_nEdgeDetectMode == EdgeDetectMode_AutoDetect)
            {
                Ipvm::Point32r2 firstEdge{};
                if (m_pEdgeDetect->PI_ED_CalcAvgEdgePointFromImage(nEdgeDirection, ptStart, nSearchDirection,
                        rtROI.Height(), CAST_INT(nSearchROIWidth), image, firstEdge, TRUE))
                {
                    m_result_edgeAlign.vecBottomEdgeAlignPoints_Total.push_back(firstEdge);

                    if (rtTempROI.PtInRect(Ipvm::Point32s2((long)firstEdge.m_x, (long)firstEdge.m_y))
                        && bFirstEdgeContinue)
                    {
                        long nEdgeSize = (long)vecFirstEdge.size();
                        if (nEdgeSize > 0)
                        {
                            if (vecFirstEdge[nEdgeSize - 1].m_x == firstEdge.m_x)
                                continue;
                        }
                        vecBottomNewpoint.push_back(firstEdge);
                        vecFirstEdge.push_back(firstEdge);
                    }
                }
            }

            if (para.m_nEdgeDetectMode == EdgeDetectMode_BestEdge
                || para.m_nEdgeDetectMode == EdgeDetectMode_AutoDetect)
            {
                Ipvm::Point32r2 bestEdge{};
                if (m_pEdgeDetect->PI_ED_CalcAvgEdgePointFromImage(nEdgeDirection, ptStart, nSearchDirection,
                        rtROI.Height(), CAST_INT(nSearchROIWidth), image, bestEdge, FALSE))
                {
                    m_result_edgeAlign.vecBottomEdgeAlignPoints_Total.push_back(bestEdge);

                    if (rtTempROI.PtInRect(Ipvm::Point32s2((long)bestEdge.m_x, (long)bestEdge.m_y))
                        && bBestEdgeContinue)
                    {
                        long nEdgeSize = (long)vecBestEdge.size();
                        if (nEdgeSize > 0)
                        {
                            if (vecBestEdge[nEdgeSize - 1].m_x == bestEdge.m_x)
                                continue;
                        }
                        vecBottomNewpoint.push_back(bestEdge); // 하나의 공간에 first와 Best의 Edge를 모두 넣는다.
                        vecBestEdge.push_back(bestEdge);
                    }
                }
            }
        }

        if (vecBottomNewpoint.size() <= 0)
        {
            continue;
        }
    }

    if (RoughAlign_LineAlign(para, enum_Bottom, vecBottomNewpoint, vecFirstEdge, vecBestEdge))
        return -1;

    if (m_result_edgeAlign.vecBottomEdgeAlignPoints.size() <= 0)
        return -1;

    return 0;
}

long Inspection_Impl::BodyEdgeDetect_Lowtop(
    const Para& para, const Ipvm::Image8u& image, const Ipvm::Point32r2& paneCenter)
{
    float nSearchROIWidth = 0.f;
    Ipvm::Point32s2 ptStart(0, 0);
    long nEdgeSearchDirection = UP;

    auto itrLowTop = para.m_vecrtLowTopSearchROI_BCU.begin();

    std::vector<Ipvm::Point32r2> vecLowTopNewpoint;
    std::vector<Ipvm::Point32r2> vecFirstEdge;
    std::vector<Ipvm::Point32r2> vecBestEdge;

    long nEdgeDirection = para.m_nEdgeDirection ? PI_ED_DIR_RISING : PI_ED_DIR_FALLING;

    while (itrLowTop != para.m_vecrtLowTopSearchROI_BCU.end())
    {
        Ipvm::Rect32s rtROI = m_processor.getScale().convert_BCUToPixel(*itrLowTop, paneCenter);

        rtROI = ModifiedROIbyImage(image, rtROI);

        m_result_edgeAlign.vecLowTopSearchROI.push_back(rtROI);
        nSearchROIWidth = (rtROI.m_right - rtROI.m_left) * 0.1f;

        BOOL bFirstEdgeContinue = TRUE;
        BOOL bBestEdgeContinue = TRUE;

        // 영훈 20150927 : Edge를 2Pixel Sampling 간격을 준다.
        long nROISize = rtROI.Width();
        for (long nEdgeCount = 0; nEdgeCount < nROISize; nEdgeCount += 2)
        {
            if (m_packageSpec.m_deadBug)
            {
                if (para.m_nSearchDirection == PI_ED_DIR_OUTER) //In->Out)
                {
                    nEdgeSearchDirection = DOWN;
                    ptStart = Ipvm::Point32s2(rtROI.m_left + nEdgeCount, rtROI.m_top);
                }
                else
                {
                    nEdgeSearchDirection = UP;
                    ptStart = Ipvm::Point32s2(rtROI.m_left + nEdgeCount, rtROI.m_bottom);
                }
            }
            else
            {
                if (para.m_nSearchDirection == PI_ED_DIR_OUTER) //In->Out)
                {
                    nEdgeSearchDirection = UP;
                    ptStart = Ipvm::Point32s2(rtROI.m_left + nEdgeCount, rtROI.m_bottom);
                }
                else
                {
                    nEdgeSearchDirection = DOWN;
                    ptStart = Ipvm::Point32s2(rtROI.m_left + nEdgeCount, rtROI.m_top);
                }
            }

            if (para.m_nEdgeDetectMode == EdgeDetectMode_FirstEdge
                || para.m_nEdgeDetectMode == EdgeDetectMode_AutoDetect)
            {
                Ipvm::Point32r2 firstEdge{};
                if (m_pEdgeDetect->PI_ED_CalcAvgEdgePointFromImage(nEdgeDirection, ptStart, nEdgeSearchDirection,
                        rtROI.Height(), CAST_INT(nSearchROIWidth), image, firstEdge, TRUE)
                    == TRUE)
                {
                    if (rtROI.PtInRect(Ipvm::Point32s2((long)firstEdge.m_x, (long)firstEdge.m_y)) && bFirstEdgeContinue)
                    {
                        vecLowTopNewpoint.push_back(firstEdge);
                        vecFirstEdge.push_back(firstEdge);
                    }
                }
            }

            if (para.m_nEdgeDetectMode == EdgeDetectMode_BestEdge
                || para.m_nEdgeDetectMode == EdgeDetectMode_AutoDetect)
            {
                Ipvm::Point32r2 bestEdge{};
                if (m_pEdgeDetect->PI_ED_CalcAvgEdgePointFromImage(nEdgeDirection, ptStart, nEdgeSearchDirection,
                        rtROI.Height(), CAST_INT(nSearchROIWidth), image, bestEdge, FALSE)
                    == TRUE)
                {
                    if (rtROI.PtInRect(Ipvm::Point32s2((long)bestEdge.m_x, (long)bestEdge.m_y)) && bBestEdgeContinue)
                    {
                        vecLowTopNewpoint.push_back(bestEdge); // 하나의 공간에 first와 Best의 Edge를 모두 넣는다.
                        vecBestEdge.push_back(bestEdge);
                    }
                }
            }
        }

        if (vecLowTopNewpoint.size() <= 0)
        {
            itrLowTop++;
            continue;
        }

        itrLowTop++;
    }

    if (RoughAlign_LineAlign(para, enum_LowTop, vecLowTopNewpoint, vecFirstEdge, vecBestEdge))
        return -1;

    if (m_result_edgeAlign.vecLowTopEdgeAlignPoints.size() <= 0)
        return -1;

    return 0;
}

Ipvm::Rect32s Inspection_Impl::ModifiedROIbyImage(const Ipvm::Image8u& i_image, const Ipvm::Rect32s& i_rtEdgeSearchROI)
{
    Ipvm::Rect32s rtROI = i_rtEdgeSearchROI;
    rtROI &= Ipvm::Rect32s(i_image);

    Ipvm::Rect32s rtImage = Ipvm::Rect32s(i_image);
    if (rtROI.m_left <= rtImage.m_left)
        rtROI.m_left = rtImage.m_left + 1;
    else if (rtROI.m_left >= rtImage.m_right)
        rtROI.m_left = rtImage.m_right - 1;

    if (rtROI.m_right <= rtImage.m_left)
        rtROI.m_right = rtImage.m_left + 1;
    else if (rtROI.m_right >= rtImage.m_right)
        rtROI.m_right = rtImage.m_right - 1;

    if (rtROI.m_top <= rtImage.m_top)
        rtROI.m_top = rtImage.m_top + 1;
    else if (rtROI.m_top >= rtImage.m_bottom)
        rtROI.m_top = rtImage.m_bottom - 1;

    if (rtROI.m_bottom <= rtImage.m_top)
        rtROI.m_bottom = rtImage.m_top + 1;
    else if (rtROI.m_bottom >= rtImage.m_bottom)
        rtROI.m_bottom = rtImage.m_bottom - 1;

    return rtROI;
}

long Inspection_Impl::SetResult(BOOL bLowtopMode)
{
    // 처음에 네 직선 방정식을 구한다.
    auto& vecsRefLine = m_result_edgeAlign.vecsRefLine;

    if (m_result_edgeAlign.vecLeftEdgeAlignPoints.size() == 0)
        return -1;
    if (m_result_edgeAlign.vecTopEdgeAlignPoints.size() == 0)
        return -1;
    if (m_result_edgeAlign.vecRightEdgeAlignPoints.size() == 0)
        return -1;
    if (m_result_edgeAlign.vecBottomEdgeAlignPoints.size() == 0)
        return -1;

    if (Ipvm::DataFitting::FitToLineRn(long(m_result_edgeAlign.vecLeftEdgeAlignPoints.size()),
            &m_result_edgeAlign.vecLeftEdgeAlignPoints[0], 3.f, vecsRefLine[LEFT])
            != Ipvm::Status::e_ok
        || Ipvm::DataFitting::FitToLineRn(long(m_result_edgeAlign.vecTopEdgeAlignPoints.size()),
               &m_result_edgeAlign.vecTopEdgeAlignPoints[0], 3.f, vecsRefLine[UP])
            != Ipvm::Status::e_ok
        || Ipvm::DataFitting::FitToLineRn(long(m_result_edgeAlign.vecRightEdgeAlignPoints.size()),
               &m_result_edgeAlign.vecRightEdgeAlignPoints[0], 3.f, vecsRefLine[RIGHT])
            != Ipvm::Status::e_ok
        || Ipvm::DataFitting::FitToLineRn(long(m_result_edgeAlign.vecBottomEdgeAlignPoints.size()),
               &m_result_edgeAlign.vecBottomEdgeAlignPoints[0], 3.f, vecsRefLine[DOWN])
            != Ipvm::Status::e_ok)
    {
        return -1;
    }

    if (bLowtopMode)
    {
        if (m_result_edgeAlign.vecLowTopEdgeAlignPoints.size() == 0)
            return -1;

        if (Ipvm::DataFitting::FitToLineRn(long(m_result_edgeAlign.vecLowTopEdgeAlignPoints.size()),
                &m_result_edgeAlign.vecLowTopEdgeAlignPoints[0], 3.f, vecsRefLine[LOWTOP])
            != Ipvm::Status::e_ok)
        {
            return -1;
        }
    }

    // 직선 방정식으로부터 바디 센터 라인을 계산한다.
    Ipvm::Geometry::GetBisection(vecsRefLine[LEFT], vecsRefLine[RIGHT], vecsRefLine[VER_CENTER_LINE]);
    Ipvm::Geometry::GetBisection(vecsRefLine[UP], vecsRefLine[DOWN], vecsRefLine[HOR_CENTER_LINE]);

    // 직선 방정식으로부터 네 모서리를 계산한다.
    Ipvm::Point32r2& ptLeftTop = m_result_edgeAlign.fptLT;
    Ipvm::Point32r2& ptRightTop = m_result_edgeAlign.fptRT;
    Ipvm::Point32r2& ptRightBottom = m_result_edgeAlign.fptRB;
    Ipvm::Point32r2& ptLeftBottom = m_result_edgeAlign.fptLB;

    if (Ipvm::Geometry::GetCrossPoint(vecsRefLine[LEFT], vecsRefLine[UP], ptLeftTop) != Ipvm::Status::e_ok)
        return -1;
    if (Ipvm::Geometry::GetCrossPoint(vecsRefLine[UP], vecsRefLine[RIGHT], ptRightTop) != Ipvm::Status::e_ok)
        return -1;
    if (Ipvm::Geometry::GetCrossPoint(vecsRefLine[RIGHT], vecsRefLine[DOWN], ptRightBottom) != Ipvm::Status::e_ok)
        return -1;
    if (Ipvm::Geometry::GetCrossPoint(vecsRefLine[DOWN], vecsRefLine[LEFT], ptLeftBottom) != Ipvm::Status::e_ok)
        return -1;

    // 영훈 - Lowtop도 그려주자
    if (bLowtopMode)
    {
        Ipvm::Point32r2& ptLowtopL = m_result_edgeAlign.fptLowtopLT;
        Ipvm::Point32r2& ptLowtopR = m_result_edgeAlign.fptLowtopRT;

        if (Ipvm::Geometry::GetCrossPoint(vecsRefLine[LOWTOP], vecsRefLine[UP], ptLowtopL) != Ipvm::Status::e_ok)
            return -1;
        if (Ipvm::Geometry::GetCrossPoint(vecsRefLine[LOWTOP], vecsRefLine[RIGHT], ptLowtopR) != Ipvm::Status::e_ok)
            return -1;

        //Lowtop Align Result
        if (m_packageSpec.m_deadBug)
        {
            m_result_edgeAlign.fLowtopLeft = (ptLeftTop.m_x + ptLeftBottom.m_x) * 0.5f;
            m_result_edgeAlign.fLowtopTop = (ptLeftTop.m_y + ptRightTop.m_y) * 0.5f;
            m_result_edgeAlign.fLowtopRight = (ptRightTop.m_x + ptRightBottom.m_x) * 0.5f;
            m_result_edgeAlign.fLowtopBottom = (ptLowtopL.m_y + ptLowtopR.m_y) * 0.5f;
        }
        else
        {
            m_result_edgeAlign.fLowtopLeft = (ptLeftTop.m_x + ptLeftBottom.m_x) * 0.5f;
            m_result_edgeAlign.fLowtopTop = (ptLowtopL.m_y + ptLowtopR.m_y) * 0.5f;
            m_result_edgeAlign.fLowtopRight = (ptRightTop.m_x + ptRightBottom.m_x) * 0.5f;
            m_result_edgeAlign.fLowtopBottom = (ptLeftBottom.m_y + ptRightBottom.m_y) * 0.5f;
        }
    }

    // 네 모서리로부터 바디 센터를 얻는다.
    m_result_edgeAlign.m_center.m_x = (ptLeftTop.m_x + ptRightTop.m_x + ptRightBottom.m_x + ptLeftBottom.m_x) * 0.25f;
    m_result_edgeAlign.m_center.m_y = (ptLeftTop.m_y + ptRightTop.m_y + ptRightBottom.m_y + ptLeftBottom.m_y) * 0.25f;

    auto panelCenter = m_result_edgeAlign.getPaneCenter();
    const auto& scale = m_processor.getScale();

    m_result_edgeAlign.m_analysis_packageOffsetX_um
        = scale.convert_pixelToUmX(m_result_edgeAlign.m_center.m_x - panelCenter.m_x);
    m_result_edgeAlign.m_analysis_packageOffsetY_um
        = scale.convert_pixelToUmY(m_result_edgeAlign.m_center.m_y - panelCenter.m_y);

    // 네 모서리로부터 바디 앵글을 얻는다.
    const float angleLower = (float)atan2(ptRightBottom.m_y - ptLeftBottom.m_y, ptRightBottom.m_x - ptLeftBottom.m_x);
    const float angleUpper = (float)atan2(ptRightTop.m_y - ptLeftTop.m_y, ptRightTop.m_x - ptLeftTop.m_x);

    float fTopBottomAngle = (angleLower + angleUpper) * 0.5f;
    m_result_edgeAlign.SetAngle(fTopBottomAngle);

    float fAngleLeft = CAST_FLOAT(
        atan2(ptLeftBottom.m_y - ptLeftTop.m_y, ptLeftBottom.m_x - ptLeftTop.m_x) - (90. * 0.017453292519943));
    float fAngleRight = CAST_FLOAT(
        atan2(ptRightBottom.m_y - ptRightTop.m_y, ptRightBottom.m_x - ptRightTop.m_x) - (90. * 0.017453292519943));
    float fLeftRightAngle = (fAngleLeft + fAngleRight) / 2.f;

    float fAngle_radVal = (fTopBottomAngle + fLeftRightAngle) * 0.5f;

    // 장축기준으로.. PKG Size X,Y를 참조하여 Max/Min이 5이상 차이 날경우 무조건 장축기준으로 Angle을 Setting하여 준다
    const long nLimitDiffSizeVal(5);
    float fPackageSizeX = 0.f;
    float fPackageSizeY = 0.f;
    m_processor.GetPackageSize(false, fPackageSizeX, fPackageSizeY);

    float fMaxSize = max(fPackageSizeX, fPackageSizeY);
    float fMinSize = min(fPackageSizeX, fPackageSizeY);
    long nDiffSize = CAST_LONG((fMaxSize / fMinSize) + .5f);
    if (nDiffSize > nLimitDiffSizeVal)
    {
        fAngle_radVal = fPackageSizeX > fPackageSizeY ? fTopBottomAngle : fLeftRightAngle;
    }

    static const BOOL bIsSideVision = (SystemConfig::GetInstance().IsVisionTypeSide() == TRUE);
    if (bIsSideVision)
        fAngle_radVal = fTopBottomAngle;

    m_result_edgeAlign.SetAngle(fAngle_radVal);

    // 결과 최종 업데이트!!
    m_result_edgeAlign.bAvailable = TRUE;

    return 0;
}

long Inspection_Impl::SetResultForRound_UpDown(
    const bool detailSetupMode, const Ipvm::Image8u& image, long nSelPackType) //kircheis_FPS
{
    if (nSelPackType != Round_UpDown) //PKG_TYPE_FPS_TB)
        return -1;
    // 먼저 좌우의 직선 방정식과 센터 라인의 방정식을 구한다.
    auto& vecsRefLine = m_result_edgeAlign.vecsRefLine;
    std::vector<Ipvm::Point32r2> vecfptValue;

    if (m_result_edgeAlign.vecLeftEdgeAlignPoints.size() == 0)
        return -1;
    if (m_result_edgeAlign.vecRightEdgeAlignPoints.size() == 0)
        return -1;

    if (Ipvm::DataFitting::FitToLineRn(long(m_result_edgeAlign.vecLeftEdgeAlignPoints.size()),
            &m_result_edgeAlign.vecLeftEdgeAlignPoints[0], 3.f, vecsRefLine[LEFT])
            != Ipvm::Status::e_ok
        || Ipvm::DataFitting::FitToLineRn(long(m_result_edgeAlign.vecRightEdgeAlignPoints.size()),
               &m_result_edgeAlign.vecRightEdgeAlignPoints[0], 3.f, vecsRefLine[RIGHT])
            != Ipvm::Status::e_ok)
    {
        return -1;
    }

    Ipvm::Geometry::GetBisection(vecsRefLine[LEFT], vecsRefLine[RIGHT], vecsRefLine[VER_CENTER_LINE]);

    // Top Edge중 가장 상단의 Point를 구한다. 이때 Package가 기울었으면 오류가 발생할 가능성이 높으므로
    //센터라인을 기준의 정사영으로 사용하도록한다.

    // 배열 변수명 다 쓰려면 옆으로 길어지니까 짧게 가자
    Ipvm::LineEq32r centerLine = vecsRefLine[VER_CENTER_LINE];
    Ipvm::LineEq32r leftLine = vecsRefLine[LEFT];
    Ipvm::LineEq32r rightLine = vecsRefLine[RIGHT];

    //사용할 변수들 다 선언해 놓고...
    long nMinID(0), nMaxID(0);
    Ipvm::Status ellipseResult = Ipvm::Status::e_ok;

    BOOL bCurveResult;
    long nPointNum;
    float fMinMaxY = 1000000.f;
    double dPointZero;
    Ipvm::Point32r2 fCurPoint;
    Ipvm::EllipseEq32r ellipse;
    Ipvm::EllipseEq32r sEllipse;
    Curve2DEq curveEq;
    std::vector<Ipvm::Point32r2> vecfptNewLine(3);
    std::vector<Ipvm::EllipseEq32r> vecEllipse(0);

    nPointNum
        = (long)
              m_result_edgeAlign.vecTopEdgeAlignPoints_Total.size(); //Edge Point 갯수가 8개 이상일 때만 Align 시도하자
    if (nPointNum < 8) //kircheis_FootTip_Round
        return -1;

    //먼저 Ellipse Fitting을 시도해보고...
    ellipseResult
        = Ipvm::DataFitting::FitToEllipse(nPointNum, &m_result_edgeAlign.vecTopEdgeAlignPoints_Total[0], ellipse);

    if (ellipseResult == Ipvm::Status::e_ok && ellipse.m_xradius > 1
        && ellipse.m_yradius > 1) //성공했으면 그 결과를 적용한다.
    {
        sEllipse = ellipse;
        vecEllipse.push_back(sEllipse);
        vecfptNewLine[1].m_x = ellipse.m_x;
        vecfptNewLine[1].m_y = ellipse.m_y - ellipse.m_yradius;
    }
    else //실패했으면
    {
        bCurveResult = CPI_Geometry::Get2DCurveFitting(
            m_result_edgeAlign.vecTopEdgeAlignPoints_Total, curveEq); //Curve Fitting을 시도하고...
        if (bCurveResult && curveEq.m_c != -1) //성공했으면 그 결과를 적용한다.
        {
            dPointZero = (-curveEq.m_b / (curveEq.m_c * 2.));
            vecfptNewLine[1].m_x = (float)dPointZero;
            vecfptNewLine[1].m_y
                = (float)((curveEq.m_c * dPointZero * dPointZero) + (curveEq.m_b * dPointZero) + curveEq.m_a);
        }
        else //위의 두가지 Fitting에 모두 실패했으면...
        {
            for (long idx = 0; idx < nPointNum; idx++)
            {
                fCurPoint = m_result_edgeAlign.vecTopEdgeAlignPoints_Total[idx];
                Ipvm::Point32r2 findCur;
                Ipvm::Geometry::GetFootOfPerpendicular(centerLine, fCurPoint, findCur);
                if (fMinMaxY >= findCur.m_y) //그냥 중심에서 가장 먼 Point하나를 쓰자...
                {
                    fMinMaxY = findCur.m_y;
                    nMinID = idx;
                }
            }
            vecfptNewLine[1] = m_result_edgeAlign.vecTopEdgeAlignPoints_Total[nMinID];
        }
    }

    //위에서 찾은 Point로 좌우 측면 직선과의 정사영을 구하고 이를 연결하는 직선을 구성하여 Align 결과로 적용
    Ipvm::Geometry::GetFootOfPerpendicular(leftLine, vecfptNewLine[1], vecfptNewLine[0]);
    Ipvm::Geometry::GetFootOfPerpendicular(rightLine, vecfptNewLine[1], vecfptNewLine[2]);

    if (Ipvm::DataFitting::FitToLineRn(long(vecfptNewLine.size()), &vecfptNewLine[0], 3.f, vecsRefLine[UP])
        != Ipvm::Status::e_ok)
    {
        return -1;
    }

    // Top Edge와 같은 방식으로 하단의 Edge Point를 구한다.
    fMinMaxY = -1000000.f;
    nPointNum = (long)m_result_edgeAlign.vecBottomEdgeAlignPoints_Total.size();
    if (nPointNum < 8) //kircheis_FootTip_Round
        return -1;

    ellipseResult
        = Ipvm::DataFitting::FitToEllipse(nPointNum, &m_result_edgeAlign.vecBottomEdgeAlignPoints_Total[0], ellipse);

    if (ellipseResult == Ipvm::Status::e_ok && ellipse.m_xradius > 1 && ellipse.m_yradius > 1)
    {
        sEllipse = ellipse;
        vecEllipse.push_back(sEllipse);
        vecfptNewLine[1].m_x = ellipse.m_x;
        vecfptNewLine[1].m_y = ellipse.m_y + ellipse.m_yradius;
    }
    else
    {
        bCurveResult = CPI_Geometry::Get2DCurveFitting(m_result_edgeAlign.vecBottomEdgeAlignPoints_Total, curveEq);
        if (bCurveResult && curveEq.m_c != -1)
        {
            dPointZero = (-curveEq.m_b / (curveEq.m_c * 2.));
            vecfptNewLine[1].m_x = (float)dPointZero;
            vecfptNewLine[1].m_y
                = (float)((curveEq.m_c * dPointZero * dPointZero) + (curveEq.m_b * dPointZero) + curveEq.m_a);
        }
        else
        {
            for (long idx = 0; idx < nPointNum; idx++)
            {
                fCurPoint = m_result_edgeAlign.vecBottomEdgeAlignPoints_Total[idx];
                Ipvm::Point32r2 findCur;
                Ipvm::Geometry::GetFootOfPerpendicular(centerLine, fCurPoint, findCur);
                if (fMinMaxY <= findCur.m_y)
                {
                    fMinMaxY = findCur.m_y;
                    nMaxID = idx;
                }
            }
            vecfptNewLine[1] = m_result_edgeAlign.vecBottomEdgeAlignPoints_Total[nMaxID];
        }
    }

    Ipvm::Geometry::GetFootOfPerpendicular(leftLine, vecfptNewLine[1], vecfptNewLine[0]);
    Ipvm::Geometry::GetFootOfPerpendicular(rightLine, vecfptNewLine[1], vecfptNewLine[2]);

    if (Ipvm::DataFitting::FitToLineRn(long(vecfptNewLine.size()), &vecfptNewLine[0], 3.f, vecsRefLine[DOWN])
        != Ipvm::Status::e_ok)
    {
        return -1;
    }

    m_processor.SetDebugInfoItem(detailSetupMode, _T("Round Ellipse"), vecEllipse);

    //만들어진 Align 정보를 이용하여 상하단의 타원을 만들어 확인한다.
    vecEllipse.clear();

    const auto& scale = m_processor.getScale();
    float fDistR = scale.convert_umToPixelY(m_packageSpec.m_bodyInfoMaster->m_fRoundPackageRadius);

    Ipvm::Point32r2 fptCross, fptOrigin, fptLeftCross, fptRightCross;

    Ipvm::Geometry::GetCrossPoint(centerLine, vecsRefLine[UP], fptCross);

    CPI_Geometry::GetDistPointOnLine(
        fptCross.m_x, fptCross.m_y, centerLine.m_a, centerLine.m_b, fDistR, TRUE, fptOrigin.m_x, fptOrigin.m_y);
    Ipvm::Geometry::GetFootOfPerpendicular(leftLine, fptOrigin, fptLeftCross);
    Ipvm::Geometry::GetFootOfPerpendicular(rightLine, fptOrigin, fptRightCross);
    fptOrigin.m_x = (fptLeftCross.m_x + fptRightCross.m_x) * .5f;
    fptOrigin.m_y = (fptLeftCross.m_y + fptRightCross.m_y) * .5f;

    float fDist(0.f);
    Ipvm::Geometry::GetDistance(fptLeftCross, fptRightCross, fDist);

    sEllipse.m_x = fptOrigin.m_x;
    sEllipse.m_y = fptOrigin.m_y;
    sEllipse.m_xradius = fDist * .5f;
    sEllipse.m_yradius = (float)fabs(fptOrigin.m_y - fptCross.m_y);
    vecEllipse.push_back(sEllipse);

    Ipvm::Geometry::GetCrossPoint(centerLine, vecsRefLine[DOWN], fptCross);

    CPI_Geometry::GetDistPointOnLine(
        fptCross.m_x, fptCross.m_y, centerLine.m_a, centerLine.m_b, -fDistR, TRUE, fptOrigin.m_x, fptOrigin.m_y);
    Ipvm::Geometry::GetFootOfPerpendicular(leftLine, fptOrigin, fptLeftCross);
    Ipvm::Geometry::GetFootOfPerpendicular(rightLine, fptOrigin, fptRightCross);
    fptOrigin.m_x = (fptLeftCross.m_x + fptRightCross.m_x) * .5f;
    fptOrigin.m_y = (fptLeftCross.m_y + fptRightCross.m_y) * .5f;

    Ipvm::Geometry::GetDistance(fptLeftCross, fptRightCross, fDist);

    sEllipse.m_x = fptOrigin.m_x;
    sEllipse.m_y = fptOrigin.m_y;
    sEllipse.m_xradius = fDist * .5f;
    sEllipse.m_yradius = (float)fabs(fptOrigin.m_y - fptCross.m_y);
    vecEllipse.push_back(sEllipse);

    m_processor.SetDebugInfoItem(detailSetupMode, _T("Round Spec Ellipse"), vecEllipse);

    if (vecEllipse.size() == 2)
    {
        if (ReAlignForRound_UpDown(detailSetupMode, image, vecEllipse[0], vecEllipse[1]) != 0) //kircheis_FootTip_Round
            return -1;
    }

    // 직선 방정식으로부터 바디 센터 라인을 계산한다.
    Ipvm::Geometry::GetBisection(vecsRefLine[UP], vecsRefLine[DOWN], vecsRefLine[HOR_CENTER_LINE]);

    // 직선 방정식으로부터 네 모서리를 계산한다.
    Ipvm::Point32r2& ptLeftTop = m_result_edgeAlign.fptLT;
    Ipvm::Point32r2& ptRightTop = m_result_edgeAlign.fptRT;
    Ipvm::Point32r2& ptRightBottom = m_result_edgeAlign.fptRB;
    Ipvm::Point32r2& ptLeftBottom = m_result_edgeAlign.fptLB;

    if (Ipvm::Geometry::GetCrossPoint(vecsRefLine[LEFT], vecsRefLine[UP], ptLeftTop) != Ipvm::Status::e_ok)
        return -1;
    if (Ipvm::Geometry::GetCrossPoint(vecsRefLine[UP], vecsRefLine[RIGHT], ptRightTop) != Ipvm::Status::e_ok)
        return -1;
    if (Ipvm::Geometry::GetCrossPoint(vecsRefLine[RIGHT], vecsRefLine[DOWN], ptRightBottom) != Ipvm::Status::e_ok)
        return -1;
    if (Ipvm::Geometry::GetCrossPoint(vecsRefLine[DOWN], vecsRefLine[LEFT], ptLeftBottom) != Ipvm::Status::e_ok)
        return -1;

    // 네 모서리로부터 바디 센터를 얻는다.
    m_result_edgeAlign.m_center.m_x = (ptLeftTop.m_x + ptRightTop.m_x + ptRightBottom.m_x + ptLeftBottom.m_x) / 4.f;
    m_result_edgeAlign.m_center.m_y = (ptLeftTop.m_y + ptRightTop.m_y + ptRightBottom.m_y + ptLeftBottom.m_y) / 4.f;

    // 네 모서리로부터 바디 앵글을 얻는다.
    const float angleLower = (float)atan2(ptRightBottom.m_y - ptLeftBottom.m_y, ptRightBottom.m_x - ptLeftBottom.m_x);
    const float angleUpper = (float)atan2(ptRightTop.m_y - ptLeftTop.m_y, ptRightTop.m_x - ptLeftTop.m_x);

    //const float angleLeft = (float)atan2(ptLeftTop.m_y - ptLeftBottom.m_y, ptLeftTop.m_x - ptLeftBottom.m_x);
    //const float angleRight = (float)atan2(ptRightTop.m_y - ptRightBottom.m_y, ptRightTop.m_x - ptRightBottom.m_x);

    m_result_edgeAlign.SetAngle((angleLower + angleUpper) / 2);

    // 결과 최종 업데이트!!
    m_result_edgeAlign.bAvailable = TRUE;

    return 0;
}

long Inspection_Impl::ReAlignForRound_UpDown(const bool detailSetupMode, const Ipvm::Image8u& image,
    const Ipvm::EllipseEq32r& sEllipseUp, const Ipvm::EllipseEq32r& sEllipseDown) //kircheis_RoundPKG
{
    if (Config::getVisionType() != VISIONTYPE_2D_INSP)
        return 0;

    const auto& scale = m_processor.getScale();

    Ipvm::Point32s2 ptEllipseCenter;
    Ipvm::Point32s2 ptStart, ptEnd;
    const static long nDiv = 36;
    const static long nDivHalf = nDiv / 2;
    const static long n150umPXL = CAST_LONG(150.f * scale.umToPixelXY());
    long nXR, nYR, nMaxR;
    long nEdgeNum;
    float fOneStepGain;
    float fSeedGain;
    float fAngle;
    Ipvm::Point32r2 edgeXY;
    float fDist;
    float fCalcY;

    Ipvm::Point32r2 point_f(0.f, 0.f);
    Ipvm::LineEq32r line(0.f, 0.f, 0.f);

    std::vector<Ipvm::Point32r2> vecfptDist(0);
    Ipvm::Point32r2 fptEdge;
    std::vector<Ipvm::Point32r2> vecfptEdge(0);
    std::vector<Ipvm::Point32r2> vecfptEdgeAll(0);
    std::vector<Ipvm::Point32r2> vecfptFiltetingEdge(0);
    std::vector<Ipvm::Point32r2> vecfptFiltetingEdgeAll(0);
    BOOL bEllipseIsCircle;
    std::vector<Ipvm::LineSeg32r> vecsLineDebug(0);
    Ipvm::EllipseEq32r ellipseResult;
    Ipvm::EllipseEq32r sEllipseResult;
    std::vector<Ipvm::EllipseEq32r> vecsEllipse(0);
    Curve2DEq curveEq;

    //{{ Up Detail Align
    ptEllipseCenter = Ipvm::Point32s2((long)(sEllipseUp.m_x + .5f), (long)(sEllipseUp.m_y + .5f));
    fOneStepGain = (float)((sEllipseUp.m_xradius - sEllipseUp.m_yradius) / (float)nDivHalf);
    bEllipseIsCircle = (sEllipseUp.m_xradius == sEllipseUp.m_yradius);
    nXR = (long)(sEllipseUp.m_xradius + .5f);
    nYR = (long)(sEllipseUp.m_yradius + .5f);
    nMaxR = (long)max(nXR, nYR);

    for (long i = 0; i <= nDiv; i++)
    {
        fAngle = -(float)(ITP_PI * ((double)i / (double)nDiv));
        fSeedGain = (i <= nDivHalf) ? (float)i : (float)(nDiv - i);
        ptEnd.m_y = ptStart.m_y = 0;
        if (bEllipseIsCircle)
            ptStart.m_x = nXR - n150umPXL;
        else
            ptStart.m_x = nXR - (long)(fSeedGain * fOneStepGain + .5f) - n150umPXL;
        ptEnd.m_x = ptStart.m_x + n150umPXL + n150umPXL;

        point_f = Ipvm::Conversion::ToPoint32r2(ptStart);
        CPI_Geometry::RotatePoint(point_f, fAngle, Ipvm::Point32r2(0.f, 0.f));
        ptStart = Ipvm::Conversion::ToPoint32s2(point_f);
        ptStart += ptEllipseCenter;

        point_f = Ipvm::Conversion::ToPoint32r2(ptEnd);
        CPI_Geometry::RotatePoint(point_f, fAngle, Ipvm::Point32r2(0.f, 0.f));
        ptEnd = Ipvm::Conversion::ToPoint32s2(point_f);
        ptEnd += ptEllipseCenter;
        vecsLineDebug.push_back(
            Ipvm::LineSeg32r((float)ptStart.m_x, (float)ptStart.m_y, (float)ptEnd.m_x, (float)ptEnd.m_y));

        if (m_pEdgeDetect->PI_ED_CalcEdgePointFromImage(PI_ED_DIR_FALLING, ptStart, ptEnd, image, edgeXY, TRUE))
        {
            fptEdge = edgeXY;
            vecfptEdge.push_back(fptEdge);
            vecfptEdgeAll.push_back(fptEdge);

            fDist = CPI_Geometry::GetDistance_PointToPoint(sEllipseUp.m_x, sEllipseUp.m_y, edgeXY.m_x, edgeXY.m_y);
            vecfptDist.push_back(Ipvm::Point32r2((float)i, fDist));
        }
    }
    nEdgeNum = (long)vecfptEdge.size();
    if (nEdgeNum >= nDivHalf)
    {
        CPI_Geometry::Get2DCurveFitting(vecfptDist, curveEq); //spec 상의 Center와 Edge 와의 거리로 2차 피팅하고
        for (long idx = 0; idx < nEdgeNum; idx++)
        {
            fCalcY = CAST_FLOAT(((float)idx * curveEq.m_c * curveEq.m_c) + ((float)idx * curveEq.m_b) + curveEq.m_a);
            fDist = (float)fabs(fCalcY - vecfptDist[idx].m_y);
            vecfptDist[idx].m_y
                = fDist; //2차 피팅 라인과 Edge와의 오차를 구한다. 원래 이 값으로 필터링을 했으나 Center에 편심이 있을 경우 특정 구간 Edge가 다 날라가는 문제가 있었다
        }

        //위에서 구한 오차들로 1차 피팅을 하고
        Ipvm::DataFitting::FitToLine(long(vecfptDist.size()), &vecfptDist[0], line);
        for (long idx = 0; idx < nEdgeNum; idx++)
        {
            fCalcY = -(line.m_a / line.m_b) * (float)idx - (line.m_c / line.m_b);
            fDist = (float)fabs(fCalcY - vecfptDist[idx].m_y); //피팅라인과 오차와의 차이를 구해서
            if (fDist < 2.f) //튀는 오차는 노이즈 처리
            {
                vecfptFiltetingEdge.push_back(vecfptEdge[idx]);
                vecfptFiltetingEdgeAll.push_back(vecfptEdge[idx]);
            }
        }
        if (vecfptFiltetingEdge.size() < 8) //kircheis_FootTip_Round
            return -1;

        if (Ipvm::DataFitting::FitToEllipse(long(vecfptFiltetingEdge.size()), &vecfptFiltetingEdge[0], ellipseResult)
            != Ipvm::Status::e_ok)
        {
            return -1;
        }

        sEllipseResult = Ipvm::EllipseEq32r(
            ellipseResult.m_x, ellipseResult.m_y, ellipseResult.m_xradius, ellipseResult.m_yradius);
        vecsEllipse.push_back(sEllipseResult);
    }
    //}}

    //{{ Down Detail Align
    vecfptEdge.clear();
    vecfptFiltetingEdge.clear();
    vecfptDist.clear();
    ptEllipseCenter = Ipvm::Point32s2((long)(sEllipseDown.m_x + .5f), (long)(sEllipseDown.m_y + .5f));
    fOneStepGain = (float)((sEllipseDown.m_xradius - sEllipseDown.m_yradius) / (float)nDivHalf);
    bEllipseIsCircle = (sEllipseDown.m_xradius == sEllipseDown.m_yradius);
    nXR = (long)(sEllipseDown.m_xradius + .5f);
    nYR = (long)(sEllipseDown.m_yradius + .5f);
    nMaxR = (long)max(nXR, nYR);

    for (long i = 0; i <= nDiv; i++)
    {
        fAngle = (float)(ITP_PI * ((double)i / (double)nDiv));
        fSeedGain = (i <= nDivHalf) ? (float)i : (float)(nDiv - i);
        ptEnd.m_y = ptStart.m_y = 0;
        if (bEllipseIsCircle)
            ptStart.m_x = nXR - n150umPXL;
        else
            ptStart.m_x = nXR - (long)(fSeedGain * fOneStepGain + .5f) - n150umPXL;
        ptEnd.m_x = ptStart.m_x + n150umPXL + n150umPXL;

        point_f = Ipvm::Conversion::ToPoint32r2(ptStart);
        CPI_Geometry::RotatePoint(point_f, fAngle, Ipvm::Point32r2(0.f, 0.f));
        ptStart = Ipvm::Conversion::ToPoint32s2(point_f);
        ptStart += ptEllipseCenter;

        point_f = Ipvm::Conversion::ToPoint32r2(ptEnd);
        CPI_Geometry::RotatePoint(point_f, fAngle, Ipvm::Point32r2(0.f, 0.f));
        ptEnd = Ipvm::Conversion::ToPoint32s2(point_f);
        ptEnd += ptEllipseCenter;
        vecsLineDebug.push_back(
            Ipvm::LineSeg32r((float)ptStart.m_x, (float)ptStart.m_y, (float)ptEnd.m_x, (float)ptEnd.m_y));

        if (m_pEdgeDetect->PI_ED_CalcEdgePointFromImage(PI_ED_DIR_FALLING, ptStart, ptEnd, image, edgeXY, TRUE))
        {
            fptEdge = edgeXY;
            vecfptEdge.push_back(fptEdge);
            vecfptEdgeAll.push_back(fptEdge);
            fDist = CPI_Geometry::GetDistance_PointToPoint(sEllipseDown.m_x, sEllipseDown.m_y, edgeXY.m_x, edgeXY.m_y);
            vecfptDist.push_back(Ipvm::Point32r2((float)i, fDist));
        }
    }
    nEdgeNum = (long)vecfptEdge.size();
    if (nEdgeNum >= nDivHalf)
    {
        CPI_Geometry::Get2DCurveFitting(vecfptDist, curveEq);
        for (long idx = 0; idx < nEdgeNum; idx++)
        {
            fCalcY = CAST_FLOAT(((float)idx * curveEq.m_c * curveEq.m_c) + ((float)idx * curveEq.m_b) + curveEq.m_a);
            fDist = (float)fabs(fCalcY - vecfptDist[idx].m_y);
            vecfptDist[idx].m_y
                = fDist; //2차 피팅 라인과 Edge와의 오차를 구한다. 원래 이 값으로 필터링을 했으나 Center에 편심이 있을 경우 특정 구간 Edge가 다 날라가는 문제가 있었다
        }

        Ipvm::DataFitting::FitToLine(
            long(vecfptDist.size()), &vecfptDist[0], line); //위에서 구한 오차들로 1차 피팅을 하고
        for (long idx = 0; idx < nEdgeNum; idx++)
        {
            fCalcY = -(line.m_a / line.m_b) * (float)idx - (line.m_c / line.m_b);
            fDist = (float)fabs(fCalcY - vecfptDist[idx].m_y); //피팅라인과 오차와의 차이를 구해서
            if (fDist < 2.f) //튀는 오차는 노이즈 처리
            {
                vecfptFiltetingEdge.push_back(vecfptEdge[idx]);
                vecfptFiltetingEdgeAll.push_back(vecfptEdge[idx]);
            }
        }
        if (vecfptFiltetingEdge.size() < 8) //kircheis_FootTip_Round
            return -1;

        if (Ipvm::DataFitting::FitToEllipse(long(vecfptFiltetingEdge.size()), &vecfptFiltetingEdge[0], ellipseResult)
            != Ipvm::Status::e_ok)
        {
            return -1;
        }

        sEllipseResult = Ipvm::EllipseEq32r(
            ellipseResult.m_x, ellipseResult.m_y, ellipseResult.m_xradius, ellipseResult.m_yradius);
        vecsEllipse.push_back(sEllipseResult);
    }
    //}}

    m_processor.SetDebugInfoItem(detailSetupMode, _T("Round Edge Research Range"), vecsLineDebug, TRUE);
    m_processor.SetDebugInfoItem(detailSetupMode, _T("Round Edge Research Result"), vecfptEdgeAll, TRUE);
    m_processor.SetDebugInfoItem(detailSetupMode, _T("Round Edge Research Result Final"), vecfptFiltetingEdgeAll, TRUE);
    m_processor.SetDebugInfoItem(detailSetupMode, _T("Round Research Ellipse"), vecsEllipse, TRUE);

    return 0;
}

long Inspection_Impl::RoughAlign_LineAlign(const Para& para, long nDirection,
    std::vector<Ipvm::Point32r2> vecfpNewPoints, std::vector<Ipvm::Point32r2> vecfpFirstEdge,
    std::vector<Ipvm::Point32r2> vecfpBestEdge)
{
    if ((long)(vecfpNewPoints.size()) <= 5)
        return -1;

    const std::vector<Ipvm::Point32r2>* useVectorXY = nullptr;

    switch (para.m_nEdgeDetectMode)
    {
        case EdgeDetectMode_FirstEdge:
            useVectorXY = &vecfpFirstEdge;
            break;
        case EdgeDetectMode_BestEdge:
            useVectorXY = &vecfpBestEdge;
            break;
        case EdgeDetectMode_AutoDetect:
            useVectorXY = &vecfpNewPoints;
            break;
    }

    if (useVectorXY == nullptr)
    {
        return -1;
    }

    long nPointSize = (long)(useVectorXY->size());

    if (nPointSize <= 0)
        return -1;

    std::vector<Ipvm::Point32r2> vecfFilteredXY;

    Ipvm::LineEq32r line;

    const auto& scale = m_processor.getScale();

    //float fLimitDistance = 1.f;//일반적인 2D/3D에서는 Unit의 각 Edge는 Sawing 단면이라 직선이 아니면 문제가 있는 거니까 타이트하게 보는게 맞는데, Pixel 값으로 놓는게 맞을까?
    static const bool bIs3DVision = (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_3D_INSP);
    static const float fFilteringDistanceUm
        = bIs3DVision ? 15.f : 10.f; //3D면 Edge가 좀 지저분하니까 약간 더 여유를 주자
    float fLimitDistance = (nDirection == enum_Left || nDirection == enum_Right)
        ? scale.convert_umToPixelX(fFilteringDistanceUm)
        : scale.convert_umToPixelY(fFilteringDistanceUm); //일단은 이렇게 써보자

    //{{//Side Vision의 경우 Edge는 잘 추출되나 Filtering Distance가 너무 타이트하면 Warpage때문에 제대로 찾은 Edge까지 걸러진다.
    // 이에 Side Vision의 Top / Bottom 일 경우 Filtering Distance를 광학계 기준 30um까지는 봐준다.
    static const bool bIsSideVision = (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_SIDE_INSP);
    const float f30umToPixelY = scale.convert_umToPixelY(30.f);

    if (bIsSideVision && (nDirection == enum_Top || nDirection == enum_Bottom))
        fLimitDistance = f30umToPixelY;
    //}}

    if (!CPI_Geometry::RoughLineFitting(nPointSize, &(*useVectorXY)[0], line, 5, fLimitDistance, vecfFilteredXY))
    {
        return -1;
    }

    nPointSize = (long)(vecfFilteredXY.size());

    if (nPointSize <= 0)
        return -1;

    for (long i = 0; i < nPointSize; i++)
    {
        if (nDirection == enum_Left)
            m_result_edgeAlign.vecLeftEdgeAlignPoints.push_back(vecfFilteredXY[i]);
        else if (nDirection == enum_Top)
            m_result_edgeAlign.vecTopEdgeAlignPoints.push_back(vecfFilteredXY[i]);
        else if (nDirection == enum_Right)
            m_result_edgeAlign.vecRightEdgeAlignPoints.push_back(vecfFilteredXY[i]);
        else if (nDirection == enum_Bottom)
            m_result_edgeAlign.vecBottomEdgeAlignPoints.push_back(vecfFilteredXY[i]);
        else if (nDirection == enum_LowTop)
            m_result_edgeAlign.vecLowTopEdgeAlignPoints.push_back(vecfFilteredXY[i]);
    }

    return 0;
}

BOOL Inspection_Impl::CheckEmptyPocket3D(
    const Para& para, const Ipvm::Image8u& i_image, float& o_fCurPackageHeight, BOOL& o_bIsEmptyPocket)
{
    BOOL bResult = TRUE;
    o_bIsEmptyPocket = FALSE;

    const auto& scale = m_processor.getScale();
    Ipvm::Point32r2 fptPkgModeSizePXL = Ipvm::Point32r2(0.f, 0.f);
    fptPkgModeSizePXL.m_x = scale.convert_umToPixelX(m_packageSpec.m_bodyInfoMaster->fBodySizeX);
    fptPkgModeSizePXL.m_y = scale.convert_umToPixelY(m_packageSpec.m_bodyInfoMaster->fBodySizeY);
    float fPackageHeightGV = m_packageSpec.m_bodyInfoMaster->fBodyThickness
        / scale.pixelToUmZ(); //GV로 환산된 0 지점에서의 Spec상의 Package 높이//kircheis_3DRange

    Ipvm::Point32s2 ptPaneCenter = Ipvm::Rect32s(i_image).CenterPoint();

    bResult = EmptyCheckUsePaneCenter(para, i_image, ptPaneCenter, fptPkgModeSizePXL.m_x, fptPkgModeSizePXL.m_y,
        fPackageHeightGV, o_fCurPackageHeight, o_bIsEmptyPocket);

    return bResult;
}

BOOL Inspection_Impl::EmptyCheckUsePaneCenter(const Para& para, const Ipvm::Image8u& i_image,
    Ipvm::Point32s2 i_ptPaneCenter, float i_fPkgMoldSizeX_PXL, float i_fPkgMoldSizeY_PXL, float i_fPkgHeightGV,
    float& o_fCenterHeightGV, BOOL& o_bIsEmpty)
{
    o_fCenterHeightGV = 0.f;
    o_bIsEmpty = FALSE;
    if ((i_image.GetSizeX() * i_image.GetSizeY()) == 0)
        return FALSE;

    long nEmptyRoiWidth = (long)(i_fPkgMoldSizeX_PXL * .1f); //Package Size X의 10%
    long nEmptyRoiHeight = (long)(i_fPkgMoldSizeY_PXL * .1f); //Package Size Y의 10%

    Ipvm::Rect32s rtPaneEmptyROI(int32_t(i_ptPaneCenter.m_x - nEmptyRoiWidth),
        int32_t(i_ptPaneCenter.m_y - nEmptyRoiHeight), int32_t(i_ptPaneCenter.m_x + nEmptyRoiWidth),
        int32_t(i_ptPaneCenter.m_y + nEmptyRoiHeight));
    Ipvm::Rect32s rtPaneLeft = rtPaneEmptyROI; //좌우 양분 ROI 설정
    rtPaneLeft.m_right = i_ptPaneCenter.m_x;
    Ipvm::Rect32s rtPaneRight = rtPaneEmptyROI;
    rtPaneRight.m_left = i_ptPaneCenter.m_x;

    //{{ //kircheis_3DEmptyOverKill
    //Body Size의 10% Size에 불과해서 발생하는 과검이 있어 Edge Detect ROI로 Empty Check ROI를 만든다
    Ipvm::Rect32s rtBodySearchInnerROI;
    Ipvm::Rect32s rtTeachedSearchROI;

    const auto& scale = m_processor.getScale();
    float bodySizeX_px; // = scale.convert_umToPixelX(m_packageSpec.m_bodyInfoMaster->GetBodySizeX());
    float bodySizeY_px; // = scale.convert_umToPixelY(m_packageSpec.m_bodyInfoMaster->GetBodySizeY());
    m_processor.GetPackageSize(true, bodySizeX_px, bodySizeY_px);

    long nPos = 0;
    for (auto& searchPara : para.m_searchPara[LEFT])
    {
        rtTeachedSearchROI
            = searchPara.ToRect(scale, LEFT, Ipvm::Conversion::ToPoint32r2(i_ptPaneCenter), bodySizeX_px, bodySizeY_px);
        nPos = (long)max(nPos, rtTeachedSearchROI.m_right);
    }
    rtBodySearchInnerROI.m_left = nPos;

    nPos = 0;
    for (auto& searchPara : para.m_searchPara[UP])
    {
        rtTeachedSearchROI
            = searchPara.ToRect(scale, UP, Ipvm::Conversion::ToPoint32r2(i_ptPaneCenter), bodySizeX_px, bodySizeY_px);
        nPos = (long)max(nPos, rtTeachedSearchROI.m_top);
    }
    rtBodySearchInnerROI.m_top = nPos;

    nPos = i_image.GetSizeX();

    for (auto& searchPara : para.m_searchPara[RIGHT])
    {
        rtTeachedSearchROI = searchPara.ToRect(
            scale, RIGHT, Ipvm::Conversion::ToPoint32r2(i_ptPaneCenter), bodySizeX_px, bodySizeY_px);
        nPos = (long)min(nPos, rtTeachedSearchROI.m_right);
    }
    rtBodySearchInnerROI.m_right = nPos;

    nPos = i_image.GetSizeY();

    for (auto& searchPara : para.m_searchPara[DOWN])
    {
        rtTeachedSearchROI
            = searchPara.ToRect(scale, DOWN, Ipvm::Conversion::ToPoint32r2(i_ptPaneCenter), bodySizeX_px, bodySizeY_px);
        nPos = (long)min(nPos, rtTeachedSearchROI.m_bottom);
    }
    rtBodySearchInnerROI.m_bottom = nPos;

    rtBodySearchInnerROI.NormalizeRect();
    if (rtBodySearchInnerROI.Width() < nEmptyRoiWidth)
        rtBodySearchInnerROI.InflateRect(nEmptyRoiWidth, 0);
    if (rtBodySearchInnerROI.Height() < nEmptyRoiHeight)
        rtBodySearchInnerROI.InflateRect(0, nEmptyRoiHeight);
    //}}

    long nValidDataNum = 0;
    float fValidDataRatio = 0.f;
    float fMean_PackHeight = 0.f;
    o_fCenterHeightGV = CippModules::GetMeanNotUseZero_BYTE(
        i_image, rtBodySearchInnerROI /*rtPaneEmptyROI*/, nValidDataNum, fValidDataRatio); //kircheis_3DEmptyOverKill

    fMean_PackHeight = (m_packageSpec.nDeviceType == enDeviceType::PACKAGE_GULLWING)
        ? o_fCenterHeightGV - i_fPkgHeightGV
        : o_fCenterHeightGV;
    // Lead Type PAckage 일때는 몰드 높이가 Package 두께보다 높아야한다.
    // Lead Type이 아닐때에는 이러면 안된다.

    if (fValidDataRatio < 0.01f
        || fMean_PackHeight
            < -2.f) //Data가 극소수(확인 면적의 1%이하의 Data 수)이거나 높이가 Spec상의 Height보다 낮으면 Empty
    {
        o_bIsEmpty = TRUE;
        return TRUE;
    }

    //트레이 마지막 영역의 경우 Pane Center에 해당하는 부분에 트레이의 일부가 걸릴 수도 있다
    // 이때문에 Pane 영역 내부를 좌우로 양분하여 유효한 Data의 좌우 비율을 확인하도록 한다.
    //그런데 이구간은 실제 검사에서 굳이 필요 없는 부분이다. 일단 하지 말아 부러라
    BOOL bUseTrayEndColEmptyCheck = FALSE;
    if (bUseTrayEndColEmptyCheck)
    {
        float fMeanLeft = 0.f;
        float fMeanRight = 0.f;
        float fMeanDiffRatio = 0.f;
        long nValidDataNumLeft = 0;
        long nValidDataNumRight = 0;
        float fValidDataRatioLeft = 0.f;
        float fValidDataRatioRight = 0.f;
        float fValidDataDiffRatio = 0.f;

        fMeanLeft = CippModules::GetMeanNotUseZero_BYTE(i_image, rtPaneLeft, nValidDataNumLeft, fValidDataRatioLeft);
        fMeanRight
            = CippModules::GetMeanNotUseZero_BYTE(i_image, rtPaneRight, nValidDataNumRight, fValidDataRatioRight);
        fMeanDiffRatio = (float)min(fMeanLeft, fMeanRight) / (float)max(fMeanLeft, fMeanRight);
        fValidDataDiffRatio = (float)min(fValidDataRatioLeft, fValidDataRatioRight)
            / (float)max(fValidDataRatioLeft, fValidDataRatioRight);

        if (fMeanDiffRatio < 0.5f
            || fValidDataDiffRatio
                < 0.015f) //확인 영역을 좌우로 양분하여 높이차이가 두배이거나 유효 Data 수가 (70%->85%) 이상 차이나면 Empty//kircheis_3DEmptyBug//kircheis_3DEmpty 조건 완화 0.15->0.015
            o_bIsEmpty = TRUE;
    }

    return TRUE;
}
BOOL Inspection_Impl::EmptyDoubleCheckUseTrayWallForLeadless(const Para& para, const bool detailSetupMode,
    const Ipvm::Image8u& i_image, BOOL& o_bIsEmpty, BOOL& o_bIsDouble) //kircheis_20160713
{
    const auto& pixel2umZ = m_processor.getScale().pixelToUmZ();
    float fHeightOfTrayWallFromMoldGV = para.m_3D_heightOfTrayWallFromMold / pixel2umZ; //kircheis_3DRange
    float fPackageHeightGV = m_packageSpec.m_bodyInfoMaster->fBodyThickness
        / pixel2umZ; //GV로 환산된 0 지점에서의 Spec상의 Package 높이//kircheis_3DRange
    float fBodySizeX = m_packageSpec.m_bodyInfoMaster->fBodySizeX;
    float fBodySizeY = m_packageSpec.m_bodyInfoMaster->fBodySizeY;
    float fPocketPitchX_um = m_processor.getTrayScanSpec().m_pocketPitchX_mm * 1000.f;
    float fPocketPitchY_um = m_processor.getTrayScanSpec().m_pocketPitchY_mm * 1000.f;
    float fMoldHeight = 0.f;
    float fTrayWallHeight = 0.f;
    float fDiff = 0.f;
    BOOL bUseHalfRoiX = (fPocketPitchX_um > (fBodySizeX * 2.f));
    BOOL bUseHalfRoiY = (fPocketPitchY_um > (fBodySizeY * 2.f));
    BOOL bRes = FALSE;
    Ipvm::Rect32s rtPackage, rtTrayWall;
    Ipvm::Rect32s rtImage(i_image);

    bRes = CalcMoldHeight(para, detailSetupMode, i_image, fMoldHeight, o_bIsEmpty, o_bIsDouble);
    if (para.m_emptyInsp && bRes && o_bIsEmpty)
    {
        m_processor.m_bEmpty = TRUE;
        return TRUE;
    }
    if (para.m_3D_doubleInsp && bRes && o_bIsDouble)
    {
        m_processor.m_bDoubleDevice = TRUE;
        return TRUE;
    }

    rtPackage = Ipvm::Rect32s((int32_t)(min(m_result_edgeAlign.fptLT.m_x, m_result_edgeAlign.fptLB.m_x) + .5f),
        (int32_t)(min(m_result_edgeAlign.fptLT.m_y, m_result_edgeAlign.fptRT.m_y) + .5f),
        (int32_t)(max(m_result_edgeAlign.fptRT.m_x, m_result_edgeAlign.fptRB.m_x) + .5f),
        (int32_t)(max(m_result_edgeAlign.fptLB.m_y, m_result_edgeAlign.fptRB.m_y) + .5f));

    rtTrayWall = rtImage;
    if (bUseHalfRoiX)
    {
        rtTrayWall.m_left = (long)((float)(rtImage.m_left + rtPackage.m_left) * .5f + .5f);
        rtTrayWall.m_right = (long)((float)(rtImage.m_right + rtPackage.m_right) * .5f + .5f);
    }
    if (bUseHalfRoiY)
    {
        rtTrayWall.m_top = (long)((float)(rtImage.m_top + rtPackage.m_top) * .5f + .5f);
        rtTrayWall.m_bottom = (long)((float)(rtImage.m_bottom + rtPackage.m_bottom) * .5f + .5f);
    }

    rtPackage.InflateRect(3, 3);

    bRes = CalcTrayWallHeight(para, i_image, rtTrayWall, rtPackage, fMoldHeight, fTrayWallHeight);
    if (!bRes)
    {
        if (bUseHalfRoiX || bUseHalfRoiY)
            bRes = CalcTrayWallHeight(para, i_image, rtImage, rtPackage, fMoldHeight, fTrayWallHeight);
    }
    if (bRes)
    {
        fDiff = fTrayWallHeight - fMoldHeight;
        static const float fScaleGrayToReal = pixel2umZ; //kircheis_3DRange
        m_fHeightMoldToWall = fDiff * fScaleGrayToReal;
        if (para.m_3D_heightOfTrayWallFromMold == 0.f)
        {
            if (fDiff > fPackageHeightGV && para.m_emptyInsp)
                o_bIsEmpty = TRUE;
            else if (fDiff < 0 && para.m_3D_doubleInsp)
            {
                o_bIsDouble = TRUE;
            }
        }
        else
        {
            float fCutOffGain = 0.5f;
            if (fDiff > (fHeightOfTrayWallFromMoldGV + (fPackageHeightGV * fCutOffGain)) && para.m_emptyInsp)
                o_bIsEmpty = TRUE;
            else if (fDiff < (fHeightOfTrayWallFromMoldGV - (fPackageHeightGV * fCutOffGain)) && para.m_3D_doubleInsp)
            {
                o_bIsDouble = TRUE;
            }
        }
    }

    return TRUE;
}

BOOL Inspection_Impl::CalcMoldHeight(const Para& para, const bool detailSetupMode, const Ipvm::Image8u& i_image,
    float& o_fMoldHeight, BOOL& o_bIsEmpty, BOOL& o_bIsDouble) //kircheis_20160713
{
    o_bIsEmpty = o_bIsDouble = FALSE;
    o_fMoldHeight = -1.f;
    Ipvm::Rect32s rtImage(i_image);

    const BOOL bInspEmpty = para.m_emptyInsp;
    const BOOL bInspDouble = para.m_3D_doubleInsp;

    const BOOL bUSD = (m_packageSpec.nPackageType == enPackageType::CARD);

    const auto& scale = m_processor.getScale();
    const auto& pixel2umZ = scale.pixelToUmZ();
    Ipvm::Point32r2 fptPkgModeSizePXL = Ipvm::Point32r2(0.f, 0.f); //Package Pixel Size
    fptPkgModeSizePXL.m_x = scale.convert_umToPixelX(m_packageSpec.m_bodyInfoMaster->fBodySizeX);
    fptPkgModeSizePXL.m_y = scale.convert_umToPixelY(m_packageSpec.m_bodyInfoMaster->fBodySizeY);
    float fMoldThicknessGV = m_packageSpec.m_bodyInfoMaster->fBodyThickness
        / pixel2umZ; //GV로 환산된 0 지점에서의 Spec상의 Package 높이//kircheis_3DRange
    float fBallHeightGV = 0.f; //Ball Height의 GV
    if (m_packageSpec.nDeviceType == enDeviceType::PACKAGE_BALL)
    {
        if (m_packageSpec.m_originalballMap->m_ballTypes.size())
        {
            fBallHeightGV = CAST_FLOAT(
                m_packageSpec.m_originalballMap->m_ballTypes.front().m_height_um / pixel2umZ); //kircheis_3DRange
        }
    }

    float fPackageHeightGV = fMoldThicknessGV + fBallHeightGV; //Ball Height를 포함하는 Package Height GV
    float fTrayWallFromMoldGV = para.m_3D_heightOfTrayWallFromMold
        / pixel2umZ; //정상 Package Mold에서 Tray Wall까지의 높이 GV//kircheis_3DRange
    float fDoubleModFromTrayWallGV
        = fPackageHeightGV - fTrayWallFromMoldGV; //정합 Double일 경우 Tray Wall에서 Double Mold까지의 높이 GV
    static float fLowCutHeight = 50.f; //얘 이하의 높이는 쓰레기 취급
    static const long n100umGV = (long)((100.f / pixel2umZ) + .5f); //kircheis_3DRange
    static const long n50umGV = (long)((float)n100umGV * .5f + .5f);

    Ipvm::Image32s histogram;
    long nPeakNum = 256;
    long* pnPeakID = new long[nPeakNum];
    long* pnPeakCount = new long[nPeakNum];
    long nCountThresh = (long)((float)rtImage.Height() * 0.8f + .5f);

    CippModules::GetHistogramPeaks(i_image, rtImage, histogram, pnPeakID, pnPeakCount, nPeakNum, nCountThresh, 20, 230);

    if (nPeakNum == 0) //Peak가 없으면
    { //깔끔히 포기
        if (bInspEmpty)
            o_bIsEmpty = TRUE;

        delete[] pnPeakID;
        delete[] pnPeakCount;

        return FALSE;
    }

    float fEmptyCheckMoldAreaRatioSpec = para.m_3D_emptyCheckMoldAreaRatioSpec; //kircheis_580 Chip 3D Empty

    long nPackageArea = (long)(fptPkgModeSizePXL.m_x * fptPkgModeSizePXL.m_y + .5f); //Package의 Pixel 면적
    float fPackageAreaRatio = 0.f;
    float fPackageAreaRatioMax = -100.f; //kircheis_3DEmp_Spec
    float fCurPeakHeight = 0.f;
    static float fNoiseCutAreaRatio = 0.00f; // 0%;
    long nMoldID = -1;
    long nMoldPeakID = -1;
    long nMoldHistoCnt = -1;
    long nMaxAreaID = -1;
    std::vector<long> vecnPeakHistogram(0);
    std::vector<float> vecfPeakHistoHeight(0);
    std::vector<float> vecfPeakAreaRatio(0);

    const auto* histogramValues = histogram.GetMem();
    for (long nID = 0; nID < nPeakNum; nID++)
    {
        long nMoldHisto = 0;
        for (long i = -n100umGV; i <= n100umGV; i++) //Peak 기준 상하 +-100um 이내의 Pixel수를 카운트(면적)
            nMoldHisto += histogramValues[pnPeakID[nID] + i];

        vecnPeakHistogram.push_back(nMoldHisto);
        fCurPeakHeight = CippModules::GetHistogramMassCenter(
            histogram, pnPeakID[nID], n50umGV); //Peak기준 상하 +-50um기준으로 무게 중심을 이용해 높이를 재산출
        vecfPeakHistoHeight.push_back(fCurPeakHeight);
        fPackageAreaRatio = (float)nMoldHisto / (float)nPackageArea; //해당 Peak의 면적과 Package면적의 비율 계산
        vecfPeakAreaRatio.push_back(fPackageAreaRatio);
        fPackageAreaRatioMax = (float)max(fPackageAreaRatioMax, fPackageAreaRatio); //kircheis_3DEmp_Spec
        if (nMoldHistoCnt < nMoldHisto) //면적이 가장 크면서, 면적 비율이 정해진 기준이상 이면 실제 Package위치로 인정
        {
            nMaxAreaID = nID;
            nMoldHistoCnt = nMoldHisto;
            if (fPackageAreaRatio > fEmptyCheckMoldAreaRatioSpec) //kircheis_580 Chip 3D Empty
            {
                nMoldPeakID = nID;
                nMoldID = pnPeakID[nID];
                o_fMoldHeight = fCurPeakHeight;
            }
        }
    }

    //{{//kircheis_3DEmp_Spec
    std::vector<float> vecfPackageAreaRatioMax(1);
    vecfPackageAreaRatioMax[0] = fPackageAreaRatioMax * 100.f;
    m_processor.SetDebugInfoItem(detailSetupMode, _T("Current Empty Check Value"), vecfPackageAreaRatioMax);
    //}}

    delete[] pnPeakID;
    delete[] pnPeakCount;

    //{{각 Peak의 높이 분석을 수행한다
    float fHeightGap = 0.f;
    float fJudgeGap = 0.f;
    //Peak 1~2개는 바로 판단 하고
    if (nPeakNum == 1 && nMoldID > 0) //Peak가 하나이고 실 Package로 예상되는 부분이 있으면 이후 로 진행
        return TRUE;
    else if (nPeakNum == 2 && nMoldID == 0) //Peak가 둘이고 실 Package로 예상되는 부분이 첫 Peak이면 이후로 진행
        return TRUE;
    else if (nPeakNum == 2 && nMoldID == 1) //Peak가 둘이고 실 Package로 예상되는 부분이 두번째 Peak이면 Double 판정
    {
        if (bInspDouble)
        {
            fHeightGap = vecfPeakHistoHeight[1] - vecfPeakHistoHeight[0];
            fJudgeGap = (float)fabs(fDoubleModFromTrayWallGV - fHeightGap);
            o_bIsDouble = (fJudgeGap < (float)n100umGV);
        }
        return TRUE;
    }
    else if (bInspDouble && !bUSD
        && FALSE) //Peak 3개 이상 부터는 알고리즘 돌리자 uSD는 노이즈가 심해 높이 비교 알고리즘 돌리면 난리난다. //일단 이구간은 쓰지 말아 보자.. 노이즈에 넘 민감
    {
        for (long nPeak = nPeakNum - 1; nPeak > 0; nPeak--)
        {
            if (vecfPeakHistoHeight[nPeak - 1] < fLowCutHeight) // 현재 확인할 Peak가 기준 높이 이하면 중단
                break;
            if (vecfPeakAreaRatio[nPeak] < fNoiseCutAreaRatio)
                continue;

            float fCalcMoldToWallPosZ_A = vecfPeakHistoHeight[nPeak] - fTrayWallFromMoldGV;
            float fCalcDoubleMoldPosZ_B = vecfPeakHistoHeight[nPeak] - fPackageHeightGV;
            float fCalcDoubleMoldPosZ_IgBall_C = (m_packageSpec.nDeviceType == enDeviceType::PACKAGE_BALL)
                ? vecfPeakHistoHeight[nPeak] - fMoldThicknessGV
                : 0.f;
            float fCalcWallToDoubleMoldPosZ_D = vecfPeakHistoHeight[nPeak] - fDoubleModFromTrayWallGV;

            float fGapA, fGapB, fGapC, fGapD, fGapMin;
            BOOL bInA = FALSE, bInB = FALSE, bInC = FALSE, bInD = FALSE;

            for (long nCheck = nPeak - 1; nCheck >= 0; nCheck--)
            {
                if (vecfPeakHistoHeight[nCheck] < fLowCutHeight) // 현재 확인할 Peak가 기준 높이 이하면 중단
                    break;
                if (vecfPeakAreaRatio[nCheck] < fNoiseCutAreaRatio)
                    continue;
                fGapA = (float)(fabs(vecfPeakHistoHeight[nCheck] - fCalcMoldToWallPosZ_A));
                fGapB = (float)(fabs(vecfPeakHistoHeight[nCheck] - fCalcDoubleMoldPosZ_B));
                fGapC = (m_packageSpec.nDeviceType == enDeviceType::PACKAGE_BALL)
                    ? (float)(fabs(vecfPeakHistoHeight[nCheck] - fCalcDoubleMoldPosZ_IgBall_C))
                    : 1000000;
                fGapD = (float)(fabs(vecfPeakHistoHeight[nCheck] - fCalcWallToDoubleMoldPosZ_D));

                bInA = ((long)(fGapA + .5f) <= n50umGV);
                bInB = ((long)(fGapB + .5f) <= n50umGV);
                bInC = ((long)(fGapC + .5f) <= n50umGV);
                bInD
                    = FALSE; //((long)(fGapD + .5f) <= n50umGV) && (vecfPeakAreaRatio[nPeak] > vecfPeakAreaRatio[nCheck]);

                if (!bInA && !bInB && !bInC && !bInD)
                    continue;
                if (bInA && !bInB && !bInC && !bInD)
                    continue;

                if (!bInA && (bInB || bInC || bInD))
                {
                    o_bIsDouble = TRUE;
                    return TRUE;
                }

                fGapMin = (float)min(min(fGapA, fGapB), min(fGapC, fGapD));
                if (fGapMin < fGapA && nCheck != nMoldPeakID)
                {
                    o_bIsDouble = TRUE;
                    return TRUE;
                }
            }
        }
    }

    //{{ // kircheis_3DEmpty2
    if (m_packageSpec.m_deadBug
        && (m_packageSpec.nDeviceType == enDeviceType::PACKAGE_BALL
            || m_packageSpec.nDeviceType == enDeviceType::PACKAGE_LEADLESS
            || m_packageSpec.nDeviceType == enDeviceType::PACKAGE_LAND))
    {
        if (nMoldID < 0 && bInspEmpty)
        {
            o_bIsEmpty = TRUE;
            return TRUE;
        }
    }
    //}}
    if (nMoldID > 0 && bInspDouble)
    {
        float fAreaRatio;
        BOOL bNotNoise = FALSE;
        for (long nID = nMoldPeakID + 1; nID < nPeakNum; nID++)
        {
            fAreaRatio = (float)vecnPeakHistogram[nID] / (float)vecnPeakHistogram[nMoldPeakID];
            if (fAreaRatio > 0.3f)
            {
                if ((vecfPeakHistoHeight[nID] - vecfPeakHistoHeight[nMoldPeakID]) > fPackageHeightGV)
                {
                    o_bIsDouble = TRUE;
                    return TRUE;
                }
            }

            if (vecfPeakAreaRatio[nID] > fNoiseCutAreaRatio)
                bNotNoise = TRUE;
        }
    }
    else if (nMoldID < 0 && bInspDouble)
    {
        float fAreaRatio(0.f);
        float fFirstHeight(0.f);
        long nOver30PerCnt = 0;
        for (long nID = 0; nID < nPeakNum; nID++)
        {
            fAreaRatio = (float)vecnPeakHistogram[nID] / (float)nPackageArea;
            ;
            if (fAreaRatio > 0.20f)
            {
                if (nOver30PerCnt == 0)
                {
                    fFirstHeight = vecfPeakHistoHeight[nID];
                    nOver30PerCnt++;
                }
                else if ((vecfPeakHistoHeight[nID] - fFirstHeight) > fPackageHeightGV)
                    nOver30PerCnt++;
            }
        }
        if (nOver30PerCnt > 1)
        {
            o_bIsDouble = TRUE;
            return TRUE;
        }
    }
    //}}

    return TRUE;
}

BOOL Inspection_Impl::CalcTrayWallHeight(const Para& para, const Ipvm::Image8u& i_image,
    Ipvm::Rect32s i_rtTrayWallSearchArea, Ipvm::Rect32s i_rtMold, float i_fMoldHeight, float& o_fTrayWallHeight)
{
    o_fTrayWallHeight = -1.f;
    Ipvm::Rect32s rtPane = Ipvm::Rect32s(i_image);

    const auto& scale = m_processor.getScale();
    const auto& pixel2umZ = scale.pixelToUmZ();

    long nPeakNumNoMold = 256;
    long* pnPeakIDNoMold = new long[nPeakNumNoMold];
    long* pnPeakCountNoMold = new long[nPeakNumNoMold];
    long nCountThresh = i_rtMold.Height() / 3;
    float fMoldThicknessGV = m_packageSpec.m_bodyInfoMaster->fBodyThickness
        / pixel2umZ; //GV로 환산된 0 지점에서의 Spec상의 Package 높이//kircheis_3DRange
    float fBallHeightGV = 0.f; //Ball Height의 GV
    if ((m_packageSpec.nDeviceType == enDeviceType::PACKAGE_BALL)
        && (m_packageSpec.m_originalballMap->m_balls.size()
            > 0)) // SDY Device Type이 Ball로 되어있어도 Ball이 존재하지 않으면 Ball 높이는 추가하지 않는다.
        fBallHeightGV = CAST_FLOAT(
            m_packageSpec.m_originalballMap->m_ballTypes.front().m_height_um / pixel2umZ); //kircheis_3DRange

    float fPackageHeightGV = fMoldThicknessGV + fBallHeightGV; //Ball Height를 포함하는 Package Height GV
    long nLowThresh = (long)((i_fMoldHeight - fPackageHeightGV) * 1.1f + .5f);
    nLowThresh = (long)max(30, nLowThresh);
    long nHighThresh = (long)((i_fMoldHeight + fPackageHeightGV) * 1.4f + .5f);

    auto& memory = m_processor.getReusableMemory();

    Ipvm::Image8u imageTemp;
    if (!memory.GetByteImage(imageTemp, i_image.GetSizeX(), i_image.GetSizeY()))
    {
        delete[] pnPeakIDNoMold;
        delete[] pnPeakCountNoMold;

        return FALSE;
    }

    Ipvm::ImageProcessing::Copy(i_image, i_rtTrayWallSearchArea, imageTemp);
    Ipvm::ImageProcessing::Fill(i_rtMold, 0, imageTemp);

    Ipvm::Image32s histogram;
    CippModules::GetHistogramPeaks(imageTemp, i_rtTrayWallSearchArea, histogram, pnPeakIDNoMold, pnPeakCountNoMold,
        nPeakNumNoMold, nCountThresh, nLowThresh, nHighThresh);

    static const long n100umGV = (long)((100.f / pixel2umZ) + .5f); //kircheis_3DRange
    static const long n50umGV = (long)((float)n100umGV * .5f + .5f);
    float fTrayWallFromMoldGV = para.m_3D_heightOfTrayWallFromMold
        / pixel2umZ; //정상 Package Mold에서 Tray Wall까지의 높이 GV//kircheis_3DRange
    Ipvm::Point32r2 fptPkgModeSizePXL = Ipvm::Point32r2(0.f, 0.f); //Package Pixel Size
    fptPkgModeSizePXL.m_x = scale.convert_umToPixelX(m_packageSpec.m_bodyInfoMaster->fBodySizeX);
    fptPkgModeSizePXL.m_y = scale.convert_umToPixelY(m_packageSpec.m_bodyInfoMaster->fBodySizeY);
    long nPackageArea = (long)(fptPkgModeSizePXL.m_x * fptPkgModeSizePXL.m_y + .5f); //Package의 Pixel 면적

    long nTrayWallID = -1;
    float fExpectedWallHeight = i_fMoldHeight + fTrayWallFromMoldGV;
    float fGap;
    float fCurHeight;

    const auto* histogramValues = histogram.GetMem();

    if (nPeakNumNoMold == 1)
    {
        nTrayWallID = pnPeakIDNoMold[0];
    }
    else if (nPeakNumNoMold > 0)
    {
        for (long nID = 0; nID < nPeakNumNoMold; nID++)
        {
            fCurHeight = CippModules::GetHistogramMassCenter(histogram, pnPeakIDNoMold[nID], n50umGV);
            fGap = (float)fabs(fCurHeight - fExpectedWallHeight);
            if (fGap < (float)n50umGV)
            {
                nTrayWallID = pnPeakIDNoMold[nID];
                break;
            }
        }
        if (nTrayWallID < 0)
        {
            for (long nID = nPeakNumNoMold - 1; nID > 0; nID--)
            {
                fCurHeight = CippModules::GetHistogramMassCenter(histogram, pnPeakIDNoMold[nID], n50umGV);

                long nMoldHisto = 0;
                for (long i = -n100umGV; i <= n100umGV; i++) //Peak 기준 상하 +-100um 이내의 Pixel수를 카운트(면적)
                    nMoldHisto += histogramValues[pnPeakIDNoMold[nID] + i];

                float fPackageAreaRatio
                    = (float)nMoldHisto / (float)nPackageArea; //해당 Peak의 면적과 Package면적의 비율 계산

                if (fPackageAreaRatio > 0.02f || nMoldHisto > 500) //kircheis_3D_Double
                {
                    nTrayWallID = pnPeakIDNoMold[nID];
                    break;
                }
            }
        }

        if (m_teachModeForHeightMoldToWall && nTrayWallID < 0)
        {
            nTrayWallID = pnPeakIDNoMold[nPeakNumNoMold - 1];
        }
    }

    BOOL bReturn = FALSE;
    if (nTrayWallID > 0 && para.m_3D_doubleInsp)
    {
        o_fTrayWallHeight = CippModules::GetHistogramMassCenter(histogram, nTrayWallID, n50umGV);
        bReturn = TRUE;
    }
    else if (nTrayWallID <= 0 && para.m_3D_doubleInsp)
        bReturn = FALSE;

    delete[] pnPeakIDNoMold;
    delete[] pnPeakCountNoMold;

    return bReturn;
}

bool Inspection_Impl::emptyCheck2D(const Para& para)
{
    static const bool bIsSideVision = SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_SIDE_INSP;

    const Ipvm::Image8u& image = m_result_2DEmpty.getImage(true);

    Ipvm::Image8u thresholdImage;
    Ipvm::Image8u bufferImage;
    Ipvm::Image8u maskImage;

    auto& memory = m_processor.getReusableMemory();

    if (!memory.GetByteImage(thresholdImage, image.GetSizeX(), image.GetSizeY()))
        return FALSE;
    if (!memory.GetByteImage(bufferImage, image.GetSizeX(), image.GetSizeY()))
        return FALSE;
    if (!memory.GetByteImage(maskImage, image.GetSizeX(), image.GetSizeY()))
        return FALSE;

    Ipvm::Point32r2 imageCenter(image.GetSizeX() * 0.5f, image.GetSizeY() * 0.5f);

    const auto& scale = m_processor.getScale();

    float minBlobSize = para.m_2D_emptyMinBlobSize_umSqure * scale.umToPixel().m_x * scale.umToPixel().m_y;
    minBlobSize /= (m_result_2DEmpty.getSamplingRate().m_x * m_result_2DEmpty.getSamplingRate().m_y);

    BYTE tabThresholdValue = (BYTE)min(255, max(0, para.m_2D_emptyThresholdValue));

    Ipvm::Rect32s rtTabThresholdROI = m_processor.getScale().convert_BCUToPixel(para.m_2D_emptyBlobROI_BCU, imageCenter,
        m_result_2DEmpty.getSamplingRate().m_x, m_result_2DEmpty.getSamplingRate().m_y);

    rtTabThresholdROI &= Ipvm::Rect32s(image);

    Ipvm::Rect32s rtMask = rtTabThresholdROI;
    rtMask.DeflateRect(2, 2);
    Ipvm::ImageProcessing::Fill(rtMask, 255, maskImage);

    BOOL bReverseThreshold = para.m_2D_emptyReverseThreshold;

    bufferImage.FillZero();
    thresholdImage.FillZero();
    Ipvm::ImageProcessing::Copy(image, rtTabThresholdROI, bufferImage);

    //먼저 Threshold 실행
    Ipvm::ImageProcessing::BinarizeGreaterEqual(bufferImage, rtTabThresholdROI, tabThresholdValue, thresholdImage);
    if (bReverseThreshold)
    {
        Ipvm::ImageProcessing::BitwiseNot(rtTabThresholdROI, thresholdImage);
    }

    // 영훈 20130905 : Blob ROI내부에 엉뚱한 이미지가 발생할 수 있어 Ignore ROI를 적용할 수 있도록 수정
    for (long n = 0; n < (long)para.m_2D_emptyIgnoreROIs_BCU.size(); n++)
    {
        Ipvm::Rect32s rtROI = m_processor.getScale().convert_BCUToPixel(para.m_2D_emptyIgnoreROIs_BCU[n], imageCenter,
            m_result_2DEmpty.getSamplingRate().m_x, m_result_2DEmpty.getSamplingRate().m_y);

        Ipvm::ImageProcessing::Fill(rtROI, 0, thresholdImage);
    }

    //{{ //kircheis_TRV
    Ipvm::Rect32s rtROI = rtTabThresholdROI;
    rtROI.InflateRect(-5, -5);
    double fMean = 0.;
    Ipvm::ImageProcessing::GetMean(image, rtROI, fMean);
    BOOL bGrabTimeOut = (fMean == 25 || fMean == 0);
    //}}
    //Blob 추출
    int32_t nBlobNum = BLOB_INFO_SMALL_SIZE;

    Ipvm::Image32s imageLabel;
    if (!memory.GetLongImage(imageLabel, image.GetSizeX(), image.GetSizeY()))
    {
        return false;
    }

    auto* blobClass = memory.GetBlob();
    auto* blobInfos = memory.GetBlobInfo();

    auto& result = m_result_2DEmpty;

    Ipvm::ImageProcessing::BitwiseAnd(maskImage, rtTabThresholdROI, thresholdImage);

    blobClass->DoBlob(
        thresholdImage, rtTabThresholdROI, BLOB_INFO_SMALL_SIZE, imageLabel, blobInfos, nBlobNum, long(minBlobSize));

    if (para.m_2D_emptyUseFilterBlobFullROI == TRUE)
        blobClass->FilteringROI(imageLabel, rtTabThresholdROI, 1, blobInfos, nBlobNum);

    result.setBlobRois(blobInfos, nBlobNum);

    if (bGrabTimeOut) //kircheis_TRV
    {
        return false;
    }

    if (nBlobNum < para.m_2D_emptyMinBlobCount)
    {
        return false;
    }

    return true;
}

void Inspection_Impl::calcParallelism()
{
    auto& vecsRefLine = m_result_edgeAlign.vecsRefLine;

    const auto& vecLeftEdgeAlignPoints = m_result_edgeAlign.vecLeftEdgeAlignPoints;
    const auto& vecTopEdgeAlignPoints = m_result_edgeAlign.vecTopEdgeAlignPoints;
    const auto& vecRightEdgeAlignPoints = m_result_edgeAlign.vecRightEdgeAlignPoints;
    const auto& vecBottomEdgeAlignPoints = m_result_edgeAlign.vecBottomEdgeAlignPoints;

    auto& vecfParallelismDistLeft = m_result_edgeAlign.vecfParallelismDistLeft;
    auto& vecfParallelismDistRight = m_result_edgeAlign.vecfParallelismDistRight;
    auto& vecfParallelismDistTop = m_result_edgeAlign.vecfParallelismDistTop;
    auto& vecfParallelismDistBottom = m_result_edgeAlign.vecfParallelismDistBottom;

    float fTemp = 0.5f;
    float fReference = 0.f;

    // LEFT
    fTemp = -(vecsRefLine[LEFT].m_b * vecLeftEdgeAlignPoints[0].m_y + vecsRefLine[LEFT].m_c) / vecsRefLine[LEFT].m_a;
    fReference = CPI_Geometry::GetDistance_PointToLine(fTemp, vecLeftEdgeAlignPoints[0].m_y, vecsRefLine[RIGHT]);
    for (long n = 1; n < (long)(vecLeftEdgeAlignPoints.size()); n++)
    {
        fTemp
            = -(vecsRefLine[LEFT].m_b * vecLeftEdgeAlignPoints[n].m_y + vecsRefLine[LEFT].m_c) / vecsRefLine[LEFT].m_a;
        float fDist = CPI_Geometry::GetDistance_PointToLine(fTemp, vecLeftEdgeAlignPoints[n].m_y, vecsRefLine[RIGHT]);
        fDist -= fReference;
        vecfParallelismDistLeft.push_back(fDist);
    }

    // RIGHT
    fTemp
        = -(vecsRefLine[RIGHT].m_b * vecRightEdgeAlignPoints[0].m_y + vecsRefLine[RIGHT].m_c) / vecsRefLine[RIGHT].m_a;
    fReference = CPI_Geometry::GetDistance_PointToLine(fTemp, vecRightEdgeAlignPoints[0].m_y, vecsRefLine[LEFT]);
    for (long n = 1; n < (long)(vecRightEdgeAlignPoints.size()); n++)
    {
        fTemp = -(vecsRefLine[RIGHT].m_b * vecRightEdgeAlignPoints[n].m_y + vecsRefLine[RIGHT].m_c)
            / vecsRefLine[RIGHT].m_a;
        float fDist = CPI_Geometry::GetDistance_PointToLine(fTemp, vecRightEdgeAlignPoints[n].m_y, vecsRefLine[LEFT]);
        fDist -= fReference;
        vecfParallelismDistRight.push_back(fDist);
    }

    // TOP
    fTemp = -(vecsRefLine[UP].m_a * vecTopEdgeAlignPoints[0].m_x + vecsRefLine[UP].m_c) / vecsRefLine[UP].m_b;
    fReference = CPI_Geometry::GetDistance_PointToLine(vecTopEdgeAlignPoints[0].m_x, fTemp, vecsRefLine[DOWN]);
    for (long n = 1; n < (long)(vecTopEdgeAlignPoints.size()); n++)
    {
        fTemp = -(vecsRefLine[UP].m_a * vecTopEdgeAlignPoints[n].m_x + vecsRefLine[UP].m_c) / vecsRefLine[UP].m_b;
        float fDist = CPI_Geometry::GetDistance_PointToLine(vecTopEdgeAlignPoints[n].m_x, fTemp, vecsRefLine[DOWN]);
        fDist -= fReference;
        vecfParallelismDistTop.push_back(fDist);
    }

    // BOTTOM
    fTemp = -(vecsRefLine[DOWN].m_a * vecBottomEdgeAlignPoints[0].m_x + vecsRefLine[DOWN].m_c) / vecsRefLine[DOWN].m_b;
    fReference = CPI_Geometry::GetDistance_PointToLine(vecBottomEdgeAlignPoints[0].m_x, fTemp, vecsRefLine[UP]);
    for (long n = 1; n < (long)(vecBottomEdgeAlignPoints.size()); n++)
    {
        fTemp = -(vecsRefLine[DOWN].m_a * vecBottomEdgeAlignPoints[n].m_x + vecsRefLine[DOWN].m_c)
            / vecsRefLine[DOWN].m_b;
        float fDist = CPI_Geometry::GetDistance_PointToLine(vecBottomEdgeAlignPoints[n].m_x, fTemp, vecsRefLine[UP]);
        fDist -= fReference;
        vecfParallelismDistBottom.push_back(fDist);
    }
}
