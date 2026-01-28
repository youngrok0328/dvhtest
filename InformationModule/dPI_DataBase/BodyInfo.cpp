//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "BodyInfo.h"

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
BodyInfo::BodyInfo()
{
    Init();
}

void BodyInfo::Init()
{
    fBodySizeX = 0.f;
    fBodySizeY = 0.f;
    fBodyThickness = 0.f;

    fGerberBodyOffsetX = 0.f;
    fGerberBodyOffsetY = 0.f;

    m_fDistFromLeft = 0.f;
    m_fLowFromTop = 0.f;
    m_fRoundRadius = 31.496f;
    m_bSDCardMode = FALSE;

    m_bOctagonMode = FALSE;
    m_fDistOctagonChamfer = 0.f;

    //{{ //kircheis_RoundPKG
    m_nRoundPackageOption = 0; //Round_Normal
    m_fRoundPackageRadius = 0.f;
    //}}

    m_fKOZWidthLeft = 0.f;
    m_fKOZWidthUp = 0.f;
    m_fKOZWidthRight = 0.f;
    m_fKOZWidthDown = 0.f; //kircheis_KOZ2D

    //{{//kircheis_SideInsp
    m_bGlassCorePackage = FALSE;
    m_fGlassCoreSubstrateSizeX = 0.f;
    m_fGlassCoreSubstrateSizeY = 0.f;
    m_fGlassCoreTopSubstrateThickness = 0.f;
    m_fGlassCoreBottomSubstrateThickness = 0.f;
    //}}
}

