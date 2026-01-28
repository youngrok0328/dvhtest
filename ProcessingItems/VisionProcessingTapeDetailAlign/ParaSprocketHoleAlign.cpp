//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "ParaSprocketHoleAlign.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/TxtRecipeParameter.h" //kircheis_TxtRecipe
#include "../../SharedCommunicationModules/VisionHostCommon/DBObject.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
BOOL ParaSprocketHoleAlign::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    if (!bSave)
    {
        *this = {};
    }

    db[_T("EdgeType")].Link(bSave, m_edgeType);
    db[_T("EdgeDetectMode")].Link(bSave, (int &)m_edgeDetectMode);
    db[_T("FirstEdgeMinThreshold")].Link(bSave, m_firstEdgeMinThreshold);
    db[_T("EdgeCount")].Link(bSave, m_edgeCount);
    db[_T("PersentStart")].Link(bSave, m_persent_start);
    db[_T("PersentEnd")].Link(bSave, m_persent_end);

    return TRUE;
}
