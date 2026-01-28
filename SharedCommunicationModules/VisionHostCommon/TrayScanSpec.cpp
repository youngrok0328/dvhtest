//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "TrayScanSpec.h"

//CPP_2_________________________________ This project's headers
#include "DBObject.h"

//CPP_3_________________________________ Other projects' headers
#include "../../SharedComponent/Serialization/SerializeVector.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
BOOL TrayScanSpec::IncludeUnitInfo::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    db[_T("Unit ID")].Link(bSave, m_unitID);
    db[_T("FOV Left")].Link(bSave, m_unitInFOV.m_left);
    db[_T("FOV Top")].Link(bSave, m_unitInFOV.m_top);
    db[_T("FOV Right")].Link(bSave, m_unitInFOV.m_right);
    db[_T("FOV Bottom")].Link(bSave, m_unitInFOV.m_bottom);

    return TRUE;
}

BOOL TrayScanSpec::FOVInfo::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    db[_T("Fov Center X")].Link(bSave, m_fovCenter.m_x);
    db[_T("Fov Center Y")].Link(bSave, m_fovCenter.m_y);

    long includeCount = (long)m_vecIncludeUnitInfo.size();

    if (!db[_T("Include Count")].Link(bSave, includeCount))
    {
        includeCount = 0;
    }

    if (!bSave)
    {
        m_vecIncludeUnitInfo.resize(includeCount);
    }

    for (long index = 0; index < includeCount; index++)
    {
        m_vecIncludeUnitInfo[index].LinkDataBase(bSave, db.GetSubDBFmt(_T("Include %d"), index + 1));
    }

    return TRUE;
}

BOOL TrayScanSpec::UnitInfo::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    db[_T("Left")].Link(bSave, m_position.m_left);
    db[_T("Top")].Link(bSave, m_position.m_top);
    db[_T("Right")].Link(bSave, m_position.m_right);
    db[_T("Bottom")].Link(bSave, m_position.m_bottom);

    long fovCount = (long)m_fovList.size();
    long stitchCountY = (long)m_stitchFovList.size();

    if (!db[_T("FOV Count"		)].Link(bSave, fovCount))
        fovCount = 0;
    if (!db[_T("Stitch Count Y"	)].Link(bSave, stitchCountY))
        stitchCountY = 0;

    if (!bSave)
    {
        m_fovList.clear();
        m_stitchFovList.clear();

        m_fovList.resize(fovCount);
        m_stitchFovList.resize(stitchCountY);
    }

    if (fovCount > 0)
    {
        db[_T("FOV List")].LinkArray(bSave, &m_fovList[0], fovCount);
    }

    if (stitchCountY > 0)
    {
        for (long stitchRow = 0; stitchRow < stitchCountY; stitchRow++)
        {
            auto& stitch_db = db.GetSubDBFmt(_T("StitchRow%d"), stitchRow + 1);
            auto& info = m_stitchFovList[stitchRow];
            long stitchCountX = (long)info.size();

            stitch_db[_T("Count")].Link(bSave, stitchCountX);

            if (!bSave)
            {
                info.resize(stitchCountX);
            }

            if (stitchCountX > 0)
            {
                stitch_db[_T("Data")].LinkArray(bSave, &info[0], stitchCountX);
            }
        }
    }

    return TRUE;
}

//============================================================

TrayScanSpec::TrayScanSpec()
    : m_pocketNumX(0)
    , m_pocketNumY(0)
    , m_pocketPitchX_mm(0.f)
    , m_pocketPitchY_mm(0.f)
{
    reset();
}

TrayScanSpec::~TrayScanSpec()
{
}

void TrayScanSpec::reset()
{
    m_vecUnits.clear();
    m_vecFovInfo.clear();

    m_pocketNumX = 0;
    m_pocketNumY = 0;
    m_pocketPitchX_mm = 0.f;
    m_pocketPitchY_mm = 0.f;
}

