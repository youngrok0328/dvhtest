//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionProcessingAlign3D.h"

//CPP_2_________________________________ This project's headers
#include "DlgVisionProcessingAlign3D.h"
#include "MergeResult.h"
#include "Result.h"

//CPP_3_________________________________ Other projects' headers
#include "../../AlgorithmModules/dPI_Blob/dPI_BlobLib.h"
#include "../../AlgorithmModules/dPI_Geometry/dPI_GeometryLib.h"
#include "../../DefineModules/dA_Base/DynamicSystemPath.h"
#include "../../DefineModules/dA_Base/FPI_RECT.h"
#include "../../DefineModules/dA_Base/PI_RECT.h"
#include "../../DefineModules/dA_Base/iDataType.h"
#include "../../ImageCombineModules/dPI_ippModules/ippModules.h"
#include "../../InformationModule/dPI_DataBase/BodyInfo.h"
#include "../../InformationModule/dPI_DataBase/PackageSpec.h"
#include "../../InformationModule/dPI_DataBase/VisionTrayScanSpec.h"
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../../MemoryModules/VisionReusableMemory/VisionReusableMemory.h"
#include "../../VisionNeedLibrary/VisionCommon/UnitAreaInfo.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionImageLot.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionImageLotInsp.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionUnitAgent.h"
#include "../../VisionNeedLibrary/VisionEdgeAlign/Inspection.h"
#include "../../VisionNeedLibrary/VisionEdgeAlign/Result_EdgeAlign.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Algorithm/DataFitting.h>
#include <Ipvm/Algorithm/Geometry.h>
#include <Ipvm/Algorithm/ImageProcessing.h>
#include <Ipvm/Base/Conversion.h>
#include <Ipvm/Base/EllipseEq32r.h>
#include <Ipvm/Base/Enum.h>
#include <Ipvm/Base/Image32r.h>
#include <Ipvm/Base/Image32s.h>
#include <Ipvm/Base/Image64r.h>
#include <Ipvm/Base/Image8u.h>
#include <Ipvm/Base/LineEq32r.h>
#include <Ipvm/Base/LineSeg32r.h>
#include <Ipvm/Base/PlaneEq64r.h>
#include <Ipvm/Base/Point64r2.h>
#include <Ipvm/Base/Point64r3.h>
#include <Ipvm/Base/Quadrangle32r.h>
#include <Ipvm/Base/Size32s2.h>
#include <Ipvm/Base/TimeCheck.h>

//CPP_5_________________________________ Standard library headers
#include <algorithm>

//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//#define USE_SR_CENTER
#define USE_SR_PLANE

//CPP_7_________________________________ Implementation body
//
struct S3DImageData
{
    Ipvm::Image8u m_byHmap;
    Ipvm::Image8u m_byInten;

    Ipvm::Image32r m_zmap;
    Ipvm::Image16u m_vmap;
};

BOOL VisionProcessingAlign3D::OnInspection()
{
    m_pVisionInspDlg->OnBnClickedButtonInspect();
    return TRUE;
}

BOOL VisionProcessingAlign3D::DoInspection(const bool detailSetupMode, const enSideVisionModule i_ModuleStatus)
{
    //mc_Side Vision
    SetCurVisionModule_Status(i_ModuleStatus);
    //

    Ipvm::TimeCheck TimeInspectionTotal;

    ResetResult();
    m_processingState.SetNum(2);

    //{{ //kircheis_3DEmpty //검사 결과를 관리하는 민감한 변수니까 무조건 시작할 때 초기화 한다.
    m_bInvalid = FALSE;
    m_bEmpty = FALSE;
    m_bMarginal = FALSE;
    m_bDoubleDevice = FALSE;
    //}}

    Ipvm::TimeCheck fTime;

    bool success = true;

    if (getImageLot().GetImageFrameCount() < 2)
    {
        m_bInvalid = TRUE;
        return FALSE;
    }

    long paneIndex = GetCurrentPaneID();
    if (paneIndex < 0 || paneIndex >= (long)getInspectionAreaInfo().m_unitIndexList.size()
        || getInspectionAreaInfo().m_fovList.size() < 1)
    {
        // Job이 이상한데

        return FALSE;
    }

    Ipvm::Image8u imageHMap = getImageLot().GetImageFrame(0, enSideVisionModule::SIDE_VISIONMODULE_FRONT);
    Ipvm::Image8u imageIntensity = getImageLot().GetImageFrame(1, enSideVisionModule::SIDE_VISIONMODULE_FRONT);
    Ipvm::Image32r imageZMap = getImageLot().GetZmapImage(0);
    Ipvm::Image16u imageVMap = getImageLot().GetVmapImage(0);

    double paneOffsetX_um = getTrayScanSpec().GetHorPaneOffset_um();
    double paneOffsetY_um = getTrayScanSpec().GetVerPaneOffset_um();

    if (m_VisionPara.m_eRoughAlignUsingDefine != eRoughAlignUsingDefine::eRoughAlignUsingDefine_NOT_USE
        && getInspectionAreaInfo().m_stichCountY > 1)
    {
        calcAutoPaneOffsetX(m_VisionPara.m_eRoughAlignUsingDefine, paneOffsetX_um);
        calcAutoPaneOffsetY(m_VisionPara.m_eRoughAlignUsingDefine, paneOffsetY_um);
    }

    m_result->Prepare(*this, paneOffsetX_um, paneOffsetY_um);

    Ipvm::Rect32s edgeAlignRegion(0, 0, 0, 0);
    Ipvm::Point32r2 alignOffsetPlus(0.f, 0.f);

    Ipvm::Point32r2 paneCenterInEdgeAlignImage;

    if (getInspectionAreaInfo().m_stichCountX > 1 || getInspectionAreaInfo().m_stichCountY > 1)
    {
        Ipvm::TimeCheck TimeStitchingAlign;
        success = DoAlign(detailSetupMode, imageHMap, imageIntensity, imageZMap, imageVMap);
        AddProcessingFunctionExcuteTimeLog(
            m_strModuleName, _T("StitchingAlignTime"), CAST_FLOAT(TimeStitchingAlign.Elapsed_ms()));

        // Debug Info...
        SetDebugInfo(detailSetupMode);

        auto& deviceRoi = m_3DMergeResult->vecrtDevice[paneIndex];

        Ipvm::Point32r2 deviceCenter(
            (deviceRoi.m_left + deviceRoi.m_right) * 0.5f, (deviceRoi.m_top + deviceRoi.m_bottom) * 0.5f);

        Ipvm::Rect32s roiForCutting;
        auto imageRoi = Ipvm::Rect32s(imageHMap);

        roiForCutting.m_left = long(deviceCenter.m_x - getReusableMemory().GetInspImageSizeX() / 2);
        roiForCutting.m_top = long(deviceCenter.m_y - getReusableMemory().GetInspImageSizeY() / 2);
        roiForCutting.m_right = roiForCutting.m_left + getReusableMemory().GetInspImageSizeX();
        roiForCutting.m_bottom = roiForCutting.m_top + getReusableMemory().GetInspImageSizeY();

        auto centerForEdgeAlign = Ipvm::Point32r2(roiForCutting.Width() * 0.5f, roiForCutting.Height() * 0.5f);

        // 실제 화면에서 찾은 Device Center가 얼마나 틀어졌는지 계산하고
        // 후에 이미지가 얼마나 틀어졌는지 옵셋을 계산하여 전달할 때 더해 준다.
        // Stitch는 첫 FOV에 맞추므로 첫 FOV 기준으로 Unit의 중심이 어디에 있나 보자

        Ipvm::Point32r2 unitCenter = getUnitAreaInfo().GetUnitCenterBasedOnFirstImageFOV();

        alignOffsetPlus.m_x = deviceCenter.m_x - unitCenter.m_x;
        alignOffsetPlus.m_y = deviceCenter.m_y - unitCenter.m_y;

        paneCenterInEdgeAlignImage = centerForEdgeAlign;

        edgeAlignRegion = roiForCutting;

        if (!m_VisionPara.m_skipEdgeAlign)
        {
            // Edge Align을 위해 Device 이미지를 한가운데로 이동시킨다
            Ipvm::Image8u imageForEdgeAlign;
            if (!getReusableMemory().GetByteImage(imageForEdgeAlign, roiForCutting.Width(), roiForCutting.Height()))
            {
                return false;
            }

            Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(imageForEdgeAlign), 0, imageForEdgeAlign);
            CippModules::CopyValid(imageHMap, roiForCutting.TopLeft(), Ipvm::Point32s2(0, 0),
                Ipvm::Conversion::ToSize32s2(roiForCutting), imageForEdgeAlign);
            m_edgeAlign_inspection->getResult_EdgeAlign()->SetImage(imageForEdgeAlign, paneCenterInEdgeAlignImage);
        }
    }
    else
    {
        Ipvm::Point32r2 imageCenter(getImageLot().GetImageSizeX() * 0.5f, getImageLot().GetImageSizeY() * 0.5f);

        long fovIndex = getInspectionAreaInfo().m_fovList[0];
        long unitID = getInspectionAreaInfo().m_unitIndexList[paneIndex];
        paneCenterInEdgeAlignImage = Ipvm::Conversion::ToPoint32r2(getScale().convert_mmToPixel(
                                         getTrayScanSpec().GetUnitCenterInFOV(fovIndex, unitID)))
            + imageCenter;

        if (!m_VisionPara.m_skipEdgeAlign)
        {
            Ipvm::Image8u image;
            edgeAlignRegion = m_edgeAlign_inspection->getImageSourceRoiForNoStitch();
            if (!m_edgeAlign_inspection->getImageForAlign(*m_edgeAlign_para, false, image))
            {
                success = false;
            }
            else
            {
                imageCenter = Ipvm::Point32r2(image.GetSizeX() * 0.5f, image.GetSizeY() * 0.5f);
                m_edgeAlign_inspection->getResult_EdgeAlign()->SetImage(image, imageCenter);
            }
        }
    }

    m_processingState.SetState(0,
        success ? VisionProcessingState::enumState::Active_Success : VisionProcessingState::enumState::Active_Invalid);

    if (success)
    {
        if (!m_VisionPara.m_skipEdgeAlign)
        {
            Ipvm::TimeCheck TimeEdgeAlign;
            if (!m_edgeAlign_inspection->run(*m_edgeAlign_para, detailSetupMode))
            {
                success = false;
            }

            AddProcessingFunctionExcuteTimeLog(
                m_strModuleName, _T("EdgeAlignTime"), CAST_FLOAT(TimeEdgeAlign.Elapsed_ms()));

            if (m_edgeAlign_inspection->getResult_EdgeAlign()->m_inspectioned)
            {
                if (m_edgeAlign_inspection->getResult_EdgeAlign()->m_success)
                {
                    m_processingState.SetState(1, VisionProcessingState::enumState::Active_Success);
                }
                else
                {
                    m_processingState.SetState(1, VisionProcessingState::enumState::Active_Invalid);
                }
            }
        }
        else
        {
            // Edge Align 결과로 보정되는 것은 전혀없는 것으로 만들어 주자
            auto* alignResult = m_edgeAlign_inspection->getResult_EdgeAlign();

            alignResult->Init(paneCenterInEdgeAlignImage);
        }
    }

    int inspImageSizeX = getReusableMemory().GetInspImageSizeX();
    int inspImageSizeY = getReusableMemory().GetInspImageSizeY();

    auto& dstZmap = getImageLotInsp().m_zmapImage;
    auto& dstVmap = getImageLotInsp().m_vmapImage;

    for (auto& image : getImageLotInsp().m_vecImages[GetCurVisionModule_Status()])
    {
        image.Create(inspImageSizeX, inspImageSizeY);
    }

    dstZmap.Free();
    dstZmap.Create(inspImageSizeX, inspImageSizeY);
    Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(dstZmap), Ipvm::k_noiseValue32r, dstZmap);

    dstVmap.Free();
    dstVmap.Create(inspImageSizeX, inspImageSizeY);
    dstVmap.FillZero();

    if (!success)
    {
        // 실패했을 때 쓰레기 이미지를 클리어해서 아무것도 보여주지 말자
        for (auto& image : getImageLotInsp().m_vecImages[GetCurVisionModule_Status()])
        {
            image.FillZero();
        }

        m_edgeAlign_inspection->setGlobalAlignResult();

        // 결과 표시
        m_fCalcTime = CAST_FLOAT(fTime.Elapsed_ms());

        return false;
    }

    auto* alignResult = m_edgeAlign_inspection->getResult_EdgeAlign();

    // Zmap / Vmap 을 회전해서 다시 만들자

    auto edgeAlignImageLT = edgeAlignRegion.TopLeft();

    Ipvm::Point32r2 origin(
        alignResult->m_center.m_x + edgeAlignImageLT.m_x, alignResult->m_center.m_y + edgeAlignImageLT.m_y);
    Ipvm::Point32r2 shift(inspImageSizeX * 0.5f - origin.m_x, inspImageSizeY * 0.5f - origin.m_y);

    if (Ipvm::ImageProcessing::RotateNearestInterpolation(
            imageZMap, edgeAlignRegion, origin, -alignResult->m_angle_rad * ITP_RAD_TO_DEG, shift, dstZmap)
        != Ipvm::Status::e_ok)
    {
        return FALSE;
    }

    if (Ipvm::ImageProcessing::RotateNearestInterpolation(
            imageVMap, edgeAlignRegion, origin, -alignResult->m_angle_rad * ITP_RAD_TO_DEG, shift, dstVmap)
        != Ipvm::Status::e_ok)
    {
        return FALSE;
    }

    const auto& imageInfo = getImageLot().GetInfo();

    Ipvm::Rect32s imageRoi(0, 0, inspImageSizeX, inspImageSizeY);
    Ipvm::ImageProcessing::ScaleInRangeWithNoise(dstZmap, Ipvm::Rect32s(dstZmap), Ipvm::k_noiseValue32r,
        imageInfo.m_heightRangeMin, imageInfo.m_heightRangeMax, 0, 1, 255,
        getImageLotInsp().m_vecImages[GetCurVisionModule_Status()][0]);
    Ipvm::ImageProcessing::Scale(
        dstVmap, Ipvm::Rect32s(dstVmap), getImageLotInsp().m_vecImages[GetCurVisionModule_Status()][1]);

    Ipvm::Point32r2 shift2(
        inspImageSizeX * 0.5f - alignResult->m_center.m_x, inspImageSizeY * 0.5f - alignResult->m_center.m_y);

    // Stitch에서 보상한 Package Offset을 적용하여 준다
    m_edgeAlign_inspection->getResult_EdgeAlign()->m_analysis_packageOffsetX_um
        += getScale().convert_pixelToUmX(alignOffsetPlus.m_x);
    m_edgeAlign_inspection->getResult_EdgeAlign()->m_analysis_packageOffsetY_um
        += getScale().convert_pixelToUmY(alignOffsetPlus.m_y);

    m_edgeAlign_inspection->setGlobalAlignResult_ZeroAngle(shift2);

    getImageLotInsp().m_paneIndexForCalculationImage = GetCurrentPaneID();

    if (SystemConfig::GetInstance().m_bIsAnalysisResult)
    {
        long nDataNum = 0;
        auto* pData = m_visionUnit.GetVisionDebugInfo(
            m_visionUnit.GetVisionAlignProcessingModuleGUID(), _T("EDGE Align Result"), nDataNum);

        if (pData != nullptr && nDataNum > 0)
        {
            auto* edgeAlignResult = (VisionAlignResult*)pData;
            long h_count = 1000;
            long h_c = h_count / 2;

            edgeAlignResult->m_analysis_histogramZ.resize(h_count);

            auto bodyRect = edgeAlignResult->getBodyRect32s();

            for (long y = bodyRect.m_top; y < bodyRect.m_bottom; y++)
            {
                auto* zmap_y = dstZmap.GetMem(0, y);
                for (long x = bodyRect.m_left; x < bodyRect.m_right; x++)
                {
                    float z = zmap_y[x];
                    if (z == Ipvm::k_noiseValue32r)
                        continue;

                    long z_index = CAST_LONG(z / 10.f) + h_c; // 10um 단위로 저장
                    if (z_index < 0 || z_index >= h_count)
                    {
                        continue;
                    }

                    edgeAlignResult->m_analysis_histogramZ[z_index]++;
                }
            }
        }
    }

    // 결과 표시
    m_fCalcTime = CAST_FLOAT(fTime.Elapsed_ms());

    AddProcessingFunctionExcuteTimeLog(
        m_strModuleName, _T("InspTotalTime"), CAST_FLOAT(TimeInspectionTotal.Elapsed_ms()));

    return TRUE;
}

void VisionProcessingAlign3D::SetDebugInfo(const bool detailSetupMode)
{
    SetDebugInfoItem(detailSetupMode, _T("Raw Package Rect"), m_3DMergeResult->vecsfrtRawDevice);
    SetDebugInfoItem(detailSetupMode, _T("Rough Matching Line"), m_3DMergeResult->vecsRoughMatchLine);
    SetDebugInfoItem(detailSetupMode, _T("2D Matching ROI"), m_3DMergeResult->vecrt2DMatch);
    SetDebugInfoItem(detailSetupMode, _T("2D Matching Line"), m_3DMergeResult->vecsSplitCrossLine);
    SetDebugInfoItem(detailSetupMode, _T("Overlap ROI"), m_3DMergeResult->vecrtOverlap);
    SetDebugInfoItem(detailSetupMode, _T("Overlap Matching ROI"), m_3DMergeResult->vecrtMatchOverlap);
    SetDebugInfoItem(detailSetupMode, _T("Spec Object ROI"), m_3DMergeResult->vecrtSpecObj);
    SetDebugInfoItem(detailSetupMode, _T("Search Object ROI"), m_3DMergeResult->vecrtSearchObj);
    SetDebugInfoItem(detailSetupMode, _T("Object ROI"), m_3DMergeResult->vecrtObj);
    SetDebugInfoItem(detailSetupMode, _T("Object Center"), m_3DMergeResult->vecfptObj);
    SetDebugInfoItem(detailSetupMode, _T("Measure ROI"), m_3DMergeResult->vecrtMeasure);
    SetDebugInfoItem(detailSetupMode, _T("Cross Line"), m_3DMergeResult->vecsCrossLine);
    SetDebugInfoItem(detailSetupMode, _T("Pane ROI"), m_3DMergeResult->vecrtPane);
    SetDebugInfoItem(detailSetupMode, _T("Device Rect"), m_3DMergeResult->vecrtDevice, true);
}

BOOL VisionProcessingAlign3D::MakeFixMeasureROI(
    Ipvm::Rect32s rtMatchOverlap, std::vector<Ipvm::Point32r2>& o_vecfptObjCenter, long nNumX, long nNumY)
{
    o_vecfptObjCenter.clear();

    long nWidth = rtMatchOverlap.Width();
    long nHeight = rtMatchOverlap.Height();

    if (nNumY <= 1 || nNumX <= 1)
        return FALSE;

    long nStepX = nWidth / (nNumX - 1);
    long nStepY = nHeight / (nNumY - 1);

    for (long i = 0; i < nNumY; i++)
    {
        for (long j = 0; j < nNumX; j++)
        {
            Ipvm::Point32r2 pt;
            pt.m_x = (float)(rtMatchOverlap.m_left + (j * nStepX));
            pt.m_y = (float)(rtMatchOverlap.m_top + (i * nStepY));
            o_vecfptObjCenter.push_back(pt);
        }
    }

    return TRUE;
}

BOOL VisionProcessingAlign3D::SplitPackageAlign(
    const Ipvm::Image8u& image, long stitchCount, FPI_RECT& o_sfrtRawDevice, std::vector<Ipvm::Rect32s>& o_vecrtOverlap)
{
    UNREFERENCED_PARAMETER(image);

    o_vecrtOverlap.clear();

    //const auto& mmToPixel = getScale().mmToPixel();

    int lastFovIndex = (int)m_result->m_fovs.size() - 1;

    o_sfrtRawDevice.fptLT = m_result->m_fovs[0].m_deviceImageRoi.TopLeft();
    o_sfrtRawDevice.fptRT = m_result->m_fovs[0].m_deviceImageRoi.TopRight();
    o_sfrtRawDevice.fptLB = m_result->m_fovs[lastFovIndex].m_deviceImageRoi.BottomLeft()
        + Ipvm::Conversion::ToPoint32r2(m_result->m_fovs[lastFovIndex].m_imageRoi.TopLeft());
    o_sfrtRawDevice.fptRB = m_result->m_fovs[lastFovIndex].m_deviceImageRoi.BottomRight()
        + Ipvm::Conversion::ToPoint32r2(m_result->m_fovs[lastFovIndex].m_imageRoi.TopLeft());

    o_vecrtOverlap.clear();

    for (int fovIndex = 0; fovIndex < (int)m_result->m_fovs.size(); fovIndex++)
    {
        auto curRegion = m_result->m_fovs[fovIndex].m_validUnitRoi_mm;

        if (fovIndex > 0)
        {
            auto targetFovIndex = fovIndex - 1;
            auto targetRegion = m_result->m_fovs[targetFovIndex].m_validUnitRoi_mm;
            auto overlapRoi = curRegion & targetRegion;

            if (!overlapRoi.IsRectEmpty())
            {
                overlapRoi -= m_result->m_fovs[fovIndex].m_fovRoi_mm.CenterPoint();
                overlapRoi = overlapRoi * getScale().mmToPixel();
                overlapRoi
                    += Ipvm::Point32r2(m_result->m_fovImageSizeX * 0.5f, m_result->m_fovImageSizeY * (fovIndex + 0.5f));

                o_vecrtOverlap.push_back(Ipvm::Conversion::ToRect32s(overlapRoi));
            }
        }

        if (fovIndex < stitchCount - 1)
        {
            auto targetFovIndex = fovIndex + 1;
            auto targetRegion = m_result->m_fovs[targetFovIndex].m_validUnitRoi_mm;
            auto overlapRoi = curRegion & targetRegion;

            if (!overlapRoi.IsRectEmpty())
            {
                overlapRoi -= m_result->m_fovs[fovIndex].m_fovRoi_mm.CenterPoint();
                overlapRoi = overlapRoi * getScale().mmToPixel();
                overlapRoi
                    += Ipvm::Point32r2(m_result->m_fovImageSizeX * 0.5f, m_result->m_fovImageSizeY * (fovIndex + 0.5f));

                o_vecrtOverlap.push_back(Ipvm::Conversion::ToRect32s(overlapRoi));
            }
        }
    }

    return TRUE;
}

