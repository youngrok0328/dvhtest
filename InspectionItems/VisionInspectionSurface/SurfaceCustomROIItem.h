#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
#include <Ipvm/Base/Image8u.h>

//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
struct PI_RECT;

//HDR_6_________________________________ Header body
//
struct __DPI_SURFACE_API__ SSurfaceCustomROIItem
{
    SSurfaceCustomROIItem();

    BOOL Resize(int nNum);
    BOOL SetRect(int index, const Ipvm::Rect32s& rtROI);
    BOOL SetImage(const Ipvm::Image8u& image);
    BOOL SetArbitrary(int index, PI_RECT artROI);

    long nType; // 현재 도형의 타입 (0 : RECT, 1 : Ellipse, 2 : Image, 3~: Polygon_32f)
    CString strGroupName; // 현재 그룹의 이름

    // if(nType == 2)
    Ipvm::Image8u m_image;

    // else
    struct SPolygonShape
    {
        //======================================
        // SI의 마스크 영역을 위해 만든 Polygon_32f
        //======================================

        std::vector<Ipvm::Point32s2> vecptPos;
    };

    std::vector<SPolygonShape> vecShape; // 도형 벡터
    // end
};