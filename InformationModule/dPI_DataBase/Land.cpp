//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "Land.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
#include <minwindef.h>
#include <winnt.h>

//CPP_6_________________________________ Preprocessor macros
//CPP_7_________________________________ Implementation body
//
namespace Package
{
Land::Land(LPCTSTR i_groupID, LPCTSTR i_strLandID, float i_fOffsetX, float i_fOffsetY, float i_fWidth, float i_fLength,
    float i_fHeight, long i_nAngle, long i_nIndex, BOOL i_bIgnore, long i_nLandShapeType)
{
    m_groupID = i_groupID;
    strLandID = i_strLandID;
    fOffsetX = i_fOffsetX;
    fOffsetY = i_fOffsetY;
    fWidth = i_fWidth;
    fLength = i_fLength;
    fHeight = i_fHeight;
    nAngle = i_nAngle;
    nIndex = i_nIndex;
    bIgnore = i_bIgnore;
    nLandShapeType = i_nLandShapeType; //kircheis_LandShape
}
} // namespace Package
