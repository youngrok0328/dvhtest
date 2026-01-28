#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
#include <Ipvm/Base/Image8u.h>

//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
struct FunctionResult
{
    Ipvm::Image8u m_mask;
    Ipvm::Image8u m_dest;

    void Reset()
    {
        m_mask.Free();
        m_dest.Free();
    }
};
