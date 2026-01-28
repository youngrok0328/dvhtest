//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "ParaEdgeParameters.h"

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
BOOL ParaEdgeParameters::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    if (!bSave)
    {
        *this = {};
    }

    db[_T("Edge_Direction")].Link(bSave, m_edgeType);
    db[_T("Search_Direction")].Link(bSave, m_searchDirection);
    db[_T("Edge_Detect_Mode")].Link(bSave, m_edgeDetectMode);
    db[_T("RoughAlign_Edge_Gap")].Link(bSave, m_edgeAlignGap_um);
    db[_T("FirstEdgeMinThreshold")].Link(bSave, m_firstEdgeMinThreshold);

    return TRUE;
}
