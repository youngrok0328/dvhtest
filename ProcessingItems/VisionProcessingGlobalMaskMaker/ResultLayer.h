#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "ResultLayerPre.h"

//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class SpecLayer;

//HDR_6_________________________________ Header body
//
class ResultLayer
{
public:
    ResultLayer();
    ResultLayer(const SpecLayer& spec, const Ipvm::Point32r2& px2um, const Ipvm::Point32r2& imageCenter);
    ~ResultLayer();

    CString m_name;
    CString m_surfaceMaskName;
    ResultLayerPre m_pre;
};
