//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "SimpleMessage.h"

//CPP_2_________________________________ This project's headers
#include "dlgMessage.h"

//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
int SimpleMessage(LPCTSTR szMessage, int nType)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CdlgMessage Dlg(NULL, szMessage);

    return Dlg.DoModal(nType);
}
