//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionProcessingCarrierTapeAlign.h"

//CPP_2_________________________________ This project's headers
#include "DlgVisionProcessingCarrierTapeAlign.h"
#include "Inspection.h"

//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/PI_RECT.h"
#include "../../DefineModules/dA_Base/iDataType.h"
#include "../../ImageCombineModules/dPI_ippModules/ippModules.h"
#include "../../InformationModule/dPI_DataBase/PackageSpec.h"
#include "../../InformationModule/dPI_DataBase/VisionTapeSpec.h"
#include "../../InformationModule/dPI_DataBase/VisionTrayScanSpec.h"
#include "../../MemoryModules/VisionReusableMemory/SurfaceLayerMask.h"
#include "../../MemoryModules/VisionReusableMemory/VisionReusableMemory.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionAlignResult.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionImageLot.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionImageLotInsp.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionUnitAgent.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Algorithm/ImageProcessing.h>
#include <Ipvm/Base/Conversion.h>
#include <Ipvm/Base/Enum.h>
#include <Ipvm/Base/ImageFile.h>
#include <Ipvm/Base/Polygon32r.h>
#include <Ipvm/Base/Size32s2.h>
#include <Ipvm/Base/TimeCheck.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//#define USE_VIRTUAL_TRAY_SCAN_SPEC

//CPP_7_________________________________ Implementation body
//
BOOL VisionProcessingCarrierTapeAlign::OnInspection()
{
    m_pVisionInspDlg->OnBnClickedButtonInspect();
    return TRUE;
}