BOOL TrayScanSpec::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    db[_T("Pocket NumX")].Link(bSave, m_pocketNumX);
    db[_T("Pocket NumY")].Link(bSave, m_pocketNumY);
    db[_T("Pocket Pitch X")].Link(bSave, m_pocketPitchX_mm);
    db[_T("Pocket Pitch Y")].Link(bSave, m_pocketPitchY_mm);

    if (true)
    {
        // Unit Location
        long unitCount = (long)m_vecUnits.size();
        auto& sub_db = db[_T("Units")];

        if (!sub_db[_T("Count")].Link(bSave, unitCount))
        {
            unitCount = 0;
        }

        if (!bSave)
        {
            m_vecUnits.clear();
            m_vecUnits.resize(unitCount);
        }

        for (long unitIndex = 0; unitIndex < unitCount; unitIndex++)
        {
            m_vecUnits[unitIndex].LinkDataBase(bSave, sub_db.GetSubDBFmt(_T("%d"), unitIndex + 1));
        }
    }

    if (true)
    {
        // FOV Info
        long fovCount = (long)m_vecFovInfo.size();
        auto& sub_db = db[_T("Fov Info")];

        if (!sub_db[_T("Count")].Link(bSave, fovCount))
        {
            fovCount = 0;
        }

        if (!bSave)
        {
            m_vecFovInfo.resize(fovCount);
        }

        for (long fovIndex = 0; fovIndex < fovCount; fovIndex++)
        {
            m_vecFovInfo[fovIndex].LinkDataBase(bSave, sub_db.GetSubDBFmt(_T("%d"), fovIndex));
        }
    }

    return TRUE;
}

//================================================================================
// Archive
//================================================================================

CArchive& operator<<(CArchive& ar, const TrayScanSpec::IncludeUnitInfo& obj)
{
    ar << obj.m_unitID;
    ar << obj.m_unitInFOV;

    return ar;
}

CArchive& operator>>(CArchive& ar, TrayScanSpec::IncludeUnitInfo& obj)
{
    ar >> obj.m_unitID;
    ar >> obj.m_unitInFOV;

    return ar;
}

CArchive& operator<<(CArchive& ar, const TrayScanSpec::FOVInfo& obj)
{
    ar << obj.m_fovCenter.m_x;
    ar << obj.m_fovCenter.m_y;
    ar << obj.m_vecIncludeUnitInfo;

    return ar;
}

CArchive& operator>>(CArchive& ar, TrayScanSpec::FOVInfo& obj)
{
    ar >> obj.m_fovCenter.m_x;
    ar >> obj.m_fovCenter.m_y;
    ar >> obj.m_vecIncludeUnitInfo;

    return ar;
}

CArchive& operator<<(CArchive& ar, const TrayScanSpec::UnitInfo& obj)
{
    ar << obj.m_position;
    ar << obj.m_fovList;
    ar << obj.m_stitchFovList;

    return ar;
}

CArchive& operator>>(CArchive& ar, TrayScanSpec::UnitInfo& obj)
{
    ar >> obj.m_position;
    ar >> obj.m_fovList;
    ar >> obj.m_stitchFovList;

    return ar;
}

CArchive& operator<<(CArchive& ar, const TrayScanSpec& obj)
{
    ar << obj.m_pocketNumX;
    ar << obj.m_pocketNumY;
    ar << obj.m_pocketPitchX_mm;
    ar << obj.m_pocketPitchY_mm;

    ar << obj.m_vecUnits;
    ar << obj.m_vecFovInfo;

    return ar;
}

CArchive& operator>>(CArchive& ar, TrayScanSpec& obj)
{
    ar >> obj.m_pocketNumX;
    ar >> obj.m_pocketNumY;
    ar >> obj.m_pocketPitchX_mm;
    ar >> obj.m_pocketPitchY_mm;

    ar >> obj.m_vecUnits;
    ar >> obj.m_vecFovInfo;

    return ar;
}

CArchive& operator<<(CArchive& ar, const Ipvm::Rect32r& obj)
{
    ar << obj.m_left;
    ar << obj.m_top;
    ar << obj.m_right;
    ar << obj.m_bottom;

    return ar;
}

CArchive& operator>>(CArchive& ar, Ipvm::Rect32r& obj)
{
    ar >> obj.m_left;
    ar >> obj.m_top;
    ar >> obj.m_right;
    ar >> obj.m_bottom;

    return ar;
}
