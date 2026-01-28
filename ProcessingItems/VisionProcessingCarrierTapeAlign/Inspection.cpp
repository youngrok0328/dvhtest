//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "Inspection.h"

//CPP_2_________________________________ This project's headers
#include "Para.h"

//CPP_3_________________________________ Other projects' headers
#include "../../AlgorithmModules/dPI_Blob/dPI_BlobLib.h"
#include "../../AlgorithmModules/dPI_EdgeDetect/dPI_EdgeDetectLib.h"
#include "../../AlgorithmModules/dPI_Geometry/dPI_GeometryLib.h"
#include "../../DefineModules/dA_Base/PI_RECT.h"
#include "../../DefineModules/dA_Base/SimpleFunction.h"
#include "../../ImageCombineModules/dPI_ippModules/ippModules.h"
#include "../../InformationModule/dPI_DataBase/PackageSpec.h"
#include "../../InformationModule/dPI_DataBase/VisionTapeSpec.h"
#include "../../InformationModule/dPI_DataBase/VisionTrayScanSpec.h"
#include "../../InformationModule/dPI_SystemIni/Config.h"
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../../MemoryModules/VisionReusableMemory/SurfaceLayerRoi.h"
#include "../../MemoryModules/VisionReusableMemory/VisionReusableMemory.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionAlignResult.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionBaseDef.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionDebugInfo.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionImageLot.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionImageLotInsp.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionInspectionOverlayResult.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionProcessing.h"

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
#include <math.h>

//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
Inspection::Inspection(VisionProcessing& processor)
    : m_processor(processor)
    , m_packageSpec(processor.m_packageSpec)
    , m_fCalcTime(0.f)
{
    auto& debugInfoGroup = m_processor.m_DebugInfoGroup;

    debugInfoGroup.Add(_T("EDGE Align Result"), enumDebugInfoType::None, 1);

    debugInfoGroup.Add(_T("Carrier Tape Left Search ROI"), enumDebugInfoType::Rect, 1);
    debugInfoGroup.Add(_T("Carrier Tape Right Search ROI"), enumDebugInfoType::Rect, 1);
    debugInfoGroup.Add(_T("Carrier Tape Edge Point Left Total"), enumDebugInfoType::Point_32f_C2, 1);
    debugInfoGroup.Add(_T("Carrier Tape Edge Point Right Total"), enumDebugInfoType::Point_32f_C2, 1);
    debugInfoGroup.Add(_DEBUG_INFO_CTA_CARRIER_LEFT_EDGE_POINT, enumDebugInfoType::Point_32f_C2, 1);
    debugInfoGroup.Add(_DEBUG_INFO_CTA_CARRIER_RIGHT_EDGE_POINT, enumDebugInfoType::Point_32f_C2, 1);
    debugInfoGroup.Add(_DEBUG_INFO_CTA_CARRIER_EDGE_LINE, enumDebugInfoType::Point_32f_C2, 1);
    debugInfoGroup.Add(_DEBUG_INFO_CTA_CARRIER_TAPE_ALIGN_FRECT, enumDebugInfoType::Rect_32f, 1);
    debugInfoGroup.Add(_DEBUG_INFO_CTA_CARRIER_TAPE_ALIGN_RECT, enumDebugInfoType::Rect, 1);

    debugInfoGroup.Add(_T("Pocket Left Search ROI"), enumDebugInfoType::Rect, 1);
    debugInfoGroup.Add(_T("Pocket Top Search ROI"), enumDebugInfoType::Rect, 1);
    debugInfoGroup.Add(_T("Pocket Right Search ROI"), enumDebugInfoType::Rect, 1);
    debugInfoGroup.Add(_T("Pocket Bottom Search ROI"), enumDebugInfoType::Rect, 1);
    debugInfoGroup.Add(_T("Pocket Edge Point Left Total"), enumDebugInfoType::Point_32f_C2, 1);
    debugInfoGroup.Add(_T("Pocket Edge Point Left Final"), enumDebugInfoType::Point_32f_C2, 1);
    debugInfoGroup.Add(_T("Pocket Edge Point Top Total"), enumDebugInfoType::Point_32f_C2, 1);
    debugInfoGroup.Add(_T("Pocket Edge Point Top Final"), enumDebugInfoType::Point_32f_C2, 1);
    debugInfoGroup.Add(_T("Pocket Edge Point Right Total"), enumDebugInfoType::Point_32f_C2, 1);
    debugInfoGroup.Add(_T("Pocket Edge Point Right Final"), enumDebugInfoType::Point_32f_C2, 1);
    debugInfoGroup.Add(_T("Pocket Edge Point Bottom Total"), enumDebugInfoType::Point_32f_C2, 1);
    debugInfoGroup.Add(_T("Pocket Edge Point Bottom Final"), enumDebugInfoType::Point_32f_C2, 1);
    debugInfoGroup.Add(_DEBUG_INFO_CTA_POCKET_ROUGH_ALIGN_QRECT, enumDebugInfoType::Quadrangle_32f, 1);
    debugInfoGroup.Add(_DEBUG_INFO_CTA_POCKET_ROUGH_ALIGN_RECT, enumDebugInfoType::Rect, 1);
    debugInfoGroup.Add(_DEBUG_INFO_CTA_POCKET_ROUGH_ALIGN_CENTER, enumDebugInfoType::Point_32f_C2, 1);
    debugInfoGroup.Add(_DEBUG_INFO_CTA_POCKET_ROUGH_ALIGN_CENTER_GAP_Y, enumDebugInfoType::Float, 1);

    debugInfoGroup.Add(_T("Body Rect"), enumDebugInfoType::Rect_32f, 1);
    debugInfoGroup.Add(_T("Body Center"), enumDebugInfoType::Point_32f_C2, 1);

    if (Config::getVisionType() == VISIONTYPE_2D_INSP
        || Config::getVisionType() == VISIONTYPE_SWIR_INSP) //kircheis_SideVision고려 //kircheis_SWIR
    {
        debugInfoGroup.Add(_T("Empty Blob Rect"), enumDebugInfoType::Rect_32f, 2);
        debugInfoGroup.Add(_T("Empty Blob Count"), enumDebugInfoType::Long, 2);
    }
}

