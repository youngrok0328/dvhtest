//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "Inspection.h"

//CPP_2_________________________________ This project's headers
#include "Para.h"

//CPP_3_________________________________ Other projects' headers
#include "../../AlgorithmModules/dPI_EdgeDetect/dPI_EdgeDetectLib.h"
#include "../../AlgorithmModules/dPI_Geometry/dPI_GeometryLib.h"
#include "../../DefineModules/dA_Base/VisionScale.h"
#include "../../ImageCombineModules/dPI_ippModules/ippModules.h"
#include "../../InformationModule/dPI_DataBase/VisionTapeSpec.h"
#include "../../MemoryModules/VisionReusableMemory/SurfaceLayerMask.h"
#include "../../MemoryModules/VisionReusableMemory/VisionReusableMemory.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionImageLotInsp.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionProcessing.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Algorithm/DataFitting.h>
#include <Ipvm/Algorithm/Geometry.h>
#include <Ipvm/Base/CircleEq32r.h>
#include <Ipvm/Base/Enum.h>
#include <Ipvm/Base/LineEq32r.h>

//CPP_5_________________________________ Standard library headers
#include <numbers>

//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
class EdgeThresholdSelector
{
public:
    EdgeThresholdSelector() = delete;
    EdgeThresholdSelector(const EdgeThresholdSelector&) = delete;
    EdgeThresholdSelector& operator=(const EdgeThresholdSelector&) = delete;

    EdgeThresholdSelector(CPI_EdgeDetect* edgeDetector, const ParaEdgeParameters& edgeParams)
        : EdgeThresholdSelector(edgeDetector, edgeParams.m_firstEdgeMinThreshold)
    {
    }

    EdgeThresholdSelector(CPI_EdgeDetect* edgeDetector, float firstEdgeMinThreshold)
        : m_edgeDetector(edgeDetector)
        , m_oldThresholdValue(edgeDetector->SetMininumThreshold(firstEdgeMinThreshold))
    {
        float fCurEdgeThresh = edgeDetector->GetMininumThreshold();
        if (fCurEdgeThresh < 1.f)
        {
            float fDefaultEdgeThresh = 1.f;
            m_edgeDetector->SetMininumThreshold(fDefaultEdgeThresh);
        }
    }

    ~EdgeThresholdSelector()
    {
        if (m_oldThresholdValue != Ipvm::k_noiseValue32r)
        {
            m_edgeDetector->SetMininumThreshold(m_oldThresholdValue);
        }
    }

private:
    CPI_EdgeDetect* m_edgeDetector;
    float m_oldThresholdValue = Ipvm::k_noiseValue32r;
};

Inspection::Inspection(VisionProcessing& processor)
    : m_processor(processor)
{
}

void Inspection::ResetResult()
{
    // Get Algorithm...
    m_edgeDetector = m_processor.getReusableMemory().GetEdgeDetect();

    m_result.Init();
}

bool Inspection::Align(const bool detailSetupMode, const Para& para)
{
    const auto* tapeSpec = m_processor.getTapeSpec();
    if (tapeSpec == nullptr)
    {
        return false;
    }

    // Sprocket Hole Align
    if (!AlignSprocketHole(detailSetupMode, *tapeSpec, para))
    {
        return false;
    }

    // Pocket Align
    if (!AlignPocket(detailSetupMode, *tapeSpec, para))
    {
        return false;
    }

    // Device Align
    if (!AlignDevice(detailSetupMode, para))
    {
        return false;
    }

    // 현재 Align 기능은 완성되지 않았다
    return true;
}

bool Inspection::GetImageForAlign(const VisionInspFrameIndex& imageFrameIndex, const ImageProcPara& imageProc, Ipvm::Image8u& o_image)
{
    Ipvm::Image8u source = imageFrameIndex.getImage(false);
    if (source.GetMem() == nullptr)
    {
        return false;
    }

    if (!m_processor.getReusableMemory().GetInspByteImage(o_image))
    {
        return false;
    }

    if (!CippModules::GrayImageProcessingManage(
            m_processor.getReusableMemory(), &source, false, Ipvm::Rect32s(source), imageProc, o_image))
    {
        return false;
    }

    return true;
}

