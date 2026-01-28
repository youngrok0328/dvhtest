#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
#include <Ipvm/Base/Point32r2.h>
#include <Ipvm/Base/Rect32s.h>

//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
class VisionProcessingCarrierTapeAlign;

//HDR_6_________________________________ Header body
//
class Constants
{
public:
    Constants(VisionProcessingCarrierTapeAlign* processor);
    ~Constants() = default;

    bool Make();

    long m_rawImageSizeX = 0;
    long m_rawImageSizeY = 0;
    Ipvm::Point32r2 m_rawImageCenter = {0.f, 0.f};

    long m_sensorSizeX = 0;
    long m_sensorSizeY = 0;

private:
    VisionProcessingCarrierTapeAlign* m_processor;
};
