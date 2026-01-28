#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
#include "SurfaceLayerRoi.h"

//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
#include <Ipvm/Base/Image8u.h>
#include <Ipvm/Base/Point32r2.h>

//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
class VisionReusableMemory;
class SurfaceLayerRoi;
struct FPI_RECT;
struct PI_RECT;

//HDR_6_________________________________ Header body
//
class __VISION_REUSABLE_MEMORY_CLASS__ SurfaceLayerMask : protected SurfaceLayerRoi
{
public:
    SurfaceLayerMask(VisionReusableMemory* memory);
    ~SurfaceLayerMask();

    bool Ready(const Ipvm::Point32r2& bodyCenter);

    void Reset();
    bool CopyFrom(const Ipvm::Image8u& image);
    bool ResizeFrom(const Ipvm::Image8u& image);
    bool Add(const Ipvm::Point32r2& center, const Ipvm::EllipseEq32r& objectShape, bool fill);
    bool Add(const Ipvm::Point32r2& center, const Ipvm::Polygon32r& objectShape, bool fill);
    bool Add(const Ipvm::EllipseEq32r& objectShape, bool fill);
    bool Add(const Ipvm::Polygon32r& object, bool fill);
    bool Add(const FPI_RECT& object, bool fill);
    bool Add(const PI_RECT& object, bool fill);
    bool Add(const std::vector<PI_RECT>& objects, bool fill);
    bool Add(const Ipvm::Image8u& mask);
    bool Add(const Ipvm::Image8u& mask,
        float
            i_fAngleParameter_deg); //mc_Mark 검사 후 Surface에 연동 시 Angle값에 의하여 2~3px 밀리는 경우가 발생 기존 Code는 살리고 함수하나 추가해서 사용
    bool Add(const Ipvm::Image8u& mask, const Ipvm::Point32r2& center, const Ipvm::Point32r2& bodyCenter,
        float markAngle_deg);

    bool AddLabel(const Ipvm::Image32s& labelImage, const Ipvm::BlobInfo& blobInfo, bool fill);

    const Ipvm::Image8u& GetMask() const;

private:
    Ipvm::Image8u m_maskImage;
};