bool Inspection::AlignSprocketHole(const bool detailSetupMode, const VisionTapeSpec& tapeSpec, const Para& para)
{
    if (!MakeSpecSprocketHoleRois(detailSetupMode, tapeSpec))
    {
        return false;
    }

    if (!MakeSearchSprocketHoleRois(detailSetupMode, tapeSpec, para))
    {
        return false;
    }

    // Align을 위한 이미지를 준비한다
    Ipvm::Image8u image;
    if (!GetImageForAlign(para.m_sprocketHoleAlign.m_imageFrameIndex, para.m_sprocketHoleAlign.m_imageProc, image))
    {
        return false;
    }

    // 가장 먼저 Sprocket Hole Align 을 한다
    if (!FindSprocketHoleRois(image, detailSetupMode, para.m_sprocketHoleAlign))
    {
        return false;
    }

    CalculateSprocketOffset();

    return true;
}

bool Inspection::AlignPocket(const bool detailSetupMode, const VisionTapeSpec& tapeSpec, const Para& para)
{
    if (para.m_pocketAlign.m_skipAlign)
    {
        // Align 기능을 사용하지 않는다면 Image Center 기반으로 Pocket ROI를 만든다
        const auto& scale = m_processor.getScale();
        const auto& imageLotInsp = m_processor.getImageLotInsp();
        auto imageSizeX = imageLotInsp.GetImageSizeX();
        auto imageSizeY = imageLotInsp.GetImageSizeY();
        auto imageCenterX = static_cast<float>(imageSizeX) * 0.5f;
        auto imageCenterY = static_cast<float>(imageSizeY) * 0.5f;

        float pocketSizeX_px = scale.convert_umToPixelX(tapeSpec.m_b0 * 1000.f);
        float pocketSizeY_px = scale.convert_umToPixelY(tapeSpec.m_a0 * 1000.f);

        Ipvm::Rect32r specPocketRoi = {
            imageCenterX - pocketSizeX_px * 0.5f,
            imageCenterY - pocketSizeY_px * 0.5f,
            imageCenterX + pocketSizeX_px * 0.5f,
            imageCenterY + pocketSizeY_px * 0.5f};

        m_result.m_pocket.m_alignedPocket = Ipvm::Quadrangle32r{
            specPocketRoi.m_left, specPocketRoi.m_top,
            specPocketRoi.m_right, specPocketRoi.m_top,
            specPocketRoi.m_left, specPocketRoi.m_bottom,
            specPocketRoi.m_right, specPocketRoi.m_bottom};

        m_processor.SetDebugInfoItem(
            detailSetupMode, _DEBUG_INFO_TDA_POCKET_ALIGN_QRECT, m_result.m_pocket.m_alignedPocket, true);

        m_processor.SetDebugInfoItem(
            detailSetupMode, _DEBUG_INFO_TDA_POCKET_RECT, m_result.m_pocket.m_alignedPocket.GetExtRect(), true);

        auto& memory = m_processor.getReusableMemory();
        if (auto* surfaceRoi = memory.AddSurfaceLayerRoiClass(_T("Measured - Pocket Align")))
        {
            surfaceRoi->Reset();
            surfaceRoi->Add(m_result.m_pocket.m_alignedPocket);
        }

        return true;
    }

    MakeSpecPocketRoi(detailSetupMode, tapeSpec);

    if (!MakeSearchPocketRoi(detailSetupMode, tapeSpec, para))
    {
        return false;
    }

    // Align을 위한 이미지를 준비한다
    Ipvm::Image8u image;
    if (!GetImageForAlign(para.m_pocketAlign.m_imageFrameIndex, para.m_pocketAlign.m_imageProc, image))
    {
        return false;
    }

    // Pocket Align
    if (!FindPocketRoi(image, detailSetupMode, para.m_pocketAlign))
    {
        return false;
    }

    return true;
}

bool Inspection::AlignDevice(const bool detailSetupMode, const Para& para)
{
    MakeSpecDeviceRoi(detailSetupMode);

    if (!MakeSearchDeviceRoi(detailSetupMode, para))
    {
        return false;
    }

    // Align을 위한 이미지를 준비한다
    Ipvm::Image8u image;
    if (!GetImageForAlign(para.m_deviceAlign.m_imageFrameIndex, para.m_deviceAlign.m_imageProc, image))
    {
        return false;
    }

    if (!FindDeviceRoi(image, detailSetupMode, para.m_deviceAlign))
    {
        return false;
    }

    return true;
}