Inspection::~Inspection()
{
}

void Inspection::resetResult()
{
    m_errorLogText.Empty();
    m_result.Init(Ipvm::Point32r2{0.f, 0.f});

    // Get Algorithm...
    auto& memory = m_processor.getReusableMemory();

    m_edgeDetector = memory.GetEdgeDetect();
}

bool Inspection::run(const Para& para, const bool detailSetupMode)
{
    Ipvm::TimeCheck time;

    m_result.m_inspectioned = true;

    m_result.m_success = DoAlign(para, detailSetupMode);

    if (!m_result.m_success)
    {
        m_processor.m_bInvalid = TRUE;
    }

    setDebugInfo(detailSetupMode);

    if (!detailSetupMode)
    {
        m_result.FreeMemoryForCalculation();
    }

    m_fCalcTime = CAST_FLOAT(time.Elapsed_ms());

    return m_result.m_success;
}

Ipvm::Rect32s Inspection::getImageSourceRoiForNoStitch()
{
    Ipvm::Point32r2 ptImageCenter
    {
        m_processor.getImageLot().GetImageSizeX() * 0.5f,
        m_processor.getImageLot().GetImageSizeY() * 0.5f
    };

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

bool Inspection::GetImageForAlign(const VisionInspFrameIndex& imageFrameIndex, const ImageProcPara& imageProc, Ipvm::Image8u& dstImage)
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
        mainImage = imageFrameIndex.getImage(true);
        if (mainImage.GetMem() == nullptr)
            return false;
    }

    if (mainImage.GetMem() == nullptr)
    {
        m_errorLogText.AppendFormat(_T("  Not enough Resuable Memory\r\n"));
        m_processor.m_bInvalid = FALSE;
        return false;
    }

    Ipvm::Rect32s paneRoi = getImageSourceRoiForNoStitch();
    if (paneRoi.IsRectEmpty())
    {
        m_errorLogText.AppendFormat(_T("  Pane ROI is empty\r\n"));
        return false;
    }

    Ipvm::Image8u paneSource;

    if (!memory.GetByteImage(paneSource, paneRoi.Width(), paneRoi.Height()))
    {
        m_errorLogText.AppendFormat(_T("  Not enough Resuable Memory\r\n"));
        return false;
    }

    // Pane Center 중심의 이미지를 만든다
    paneSource.FillZero();
    CippModules::CopyValid(mainImage, paneRoi.TopLeft(), Ipvm::Point32s2{0, 0},
        Ipvm::Size32s2{paneRoi.Width(), paneRoi.Height()}, paneSource);

    if (!memory.GetByteImage(dstImage, paneRoi.Width(), paneRoi.Height()))
    {
        m_errorLogText.AppendFormat(_T("  Not enough Resuable Memory\r\n"));
        return false;
    }

    return CippModules::GrayImageProcessingManageForAlignUseRawInPane(
        memory, &paneSource, paneRoi, imageProc, dstImage);
}

void Inspection::SetGlobalAlignResult()
{
    auto* newResult = new ResultAlign[1];
    *newResult = m_result;
    SetGlobalAlignResult(newResult);
}

void Inspection::SetGlobalAlignResult_ZeroAngle(const Ipvm::Point32r2& shift)
{
    SetGlobalAlignResult(GetNormalizedRotationResult(shift));
}

void Inspection::GetOverlayResult(
    VisionInspectionOverlayResult* overlayResult, long overlayMode, const bool detailSetupMode)
{
    UNREFERENCED_PARAMETER(detailSetupMode);

    if (overlayMode == OverlayType_ShowAll)
    {
        auto& debugInfoGroup = m_processor.m_DebugInfoGroup;
        const auto* psDebugInfo = debugInfoGroup.GetDebugInfo(_T("EDGE Align Result"));
        if (psDebugInfo != nullptr && psDebugInfo->nDataNum == 1)
        {
            auto* bodyAlignResult = (const VisionAlignResult*)psDebugInfo->pData;

            overlayResult->AddCrPoint(bodyAlignResult->m_center, RGB(0, 255, 0));
            overlayResult->AddLine(bodyAlignResult->fptLT, bodyAlignResult->fptRT, RGB(0, 255, 0));
            overlayResult->AddLine(bodyAlignResult->fptRT, bodyAlignResult->fptRB, RGB(0, 255, 0));
            overlayResult->AddLine(bodyAlignResult->fptRB, bodyAlignResult->fptLB, RGB(0, 255, 0));
            overlayResult->AddLine(bodyAlignResult->fptLB, bodyAlignResult->fptLT, RGB(0, 255, 0));
        }
    }
}

void Inspection::AppendTextResult(CString& textResult) const
{
    textResult.AppendFormat(_T("  [ Align Inspection Time : %.2f ]\r\n"), GetCalculationTime());
    textResult.AppendFormat(
        _T("    Body Left Top Point(X,Y) : %.2f, %.2f\r\n"), m_result.fptLT.m_x, m_result.fptLT.m_y);
    textResult.AppendFormat(
        _T("    Body Right Top Point(X,Y) : %.2f, %.2f\r\n"), m_result.fptRT.m_x, m_result.fptRT.m_y);
    textResult.AppendFormat(
        _T("    Body Left Bottom Point(X,Y) : %.2f, %.2f\r\n"), m_result.fptLB.m_x, m_result.fptLB.m_y);
    textResult.AppendFormat(
        _T("    Body Right Bottom Point(X,Y) : %.2f, %.2f\r\n"), m_result.fptRB.m_x, m_result.fptRB.m_y);
    textResult.AppendFormat(
        _T("    Body Center Point(X,Y) : %.2f, %.2f\r\n"), m_result.m_center.m_x, m_result.m_center.m_y);
    textResult.AppendFormat(_T("    Body Angle : %.2f\r\n\r\n"), m_result.m_angle_rad * ITP_RAD_TO_DEG);

    if (m_processor.m_bEmpty)
    {
        textResult.AppendFormat(_T("  [ Alin Empty ]\r\n"));
        textResult += GetErrorLogText();
    }

    if (m_processor.m_bInvalid)
    {
        textResult.AppendFormat(_T("  [ Edge Alin Invalid ]\r\n"));
        textResult += GetErrorLogText();
    }
}

