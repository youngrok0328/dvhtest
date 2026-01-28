//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "SurfaceCustomROIItem.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/PI_RECT.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Base/Point32s2.h>
#include <Ipvm/Base/Rect32s.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
SSurfaceCustomROIItem::SSurfaceCustomROIItem()
    : nType(-1) // 생성시 타입은 지정되지 않은 타입
{};

BOOL SSurfaceCustomROIItem::Resize(int nNum)
{
    // 데이터의 갯수를 설정한다. 미리 설정해놔야 메모리를 할당해 놓는다.

    int i, nPtNum;

    if (nType == -1)
    {
        return FALSE;
    }

    switch (nType)
    {
        case 0:
        case 1:
            nPtNum = 2;
            break;
        case 2:
            nPtNum = 4;
            break;
        default:
            nPtNum = nType;
            break;
    }

    vecShape.resize(nNum);
    for (i = 0; i < nNum; i++)
        vecShape[i].vecptPos.resize(nPtNum);
    return TRUE;
}

BOOL SSurfaceCustomROIItem::SetRect(int index, const Ipvm::Rect32s& rtROI) ///< Rect 데이터를 셋팅한다.
{
    if (nType != 0 && nType != 1)
        return FALSE;
    vecShape[index].vecptPos[0].m_x = rtROI.m_left;
    vecShape[index].vecptPos[0].m_y = rtROI.m_top;
    vecShape[index].vecptPos[1].m_x = rtROI.m_right;
    vecShape[index].vecptPos[1].m_y = rtROI.m_bottom;

    return TRUE;
}

BOOL SSurfaceCustomROIItem::SetImage(const Ipvm::Image8u& image)
{
    if (nType != 2)
        return FALSE;

    m_image = image;

    return TRUE;
}
BOOL SSurfaceCustomROIItem::SetArbitrary(int index, PI_RECT artROI) ///< Arbitrary 데이터를 셋팅한다.
{
    if (nType != 4)
        return FALSE;

    vecShape[index].vecptPos[0].m_x = artROI.ltX;
    vecShape[index].vecptPos[0].m_y = artROI.ltY;
    vecShape[index].vecptPos[1].m_x = artROI.rtX;
    vecShape[index].vecptPos[1].m_y = artROI.rtY;
    vecShape[index].vecptPos[2].m_x = artROI.brX;
    vecShape[index].vecptPos[2].m_y = artROI.brY;
    vecShape[index].vecptPos[3].m_x = artROI.blX;
    vecShape[index].vecptPos[3].m_y = artROI.blY;

    return TRUE;
}