bool Inspection::MakeSpecSprocketHoleRois(const bool detailSetupMode, const VisionTapeSpec& tapeSpec)
{
    const auto& scale = m_processor.getScale();
    const auto& imageLotInsp = m_processor.getImageLotInsp();
    auto imageSizeX = imageLotInsp.GetImageSizeX();
    auto imageSizeY = imageLotInsp.GetImageSizeY();
    auto imageCenterX = static_cast<float>(imageSizeX) * 0.5f;
    auto imageCenterY = static_cast<float>(imageSizeY) * 0.5f;
    auto interestRangeY_px = scale.convert_umToPixelY((tapeSpec.m_p1 - tapeSpec.m_a0 * 0.5f) * 1000.f);

    // Tape Detail Align에서 Tape을 중앙에 배치해 놓았으므로
    // Tape Width의 절반만큼 이동하면 Tape의 끝이다
    auto tapeL_px = imageCenterX - scale.convert_umToPixelX(tapeSpec.m_w0 * 1000.f) * 0.5f;
    auto tapeR_px = imageCenterX + scale.convert_umToPixelX(tapeSpec.m_w0 * 1000.f) * 0.5f;

    // Tape 끝에서 여기서 E1만큼 이동하면 Sprocket Hole 중심이다
    auto leftSprocketX_px = tapeL_px + scale.convert_umToPixelX(tapeSpec.m_e1 * 1000.f);
    auto rightSprocketX_px = tapeR_px + scale.convert_umToPixelX(tapeSpec.m_e1 * 1000.f);

    auto sprocketPitch_px = scale.convert_umToPixelY(tapeSpec.m_p0 * 1000.f);
    auto sprocketDiameterX_px = scale.convert_umToPixelX(tapeSpec.m_d1 * 1000.f);
    auto sprocketDiameterY_px = scale.convert_umToPixelY(tapeSpec.m_d1 * 1000.f);

    if (sprocketPitch_px < 0.1f)
    {
        // Sprocket Pitch가 너무 작으면 Spec이 잘못된 것이다
        return false;
    }

    // Sprocket Hole Spec ROI 만들기
    // Spec ROI는 이미지 안에 들어오는 부분만 만든다 ==> 2025.09.11 다음 Pocket과 중첩되지 않게 만든다
    auto pocketPitchY_px = scale.convert_umToPixelY(tapeSpec.m_p1 * 1000.f);
    auto pocketSizeY_px = scale.convert_umToPixelY(tapeSpec.m_a0 * 1000.f);
    auto pocketGapY_px = pocketPitchY_px - pocketSizeY_px;
    if (pocketGapY_px < 0.1f)
    {
        // Pocket Gap이 너무 작으면 Spec이 잘못된 것이다
        return false;
    }
    interestRangeY_px = (pocketSizeY_px * .5f) + (pocketGapY_px * 0.5f);
    auto startY = imageCenterY - interestRangeY_px;
    auto endY = imageCenterY + interestRangeY_px;

    // 먼저 화면 최상단에 있는 구멍을 찾는다
    // image Center로 부터 sprocketPitch/2 간격으로 이동 후 sprocketPitch 간격으로 구멍을 찾는다
    auto sprocketY_px = imageCenterY - sprocketPitch_px * 0.5f;
    while (sprocketY_px > imageCenterY - interestRangeY_px)
    {
        sprocketY_px -= sprocketPitch_px;
    }

    // 첫 구멍의 중심점이 결정되었으니, 화면 안에 들어오는 구멍들을 모두 만든다
    m_result.m_sprocketHoles.clear();
    std::vector<Ipvm::Rect32r> specSprocketRois;

    while (sprocketY_px < imageCenterY + interestRangeY_px + sprocketDiameterY_px * 0.5f)
    {
        if (sprocketY_px < startY)
        {
            sprocketY_px += sprocketPitch_px;
            continue;
        }
        else if (sprocketY_px > endY)
        {
            break;
        }

        if (tapeSpec.m_sprocketHoleExistType == TapeSprocketHoleExistType::e_leftOnly ||
            tapeSpec.m_sprocketHoleExistType == TapeSprocketHoleExistType::e_both)
        {
            ResultSprocketHole sprocketHole;
            sprocketHole.m_specHoleSizeX_px = sprocketDiameterX_px;
            sprocketHole.m_specHoleSizeY_px = sprocketDiameterY_px;
            sprocketHole.m_specHoleRect_px = {
                leftSprocketX_px - sprocketDiameterX_px * 0.5f,
                sprocketY_px - sprocketDiameterY_px * 0.5f,
                leftSprocketX_px + sprocketDiameterX_px * 0.5f,
                sprocketY_px + sprocketDiameterY_px * 0.5f
            };

            m_result.m_sprocketHoles.push_back(sprocketHole);
            specSprocketRois.push_back(sprocketHole.m_specHoleRect_px);
        }

        if (tapeSpec.m_sprocketHoleExistType == TapeSprocketHoleExistType::e_rightOnly ||
            tapeSpec.m_sprocketHoleExistType == TapeSprocketHoleExistType::e_both)
        {
            ResultSprocketHole sprocketHole;
            sprocketHole.m_specHoleSizeX_px = sprocketDiameterX_px;
            sprocketHole.m_specHoleSizeY_px = sprocketDiameterY_px;
            sprocketHole.m_specHoleRect_px = {
                rightSprocketX_px - sprocketDiameterX_px * 0.5f,
                sprocketY_px - sprocketDiameterY_px * 0.5f,
                rightSprocketX_px + sprocketDiameterX_px * 0.5f,
                sprocketY_px + sprocketDiameterY_px * 0.5f
            };

            m_result.m_sprocketHoles.push_back(sprocketHole);
            specSprocketRois.push_back(sprocketHole.m_specHoleRect_px);
        }

        sprocketY_px += sprocketPitch_px;
    }

    m_processor.SetDebugInfoItem(detailSetupMode, _T("Spec Sprocket Hole ROIs"), specSprocketRois, true);

    return true;
}

