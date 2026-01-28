#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
#include "ButtonST.h"

//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
enum btn_border_type
{
    SHOW_BORDER_ALWAYS,
    SHOW_BORDER_ON_FOCUS
};

class __INTEKPLUS_SHARED_BUTTON_API__ CWinXPButtonST : public CButtonST
{
public:
    CWinXPButtonST();
    ~CWinXPButtonST() override;

    DWORD SetRounded(BOOL bRounded, btn_border_type type = SHOW_BORDER_ON_FOCUS, BOOL bRepaint = TRUE);

protected:
    DWORD OnDrawBackground(CDC* pDC, CRect* pRect) override;
    DWORD OnDrawBorder(CDC* pDC, CRect* pRect) override;

private:
    BOOL m_bIsRounded; // Borders must be rounded?
    BOOL m_bAlwaysBorder;
};
