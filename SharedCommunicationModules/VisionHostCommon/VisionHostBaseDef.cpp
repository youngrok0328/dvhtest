//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionHostBaseDef.h"

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
COLORREF Result2Color(long nResult)
{
    switch (nResult)
    {
        case NOT_MEASURED:
            return RGB(128, 128, 128);
            break;
        case PASS:
            return RGB(150, 250, 150);
            break;
        case MARGINAL:
            return RGB(243, 157, 58);
            break;
        case REJECT:
            return RGB(250, 150, 150);
            break;
        case INVALID:
            return RGB(250, 250, 100);
            break;
        case EMPTY:
            return RGB(200, 200, 200);
            break;
        case COUPON:
            return RGB(250, 128, 00);
            break;
        case DOUBLEDEVICE:
            return RGB(150, 250, 250);
            break;
        default:
            return RGB(0, 0, 0);
    }
};
////////////////////////////////////////////////////////////////////////////////////////////////////
/**
@brief  Result2String function
*/
////////////////////////////////////////////////////////////////////////////////////////////////////
CString Result2String(long nResult)
{
    if (nResult == NOT_MEASURED)
    {
        return _T("Not Measured");
    }
    else if (nResult == PASS)
    {
        return _T("Pass");
    }
    else if (nResult == MARGINAL)
    {
        return _T("Marginal");
    }
    else if (nResult == REJECT)
    {
        return _T("Reject");
    }
    else if (nResult == INVALID)
    {
        return _T("Invalid");
    }
    else if (nResult == EMPTY)
    {
        return _T("Empty");
    }
    else if (nResult == COUPON)
    {
        return _T("Coupon");
    }
    else if (nResult == DOUBLEDEVICE)
    {
        return _T("Double");
    }

    return _T("Not Measured");
}