void Inspection::MakeSpecPocketRoi(const bool detailSetupMode, const VisionTapeSpec& tapeSpec)
{
    const auto& scale = m_processor.getScale();
    const auto& imageLotInsp = m_processor.getImageLotInsp();
    auto imageSizeX = imageLotInsp.GetImageSizeX();
    auto imageSizeY = imageLotInsp.GetImageSizeY();
    auto imageCenterX = static_cast<float>(imageSizeX) * 0.5f;
    auto pocketCenterY = static_cast<float>(imageSizeY) * 0.5f;

    float pocketSizeX_px = scale.convert_umToPixelX(tapeSpec.m_b0 * 1000.f);
    float pocketSizeY_px = scale.convert_umToPixelY(tapeSpec.m_a0 * 1000.f);

    // Sprocket Hole Offset 적용
    auto offset_px = m_result.m_sprocketHoleOffset_px;

    // Sprocket Hole에서 F0만큼 떨어진 곳이 Pocket의 중심이다
    float pocketCenterX = 0.f;
    if (tapeSpec.m_sprocketHoleExistType == TapeSprocketHoleExistType::e_rightOnly)
    {
        auto tapeR_px = imageCenterX + scale.convert_umToPixelX(tapeSpec.m_w0 * 1000.f) * 0.5f + offset_px.m_x;
        auto sprocketR_px = tapeR_px - scale.convert_umToPixelX(tapeSpec.m_e1 * 1000.f);
        pocketCenterX = sprocketR_px - scale.convert_umToPixelX(tapeSpec.m_f0 * 1000.f);
    }
    else
    {
        auto tapeL_px = imageCenterX - scale.convert_umToPixelX(tapeSpec.m_w0 * 1000.f) * 0.5f + offset_px.m_x;
        auto sprocketL_px = tapeL_px + scale.convert_umToPixelX(tapeSpec.m_e1 * 1000.f);
        pocketCenterX = sprocketL_px + scale.convert_umToPixelX(tapeSpec.m_f0 * 1000.f);
    }

    Ipvm::Rect32r specPocketRoi
        = {
        pocketCenterX - pocketSizeX_px * 0.5f,
        pocketCenterY - pocketSizeY_px * 0.5f,
        pocketCenterX + pocketSizeX_px * 0.5f,
        pocketCenterY + pocketSizeY_px * 0.5f};

    m_result.m_pocket.m_specPocketSizeX_px = pocketSizeX_px;
    m_result.m_pocket.m_specPocketSizeY_px = pocketSizeY_px;
    m_result.m_pocket.m_specPocketRect_px = specPocketRoi;

    m_processor.SetDebugInfoItem(detailSetupMode, _T("Spec Pocket ROI"), specPocketRoi, true);
}

void Inspection::MakeSpecDeviceRoi(const bool detailSetupMode)
{
    // Device Search Center Point는 찾은 Pocket Center Point로 한다
    auto searchCenter = m_result.m_pocket.m_alignedPocket.CenterPoint();

    float deviceSizeX_px = 0.f;
    float deviceSizeY_px = 0.f;
    m_processor.GetPackageSize(true, deviceSizeX_px, deviceSizeY_px);
    
    Ipvm::Rect32r specDeviceRoi = {
        searchCenter.m_x - deviceSizeX_px * 0.5f,
        searchCenter.m_y - deviceSizeY_px * 0.5f,
        searchCenter.m_x + deviceSizeX_px * 0.5f,
        searchCenter.m_y + deviceSizeY_px * 0.5f};
    m_result.m_device.m_specPacketSizeX_px = deviceSizeX_px;
    m_result.m_device.m_specPacketSizeY_px = deviceSizeY_px;
    m_result.m_device.m_specPacketRect_px = specDeviceRoi;
    m_processor.SetDebugInfoItem(detailSetupMode, _T("Spec Device ROI"), specDeviceRoi, true);
}

