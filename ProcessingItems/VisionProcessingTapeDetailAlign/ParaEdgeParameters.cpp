//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "ParaEdgeParameters.h"

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
BOOL ParaEdgeParameters::LinkDataBase(BOOL bSave, CiDataBase& db)
{
	if (!bSave)
	{
		*this = {};
	}

	db[_T("EdgeType")].Link(bSave, m_edgeType);
	db[_T("SearchDirection")].Link(bSave, m_searchDirection);
	db[_T("EdgeDetectMode")].Link(bSave, (int &)m_edgeDetectMode);
	db[_T("FirstEdgeMinThreshold")].Link(bSave, m_firstEdgeMinThreshold);
	db[_T("EdgeAlignGap_um")].Link(bSave, m_edgeAlignGap_um);

	return TRUE;
}