// Save & Load...
BOOL BodyInfo::LinkDataBase_0(BOOL bSave, CiDataBase& db)
{
    BOOL bIsUnitUM = TRUE;
    if (!db[_T("BodyInfoMaster_Unit_um")].Link(bSave, bIsUnitUM))
        bIsUnitUM = FALSE;

    if (!db[_T("BodyInfoMaster_Body_Size_X")].Link(bSave, fBodySizeX))
        fBodySizeX = 0.f;
    if (!db[_T("BodyInfoMaster_Body_Size_Y")].Link(bSave, fBodySizeY))
        fBodySizeY = 0.f;
    if (!db[_T("BodyInfoMaster_Body_Thickness")].Link(bSave, fBodyThickness))
        fBodyThickness = 0.f;

    if (!db[_T("GerberBodyOffsetX")].Link(bSave, fGerberBodyOffsetX))
        fGerberBodyOffsetX = 0.f;
    if (!db[_T("GerberBodyOffsetY")].Link(bSave, fGerberBodyOffsetY))
        fGerberBodyOffsetY = 0.f;

    if (!db[_T("SDCardMode_Use")].Link(bSave, m_bSDCardMode))
        m_bSDCardMode = FALSE;
    if (!db[_T("DistFromLeft")].Link(bSave, m_fDistFromLeft))
        m_fDistFromLeft = 0.f;
    if (!db[_T("LowFromTop")].Link(bSave, m_fLowFromTop))
        m_fLowFromTop = 0.f;
    if (!db[_T("RoundRadius")].Link(bSave, m_fRoundRadius))
        m_fRoundRadius = 800.f;
    if (!bSave && m_fRoundRadius == 0
        && m_bSDCardMode) //기존 Job에 저장이 안되어 있을거니까 0.8mm로 초기화할 필요가 있다.
        m_fRoundRadius = 800.f;

    if (!db[_T("OctagonMode_Use")].Link(bSave, m_bOctagonMode))
        m_bOctagonMode = FALSE;
    if (!db[_T("DistOctagonChamfer")].Link(bSave, m_fDistOctagonChamfer))
        m_fDistOctagonChamfer = 0.f;

    //{{ //kircheis_RoundPKG
    if (!db[_T("RoundPackageOption")].Link(bSave, m_nRoundPackageOption))
        m_nRoundPackageOption = 0; //Round_Normal
    if (!db[_T("RoundPackageRadous")].Link(bSave, m_fRoundPackageRadius))
        m_fRoundPackageRadius = 0.f;
    //}}

    //{{//kircheis_KOZ2D
    if (!db[_T("KOZ_Width_Left")].Link(bSave, m_fKOZWidthLeft))
        m_fKOZWidthLeft = 0.f;
    if (!db[_T("KOZ_Width_UP")].Link(bSave, m_fKOZWidthUp))
        m_fKOZWidthUp = 0.f;
    if (!db[_T("KOZ_Width_Right")].Link(bSave, m_fKOZWidthRight))
        m_fKOZWidthRight = 0.f;
    if (!db[_T("KOZ_Width_Down")].Link(bSave, m_fKOZWidthDown))
        m_fKOZWidthDown = 0.f;

    m_fKOZWidthLeft = (float)max(0.f, m_fKOZWidthLeft);
    m_fKOZWidthUp = (float)max(0.f, m_fKOZWidthUp);
    m_fKOZWidthRight = (float)max(0.f, m_fKOZWidthRight);
    m_fKOZWidthDown = (float)max(0.f, m_fKOZWidthDown);
    //}}

    //{{//kircheis_SideInsp
    if (!db[_T("GlassCorePackage")].Link(bSave, m_bGlassCorePackage))
        m_bGlassCorePackage = FALSE;
    if (!db[_T("GlassCoreThickness")].Link(bSave, m_fGlassCoreThickness))
        m_fGlassCoreThickness = 0.f;
    if (!db[_T("GlassCoreSubstrateSizeX")].Link(bSave, m_fGlassCoreSubstrateSizeX))
        m_fGlassCoreSubstrateSizeX = 0.f;
    if (!db[_T("GlassCoreSubstrateSizeY")].Link(bSave, m_fGlassCoreSubstrateSizeY))
        m_fGlassCoreSubstrateSizeY = 0.f;
    if (!db[_T("GlassCoreTopSubstrateThickness")].Link(bSave, m_fGlassCoreTopSubstrateThickness))
        m_fGlassCoreTopSubstrateThickness = 0.f;
    if (!db[_T("GlassCoreBottomSubstrateThickness")].Link(bSave, m_fGlassCoreBottomSubstrateThickness))
        m_fGlassCoreBottomSubstrateThickness = 0.f;

    m_fGlassCoreThickness = (float)max(0.f, m_fGlassCoreThickness);
    m_fGlassCoreSubstrateSizeX = (float)max(0.f, m_fGlassCoreSubstrateSizeX);
    m_fGlassCoreSubstrateSizeY = (float)max(0.f, m_fGlassCoreSubstrateSizeY);
    m_fGlassCoreTopSubstrateThickness = (float)max(0.f, m_fGlassCoreTopSubstrateThickness);
    m_fGlassCoreBottomSubstrateThickness = (float)max(0.f, m_fGlassCoreBottomSubstrateThickness);
    //}}

    if (!bSave && !bIsUnitUM)
    {
        fBodySizeX *= 25.4f;
        fBodySizeY *= 25.4f;
        fBodyThickness *= 25.4f;

        fGerberBodyOffsetX *= 25.4f;
        fGerberBodyOffsetY *= 25.4f;
        m_fDistFromLeft *= 25.4f;
        m_fLowFromTop *= 25.4f;
        m_fRoundRadius *= 25.4f;

        m_fDistOctagonChamfer *= 25.4f;
        m_fRoundPackageRadius *= 25.4f;

        //{{//kircheis_KOZ2D
        m_fKOZWidthLeft *= 25.4f;
        m_fKOZWidthUp *= 25.4f;
        m_fKOZWidthRight *= 25.4f;
        m_fKOZWidthDown *= 25.4f;
        //}}

        //{{//kircheis_SideInsp
        m_fGlassCoreThickness *= 25.4f;
        m_fGlassCoreSubstrateSizeX *= 25.4f;
        m_fGlassCoreSubstrateSizeY *= 25.4f;
        m_fGlassCoreTopSubstrateThickness *= 25.4f;
        m_fGlassCoreBottomSubstrateThickness *= 25.4f;
        //}}
    }
    return TRUE;
}

