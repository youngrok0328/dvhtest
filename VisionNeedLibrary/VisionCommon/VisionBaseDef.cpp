//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionBaseDef.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
enum enumPredefineGUID
{
    _enumPredefineGUID_Start = 0,
    _enumPredefineGUID_PackageSpec = _enumPredefineGUID_Start,
    _enumPredefineGUID_FOV,
    _enumPredefineGUID_Stitching2D,
    _enumPredefineGUID_Stitching3D,
    _enumPredefineGUID_PadAlign2D,
    _enumPredefineGUID_2dMatrix,
    _enumPredefineGUID_Ball2D,
    _enumPredefineGUID_Ball3D,
    _enumPredefineGUID_BallPixelQuality,
    _enumPredefineGUID_Land2D,
    _enumPredefineGUID_Land3D,
    _enumPredefineGUID_Passive2D,
    _enumPredefineGUID_Passive3D,
    _enumPredefineGUID_OtherAlign,
    _enumPredefineGUID_End,
};

static LPCTSTR g_strPredefineVisionModulGUID[] = {
    _VISION_INSP_GUID_PACKAGE_SPEC,
    _VISION_INSP_GUID_FOV,
    _VISION_INSP_GUID_ALIGN_2D,
    _VISION_INSP_GUID_ALIGN_3D,
    _VISION_INSP_GUID_PAD_ALIGN_2D,
    _VISION_INSP_GUID_2D_MATRIX,
    _VISION_INSP_GUID_BGA_BALL_2D,
    _VISION_INSP_GUID_BGA_BALL_3D,
    _VISION_INSP_GUID_BGA_BALL_PQ,
    _VISION_INSP_GUID_LGA_LAND_2D,
    _VISION_INSP_GUID_LGA_LAND_3D,
    _VISION_INSP_GUID_PASSIVE_2D,
    _VISION_INSP_GUID_PASSIVE_3D,
    _VISION_INSP_GUID_OTHER_ALIGN,
};

CString CreateGUID()
{
    GUID guid;

    if (S_OK != ::CoCreateGuid(&guid))
    {
        return _T("");
    }

    TCHAR guidString[128];

    ::StringFromGUID2(guid, guidString, sizeof(guidString) / sizeof(TCHAR));

    return guidString;
}

BOOL IsPredefineModule(CString strGUID)
{
    for (long index = 0; index < _enumPredefineGUID_End; index++)
    {
        if (g_strPredefineVisionModulGUID[index] == strGUID)
            return TRUE;
    }
    return FALSE;
}

CRect GetContentSubArea(const CRect& rtBase, const long colBegin, const long colEnd, const long colCount,
    const long rowBegin, const long rowEnd, const long rowCount)
{
    ASSERT(colBegin >= 0 && colEnd > colBegin && colEnd <= colCount && colCount >= 1);
    ASSERT(rowBegin >= 0 && rowEnd > rowBegin && rowEnd <= rowCount && rowCount >= 1);

    const long gap = 5;

    const long columnWidth = (rtBase.Width() - (colCount - 1) * gap) / colCount;
    const long rowHeight = (rtBase.Height() - (rowCount - 1) * gap) / rowCount;

    const long xBegin = rtBase.left + colBegin * columnWidth + colBegin * gap;
    const long xEnd = (colEnd < colCount) ? (rtBase.left + colEnd * columnWidth + (colEnd - 1) * gap) : rtBase.right;
    const long yBegin = rtBase.top + rowBegin * rowHeight + rowBegin * gap;
    const long yEnd = (rowEnd < rowCount) ? (rtBase.top + rowEnd * rowHeight + (rowEnd - 1) * gap) : rtBase.bottom;

    return CRect(xBegin, yBegin, xEnd, yEnd);
}
