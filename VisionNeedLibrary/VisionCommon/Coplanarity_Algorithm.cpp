//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "Coplanarity_Algorithm.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
#include <Ipvm/Algorithm/DataFitting.h>
#include <Ipvm/Base/Enum.h>
#include <Ipvm/Base/PlaneEq64r.h>
#include <Ipvm/Base/Point32r3.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
Coplanarity_Algorithm::Coplanarity_Algorithm()
{
}

Coplanarity_Algorithm::~Coplanarity_Algorithm()
{
}

BOOL Coplanarity_Algorithm::CalcReferencePlaneDeviation(const std::vector<Ipvm::Point32r3>& pointCloud_px_px_um,
    const float px2um_x, const float px2um_y, const float inputNoiseValue_um, const float outputNoiseValue_um,
    std::vector<float>& coplanarity, BOOL isDeadBugWarpage)
{
    const long pointCount = (long)pointCloud_px_px_um.size();

    if (pointCount <= 0)
    {
        return FALSE;
    }

    if (coplanarity.size() < pointCount)
    {
        return FALSE;
    }

    // 오른손 좌표계를 사용하기 위해서 Y 에 -1 곱해줌
    const float px2um_y_new = px2um_y > 0.f ? (-px2um_y) : px2um_y;

    std::vector<Ipvm::Point32r3> vecPoints;
    vecPoints.reserve(pointCount);

    Ipvm::Point32r3 temp;

    const float fDeadBugGain = isDeadBugWarpage ? -1.f : 1.f;

    for (int idx = 0; idx < pointCount; idx++)
    {
        const auto& src = pointCloud_px_px_um[idx];

        if (src.m_z == inputNoiseValue_um)
        {
            continue;
        }

        // 단위계를 실 단위계로 바꿔 주고...
        temp.m_x = src.m_x * px2um_x * fDeadBugGain;
        temp.m_y = src.m_y * px2um_y_new;
        temp.m_z = src.m_z * fDeadBugGain;

        vecPoints.push_back(temp);
    }

    if (vecPoints.size() < 3)
    {
        for (int idx = 0; idx < pointCount; idx++)
        {
            coplanarity[idx] = outputNoiseValue_um;
        }

        return FALSE;
    }

    // 기준 평면을 계산한다.
    Ipvm::PlaneEq64r planeEq = {0., 0., 0., 0.};

    if (Ipvm::Status::e_ok != Ipvm::DataFitting::FitToPlane((long)vecPoints.size(), &vecPoints[0], planeEq))
    {
        return FALSE;
    }

    const float planeNorm
        = float(::sqrt(planeEq.m_a * planeEq.m_a + planeEq.m_b * planeEq.m_b + planeEq.m_c * planeEq.m_c));

    // Coplanarity 계산
    for (int idx = 0; idx < pointCount; idx++)
    {
        const auto& src = pointCloud_px_px_um[idx];

        if (src.m_z == inputNoiseValue_um)
        {
            coplanarity[idx] = outputNoiseValue_um;
            continue;
        }

        temp.m_x = src.m_x * px2um_x * fDeadBugGain;
        temp.m_y = src.m_y * px2um_y_new;
        temp.m_z = src.m_z * fDeadBugGain;

        const float distToPlane_um = float(
            (planeEq.m_a * temp.m_x + planeEq.m_b * temp.m_y + planeEq.m_c * temp.m_z + planeEq.m_d) / planeNorm);

        coplanarity[idx] = distToPlane_um;
    }

    return TRUE;
}