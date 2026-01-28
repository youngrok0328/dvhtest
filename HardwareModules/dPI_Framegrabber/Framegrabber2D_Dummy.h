#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "Framegrabber_Base.h"

//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
class Framegrabber2D_Dummy : public FrameGrabber_Base
{
public:
    Framegrabber2D_Dummy();
    ~Framegrabber2D_Dummy() override;

protected:
    virtual bool LiveCallback(const int& graaber_id = 0, const int& camera_id = 0) override;
    virtual bool SetTriggerMode(int nMode) override;
};
