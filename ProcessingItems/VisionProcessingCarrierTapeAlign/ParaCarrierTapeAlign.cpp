//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "ParaCarrierTapeAlign.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
#include "../../SharedCommunicationModules/VisionHostCommon/DBObject.h"

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
BOOL ParaCarrierTapeAlign::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    if (!bSave)
    {
        *this = {};
    }

    m_edgeParams.LinkDataBase(bSave, db[_T("Edge Detect Parameter")]);
    
    // Edge Detect ROI Parameter
    m_edgeSearchRois[0].LinkDataBase(bSave, db[_T("Left_Edge_Detect_ROI")]);
    m_edgeSearchRois[1].LinkDataBase(bSave, db[_T("Right_Edge_Detect_ROI")]);

    return true;
}
