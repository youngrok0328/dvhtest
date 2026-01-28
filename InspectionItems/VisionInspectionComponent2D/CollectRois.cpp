//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "CollectRois.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/iDataType.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Base/Quadrangle32r.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
CollectRois::CollectRois()
{
}

CollectRois::~CollectRois()
{
}

void CollectRois::push(const Ipvm::Point32r2& rotateCenter, float angle, const FPI_RECT& roi)
{
    if (angle == 0.f || angle == 90.f)
    {
        m_datas.push_back(roi.GetSPI_RECT());
    }
    else
    {
        auto rotateRoi = roi.Rotate(angle * DEF_DEG_TO_RAD, rotateCenter);
        m_datas.push_back(rotateRoi.GetSPI_RECT());
    }
}

void CollectRois::push(const Ipvm::Point32r2& rotateCenter, float angle, const Ipvm::Rect32s& roi)
{
    FPI_RECT frect;
    frect.SetSPI_RECT(PI_RECT(roi));

    push(rotateCenter, angle, frect);
}

void CollectRois::push(const Ipvm::Point32r2& rotateCenter, float angle, const Ipvm::Quadrangle32r& roi)
{
    FPI_RECT frect = FPI_RECT(Ipvm::Point32r2(roi.m_ltX, roi.m_ltY), Ipvm::Point32r2(roi.m_rtX, roi.m_rtY),
        Ipvm::Point32r2(roi.m_lbX, roi.m_lbY), Ipvm::Point32r2(roi.m_rbX, roi.m_rbY));

    push(rotateCenter, angle, frect);
}

const std::vector<PI_RECT>& CollectRois::get() const
{
    return m_datas;
}