ResultAlign& Inspection::GetResult()
{
    return m_result;
}

float Inspection::GetCalculationTime() const
{
    return m_fCalcTime;
}

LPCTSTR Inspection::GetErrorLogText() const
{
    return m_errorLogText;
}

ResultAlign* Inspection::GetNormalizedRotationResult(const Ipvm::Point32r2& shift)
{
    auto* normalizedResult = new ResultAlign[1];
    *normalizedResult = m_result;

    {
        // Body를 제외한 영역은 SetBodyAlign_AngleZero에서 처리하지 않으므로 이곳에서 처리한다
        float rotateAngle_rad = -m_result.m_angle_rad;
        const Ipvm::Point32r2 origin = m_result.m_center;
        normalizedResult->m_carrierTapeInspectionArea = Rotate(origin, rotateAngle_rad, m_result.m_carrierTapeInspectionArea) + shift;
    }

    normalizedResult->SetBodyAlign_AngleZero(shift);

    return normalizedResult;
}

Ipvm::Quadrangle32r Inspection::Rotate(
    const Ipvm::Point32r2& origin, float angle_rad, const Ipvm::Quadrangle32r& object) const
{
    auto lt = Rotate(origin, angle_rad, Ipvm::Point32r2{object.m_ltX, object.m_ltY});
    auto rt = Rotate(origin, angle_rad, Ipvm::Point32r2{object.m_rtX, object.m_rtY});
    auto rb = Rotate(origin, angle_rad, Ipvm::Point32r2{object.m_rbX, object.m_rbY});
    auto lb = Rotate(origin, angle_rad, Ipvm::Point32r2{object.m_lbX, object.m_lbY});
    
    Ipvm::Quadrangle32r retValue;
    retValue.m_ltX = lt.m_x;
    retValue.m_ltY = lt.m_y;
    retValue.m_rtX = rt.m_x;
    retValue.m_rtY = rt.m_y;
    retValue.m_rbX = rb.m_x;
    retValue.m_rbY = rb.m_y;
    retValue.m_lbX = lb.m_x;
    retValue.m_lbY = lb.m_y;

    return retValue;
}

Ipvm::Point32r2 Inspection::Rotate(const Ipvm::Point32r2& origin, float angle_rad, const Ipvm::Point32r2& object) const
{
    auto cosVal = float(cos(angle_rad));
    auto sinVal = float(sin(angle_rad));

    float fX = object.m_x - origin.m_x;
    float fY = object.m_y - origin.m_y;

    Ipvm::Point32r2 retValue;
    retValue.m_x = cosVal * fX - sinVal * fY + origin.m_x;
    retValue.m_y = sinVal * fX + cosVal * fY + origin.m_y;

    return retValue;
}

void Inspection::SetGlobalAlignResult(ResultAlign* result)
{
    auto& debugInfoGroup = m_processor.m_DebugInfoGroup;
    
    VisionTapeSpec* tapeSpec = m_processor.getTapeSpec();
    const auto& scale = m_processor.getScale();
    //Ipvm::Point32r2 pocketCenter{m_result.getImage(false).GetSizeX() * 0.5f, m_result.getImage(false).GetSizeY() * 0.5f};
    Ipvm::Point32r2 pocketCenter{
        m_processor.getImageLotInsp().GetImageSizeX() * 0.5f, m_processor.getImageLotInsp().GetImageSizeY() * 0.5f};

    auto pocketPitchY_px = scale.convert_umToPixelY(tapeSpec->m_p1 * 1000.f);
    auto pocketSizeY_px = scale.convert_umToPixelY(tapeSpec->m_a0 * 1000.f);
    auto pocketGapY_px = pocketPitchY_px - pocketSizeY_px;
    auto interestRangeY_px = (pocketSizeY_px * .5f) + (pocketGapY_px * 0.5f);
    auto startY = pocketCenter.m_y - interestRangeY_px;
    auto endY = pocketCenter.m_y + interestRangeY_px;

    if (auto* psDebugInfo = debugInfoGroup.GetDebugInfo(_T("EDGE Align Result")))
    {
        if (psDebugInfo->pData != nullptr)
        {
            psDebugInfo->Reset();
        }

        psDebugInfo->nDataNum = 1;
        psDebugInfo->pData = result;

        // Align 정보를 기록해 준다
        auto& memory = m_processor.getReusableMemory();
        memory.SetAlignInfo(result->m_center, float(result->m_angle_rad * IV_RAD_TO_DEG));
        if (auto* surfaceRoi = memory.AddSurfaceLayerRoiClass(_T("Measured - Body")))
        {
            Ipvm::Quadrangle32r body;
            body.m_ltX = result->fptLT.m_x;
            body.m_ltY = ClampBetween(result->fptLT.m_y, startY, endY);
            body.m_rtX = result->fptRT.m_x;
            body.m_rtY = ClampBetween(result->fptRT.m_y, startY, endY);
            body.m_lbX = result->fptLB.m_x;
            body.m_lbY = ClampBetween(result->fptLB.m_y, startY, endY);
            body.m_rbX = result->fptRB.m_x;
            body.m_rbY = ClampBetween(result->fptRB.m_y, startY, endY);

            surfaceRoi->Reset();
            surfaceRoi->Add(body);
        }

        if (auto* surfaceRoi = memory.AddSurfaceLayerRoiClass(_T("Measured - Carrier Tape")))
        {
            Ipvm::Quadrangle32r carrierTape = result->m_carrierTapeInspectionArea;
            surfaceRoi->Reset();
            carrierTape.m_ltY = ClampBetween(carrierTape.m_ltX, startY, endY);
            carrierTape.m_rtY = ClampBetween(carrierTape.m_rtY, startY, endY);
            carrierTape.m_lbY = ClampBetween(carrierTape.m_lbY, startY, endY);
            carrierTape.m_rbY = ClampBetween(carrierTape.m_rbY, startY, endY);
            surfaceRoi->Add(carrierTape);
        }
    }
}