BOOL VisionProcessingAlign3D::CalcMatchOverlapROI(S3DImageData& split3DImage, Ipvm::Rect32s rtTopOverlap,
    Ipvm::Rect32s rtBottomOverlap, std::vector<Ipvm::Rect32s>& vecrtTempl, std::vector<Ipvm::Rect32s>& vecrtSearch,
    std::vector<Ipvm::Rect32s>& o_vecrtMatchOverlap, std::vector<Ipvm::LineSeg32r>& o_vecsMatchingLine)
{
    o_vecrtMatchOverlap.clear();
    o_vecsMatchingLine.clear();

    o_vecrtMatchOverlap.resize(2);
    o_vecsMatchingLine.resize(2);

    const auto& mmToPixel = getScale().mmToPixel();

    auto byInten = split3DImage.m_byInten;

    Ipvm::Rect32s rtImage = Ipvm::Rect32s(byInten);

    vecrtSearch.clear();

    //BOOL bModyfyGapY = FALSE;
    float lfLimitMatchScore = 1.f;
    float fTemplSizeMM_X = m_VisionPara.m_fPatternSizeX;
    float fTemplSizeMM_Y = m_VisionPara.m_fPatternSizeY;

    long nMatchingPosOffsetY = (long)(m_VisionPara.m_fMatchingPosOffset * mmToPixel.m_y + .5f);
    long nTemplSizeX = static_cast<long>(fTemplSizeMM_X * mmToPixel.m_x + .5f);
    long nTemplSizeY = static_cast<long>(fTemplSizeMM_Y * mmToPixel.m_y + .5f);
    float fTemplShiftX = 0.3f;
    //float fTemplShiftY = -0.3f; // 외곽라인의 정보를 포함하지 말자.
    long nTemplShitX = static_cast<long>(fTemplShiftX * mmToPixel.m_x + .5f);
    float fHardwareLimitX = m_VisionPara.m_fMovingErrorX * mmToPixel.m_x;
    float fHardwareLimitY = m_VisionPara.m_fMovingErrorY * mmToPixel.m_y;

    Ipvm::Point32r2 ptRef[2]{};
    Ipvm::Point32r2 ptMatch[2]{};
    ptMatch[0] = Ipvm::Point32r2(0.f, 0.f);
    ptMatch[1] = Ipvm::Point32r2(0.f, 0.f);

    for (long dir = 0; dir < 2; dir++)
    {
        Ipvm::Point32s2 ptSearch{};
        Ipvm::Point32s2 ptTemplate{};

        switch (dir)
        {
            case 0:
                // Left 영역 매칭
                ptSearch = Ipvm::Point32s2(rtTopOverlap.m_left, rtTopOverlap.CenterPoint().m_y + nMatchingPosOffsetY);
                ptTemplate = Ipvm::Point32s2(rtBottomOverlap.m_left, rtBottomOverlap.CenterPoint().m_y);
                break;

            case 1:
                // Right 영역 매칭
                ptSearch = Ipvm::Point32s2(
                    rtTopOverlap.m_right - nTemplSizeX, rtTopOverlap.CenterPoint().m_y + nMatchingPosOffsetY);
                ptTemplate = Ipvm::Point32s2(rtBottomOverlap.m_right - nTemplSizeX, rtBottomOverlap.CenterPoint().m_y);
                break;
            default:
                return FALSE;
        }

        // 좌/우 매칭
        Ipvm::Rect32s rtTemplate;
        rtTemplate.m_left = ptTemplate.m_x;
        rtTemplate.m_right = rtTemplate.m_left + nTemplSizeX;
        rtTemplate.m_top = (long)((float)ptTemplate.m_y - (float)nTemplSizeY / 2.f + .5f);
        rtTemplate.m_bottom = rtTemplate.m_top + nTemplSizeY;
        rtTemplate &= rtImage;

        vecrtTempl.push_back(rtTemplate);

        ptRef[dir] = Ipvm::Conversion::ToPoint32r2(Ipvm::Point32s2(rtTemplate.m_left, rtTemplate.m_top));

        float minMatchingScore = FLT_MAX;

        Ipvm::Rect32s rtSearch;
        rtSearch.m_left = static_cast<long>(ptSearch.m_x);
        rtSearch.m_right = static_cast<long>(ptSearch.m_x + nTemplSizeX);
        rtSearch.m_top = static_cast<long>(ptSearch.m_y - (float)nTemplSizeY / 2.f + .5f);
        rtSearch.m_bottom = static_cast<long>(ptSearch.m_y + (float)nTemplSizeY / 2.f + .5f);
        rtSearch -= Ipvm::Point32s2(nTemplShitX, 0);
        rtSearch += Ipvm::Point32s2(0, nMatchingPosOffsetY);
        rtSearch.InflateRect(long(fHardwareLimitX / 2.f), long(fHardwareLimitY / 2.f));
        rtSearch &= rtImage;

        vecrtSearch.push_back(rtSearch);

        // Inten Map이 매칭률이 더 안 좋더라도 매칭 결과가 더 좋다.
        Ipvm::Image8u byCalc = byInten;
        if (1)
        {
            Ipvm::Image8u imgTemplate(byCalc, rtTemplate);

            Ipvm::Point32s2 curMatchingPos;
            float lfMatch = 0.f;
            if (Ipvm::ImageProcessing::SearchTemplate(byCalc, imgTemplate, rtSearch, curMatchingPos, lfMatch)
                != Ipvm::Status::e_ok)
            {
                return FALSE;
            }

            if (lfMatch > lfLimitMatchScore)
            {
                return FALSE;
            }

            if (lfMatch < minMatchingScore)
            {
                minMatchingScore = lfMatch;
                ptMatch[dir] = Ipvm::Conversion::ToPoint32r2(curMatchingPos);
            }
        }
    }

    // 2D 틀어짐 계산.
    //float fDiffX = (float)(ptMatch[1].m_x - ptMatch[0].m_x);
    //float fDiffY = (float)(ptMatch[1].m_y - ptMatch[0].m_y);
    //float fRadian = atan2(fDiffY, fDiffX);

    long nReduceOverlapX = (long)(m_VisionPara.m_fMatchingReduceSizeX / 2.f * mmToPixel.m_x + .5f);

    Ipvm::Rect32s rtTopLeftMatch = Ipvm::Rect32s(
        (int)ptMatch[0].m_x, rtTopOverlap.m_top, (int)(ptMatch[0].m_x + nTemplSizeX), rtTopOverlap.m_bottom);
    Ipvm::Rect32s rtTopRightMatch = Ipvm::Rect32s(
        (int)ptMatch[1].m_x, rtTopOverlap.m_top, (int)(ptMatch[1].m_x + nTemplSizeX), rtTopOverlap.m_bottom);

    Ipvm::Rect32s rtBottomLeftMatch = Ipvm::Rect32s(
        (int)ptRef[0].m_x, rtBottomOverlap.m_top, (int)(ptRef[0].m_x + nTemplSizeX), rtBottomOverlap.m_bottom);
    Ipvm::Rect32s rtBottomRightMatch = Ipvm::Rect32s(
        (int)ptRef[1].m_x, rtBottomOverlap.m_top, (int)(ptRef[1].m_x + nTemplSizeX), rtBottomOverlap.m_bottom);

    Ipvm::Rect32s overlapRois[2];
    overlapRois[UP] = rtTopLeftMatch | rtTopRightMatch;
    overlapRois[DOWN] = rtBottomLeftMatch | rtBottomRightMatch;

    long overlapCenterYs[2];
    overlapCenterYs[UP] = overlapRois[UP].CenterPoint().m_y;
    overlapCenterYs[DOWN] = overlapRois[DOWN].CenterPoint().m_y;
    long nHalfMatchSizeY = static_cast<long>(m_VisionPara.m_fMatchingSize / 2.f * mmToPixel.m_y + .5f);

    // Overlap Roi 가 Fov 화면을 벗어나서 만들어지면 안된다.
    // Match Size Y를 설정하였더라도 Overlap Roi가 화면을 벗어나지 않는 한도까지 크기를 줄인다

    for (int direction = 0; direction < 2; direction++)
    {
        for (auto& fov : m_result->m_fovs)
        {
            int dist1 = overlapCenterYs[direction] - fov.m_imageRoi.m_top;
            int dist2 = fov.m_imageRoi.m_bottom - overlapCenterYs[direction];

            if (dist1 >= 0)
            {
                nHalfMatchSizeY = min(nHalfMatchSizeY, dist1);
            }

            if (dist2 >= 0)
            {
                nHalfMatchSizeY = min(nHalfMatchSizeY, dist2);
            }
        }
    }

    for (int direction = 0; direction < 2; direction++)
    {
        overlapRois[direction].m_top = overlapCenterYs[direction] - nHalfMatchSizeY;
        overlapRois[direction].m_bottom = overlapCenterYs[direction] + nHalfMatchSizeY;
        overlapRois[direction].m_left += nReduceOverlapX;
        overlapRois[direction].m_right -= nReduceOverlapX;
        overlapRois[direction] &= rtImage;
    }

    o_vecrtMatchOverlap[0] = overlapRois[UP];
    o_vecrtMatchOverlap[1] = overlapRois[DOWN];

    // Top Matching Line
    o_vecsMatchingLine[0]
        = Ipvm::LineSeg32r((float)ptMatch[0].m_x, (float)ptMatch[0].m_y, (float)ptMatch[1].m_x, (float)ptMatch[1].m_y);

    // Bottom Matching Line
    o_vecsMatchingLine[1]
        = Ipvm::LineSeg32r((float)ptRef[0].m_x, (float)ptRef[0].m_y, (float)ptRef[1].m_x, (float)ptRef[1].m_y);

    return TRUE;
}

BOOL VisionProcessingAlign3D::StitchingImage(S3DImageData& split3DImage, const Ipvm::LineSeg32r& sTopMatchLineSeg,
    const Ipvm::LineSeg32r& sBottomMatchLineSeg, Ipvm::Rect32s rtRotate, const Ipvm::Point64r3& s3DTheta,
    const Ipvm::Point64r3& s3DTopOrigin_um, const Ipvm::Point64r3& s3DBottomOrigin_um, S3DImageData& ref3DImage,
    std::vector<Ipvm::Point64r3>& o_vecConfirmBottom3Ddata)
{
    o_vecConfirmBottom3Ddata.clear();

    // 임시 : 회전 영역.
    // 속도를 높이려면 Rotate시 Offset를 적용하는게.. 오차가 조금 더 발생하겠지만...

    Ipvm::Image32r tempRotate;
    int32_t rawImageSizeX = getImageLot().GetImageSizeX();
    int32_t rawImageSizeY = getImageLot().GetImageSizeY();

    if (!getReusableMemory().GetFloatImage(tempRotate, rawImageSizeX, rawImageSizeY))
    {
        return FALSE;
    }

    const double lfOffsetZ_um = s3DTopOrigin_um.m_z - s3DBottomOrigin_um.m_z;

    Ipvm::Image32r splitZmap = split3DImage.m_zmap;

    // Bottom 에서 Top 으로의 변환이므로 각도는 반대로..
    if (!Rotate3DImage(splitZmap, rtRotate, s3DBottomOrigin_um, -s3DTheta.m_x, -s3DTheta.m_y, -s3DTheta.m_z,
            (float)lfOffsetZ_um, tempRotate))
    {
        return FALSE;
    }

    Ipvm::Image8u dstHmap = ref3DImage.m_byHmap;

    Ipvm::LineEq32r sBottomMatchLineEq(0.f, 0.f, 0.f);
    if (CPI_Geometry::mMakeLineByTwoPoints(sBottomMatchLineEq, sBottomMatchLineSeg.m_sx, sBottomMatchLineSeg.m_sy,
            sBottomMatchLineSeg.m_ex, sBottomMatchLineSeg.m_ey)
        < 0)
    {
        return FALSE;
    }

    Ipvm::LineEq32r sTopMatchLineEq(0.f, 0.f, 0.f);
    if (CPI_Geometry::mMakeLineByTwoPoints(
            sTopMatchLineEq, sTopMatchLineSeg.m_sx, sTopMatchLineSeg.m_sy, sTopMatchLineSeg.m_ex, sTopMatchLineSeg.m_ey)
        < 0)
    {
        return FALSE;
    }

    const long imageSizeX = split3DImage.m_vmap.GetSizeX();
    const long imageSizeY = split3DImage.m_vmap.GetSizeY();

//#define PASTE_TEST
#ifdef PASTE_TEST
    const float um2px_x = getScale().pixelToUm().m_x;
    const float um2px_y = getScale().pixelToUm().m_y;

    const Ipvm::Point32r2 topOrigin_px = {float(s3DTopOrigin_um.m_x * um2px_x), float(s3DTopOrigin_um.m_y * um2px_y)};
    const Ipvm::Point32r2 bottomOrigin_px
        = {float(s3DBottomOrigin_um.m_x * um2px_x), float(s3DBottomOrigin_um.m_y * um2px_y)};

    Ipvm::Rect32s rtSrcROI(rtRotate);
    rtSrcROI.m_top = sBottomMatchLineEq.GetPositionY(rtRotate.m_left);

    Ipvm::Rect32s rtDstROI(rtSrcROI);
    rtDstROI.OffsetRect(
        long(topOrigin_px.m_x - bottomOrigin_px.m_x + 0.5f), long(topOrigin_px.m_y - bottomOrigin_px.m_y + 0.5f));

    Ipvm::ImageProcessing::Copy(tempRotate, rtSrcROI, rtDstROI, ref3DImage.m_zmap);
    Ipvm::ImageProcessing::Copy(split3DImage.m_vmap, rtSrcROI, rtDstROI, ref3DImage.m_vmap);

#else
    // Copy Bottom Device
    int nOffsetLeftX = (int)(sTopMatchLineSeg.m_sx - sBottomMatchLineSeg.m_sx);
    int nOffsetRightX = (int)(sTopMatchLineSeg.m_ex - sBottomMatchLineSeg.m_ex);
    int nOffsetX = (int)((float)(nOffsetLeftX + nOffsetRightX) / 2.f);

    for (int x = rtRotate.m_left; x < rtRotate.m_right; x++)
    {
        int nSrcy = (int)sBottomMatchLineEq.GetPositionY((float)x);
        int nOffsetDstX = x + nOffsetX;

        int nDsty = (int)sTopMatchLineEq.GetPositionY((float)nOffsetDstX);

        if (nSrcy <= 0 || nSrcy >= imageSizeY)
            continue;
        if (nDsty <= 0 || nDsty >= imageSizeY)
            continue;

        if (nOffsetDstX <= 0)
            continue;
        if (nOffsetDstX >= imageSizeX)
            break;

        Ipvm::Rect32s rtSrcROI(x, nSrcy, x + 1, rtRotate.m_bottom);
        Ipvm::Rect32s rtDstROI(
            (int32_t)nOffsetDstX, (int32_t)nDsty, (int32_t)nOffsetDstX + 1, (int32_t)nDsty + rtSrcROI.Height());

        const long roiHeight = rtSrcROI.Height();

        for (long y = 0; y < roiHeight; y++)
        {
            const auto& srcZValue = tempRotate[y + nSrcy][x];
            const auto& srcVValue = split3DImage.m_vmap[y + nSrcy][x];

            auto& dstZValue = ref3DImage.m_zmap[y + nDsty][nOffsetDstX];
            auto& dstVValue = ref3DImage.m_vmap[y + nDsty][nOffsetDstX];

            if (srcZValue != Ipvm::k_noiseValue32r && dstZValue != Ipvm::k_noiseValue32r && srcVValue != 0)
            {
                dstZValue = (srcZValue * srcVValue + dstZValue * dstVValue) / (srcVValue + dstVValue);
                dstVValue = (srcVValue + dstVValue) / 2;
            }
            else if (srcZValue != Ipvm::k_noiseValue32r)
            {
                dstZValue = srcZValue;
                dstVValue = srcVValue;
            }
        }

        //Ipvm::ImageProcessing::Copy(tempRotate, rtSrcROI, rtDstROI, ref3DImage.m_zmap);
        //Ipvm::ImageProcessing::Copy(split3DImage.m_vmap, rtSrcROI, rtDstROI, ref3DImage.m_vmap);
    }
#endif

    return TRUE;
}

BOOL VisionProcessingAlign3D::Stitching2DImage(S3DImageData& split3DImage, const Ipvm::LineSeg32r& sTopMatchLineSeg,
    const Ipvm::LineSeg32r& sBottomMatchLineSeg, Ipvm::Rect32s rtCalc, S3DImageData& ref3DImage)
{
    Ipvm::LineEq32r sBottomMatchLineEq(0.f, 0.f, 0.f);
    if (CPI_Geometry::mMakeLineByTwoPoints(sBottomMatchLineEq, sBottomMatchLineSeg.m_sx, sBottomMatchLineSeg.m_sy,
            sBottomMatchLineSeg.m_ex, sBottomMatchLineSeg.m_ey)
        < 0)
    {
        return FALSE;
    }

    Ipvm::LineEq32r sTopMatchLineEq(0.f, 0.f, 0.f);
    if (CPI_Geometry::mMakeLineByTwoPoints(
            sTopMatchLineEq, sTopMatchLineSeg.m_sx, sTopMatchLineSeg.m_sy, sTopMatchLineSeg.m_ex, sTopMatchLineSeg.m_ey)
        < 0)
    {
        return FALSE;
    }

    // Copy Right Device
    int nOffsetLeftX = (int)(sTopMatchLineSeg.m_sx - sBottomMatchLineSeg.m_sx);
    int nOffsetRightX = (int)(sTopMatchLineSeg.m_ex - sBottomMatchLineSeg.m_ex);
    int nOffsetX = (int)((float)(nOffsetLeftX + nOffsetRightX) / 2.f);

    for (int x = rtCalc.m_left; x < rtCalc.m_right; x++)
    {
        int nSrcy = (int)sBottomMatchLineEq.GetPositionY((float)x);
        int nOffsetDstX = x + nOffsetX;
        int nDsty = (int)sTopMatchLineEq.GetPositionY((float)nOffsetDstX);

        Ipvm::Rect32s rtSrcROI(x, nSrcy, x + 1, rtCalc.m_bottom);
        Ipvm::Rect32s rtDstROI(
            (int32_t)nOffsetDstX, (int32_t)nDsty, (int32_t)nOffsetDstX + 1, (int32_t)nDsty + rtSrcROI.Height());

        Ipvm::ImageProcessing::Copy(split3DImage.m_zmap, rtSrcROI, rtDstROI, ref3DImage.m_zmap);
        Ipvm::ImageProcessing::Copy(split3DImage.m_vmap, rtSrcROI, rtDstROI, ref3DImage.m_vmap);
    }

    return TRUE;
}

BOOL VisionProcessingAlign3D::GetPlaneData_byTop(const S3DImageData& split3DImage,
    const std::vector<Ipvm::Point32r2>& vecfptObjCenter, const std::vector<long>& vecnObjectLayer,
    std::vector<Ipvm::Rect32s>& o_vecrtMeasure, std::vector<Ipvm::Rect32s>& o_vecrtMeasured,
    std::vector<Ipvm::Point64r3>& o_vec3Ddata, Ipvm::PlaneEq64r& o_sPlaneEq, Ipvm::Point64r3& o_s3DAverage)
{
    int32_t rawImageSizeX = getImageLot().GetImageSizeX();
    int32_t rawImageSizeY = getImageLot().GetImageSizeY();

    Ipvm::Image8u maskImage;
    if (!getReusableMemory().GetByteImage(maskImage, rawImageSizeX, rawImageSizeY))
        return FALSE;

    if (vecnObjectLayer.size() <= 0)
        return FALSE;

    o_vecrtMeasure.clear();
    o_vecrtMeasured.clear();
    o_vec3Ddata.clear();

    o_s3DAverage.m_x = 0.f;
    o_s3DAverage.m_y = 0.f;
    o_s3DAverage.m_z = 0.f;

    long nObjNum = (long)vecfptObjCenter.size();

    Ipvm::Rect32s imageROI(split3DImage.m_byHmap);

    for (long nObj = 0; nObj < nObjNum; nObj++)
    {
        float fBallZPos = 0.f;
        Ipvm::Point32r2 fptBallCentroid = Ipvm::Point32r2(0.f, 0.f);

        Ipvm::EllipseEq32r ellipse;
        ellipse.m_x = vecfptObjCenter[nObj].m_x;
        ellipse.m_y = vecfptObjCenter[nObj].m_y;
        ellipse.m_xradius = getScale().convert_umToPixelX(m_VisionPara.m_topStichRoiRadiusX_um);
        ellipse.m_yradius = getScale().convert_umToPixelY(m_VisionPara.m_topStichRoiRadiusY_um);

        const Ipvm::Rect32s rtObject(Ipvm::Conversion::ToRect32s(ellipse));

        Ipvm::ImageProcessing::Fill(rtObject, 0, maskImage);
        Ipvm::ImageProcessing::Fill(ellipse, 255, maskImage);

        o_vecrtMeasure.push_back(rtObject);
        if (CalcZPosbyMask(split3DImage.m_zmap, rtObject, fBallZPos, fptBallCentroid, maskImage))
        {
            Ipvm::Point64r3 s3DTop = Ipvm::Point64r3(fptBallCentroid.m_x, fptBallCentroid.m_y, fBallZPos);
            o_s3DAverage.m_x += s3DTop.m_x;
            o_s3DAverage.m_y += s3DTop.m_y;
            o_s3DAverage.m_z += s3DTop.m_z;
            o_vec3Ddata.push_back(s3DTop);
            o_vecrtMeasured.push_back(rtObject);
        }
    }

    long nValidDataNum = (long)o_vec3Ddata.size();
    if (nValidDataNum <= 0)
        return FALSE;

    o_s3DAverage.m_x /= (double)nValidDataNum;
    o_s3DAverage.m_y /= (double)nValidDataNum;
    o_s3DAverage.m_z /= (double)nValidDataNum;

    /*if (Ipvm::DataFitting::FitToPlane((long)o_vec3Ddata.size(), &o_vec3Ddata[0], o_sPlaneEq) != Ipvm::Status::e_ok)
	{
		return FALSE;
	}*/

    o_sPlaneEq.m_a = 0;
    o_sPlaneEq.m_b = 0;
    o_sPlaneEq.m_c = 0;
    o_sPlaneEq.m_d = 0;

    return TRUE;
}