BOOL VisionProcessingCarrierTapeAlign::DoInspection(const bool detailSetupMode, const enSideVisionModule i_ModuleStatus)
{
    //mc_Side Vision
    SetCurVisionModule_Status(i_ModuleStatus);
    //

#ifdef USE_VIRTUAL_TRAY_SCAN_SPEC
    if (auto* tapeSpec = getTapeSpec())
    {
        // TrayScanSpec을 테스트를 위해 임시로 만든다
        // FOV 내에 Pocket이 1개만 있도록 강제한다.
        auto& trayScanSpec = getTrayScanSpec();
        auto marginX_mm = 1.f;
        auto marginY_mm = 1.f;
        trayScanSpec.m_pocketNumX = 1;
        trayScanSpec.m_pocketNumY = 1000;
        trayScanSpec.m_pocketPitchX_mm = tapeSpec->m_w0 + marginX_mm * 2; // Carrier Tape Width
        trayScanSpec.m_pocketPitchY_mm
            = tapeSpec->m_p1 * 2.f - tapeSpec->m_a0 + marginY_mm * 2; // Pocket Pitch - Pocket Size Y
        trayScanSpec.m_vecUnits.clear();
        trayScanSpec.m_vecFovInfo.clear();

        for (auto unitIndex = 0; unitIndex < trayScanSpec.m_pocketNumY; unitIndex++)
        {
            // 한 Fov에 한개의 Pocket만 포함하도록 함. 그러므로 fovIndex == unitIndex
            auto fovIndex = unitIndex;

            TrayScanSpec::UnitInfo unitInfo;
            unitInfo.m_position = {0.f, 0.f, 0.f, 0.f}; // Not used
            unitInfo.m_fovList.push_back(fovIndex);
            unitInfo.m_stitchFovList.push_back({fovIndex}); // 1x1 Stitch
            trayScanSpec.m_vecUnits.push_back(unitInfo);

            TrayScanSpec::IncludeUnitInfo includeUnitInfo;
            includeUnitInfo.m_unitID = unitIndex;
            includeUnitInfo.m_unitInFOV = {0.f, 0.f, 0.f, 0.f}; // Not used

            TrayScanSpec::FOVInfo fovInfo;
            fovInfo.m_fovCenter = {0.f, 0.f}; // Not used
            fovInfo.m_vecIncludeUnitInfo.push_back(includeUnitInfo);
            trayScanSpec.m_vecFovInfo.push_back(fovInfo);
        }
    }
#endif

    Ipvm::TimeCheck TimeCheck;

    if (!GetConstants())
    {
        return FALSE;
    }

    ResetResult();
    m_processingState.SetNum(2);

    //{{ //kircheis_3DEmpty //검사 결과를 관리하는 민감한 변수니까 무조건 시작할 때 초기화 한다.
    m_bInvalid = FALSE;
    m_bEmpty = FALSE;
    m_bMarginal = FALSE;
    m_bDoubleDevice = FALSE;
    //}}

    m_fCalcTime = 0.f;

    bool success = true;

    if (getImageLot().GetImageFrameCount() == 0)
    {
        return FALSE;
    }

    Ipvm::Rect32s alignImageRoi(getImageLot().GetImageFrame(0, GetCurVisionModule_Status()));

    if (!m_visionPara.m_skipEdgeAlign)
    {
        Ipvm::Image8u alignCarrierTapeImage;
        Ipvm::Image8u alignPocketImage;

        alignImageRoi = m_inspector->getImageSourceRoiForNoStitch();

        success &= m_inspector->GetImageForAlign(m_visionPara.m_carrierTapeAlign.m_imageFrameIndex,
            m_visionPara.m_carrierTapeAlign.m_imageProc, alignCarrierTapeImage);

        success &= m_inspector->GetImageForAlign(
            m_visionPara.m_pocketAlign.m_imageFrameIndex, m_visionPara.m_pocketAlign.m_imageProc, alignPocketImage);

        if (success)
        {
            Ipvm::Point32r2 imageCenter{
                alignCarrierTapeImage.GetSizeX() * 0.5f, alignCarrierTapeImage.GetSizeY() * 0.5f};

            m_inspector->GetResult().SetImage(alignCarrierTapeImage, alignPocketImage, imageCenter);
        }
    }

    m_processingState.SetState(0,
        success ? VisionProcessingState::enumState::Active_Success : VisionProcessingState::enumState::Active_Invalid);

    if (success)
    {
        if (!m_visionPara.m_skipEdgeAlign)
        {
            if (!m_inspector->run(m_visionPara, detailSetupMode))
            {
                success = false;
            }

            if (m_inspector->GetResult().m_inspectioned)
            {
                if (m_inspector->GetResult().m_success)
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
            auto& alignResult = m_inspector->GetResult();

            Ipvm::Point32r2 center{getImageLot().GetImageSizeX() * 0.5f, getImageLot().GetImageSizeY() * 0.5f};
            alignResult.Init(center);
        }
    }

    if (success)
    {
        // 영상을 회전시켜준다.
        success = AlignAllImages(alignImageRoi, detailSetupMode);

        if (success)
        {
            getImageLotInsp().m_paneIndexForCalculationImage = GetCurrentPaneID();
        }

        const auto& alignResult = m_inspector->GetResult();

        Ipvm::Point32r2 shift{getReusableMemory().GetInspImageSizeX() * 0.5f - alignResult.m_center.m_x,
            getReusableMemory().GetInspImageSizeY() * 0.5f - alignResult.m_center.m_y};

        m_inspector->SetGlobalAlignResult_ZeroAngle(shift);
    }
    else
    {
        //------------------------------------------------------------------------------------
        // Align 실패시나 Empty 일때 뒤에 검사설정탭에서 이전이미지가 보이지 않게
        // 기존 검사 이미지를 지워준다
        //------------------------------------------------------------------------------------

        long inspImageSizeX = getReusableMemory().GetInspImageSizeX();
        long inspImageSizeY = getReusableMemory().GetInspImageSizeY();

        //mc_Image를 Open하지 않고 아래의 작업을 수행하려고 했을 때 Crash 방지
        for (auto& dstImage : getImageLotInsp().m_vecImages[GetCurVisionModule_Status()])
        {
            dstImage.Create(inspImageSizeX, inspImageSizeY);
            Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(dstImage), 0, dstImage);
        }

        //------------------------------------------------------------------------------------

        m_inspector->SetGlobalAlignResult();
    }

    m_fCalcTime = (float)TimeCheck.Elapsed_ms();

    return success;
}