void Inspection::setDebugInfo(const bool detailSetupMode)
{
    m_processor.SetDebugInfoItem(detailSetupMode, _T("Body Rect"), m_result.getBodyRect());
    m_processor.SetDebugInfoItem(detailSetupMode, _T("Body Center"), m_result.m_center);

    m_processor.SetDebugInfoItem(
        detailSetupMode, _T("Carrier Tape Left Search ROI"), m_result.m_carrierTapeEdgeSearchRois[0]);
    m_processor.SetDebugInfoItem(
        detailSetupMode, _T("Carrier Tape Right Search ROI"), m_result.m_carrierTapeEdgeSearchRois[1]);

    m_processor.SetDebugInfoItem(
        detailSetupMode, _T("Carrier Tape Edge Point Left Total"), m_result.m_carrierTapeEdgePointsTotal[0]);

    m_processor.SetDebugInfoItem(
        detailSetupMode, _T("Carrier Tape Edge Point Right Total"), m_result.m_carrierTapeEdgePointsTotal[1]);

    m_processor.SetDebugInfoItem(detailSetupMode, _DEBUG_INFO_CTA_CARRIER_LEFT_EDGE_POINT, m_result.m_carrierTapeEdgePoints[0], true);
    m_processor.SetDebugInfoItem(detailSetupMode, _DEBUG_INFO_CTA_CARRIER_RIGHT_EDGE_POINT, m_result.m_carrierTapeEdgePoints[1], true);
    m_processor.SetDebugInfoItem(detailSetupMode, _DEBUG_INFO_CTA_CARRIER_EDGE_LINE, m_result.m_carrierTapeRefLine, true);

    Ipvm::Rect32r tapeInspectionAreaFRect;
    Ipvm::Rect32s tapeInspectionAreaRect;
    tapeInspectionAreaFRect = m_result.m_carrierTapeInspectionArea.GetExtRect();
    tapeInspectionAreaRect = {
        LONG(tapeInspectionAreaFRect.m_left), LONG(tapeInspectionAreaFRect.m_top),
        LONG(tapeInspectionAreaFRect.Width()), LONG(tapeInspectionAreaFRect.Height())};
    m_processor.SetDebugInfoItem(detailSetupMode, _DEBUG_INFO_CTA_CARRIER_TAPE_ALIGN_FRECT, tapeInspectionAreaFRect, true);
    m_processor.SetDebugInfoItem(detailSetupMode, _DEBUG_INFO_CTA_CARRIER_TAPE_ALIGN_RECT, tapeInspectionAreaRect, true);

    m_processor.SetDebugInfoItem(detailSetupMode, _T("Pocket Left Search ROI"), m_result.m_pocketEdgeSearchRois[LEFT]);
    m_processor.SetDebugInfoItem(detailSetupMode, _T("Pocket Top Search ROI"), m_result.m_pocketEdgeSearchRois[UP]);
    m_processor.SetDebugInfoItem(
        detailSetupMode, _T("Pocket Right Search ROI"), m_result.m_pocketEdgeSearchRois[RIGHT]);
    m_processor.SetDebugInfoItem(
        detailSetupMode, _T("Pocket Bottom Search ROI"), m_result.m_pocketEdgeSearchRois[DOWN]);

    m_processor.SetDebugInfoItem(
        detailSetupMode, _T("Pocket Edge Point Left Total"), m_result.m_pocketEdgePointsTotal[LEFT]);
    m_processor.SetDebugInfoItem(
        detailSetupMode, _T("Pocket Edge Point Left Final"), m_result.m_pocketEdgePoints[LEFT]);

    m_processor.SetDebugInfoItem(
        detailSetupMode, _T("Pocket Edge Point Top Total"), m_result.m_pocketEdgePointsTotal[UP]);
    m_processor.SetDebugInfoItem(detailSetupMode, _T("Pocket Edge Point Top Final"), m_result.m_pocketEdgePoints[UP]);

    m_processor.SetDebugInfoItem(
        detailSetupMode, _T("Pocket Edge Point Right Total"), m_result.m_pocketEdgePointsTotal[RIGHT]);
    m_processor.SetDebugInfoItem(
        detailSetupMode, _T("Pocket Edge Point Right Final"), m_result.m_pocketEdgePoints[RIGHT]);

    m_processor.SetDebugInfoItem(
        detailSetupMode, _T("Pocket Edge Point Bottom Total"), m_result.m_pocketEdgePointsTotal[DOWN]);
    m_processor.SetDebugInfoItem(
        detailSetupMode, _T("Pocket Edge Point Bottom Final"), m_result.m_pocketEdgePoints[DOWN]);

    Ipvm::Point32r2 pocketAlign[4];
    Ipvm::Geometry::GetCrossPoint(m_result.m_pocketRefLine[LEFT], m_result.m_pocketRefLine[UP], pocketAlign[UP]);
    Ipvm::Geometry::GetCrossPoint(m_result.m_pocketRefLine[UP], m_result.m_pocketRefLine[RIGHT], pocketAlign[RIGHT]);
    Ipvm::Geometry::GetCrossPoint(m_result.m_pocketRefLine[RIGHT], m_result.m_pocketRefLine[DOWN], pocketAlign[DOWN]);
    Ipvm::Geometry::GetCrossPoint(m_result.m_pocketRefLine[DOWN], m_result.m_pocketRefLine[LEFT], pocketAlign[LEFT]);

    m_result.m_roughPocketAlignQuadRect.m_ltX = pocketAlign[UP].m_x;
    m_result.m_roughPocketAlignQuadRect.m_ltY = pocketAlign[UP].m_y;
    m_result.m_roughPocketAlignQuadRect.m_rtX = pocketAlign[RIGHT].m_x;
    m_result.m_roughPocketAlignQuadRect.m_rtY = pocketAlign[RIGHT].m_y;
    m_result.m_roughPocketAlignQuadRect.m_rbX = pocketAlign[DOWN].m_x;
    m_result.m_roughPocketAlignQuadRect.m_rbY = pocketAlign[DOWN].m_y;
    m_result.m_roughPocketAlignQuadRect.m_lbX = pocketAlign[LEFT].m_x;
    m_result.m_roughPocketAlignQuadRect.m_lbY = pocketAlign[LEFT].m_y;
    m_processor.SetDebugInfoItem(
        detailSetupMode, _DEBUG_INFO_CTA_POCKET_ROUGH_ALIGN_QRECT, m_result.m_roughPocketAlignQuadRect, true);

    Ipvm::Point32r2 pocketCenter = m_result.m_roughPocketAlignQuadRect.CenterPoint();
    m_processor.SetDebugInfoItem(detailSetupMode, _DEBUG_INFO_CTA_POCKET_ROUGH_ALIGN_CENTER, pocketCenter, true);

    const auto& scale = m_processor.getScale();
    float pocketCenterY = pocketCenter.m_y;
    float pocketCenterGapY = (pocketCenterY - m_result.getPaneCenter().m_y) * scale.pixelToMm().m_y;
    m_processor.SetDebugInfoItem(
        detailSetupMode, _DEBUG_INFO_CTA_POCKET_ROUGH_ALIGN_CENTER_GAP_Y, pocketCenterGapY, true);

    Ipvm::Rect32r pocketAlignFRect = m_result.m_roughPocketAlignQuadRect.GetExtRect();
    m_result.m_roughPocketAlignRect = {LONG(pocketAlignFRect.m_left), LONG(pocketAlignFRect.m_top),
        LONG(pocketAlignFRect.m_right), LONG(pocketAlignFRect.m_bottom)};
    m_processor.SetDebugInfoItem(
        detailSetupMode, _DEBUG_INFO_CTA_POCKET_ROUGH_ALIGN_RECT, m_result.m_roughPocketAlignRect, true);
}