BOOL VisionProcessingAlign3D::Get3DPoints_byBall(const S3DImageData& split3DImage,
    const std::vector<Ipvm::Point32r2>& vecfptObjCenter, std::vector<Ipvm::Rect32s>& o_vecrtMeasure,
    std::vector<Ipvm::Rect32s>& o_vecrtMeasured, std::map<long, BOOL>& o_validObjectIndex,
    std::vector<Ipvm::Point64r3>& o_vec3Ddata_um)
{
    int32_t rawImageSizeX = getImageLot().GetImageSizeX();
    int32_t rawImageSizeY = getImageLot().GetImageSizeY();

    Ipvm::Image8u maskImage;
    if (!getReusableMemory().GetByteImage(maskImage, rawImageSizeX, rawImageSizeY))
        return FALSE;

    o_vecrtMeasure.clear();
    o_vecrtMeasured.clear();
    o_vec3Ddata_um.clear();
    o_validObjectIndex.clear();

    long nObjNum = (long)vecfptObjCenter.size();

    Ipvm::Rect32s imageROI(split3DImage.m_byHmap);

    const auto& px2um = getScale().pixelToUm();

    Ipvm::Point64r3 tempPoint;

#ifdef USE_SR_PLANE
    std::vector<Ipvm::Rect32s> srROIs;
    std::vector<Ipvm::EllipseEq32r> ignoreBallROIs;

    for (long nObj = 0; nObj < nObjNum; nObj++)
    {
        float ignoreBallRatioX = 1.7f;
        float ignoreBallRatioY = 1.1f;

        Ipvm::EllipseEq32r ellipse;
        ellipse.m_x = vecfptObjCenter[nObj].m_x;
        ellipse.m_y = vecfptObjCenter[nObj].m_y;
        ellipse.m_xradius = (float)fabs(m_packageSpec.m_ballMapAll->m_balls[nObj].m_radiusX_px) * ignoreBallRatioX;
        ellipse.m_yradius = (float)fabs(m_packageSpec.m_ballMapAll->m_balls[nObj].m_radiusY_px) * ignoreBallRatioY;

        Ipvm::Rect32s rtSr = Ipvm::Conversion::ToRect32s(ellipse);
        // SR 면적은 Ellipse 영역의 200% 로 고정
        rtSr.InflateRect(long(ellipse.m_xradius + 0.5f), long(ellipse.m_yradius + 0.5f));

        if ((rtSr & imageROI) != rtSr)
        {
            continue;
        }

        srROIs.push_back(rtSr);
        ignoreBallROIs.push_back(ellipse);

        Ipvm::ImageProcessing::Fill(rtSr, 255, maskImage);

        o_vecrtMeasure.push_back(rtSr);
    }

    for (auto& roi : ignoreBallROIs)
        Ipvm::ImageProcessing::Fill(roi, 0, maskImage);

    //maskImage.SaveBmp(_T("C:\\Temp\\mask.bmp"));

    for (long nObj = 0; nObj < long(srROIs.size()); nObj++)
    {
        float srZ = CalcSrZPosbyMask(split3DImage.m_zmap, srROIs[nObj], maskImage);

        if (srZ == Ipvm::k_noiseValue32r)
        {
            continue;
        }

        tempPoint.m_x = ignoreBallROIs[nObj].m_x * px2um.m_x;
        tempPoint.m_y = ignoreBallROIs[nObj].m_y * px2um.m_y;
        tempPoint.m_z = srZ;

        o_vec3Ddata_um.push_back(tempPoint);
        o_vecrtMeasured.push_back(srROIs[nObj]);
        o_validObjectIndex[nObj] = TRUE;
    }
#else
    for (long nObj = 0; nObj < nObjNum; nObj++)
    {
        Ipvm::Point32r2 fptBallCentroid = Ipvm::Point32r2(0.f, 0.f);
        float fBallCalcAreaRatio = 0.2f; // 중심의 20%에서 산출.

        Ipvm::EllipseEq32r ellipse;
        ellipse.m_x = vecfptObjCenter[nObj].m_x;
        ellipse.m_y = vecfptObjCenter[nObj].m_y;
        ellipse.m_xradius = (float)fabs(m_packageSpec.m_ballMapAll.m_balls[nObj].m_radiusX_px);
        ellipse.m_yradius = (float)fabs(m_packageSpec.m_ballMapAll.m_balls[nObj].m_radiusY_px);

        Ipvm::Rect32s rtBall(ellipse);
        rtBall.InflateRect(1, 1);

        if ((rtBall & imageROI) != rtBall)
        {
            continue;
        }

        Ipvm::ImageProcessing::Fill(rtBall, 0, maskImage);

        ellipse.m_xradius = ellipse.m_xradius * fBallCalcAreaRatio;
        ellipse.m_yradius = ellipse.m_yradius * fBallCalcAreaRatio;

        Ipvm::ImageProcessing::Fill(ellipse, 255, maskImage);

        const Ipvm::Rect32s rtObject(ellipse);

        o_vecrtMeasure.push_back(rtObject);

        float fBallZPos = Ipvm::k_noiseValue32r;

        if (CalcZPosbyMask(split3DImage.m_zmap, rtObject, fBallZPos, fptBallCentroid, maskImage, &split3DImage.m_vmap))
        {
            tempPoint.m_x = fptBallCentroid.m_x * px2um_x;
            tempPoint.m_y = fptBallCentroid.m_y * px2um_y;
            tempPoint.m_z = fBallZPos;

            o_s3DAverage_um.m_x += tempPoint.m_x;
            o_s3DAverage_um.m_y += tempPoint.m_y;
            o_s3DAverage_um.m_z += fBallZPos;

            o_vec3Ddata_um.push_back(tempPoint);
            o_vecrtMeasured.push_back(rtObject);
        }
    }
#endif

    return TRUE;
}

BOOL VisionProcessingAlign3D::GetPlaneData_RemoveNoise(std::map<long, BOOL>& io_validObjectIndexTop,
    std::map<long, BOOL>& io_validObjectIndexBtm, std::vector<Ipvm::Point64r3>& io_top3DData,
    std::vector<Ipvm::Point64r3>& io_btm3DData, Ipvm::Point64r3& o_averageTop, Ipvm::Point64r3& o_averageBtm,
    Ipvm::Point64r3& o_planeABC)
{
    long validNum_Top = (long)io_top3DData.size();
    long validNum_Btm = (long)io_btm3DData.size();

    //------------------------------------------------------------------------------------------
    // 서로 다 같이 계산된 위치 포인트만 가지고 계산한다
    //------------------------------------------------------------------------------------------

    for (long index = 0; index < validNum_Top; index++)
    {
        long objectIndex = io_validObjectIndexTop[index];
        if (io_validObjectIndexBtm.find(objectIndex) == io_validObjectIndexBtm.end())
        {
            // 해당 포인트는 반대편에서 계산하지 못했다
            io_top3DData.erase(io_top3DData.begin() + index);
            io_validObjectIndexTop.erase(objectIndex);

            index--;
            validNum_Top--;
        }
    }

    for (long index = 0; index < validNum_Btm; index++)
    {
        long objectIndex = io_validObjectIndexBtm[index];
        if (io_validObjectIndexTop.find(objectIndex) == io_validObjectIndexTop.end())
        {
            // 해당 포인트는 반대편에서 계산하지 못했다
            io_btm3DData.erase(io_btm3DData.begin() + index);
            io_validObjectIndexBtm.erase(objectIndex);

            index--;
            validNum_Btm--;
        }
    }

    //long nDiff = (long)abs(validNum_Top - validNum_Btm);
    //float fDiffRatio = ((float)nDiff / (float)min(validNum_Top, validNum_Btm));
    if (validNum_Top != validNum_Btm) //if (fDiffRatio > 0.01f)
    {
        ASSERT(_T("남은 데이터 유효 포인트 수가 같아야 한다."));
        return FALSE;
    }

    long validNum = validNum_Top;

    if (validNum < 3)
    {
        // 평면을 구하기에 데이터 수가 너무 적다
        return FALSE;
    }

    //------------------------------------------------------------------------------------------
    // Point간의 차이를 가지고 평면을 만든다
    //------------------------------------------------------------------------------------------

    std::vector<Ipvm::Point64r3> diff3DData(validNum);

    for (long index = 0; index < validNum; index++)
    {
        // 가정. 평면의 X,Y 좌표는 Top, Bottom이 같은 위치다
        diff3DData[index] = io_top3DData[index];
        diff3DData[index].m_z = io_top3DData[index].m_z - io_btm3DData[index].m_z;
    }

    Ipvm::PlaneEq64r diffPlane;

    if (Ipvm::DataFitting::FitToPlane(validNum, &diff3DData[0], diffPlane) != Ipvm::Status::e_ok)
    {
        return FALSE;
    }

    //------------------------------------------------------------------------------------------
    // 평면과의 차이가 큰 Z 값의 경우는 Noise라고 가정하고 데이터를 버린다
    //------------------------------------------------------------------------------------------

    double errorSum = 0.;

    std::vector<double> errorData(validNum);

    for (long index = 0; index < validNum; index++)
    {
        float refZ = CAST_FLOAT(diffPlane.GetPositionZ(io_top3DData[index].m_x, io_top3DData[index].m_y));

        auto& errorZ = errorData[index];
        errorZ = fabs(diff3DData[index].m_z - refZ);
        errorSum += errorZ * errorZ;
    }

    float errorStd = (float)(sqrt(errorSum / (validNum - 1)));

    for (long index = validNum - 1; index >= 0; index--)
    {
        auto& errorZ = errorData[index];

        if (errorZ > errorStd)
        {
            io_top3DData.erase(io_top3DData.begin() + index);
            io_btm3DData.erase(io_btm3DData.begin() + index);
            diff3DData.erase(diff3DData.begin() + index);
        }
    }

    //------------------------------------------------------------------------------------------
    // Noise를 제거한 데이터를 가지고 다시 Fitting 한다
    //------------------------------------------------------------------------------------------

    validNum = long(io_top3DData.size());

    if (validNum < 3)
    {
        // 평면을 구하기에 데이터 수가 너무 적다
        return FALSE;
    }

    o_averageTop.m_x = 0.;
    o_averageTop.m_y = 0.;
    o_averageTop.m_z = 0.;

    o_averageBtm.m_x = 0.;
    o_averageBtm.m_y = 0.;
    o_averageBtm.m_z = 0.;

    for (long index = 0; index < validNum; index++)
    {
        o_averageTop.m_x += io_top3DData[index].m_x;
        o_averageTop.m_y += io_top3DData[index].m_y;
        o_averageTop.m_z += io_top3DData[index].m_z;

        o_averageBtm.m_x += io_btm3DData[index].m_x;
        o_averageBtm.m_y += io_btm3DData[index].m_y;
        o_averageBtm.m_z += io_btm3DData[index].m_z;
    }

    o_averageTop.m_x /= validNum;
    o_averageTop.m_y /= validNum;
    o_averageTop.m_z /= validNum;

    o_averageBtm.m_x /= validNum;
    o_averageBtm.m_y /= validNum;
    o_averageBtm.m_z /= validNum;

    //------------------------------------------------------------------------------------------
    // -1~1로 Normalize 한 다음 계산한다
    //------------------------------------------------------------------------------------------

    std::vector<Ipvm::Point64r3> diff3DData_Normalize;
    Ipvm::Point64r3 normalizeCenter;
    Ipvm::Point64r3 normalizeHalfDiff;

    if (!getNormalizePoints(diff3DData, diff3DData_Normalize, normalizeCenter, normalizeHalfDiff))
    {
        return FALSE;
    }

    if (Ipvm::DataFitting::FitToPlane(validNum, &diff3DData_Normalize[0], diffPlane) != Ipvm::Status::e_ok)
    {
        return FALSE;
    }

    o_planeABC.m_x = diffPlane.m_a / normalizeHalfDiff.m_x;
    o_planeABC.m_y = diffPlane.m_b / normalizeHalfDiff.m_y;
    o_planeABC.m_z = diffPlane.m_c / normalizeHalfDiff.m_z;

    return TRUE;
}

// v1 cross_product v2 = |v1||v2|sin@

double VisionProcessingAlign3D::GetDeltaTheta(const Ipvm::Point64r2& v1, const Ipvm::Point64r2& v2)
{
    return asin((v1.m_x * v2.m_y - v1.m_y * v2.m_x)
        / (sqrt(v1.m_x * v1.m_x + v1.m_y * v1.m_y) * sqrt(v2.m_x * v2.m_x + v2.m_y * v2.m_y)));
}

Ipvm::Point64r3 VisionProcessingAlign3D::Rotate3D(const Ipvm::Point64r3& i_Data, const Ipvm::Point64r3& i_Origin,
    const double& cosThetaX, const double& cosThetaY, const double& cosThetaZ, const double& sinThetaX,
    const double& sinThetaY, const double& sinThetaZ)
{
    Ipvm::Point64r3 o_Data = i_Data;

    double dx = i_Data.m_x - i_Origin.m_x;
    double dy = i_Data.m_y - i_Origin.m_y;
    double dz = i_Data.m_z - i_Origin.m_z;

    o_Data.m_x = (dx * (cosThetaY * cosThetaZ)) + (dy * (sinThetaX * sinThetaY * cosThetaZ - cosThetaX * sinThetaZ))
        + (dz * (cosThetaX * sinThetaY * cosThetaZ + sinThetaX * sinThetaZ)) + i_Origin.m_x;
    o_Data.m_y = (dx * (cosThetaY * sinThetaZ)) + (dy * (sinThetaX * sinThetaY * sinThetaZ + cosThetaX * cosThetaZ))
        + (dz * (cosThetaX * sinThetaY * sinThetaZ - sinThetaX * cosThetaZ)) + i_Origin.m_y;
    o_Data.m_z = (dx * (-sinThetaY)) + (dy * (sinThetaX * cosThetaY)) + (dz * (cosThetaX * cosThetaY)) + i_Origin.m_z;

    return o_Data;
}

BOOL VisionProcessingAlign3D::Rotate3DImage(const Ipvm::Image32r& i_image, const Ipvm::Rect32s& rtROI,
    const Ipvm::Point64r3& sOrigin_um, double xTheta, double yTheta, double zTheta, float fOffsetZ,
    Ipvm::Image32r& o_image)
{
    UNREFERENCED_PARAMETER(fOffsetZ);
    UNREFERENCED_PARAMETER(zTheta);

    if (Ipvm::ImageProcessing::Fill(rtROI, Ipvm::k_noiseValue32r, o_image) != Ipvm::Status::e_ok)
    {
        return FALSE;
    }

    const auto& px2um = getScale().pixelToUm();

    const double cosx = cos(xTheta);
    const double cosy = cos(yTheta);
    //const double cosz = cos(zTheta);

    const double sinx = sin(xTheta);
    const double siny = sin(yTheta);
    //const double sinz = sin(zTheta);

    for (long y = rtROI.m_top; y < rtROI.m_bottom; y++)
    {
        auto* src = i_image.GetMem(0, y);
        auto* dst = o_image.GetMem(0, y);

        for (long x = rtROI.m_left; x < rtROI.m_right; x++)
        {
            if (src[x] == Ipvm::k_noiseValue32r)
            {
                continue;
            }

            double dx_um = x * px2um.m_x - sOrigin_um.m_x;
            double dy_um = y * px2um.m_y - sOrigin_um.m_y;
            double dz_um = src[x] - sOrigin_um.m_z;

            //double new_x_um = (dx_um * (cosy*cosz)) + (dy_um * (sinx*siny*cosz - cosx*sinz)) + (dz_um * (cosx*siny*cosz + sinx*sinz)) + sOrigin_um.m_x;
            //double new_y_um = (dx_um * (cosy*sinz)) + (dy_um * (sinx*siny*sinz + cosx*cosz)) + (dz_um * (cosx*siny*sinz - sinx*cosz)) + sOrigin_um.m_y;
            double new_z_um = (dx_um * (-siny)) + (dy_um * (sinx * cosy)) + (dz_um * (cosx * cosy)) + sOrigin_um.m_z;

            dst[x] = (float)(new_z_um + fOffsetZ);
        }
    }

    return TRUE;
}

BOOL VisionProcessingAlign3D::MakeSpecBallROI(FPI_RECT sfrtPackageBody, Ipvm::Rect32s rtMatchOverlap,
    std::vector<Ipvm::Point32r2>& vecfptBallSpec, std::vector<Ipvm::Rect32s>& vecrtBallSpec,
    std::vector<Ipvm::Rect32s>& vecrtBallSearch, std::vector<long>& vecnBallID)
{
    vecfptBallSpec.clear();
    vecrtBallSpec.clear();
    vecrtBallSearch.clear();
    vecnBallID.clear();

    /////============[Gerber와 Image상의 Coefficient 구하기]===============///////
    /// Xi = a0 + a1*xr + a2*yr + a3*xr*yr
    /// Yi = b0 + b1*xr + b2*yr + a3*xr*yr
    /// 실제 좌표계에서 Packge Center를 기준으로 하는 4개의 꼭지점.
    Ipvm::Point32r2 pfptRealBody[4] = {};
    double dszX = getScale().convert_umToPixelX(m_packageSpec.m_bodyInfoMaster->fBodySizeX);
    double dszY = getScale().convert_umToPixelY(m_packageSpec.m_bodyInfoMaster->fBodySizeY);
    pfptRealBody[0] = Ipvm::Point32r2(-(float)dszX / 2.0f, -(float)dszY / 2.0f);
    pfptRealBody[1] = Ipvm::Point32r2(+(float)dszX / 2.0f, -(float)dszY / 2.0f);
    pfptRealBody[2] = Ipvm::Point32r2(+(float)dszX / 2.0f, +(float)dszY / 2.0f);
    pfptRealBody[3] = Ipvm::Point32r2(-(float)dszX / 2.0f, +(float)dszY / 2.0f);

    /// Image 좌표계에서 찾아진 4개의 꼭지점
    Ipvm::Point32r2 ptPcakageCenter = sfrtPackageBody.GetCenter();

    Ipvm::Point32r2 pfptImgBody[4] = {};
    //pfptImgBody에 pfptRealBody의 좌표를 넣으면서 ptPcakageCenter만큼 이동 시킨다.
    for (int i = 0; i < 4; i++)
    {
        pfptImgBody[i].m_x = pfptRealBody[i].m_x + ptPcakageCenter.m_x;
        pfptImgBody[i].m_y = pfptRealBody[i].m_y + ptPcakageCenter.m_y;
    }

    /// 실좌표와 Image좌표간의 방정식 계수를 구한다. (Polynomial warpping 또는 Bilinear transform을 참고하라)
    double bilinearTransform[2][4] = {
        0.,
    };

    if (Ipvm::Status::e_ok != Ipvm::Geometry::GetBilinearTransform(pfptRealBody, pfptImgBody, 4, bilinearTransform))
    {
        return FALSE;
    }

    long nBallID = 0;

    for (const auto& ball : m_packageSpec.m_ballMapAll->m_balls)
    {
        const float fHalfBallWidthX = (float)ball.m_radiusX_px; // TODO : X, Y 스케일 다른 것 고려할 것
        const float fHalfBallWidthY = (float)ball.m_radiusY_px; // TODO : X, Y 스케일 다른 것 고려할 것

        Ipvm::Point32r2 fptBallImageCenter = Ipvm::Geometry::TransformBilinear(
            Ipvm::Point32r2((float)ball.m_posX_px, (float)ball.m_posY_px), bilinearTransform);

        Ipvm::Rect32s rtImageBall = Ipvm::Rect32s((int32_t)(fptBallImageCenter.m_x - fHalfBallWidthX + 0.5f),
            (int32_t)(fptBallImageCenter.m_y - fHalfBallWidthY + 0.5f),
            (int32_t)(fptBallImageCenter.m_x + fHalfBallWidthX + 1.5f),
            (int32_t)(fptBallImageCenter.m_y + fHalfBallWidthY + 1.5f));
        rtImageBall.NormalizeRect();

        Ipvm::Point32s2 ptLT = Ipvm::Point32s2(rtImageBall.m_left, rtImageBall.m_top);
        Ipvm::Point32s2 ptRT = Ipvm::Point32s2(rtImageBall.m_right, rtImageBall.m_top);
        Ipvm::Point32s2 ptRB = Ipvm::Point32s2(rtImageBall.m_right, rtImageBall.m_bottom);
        Ipvm::Point32s2 ptLB = Ipvm::Point32s2(rtImageBall.m_left, rtImageBall.m_bottom);
        //if( rtMatchOverlap.PtInRect(rtImageBall.CenterPoint()) )
        if (rtMatchOverlap.PtInRect(ptLT) || rtMatchOverlap.PtInRect(ptRT) || rtMatchOverlap.PtInRect(ptRB)
            || rtMatchOverlap.PtInRect(ptLB))
        {
            vecfptBallSpec.push_back(fptBallImageCenter);
            vecrtBallSpec.push_back(rtImageBall);
            vecnBallID.push_back(nBallID);

            Ipvm::Rect32s rtBallSearch = rtImageBall;
            rtBallSearch.InflateRect(long(0.5f * fHalfBallWidthX + .5f), long(0.5f * fHalfBallWidthY + .5f));
            vecrtBallSearch.push_back(rtBallSearch);
        }

        nBallID++;
    }

    return TRUE;
}

BOOL VisionProcessingAlign3D::MakeSpecBallROI(const std::vector<long>& i_vecnRefBallID, FPI_RECT sfrtPackageBody,
    Ipvm::Rect32s rtMatchOverlap, std::vector<Ipvm::Point32r2>& vecfptBallSpec,
    std::vector<Ipvm::Rect32s>& vecrtBallSpec, std::vector<Ipvm::Rect32s>& vecrtBallSearch)
{
    vecfptBallSpec.clear();
    vecrtBallSpec.clear();
    vecrtBallSearch.clear();
    rtMatchOverlap.IsRectEmpty();

    /////============[Gerber와 Image상의 Coefficient 구하기]===============///////
    /// Xi = a0 + a1*xr + a2*yr + a3*xr*yr
    /// Yi = b0 + b1*xr + b2*yr + a3*xr*yr
    /// 실제 좌표계에서 Packge Center를 기준으로 하는 4개의 꼭지점.
    Ipvm::Point32r2 pfptRealBody[4] = {};
    double dszX = getScale().convert_umToPixelX(m_packageSpec.m_bodyInfoMaster->fBodySizeX);
    double dszY = getScale().convert_umToPixelY(m_packageSpec.m_bodyInfoMaster->fBodySizeY);
    pfptRealBody[0] = Ipvm::Point32r2(-(float)dszX / 2.0f, -(float)dszY / 2.0f);
    pfptRealBody[1] = Ipvm::Point32r2(+(float)dszX / 2.0f, -(float)dszY / 2.0f);
    pfptRealBody[2] = Ipvm::Point32r2(+(float)dszX / 2.0f, +(float)dszY / 2.0f);
    pfptRealBody[3] = Ipvm::Point32r2(-(float)dszX / 2.0f, +(float)dszY / 2.0f);

    /// Image 좌표계에서 찾아진 4개의 꼭지점
    Ipvm::Point32r2 ptPcakageCenter = sfrtPackageBody.GetCenter();

    Ipvm::Point32r2 pfptImgBody[4] = {};
    for (int i = 0; i < 4; i++)
    {
        pfptImgBody[i].m_x = pfptRealBody[i].m_x + ptPcakageCenter.m_x;
        pfptImgBody[i].m_y = pfptRealBody[i].m_y + ptPcakageCenter.m_y;
    }

    /// 실좌표와 Image좌표간의 방정식 계수를 구한다. (Polynomial warpping 또는 Bilinear transform을 참고하라)
    double bilinearTransform[2][4] = {
        0.,
    };

    if (Ipvm::Status::e_ok != Ipvm::Geometry::GetBilinearTransform(pfptRealBody, pfptImgBody, 4, bilinearTransform))
    {
        return FALSE;
    }

    long nRefBallNum = (long)i_vecnRefBallID.size();

    for (long nBall = 0; nBall < nRefBallNum; nBall++)
    {
        const long nBall_ID = i_vecnRefBallID[nBall];
        const auto& ballInfo = m_packageSpec.m_ballMapAll->m_balls[nBall_ID];

        const float fHalfBallWidthX = (float)ballInfo.m_radiusX_px;
        const float fHalfBallWidthY = (float)ballInfo.m_radiusY_px;

        Ipvm::Point32r2 fptBallImageCenter = Ipvm::Geometry::TransformBilinear(
            Ipvm::Point32r2((float)ballInfo.m_posX_px, (float)ballInfo.m_posY_px), bilinearTransform);

        Ipvm::Rect32s rtImageBall = Ipvm::Rect32s((int32_t)(fptBallImageCenter.m_x - fHalfBallWidthX + 0.5f),
            (int32_t)(fptBallImageCenter.m_y - fHalfBallWidthY + 0.5f),
            (int32_t)(fptBallImageCenter.m_x + fHalfBallWidthX + 1.5f),
            (int32_t)(fptBallImageCenter.m_y + fHalfBallWidthY + 1.5f));
        rtImageBall.NormalizeRect();
        vecfptBallSpec.push_back(fptBallImageCenter);
        vecrtBallSpec.push_back(rtImageBall);

        Ipvm::Rect32s rtBallSearch = rtImageBall;
        rtBallSearch.InflateRect(long(0.5f * fHalfBallWidthX + .5f), long(0.5f * fHalfBallWidthY + .5f));
        vecrtBallSearch.push_back(rtBallSearch);
    }

    return TRUE;
}

