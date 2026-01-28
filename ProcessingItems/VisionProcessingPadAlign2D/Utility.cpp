//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "Utility.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
#include <Ipvm/Base/Point32r2.h>
#include <Ipvm/Base/Polygon32r.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
Ipvm::Rect32r Utility::ConvertPolygonToRect_32f(const Ipvm::Polygon32r& polygon)
{
    long vertexNum = static_cast<long>(polygon.GetVertexNum());

    Ipvm::Rect32r roi(0.f, 0.f, 0.f, 0.f);
    if (vertexNum > 0)
    {
        const auto* pts = polygon.GetVertices();
        roi.m_left = pts[0].m_x;
        roi.m_right = roi.m_left;
        roi.m_top = pts[0].m_y;
        roi.m_bottom = roi.m_top;

        for (long n = 1; n < vertexNum; n++)
        {
            if (roi.m_left > pts[n].m_x)
                roi.m_left = pts[n].m_x;
            if (roi.m_right < pts[n].m_x)
                roi.m_right = pts[n].m_x;
            if (roi.m_top > pts[n].m_y)
                roi.m_top = pts[n].m_y;
            if (roi.m_bottom < pts[n].m_y)
                roi.m_bottom = pts[n].m_y;
        }
    }

    return roi;
}