BOOL BodyInfo::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    BOOL bIsUnitUM = TRUE;
    if (!db[_T("8CE3D39D-6440-49D8-9D08-0C44DC87741D")].Link(bSave, bIsUnitUM))
        bIsUnitUM = FALSE; //BodyInfoMaster_Unit_um

    if (!db[_T("{209C9B78-E310-4820-BBF6-7FAF86A941AE}")].Link(bSave, fBodySizeX))
        fBodySizeX = 0.f;
    if (!db[_T("{592DFF53-7FFC-4805-8CFB-3B6EDA107D41}")].Link(bSave, fBodySizeY))
        fBodySizeY = 0.f;
    if (!db[_T("{A81F8797-DE4F-483C-90A6-12EDD9369914}")].Link(bSave, fBodyThickness))
        fBodyThickness = 0.f;

    if (!db[_T("{0891B98B-6DDC-482B-BD6C-448BFEDDD7D8}")].Link(bSave, fGerberBodyOffsetX))
        fGerberBodyOffsetX = 0.f;
    if (!db[_T("{36980675-FCD7-4D47-8AF4-8CE37E7C9437}")].Link(bSave, fGerberBodyOffsetY))
        fGerberBodyOffsetY = 0.f;

    if (!db[_T("{D2A23C0A-BF57-48FB-B625-F3276289B324}")].Link(bSave, m_bSDCardMode))
        m_bSDCardMode = FALSE;
    if (!db[_T("{93749852-AB69-43A7-B25A-96580315FD8F}")].Link(bSave, m_fDistFromLeft))
        m_fDistFromLeft = 0.f;
    if (!db[_T("{E3B93D51-032F-457C-8BB3-3321F8E0F0EA}")].Link(bSave, m_fLowFromTop))
        m_fLowFromTop = 0.f;
    if (!db[_T("{9D450FE0-BC43-4675-93AA-2E6FB5AE4804}")].Link(bSave, m_fRoundRadius))
        m_fRoundRadius = 31.496f;

    if (!bSave && m_fRoundRadius == 0
        && m_bSDCardMode) //기존 Job에 저장이 안되어 있을거니까 0.8mm로 초기화할 필요가 있다.
        m_fRoundRadius = 31.496f;

    if (!db[_T("{9333B199-1697-4E0D-BFD5-3E1A4FAFE31A}")].Link(bSave, m_bOctagonMode))
        m_bOctagonMode = FALSE;
    if (!db[_T("{BE883C49-957E-47FE-A9B5-161273E2857F}")].Link(bSave, m_fDistOctagonChamfer))
        m_fDistOctagonChamfer = 0.f;

    //{{ //kircheis_RoundPKG
    if (!db[_T("{BE3DE1E7-DB6D-4961-BE4C-16EC087D218E}")].Link(bSave, m_nRoundPackageOption))
        m_nRoundPackageOption = 0; //Round_Normal
    if (!db[_T("{5B22A733-20C8-4012-837F-31D06B7FC9D2}")].Link(bSave, m_fRoundPackageRadius))
        m_fRoundPackageRadius = 0.f;
    //}}

    //{{//kircheis_KOZ2D
    if (!db[_T("{E1AA4FA5-192A-4566-B2E9-3F219EFD5997}")].Link(bSave, m_fKOZWidthLeft))
        m_fKOZWidthLeft = 0.f;
    if (!db[_T("{98EFF9AD-E1B9-4089-B62C-9C3855850225}")].Link(bSave, m_fKOZWidthUp))
        m_fKOZWidthUp = 0.f;
    if (!db[_T("{21C049DA-0113-449E-B157-E15B27E4AC61}")].Link(bSave, m_fKOZWidthRight))
        m_fKOZWidthRight = 0.f;
    if (!db[_T("{A996F6A7-8A7B-4EB0-B6A7-48874961395F}")].Link(bSave, m_fKOZWidthDown))
        m_fKOZWidthDown = 0.f;

    m_fKOZWidthLeft = (float)max(0.f, m_fKOZWidthLeft);
    m_fKOZWidthUp = (float)max(0.f, m_fKOZWidthUp);
    m_fKOZWidthRight = (float)max(0.f, m_fKOZWidthRight);
    m_fKOZWidthDown = (float)max(0.f, m_fKOZWidthDown);
    //}}

    //{{//kircheis_SideInsp
    if (!db[_T("{85EE7DE8-35BC-461B-BDA4-786F82DAF361}")].Link(bSave, m_bGlassCorePackage))
        m_bGlassCorePackage = FALSE;
    if (!db[_T("{F6583698-D8F2-48AC-9BC6-3F06AE5968A0}")].Link(bSave, m_fGlassCoreThickness))
        m_fGlassCoreThickness = 0.f;
    if (!db[_T("{5EC5EE25-28A3-4674-9AB1-2197CB612670}")].Link(bSave, m_fGlassCoreSubstrateSizeX))
        m_fGlassCoreSubstrateSizeX = 0.f;
    if (!db[_T("{A7141C4C-E846-4854-A626-63EC3872CBF6}")].Link(bSave, m_fGlassCoreSubstrateSizeY))
        m_fGlassCoreSubstrateSizeY = 0.f;
    if (!db[_T("{D8E646BB-0B23-4ACD-80EE-A2527C72B866}")].Link(bSave, m_fGlassCoreTopSubstrateThickness))
        m_fGlassCoreTopSubstrateThickness = 0.f;
    if (!db[_T("{91999267-F9F8-43D9-9225-D67035C11B9B}")].Link(bSave, m_fGlassCoreBottomSubstrateThickness))
        m_fGlassCoreBottomSubstrateThickness = 0.f;

    m_fGlassCoreThickness = (float)max(0.f, m_fGlassCoreThickness);
    m_fGlassCoreSubstrateSizeX = (float)max(0.f, m_fGlassCoreSubstrateSizeX);
    m_fGlassCoreSubstrateSizeY = (float)max(0.f, m_fGlassCoreSubstrateSizeY);
    m_fGlassCoreTopSubstrateThickness = (float)max(0.f, m_fGlassCoreTopSubstrateThickness);
    m_fGlassCoreBottomSubstrateThickness = (float)max(0.f, m_fGlassCoreBottomSubstrateThickness);
    //}}

    if (!bSave && !bIsUnitUM)
    {
        fBodySizeX *= 25.4f;
        fBodySizeY *= 25.4f;
        fBodyThickness *= 25.4f;

        fGerberBodyOffsetX *= 25.4f;
        fGerberBodyOffsetY *= 25.4f;
        m_fDistFromLeft *= 25.4f;
        m_fLowFromTop *= 25.4f;
        m_fRoundRadius *= 25.4f;

        m_fDistOctagonChamfer *= 25.4f;
        m_fRoundPackageRadius *= 25.4f;

        //{{//kircheis_KOZ2D
        m_fKOZWidthLeft *= 25.4f;
        m_fKOZWidthUp *= 25.4f;
        m_fKOZWidthRight *= 25.4f;
        m_fKOZWidthDown *= 25.4f;
        //}}

        //{{//kircheis_SideInsp
        m_fGlassCoreThickness *= 25.4f;
        m_fGlassCoreSubstrateSizeX *= 25.4f;
        m_fGlassCoreSubstrateSizeY *= 25.4f;
        m_fGlassCoreTopSubstrateThickness *= 25.4f;
        m_fGlassCoreBottomSubstrateThickness *= 25.4f;
        //}}
    }

    return TRUE;
}

} // namespace Package