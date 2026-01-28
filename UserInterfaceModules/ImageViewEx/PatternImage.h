#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
#include <Ipvm/Base/Image8u3.h>

//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
class DPI_IMAGE_LOT_VIEW_API PatternImage : public Ipvm::Image8u3
{
public:
    PatternImage();
    ~PatternImage();

    void setImage(const Ipvm::Image8u& image);
    bool setThresholdImage(const Ipvm::Image8u& image);
    bool addPattern(const Ipvm::Image8u& mask, COLORREF rgb);
    bool addPattern(const Ipvm::Image32s& label, long labelindex, const Ipvm::Rect32s& roi, COLORREF rgb);
};