BOOL VisionProcessingAlign3D::MakeSplitPackage(const FPI_RECT& sfrtRawDevice, long nGrabCount,
    const std::vector<Ipvm::LineSeg32r>& vecsTotalMatchingLine, std::vector<FPI_RECT>& o_vecsfrtSplit,
    std::vector<FPI_RECT>& o_vecsfrtSplitPackage)
{
    if (nGrabCount <= 1)
        return FALSE;

    long nMatchLineNum = nGrabCount * 2 - 2; // 처음과 마지막은 1라인.
    if (nMatchLineNum != (long)vecsTotalMatchingLine.size())
        return FALSE;
    float fTopOverlapSizeLeftY = 0.f;
    float fTopOverlapSizeRightY = 0.f;

    float fBtmOverlapSizeLeftY = 0.f;
    float fBtmOverlapSizeRightY = 0.f;

    o_vecsfrtSplit.clear();
    o_vecsfrtSplit.resize(nGrabCount);
    for (long nGrab = 0; nGrab < nGrabCount; nGrab++)
    {
        FPI_RECT sfrtSplitPackage = sfrtRawDevice;
        sfrtSplitPackage.fptLT.m_x = m_result->m_deviceLeftInImage;
        sfrtSplitPackage.fptLB.m_x = m_result->m_deviceLeftInImage;
        sfrtSplitPackage.fptRT.m_x = m_result->m_deviceRightInImage;
        sfrtSplitPackage.fptRB.m_x = m_result->m_deviceRightInImage;
        if (nGrab == 0)
        {
            long nLine = 0;
            sfrtSplitPackage.fptLB.m_y = vecsTotalMatchingLine[nLine].m_sy;
            sfrtSplitPackage.fptRB.m_y = vecsTotalMatchingLine[nLine].m_ey;

            fTopOverlapSizeLeftY = vecsTotalMatchingLine[nLine].m_sy - sfrtRawDevice.fptLT.m_y;
            fTopOverlapSizeRightY = vecsTotalMatchingLine[nLine].m_ey - sfrtRawDevice.fptRT.m_y;
        }
        else if (nGrab == nGrabCount - 1)
        {
            long nLine = nMatchLineNum - 1;
            sfrtSplitPackage.fptLT.m_y = vecsTotalMatchingLine[nLine].m_sy;
            sfrtSplitPackage.fptRT.m_y = vecsTotalMatchingLine[nLine].m_ey;

            fBtmOverlapSizeLeftY = sfrtRawDevice.fptLB.m_y - vecsTotalMatchingLine[nLine].m_sy;
            fBtmOverlapSizeRightY = sfrtRawDevice.fptRB.m_y - vecsTotalMatchingLine[nLine].m_ey;
        }
        else
        {
            long nLine1 = 2 * nGrab - 1;
            long nLine2 = 2 * nGrab;
            sfrtSplitPackage.fptLT.m_y = vecsTotalMatchingLine[nLine1].m_sy;
            sfrtSplitPackage.fptRT.m_y = vecsTotalMatchingLine[nLine1].m_ey;

            sfrtSplitPackage.fptLB.m_y = vecsTotalMatchingLine[nLine2].m_sy;
            sfrtSplitPackage.fptRB.m_y = vecsTotalMatchingLine[nLine2].m_ey;
        }
        o_vecsfrtSplit[nGrab] = sfrtSplitPackage;
    }

    float fSpecPackageSizeY = getScale().convert_umToPixelY(m_packageSpec.m_bodyInfoMaster->fBodySizeY);
    o_vecsfrtSplitPackage.clear();
    o_vecsfrtSplitPackage.resize(nGrabCount);

    for (long nGrab = 0; nGrab < nGrabCount; nGrab++)
    {
        FPI_RECT sfrtSplitPackage = o_vecsfrtSplit[nGrab];
        if (nGrab == 0)
        {
            sfrtSplitPackage.fptLB.m_y = o_vecsfrtSplit[nGrab].fptLT.m_y + fSpecPackageSizeY;
            sfrtSplitPackage.fptRB.m_y = o_vecsfrtSplit[nGrab].fptRT.m_y + fSpecPackageSizeY;
        }
        else if (nGrab == nGrabCount - 1)
        {
            sfrtSplitPackage.fptLT.m_y = o_vecsfrtSplit[nGrab].fptLB.m_y - fSpecPackageSizeY;
            sfrtSplitPackage.fptRT.m_y = o_vecsfrtSplit[nGrab].fptRB.m_y - fSpecPackageSizeY;
        }
        else
        {
            sfrtSplitPackage.fptLT.m_y = o_vecsfrtSplit[nGrab].fptLT.m_y - fTopOverlapSizeLeftY;
            sfrtSplitPackage.fptRT.m_y = o_vecsfrtSplit[nGrab].fptRT.m_y - fTopOverlapSizeRightY;

            sfrtSplitPackage.fptLB.m_y = o_vecsfrtSplit[nGrab].fptLB.m_y + fBtmOverlapSizeLeftY;
            sfrtSplitPackage.fptRB.m_y = o_vecsfrtSplit[nGrab].fptRB.m_y + fBtmOverlapSizeRightY;
        }

        o_vecsfrtSplitPackage[nGrab] = sfrtSplitPackage;
    }

    return TRUE;
}

BOOL VisionProcessingAlign3D::CopyFirstSplitImage(
    S3DImageData& split3DImage, S3DImageData& ref3DImage, Ipvm::Rect32s rtFirstSplit)
{
    if (Ipvm::ImageProcessing::Copy(split3DImage.m_zmap, rtFirstSplit, ref3DImage.m_zmap) != Ipvm::Status::e_ok)
        return FALSE;
    if (Ipvm::ImageProcessing::Copy(split3DImage.m_vmap, rtFirstSplit, ref3DImage.m_vmap) != Ipvm::Status::e_ok)
        return FALSE;

    const auto& imageInfo = getImageLot().GetInfo();
    return Ipvm::Status::e_ok
        == Ipvm::ImageProcessing::ScaleInRangeWithNoise(ref3DImage.m_zmap, rtFirstSplit, Ipvm::k_noiseValue32r,
            imageInfo.m_heightRangeMin, imageInfo.m_heightRangeMax, 0, 1, 255, ref3DImage.m_byHmap);
}

BOOL VisionProcessingAlign3D::CopySplitImage(
    S3DImageData& split3DImage, S3DImageData& ref3DImage, Ipvm::Rect32s rtSrc, Ipvm::Rect32s rtDst)
{
    if (Ipvm::ImageProcessing::Copy(split3DImage.m_zmap, rtSrc, rtDst, ref3DImage.m_zmap) != Ipvm::Status::e_ok)
        return FALSE;
    if (Ipvm::ImageProcessing::Copy(split3DImage.m_vmap, rtSrc, rtDst, ref3DImage.m_vmap) != Ipvm::Status::e_ok)
        return FALSE;

    const auto& imageInfo = getImageLot().GetInfo();
    return Ipvm::Status::e_ok
        == Ipvm::ImageProcessing::ScaleInRangeWithNoise(ref3DImage.m_zmap, rtDst, Ipvm::k_noiseValue32r,
            imageInfo.m_heightRangeMin, imageInfo.m_heightRangeMax, 0, 1, 255, ref3DImage.m_byHmap);
}

void VisionProcessingAlign3D::SaveResulteData(long nPane, CString strResult)
{
    FILE* fp = NULL;
    CString strFileName;
    strFileName.Format(_T("%sPane%d Stitching Data.csv"), LPCTSTR(DynamicSystemPath::get(DefineFolder::Log)), nPane);
    _tfopen_s(&fp, strFileName, _T("a"));

    if (fp != NULL)
    {
        fprintf(fp, "%s", (LPCSTR)CStringA(strResult));
        fprintf(fp, "\n");
        fclose(fp);
    }
}

void VisionProcessingAlign3D::SaveResulteData(long nPane, const Ipvm::Point64r3& s3DTheta, float fOffsetZ)
{
    FILE* fp = NULL;
    CString strFileName;
    strFileName.Format(_T("%sPane%d Stitching Data.csv"), LPCTSTR(DynamicSystemPath::get(DefineFolder::Log)), nPane);
    _tfopen_s(&fp, strFileName, _T("a"));

    if (fp != NULL)
    {
        fprintf(fp, "Pane %d\n", nPane);

        fprintf(fp, "Theta, %f, %f\n", s3DTheta.m_x, s3DTheta.m_y);
        fprintf(fp, "Offset Z, %f\n", fOffsetZ);
        fprintf(fp, "\n");
        fclose(fp);
    }
}

void VisionProcessingAlign3D::SaveResulteData(
    long nPane, const std::vector<Ipvm::Point64r3>& vecs3DTheta, const Ipvm::Point64r3& s3DAverageTheta, float fOffsetZ)
{
    FILE* fp = NULL;
    CString strFileName;
    strFileName.Format(_T("%sPane%d Stitching Data.csv"), LPCTSTR(DynamicSystemPath::get(DefineFolder::Log)), nPane);
    _tfopen_s(&fp, strFileName, _T("a"));

    if (fp != NULL)
    {
        fprintf(fp, "Pane %d\n", nPane);
        for (long i = 0; i < (long)vecs3DTheta.size(); i++)
        {
            fprintf(fp, "Area%d Theta, %f, %f\n", i, vecs3DTheta[i].m_x, vecs3DTheta[i].m_y);
        }
        fprintf(fp, "Average Theta, %f, %f\n", s3DAverageTheta.m_x, s3DAverageTheta.m_y);
        fprintf(fp, "Offset Z, %f\n", fOffsetZ);
        fprintf(fp, "\n");
        fclose(fp);
    }

    strFileName.Empty();
}

BOOL VisionProcessingAlign3D::MakeSpecLand_ByPKGSize(FPI_RECT sfrtPackageBody, Ipvm::Rect32s rtMatchOverlap,
    float fObjSearchX_mm, float fObjSearchY_mm, std::vector<Ipvm::Point32r2>& o_vec2fptObjectSpec,
    std::vector<Ipvm::Rect32s>& o_vec2rtObjSearch, std::vector<long>& o_vec2rtObjID)
{
    o_vec2fptObjectSpec.clear();
    o_vec2rtObjSearch.clear();
    o_vec2rtObjID.clear();

    /////============[Gerber와 Image상의 Coefficient 구하기]===============///////
    /// Xi = a0 + a1*xr + a2*yr + a3*xr*yr
    /// Yi = b0 + b1*xr + b2*yr + a3*xr*yr
    /// 실제 좌표계에서 Packge Center를 기준으로 하는 4개의 꼭지점.

    Ipvm::Point32r2 pfptRealBody[4];
    double dszX = getScale().convert_umToPixelX(m_packageSpec.m_bodyInfoMaster->fBodySizeX);
    double dszY = getScale().convert_umToPixelY(m_packageSpec.m_bodyInfoMaster->fBodySizeY);
    pfptRealBody[0] = Ipvm::Point32r2(-(float)dszX / 2.0f, -(float)dszY / 2.0f);
    pfptRealBody[1] = Ipvm::Point32r2(+(float)dszX / 2.0f, -(float)dszY / 2.0f);
    pfptRealBody[2] = Ipvm::Point32r2(+(float)dszX / 2.0f, +(float)dszY / 2.0f);
    pfptRealBody[3] = Ipvm::Point32r2(-(float)dszX / 2.0f, +(float)dszY / 2.0f);

    Ipvm::Rect32s rtPackage = sfrtPackageBody.GetCRect();

    /// Image 좌표계에서 찾아진 4개의 꼭지점
    Ipvm::Point32r2 pfptImgBody[4];
    pfptImgBody[0] = sfrtPackageBody.fptLT;
    pfptImgBody[1] = sfrtPackageBody.fptRT;
    pfptImgBody[2] = sfrtPackageBody.fptRB;
    pfptImgBody[3] = sfrtPackageBody.fptLB;

    /// 실좌표와 Image좌표간의 방정식 계수를 구한다. (Polynomial warpping 또는 Bilinear transform을 참고하라)
    double bilinearTransform[2][4] = {
        0.,
    };

    if (Ipvm::Status::e_ok != Ipvm::Geometry::GetBilinearTransform(pfptRealBody, pfptImgBody, 4, bilinearTransform))
    {
        return FALSE;
    }

    Ipvm::Point32r2 fptTemp = Ipvm::Point32r2(0.f, 0.f);
    const auto& mmToPixel = getScale().mmToPixel();

    long nObjSearchX = static_cast<long>(fObjSearchX_mm / 2.f * mmToPixel.m_x + .5f);
    long nObjSearchY = static_cast<long>(fObjSearchY_mm / 2.f * mmToPixel.m_y + .5f);

    // Stitching 에서는 모든 Land 를 사용한다.
    //mc_Land 수정
    long nLand(0);
    //	if (m_packageSpec.m_LandMapManager.vecLandData.size() <= 0)
    if (m_packageSpec.m_LandMapConvertOrigin->vecLandData.size() <= 0)
    {
        return FALSE;
    }

    // Stitching 에서는 모든 Land 를 사용한다.
    //	for (auto LandInfo : m_packageSpec.m_LandMapManager.vecLandData)
    for (const auto& landInfo : m_packageSpec.m_LandMapConvertOrigin->vecLandData)
    {
        float fOffsetX = landInfo.fOffsetX;
        float fOffsetY = -landInfo.fOffsetY; // 이거 랜드3D 코드에서 -1 을 곱해주고 있어서 똑같이 함..썅.. 정리좀 하지..
        float fX = getScale().convert_umToPixelX(fOffsetX * 1000.f); // fptPixelperMM.m_x;
        float fY = getScale().convert_umToPixelY(fOffsetY * 1000.f); // fptPixelperMM.m_y;

        Ipvm::Point32r2 ptObjCenter = Ipvm::Geometry::TransformBilinear(Ipvm::Point32r2(fX, fY), bilinearTransform);

        if (rtMatchOverlap.PtInRect(ptObjCenter))
        {
            o_vec2fptObjectSpec.push_back(ptObjCenter);

            float fLengthX = getScale().convert_umToPixelX(landInfo.fLength * 1000.f); // fptPixelperMM.m_x;
            float fLengthY = getScale().convert_umToPixelX(landInfo.fLength * 1000.f); // fptPixelperMM.m_y;
            Ipvm::Rect32s rtObjSearch;
            rtObjSearch.m_left = static_cast<long>(ptObjCenter.m_x - fLengthX / 2.f + 0.5f);
            rtObjSearch.m_right = static_cast<long>(ptObjCenter.m_x + fLengthX / 2.f + 1.5f);
            rtObjSearch.m_top = static_cast<long>(ptObjCenter.m_y - fLengthY / 2.f + 0.5f);
            rtObjSearch.m_bottom = static_cast<long>(ptObjCenter.m_y + fLengthY / 2.f + 1.5f);
            rtObjSearch.InflateRect(nObjSearchX, nObjSearchY);
            o_vec2rtObjSearch.push_back(rtObjSearch);

            o_vec2rtObjID.push_back(nLand);
        }
        nLand++;
    }

    return TRUE;
}

BOOL VisionProcessingAlign3D::GetLandPlaneData(const S3DImageData& split3DImage, FPI_RECT sfrtPackage,
    Ipvm::Rect32s rtMatchingOverlap, const std::vector<Ipvm::Point32r2>& i_vec2fptObjectSpec,
    const std::vector<Ipvm::Rect32s>& i_vec2rtObjSearch, std::vector<Ipvm::Point64r3>& o_vec3DObj_um,
    Ipvm::Point64r3& o_s3DAverage_um, Ipvm::PlaneEq64r& o_sPlaneEq_um)
{
    int32_t rawImageSizeX = getImageLot().GetImageSizeX();
    int32_t rawImageSizeY = getImageLot().GetImageSizeY();

    o_vec3DObj_um.clear();
    o_s3DAverage_um = Ipvm::Point64r3(0.f, 0.f, 0.f);
    rtMatchingOverlap.IsRectEmpty();

    const auto& mmToPixel = getScale().mmToPixel();

    float fObjSearchX_mm = 0.5f;
    float fObjSearchY_mm = 0.5f;
    long nObjSearchX = static_cast<long>(fObjSearchX_mm * mmToPixel.m_x + .5f);
    long nObjSearchY = static_cast<long>(fObjSearchY_mm * mmToPixel.m_y + .5f);

    Ipvm::Image8u binaryImage;
    if (!getReusableMemory().GetByteImage(binaryImage, rawImageSizeX, rawImageSizeY))
        return FALSE;

    //float fSizeOffsetWidthRatio = -0.2f; // 10%
    //float fSizeOffsetLengthRatio = -0.2f; // 10%

    Ipvm::Rect32s rtROI = Ipvm::Rect32s(0, 0, 0, 0);
    for (long i = 0; i < (long)i_vec2rtObjSearch.size(); i++)
    {
        rtROI |= i_vec2rtObjSearch[i];
    }
    rtROI.InflateRect(long(nObjSearchX * 1.5f), (long)(nObjSearchY * 1.5f));
    rtROI &= sfrtPackage.GetCRect();

    std::vector<Ipvm::Rect32s> vecrtSpecLand;
    for (long i = 0; i < (long)i_vec2fptObjectSpec.size(); i++)
    {
        vecrtSpecLand.push_back(
            Ipvm::Rect32s((int32_t)(i_vec2fptObjectSpec[i].m_x - 3.5f), (int32_t)(i_vec2fptObjectSpec[i].m_y - 3.5f),
                (int32_t)(i_vec2fptObjectSpec[i].m_x + 3.5f), (int32_t)(i_vec2fptObjectSpec[i].m_y + 3.5f)));
    }

    std::vector<Ipvm::Rect32s> vec2rtObj;
    if (!FindLand_Blob(split3DImage, rtROI, binaryImage, i_vec2rtObjSearch, vec2rtObj))
    {
        return FALSE;
    }

    m_3DMergeResult->vecrtSpecObj.insert(
        m_3DMergeResult->vecrtSpecObj.end(), vecrtSpecLand.begin(), vecrtSpecLand.end());
    m_3DMergeResult->vecrtSearchObj.insert(
        m_3DMergeResult->vecrtSearchObj.end(), i_vec2rtObjSearch.begin(), i_vec2rtObjSearch.end());
    m_3DMergeResult->vecrtObj.insert(m_3DMergeResult->vecrtObj.end(), vec2rtObj.begin(), vec2rtObj.end());

    std::vector<Ipvm::Point32r2> vec2fptBlobCenter((long)vec2rtObj.size());
    for (long i = 0; i < (long)vec2rtObj.size(); i++)
    {
        vec2fptBlobCenter[i]
            = Ipvm::Point32r2((float)vec2rtObj[i].CenterPoint().m_x, (float)vec2rtObj[i].CenterPoint().m_y);
    }
    m_3DMergeResult->vecrtMeasure.insert(m_3DMergeResult->vecrtMeasure.end(), vec2rtObj.begin(), vec2rtObj.end());

    Ipvm::Image8u tempImage;
    Ipvm::Image8u maskImage;
    if (!getReusableMemory().GetByteImage(tempImage, rawImageSizeX, rawImageSizeY))
        return FALSE;
    if (!getReusableMemory().GetByteImage(maskImage, rawImageSizeX, rawImageSizeY))
        return FALSE;

    Ipvm::ImageProcessing::Fill(rtROI, 0, tempImage);

    //{{mc_Land 수정
    for (auto& roi : vec2rtObj)
    {
        Ipvm::ImageProcessing::Fill(roi, 255, tempImage);
    }
    //}}

    // 빵구 떼우고 줄인다.
    if (Ipvm::ImageProcessing::MorphDilateIter(rtROI, 3, 3, 2, binaryImage) != Ipvm::Status::e_ok)
        return FALSE;
    if (Ipvm::ImageProcessing::MorphErodeIter(rtROI, 3, 3, 4, binaryImage) != Ipvm::Status::e_ok)
        return FALSE;

    if (Ipvm::ImageProcessing::BitwiseAnd(binaryImage, tempImage, rtROI, maskImage) != Ipvm::Status::e_ok)
        return FALSE;

    //binaryImage.SaveBmp(_T("C:\\Temp\\Bin.bmp"));

    o_s3DAverage_um.m_x = 0.f;
    o_s3DAverage_um.m_y = 0.f;
    o_s3DAverage_um.m_z = 0.f;
    std::vector<Ipvm::Rect32s> vecrtMeasured;

    //{{mc_Land 수정
    for (long nLand = 0; nLand < long(vec2rtObj.size()); nLand++)
    {
        float fLandZPos(0.f);
        Ipvm::Point32r2 fptLandCenter = Ipvm::Point32r2(0.f, 0.f);
        Ipvm::Rect32s rtSearchLand = vec2rtObj[nLand];
        vecrtMeasured.push_back(rtSearchLand);
        if (CalcZPosbyMask(split3DImage.m_zmap, rtSearchLand, fLandZPos, fptLandCenter, maskImage))
        {
            Ipvm::Point64r3 s3DLand = Ipvm::Point64r3(getScale().convert_pixelToUmX(fptLandCenter.m_x),
                getScale().convert_pixelToUmY(fptLandCenter.m_y), fLandZPos);

            o_s3DAverage_um.m_x += s3DLand.m_x;
            o_s3DAverage_um.m_y += s3DLand.m_y;
            o_s3DAverage_um.m_z += s3DLand.m_z;

            o_vec3DObj_um.push_back(s3DLand);
        }
    }
    //}}

    m_3DMergeResult->vecrtMeasured.insert(
        m_3DMergeResult->vecrtMeasured.end(), vecrtMeasured.begin(), vecrtMeasured.end());

    long nValidDataNum = (long)o_vec3DObj_um.size();
    if (nValidDataNum <= 0)
        return FALSE;

    o_s3DAverage_um.m_x /= (double)nValidDataNum;
    o_s3DAverage_um.m_y /= (double)nValidDataNum;
    o_s3DAverage_um.m_z /= (double)nValidDataNum;

    if (Ipvm::DataFitting::FitToPlane((long)o_vec3DObj_um.size(), &o_vec3DObj_um[0], o_sPlaneEq_um)
        != Ipvm::Status::e_ok)
    {
        return FALSE;
    }

    return TRUE;
}