bool Inspection::MakeSearchSprocketHoleRois(
    const bool detailSetupMode, const VisionTapeSpec& tapeSpec, const Para& para)
{
    auto& searchPara = para.m_sprocketHoleAlign;

    if (searchPara.m_edgeCount <= 0)
    {
        return false;
    }

    const auto& scale = m_processor.getScale();
    auto sprocketDiameterX_px = scale.convert_umToPixelX(tapeSpec.m_d1 * 1000.f);
    auto sprocketDiameterY_px = scale.convert_umToPixelY(tapeSpec.m_d1 * 1000.f);
    float maxSearchPecent = max(searchPara.m_persent_end, searchPara.m_persent_start);
    float searchSizeX = sprocketDiameterX_px * (maxSearchPecent / 100.f);
    float searchSizeY = sprocketDiameterY_px * (maxSearchPecent / 100.f);

    std::vector<Ipvm::Rect32r> searchSprocketRois;
    std::vector<Ipvm::LineSeg32r> searchSprocketLines;

    for (auto& sprocketHole : m_result.m_sprocketHoles)
    {
        auto holeCenter = sprocketHole.m_specHoleRect_px.CenterPoint();

        // Debug 표시용 Search ROI 만들기
        sprocketHole.m_searchHoleRect_px = {
            holeCenter.m_x - searchSizeX * 0.5f,
            holeCenter.m_y - searchSizeY * 0.5f,
            holeCenter.m_x + searchSizeX * 0.5f,
            holeCenter.m_y + searchSizeY * 0.5f};

        // Sprocket Hole을 찾기 위한 Search Line 만들기
        auto stepRadian = 2.f * std::numbers::pi_v<float> / static_cast<float>(searchPara.m_edgeCount);

        for (auto step = 0; step < searchPara.m_edgeCount; step++)
        {
            auto radian = stepRadian * static_cast<float>(step);

            auto lineStart = Ipvm::Point32r2{
                holeCenter.m_x + cosf(radian) * (sprocketDiameterX_px * 0.5f * (searchPara.m_persent_start / 100.f)),
                holeCenter.m_y + sinf(radian) * (sprocketDiameterY_px * 0.5f * (searchPara.m_persent_start / 100.f))};
            auto lineEnd = Ipvm::Point32r2{
                holeCenter.m_x + cosf(radian) * (sprocketDiameterX_px * 0.5f * (searchPara.m_persent_end / 100.f)),
                holeCenter.m_y + sinf(radian) * (sprocketDiameterY_px * 0.5f * (searchPara.m_persent_end / 100.f))};

            Ipvm::LineSeg32r searchLine;
            searchLine.m_sx = lineStart.m_x;
            searchLine.m_sy = lineStart.m_y;
            searchLine.m_ex = lineEnd.m_x;
            searchLine.m_ey = lineEnd.m_y;

            searchSprocketLines.push_back(searchLine);
            sprocketHole.m_searchHoleLines_px.push_back(searchLine);
        }

        searchSprocketRois.push_back(sprocketHole.m_searchHoleRect_px);
    }

    m_processor.SetDebugInfoItem(detailSetupMode, _DEBUG_INFO_TDA_SPROCKET_HOLE_SEARCH_ROI, searchSprocketRois);
    m_processor.SetDebugInfoItem(detailSetupMode, _DEBUG_INFO_TDA_SPROCKET_HOLE_SEARCH_LINE, searchSprocketLines);

    return true;
}

bool Inspection::MakeSearchPocketRoi(const bool detailSetupMode, const VisionTapeSpec& tapeSpec, const Para& para)
{
    auto& searchPara = para.m_pocketAlign;

    const auto& scale = m_processor.getScale();
    float pocketSizeX_px = scale.convert_umToPixelX(tapeSpec.m_b0 * 1000.f);
    float pocketSizeY_px = scale.convert_umToPixelY(tapeSpec.m_a0 * 1000.f);

    for (auto direction = 0; direction < 4; direction++)
    {
        auto& edgeSearchRoiPara = searchPara.m_edgeSearchRois[direction];

        auto serachRoi = edgeSearchRoiPara.ToRect(scale,
            direction,
            m_result.m_pocket.m_specPocketRect_px.CenterPoint(),
            pocketSizeX_px,
            pocketSizeY_px);

        m_result.m_pocket.m_searchPocketRects.push_back(serachRoi);
    }

    m_processor.SetDebugInfoItem(
        detailSetupMode, _DEBUG_INFO_TDA_POCKET_SEARCH_ROI, m_result.m_pocket.m_searchPocketRects);

    return true;
}

