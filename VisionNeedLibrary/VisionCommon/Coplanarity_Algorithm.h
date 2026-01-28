#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
class __VISION_COMMON_API__ Coplanarity_Algorithm
{
public:
    Coplanarity_Algorithm();
    ~Coplanarity_Algorithm();

    // 아래 입력 포인트에서 X, Y 는 Pixel 좌표계이고, Z 는 um 단위임.
    static BOOL CalcReferencePlaneDeviation(const std::vector<Ipvm::Point32r3>& pointCloud_px_px_um,
        const float px2um_x, const float px2um_y, const float inputNoiseValue_um, const float outputNoiseValue_um,
        std::vector<float>& coplanarity, BOOL isDeadBugWarpage = FALSE);
};
