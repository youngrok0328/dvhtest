#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/FPI_RECT.h"
#include "../../DefineModules/dA_Base/PI_RECT.h"

//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
class CollectRois
{
public:
    CollectRois();
    ~CollectRois();

    void push(const Ipvm::Point32r2& rotateCenter, float angle, const FPI_RECT& roi);
    void push(const Ipvm::Point32r2& rotateCenter, float angle, const Ipvm::Rect32s& roi);
    void push(const Ipvm::Point32r2& rotateCenter, float angle, const Ipvm::Quadrangle32r& roi);

    const std::vector<PI_RECT>& get() const;

private:
    std::vector<PI_RECT> m_datas;
};