BOOL VisionProcessingAlign3D::FindLand_Blob(S3DImageData s3DImageInfo, Ipvm::Rect32s rtROI,
    Ipvm::Image8u& o_binaryImage, const std::vector<Ipvm::Rect32s>& vec2rtObjSearch,
    std::vector<Ipvm::Rect32s>& o_vec2rtObj)
{
    float maxHeightFactor = 0.3f; // 0.8f

    // Adaptive Threshold 적용을 시도한다.

    // Z-Map 을 가지고 적분 영상을 만든 후,
    // 최대 Ball 크기의 3배 영역 크기로 배경 높이를 구한 후,
    // 배경 높이를 제거해 준다.
    // 이렇게 하면 Ball 또는 높이를 가지는 녀석들만 남게 됨...

    // 이것을 최소 Ball Spec 의 최소 높이를 기준으로 255 밝기로 정규화한 이미지를 만든다..
    // 이것을 가지고 Ball 들을 검출하자.

    int32_t rawImageSizeX = getImageLot().GetImageSizeX();
    int32_t rawImageSizeY = getImageLot().GetImageSizeY();

    o_vec2rtObj.clear();

    const auto& mmToPixel = getScale().mmToPixel();

    CPI_Blob* pBlob = getReusableMemory().GetBlob();
    if (pBlob == NULL)
        return FALSE;

    Ipvm::Image32s labelImage;
    if (!getReusableMemory().GetLongImage(labelImage, rawImageSizeX, rawImageSizeY))
        return FALSE;

    Ipvm::Image32r zmap = s3DImageInfo.m_zmap;

    Ipvm::Image64r integralImage;
    Ipvm::Image32s sumCountImage;

    if (!getReusableMemory().GetDoubleImage(integralImage, rawImageSizeX, rawImageSizeY))
        return FALSE;
    if (!getReusableMemory().GetLongImage(sumCountImage, rawImageSizeX, rawImageSizeY))
        return FALSE;

    if (Ipvm::ImageProcessing::IntegralInclusiveWithNoise(
            zmap, rtROI, Ipvm::k_noiseValue32r, integralImage, sumCountImage)
        != Ipvm::Status::e_ok)
    {
        return FALSE;
    }

    float maxLandLengthX_px = 0.f;
    float maxLandLengthY_px = 0.f;
    float maxHeight_um = 0.f;

    // Stitching 에서는 모든 Land 를 사용한다.
    //	if (m_packageSpec.m_LandMapManager.vecLandData.size() <= 0)
    if (m_packageSpec.m_LandMapConvertOrigin->vecLandData.size() <= 0)
    {
        return FALSE;
    }

    // Stitching 에서는 모든 Land 를 사용한다.
    //	for (auto LandInfo : m_packageSpec.m_LandMapManager.vecLandData)
    for (const auto& LandInfo : m_packageSpec.m_LandMapConvertOrigin->vecLandData)
    {
        const float LandLengthX_px = LandInfo.fLength * mmToPixel.m_x;
        if (LandLengthX_px > maxLandLengthX_px)
            maxLandLengthX_px = LandLengthX_px;

        const float LandLengthY_px = LandInfo.fLength * mmToPixel.m_y;
        if (LandLengthY_px > maxLandLengthY_px)
            maxLandLengthY_px = LandLengthY_px;

        const float Height_um = CAST_FLOAT(maxHeightFactor * fabs(LandInfo.fHeight * 1000.));
        if (Height_um > maxHeight_um)
            maxHeight_um = Height_um;
    }

    Ipvm::Image32r backZmap;
    if (!getReusableMemory().GetFloatImage(backZmap, rawImageSizeX, rawImageSizeY))
        return FALSE;

    Ipvm::Size32s2 windowSize(long(2.5 * maxLandLengthX_px + 0.5f), long(2.5 * maxLandLengthY_px + 0.5f));
    if (Ipvm::ImageProcessing::MakeBackgroundImageWithNoise(
            integralImage, sumCountImage, rtROI, windowSize, Ipvm::k_noiseValue32r, backZmap)
        != Ipvm::Status::e_ok)
    {
        return FALSE;
    }

    Ipvm::Image8u grayImage;
    Ipvm::Image8u tempImage;

    if (!getReusableMemory().GetByteImage(grayImage, rawImageSizeX, rawImageSizeY))
        return FALSE;
    if (!getReusableMemory().GetByteImage(tempImage, rawImageSizeX, rawImageSizeY))
        return FALSE;

    for (long y = rtROI.m_top; y < rtROI.m_bottom; y++)
    {
        const float* zmap_y = zmap.GetMem(0, y);
        const float* back_y = backZmap.GetMem(0, y);
        BYTE* gray_y = grayImage.GetMem(0, y);

        for (long x = rtROI.m_left; x < rtROI.m_right; x++)
        {
            if (zmap_y[x] != Ipvm::k_noiseValue32r && back_y[x] != Ipvm::k_noiseValue32r)
            {
                const float diff = back_y[x] - zmap_y[x];

                if (diff >= maxHeight_um)
                {
                    gray_y[x] = 255;
                }
                else if (diff <= 0.f)
                {
                    gray_y[x] = 0;
                }
                else
                {
                    gray_y[x] = (BYTE)min(255, (diff / maxHeight_um * 255.f + .5f));
                }
            }
            else
            {
                gray_y[x] = 0;
            }
        }
    }

    Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(tempImage), 0, tempImage);

    Ipvm::ImageProcessing::FilterMedian3x3(grayImage, rtROI, true, tempImage);
    Ipvm::ImageProcessing::Fill(rtROI, 0, grayImage);
    Ipvm::ImageProcessing::FilterMedian3x3(tempImage, rtROI, true, grayImage);

    const long nMaxBlobNum = 100;
    long nLandNum = (long)vec2rtObjSearch.size();

    o_vec2rtObj.resize(nLandNum);
    for (long nLand = 0; nLand < nLandNum; nLand++)
    {
        Ipvm::Rect32s rtObjSearch = vec2rtObjSearch[nLand];

        BYTE nLowMean = 0;
        BYTE nHighMean = 0;
        BYTE nThreshold = 0;

        Ipvm::ImageProcessing::GetThresholdOtsu(grayImage, rtObjSearch, nLowMean, nHighMean, nThreshold);
        CippModules::Binarize(grayImage, o_binaryImage, rtObjSearch, nThreshold, FALSE);

        std::vector<Ipvm::BlobInfo> vecObjBlob(nMaxBlobNum);
        int32_t nCurBlobNum = nMaxBlobNum;

        pBlob->DoBlob(o_binaryImage, rtObjSearch, nMaxBlobNum, labelImage, &vecObjBlob[0], nCurBlobNum);

        if (nCurBlobNum <= 0)
        {
            return FALSE;
        }

        long maxBlobIndex = 0;
        long maxBlobArea = 0;
        for (long i = 0; i < nCurBlobNum; i++)
        {
            if (vecObjBlob[i].m_area > maxBlobArea)
            {
                maxBlobArea = vecObjBlob[i].m_area;
                maxBlobIndex = i;
            }
        }
        const auto& rtObject = vecObjBlob[maxBlobIndex].m_roi;
        //const long label = vecObjBlob[maxBlobIndex].m_label;

        o_vec2rtObj[nLand] = rtObject;
    }

    return TRUE;
}

BOOL VisionProcessingAlign3D::FillArbitraryRect(const PI_RECT& srtFillRect, BYTE byVal, Ipvm::Image8u& io_image)
{
    Ipvm::Quadrangle32r quadRangle;
    quadRangle.m_ltX = float(srtFillRect.ltX);
    quadRangle.m_ltY = float(srtFillRect.ltY);
    quadRangle.m_rtX = float(srtFillRect.rtX);
    quadRangle.m_rtY = float(srtFillRect.rtY);
    quadRangle.m_lbX = float(srtFillRect.blX);
    quadRangle.m_lbY = float(srtFillRect.blY);
    quadRangle.m_rbX = float(srtFillRect.brX);
    quadRangle.m_rbY = float(srtFillRect.brY);

    if (Ipvm::ImageProcessing::Fill(quadRangle, byVal, io_image) != Ipvm::Status::e_ok)
    {
        return FALSE;
    }

    return TRUE;
}

BOOL VisionProcessingAlign3D::CalcZPosbyMask(const Ipvm::Image32r& zmap, const Ipvm::Rect32s& rtArea, float& o_fZPos,
    Ipvm::Point32r2& o_fptCenter, const Ipvm::Image8u& mask, const Ipvm::Image16u* pwInten)
{
    o_fZPos = 0.f;
    o_fptCenter = Ipvm::Point32r2(0.f, 0.f);

    BOOL bWithIMap = FALSE;
    if (pwInten != nullptr && pwInten->GetMem() != nullptr)
    {
        bWithIMap = TRUE;
    }

    Ipvm::Rect32s validArea = Ipvm::Rect32s(zmap) & rtArea;
    if (validArea.IsRectEmpty())
    {
        return FALSE;
    }

    float fTotalInten = 0.f;
    float fTotalWeightZ = 0.f;
    float fTotalWeightX = 0.f;
    float fTotalWeightY = 0.f;

    float fSumZPos = 0.f;
    float fSumXPos = 0.f;
    float fSumYPos = 0.f;

    if (bWithIMap)
    {
        long nValidDataNum = 0;

        for (long y = validArea.m_top; y < validArea.m_bottom; y++)
        {
            auto* inten_y = pwInten->GetMem(0, y);
            auto* zmap_y = zmap.GetMem(0, y);
            auto* mask_y = mask.GetMem(0, y);

            for (long x = validArea.m_left; x < validArea.m_right; x++)
            {
                if (mask_y[x] == 255 && zmap_y[x] != Ipvm::k_noiseValue32r)
                {
                    nValidDataNum++;

                    fTotalInten += (float)inten_y[x];
                    fTotalWeightZ += zmap_y[x] * (float)inten_y[x];
                    fTotalWeightX += (float)inten_y[x] * (float)x;
                    fTotalWeightY += (float)inten_y[x] * (float)y;
                }
            }
        }

        if (nValidDataNum <= 0)
        {
            return FALSE;
        }

        o_fZPos = fTotalWeightZ / fTotalInten;
        o_fptCenter.m_x = fTotalWeightX / fTotalInten;
        o_fptCenter.m_y = fTotalWeightY / fTotalInten;

        return TRUE;
    }

    long nValidDataNum = 0;
    for (long y = validArea.m_top; y < validArea.m_bottom; y++)
    {
        auto* zmap_y = zmap.GetMem(0, y);
        auto* mask_y = mask.GetMem(0, y);

        for (long x = validArea.m_left; x < validArea.m_right; x++)
        {
            if (mask_y[x] == 255 && zmap_y[x] != Ipvm::k_noiseValue32r)
            {
                nValidDataNum++;
                fSumZPos += zmap_y[x];

                fSumXPos += (float)x;
                fSumYPos += (float)y;
            }
        }
    }

    if (nValidDataNum <= 0)
    {
        return FALSE;
    }

    o_fZPos = fSumZPos / (float)nValidDataNum;
    o_fptCenter.m_x = fSumXPos / (float)nValidDataNum;
    o_fptCenter.m_y = fSumYPos / (float)nValidDataNum;

    return TRUE;
}

float VisionProcessingAlign3D::CalcSrZPosbyMask(
    const Ipvm::Image32r& zmap, const Ipvm::Rect32s& roi, const Ipvm::Image8u& maskImage)
{
    float fZPos = Ipvm::k_noiseValue32r;

    const long allPixelCount = roi.Width() * roi.Height();
    const long desiredMaxPixelCount = 4000;

    // 소팅시 계산량을 줄이기 위해서 샘플링을 시도한다.
    const long samplingStep = max(1, long(sqrt((allPixelCount + desiredMaxPixelCount - 1) / desiredMaxPixelCount)));
    long dataCount = 0;

    int32_t rawImageSizeX = getImageLot().GetImageSizeX();
    int32_t rawImageSizeY = getImageLot().GetImageSizeY();

    Ipvm::Image32r temp;
    if (!getReusableMemory().GetFloatImage(temp, rawImageSizeX, rawImageSizeY))
    {
        return Ipvm::k_noiseValue32r;
    }

    Ipvm::Rect32s validArea = Ipvm::Rect32s(zmap) & roi;
    if (validArea.IsRectEmpty())
    {
        return Ipvm::k_noiseValue32r;
    }

    float* tempBuffer = temp.GetMem();

    for (long y = validArea.m_top; y < validArea.m_bottom; y += samplingStep)
    {
        const float* zmap_y = zmap.GetMem(0, y);
        const BYTE* mask_y = maskImage.GetMem(0, y);

        for (long x = validArea.m_left; x < validArea.m_right; x += samplingStep)
        {
            if (mask_y[x] == 255)
            {
                if (zmap_y[x] != Ipvm::k_noiseValue32r)
                {
                    tempBuffer[dataCount++] = zmap_y[x];
                }
            }
        }
    }

    if (dataCount < 3)
    {
        return fZPos;
    }

    // 이현민 : 속도 문제로 알고리즘 변경
    std::nth_element(tempBuffer, tempBuffer + dataCount / 2, tempBuffer + dataCount);
    //	std::partial_sort(tempBuffer, tempBuffer + dataCount / 2 + 1, tempBuffer + dataCount);

    const float median_um = tempBuffer[dataCount / 2];
    const float min_um = median_um - 5.f;
    const float max_um = median_um + 5.f;

    float sum = 0.f;
    long sumCount = 0;

    for (long y = validArea.m_top; y < validArea.m_bottom; y += 2)
    {
        const float* zmap_y = zmap.GetMem(0, y);

        for (long x = validArea.m_left; x < validArea.m_right; x += 2)
        {
            if (zmap_y[x] > min_um && zmap_y[x] < max_um)
            {
                sum += zmap_y[x];
                sumCount++;
            }
        }
    }

    fZPos = sum / sumCount;

    return fZPos;
}

BOOL VisionProcessingAlign3D::FindBall_Blob(S3DImageData s3DImageInfo, Ipvm::Rect32s rtROI,
    std::vector<Ipvm::Rect32s>& vecrtObjSearch, std::vector<Ipvm::Rect32s>& o_vecrtObj,
    std::vector<Ipvm::Point32r2>& o_vecfptObjCenter)
{
#ifdef USE_SR_CENTER
    // Ball의 Noise 영역 (그림자 영역)을 Blob하여 이를 Ball의 Center로 사용하자

    Ipvm::Image8u binaryImage;
    Ipvm::Image32s labelImage;

    if (!getReusableMemory().GetByteImage(binaryImage))
        return FALSE;
    if (!getReusableMemory().GetLongImage(labelImage))
        return FALSE;

    o_vecrtObj.clear();
    o_vecfptObjCenter.clear();

    Ipvm::Image32r zmap = s3DImageInfo.m_zmap;

    Ipvm::ImageProcessing::Fill(rtROI, 0, binaryImage);

    for (long y = rtROI.m_top; y < rtROI.m_bottom; y++)
    {
        auto* zmap_y = zmap.GetMem(0, y);
        auto* binary_y = binaryImage.GetMem(0, y);

        for (long x = rtROI.m_left; x < rtROI.m_right; x++)
        {
            if (zmap_y[x] == Ipvm::k_noiseValue32r)
            {
                binary_y[x] = 255;
            }
        }
    }
    const long nObjectNum = (long)vecrtObjSearch.size();
    const long nMaxBlobNum = 100;

    CPI_Blob* pBlob = getReusableMemory().GetBlob();

    std::vector<Ipvm::BlobInfo> vecObjBlob(nMaxBlobNum);

    for (long nObj = 0; nObj < nObjectNum; nObj++)
    {
        Ipvm::Rect32s rtSearchObj = vecrtObjSearch[nObj];

        long nCurBlobNum = nMaxBlobNum;

        pBlob->DoBlob(binaryImage, rtSearchObj, nMaxBlobNum, labelImage, &vecObjBlob[0], nCurBlobNum);
        pBlob->MergeBlobsByDistance(&vecObjBlob[0], nCurBlobNum, 5.f, labelImage);

        if (nCurBlobNum <= 0)
        {
            o_vecfptObjCenter.push_back(Ipvm::Point32r2(0.5f * (rtSearchObj.m_left + rtSearchObj.m_right - 1),
                0.5f * (rtSearchObj.m_top + rtSearchObj.m_bottom - 1)));
            o_vecrtObj.push_back(rtROI);
            continue;
        }

        long maxBlobIndex = 0;
        long maxBlobArea = 0;

        for (long i = 0; i < nCurBlobNum; i++)
        {
            if (vecObjBlob[i].m_area > maxBlobArea)
            {
                maxBlobArea = vecObjBlob[i].m_area;
                maxBlobIndex = i;
            }
        }

        o_vecrtObj.push_back(vecObjBlob[maxBlobIndex].m_roi);

        // Blob 의 무게 중심을 다시 구한다.

        Ipvm::Point32r2 massCenter;
        if (Ipvm::ImageProcessing::GetBlobMassCenter(labelImage, &vecObjBlob[maxBlobIndex], 1, &massCenter)
            != Ipvm::Status::e_ok)
        {
            return FALSE;
        }

        o_vecfptObjCenter.push_back(massCenter);
    }

    return TRUE;
#else
    Ipvm::Image8u grayImage;
    Ipvm::Image8u binaryImage;
    Ipvm::Image8u maskImage; //kircheis_Test_Stitching
    Ipvm::Image8u grayOriginImage;
    Ipvm::Image8u binaryOriginImage; //kircheis_Test_Stitching
    Ipvm::Image32s labelImage;

    int32_t rawImageSizeX = getImageLot().GetImageSizeX();
    int32_t rawImageSizeY = getImageLot().GetImageSizeY();

    if (!getReusableMemory().GetByteImage(grayImage, rawImageSizeX, rawImageSizeY))
        return FALSE;
    if (!getReusableMemory().GetByteImage(binaryImage, rawImageSizeX, rawImageSizeY))
        return FALSE;
    if (!getReusableMemory().GetByteImage(maskImage, rawImageSizeX, rawImageSizeY))
        return FALSE; //kircheis_Test_Stitching
    if (!getReusableMemory().GetByteImage(grayOriginImage, rawImageSizeX, rawImageSizeY))
        return FALSE; //kircheis_ImproveStitching
    if (!getReusableMemory().GetByteImage(binaryOriginImage, rawImageSizeX, rawImageSizeY))
        return FALSE; //kircheis_ImproveStitching
    if (!getReusableMemory().GetLongImage(labelImage, rawImageSizeX, rawImageSizeY))
        return FALSE;

    // Adaptive Threshold 적용을 시도한다.

    // Z-Map 을 가지고 적분 영상을 만든 후,
    // 최대 Ball 크기의 3배 영역 크기로 배경 높이를 구한 후,
    // 배경 높이를 제거해 준다.
    // 이렇게 하면 Ball 또는 높이를 가지는 녀석들만 남게 됨...

    // 이것을 최소 Ball Spec 의 최소 높이를 기준으로 255 밝기로 정규화한 이미지를 만든다..
    // 이것을 가지고 Ball 들을 검출하자.

    o_vecrtObj.clear();
    o_vecfptObjCenter.clear();

    Ipvm::Image32r zmap = s3DImageInfo.m_zmap;
    Ipvm::Image16u intensity = s3DImageInfo.m_vmap;

    Ipvm::Image64r integralImage;
    Ipvm::Image32s sumCountImage;

    if (!getReusableMemory().GetDoubleImage(integralImage, rawImageSizeX, rawImageSizeY))
        return FALSE;
    if (!getReusableMemory().GetLongImage(sumCountImage, rawImageSizeX, rawImageSizeY))
        return FALSE;

    if (Ipvm::ImageProcessing::IntegralInclusiveWithNoise(
            zmap, rtROI, Ipvm::k_noiseValue32r, integralImage, sumCountImage)
        != Ipvm::Status::e_ok)
    {
        return FALSE;
    }

    float maxBallDiameterX_px
        = (float)getScale().convert_umToPixelX(m_packageSpec.m_originalballMap->m_ballTypes.front().m_diameter_um);
    float maxBallDiameterY_px
        = (float)getScale().convert_umToPixelY(m_packageSpec.m_originalballMap->m_ballTypes.front().m_diameter_um);
    float maxBallHeight_um = (float)m_packageSpec.m_originalballMap->m_ballTypes.front().m_height_um;
    float minBallHeight_um = (float)m_packageSpec.m_originalballMap->m_ballTypes.back().m_height_um;

    Ipvm::Image32r backZmap;
    if (!getReusableMemory().GetFloatImage(backZmap, rawImageSizeX, rawImageSizeY))
        return FALSE;

    Ipvm::Size32s2 windowSize(
        long(3.f * (float)maxBallDiameterX_px + 0.5f), long(3.f * (float)maxBallDiameterY_px + 0.5f));

    if (Ipvm::ImageProcessing::MakeBackgroundImageWithNoise(
            integralImage, sumCountImage, rtROI, windowSize, Ipvm::k_noiseValue32r, backZmap)
        != Ipvm::Status::e_ok)
    {
        return FALSE;
    }

    const float fCalcFactor = 255.f / minBallHeight_um;

    if (maxBallHeight_um == minBallHeight_um)
    {
        for (long y = rtROI.m_top; y < rtROI.m_bottom; y++)
        {
            const float* zmap_y = zmap.GetMem(0, y);
            const float* back_y = backZmap.GetMem(0, y);
            BYTE* gray_y = grayImage.GetMem(0, y);

            for (long x = rtROI.m_left; x < rtROI.m_right; x++)
            {
                if (zmap_y[x] != Ipvm::k_noiseValue32r && back_y[x] != Ipvm::k_noiseValue32r)
                {
                    const float diff = zmap_y[x] - back_y[x];

                    if (diff >= minBallHeight_um)
                    {
                        gray_y[x] = 255;
                    }
                    else if (diff <= 0.f)
                    {
                        gray_y[x] = 0;
                    }
                    else
                    {
                        gray_y[x] = (BYTE)(diff * fCalcFactor);
                        //gray_y[x] = (BYTE)(diff / minBallHeight_um * 255.f);
                    }
                }
                else
                {
                    gray_y[x] = 0;
                }
            }
        }
    }
    else
    {
        for (long y = rtROI.m_top; y < rtROI.m_bottom; y++)
        {
            const float* zmap_y = zmap.GetMem(0, y);
            const float* back_y = backZmap.GetMem(0, y);
            BYTE* gray_y = grayImage.GetMem(0, y);

            for (long x = rtROI.m_left; x < rtROI.m_right; x++)
            {
                if (zmap_y[x] != Ipvm::k_noiseValue32r && back_y[x] != Ipvm::k_noiseValue32r)
                {
                    const float diff = zmap_y[x] - back_y[x];

                    if (diff >= maxBallHeight_um)
                    {
                        gray_y[x] = 0;
                    }
                    else if (diff >= minBallHeight_um)
                    {
                        gray_y[x] = 255;
                    }
                    else if (diff <= 0.f)
                    {
                        gray_y[x] = 0;
                    }
                    else
                    {
                        gray_y[x] = (BYTE)(diff * fCalcFactor);
                        //gray_y[x] = (BYTE)(diff / minBallHeight_um * 255.f);
                    }
                }
                else
                {
                    gray_y[x] = 0;
                }
            }
        }
    }

    CippModules::Binarize(grayImage, maskImage, rtROI, 1, FALSE); //kircheis_Test_Stitching
    Ipvm::ImageProcessing::Copy(
        grayImage, rtROI, grayOriginImage); //kircheis_ImproveStitching //Median Filter 과정에서 없어지는 문제 보완

    Ipvm::ImageProcessing::FilterMedian3x3(grayImage, rtROI, true, binaryImage);
    Ipvm::ImageProcessing::FilterMedian3x3(binaryImage, rtROI, true, grayImage);

    Ipvm::ImageProcessing::BitwiseAnd(
        maskImage, rtROI, grayImage); //kircheis_Test_Stitching//Median Filter 과정에서 불어난 영역을 지운다

    BYTE nLowMean = 0;
    BYTE nHighMean = 0;
    BYTE nThreshold = 0;

    if (Ipvm::ImageProcessing::GetThresholdOtsu(grayImage, rtROI, nLowMean, nHighMean, nThreshold)
        != Ipvm::Status::e_ok)
    {
        //return FALSE;

        nThreshold = 0;
    }

    CippModules::Binarize(grayImage, binaryImage, rtROI, nThreshold, FALSE);
    CippModules::Binarize(grayOriginImage, binaryOriginImage, rtROI, nThreshold, FALSE); //kircheis_ImproveStitching

    const long nObjectNum = (long)vecrtObjSearch.size();
    const long nMaxBlobNum = 100;

    CPI_Blob* pBlob = getReusableMemory().GetBlob();

    std::vector<Ipvm::BlobInfo> vecObjBlob(nMaxBlobNum);

    BOOL bIsReSearchBlob = FALSE;

    for (long nObj = 0; nObj < nObjectNum; nObj++)
    {
        Ipvm::Rect32s rtSearchObj = vecrtObjSearch[nObj];

        int32_t nCurBlobNum = nMaxBlobNum;
        bIsReSearchBlob = FALSE;

        pBlob->DoBlob(binaryImage, rtSearchObj, nMaxBlobNum, labelImage, &vecObjBlob[0], nCurBlobNum);

        if (nCurBlobNum <= 0)
        {
            nCurBlobNum
                = nMaxBlobNum; //kircheis_ImproveStitching //Median Filter 과정에서 Ball이 사라져 Blob이 없으면 원본으로 재시도
            pBlob->DoBlob(binaryOriginImage, rtSearchObj, nMaxBlobNum, labelImage, &vecObjBlob[0],
                nCurBlobNum); //kircheis_ImproveStitching
            if (nCurBlobNum <= 0) //그래도 없으면 어쩔수 없고
            {
                o_vecfptObjCenter.push_back(Ipvm::Point32r2(0.5f * (rtSearchObj.m_left + rtSearchObj.m_right - 1),
                    0.5f * (rtSearchObj.m_top + rtSearchObj.m_bottom - 1)));
                o_vecrtObj.push_back(rtSearchObj);
                continue;
            }
            bIsReSearchBlob = true;
        }

        long maxBlobIndex = 0;
        long maxBlobArea = 0;

        for (long i = 0; i < nCurBlobNum; i++)
        {
            if (vecObjBlob[i].m_area > maxBlobArea)
            {
                maxBlobArea = vecObjBlob[i].m_area;
                maxBlobIndex = i;
            }
        }

        const auto& rtObject = vecObjBlob[maxBlobIndex].m_roi;
        const long label = vecObjBlob[maxBlobIndex].m_label;
        o_vecrtObj.push_back(rtObject);

        // Blob 의 무게 중심을 다시 구한다.
        // 이 때는 Reliability Image 를 가중치로 더 사용한다.

        float fBallCenterX_px = 0.f;
        float fBallCenterY_px = 0.f;
        float weightSum = 0;

        for (long y = rtObject.m_top; y < rtObject.m_bottom; y++)
        {
            auto* gray_y = bIsReSearchBlob ? grayOriginImage.GetMem(0, y) : grayImage.GetMem(0, y);
            auto* label_y = labelImage.GetMem(0, y);
            auto* inten_y = intensity.GetMem(0, y);
            for (long x = rtObject.m_left; x < rtObject.m_right; x++)
            {
                if (label_y[x] == label)
                {
                    const BYTE gray = gray_y[x];
                    const WORD wInetn = inten_y[x];

                    const float weight = (float)((gray - long(nThreshold)) * wInetn);

                    fBallCenterX_px += weight * x;
                    fBallCenterY_px += weight * y;
                    weightSum += weight;
                }
            }
        }

        if (weightSum <= 0) //kircheis_3D_Stitching_Error_Test
            weightSum = 1.f;

        fBallCenterX_px /= weightSum;
        fBallCenterY_px /= weightSum;

        if (fBallCenterX_px * fBallCenterY_px <= 0) //kircheis_3D_Stitching_Error_Test
        {
            fBallCenterX_px = (float)rtObject.CenterPoint().m_x;
            fBallCenterY_px = (float)rtObject.CenterPoint().m_y;
        }

        o_vecfptObjCenter.push_back(Ipvm::Point32r2(fBallCenterX_px, fBallCenterY_px));
    }

    return TRUE;
#endif
}

