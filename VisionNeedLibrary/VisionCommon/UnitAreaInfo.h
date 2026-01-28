#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class VisionUnitAgent;

//HDR_6_________________________________ Header body
//
class __VISION_COMMON_API__ UnitAreaInfo
{
public:
    UnitAreaInfo(VisionUnitAgent* parent);
    ~UnitAreaInfo();

    Ipvm::Point32r2 GetUnitCenterBasedOnFirstImageFOV() const;
    Ipvm::Point32r2 GetUnitCenterBasedOnFOV(long fovID) const;
    void AbsoluteROItoImageROI_Raw(Ipvm::Rect32s& rtROI) const;
    void ImageROItoAbsoluteROI_Raw(Ipvm::Rect32s& rtROI) const;

private:
    VisionUnitAgent* m_parent;
    long GetFovIDtoFOVIndex(long fovID) const;
    Ipvm::Point32r2 GetUnitCenterBasedOnImage() const;
};