bool Inspection::DoAlign(const Para& para, [[maybe_unused]] const bool detailSetupMode)
{
    auto paneCenter = m_result.getPaneCenter();

    // Carrier Tape의 양쪽 Edge를 검출한다
    // 이것은 대략적인 Package X위치와 회전 Angle을 구하는데 사용된다
    if (!EdgeDetectCarrierTapeX(m_result.getImage(false), para, paneCenter))
    {
        m_errorLogText = _T("Carrier Tap Edge not found for Left side\r\n");
        return false;
    }

    if (!SetCarrierAlignResult())
    {
        m_errorLogText = _T("Carrier Tap Align fail\r\n");
        return false;
    }

    if (EdgeDetectPocket(m_result.getImage(true), para, paneCenter) == false)
    {
        m_errorLogText = _T("Pocket Edge not found\r\n");
        return false;
    }

    if (!SetPocketAlignResult())
    {
        m_errorLogText = _T("Pocket Align fail\r\n");
        return false;
    }

    calcParallelism();
    return true;
}

bool Inspection::EdgeDetectCarrierTapeX(const Ipvm::Image8u& image, const Para& para, const Ipvm::Point32r2& paneCenter)
{
    if (m_processor.getTapeSpec() == nullptr)
    {
        return false;
    }

    const auto& scale = m_processor.getScale();
    const auto& edgeParams = para.m_carrierTapeAlign.m_edgeParams;

    float carrierTapeWidth_px = scale.convert_umToPixelX(m_processor.getTapeSpec()->m_w0 * 1000.f);
    float pocketSizeY_px = scale.convert_umToPixelY(m_processor.getTapeSpec()->m_a0 * 1000.f);

    auto oldThresholdValue = Ipvm::k_noiseValue32r;
    if (edgeParams.m_edgeDetectMode == EdgeDetectMode_FirstEdge)
    {
        oldThresholdValue = m_edgeDetector->SetMininumThreshold(edgeParams.m_firstEdgeMinThreshold);
    }

    if (m_edgeDetector->GetMininumThreshold() < 1.f)
    {
        float fDefaultEdgeThresh = 1.f;
        m_edgeDetector->SetMininumThreshold(fDefaultEdgeThresh);
    }

    bool success = true;

    for (auto roiIndex = 0; roiIndex < 2; roiIndex++)
    {
        auto& rtROI = m_result.m_carrierTapeEdgeSearchRois[roiIndex];
        auto direction = 0;
        switch (roiIndex)
        {
            case 0:
                direction = LEFT;
                break;
            case 1:
                direction = RIGHT;
                break;
            default:
                success = false;
                continue;
        }
        rtROI = para.m_carrierTapeAlign.m_edgeSearchRois[roiIndex].ToRect(
            scale, direction, paneCenter, carrierTapeWidth_px, pocketSizeY_px);

        rtROI &= Ipvm::Rect32s(image);

        if (!EdgeDetectRoi(image, edgeParams, rtROI, direction, m_result.m_carrierTapeEdgePointsTotal[roiIndex],
                m_result.m_carrierTapeEdgePoints[roiIndex]))
        {
            success = false;
        }
    }

    if (oldThresholdValue != Ipvm::k_noiseValue32r)
    {
        m_edgeDetector->SetMininumThreshold(oldThresholdValue);
    }

    return success;
}

bool Inspection::EdgeDetectPocket(const Ipvm::Image8u& image, const Para& para, const Ipvm::Point32r2& paneCenter)
{
    if (m_processor.getTapeSpec() == nullptr)
    {
        return false;
    }

    auto pocketCenter = paneCenter;

    if (m_result.m_roughPocketCenter_px.m_x != Ipvm::k_noiseValue32r)
    {
        // Pocket Center 위치가 이미 계산되어 있으면 그것을 사용한다
        pocketCenter.m_x = m_result.m_roughPocketCenter_px.m_x;
    }

    if (m_result.m_roughPocketCenter_px.m_y != Ipvm::k_noiseValue32r)
    {
        // Pocket Center 위치가 이미 계산되어 있으면 그것을 사용한다
        pocketCenter.m_y = m_result.m_roughPocketCenter_px.m_y;
    }

    const auto& scale = m_processor.getScale();
    const auto& edgeParams = para.m_pocketAlign.m_edgeParams;

    float pocketSizeX_px = scale.convert_umToPixelX(m_processor.getTapeSpec()->m_b0 * 1000.f);
    float pocketSizeY_px = scale.convert_umToPixelY(m_processor.getTapeSpec()->m_a0 * 1000.f);

    auto oldThresholdValue = Ipvm::k_noiseValue32r;
    if (edgeParams.m_edgeDetectMode == EdgeDetectMode_FirstEdge)
    {
        oldThresholdValue = m_edgeDetector->SetMininumThreshold(edgeParams.m_firstEdgeMinThreshold);
    }

    if (m_edgeDetector->GetMininumThreshold() < 1.f)
    {
        float fDefaultEdgeThresh = 1.f;
        m_edgeDetector->SetMininumThreshold(fDefaultEdgeThresh);
    }

    bool success = true;

    for (auto direction = 0; direction < 4; direction++)
    {
        auto& rtROI = m_result.m_pocketEdgeSearchRois[direction];

        rtROI = para.m_pocketAlign.m_edgeSearchRois[direction].ToRect(
            scale, direction, pocketCenter, pocketSizeX_px, pocketSizeY_px);

        rtROI &= Ipvm::Rect32s(image);

        if (!EdgeDetectRoi(image, edgeParams, rtROI, direction, m_result.m_pocketEdgePointsTotal[direction],
                m_result.m_pocketEdgePoints[direction]))
        {
            success = false;
        }
    }

    if (oldThresholdValue != Ipvm::k_noiseValue32r)
    {
        m_edgeDetector->SetMininumThreshold(oldThresholdValue);
    }

    return success;
}