bool Inspection::MakeSearchDeviceRoi(const bool detailSetupMode, const Para& para)
{
    auto& searchPara = para.m_deviceAlign;
    const auto& scale = m_processor.getScale();
    float deviceSizeX_px = m_result.m_device.m_specPacketSizeX_px;
    float deviceSizeY_px = m_result.m_device.m_specPacketSizeY_px;
    auto searchCenter = m_result.m_device.m_specPacketRect_px.CenterPoint();
    
    for (auto direction = 0; direction < 4; direction++)
    {
        auto& edgeSearchRoiPara = searchPara.m_edgeSearchRois[direction];
        auto serachRoi = edgeSearchRoiPara.ToRect(scale,
            direction, searchCenter,
            deviceSizeX_px,
            deviceSizeY_px);
        m_result.m_device.m_searchPocketRects.push_back(serachRoi);
    }
    m_processor.SetDebugInfoItem(
        detailSetupMode, _DEBUG_INFO_TDA_DEVICE_SEARCH_ROI, m_result.m_device.m_searchPocketRects);
    return true;
}

bool Inspection::FindSprocketHoleRois(const Ipvm::Image8u& image, const bool detailSetupMode, const ParaSprocketHoleAlign& para)
{
    EdgeThresholdSelector selector(m_edgeDetector, para.m_firstEdgeMinThreshold);
    long edgeType = para.m_edgeType ? PI_ED_DIR_RISING : PI_ED_DIR_FALLING;
    bool success = true;

    std::vector<Ipvm::Point32r2> allEdgePoints;
    std::vector<Ipvm::EllipseEq32r> allFittedEllipses;

    for (auto& sprocketHole : m_result.m_sprocketHoles)
    {
        std::vector<Ipvm::Point32r2> edgePoints;

        for (const auto& line : sprocketHole.m_searchHoleLines_px)
        {
            Ipvm::Point32s2 startPoint{static_cast<long>(line.m_sx), static_cast<long>(line.m_sy)};
            Ipvm::Point32s2 endPoint{static_cast<long>(line.m_ex), static_cast<long>(line.m_ey)};

            Ipvm::Point32r2 edgePoint;

            if (para.m_edgeDetectMode == EdgeDetectMode_FirstEdge
                || para.m_edgeDetectMode == EdgeDetectMode_AutoDetect)
            {
                if (m_edgeDetector->PI_ED_CalcEdgePointFromImage(
                        edgeType, startPoint, endPoint, image, edgePoint, TRUE))
                {
                    edgePoints.push_back(edgePoint);
                }
            }

            if (para.m_edgeDetectMode == EdgeDetectMode_BestEdge
                || para.m_edgeDetectMode == EdgeDetectMode_AutoDetect)
            {
                if (m_edgeDetector->PI_ED_CalcEdgePointFromImage(
                        edgeType, startPoint, endPoint, image, edgePoint, FALSE))
                {
                    edgePoints.push_back(edgePoint);
                }
            }

            allEdgePoints.push_back(edgePoint);
        }

        Ipvm::CircleEq32r circle;
        if (Ipvm::DataFitting::FitToCircle(long(edgePoints.size()), edgePoints.data(), circle)
            != Ipvm::Status::e_ok)
        {
            success = false;
            continue;
        }

        sprocketHole.m_alignedHole.m_x = circle.m_x;
        sprocketHole.m_alignedHole.m_y = circle.m_y;
        sprocketHole.m_alignedHole.m_xradius = circle.m_radius;
        sprocketHole.m_alignedHole.m_yradius = circle.m_radius;

        sprocketHole.m_alignedHoleROI.m_left = static_cast<int32_t>(circle.m_x - circle.m_radius + .5f);
        sprocketHole.m_alignedHoleROI.m_right = static_cast<int32_t>(circle.m_x + circle.m_radius + .5f);
        sprocketHole.m_alignedHoleROI.m_top = static_cast<int32_t>(circle.m_y - circle.m_radius + .5f);
        sprocketHole.m_alignedHoleROI.m_bottom = static_cast<int32_t>(circle.m_y + circle.m_radius + .5f);

        allFittedEllipses.push_back(sprocketHole.m_alignedHole);
    }

    m_processor.SetDebugInfoItem(detailSetupMode, _DEBUG_INFO_TDA_SPROCKET_HOLE_EDGE_POINT, allEdgePoints, true);
    m_processor.SetDebugInfoItem(detailSetupMode, _DEBUG_INFO_TDA_SPROCKET_HOLE_CIRCLE, allFittedEllipses, true);

    auto& memory = m_processor.getReusableMemory();
    if (auto* surfaceRoi = memory.AddSurfaceLayerRoiClass(_T("Measured - Sprocket Align")))
    {
        surfaceRoi->Reset();
        for (const auto& ellipse : allFittedEllipses)
        {
            surfaceRoi->Add(ellipse);
        }
    }

    return success;
}

