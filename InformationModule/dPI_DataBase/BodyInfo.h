#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/semiinfo.h"

//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <minwindef.h>

//HDR_5_________________________________ Forward declarations
class CiDataBase;

//HDR_6_________________________________ Header body
//
namespace Package
{
struct DPI_DATABASE_API BodyInfo
{
public:
    BodyInfo();

    void Init();

    BOOL LinkDataBase_0(BOOL bSave, CiDataBase& db);
    BOOL LinkDataBase(BOOL bSave, CiDataBase& db);

    float fBodySizeX;
    float fBodySizeY;
    float fBodyThickness;

    float fGerberBodyOffsetX;
    float fGerberBodyOffsetY;

    BOOL m_bSDCardMode;
    float m_fDistFromLeft;
    float m_fLowFromTop;
    float m_fRoundRadius;

    BOOL m_bOctagonMode;
    float m_fDistOctagonChamfer;

    //kircheis_RoundPKG
    long m_nRoundPackageOption;
    float m_fRoundPackageRadius;
    //}}

    //{{//kircheis_KOZ2D
    float m_fKOZWidthLeft; //Unit : um
    float m_fKOZWidthUp; //Unit : um
    float m_fKOZWidthRight; //Unit : um
    float m_fKOZWidthDown; //Unit : um

    float GetKOZWidth(int16_t nDir)
    {
        switch (nDir)
        {
            case LEFT:
                return m_fKOZWidthLeft;
            case UP:
                return m_fKOZWidthUp;
            case RIGHT:
                return m_fKOZWidthRight;
            case DOWN:
                return m_fKOZWidthDown;
        }
        return 0.f;
    };
    //}}

    float GetBodySizeX()
    {
        return fBodySizeX;
    };
    float GetBodySizeY()
    {
        return fBodySizeY;
    };

    //{{//kircheis_SideInsp
    BOOL m_bGlassCorePackage;
    float m_fGlassCoreThickness;
    float m_fGlassCoreSubstrateSizeX;
    float m_fGlassCoreSubstrateSizeY;
    float m_fGlassCoreTopSubstrateThickness;
    float m_fGlassCoreBottomSubstrateThickness;
    //}}
};

} // namespace Package