bool VisionProcessingAlign3D::DoAlign(const bool detailSetupMode, Ipvm::Image8u& o_imageHMap,
    Ipvm::Image8u& o_imageIntensity, Ipvm::Image32r& o_imageZMap, Ipvm::Image16u& o_imageVMap)
{
    UNREFERENCED_PARAMETER(detailSetupMode);

    int stitchCount = getInspectionAreaInfo().m_stichCountY;

    if (stitchCount <= 1)
    {
        return true;
    }

    int32_t rawImageSizeX = getImageLot().GetImageSizeX();
    int32_t rawImageSizeY = getImageLot().GetImageSizeY();

    if (!getReusableMemory().GetByteImage(o_imageHMap, rawImageSizeX, rawImageSizeY)
        || !getReusableMemory().GetByteImage(o_imageIntensity, rawImageSizeX, rawImageSizeY)
        || !getReusableMemory().GetFloatImage(o_imageZMap, rawImageSizeX, rawImageSizeY)
        || !getReusableMemory().GetUShortImage(o_imageVMap, rawImageSizeX, rawImageSizeY))
    {
        m_bInvalid = TRUE;
        return false;
    }

    Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(o_imageZMap), Ipvm::k_noiseValue32r, o_imageZMap);
    Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(o_imageHMap), 0, o_imageHMap);
    Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(o_imageVMap), 0, o_imageVMap);

    m_3DMergeResult->Clear();

    S3DImageData split3DImage;
    split3DImage.m_byHmap = getImageLot().GetImageFrame(0, enSideVisionModule::SIDE_VISIONMODULE_FRONT);
    split3DImage.m_byInten = getImageLot().GetImageFrame(1, enSideVisionModule::SIDE_VISIONMODULE_FRONT);
    split3DImage.m_zmap = getImageLot().GetZmapImage(0);
    split3DImage.m_vmap = getImageLot().GetVmapImage(0);

    int nImageX = split3DImage.m_byHmap.GetSizeX();
    int nImageY = split3DImage.m_byHmap.GetSizeY();
    Ipvm::Rect32s rtImage(10, 10, nImageX - 10, nImageY - 10);

    CString strResult;
    CString strTemp;

    long paneCountX = (long)getInspectionAreaInfo().m_pocketArrayX.size();

    long rawSizeX = getImageLot().GetImageSizeX();
    long rawSizeY = getImageLot().GetImageSizeY();

    S3DImageData ref3DImage;
    ref3DImage.m_byHmap = Ipvm::Image8u(rawSizeX, rawSizeY);
    ref3DImage.m_byInten = Ipvm::Image8u(rawSizeX, rawSizeY);
    ref3DImage.m_zmap = Ipvm::Image32r(rawSizeX, rawSizeY);
    ref3DImage.m_vmap = Ipvm::Image16u(rawSizeX, rawSizeY);

    Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(ref3DImage.m_byHmap), 0, ref3DImage.m_byHmap);
    Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(ref3DImage.m_byInten), 0, ref3DImage.m_byInten);
    Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(ref3DImage.m_zmap), Ipvm::k_noiseValue32r, ref3DImage.m_zmap);
    Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(ref3DImage.m_vmap), 0, ref3DImage.m_vmap);

    std::vector<long> vecnFailPane;
    std::vector<std::vector<Ipvm::LineSeg32r>> vec2FailMatchLine;

    std::vector<float> vecfPackageCenterY;
    m_3DMergeResult->vecrtPane.resize(paneCountX);
    m_3DMergeResult->vecrtDevice.resize(paneCountX);

    long nPane = GetCurrentPaneID();

    strTemp.Format(_T("[Pane %d]\r\n"), nPane);
    strResult += strTemp;
    m_3DMergeResult->vecrtPane[nPane] = Ipvm::Rect32s(0, 0, 0, 0);

    auto& scale = getScale();

    // 1. Make Rough Overlap
    std::vector<Ipvm::Rect32s> vecrtOverlap;
    std::vector<Ipvm::LineSeg32r> vecRoughMatchLine;
    FPI_RECT sfrtRawDevice;

    if (!SplitPackageAlign(split3DImage.m_byHmap, stitchCount, sfrtRawDevice, vecrtOverlap))
    {
        return false;
    }

    if (!MakeRoughOverlap(vecrtOverlap, sfrtRawDevice, vecRoughMatchLine))
    {
        return false;
    }

    // 2. Make Matching Overlap
    std::vector<Ipvm::Rect32s> vecrtTotalMatchOverlap;
    std::vector<Ipvm::LineSeg32r> vecsTotalMatchingLine;

    if (!MakeMatchingOverlap(split3DImage, vecrtOverlap, vecrtTotalMatchOverlap, vecsTotalMatchingLine))
    {
        return false;
    }

    // 3. Split Package Area Calc.
    std::vector<FPI_RECT> vecsfrtSplitPackage;
    FPI_RECT sfrtPackage;
    std::vector<Ipvm::Rect32s> vecrtCalc(stitchCount);

    if (!SplitAreaCalc(vecrtTotalMatchOverlap, vecsTotalMatchingLine, sfrtRawDevice, stitchCount, vecsfrtSplitPackage,
            sfrtPackage, vecrtCalc))
    {
        return false;
    }

    // Pane 영역 설정.
    float fExtPaneRate = m_VisionPara.m_fPaneExtensionRatio / 100.f;
    Ipvm::Rect32s rtDevice = sfrtPackage.GetCRect();
    long nExtPaneX = (long)((float)rtDevice.Width() * fExtPaneRate / 2.f);
    long nExtPaneY = (long)((float)rtDevice.Height() * fExtPaneRate / 2.f);
    Ipvm::Rect32s rtPane = rtDevice;
    rtPane.InflateRect(nExtPaneX, nExtPaneY);
    rtPane &= Ipvm::Rect32s(0, 0, nImageX, nImageY);

    // Copy First Split Rect
    Ipvm::Rect32s rtFirstSplit = Ipvm::Rect32s(
        rtPane.m_left, 0, rtPane.m_right, split3DImage.m_zmap.GetSizeY() / stitchCount); // 딱 원본만큼만 복사
    if (!CopyFirstSplitImage(split3DImage, ref3DImage, rtFirstSplit))
    {
        vecnFailPane.push_back(nPane);
        vec2FailMatchLine.push_back(vecRoughMatchLine);
        return false;
    }

    float fRealShiftLeftY = 0.f;
    float fRealShiftRightY = 0.f;
    float fRealShiftY = 0.f;

    float fRealShiftLeftX = 0.f;
    float fRealShiftRightX = 0.f;
    float fRealShiftX = 0.f;

    const auto& imageInfo = getImageLot().GetInfo();
    long nMergeCount = (long)vecrtTotalMatchOverlap.size() / 2;
    for (long nMerge = 0; nMerge < nMergeCount; nMerge++)
    {
        // 4. Object 위치 설정.
        Ipvm::LineSeg32r sTopLine = vecsTotalMatchingLine[nMerge * 2];
        Ipvm::LineSeg32r sBottomLine = vecsTotalMatchingLine[nMerge * 2 + 1];
        float fShiftYLeft = sBottomLine.m_sy - sTopLine.m_sy;
        float fShiftYRight = sBottomLine.m_ey - sTopLine.m_ey;
        float fShiftY = (fShiftYLeft + fShiftYRight) / 2.f;

        float fShiftXLeft = sBottomLine.m_sx - sTopLine.m_sx;
        float fShiftXRight = sBottomLine.m_ex - sTopLine.m_ex;
        float fShiftX = (fShiftXLeft + fShiftXRight) / 2.f;

        Ipvm::LineSeg32r sShiftTopLine = Ipvm::LineSeg32r(sTopLine.m_sx - fRealShiftLeftX,
            sTopLine.m_sy - fRealShiftLeftY, sTopLine.m_ex - fRealShiftRightX, sTopLine.m_ey - fRealShiftRightY);

        //FPI_RECT sfrtLeftPackage = vecsfrtSplitPackage[nMerge];
        FPI_RECT sfrtTopPackage = sfrtPackage; // 1st Grab 영역으로 설정.
        FPI_RECT sfrtBottomPackage = vecsfrtSplitPackage[nMerge + 1];

        Ipvm::Rect32s rtTopMatchingOverlap = vecrtTotalMatchOverlap[nMerge * 2];
        //long nShiftX = static_cast<long>(fGrabPitchXpx * (float)nMerge + .5f);
        long nShiftX = static_cast<long>(fRealShiftX + .5f);
        long nShiftY = static_cast<long>(fRealShiftY + .5f);
        rtTopMatchingOverlap -= Ipvm::Point32s2(nShiftX, nShiftY);
        Ipvm::Rect32s rtBottomMatchingOverlap = vecrtTotalMatchOverlap[nMerge * 2 + 1];

        // 법선 벡터 계산.
        Ipvm::Point64r3 s3DTopAverage_um = Ipvm::Point64r3(0.f, 0.f, 0.f);
        Ipvm::Point64r3 s3DBottomAverage_um = Ipvm::Point64r3(0.f, 0.f, 0.f);

        Ipvm::Point64r3 s3DTheta;
        bool selectedSearchObject = false;

        // Device 타입에 따라 Object 탐색 분기

        if (!SearchMatchingObject_Ball(split3DImage, ref3DImage, sfrtRawDevice, sfrtBottomPackage, sfrtTopPackage,
                rtTopMatchingOverlap, rtBottomMatchingOverlap, nMerge, s3DTopAverage_um, s3DBottomAverage_um, s3DTheta,
                strResult, selectedSearchObject))
        {
            return false;
        }

        if (!selectedSearchObject)
        {
            // Ball Object로 Matching이 불가능하면 Land를 체크해 본다
            if (!SearchMatchingObject_Land(split3DImage, ref3DImage, sfrtBottomPackage, sfrtTopPackage,
                    rtTopMatchingOverlap, rtBottomMatchingOverlap, s3DTopAverage_um, s3DBottomAverage_um, s3DTheta,
                    selectedSearchObject))
            {
                return false;
            }
        }

        if (!selectedSearchObject)
        {
            // Ball도 Land도 불가능하면 임의의 Matching Roi를 이용하는 방법으로 진행
            if (!MakeTopMatchingObject(split3DImage, ref3DImage, rtTopMatchingOverlap, rtBottomMatchingOverlap, nMerge,
                    s3DTopAverage_um, s3DBottomAverage_um, s3DTheta, strResult))
            {
                return false;
            }
        }

        // 영상 회전한 다음 다시 회전한 영상과 Plane을 비교하여야 한다.
        // 6. 영상 Stitching
        std::vector<Ipvm::Point64r3> vecConfirmBottomData;
        Ipvm::Rect32s rtRotate = vecrtCalc[nMerge + 1];
        rtRotate.InflateRect(1, 1);
        rtRotate.m_left = rtPane.m_left;
        rtRotate.m_right = rtPane.m_right;
        rtRotate &= rtImage;
        if (!StitchingImage(split3DImage, sShiftTopLine, sBottomLine, rtRotate, s3DTheta, s3DTopAverage_um,
                s3DBottomAverage_um, ref3DImage, vecConfirmBottomData))
        {
            return FALSE;
        }
        double lfOffsetZ = (s3DTopAverage_um.m_z - s3DBottomAverage_um.m_z);

        strTemp.Format(_T("Result Rotate Angle (x=%f, y=%f)\r\n"), (float)s3DTheta.m_x, (float)s3DTheta.m_y);
        strResult += strTemp;
        strResult += _T("\r\n");

        BOOL bSaveData = FALSE;
        if (bSaveData)
        {
            //SaveResulteData(nPane, strResult);
            SaveResulteData(nPane, s3DTheta, (float)lfOffsetZ);
        }

        m_3DMergeResult->vecsCrossLine.push_back(sShiftTopLine);

        // BYTE영상 생성(다음 Matching을 위해)
        Ipvm::ImageProcessing::ScaleInRangeWithNoise(ref3DImage.m_zmap, rtPane, Ipvm::k_noiseValue32r,
            imageInfo.m_heightRangeMin, imageInfo.m_heightRangeMax, 0, 1, 255, ref3DImage.m_byHmap);

        fRealShiftLeftY += fShiftYLeft;
        fRealShiftRightY += fShiftYRight;
        fRealShiftY += fShiftY;

        fRealShiftLeftX += fShiftXLeft;
        fRealShiftRightX += fShiftXRight;
        fRealShiftX += fShiftX;

        m_3DMergeResult->vecrtPane[nPane] = rtPane;
        m_3DMergeResult->vecrtDevice[nPane] = rtDevice;
        vecfPackageCenterY.push_back((float)rtPane.CenterPoint().m_y);
        /////////////////////////////////////////////////////////////////////////////////////////////////////////
    }

    // Modify Pane ROI.
    Ipvm::Rect32s rtSpecPackage;
    float fSpecPackageSizeX = getScale().convert_umToPixelX(m_packageSpec.m_bodyInfoMaster->fBodySizeX);
    float fSpecPackageSizeY = getScale().convert_umToPixelY(m_packageSpec.m_bodyInfoMaster->fBodySizeY);
    rtSpecPackage.m_left = static_cast<long>((float)nImageX / 2.f - fSpecPackageSizeX / 2.f + .5f);
    rtSpecPackage.m_right = static_cast<long>((float)nImageX / 2.f + fSpecPackageSizeX / 2.f + .5f);
    rtSpecPackage.m_top = static_cast<long>((float)nImageY / 2.f - fSpecPackageSizeY / 2.f + .5f);
    rtSpecPackage.m_bottom = static_cast<long>((float)nImageY / 2.f + fSpecPackageSizeY / 2.f + .5f);
    Ipvm::Rect32s rtSpecPane = rtSpecPackage;

    float fExtModifyPaneRate = m_VisionPara.m_fPaneExtensionRatio / 100.f;
    long nExtModifyPaneX = static_cast<long>(rtSpecPane.Width() * fExtModifyPaneRate / 2.f);
    long nExtModifyPaneY = static_cast<long>(rtSpecPane.Height() * fExtModifyPaneRate / 2.f);
    rtSpecPane.InflateRect(nExtModifyPaneX, nExtModifyPaneY);

    float fSpecPaneHalfSizeY = (float)rtSpecPane.Height() / 2.f;
    float fPackageCentroidY = 0.f;

    for (long nData = 0; nData < (long)vecfPackageCenterY.size(); nData++)
    {
        fPackageCentroidY += vecfPackageCenterY[nData];
    }

    fPackageCentroidY /= (float)vecfPackageCenterY.size();

    Ipvm::Rect32s rtSearchPane = m_3DMergeResult->vecrtPane[nPane];

    if (rtSearchPane.IsRectEmpty())
    {
        m_errorLog = _T("(Fail) Pane Empty");

        ASSERT(0);
        return false;
    }

    if (paneCountX < 2)
    {
        rtSearchPane.m_top = (long)(fPackageCentroidY - fSpecPaneHalfSizeY);
        rtSearchPane.m_bottom = (long)(fPackageCentroidY + fSpecPaneHalfSizeY);
        rtSearchPane &= rtImage;
    }
    else
    {
        rtSearchPane.m_top = (long)(fPackageCentroidY - fSpecPaneHalfSizeY);
        rtSearchPane.m_bottom = (long)(fPackageCentroidY + fSpecPaneHalfSizeY);

        // X Pocket 위치를 찾는다
        Ipvm::Point32r2 imageCenter(getImageLot().GetImageSizeX() * 0.5f, getImageLot().GetImageSizeY() * 0.5f);
        Ipvm::Rect32s pocketRoi(0, 0, 0, 0);
        for (auto& fovIndex : getInspectionAreaInfo().m_fovList)
        {
            long unitID = getInspectionAreaInfo().m_unitIndexList[nPane];
            auto pocketRegion = getTrayScanSpec().GetUnitPocketRegionInFOV(fovIndex, unitID);
            if (!pocketRegion.IsRectEmpty())
            {
                pocketRoi = scale.convert_mmToPixel(pocketRegion) + Ipvm::Conversion::ToPoint32s2(imageCenter);
                break;
            }
        }

        rtSearchPane.m_left = pocketRoi.m_left;
        rtSearchPane.m_right = pocketRoi.m_right;
        rtSearchPane &= rtImage;
    }

    m_3DMergeResult->vecrtPane[nPane] = rtSearchPane;

    Ipvm::Image16u srcImage = ref3DImage.m_vmap;
    Ipvm::Image8u dstImage = ref3DImage.m_byInten;

    unsigned short maxValue = 0;
    Ipvm::ImageProcessing::GetMax(srcImage, Ipvm::Rect32s(srcImage), maxValue);
    Ipvm::ImageProcessing::Scale(srcImage, Ipvm::Rect32s(srcImage), 0, maxValue, dstImage);

    Ipvm::Rect32s rtPaneArea = Ipvm::Rect32s(rtPane.m_left, 0, rtPane.m_right, split3DImage.m_zmap.GetSizeY());

    if (Ipvm::ImageProcessing::Copy(ref3DImage.m_zmap, rtPaneArea, o_imageZMap) != Ipvm::Status::e_ok)
        return false;
    if (Ipvm::ImageProcessing::Copy(ref3DImage.m_vmap, rtPaneArea, o_imageVMap) != Ipvm::Status::e_ok)
        return false;

    Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(o_imageHMap), 0, o_imageHMap);
    Ipvm::ImageProcessing::ScaleInRangeWithNoise(o_imageZMap, rtPaneArea, Ipvm::k_noiseValue32r,
        imageInfo.m_heightRangeMin, imageInfo.m_heightRangeMax, 0, 1, 255, o_imageHMap);
    Ipvm::ImageProcessing::Scale(o_imageVMap, rtPaneArea, o_imageIntensity);

    return true;
}

BOOL VisionProcessingAlign3D::MakeRoughOverlap(std::vector<Ipvm::Rect32s> i_vecrtOverlap, FPI_RECT i_sfrtRawDevice,
    std::vector<Ipvm::LineSeg32r>& o_vecRoughMatchLine)
{
    long nMatchLineNum = (long)i_vecrtOverlap.size();

    for (long nMatchLine = 0; nMatchLine < nMatchLineNum; nMatchLine++)
    {
        Ipvm::LineSeg32r sRoughMatchLine = Ipvm::LineSeg32r((float)i_vecrtOverlap[nMatchLine].m_left,
            (float)i_vecrtOverlap[nMatchLine].CenterPoint().m_y, (float)i_vecrtOverlap[nMatchLine].m_right,
            (float)i_vecrtOverlap[nMatchLine].CenterPoint().m_y);
        o_vecRoughMatchLine.push_back(sRoughMatchLine);
    }

    m_3DMergeResult->vecsfrtRawDevice.push_back(i_sfrtRawDevice);
    m_3DMergeResult->vecrtOverlap.insert(
        m_3DMergeResult->vecrtOverlap.end(), i_vecrtOverlap.begin(), i_vecrtOverlap.end());
    m_3DMergeResult->vecsRoughMatchLine.insert(
        m_3DMergeResult->vecsRoughMatchLine.end(), o_vecRoughMatchLine.begin(), o_vecRoughMatchLine.end());

    return TRUE;
}

BOOL VisionProcessingAlign3D::MakeMatchingOverlap(S3DImageData& io_split3DImage,
    const std::vector<Ipvm::Rect32s> i_vecrtOverlap, std::vector<Ipvm::Rect32s>& o_vecrtTotalMatchOverlap,
    std::vector<Ipvm::LineSeg32r>& o_vecsTotalMatchingLine)
{
    BOOL returnValue = TRUE;

    long nOverlapNum = (long)i_vecrtOverlap.size();
    if (nOverlapNum % 2 != 0)
    {
        // Overlap Count는 짝수여야 한다
        m_errorLog.AppendFormat(_T("(Fail) Cann't match overlay ROI\r\n"));
        return FALSE;
    }

    for (long nOverlap = 0; nOverlap < nOverlapNum; nOverlap++)
    {
        std::vector<Ipvm::Rect32s> vecrtTempl;
        std::vector<Ipvm::Rect32s> vecrtSearch;
        std::vector<Ipvm::Rect32s> vecrtMatchOverlap;
        std::vector<Ipvm::LineSeg32r> vecsMatchingLine;
        Ipvm::Rect32s rtTopOverlap = i_vecrtOverlap[nOverlap];
        Ipvm::Rect32s rtBottomOverlap = i_vecrtOverlap[++nOverlap];
        if (!CalcMatchOverlapROI(io_split3DImage, rtTopOverlap, rtBottomOverlap, vecrtTempl, vecrtSearch,
                vecrtMatchOverlap, vecsMatchingLine))
        {
            m_errorLog.AppendFormat(_T("(Fail) Cann't match overlay ROI(%d)\r\n"), nOverlap + 1);
            returnValue = FALSE;
        }

        o_vecrtTotalMatchOverlap.insert(
            o_vecrtTotalMatchOverlap.end(), vecrtMatchOverlap.begin(), vecrtMatchOverlap.end());
        o_vecsTotalMatchingLine.insert(o_vecsTotalMatchingLine.end(), vecsMatchingLine.begin(), vecsMatchingLine.end());
        m_3DMergeResult->vecrt2DMatch.insert(m_3DMergeResult->vecrt2DMatch.end(), vecrtTempl.begin(), vecrtTempl.end());
        m_3DMergeResult->vecrt2DMatch.insert(
            m_3DMergeResult->vecrt2DMatch.end(), vecrtSearch.begin(), vecrtSearch.end());
        m_3DMergeResult->vecsSplitCrossLine.insert(
            m_3DMergeResult->vecsSplitCrossLine.end(), vecsMatchingLine.begin(), vecsMatchingLine.end());
        m_3DMergeResult->vecrtMatchOverlap.insert(
            m_3DMergeResult->vecrtMatchOverlap.end(), vecrtMatchOverlap.begin(), vecrtMatchOverlap.end());
    }

    return returnValue;
}