bool Inspection::EdgeDetectRoi(const Ipvm::Image8u& image, const ParaEdgeParameters& para,
    const Ipvm::Rect32s& searchRoi, int roiDirection, std::vector<Ipvm::Point32r2>& o_edgePointsTotal,
    std::vector<Ipvm::Point32r2>& o_edgePoints)
{
    if (para.m_searchDirection != PI_ED_DIR_OUTER)
    {
        // Out->In
        // roiDirection 반대로 바꿔준다.

        switch (roiDirection)
        {
            case LEFT:
                roiDirection = RIGHT;
                break;
            case RIGHT:
                roiDirection = LEFT;
                break;
            case UP:
                roiDirection = DOWN;
                break;
            case DOWN:
                roiDirection = UP;
                break;
        }
    }

    const auto& scale = m_processor.getScale();
    bool isVertical = (roiDirection == UP || roiDirection == DOWN);

    float fpx2um_x = scale.pixelToUm().m_x;
    auto stepJump = static_cast<long>((para.m_edgeAlignGap_um / fpx2um_x) + 0.5f);
    long roiTotalWidth = isVertical ? searchRoi.Width() : searchRoi.Height();
    auto searchLength = isVertical ? searchRoi.Height() : searchRoi.Width();
    auto searchROIWidth = static_cast<float>(roiTotalWidth) * 0.1f;
    long edgeType = para.m_edgeType ? PI_ED_DIR_RISING : PI_ED_DIR_FALLING;

    std::vector<Ipvm::Point32r2> vecNewpoint;
    std::vector<Ipvm::Point32r2> vecFirstEdge;
    std::vector<Ipvm::Point32r2> vecBestEdge;

    for (long stepPos = 0; stepPos < roiTotalWidth; stepPos += stepJump)
    {
        long nSearchDirection = roiDirection;
        Ipvm::Point32s2 ptStart{0, 0};

        switch (roiDirection)
        {
            case LEFT:
                ptStart = {searchRoi.m_right, searchRoi.m_top + stepPos};
                break;
            case RIGHT:
                ptStart = {searchRoi.m_left, searchRoi.m_top + stepPos};
                break;
            case UP:
                ptStart = {searchRoi.m_left + stepPos, searchRoi.m_bottom};
                break;
            case DOWN:
                ptStart = {searchRoi.m_left + stepPos, searchRoi.m_top};
                break;
        }

        if (para.m_edgeDetectMode == EdgeDetectMode_FirstEdge || para.m_edgeDetectMode == EdgeDetectMode_AutoDetect)
        {
            Ipvm::Point32r2 firstEdge{};
            if (m_edgeDetector->PI_ED_CalcAvgEdgePointFromImage(edgeType, ptStart, nSearchDirection, searchLength,
                    CAST_INT(searchROIWidth), image, firstEdge, TRUE))
            {
                o_edgePointsTotal.push_back(firstEdge);

                vecNewpoint.push_back(firstEdge);
                vecFirstEdge.push_back(firstEdge);
            }
        }

        if (para.m_edgeDetectMode == EdgeDetectMode_BestEdge || para.m_edgeDetectMode == EdgeDetectMode_AutoDetect)
        {
            Ipvm::Point32r2 bestEdge{};
            if (m_edgeDetector->PI_ED_CalcAvgEdgePointFromImage(edgeType, ptStart, nSearchDirection, searchLength,
                    CAST_INT(searchROIWidth), image, bestEdge, FALSE))
            {
                o_edgePointsTotal.push_back(bestEdge);

                vecNewpoint.push_back(bestEdge); // 하나의 공간에 first와 Best의 Edge를 모두 넣는다.
                vecBestEdge.push_back(bestEdge);
            }
        }
    }

    if (!RoughAlign_LineAlign(para, roiDirection, vecNewpoint, vecFirstEdge, vecBestEdge, o_edgePoints))
    {
        return false;
    }

    if (o_edgePoints.empty())
    {
        return false;
    }

    return true;
}

