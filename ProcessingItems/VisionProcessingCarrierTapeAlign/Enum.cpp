//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "Enum.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/semiinfo.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
LPCTSTR GetDirectionName(bool isPocket, int32_t serachRoiInex)
{
    if (isPocket)
    {
        switch (serachRoiInex)
        {
            case LEFT:
                return _T("Pocket Left");
            case RIGHT:
                return _T("Pocket Right");
            case UP:
                return _T("Pocket Top");
            case DOWN:
                return _T("Pocket Bottom");
        }
        ASSERT(!_T("??"));
        return _T("");
    }
    else
    {
        switch (serachRoiInex)
        {
            case 0:
                return _T("Carrier Left");
            case 1:
                return _T("Carrier Right");
        }
    }

    ASSERT(!_T("??"));
    return _T("");
}