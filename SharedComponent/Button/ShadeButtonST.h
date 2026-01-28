#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
#include "ButtonST.h"

//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class CCeXDib;

//HDR_6_________________________________ Header body
//
class __INTEKPLUS_SHARED_BUTTON_API__ CShadeButtonST : public CButtonST
{
public:
    CShadeButtonST();
    ~CShadeButtonST() override;

    enum
    {
        SHS_NOISE = 0,
        SHS_DIAGSHADE,
        SHS_HSHADE,
        SHS_VSHADE,
        SHS_HBUMP,
        SHS_VBUMP,
        SHS_SOFTBUMP,
        SHS_HARDBUMP,
        SHS_METAL
    };

    void SetShade(UINT shadeID = 0, BYTE granularity = 8, BYTE highlight = 10, BYTE coloring = 0, COLORREF color = 0);
    DWORD SetRounded(BOOL bRounded, BOOL bShowBorder);

    void DefaultStyle();

private:
    CCeXDib* m_dNormal;
    CCeXDib* m_dDown;
    CCeXDib* m_dDisabled;
    CCeXDib* m_dOver;
    CCeXDib* m_dh;
    CCeXDib* m_dv;

    BOOL m_bIsRounded;
    BOOL m_bIsBorder;

protected:
    DWORD OnDrawBorder(CDC* pDC, CRect* pRect) override;
    DWORD OnDrawBackground(CDC* pDC, CRect* pRect) override;
};
