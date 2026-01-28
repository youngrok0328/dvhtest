//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "ParaPocketAlign.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
#include "../../SharedCommunicationModules/VisionHostCommon/DBObject.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
BOOL ParaPocketAlign::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    if (!bSave)
    {
        *this = {};
    }

    db[_T("SkipAlign")].Link(bSave, m_skipAlign);
    m_edgeParams.LinkDataBase(bSave, db[_T("EdgeParameters")]);
    for (int i = 0; i < 4; i++)
    {
        CString strRoi;
        strRoi.Format(_T("EdgeSearchRoi%d"), i + 1);
        m_edgeSearchRois[i].LinkDataBase(bSave, db[strRoi]);
    }
    
    return TRUE;
}