bool VisionProcessingCarrierTapeAlign::GetConstants()
{
    const auto* tapeSpec = getTapeSpec();
    if (tapeSpec == nullptr)
    {
        return false;
    }

    if (!m_constants.Make())
    {
        return false;
    }

    static const float fMargin = 10000.f;
    float inspMargeX = getScale().convert_umToPixelX(fMargin);
    float inspMargeY = getScale().convert_umToPixelY(fMargin);

    float carrierTapeWidth_um = tapeSpec->m_w0 * 1000.f;
    float pocketSizeY_um = tapeSpec->m_a0 * 1000.f;
    float pocketPitchY_um = tapeSpec->m_p1 * 1000.f;
    float inspSizeX_um = carrierTapeWidth_um; // Carrier Tape 폭
    float inspSizeY_um = pocketPitchY_um * 2.f - pocketSizeY_um;

    auto inspSizeX = (long)(getScale().convert_umToPixelX(inspSizeX_um) + inspMargeX);
    auto inspSizeY = (long)(getScale().convert_umToPixelY(inspSizeY_um) + inspMargeY);

    // 양쪽 균일하게 만들기 위해서 4의 배수로 만들자
    inspSizeX = (inspSizeX / 4) * 4;
    inspSizeY = (inspSizeY / 4) * 4;

    getReusableMemory().SetInspImageSize(inspSizeX, inspSizeY);

    return true;
}

BOOL VisionProcessingCarrierTapeAlign::AlignAllImages(
    const Ipvm::Rect32s& alignImageRoi, const bool /*detailSetupMode*/)
{
    Ipvm::TimeCheck TimeCheck;

    // Align Angle 과 Device 중심위치를 가져온다.
    const auto& alignResult = m_inspector->GetResult();

    const double RadToDeg = 180 / ITP_PI;
    auto alignedAngle_deg = (float)(alignResult.m_angle_rad * RadToDeg);
    Ipvm::Point32r2 fptAlignedCenter = alignResult.m_center;

    // Align은 Raw Pane 영역을 Cutting해서 진행했으므로 Pane영역을 더해주자
    fptAlignedCenter.m_x += static_cast<float>(alignImageRoi.m_left);
    fptAlignedCenter.m_y += static_cast<float>(alignImageRoi.m_top);

    // Device Center를 화면 정 중앙으로 이동시킨다.
    int inspImageSizeX = getReusableMemory().GetInspImageSizeX();
    int inspImageSizeY = getReusableMemory().GetInspImageSizeY();
    Ipvm::Point32r2 rawImageCenter{getImageLot().GetImageSizeX() * 0.5f, getImageLot().GetImageSizeY() * 0.5f};
    Ipvm::Point32r2 inspImageCenter{inspImageSizeX * 0.5f, inspImageSizeY * 0.5f};
    Ipvm::Point32r2 fptAlignedShift = inspImageCenter - fptAlignedCenter;

    long paneID = GetCurrentPaneID();
    auto& scale = getImageLot().GetScale();

    Ipvm::Rect32s rawPaneRoi(0, 0, 0, 0);
    const auto& inspectionAreaInfo = getInspectionAreaInfo();

    // Pane 영역 얻기
    for (auto& fovIndex : inspectionAreaInfo.m_fovList)
    {
        if (paneID >= 0 && paneID < (long)inspectionAreaInfo.m_unitIndexList.size())
        {
            long unitID = inspectionAreaInfo.m_unitIndexList[paneID];
            auto pocketRegion
                = scale.convert_mmToPixel(inspectionAreaInfo.m_parent->GetUnitPocketRegionInFOV(fovIndex, unitID))
                + Ipvm::Conversion::ToPoint32s2(rawImageCenter);
            rawPaneRoi |= pocketRegion;
        }
    }

    if (rawPaneRoi.IsRectEmpty())
    {
        return FALSE;
    }

    auto inspRoi = Ipvm::Rect32s(0, 0, inspImageSizeX, inspImageSizeY);

    for (long i = 0; i < getImageLot().GetImageFrameCount(); i++)
    {
        if (i > (long)getImageLotInsp().m_vecImages[GetCurVisionModule_Status()].size())
            continue;

        Ipvm::Image8u curImageStitch;
        const auto& curRawImage = getImageLot().GetImageFrame(i, GetCurVisionModule_Status());

        curImageStitch.Create(inspImageSizeX, inspImageSizeY);
        curImageStitch.FillZero();

        auto error = Ipvm::ImageProcessing::RotateLinearInterpolation(
            curRawImage, rawPaneRoi, fptAlignedCenter, -alignedAngle_deg, fptAlignedShift, curImageStitch);

        VERIFY(error == Ipvm::Status::e_ok);

        // 검사 영역만 실제 검사영상으로 남기자
        getImageLotInsp().m_vecImages[GetCurVisionModule_Status()][i] = Ipvm::Image8u(curImageStitch, inspRoi);
    }

    [[maybe_unused]] auto calcTime = (float)TimeCheck.Elapsed_ms();

    return TRUE;
}