bool Inspection::SetCarrierAlignResult()
{
    const auto* tapeSpec = m_processor.getTapeSpec();
    if (tapeSpec == nullptr)
    {
        return false;
    }

    if (m_result.m_carrierTapeEdgePoints[0].empty() || m_result.m_carrierTapeEdgePoints[1].empty())
    {
        return false;
    }

    // 처음에 네 직선 방정식을 구한다.
    auto& vecsRefLine = m_result.m_carrierTapeRefLine;
    auto& leftLine = vecsRefLine[0];
    auto& rightLine = vecsRefLine[1];

    if (Ipvm::DataFitting::FitToLineRn(
            long(m_result.m_carrierTapeEdgePoints[0].size()), m_result.m_carrierTapeEdgePoints[0].data(), 3.f, leftLine)
            != Ipvm::Status::e_ok
        || Ipvm::DataFitting::FitToLineRn(long(m_result.m_carrierTapeEdgePoints[1].size()),
               m_result.m_carrierTapeEdgePoints[1].data(), 3.f, rightLine)
            != Ipvm::Status::e_ok)
    {
        return false;
    }

    // 이상적인 포켓 Center은 화면의 중심이라 가정한다.
    Ipvm::Point32r2 pocketCenter{m_result.getImage(true).GetSizeX() * 0.5f, m_result.getImage(true).GetSizeY() * 0.5f};

    // 이상적인 Rect를 만든다
    const auto& scale = m_processor.getScale();
    float rectSizeX_px = scale.convert_pixelToUmX(tapeSpec->m_w0 * 1000.f);
    float rectSizeY_px = scale.convert_umToPixelY(tapeSpec->m_a0 * 1000.f);

    Ipvm::Point32r2 ltIdeal{pocketCenter.m_x - (rectSizeX_px * 0.5f), pocketCenter.m_y - (rectSizeY_px * 0.5f)};
    Ipvm::Point32r2 rtIdeal{pocketCenter.m_x + (rectSizeX_px * 0.5f), pocketCenter.m_y - (rectSizeY_px * 0.5f)};
    Ipvm::Point32r2 rbIdeal{pocketCenter.m_x + (rectSizeX_px * 0.5f), pocketCenter.m_y + (rectSizeY_px * 0.5f)};
    Ipvm::Point32r2 lbIdeal{pocketCenter.m_x - (rectSizeX_px * 0.5f), pocketCenter.m_y + (rectSizeY_px * 0.5f)};

    Ipvm::LineEq32r upIdeal;
    Ipvm::LineEq32r downIdeal;

    if (Ipvm::DataFitting::FitToLine(ltIdeal, rtIdeal, upIdeal) != Ipvm::Status::e_ok
        || Ipvm::DataFitting::FitToLine(lbIdeal, rbIdeal, downIdeal) != Ipvm::Status::e_ok)
    {
        return false;
    }

    // 직선 방정식으로부터 네 모서리를 계산한다.
    Ipvm::Point32r2 ptLeftTop;
    Ipvm::Point32r2 ptRightTop;
    Ipvm::Point32r2 ptRightBottom;
    Ipvm::Point32r2 ptLeftBottom;

    if (Ipvm::Geometry::GetCrossPoint(leftLine, upIdeal, ptLeftTop) != Ipvm::Status::e_ok
        || Ipvm::Geometry::GetCrossPoint(upIdeal, rightLine, ptRightTop) != Ipvm::Status::e_ok
        || Ipvm::Geometry::GetCrossPoint(rightLine, downIdeal, ptRightBottom) != Ipvm::Status::e_ok
        || Ipvm::Geometry::GetCrossPoint(downIdeal, leftLine, ptLeftBottom) != Ipvm::Status::e_ok)
    {
        return false;
    }

    // 각도 계산
    auto angleLeft = CAST_FLOAT(
        atan2(ptLeftBottom.m_y - ptLeftTop.m_y, ptLeftBottom.m_x - ptLeftTop.m_x) - (90. * 0.017453292519943));
    auto angleRight = CAST_FLOAT(
        atan2(ptRightBottom.m_y - ptRightTop.m_y, ptRightBottom.m_x - ptRightTop.m_x) - (90. * 0.017453292519943));
    auto angle = (angleLeft + angleRight) / 2.f;

    float fAngle_radVal = angle;
    m_result.SetAngle(fAngle_radVal);

    // ptLeftTop.m_x를 CarrierTape의 시작점으로 생각하고 pocket center x를 구한다.
    float pocketX_px = ptLeftTop.m_x;

    switch (tapeSpec->m_sprocketHoleExistType)
    {
        case TapeSprocketHoleExistType::e_both:
        case TapeSprocketHoleExistType::e_leftOnly:
        {
            pocketX_px += scale.convert_umToPixelX(tapeSpec->m_e1 * 1000.f); // sprocket hole center
            pocketX_px += scale.convert_umToPixelX(tapeSpec->m_f0 * 1000.f); // sprocket hole to pocket center
            break;
        }
        case TapeSprocketHoleExistType::e_rightOnly:
        {
            pocketX_px += scale.convert_umToPixelX(tapeSpec->m_w0 * 1000.f); // tape width
            pocketX_px -= scale.convert_umToPixelX(tapeSpec->m_e1 * 1000.f); // sprocket hole center
            pocketX_px -= scale.convert_umToPixelX(tapeSpec->m_f0 * 1000.f); // sprocket hole to pocket center
            break;
        }
        default:
        {
            ASSERT(!"Unknown TapeSprocketHoleExistType");
            return false;
        }
    }

    m_result.m_roughPocketCenter_px.m_x = pocketX_px;

    return true;
}

