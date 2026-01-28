//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "UnitAreaInfo.h"

//CPP_2_________________________________ This project's headers
#include "VisionImageLot.h"
#include "VisionUnitAgent.h"

//CPP_3_________________________________ Other projects' headers
#include "../../InformationModule/dPI_DataBase/VisionTrayScanSpec.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Base/Conversion.h>
#include <Ipvm/Base/Image8u.h>
#include <Ipvm/Base/Point32r2.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
UnitAreaInfo::UnitAreaInfo(VisionUnitAgent* parent)
    : m_parent(parent)
{
}

UnitAreaInfo::~UnitAreaInfo()
{
}

Ipvm::Point32r2 UnitAreaInfo::GetUnitCenterBasedOnFirstImageFOV() const
{
    return GetUnitCenterBasedOnFOV(m_parent->getInspectionAreaInfo().m_fovList[0]);
}

Ipvm::Point32r2 UnitAreaInfo::GetUnitCenterBasedOnFOV(long fovID) const
{
    const auto& trayScanSpec = m_parent->getTrayScanSpec();
    const auto& inspectionAreaInfo = m_parent->getInspectionAreaInfo();
    const auto& scale = m_parent->getScale();
    const auto& imageLot = m_parent->getImageLot();
    long paneIndex = m_parent->GetCurrentPaneID();

    long fovIndex = GetFovIDtoFOVIndex(fovID);
    long stitchX = fovIndex % inspectionAreaInfo.m_stichCountX;
    long stitchY = fovIndex % inspectionAreaInfo.m_stichCountY;

    long unitID = inspectionAreaInfo.m_unitIndexList[paneIndex];
    long fovSizeX = imageLot.GetImageSizeX() / inspectionAreaInfo.m_stichCountX;
    long fovSizeY = imageLot.GetImageSizeX() / inspectionAreaInfo.m_stichCountY;
    Ipvm::Rect32s fovImageRegion(int32_t(stitchX * fovSizeX), int32_t(stitchY * fovSizeY),
        int32_t((stitchX + 1) * fovSizeX), int32_t((stitchY + 1) * fovSizeY));

    Ipvm::Point32r2 fovImageCenter((fovImageRegion.m_left + fovImageRegion.m_right) * 0.5f,
        (fovImageRegion.m_top + fovImageRegion.m_bottom) * 0.5f);

    return Ipvm::Conversion::ToPoint32r2(scale.convert_mmToPixel(trayScanSpec.GetUnitCenterInFOV(fovID, unitID)))
        + fovImageCenter;
}

void UnitAreaInfo::AbsoluteROItoImageROI_Raw(Ipvm::Rect32s& rtROI) const
{
    Ipvm::Point32r2 ptPaneCenter = GetUnitCenterBasedOnImage();

    rtROI.m_left += (long)ptPaneCenter.m_x;
    rtROI.m_right += (long)ptPaneCenter.m_x;
    rtROI.m_top += (long)ptPaneCenter.m_y;
    rtROI.m_bottom += (long)ptPaneCenter.m_y;
}

void UnitAreaInfo::ImageROItoAbsoluteROI_Raw(Ipvm::Rect32s& rtROI) const
{
    Ipvm::Point32r2 ptPaneCenter = GetUnitCenterBasedOnImage();

    rtROI.m_left -= (long)ptPaneCenter.m_x;
    rtROI.m_right -= (long)ptPaneCenter.m_x;
    rtROI.m_top -= (long)ptPaneCenter.m_y;
    rtROI.m_bottom -= (long)ptPaneCenter.m_y;
}

long UnitAreaInfo::GetFovIDtoFOVIndex(long fovID) const
{
    const auto& inspectionAreaInfo = m_parent->getInspectionAreaInfo();

    long fovCount = (long)inspectionAreaInfo.m_fovList.size();

    for (long fovIndex = 0; fovIndex < fovCount; fovIndex++)
    {
        if (inspectionAreaInfo.m_fovList[fovIndex] == fovID)
        {
            return fovIndex;
        }
    }

    return -1;
}

Ipvm::Point32r2 UnitAreaInfo::GetUnitCenterBasedOnImage() const
{
    // 과거 Image Center기준으로 Pane ROI를 만들었었기 때문에 Recipe 호환성을 위해 해당 코드는 그냥 그대로 유지한다

    const auto& trayScanSpec = m_parent->getTrayScanSpec();
    const auto& inspectionAreaInfo = m_parent->getInspectionAreaInfo();
    const auto& scale = m_parent->getScale();
    const auto& imageLot = m_parent->getImageLot();
    long paneIndex = m_parent->GetCurrentPaneID();

    if (inspectionAreaInfo.m_fovList.size() == 0)
    {
        ASSERT(!_T("FOV 가 제대로 만들어 지지 않았다."));
        return Ipvm::Point32r2{};
    }

    long unitID = inspectionAreaInfo.m_unitIndexList[paneIndex];
    Ipvm::Rect32s imageRegion(0, 0, imageLot.GetImageSizeX(), imageLot.GetImageSizeY());

    Ipvm::Point32r2 imageCenter(imageRegion.Width() * 0.5f, imageRegion.Height() * 0.5f);

    auto unitCenter = trayScanSpec.m_vecUnits[unitID].m_position.CenterPoint();

    Ipvm::Point32r2 inspCenter(0.f, 0.f);

    for (auto& fovID : inspectionAreaInfo.m_fovList)
    {
        inspCenter += trayScanSpec.m_vecFovInfo[fovID].m_fovCenter;
        auto fovCenter = scale.convert_mmToPixel(trayScanSpec.GetUnitCenterInFOV(fovID, unitID));
    }

    inspCenter.m_x /= (long)inspectionAreaInfo.m_fovList.size();
    inspCenter.m_y /= (long)inspectionAreaInfo.m_fovList.size();

    auto unitCenterForInsp = scale.convert_mmToPixel(unitCenter - inspCenter);

    return Ipvm::Conversion::ToPoint32r2(unitCenterForInsp) + imageCenter;
}
