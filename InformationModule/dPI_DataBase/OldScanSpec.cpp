//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "OldScanSpec.h"

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
OldScanSpec::OldScanSpec()
    : m_trayPocketCountX(1)
    , m_trayPocketCountY(1)
    , m_fovPaneCountX(1)
    , m_fovPaneCountY(1)
    , m_fovPanePitchX_um(0.)
    , m_fovPanePitchY_um(0.)
    , m_horPaneOffset_um(0.)
    , m_verPaneOffset_um(0.)
    , m_currentPaneIndex(0)
    , m_stitchCountX(1)
    , m_stitchCountY(1)
    , m_moveDistanceXbetweenFOVs_mm(1.f)
    , m_moveDistanceYbetweenFOVs_mm(1.f)
    , m_scanLength_mm(0.)
{
}

OldScanSpec::~OldScanSpec()
{
}

// Save & Load...
BOOL OldScanSpec::LinkAreaImageInfo(BOOL bSave, CiDataBase& db)
{
    long version = 2;

    if (!db[_T("")].Link(bSave, version))
        version = 0;

    if (!db[_T("{F4CCE09D-775B-4FE7-86E8-2CAE1350EECE}")].Link(bSave, m_trayPocketCountX))
        m_trayPocketCountX = 1;
    if (!db[_T("{2F1AF261-C040-407E-B2C0-FA1C74216789}")].Link(bSave, m_trayPocketCountY))
        m_trayPocketCountY = 1;
    if (!db[_T("{1B451D33-AEFA-490F-A81F-9760760CB500}")].Link(bSave, m_fovPaneCountX))
        m_fovPaneCountX = 1;
    if (!db[_T("{5A34DDFA-BE27-471C-A73D-814C4CEC97E6}")].Link(bSave, m_fovPaneCountY))
        m_fovPaneCountY = 1;
    if (!db[_T("{195DBFCD-0A30-4425-AFA2-CB28DEBC4D2E}")].Link(bSave, m_fovPanePitchX_um))
        m_fovPanePitchX_um = 0.f;
    if (!db[_T("{584825FE-2D97-4241-8D1C-E858257FC3BB}")].Link(bSave, m_fovPanePitchY_um))
        m_fovPanePitchY_um = 0.f;
    if (!db[_T("{D678C657-8616-47A6-AED7-FB333BA59B3D}")].Link(bSave, m_horPaneOffset_um))
        m_horPaneOffset_um = 0.f;
    if (!db[_T("{C9902F24-6E03-4146-BE20-CE013883D1EE}")].Link(bSave, m_verPaneOffset_um))
        m_verPaneOffset_um = 0.f;
    if (!db[_T("{647AD61D-D6D1-4336-B251-021E5A5D036B}")].Link(bSave, m_currentPaneIndex))
        m_currentPaneIndex = 0;

    return TRUE;
}
