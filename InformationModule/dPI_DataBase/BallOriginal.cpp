//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "BallOriginal.h"

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
BallOriginal::BallOriginal(LPCTSTR groupID, LPCTSTR name, long index, double posX_um, double posY_um,
    double diameter_um, double height_um, bool bIgnore)
    : m_groupID(groupID)
    , m_typeIndex(0)
    , m_name(name)
    , m_index(index)
    , m_posX_um(posX_um)
    , m_posY_um(posY_um)
    , m_diameter_um(diameter_um)
    , m_height_um(height_um)
    , m_ignored(bIgnore)
{
}

BOOL BallOriginal::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    db[_T("{21A56E94-AB45-46C4-BCAA-2CDE5D7EF65A}")].Link(bSave, m_name);
    db[_T("{06C8A95E-762D-452E-A53D-53371CE73053}")].Link(bSave, m_index);
    db[_T("{764BE7D7-797E-409B-B5A8-B45032AA6392}")].Link(bSave, m_posX_um);
    db[_T("{E7814E2A-88A7-492E-8181-AA133937B4EF}")].Link(bSave, m_posY_um);
    db[_T("{8DF8EF33-89C9-4DEB-A908-D979C6FBFCCB}")].Link(bSave, m_diameter_um);
    db[_T("{45726EF6-C2B0-4DBC-8B84-62A20A827CC6}")].Link(bSave, m_height_um);
    db[_T("{D324291A-7F5B-4E71-B45A-66C49A62857E}")].Link(bSave, *(BOOL*)&m_ignored);
    db[_T("{535C2217-04A6-4A61-BF8E-E4C4AAA6B2E7}")].Link(bSave, m_groupID);
    db[_T("{D644AC82-0686-4002-BEF0-B6F4F003231F}")].Link(bSave, m_typeIndex);

    if (!bSave)
    {
        CString strGroupID = m_groupID;
        strGroupID.MakeUpper();
        char chHeader = static_cast<char>(strGroupID.GetAt(0));
        if (chHeader != 'G')
            m_groupID.Insert(0, _T("G"));
    }

    return TRUE;
}

} // namespace Package