//CPP_0_________________________________ Precompiled header
#include "stdAfx.h"

//CPP_1_________________________________ Main header
#include "ParaDefine.h"

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
LPCTSTR GetBallZTypeString(enumGetherBallZType type)
{
    switch (type)
    {
        case enumGetherBallZType::Height:
            return _T("HEIGHT");
            break;
        case enumGetherBallZType::Copl:
            return _T("COPL");
            break;
    }

    ASSERT(!_T("??"));
    return _T("");
}