bool Inspection::FindPocketRoi(const Ipvm::Image8u& image, const bool detailSetupMode, const ParaPocketAlign& para)
{
    if (para.m_skipAlign)
    {
        // Align 기능을 사용하지 않는다면 Spec ROI를 그대로 사용한다
        m_result.m_pocket.m_alignedPocket = Ipvm::Quadrangle32r{
            m_result.m_pocket.m_specPocketRect_px.m_left,
            m_result.m_pocket.m_specPocketRect_px.m_top,
            m_result.m_pocket.m_specPocketRect_px.m_right,
            m_result.m_pocket.m_specPocketRect_px.m_top,
            m_result.m_pocket.m_specPocketRect_px.m_left,
            m_result.m_pocket.m_specPocketRect_px.m_bottom,
            m_result.m_pocket.m_specPocketRect_px.m_right,
            m_result.m_pocket.m_specPocketRect_px.m_bottom
            };
    }
    else
    {
        if (!FindRectangleUsingEdgeDetection(
            image,
            para.m_edgeParams,
            m_result.m_pocket.m_searchPocketRects,
            m_result.m_pocket.m_edgePointsTotal,
            m_result.m_pocket.m_edgePointsFiltered,
            m_result.m_pocket.m_alignedPocket))
        {
            return false;
        }
    }

    m_processor.SetDebugInfoItem(
        detailSetupMode, _DEBUG_INFO_TDA_POCKET_EDGE_POINT, m_result.m_pocket.m_edgePointsFiltered);

    m_processor.SetDebugInfoItem(
        detailSetupMode, _DEBUG_INFO_TDA_POCKET_ALIGN_QRECT, m_result.m_pocket.m_alignedPocket, true);

    m_processor.SetDebugInfoItem(
        detailSetupMode, _DEBUG_INFO_TDA_POCKET_RECT, m_result.m_pocket.m_alignedPocket.GetExtRect(), true);

    auto& memory = m_processor.getReusableMemory();
    if (auto* surfaceRoi = memory.AddSurfaceLayerRoiClass(_T("Measured - Pocket Align")))
    {
        surfaceRoi->Reset();
        surfaceRoi->Add(m_result.m_pocket.m_alignedPocket);
    }

    return true;
}

bool Inspection::FindDeviceRoi(const Ipvm::Image8u& image, const bool detailSetupMode, const ParaDeviceAlign& para)
{
    if (!FindRectangleUsingEdgeDetection(
        image,
        para.m_edgeParams,
        m_result.m_device.m_searchPocketRects,
        m_result.m_device.m_edgePointsTotal,
        m_result.m_device.m_edgePointsFiltered,
        m_result.m_device.m_alignedPacket))
    {
        return false;
    }

    m_processor.SetDebugInfoItem(
        detailSetupMode, _DEBUG_INFO_TDA_DEVICE_EDGE_POINT, m_result.m_device.m_edgePointsFiltered);

    m_processor.SetDebugInfoItem(
        detailSetupMode, _DEBUG_INFO_TDA_DEVICE_ALIGN_QRECT, m_result.m_device.m_alignedPacket, true);

    auto& memory = m_processor.getReusableMemory();
    if (auto* surfaceRoi = memory.AddSurfaceLayerRoiClass(_T("Measured - Device Align")))
    {
        surfaceRoi->Reset();
        surfaceRoi->Add(m_result.m_device.m_alignedPacket);
    }

    return true;
}

