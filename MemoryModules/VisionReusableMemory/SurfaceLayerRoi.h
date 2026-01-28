#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
#include <Ipvm/Base/EllipseEq32r.h>
#include <Ipvm/Base/Point32r2.h>
#include <Ipvm/Base/Quadrangle32r.h>

//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
class VisionReusableMemory;

//HDR_6_________________________________ Header body
//
class __VISION_REUSABLE_MEMORY_CLASS__ SurfaceLayerRoi
{
public:
    SurfaceLayerRoi(VisionReusableMemory* memory);
    ~SurfaceLayerRoi();

    bool Ready(const Ipvm::Point32r2& bodyCenter);
    void Reset();
    void Add(const Ipvm::EllipseEq32r& object);
    void Add(const Ipvm::Quadrangle32r& object);
    void Add(const Ipvm::Rect32s& object);

    bool AddToMask(long dilateX, long dilateY, Ipvm::Image8u& io_mask) const;
    bool SubtractToMask(long dilateX, long dilateY, Ipvm::Image8u& io_mask) const;

protected:
    VisionReusableMemory* m_memory;
    Ipvm::Point32r2 m_bodyCenter;

    std::vector<Ipvm::EllipseEq32r> m_objects_ellipse;
    std::vector<Ipvm::Quadrangle32r> m_objects_quardrangle;

    Ipvm::Point32r2 makeSurfacePos(
        const Ipvm::Point32r2& point, const Ipvm::Point32r2& bodyCenter, float bodyAngle_deg);
};