BOOL VisionProcessingAlign3D::SplitAreaCalc(const std::vector<Ipvm::Rect32s>& i_vecrtTotalMatchOverlap,
    const std::vector<Ipvm::LineSeg32r>& i_vecsTotalMatchingLine, const FPI_RECT& i_sfrtRawDevice,
    const long i_nGrabCount, std::vector<FPI_RECT>& o_vecsfrtSplitPackage, FPI_RECT& o_sfrtPackage,
    std::vector<Ipvm::Rect32s>& o_vecrtCalc)
{
    std::vector<FPI_RECT> vecsfrtSplit;
    const auto& mmToPixel = getScale().mmToPixel();

    if (!MakeSplitPackage(i_sfrtRawDevice, i_nGrabCount, i_vecsTotalMatchingLine, vecsfrtSplit, o_vecsfrtSplitPackage))
    {
        return FALSE;
    }

    o_sfrtPackage = o_vecsfrtSplitPackage[0];
    m_3DMergeResult->vecsfrtSplit.insert(m_3DMergeResult->vecsfrtSplit.end(), vecsfrtSplit.begin(), vecsfrtSplit.end());
    m_3DMergeResult->vecsfrtSplitVirtualDevice.insert(
        m_3DMergeResult->vecsfrtSplitVirtualDevice.end(), o_vecsfrtSplitPackage.begin(), o_vecsfrtSplitPackage.end());

    long nExtRateY_mm = static_cast<long>(2.5f * mmToPixel.m_y + .5f); // 5mm;
    long nOverlapCount = (long)i_vecrtTotalMatchOverlap.size();
    for (long nCalc = 0; nCalc < i_nGrabCount; nCalc++)
    {
        Ipvm::Rect32s rtCalc;
        if (nCalc == 0)
        {
            long nOverlap = 0;
            long nSlit = 0;
            Ipvm::Rect32s rtSlit = vecsfrtSplit[nSlit].GetSPI_RECT().GetExtRect();
            rtCalc = i_vecrtTotalMatchOverlap[nOverlap] | rtSlit;
            rtCalc.InflateRect(0, nExtRateY_mm);
            if (rtCalc.m_bottom > m_3DMergeResult->vecrtOverlap[nOverlap].m_bottom)
                rtCalc.m_bottom = m_3DMergeResult->vecrtOverlap[nOverlap].m_bottom;
        }
        else if (nCalc == i_nGrabCount - 1)
        {
            long nOverlap = nOverlapCount - 1;
            long nSlit = nCalc;
            Ipvm::Rect32s rtSlit = vecsfrtSplit[nSlit].GetSPI_RECT().GetExtRect();
            rtCalc = i_vecrtTotalMatchOverlap[nOverlap] | rtSlit;
            rtCalc.InflateRect(0, nExtRateY_mm);
            if (rtCalc.m_top < m_3DMergeResult->vecrtOverlap[nOverlap].m_top)
                rtCalc.m_top = m_3DMergeResult->vecrtOverlap[nOverlap].m_top;
        }
        else
        {
            long nOverlap1 = 2 * nCalc - 1;
            long nOverlap2 = 2 * nCalc;
            Ipvm::Rect32s rtOverlap1 = i_vecrtTotalMatchOverlap[nOverlap1];
            Ipvm::Rect32s rtOverlap2 = i_vecrtTotalMatchOverlap[nOverlap2];
            rtCalc = rtOverlap1 | rtOverlap2;
            rtCalc.InflateRect(0, nExtRateY_mm);

            if (rtCalc.m_top < m_3DMergeResult->vecrtOverlap[nOverlap1].m_top)
                rtCalc.m_top = m_3DMergeResult->vecrtOverlap[nOverlap1].m_top;
            if (rtCalc.m_bottom > m_3DMergeResult->vecrtOverlap[nOverlap2].m_bottom)
                rtCalc.m_bottom = m_3DMergeResult->vecrtOverlap[nOverlap2].m_bottom;
        }

        o_vecrtCalc[nCalc] = rtCalc;
    }
    m_3DMergeResult->vecrtCalc.insert(m_3DMergeResult->vecrtCalc.end(), o_vecrtCalc.begin(), o_vecrtCalc.end());

    return TRUE;
}

BOOL VisionProcessingAlign3D::MakeTopMatchingObject(const S3DImageData& i_split3DImage,
    const S3DImageData& i_ref3DImage, const Ipvm::Rect32s i_rtTopMatchingOverlap,
    const Ipvm::Rect32s i_rtBottomMatchingOverlap, const long i_nMerge, Ipvm::Point64r3& o_s3DTopAverage_um,
    Ipvm::Point64r3& o_s3DBottomAverage_um, Ipvm::Point64r3& o_s3DTheta, CString& o_strResult)
{
    float fTime = 0.f;
    Ipvm::TimeCheck TimeCheck;
    CString strTemp;

    std::vector<long> vecnObjectLayer;

    Ipvm::PlaneEq64r sTopPlaneEq_um;
    Ipvm::PlaneEq64r sBottomPlaneEq_um;
    std::vector<Ipvm::Point64r3> vecTop3Ddata_um;
    std::vector<Ipvm::Point64r3> vecBottom3Ddata_um;

    std::vector<Ipvm::Rect32s> vecrtTopMeasure;
    std::vector<Ipvm::Rect32s> vecrtBottomMeasure;
    std::vector<Ipvm::Rect32s> vecrtTopMeasured;
    std::vector<Ipvm::Rect32s> vecrtBottomMeasured;

    std::vector<Ipvm::Point32r2> vecfptBottomCenter;

    if (!MakeFixMeasureROI(i_rtBottomMatchingOverlap, vecfptBottomCenter, m_VisionPara.m_nTopStichRoiAmountX,
            m_VisionPara.m_nTopStichRoiAmountY))
    {
        m_errorLog = _T("Fail Make Fix Measure ROI Data\r\n");
        o_strResult += m_errorLog;

        return FALSE;
    }

    for (long i = 0; i < vecfptBottomCenter.size(); i++)
    {
        vecnObjectLayer.push_back(0);
    }

    //여기 이후는 같음
    if (!GetPlaneData_byTop(i_split3DImage, vecfptBottomCenter, vecnObjectLayer, vecrtBottomMeasure,
            vecrtBottomMeasured, vecBottom3Ddata_um, sBottomPlaneEq_um, o_s3DBottomAverage_um))
    {
        m_errorLog = _T("Fail Make Plane Data (Top)\r\n");
        o_strResult += m_errorLog;

        return FALSE;
    }

    std::vector<Ipvm::Point32r2> vecfptTopCenter;

    if (!MakeFixMeasureROI(i_rtTopMatchingOverlap, vecfptTopCenter, m_VisionPara.m_nTopStichRoiAmountX,
            m_VisionPara.m_nTopStichRoiAmountY))
    {
        strTemp.Format(_T("Fail Make Fix Measure ROI Data\r\n"));
        o_strResult += strTemp;
        m_errorLog = o_strResult;

        return FALSE;
    }

    if (!GetPlaneData_byTop(i_ref3DImage, vecfptTopCenter, vecnObjectLayer, vecrtTopMeasure, vecrtTopMeasured,
            vecTop3Ddata_um, sTopPlaneEq_um, o_s3DTopAverage_um))
    {
        m_errorLog = _T("Fail Make Plane Data (Bottom)\r\n");
        o_strResult += m_errorLog;

        return FALSE;
    }
    fTime = CAST_FLOAT(TimeCheck.Elapsed_ms());

    m_3DMergeResult->vecrtMeasure.insert(
        m_3DMergeResult->vecrtMeasure.end(), vecrtTopMeasure.begin(), vecrtTopMeasure.end());
    m_3DMergeResult->vecrtMeasure.insert(
        m_3DMergeResult->vecrtMeasure.end(), vecrtBottomMeasure.begin(), vecrtBottomMeasure.end());
    m_3DMergeResult->vecrtMeasured.insert(
        m_3DMergeResult->vecrtMeasured.end(), vecrtTopMeasured.begin(), vecrtTopMeasured.end());
    m_3DMergeResult->vecrtMeasured.insert(
        m_3DMergeResult->vecrtMeasured.end(), vecrtBottomMeasured.begin(), vecrtBottomMeasured.end());

    // Top plane 에서 Bottom plane 으로 얼마만큼 회전되었는지 계산
    // 왼손 좌표계이므로 Y 에 -1 을 곱해준다.
    // Top은 회전을 할 기준이 없으므로 회전값을 정할 Theta값을 0으로 초기화 했습니다....
    float lfRadThetaZ
        = 0; //GetDeltaTheta(Ipvm::Point64r2(sTopPlaneEq_um.m_a, sTopPlaneEq_um.m_b), Ipvm::Point64r2(sBottomPlaneEq_um.m_a, sBottomPlaneEq_um.m_b));
    float lfRadThetaY
        = 0; //-GetDeltaTheta(Ipvm::Point64r2(sTopPlaneEq_um.m_a, sTopPlaneEq_um.m_c), Ipvm::Point64r2(sBottomPlaneEq_um.m_a, sBottomPlaneEq_um.m_c));
    float lfRadThetaX
        = 0; //GetDeltaTheta(Ipvm::Point64r2(sTopPlaneEq_um.m_b, sTopPlaneEq_um.m_c), Ipvm::Point64r2(sBottomPlaneEq_um.m_b, sBottomPlaneEq_um.m_c));

    o_s3DTheta = Ipvm::Point64r3(lfRadThetaX, lfRadThetaY, lfRadThetaZ);

    strTemp.Format(
        _T("%d Top nVector (%f, %f, %f)\r\n"), i_nMerge, sTopPlaneEq_um.m_a, sTopPlaneEq_um.m_b, sTopPlaneEq_um.m_c);
    o_strResult += strTemp;
    strTemp.Format(_T("%d Bottom nVector (%f, %f, %f)\r\n"), i_nMerge, sBottomPlaneEq_um.m_a, sBottomPlaneEq_um.m_b,
        sBottomPlaneEq_um.m_c);
    o_strResult += strTemp;
    strTemp.Format(_T("%d Rotate Angle (x=%f, y=%f)\r\n"), i_nMerge, o_s3DTheta.m_x * DEF_RAD_TO_DEG,
        o_s3DTheta.m_y * DEF_RAD_TO_DEG);
    o_strResult += strTemp;

    return TRUE;
}

bool VisionProcessingAlign3D::SearchMatchingObject_Ball(const S3DImageData& i_split3DImage,
    const S3DImageData& i_ref3DImage, const FPI_RECT i_sfrtRawDevice, const FPI_RECT i_sfrtBottomPackage,
    const FPI_RECT i_sfrtTopPackage, const Ipvm::Rect32s i_rtTopMatchingOverlap,
    const Ipvm::Rect32s i_rtBottomMatchingOverlap, const long i_nMerge, Ipvm::Point64r3& o_s3DTopAverage_um,
    Ipvm::Point64r3& o_s3DBottomAverage_um, Ipvm::Point64r3& o_s3DTheta, CString& o_strResult,
    bool& o_selectedSearchObject)
{
    o_selectedSearchObject = false;

    std::vector<Ipvm::Rect32s> vecrtBottomSpecBall;
    CString strTemp;

    std::vector<long> vecnObjectLayer;

    std::vector<Ipvm::Point32r2> vecfptBottomSpecBall;
    std::vector<Ipvm::Rect32s> vecrtBottomSearchBall;
    std::vector<long> vecnBottomBall_ID;

    std::map<long, BOOL> validObjectIndex_Top;
    std::map<long, BOOL> validObjectIndex_Btm;
    std::vector<Ipvm::Point64r3> vecTop3Ddata_um;
    std::vector<Ipvm::Point64r3> vecBottom3Ddata_um;

    std::vector<Ipvm::Rect32s> vecrtTopMeasure;
    std::vector<Ipvm::Rect32s> vecrtBottomMeasure;
    std::vector<Ipvm::Rect32s> vecrtTopMeasured;
    std::vector<Ipvm::Rect32s> vecrtBottomMeasured;

    if (m_packageSpec.m_originalballMap->m_balls.size() == 0)
    {
        // Ball 데이터가 없다
        return true;
    }

    const float fBallWidthX
        = (float)getScale().convert_umToPixelX(m_packageSpec.m_originalballMap->m_ballTypes[0].m_diameter_um);
    const float fBallWidthY
        = (float)getScale().convert_umToPixelY(m_packageSpec.m_originalballMap->m_ballTypes[0].m_diameter_um);

    if (!MakeSpecBallROI(i_sfrtBottomPackage, i_rtBottomMatchingOverlap, vecfptBottomSpecBall, vecrtBottomSpecBall,
            vecrtBottomSearchBall, vecnBottomBall_ID))
    {
        strTemp.Format(_T("Fail Make Spec Ball ROI\r\n"));
        o_strResult += strTemp;
        m_errorLog = o_strResult;

        return false;
    }

    if (vecrtBottomSpecBall.size() < 3 || vecrtBottomSearchBall.size() < 3)
    {
        // Ball 영역이 계산에 사용할 만큼 충분하지 않다
        return true;
    }

    o_selectedSearchObject = true;

    m_3DMergeResult->vecrtSpecObj.insert(
        m_3DMergeResult->vecrtSpecObj.end(), vecrtBottomSpecBall.begin(), vecrtBottomSpecBall.end());
    m_3DMergeResult->vecrtSearchObj.insert(
        m_3DMergeResult->vecrtSearchObj.end(), vecrtBottomSearchBall.begin(), vecrtBottomSearchBall.end());

    Ipvm::Rect32s rtBottomMeasureROI = Ipvm::Rect32s(0, 0, 0, 0);
    for (long n = 0; n < (long)vecrtBottomSearchBall.size(); n++)
    {
        rtBottomMeasureROI |= vecrtBottomSearchBall[n];
    }
    rtBottomMeasureROI.InflateRect(long(fBallWidthX * 1.5f), (long)(fBallWidthY * 1.5f));
    rtBottomMeasureROI &= i_sfrtRawDevice.GetCRect();

    float fTimeFindBall = 0.f;
    Ipvm::TimeCheck TimeChkFindBall;

    // {{ Find Ball
    // Find Righit Ball
    std::vector<Ipvm::Rect32s> vecrtBottomBall;
    std::vector<Ipvm::Point32r2> vecfptBottomBallCenter;

    if (!FindBall_Blob(
            i_split3DImage, rtBottomMeasureROI, vecrtBottomSearchBall, vecrtBottomBall, vecfptBottomBallCenter))
    {
        strTemp.Format(_T("Fail Find Ball\r\n"));
        o_strResult += strTemp;
        m_errorLog = o_strResult;

        return false;
    }

    m_3DMergeResult->vecrtObj.insert(m_3DMergeResult->vecrtObj.end(), vecrtBottomBall.begin(), vecrtBottomBall.end());
    m_3DMergeResult->vecfptObj.insert(
        m_3DMergeResult->vecfptObj.end(), vecfptBottomBallCenter.begin(), vecfptBottomBallCenter.end());

    if (!Get3DPoints_byBall(i_split3DImage, vecfptBottomBallCenter, vecrtBottomMeasure, vecrtBottomMeasured,
            validObjectIndex_Btm, vecBottom3Ddata_um))
    {
        m_errorLog = _T("Fail Make Plane Data (Bottom)\r\n");
        o_strResult += m_errorLog;

        return false;
    }

    // Referece 영상에서 Spec Ball을 계산한다.
    std::vector<Ipvm::Point32r2> vecfptLeftSpecBall;
    std::vector<Ipvm::Rect32s> vecrtTopSpecBall;
    std::vector<Ipvm::Rect32s> vecrtTopSearchBall;

    if (!MakeSpecBallROI(vecnBottomBall_ID, i_sfrtTopPackage, i_rtTopMatchingOverlap, vecfptLeftSpecBall,
            vecrtTopSpecBall, vecrtTopSearchBall))
    {
        strTemp.Format(_T("Fail Make Spec Ball ROI\r\n"));
        o_strResult += strTemp;
        m_errorLog = o_strResult;

        return false;
    }

    m_3DMergeResult->vecrtSpecObj.insert(
        m_3DMergeResult->vecrtSpecObj.end(), vecrtTopSpecBall.begin(), vecrtTopSpecBall.end());
    m_3DMergeResult->vecrtSearchObj.insert(
        m_3DMergeResult->vecrtSearchObj.end(), vecrtTopSearchBall.begin(), vecrtTopSearchBall.end());

    Ipvm::Rect32s rtTopMeasureROI = Ipvm::Rect32s(0, 0, 0, 0);
    for (long n = 0; n < (long)vecrtTopSearchBall.size(); n++)
    {
        rtTopMeasureROI |= vecrtTopSearchBall[n];
    }
    rtTopMeasureROI.InflateRect(long(fBallWidthX * 1.5f), (long)(fBallWidthY * 1.5f));
    rtTopMeasureROI &= i_sfrtRawDevice.GetCRect();

    std::vector<Ipvm::Rect32s> vecrtTopBall;
    std::vector<Ipvm::Point32r2> vecfptTopBallCenter;
    if (!FindBall_Blob(i_ref3DImage, rtTopMeasureROI, vecrtTopSearchBall, vecrtTopBall, vecfptTopBallCenter))
    {
        strTemp.Format(_T("Fail Find Ball\r\n"));
        o_strResult += strTemp;
        m_errorLog = o_strResult;

        return false;
    }

    m_3DMergeResult->vecrtObj.insert(m_3DMergeResult->vecrtObj.end(), vecrtTopBall.begin(), vecrtTopBall.end());
    m_3DMergeResult->vecfptObj.insert(
        m_3DMergeResult->vecfptObj.end(), vecfptTopBallCenter.begin(), vecfptTopBallCenter.end());

    if (!Get3DPoints_byBall(i_ref3DImage, vecfptTopBallCenter, vecrtTopMeasure, vecrtTopMeasured, validObjectIndex_Top,
            vecTop3Ddata_um))
    {
        m_errorLog = _T("Fail Make Plane Data (Top)\r\n");
        o_strResult += m_errorLog;

        return false;
    }

    fTimeFindBall = CAST_FLOAT(TimeChkFindBall.Elapsed_ms());

    Ipvm::Point64r3 planeABC;

    if (!GetPlaneData_RemoveNoise(validObjectIndex_Top, validObjectIndex_Btm, vecTop3Ddata_um, vecBottom3Ddata_um,
            o_s3DTopAverage_um, o_s3DBottomAverage_um, planeABC))
    {
        m_errorLog = _T("Fail Make Plane Data\r\n");
        o_strResult += m_errorLog;

        return false;
    }

    m_3DMergeResult->vecrtMeasure.insert(
        m_3DMergeResult->vecrtMeasure.end(), vecrtTopMeasure.begin(), vecrtTopMeasure.end());
    m_3DMergeResult->vecrtMeasure.insert(
        m_3DMergeResult->vecrtMeasure.end(), vecrtBottomMeasure.begin(), vecrtBottomMeasure.end());
    m_3DMergeResult->vecrtMeasured.insert(
        m_3DMergeResult->vecrtMeasured.end(), vecrtTopMeasured.begin(), vecrtTopMeasured.end());
    m_3DMergeResult->vecrtMeasured.insert(
        m_3DMergeResult->vecrtMeasured.end(), vecrtBottomMeasured.begin(), vecrtBottomMeasured.end());

    // Top plane 에서 Bottom plane 으로 얼마만큼 회전되었는지 계산
    // 왼손 좌표계이므로 Y 에 -1 을 곱해준다.

    float lfRadThetaZ = 0.f;
    float lfRadThetaY = CAST_FLOAT(atan2(planeABC.m_y, planeABC.m_z));
    float lfRadThetaX = CAST_FLOAT(atan2(planeABC.m_x, planeABC.m_z));

    lfRadThetaZ = 0.f;
    lfRadThetaY = CAST_FLOAT(-atan2(planeABC.m_x, planeABC.m_z));
    lfRadThetaX = CAST_FLOAT(atan2(planeABC.m_y, planeABC.m_z));

    o_s3DTheta = Ipvm::Point64r3(lfRadThetaX, lfRadThetaY, lfRadThetaZ);

    strTemp.Format(_T("%d nVector (%f, %f, %f)\r\n"), i_nMerge, planeABC.m_x, planeABC.m_y, planeABC.m_z);
    o_strResult += strTemp;
    strTemp.Format(_T("%d Rotate Angle (x=%f, y=%f)\r\n"), i_nMerge, o_s3DTheta.m_x * DEF_RAD_TO_DEG,
        o_s3DTheta.m_y * DEF_RAD_TO_DEG);
    o_strResult += strTemp;

    return true;
}

bool VisionProcessingAlign3D::SearchMatchingObject_Land(const S3DImageData& i_split3DImage,
    const S3DImageData& i_ref3DImage, const FPI_RECT i_sfrtBottomPackage, const FPI_RECT i_sfrtTopPackage,
    const Ipvm::Rect32s i_rtTopMatchingOverlap, const Ipvm::Rect32s i_rtBottomMatchingOverlap,
    Ipvm::Point64r3& o_s3DTopAverage_um, Ipvm::Point64r3& o_s3DBottomAverage_um, Ipvm::Point64r3& o_s3DTheta,
    bool& o_selectedSearchObject)
{
    o_selectedSearchObject = false;
    float fObjSearchX_mm = 0.5f;
    float fObjSearchY_mm = 0.5f;

    /////////////////////////////////////////////////////////////////////////////////
    std::vector<Ipvm::Point32r2> vec2fptLandCenter_Top;
    std::vector<Ipvm::Rect32s> vec2rtSearchLand_Top;
    std::vector<long> vec2LandIndex_Top;

    Ipvm::PlaneEq64r sTopPlaneEq_um;
    Ipvm::PlaneEq64r sBottomPlaneEq_um;
    std::vector<Ipvm::Point64r3> vecTop3Ddata_um;
    std::vector<Ipvm::Point64r3> vecBottom3Ddata_um;

    if (m_packageSpec.m_LandMapConvertOrigin->vecLandData.size() <= 0)
    {
        // Land 영역이 계산에 사용할 만큼 충분하지 않다
        return true;
    }

    if (!MakeSpecLand_ByPKGSize(i_sfrtTopPackage, i_rtTopMatchingOverlap, fObjSearchX_mm, fObjSearchY_mm,
            vec2fptLandCenter_Top, vec2rtSearchLand_Top, vec2LandIndex_Top))
    {
        m_errorLog = _T("(Fail) Make Spec Land - Top");
        return false;
    }

    std::vector<Ipvm::Point32r2> vec2fptLandCenter_Bottom;
    std::vector<Ipvm::Rect32s> vec2rtSearchLand_Bottom;
    std::vector<long> vec2LandIndex_Bottom;

    if (!MakeSpecLand_ByPKGSize(i_sfrtBottomPackage, i_rtBottomMatchingOverlap, fObjSearchX_mm, fObjSearchY_mm,
            vec2fptLandCenter_Bottom, vec2rtSearchLand_Bottom, vec2LandIndex_Bottom))
    {
        m_errorLog = _T("(Fail) Make Spec Land - Bottom");
        return false;
    }

    if (vec2fptLandCenter_Top.size() < 3 || vec2fptLandCenter_Bottom.size() < 3)
    {
        // Land 영역이 계산에 사용할 만큼 충분하지 않다
        return true;
    }

    o_selectedSearchObject = true;

    std::vector<Ipvm::Point32r2> vec2fptLandCenter_TopValid;
    std::vector<Ipvm::Rect32s> vec2rtSearchLand_TopValid;

    std::vector<Ipvm::Point32r2> vec2fptLandCenter_BottomValid;
    std::vector<Ipvm::Rect32s> vec2rtSearchLand_BottomValid;

    for (long idx1 = 0; idx1 < vec2LandIndex_Top.size(); idx1++)
    {
        for (long idx2 = 0; idx2 < vec2LandIndex_Bottom.size(); idx2++)
        {
            if (vec2LandIndex_Top[idx1] == vec2LandIndex_Bottom[idx2])
            {
                vec2fptLandCenter_TopValid.push_back(vec2fptLandCenter_Top[idx1]);
                vec2rtSearchLand_TopValid.push_back(vec2rtSearchLand_Top[idx1]);

                vec2fptLandCenter_BottomValid.push_back(vec2fptLandCenter_Bottom[idx2]);
                vec2rtSearchLand_BottomValid.push_back(vec2rtSearchLand_Bottom[idx2]);

                break;
            }
        }
    }

    // Find Bottom Obj
    if (!GetLandPlaneData(i_split3DImage, i_sfrtBottomPackage, i_rtBottomMatchingOverlap, vec2fptLandCenter_BottomValid,
            vec2rtSearchLand_BottomValid, vecBottom3Ddata_um, o_s3DBottomAverage_um, sBottomPlaneEq_um))
    {
        m_errorLog = _T("(Fail) Find Bottom Object");

        return false;
    }
    // }}

    // {{ Find Top Obj
    if (!GetLandPlaneData(i_ref3DImage, i_sfrtTopPackage, i_rtTopMatchingOverlap, vec2fptLandCenter_TopValid,
            vec2rtSearchLand_TopValid, vecTop3Ddata_um, o_s3DTopAverage_um, sTopPlaneEq_um))
    {
        m_errorLog = _T("(Fail) Find Top Object");

        return false;
    }
    // }}

    // Top plane 에서 Bottom plane 으로 얼마만큼 회전되었는지 계산
    // 왼손 좌표계이므로 Y 에 -1 을 곱해준다.
    float lfRadThetaZ = CAST_FLOAT(GetDeltaTheta(Ipvm::Point64r2(sTopPlaneEq_um.m_a, sTopPlaneEq_um.m_b),
        Ipvm::Point64r2(sBottomPlaneEq_um.m_a, sBottomPlaneEq_um.m_b)));
    float lfRadThetaY = CAST_FLOAT(-GetDeltaTheta(Ipvm::Point64r2(sTopPlaneEq_um.m_a, sTopPlaneEq_um.m_c),
        Ipvm::Point64r2(sBottomPlaneEq_um.m_a, sBottomPlaneEq_um.m_c)));
    float lfRadThetaX = CAST_FLOAT(GetDeltaTheta(Ipvm::Point64r2(sTopPlaneEq_um.m_b, sTopPlaneEq_um.m_c),
        Ipvm::Point64r2(sBottomPlaneEq_um.m_b, sBottomPlaneEq_um.m_c)));

    o_s3DTheta = Ipvm::Point64r3(lfRadThetaX, lfRadThetaY, lfRadThetaZ);

    return true;
}