bool Inspection::FindRectangleUsingEdgeDetection(const Ipvm::Image8u& image, const ParaEdgeParameters& edgeParams,
    const std::vector<Ipvm::Rect32s>& edgeSearchRois, std::vector<Ipvm::Point32r2>& edgePointsTotal,
    std::vector<Ipvm::Point32r2>& edgePointsFiltered, Ipvm::Quadrangle32r& alignedRect)
{
    if (edgeSearchRois.size() != 4)
    {
        return false;
    }

    EdgeThresholdSelector selector(m_edgeDetector, edgeParams);

    bool success = true;
    std::array<Ipvm::LineEq32r, 4> edgeLines;

    for (auto direction = 0; direction < 4; direction++)
    {
        auto rtROI = edgeSearchRois[direction] & Ipvm::Rect32s(image);

        std::vector<Ipvm::Point32r2> edgeDirPointsTotal;
        std::vector<Ipvm::Point32r2> edgeDirPoints;

        if (!EdgeDetectRoi(image, edgeParams, rtROI, direction, edgeDirPointsTotal, edgeDirPoints))
        {
            success = false;
        }

        // Edge Point로 Line 구하기
        if (Ipvm::DataFitting::FitToLineRn(
            long(edgeDirPoints.size()), edgeDirPoints.data(), 3.f, edgeLines[direction])
                != Ipvm::Status::e_ok)
        {
            return false;
        }

        edgePointsTotal.insert(edgePointsTotal.end(), edgeDirPointsTotal.begin(), edgeDirPointsTotal.end());
        edgePointsFiltered.insert(edgePointsFiltered.end(), edgeDirPoints.begin(), edgeDirPoints.end());
    }

    // 직선 방정식으로부터 네 모서리를 계산한다.
    if (success)
    {
        Ipvm::Point32r2 ptLeftTop;
        Ipvm::Point32r2 ptRightTop;
        Ipvm::Point32r2 ptRightBottom;
        Ipvm::Point32r2 ptLeftBottom;

        if (Ipvm::Geometry::GetCrossPoint(edgeLines[LEFT], edgeLines[UP], ptLeftTop) != Ipvm::Status::e_ok)
            return false;
        if (Ipvm::Geometry::GetCrossPoint(edgeLines[UP], edgeLines[RIGHT], ptRightTop) != Ipvm::Status::e_ok)
            return false;
        if (Ipvm::Geometry::GetCrossPoint(edgeLines[RIGHT], edgeLines[DOWN], ptRightBottom) != Ipvm::Status::e_ok)
            return false;
        if (Ipvm::Geometry::GetCrossPoint(edgeLines[DOWN], edgeLines[LEFT], ptLeftBottom) != Ipvm::Status::e_ok)
            return false;

        alignedRect = {ptLeftTop.m_x, ptLeftTop.m_y, ptRightTop.m_x, ptRightTop.m_y, ptLeftBottom.m_x, ptLeftBottom.m_y,
            ptRightBottom.m_x, ptRightBottom.m_y};
    }

    return success;
}

void Inspection::CalculateSprocketOffset()
{
    // Sprocket Hole들이 Spec 위치에서 얼마나 벗어났는지 계산하여
    // 평균 오프셋을 구한다. 이 오프셋은 Pocket Align을 할 때 사용된다

    Ipvm::Point32r2 offsetSum{0.f, 0.f};
    int32_t offsetSumCount = 0;

    for (const auto& sprocketHole : m_result.m_sprocketHoles)
    {
        auto specCenter = sprocketHole.m_specHoleRect_px.CenterPoint();
        const auto& alignedHole = sprocketHole.m_alignedHole;

        auto offsetX = alignedHole.m_x - specCenter.m_x;
        auto offsetY = alignedHole.m_y - specCenter.m_y;
        offsetSum.m_x += offsetX;
        offsetSum.m_y += offsetY;
        offsetSumCount++;
    }

    if (offsetSumCount > 0)
    {
        m_result.m_sprocketHoleOffset_px.m_x = offsetSum.m_x / static_cast<float>(offsetSumCount);
        m_result.m_sprocketHoleOffset_px.m_y = offsetSum.m_y / static_cast<float>(offsetSumCount);
    }
    else
    {
        m_result.m_sprocketHoleOffset_px.m_x = 0.f;
        m_result.m_sprocketHoleOffset_px.m_y = 0.f;
    }
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
            ASSERT(!"Unknown Edge Detect Mode");
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
    float lLimitDistance = (nDirection == LEFT || nDirection == RIGHT)
        ? scale.convert_umToPixelX(filteringDistanceUm)
        : scale.convert_umToPixelY(filteringDistanceUm); //일단은 이렇게 써보자

    if (!CPI_Geometry::RoughLineFitting(
        pointCount,
        useVectorXY->data(),
        line,
        5,
        lLimitDistance,
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