bool Inspection::SetPocketAlignResult()
{
    // 처음에 네 직선 방정식을 구한다.
    auto& vecsRefLine = m_result.m_pocketRefLine;

    auto directionIndex = 0;
    for (const auto& edgePoints : m_result.m_pocketEdgePoints)
    {
        if (edgePoints.empty())
        {
            return false;
        }

        if (Ipvm::DataFitting::FitToLineRn(long(edgePoints.size()), &edgePoints[0], 3.f, vecsRefLine[directionIndex])
            != Ipvm::Status::e_ok)
        {
            return false;
        }

        directionIndex++;
    }

    // 직선 방정식으로부터 네 모서리를 계산한다.
    Ipvm::Point32r2 ptLeftTop;
    Ipvm::Point32r2 ptRightTop;
    Ipvm::Point32r2 ptRightBottom;
    Ipvm::Point32r2 ptLeftBottom;

    if (Ipvm::Geometry::GetCrossPoint(vecsRefLine[LEFT], vecsRefLine[UP], ptLeftTop) != Ipvm::Status::e_ok)
        return false;
    if (Ipvm::Geometry::GetCrossPoint(vecsRefLine[UP], vecsRefLine[RIGHT], ptRightTop) != Ipvm::Status::e_ok)
        return false;
    if (Ipvm::Geometry::GetCrossPoint(vecsRefLine[RIGHT], vecsRefLine[DOWN], ptRightBottom) != Ipvm::Status::e_ok)
        return false;
    if (Ipvm::Geometry::GetCrossPoint(vecsRefLine[DOWN], vecsRefLine[LEFT], ptLeftBottom) != Ipvm::Status::e_ok)
        return false;

    // 포켓 네 모서리로부터 포켓 센터를 얻는다.
    //auto pocketCenterX_px = (ptLeftTop.m_x + ptRightTop.m_x + ptRightBottom.m_x + ptLeftBottom.m_x) * 0.25f;
    auto pocketCenterY_px = (ptLeftTop.m_y + ptRightTop.m_y + ptRightBottom.m_y + ptLeftBottom.m_y) * 0.25f;

    const auto& scale = m_processor.getScale();

    // 포켓 센터로부터 pocket pitch y - pocket size y /2 만큼 떨어진 영역을 pocket inspection area로 설정한다.
    // pocket inspection area의 좌우는 carrier tape edge로 설정한다.
    float pocketSizeY_px = scale.convert_umToPixelY(m_processor.getTapeSpec()->m_a0 * 1000.f);
    float pocketPitchY_px = scale.convert_umToPixelY(m_processor.getTapeSpec()->m_p1 * 1000.f);
    float inspectionAreaTop = pocketCenterY_px - (pocketPitchY_px - pocketSizeY_px * 0.5f);
    float inspectionAreaBottom = pocketCenterY_px + (pocketPitchY_px - pocketSizeY_px * 0.5f);

    auto& crrierTapeArea = m_result.m_carrierTapeInspectionArea;
    crrierTapeArea.m_ltX = m_result.m_carrierTapeRefLine[0].GetPositionX(inspectionAreaTop);
    crrierTapeArea.m_ltY = inspectionAreaTop;
    crrierTapeArea.m_rtX = m_result.m_carrierTapeRefLine[1].GetPositionX(inspectionAreaTop);
    crrierTapeArea.m_rtY = inspectionAreaTop;
    crrierTapeArea.m_rbX = m_result.m_carrierTapeRefLine[1].GetPositionX(inspectionAreaBottom);
    crrierTapeArea.m_rbY = inspectionAreaBottom;
    crrierTapeArea.m_lbX = m_result.m_carrierTapeRefLine[0].GetPositionX(inspectionAreaBottom);
    crrierTapeArea.m_lbY = inspectionAreaBottom;

    // Body는 아니지만 TR 검사에서 검사 영역으로 사용하므로 Body 영역으로 설정해 준다
    m_result.fptLT = {crrierTapeArea.m_ltX, crrierTapeArea.m_ltY};
    m_result.fptRT = {crrierTapeArea.m_rtX, crrierTapeArea.m_rtY};
    m_result.fptRB = {crrierTapeArea.m_rbX, crrierTapeArea.m_rbY};
    m_result.fptLB = {crrierTapeArea.m_lbX, crrierTapeArea.m_lbY};

    m_result.m_center.m_x = (m_result.fptLT.m_x + m_result.fptRT.m_x + m_result.fptRB.m_x + m_result.fptLB.m_x) * 0.25f;
    m_result.m_center.m_y = (m_result.fptLT.m_y + m_result.fptRT.m_y + m_result.fptRB.m_y + m_result.fptLB.m_y) * 0.25f;

    auto panelCenter = m_result.getPaneCenter();

    m_result.m_analysis_packageOffsetX_um = scale.convert_pixelToUmX(m_result.m_center.m_x - panelCenter.m_x);
    m_result.m_analysis_packageOffsetY_um = scale.convert_pixelToUmY(m_result.m_center.m_y - panelCenter.m_y);

    // Angle은 Carrier Tape Align에서 이미 구했다

    // 결과 최종 업데이트!!
    m_result.bAvailable = TRUE;

    return true;
}

bool Inspection::RoughAlign_LineAlign(const ParaEdgeParameters& para, long nDirection,
    const std::vector<Ipvm::Point32r2>& vecfpNewPoints, const std::vector<Ipvm::Point32r2>& vecfpFirstEdge,
    const std::vector<Ipvm::Point32r2>& vecfpBestEdge, std::vector<Ipvm::Point32r2>& accumulatedFilteredPoints) const
{
    if ((long)(vecfpNewPoints.size()) <= 5)
    {
        return false;
    }

    const std::vector<Ipvm::Point32r2>* useVectorXY = nullptr;

    switch (para.m_edgeDetectMode)
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
        default:
            ASSERT(!"Unknown EdgeDetectMode");
            return false;
    }

    if (useVectorXY == nullptr)
    {
        return false;
    }

    auto pointCount = (long)(useVectorXY->size());

    if (pointCount <= 0)
    {
        return false;
    }

    std::vector<Ipvm::Point32r2> vecfFilteredXY;

    Ipvm::LineEq32r line;

    const auto& scale = m_processor.getScale();

    static const float filteringDistanceUm = 10.f;
    float limitDistance = (nDirection == LEFT || nDirection == RIGHT)
        ? scale.convert_umToPixelX(filteringDistanceUm)
        : scale.convert_umToPixelY(filteringDistanceUm); //일단은 이렇게 써보자

    if (!CPI_Geometry::RoughLineFitting(
        pointCount,
        useVectorXY->data(),
        line,
        5,
        limitDistance,
        vecfFilteredXY))
    {
        return false;
    }

    if (vecfFilteredXY.empty())
    {
        return false;
    }

    for (const auto& filteredXY : vecfFilteredXY)
    {
        accumulatedFilteredPoints.push_back(filteredXY);
    }

    return true;
}

void Inspection::calcParallelism()
{
    auto& vecsRefLine = m_result.m_pocketRefLine;

    const auto& vecLeftEdgeAlignPoints = m_result.m_pocketEdgePoints[LEFT];
    const auto& vecTopEdgeAlignPoints = m_result.m_pocketEdgePoints[UP];
    const auto& vecRightEdgeAlignPoints = m_result.m_pocketEdgePoints[RIGHT];
    const auto& vecBottomEdgeAlignPoints = m_result.m_pocketEdgePoints[DOWN];

    auto& vecfParallelismDistLeft = m_result.vecfParallelismDistLeft;
    auto& vecfParallelismDistRight = m_result.vecfParallelismDistRight;
    auto& vecfParallelismDistTop = m_result.vecfParallelismDistTop;
    auto& vecfParallelismDistBottom = m_result.vecfParallelismDistBottom;

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
