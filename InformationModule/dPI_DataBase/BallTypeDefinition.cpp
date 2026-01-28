//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "BallTypeDefinition.h"

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
namespace Package
{
BallTypeDefinition::BallTypeDefinition(
    LPCTSTR groupID, const double diameter_um, const double height_um, const long groupBallNum) //kircheis_AIBC
    : m_groupID(groupID)
    , m_diameter_um(diameter_um)
    , m_height_um(height_um)
    , m_groupBallNum(groupBallNum) //kircheis_AIBC
{
}

BOOL BallTypeDefinition::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    db[_T("{2559511F-14C6-4BD5-8A80-561C8069AE81}")].Link(bSave, m_diameter_um);
    db[_T("{C6F1A3E3-3F06-44B3-A7EF-E704A151A250}")].Link(bSave, m_height_um);
    db[_T("{52E50A5A-ADE9-4FC3-B364-22A163EF0655}")].Link(bSave, m_groupID);
    db[_T("{8A8C8AAA-E64A-4F04-8129-320596CEE538}")].Link(bSave, m_groupBallNum); //kircheis_AIBC

    return TRUE;
}

} // namespace Package