bool VisionProcessingAlign3D::getNormalizePoints(const std::vector<Ipvm::Point64r3>& i_points,
    std::vector<Ipvm::Point64r3>& o_points, Ipvm::Point64r3& o_center, Ipvm::Point64r3& o_halfRange)
{
    if (i_points.size() == 0)
        return false;

    long count = long(i_points.size());

    double xMin = i_points[0].m_x;
    double xMax = i_points[0].m_x;
    double yMin = i_points[0].m_y;
    double yMax = i_points[0].m_y;
    double zMin = i_points[0].m_z;
    double zMax = i_points[0].m_z;

    for (long index = 1; index < count; index++)
    {
        xMin = min(xMin, i_points[index].m_x);
        xMax = max(xMax, i_points[index].m_x);
        yMin = min(yMin, i_points[index].m_y);
        yMax = max(yMax, i_points[index].m_y);
        zMin = min(zMin, i_points[index].m_z);
        zMax = max(zMax, i_points[index].m_z);
    }

    auto& xHalfRange = o_halfRange.m_x;
    auto& yHalfRange = o_halfRange.m_y;
    auto& zHalfRange = o_halfRange.m_z;

    auto& x_c = o_center.m_x;
    auto& y_c = o_center.m_y;
    auto& z_c = o_center.m_z;

    xHalfRange = (xMax - xMin) * 0.5;
    yHalfRange = (yMax - yMin) * 0.5;
    zHalfRange = (zMax - zMin) * 0.5;
    x_c = (xMax + xMin) * 0.5;
    y_c = (yMax + yMin) * 0.5;
    z_c = (zMax + zMin) * 0.5;

    o_points.resize(i_points.size());

    for (long index = 0; index < count; index++)
    {
        o_points[index].m_x = (i_points[index].m_x - x_c) / xHalfRange;
        o_points[index].m_y = (i_points[index].m_y - y_c) / yHalfRange;
        o_points[index].m_z = (i_points[index].m_z - z_c) / zHalfRange;
    }

    return true;
}

bool VisionProcessingAlign3D::calcAutoPaneOffsetX(
    const eRoughAlignUsingDefine i_eRoughAlignUsingDefine, double& o_paneOffsetX_um)
{
    if (getInspectionAreaInfo().m_fovList.size() == 0 || getInspectionAreaInfo().m_unitIndexList.size() == 0)
    {
        return false;
    }

    long paneIndex = GetCurrentPaneID();

    //Ipvm::Image8u imageHMap = getImageLot().GetImageFrame(0);
    Ipvm::Image8u RoughAlign_image = getImageLot().GetImageFrame(0, enSideVisionModule::SIDE_VISIONMODULE_FRONT);

    switch ((long)i_eRoughAlignUsingDefine)
    {
        case eRoughAlignUsingDefine::eRoughAlignUsingDefine_H_Map:
            RoughAlign_image = getImageLot().GetImageFrame(0, enSideVisionModule::SIDE_VISIONMODULE_FRONT);
            break;
        case eRoughAlignUsingDefine::eRoughAlignUsingDefine_V_Map:
            RoughAlign_image = getImageLot().GetImageFrame(1, enSideVisionModule::SIDE_VISIONMODULE_FRONT);
            break;
    }

    long rawSizeX = getImageLot().GetImageSizeX();
    long rawSizeY = getImageLot().GetImageSizeY();

    //-------------------------------------------------------------------------------
    // Profile 을 구할 영역을 계산한다
    // X방향은 Device의 Pocket 영역내에서 계산하며 Y방향을 모두를 이용한다
    //-------------------------------------------------------------------------------

    //Ipvm::Rect32s profileRect(imageHMap);
    Ipvm::Rect32s profileRect(RoughAlign_image);

    float unitRegionL = 0.f;
    float unitRegionR = 0.f;
    bool unitFound = false;

    //{{ //kircheis_ImproveRoughAlign
    auto& scale = getImageLot().GetScale(); ///
    int stitchCountX = getInspectionAreaInfo().m_stichCountX;
    int stitchCountY = getInspectionAreaInfo().m_stichCountY;
    long imageSizeX = getImageLot().GetImageSizeX();
    long imageSizeY = getImageLot().GetImageSizeY();
    int fovImageSizeX = (stitchCountX <= 0) ? 0 : imageSizeX / stitchCountX;
    int fovImageSizeY = (stitchCountY <= 0) ? 0 : imageSizeY / stitchCountY;
    long nTop = rawSizeY;
    long nBottom = 0;
    long nFovIndex = -1;
    //}}

    for (auto fovID : getInspectionAreaInfo().m_fovList)
    {
        nFovIndex++;
        auto unitIndex = getInspectionAreaInfo().m_unitIndexList[paneIndex];
        Ipvm::Rect32r unitRegion = getTrayScanSpec().GetUnitPocketRegionInFOV(fovID, unitIndex);

        if (unitRegion.IsRectEmpty())
            continue;

        unitFound = true;
        unitRegionL = rawSizeX * 0.5f + (unitRegion.m_left * getScale().mmToPixel().m_x);
        unitRegionR = rawSizeX * 0.5f + (unitRegion.m_right * getScale().mmToPixel().m_x);

        //{{ //kircheis_ImproveRoughAlign
        int stitchX = nFovIndex % stitchCountX;
        int stitchY = nFovIndex % stitchCountY;

        Ipvm::Rect32s fovImageRoi = Ipvm::Rect32s(stitchX * fovImageSizeX, stitchY * fovImageSizeY,
            (stitchX + 1) * fovImageSizeX, (stitchY + 1) * fovImageSizeY);

        Ipvm::Point32r2 fovCenter(
            (fovImageRoi.m_left + fovImageRoi.m_right) * 0.5f, (fovImageRoi.m_top + fovImageRoi.m_bottom) * 0.5f);

        auto pocketRegion = scale.convert_mmToPixel(getTrayScanSpec().GetUnitPocketRegionInFOV(fovID, unitIndex))
            + Ipvm::Conversion::ToPoint32s2(fovCenter);
        nTop = (long)min(nTop, pocketRegion.m_top);
        nBottom = (long)max(nBottom, pocketRegion.m_bottom);
        //}}
    }
    //{{ //kircheis_ImproveRoughAlign
    nTop = (long)max(nTop, 0);
    nBottom = (long)min(nBottom, rawSizeY);
    //}}

    if (!unitFound)
    {
        return false;
    }

    float unitCenterX = (unitRegionL + unitRegionR) * 0.5f;

    profileRect.m_left = (long)unitRegionL;
    profileRect.m_right = (long)unitRegionR;

    //profileRect &= Ipvm::Rect32s(imageHMap);
    profileRect &= Ipvm::Rect32s(RoughAlign_image);

    long profileCount = profileRect.Width();
    if (profileCount < 2)
    {
        return false;
    }

    //-------------------------------------------------------------------------------
    // Profile 이미지를 구한다
    //-------------------------------------------------------------------------------

    Ipvm::Image8u profileImage;
    if (!getReusableMemory().GetByteImage(profileImage, rawSizeX, 1))
    {
        return false;
    }

    //-------------------------------------------------------------------------------
    // Profile 이미지내에서 왼쪽, 오른쪽에서 Edge Detect을 해서 Device의 양쪽을 구한다
    //-------------------------------------------------------------------------------

    BYTE* profileXs = profileImage.GetMem();

    //{{ //kircheis_ImproveRoughAlign
    Ipvm::Rect32s profileRect2 = profileRect;
    profileRect2.m_top = nTop;
    profileRect2.m_bottom = nBottom;

    //Ipvm::ImageProcessing::MakeProjectionProfileAxisX(imageHMap, profileRect2, profileXs);
    Ipvm::ImageProcessing::MakeProjectionProfileAxisX(RoughAlign_image, profileRect2, profileXs);
    //Ipvm::ImageProcessing::MakeProjectionProfileAxisX(imageHMap, profileRect, profileXs);
    //}}

    //{{ //kircheis_ImproveRoughAlign
    Ipvm::Rect32r frtUnit = getTrayScanSpec().m_vecUnits[getInspectionAreaInfo().m_unitIndexList[paneIndex]].m_position;

    int nPackageSizeX_Pxl = (long)(getScale().convert_umToPixelX(m_packageSpec.m_bodyInfoMaster->fBodySizeX) + .5f);
    double dSum = 0;
    long nPaneSizeX = profileRect.Width();
    //bool bIsFindOK = false;
    Ipvm::Rect32s rtCalcROI = Ipvm::Rect32s(0, 0, nPackageSizeX_Pxl, 1);
    long nMaxIdx = 0;
    double dSumMax = -1.;

    std::vector<double> vecSum(0);

    while (rtCalcROI.m_right < nPaneSizeX)
    {
        Ipvm::ImageProcessing::GetSum(profileImage, rtCalcROI, dSum);
        vecSum.push_back(dSum);
        if (dSum > dSumMax)
        {
            dSumMax = dSum;
            nMaxIdx = rtCalcROI.m_left;
        }
        rtCalcROI.m_left++;
        rtCalcROI.m_right++;
    }
    long positiveEndID = 0;
    long negativeStartID = 0;
    if (FindIdFromProfileForRoughAlign(vecSum, positiveEndID, negativeStartID) == true)
    {
        nMaxIdx = (long)(float(positiveEndID + negativeStartID) * .5f + .5f);
    }
    float fNewProfileCenterX = (float)profileRect.m_left + (float)nMaxIdx + (float)(nPackageSizeX_Pxl * .5f);
    o_paneOffsetX_um += (fNewProfileCenterX - unitCenterX) * getScale().pixelToUmXY();
    return true;
    //}}

    //{{ //kircheis_ImproveRoughAlign
    //float edgeL, edgeR;
    //float edgeV;
    //BOOL subPixelingDone;
    //if (!getReusableMemory().GetEdgeDetect()->PI_ED_CalcEdgePointFromLine(
    //	PI_ED_DIR_RISING,
    //	profileCount / 2,
    //	profileXs,
    //	edgeL,
    //	edgeV,
    //	subPixelingDone))
    //{
    //	return false;
    //}

    //edgeL += profileRect.m_left;

    //if (!getReusableMemory().GetEdgeDetect()->PI_ED_CalcEdgePointFromLine(
    //	PI_ED_DIR_FALLING,
    //	profileCount - profileCount / 2,
    //	profileXs + profileCount / 2,
    //	edgeR,
    //	edgeV,
    //	subPixelingDone))
    //{
    //	return false;
    //}

    //edgeR += profileRect.m_left + profileCount / 2;

    ////-------------------------------------------------------------------------------
    //// Profile 이미지내에서 왼쪽, 오른쪽에서 Edge Detect을 해서 Device의 양쪽을 구한다
    ////-------------------------------------------------------------------------------

    //float profileCenterX = (edgeL + edgeR) * 0.5f;
    //o_paneOffsetX_um += (profileCenterX - unitCenterX) * getScale().pixelToUmXY();
    //}}

    return false;
}

bool VisionProcessingAlign3D::calcAutoPaneOffsetY(
    const eRoughAlignUsingDefine i_eRoughAlignUsingDefine, double& o_paneOffsetY_um)
{
    if (getInspectionAreaInfo().m_fovList.size() == 0 || getInspectionAreaInfo().m_unitIndexList.size() == 0)
    {
        return false;
    }

    long paneIndex = GetCurrentPaneID();

    //Ipvm::Image8u imageHMap = getImageLot().GetImageFrame(0);
    Ipvm::Image8u RoughAlign_image = getImageLot().GetImageFrame(0, enSideVisionModule::SIDE_VISIONMODULE_FRONT);

    switch ((long)i_eRoughAlignUsingDefine)
    {
        case eRoughAlignUsingDefine::eRoughAlignUsingDefine_H_Map:
            RoughAlign_image = getImageLot().GetImageFrame(0, enSideVisionModule::SIDE_VISIONMODULE_FRONT);
            break;
        case eRoughAlignUsingDefine::eRoughAlignUsingDefine_V_Map:
            RoughAlign_image = getImageLot().GetImageFrame(1, enSideVisionModule::SIDE_VISIONMODULE_FRONT);
            break;
    }

    long rawSizeX = getImageLot().GetImageSizeX();
    long rawSizeY = getImageLot().GetImageSizeY();

    //-------------------------------------------------------------------------------
    // Profile 을 구할 영역을 계산한다
    // X방향은 Device의 Pocket 영역내에서 계산하며 Y방향을 모두를 이용한다
    //-------------------------------------------------------------------------------

    //Ipvm::Rect32s profileRect(imageHMap);
    Ipvm::Rect32s profileRect(RoughAlign_image);

    float unitRegionL = 0.f;
    float unitRegionR = 0.f;
    bool unitFound = false;

    //{{ //kircheis_ImproveRoughAlign
    auto& scale = getImageLot().GetScale(); ///
    int stitchCountX = getInspectionAreaInfo().m_stichCountX;
    int stitchCountY = getInspectionAreaInfo().m_stichCountY;
    long imageSizeX = getImageLot().GetImageSizeX();
    long imageSizeY = getImageLot().GetImageSizeY();
    int fovImageSizeX = (stitchCountX <= 0) ? 0 : imageSizeX / stitchCountX;
    int fovImageSizeY = (stitchCountY <= 0) ? 0 : imageSizeY / stitchCountY;
    long nTop = rawSizeY;
    long nBottom = 0;
    long nUnitTop = rawSizeY;
    long nUnitBottom = 0;
    long nFovIndex = -1;
    //}}

    for (auto fovID : getInspectionAreaInfo().m_fovList)
    {
        nFovIndex++;
        Ipvm::Rect32r unitRegion
            = getTrayScanSpec().GetUnitPocketRegionInFOV(fovID, getInspectionAreaInfo().m_unitIndexList[paneIndex]);

        if (unitRegion.IsRectEmpty())
            continue;

        unitFound = true;
        unitRegionL = rawSizeX * 0.5f + (unitRegion.m_left * getScale().mmToPixel().m_x);
        unitRegionR = rawSizeX * 0.5f + (unitRegion.m_right * getScale().mmToPixel().m_x);

        //{{ //kircheis_ImproveRoughAlign
        int stitchX = nFovIndex % stitchCountX;
        int stitchY = nFovIndex % stitchCountY;

        Ipvm::Rect32s fovImageRoi = Ipvm::Rect32s(stitchX * fovImageSizeX, stitchY * fovImageSizeY,
            (stitchX + 1) * fovImageSizeX, (stitchY + 1) * fovImageSizeY);

        Ipvm::Point32r2 fovCenter(
            (fovImageRoi.m_left + fovImageRoi.m_right) * 0.5f, (fovImageRoi.m_top + fovImageRoi.m_bottom) * 0.5f);

        auto pocketRegion = scale.convert_mmToPixel(getTrayScanSpec().GetUnitPocketRegionInFOV(
                                fovID, getInspectionAreaInfo().m_unitIndexList[paneIndex]))
            + Ipvm::Conversion::ToPoint32s2(fovCenter);
        nTop = (long)min(nTop, pocketRegion.m_top);
        nBottom = (long)max(nBottom, pocketRegion.m_bottom);

        auto unitRegionPxl = scale.convert_mmToPixel(getTrayScanSpec().GetUnitRegionInFOV(
                                 fovID, getInspectionAreaInfo().m_unitIndexList[paneIndex]))
            + Ipvm::Conversion::ToPoint32s2(fovCenter);
        nUnitTop = (long)min(nUnitTop, unitRegionPxl.m_top);
        nUnitBottom = (long)max(nUnitBottom, unitRegionPxl.m_bottom);
        //}}
    }
    //{{ //kircheis_ImproveRoughAlign
    long nOutOfTopRange = nTop < 0
        ? -(nTop / 2)
        : 0; //Pane 영역이 FOV 밖으로 나가면 아래의 연산이 틀어진다. 이를 보상하기 위해 일단 저장

    if (nTop < 0 && nBottom >= rawSizeY) //추가 Code // FOV가 Pane을 커버하지 못할 정도면 nOutOfTopRange를 쓰면 안된다.
        nOutOfTopRange = 0;

    nTop = (long)max(nTop, 0);
    nBottom = (long)min(nBottom, rawSizeY);
    long nUnitSizeY = nUnitBottom - nUnitTop;
    //}}

    if (!unitFound)
    {
        return false;
    }

    //float unitCenterX = (unitRegionL + unitRegionR) * 0.5f;

    profileRect.m_left = (long)unitRegionL;
    profileRect.m_right = (long)unitRegionR;

    //profileRect &= Ipvm::Rect32s(imageHMap);
    profileRect &= Ipvm::Rect32s(RoughAlign_image);

    long profileCount = profileRect.Width();
    if (profileCount < 2)
    {
        return false;
    }

    //-------------------------------------------------------------------------------
    // Profile 이미지를 구한다
    //-------------------------------------------------------------------------------

    Ipvm::Image8u profileImage;
    if (!getReusableMemory().GetByteImage(profileImage, rawSizeY, 1))
    {
        return false;
    }

    //-------------------------------------------------------------------------------
    // Profile 이미지내에서 왼쪽, 오른쪽에서 Edge Detect을 해서 Device의 양쪽을 구한다
    //-------------------------------------------------------------------------------

    BYTE* profileYs = profileImage.GetMem();

    //{{ //kircheis_ImproveRoughAlign
    Ipvm::Rect32s profileRect2 = profileRect;
    profileRect2.m_top = nTop;
    profileRect2.m_bottom = nBottom;
    profileRect.m_top = nTop;
    profileRect.m_bottom = nBottom;

    Ipvm::ImageProcessing::MakeProjectionProfileAxisY(RoughAlign_image, profileRect2, profileYs);
    //Ipvm::ImageProcessing::MakeProjectionProfileAxisY(imageHMap, profileRect2, profileYs);
    //}}

    //{{ //kircheis_ImproveRoughAlign
    Ipvm::Rect32r frtUnit = getTrayScanSpec().m_vecUnits[getInspectionAreaInfo().m_unitIndexList[paneIndex]].m_position;

    int nPackageSizeY_Pxl = nUnitSizeY;
    double dSum = 0;
    long nPaneSizeY = profileRect.Height();
    //bool bIsFindOK = false;
    Ipvm::Rect32s rtCalcROI = Ipvm::Rect32s(0, 0, nPackageSizeY_Pxl, 1);
    long nMaxIdx = 0;
    double dSumMax = -1.;

    std::vector<double> vecdSum(0);

    while (rtCalcROI.m_right < nPaneSizeY)
    {
        Ipvm::ImageProcessing::GetSum(profileImage, rtCalcROI, dSum);
        vecdSum.push_back(dSum);
        if (dSum > dSumMax)
        {
            dSumMax = dSum;
            nMaxIdx = rtCalcROI.m_left;
        }
        rtCalcROI.m_left++;
        rtCalcROI.m_right++;
    }
    long positiveEndID = 0;
    long negativeStartID = 0;
    if (FindIdFromProfileForRoughAlign(vecdSum, positiveEndID, negativeStartID) == true)
    {
        nMaxIdx = (long)(float(positiveEndID + negativeStartID) * .5f + .5f);
    }
    float fNewProfileCenterY
        = (float)(profileRect.m_top + nOutOfTopRange) + (float)nMaxIdx + (float)(nPackageSizeY_Pxl * .5f);
    o_paneOffsetY_um += (fNewProfileCenterY - profileRect.CenterPoint().m_y) * getScale().pixelToUmXY();
    return true;
    //}}
}

bool VisionProcessingAlign3D::FindIdFromProfileForRoughAlign(
    const std::vector<double>& i_vecProfile, long& o_positiveSlopeEndID, long& o_negativeSlopeStartID)
{
    long profileCount = long(i_vecProfile.size());
    o_positiveSlopeEndID = 0;
    o_negativeSlopeStartID = 0;

    if (profileCount < 2)
    {
        return false;
    }

    //i_vecProfile에서 전방검색으로 현재 값 - 직전값이 0 이하가 되는 ID를 o_positiveSlopeEndID에 저장
    for (long nId = 1; nId < profileCount; nId++)
    {
        if (i_vecProfile[nId] - i_vecProfile[nId - 1] <= 0)
        {
            o_positiveSlopeEndID = nId - 1;
            break;
        }
    }

    //i_vecProfile에서 후방검색으로 현재 값 - 직전값이 0 이상이 되는 ID를 o_negativeSlopeStartID에 저장
    for (long nId = profileCount - 1; nId >= 0; nId--)
    {
        long LimitValue = nId - 1;
        if (LimitValue < 0)
            return false;

        if (i_vecProfile[nId] - i_vecProfile[nId - 1] >= 0)
        {
            o_negativeSlopeStartID = nId;
            break;
        }
    }

    if (o_negativeSlopeStartID == 0 && o_positiveSlopeEndID == 0)
        return false;
    if (o_positiveSlopeEndID > o_negativeSlopeStartID)
        return false;

    return true;
}
