//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "CustomFixed.h"

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
CustomFixed::CustomFixed()
    : m_strCustomFixedName(_T(""))
    , m_eCustomFixedCategory(CustomFixedCategory::CustomFixedCategory_Circle)
    , m_fSpecWidth_mm(-1.f)
    , m_fSpecLength_mm(-1.f)
    , m_fSpecHeight_mm(-1.f)
    , m_fSpecCenterPos_mm(Ipvm::Point32r2(-1.f, -1.f))
    , m_bIgnore(TRUE)
    , m_fSpec_ellipse_mm(Ipvm::EllipseEq32r(-1.f, -1.f, -1.f, -1.f))
    , m_fsrtSpecROI(FPI_RECT(Ipvm::Point32r2(-1.f, -1.f), Ipvm::Point32r2(-1.f, -1.f), Ipvm::Point32r2(-1.f, -1.f),
          Ipvm::Point32r2(-1.f, -1.f)))
{
}

CustomFixed::~CustomFixed()
{
}

BOOL CustomFixed::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    if (!db[_T("{39735F5E-A4C3-42FE-982B-752E8639E025}")].Link(bSave, m_strCustomFixedName))
        m_strCustomFixedName = _T("");
    if (!db[_T("{31D4A941-7774-4717-94CF-CE444F3FE0E6}")].Link(bSave, (long&)m_eCustomFixedCategory))
        m_eCustomFixedCategory = CustomFixedCategory::CustomFixedCategory_Circle;
    if (!db[_T("{A39DEC17-337C-4703-B9D3-482C1D346DB6}")].Link(bSave, m_fSpecWidth_mm))
        m_fSpecWidth_mm = -1.f;
    if (!db[_T("{F8237657-6082-4432-8701-4E2DD77099F4}")].Link(bSave, m_fSpecLength_mm))
        m_fSpecLength_mm = -1.f;
    if (!db[_T("{9068862A-8BE8-47C6-A8DD-44DE2E10EDF9}")].Link(bSave, m_fSpecHeight_mm))
        m_fSpecHeight_mm = -1.f;
    if (!db[_T("{1229E6C2-7726-4F70-B070-758956CCA605}")].Link(bSave, m_fSpecCenterPos_mm.m_x))
        m_fSpecCenterPos_mm.m_x = -1.f;
    if (!db[_T("{F98F46A1-1C4F-437C-96C6-F2CD703BDE01}")].Link(bSave, m_fSpecCenterPos_mm.m_y))
        m_fSpecCenterPos_mm.m_x = -1.f;
    if (!db[_T("{FEC174A9-E071-416F-8234-9264680E8DC2}")].Link(bSave, m_bIgnore))
        m_bIgnore = TRUE;

    return TRUE;
}

const CustomFixedCategory CustomFixed::GetCatrgory() const
{
    return m_eCustomFixedCategory;
}

const CString CustomFixed::GetID() const
{
    return m_strCustomFixedName;
}