//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "WinXPButtonST.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
CWinXPButtonST::CWinXPButtonST()
{
    // No rounded borders
    m_bIsRounded = FALSE;
    m_bAlwaysBorder = FALSE;
}

CWinXPButtonST::~CWinXPButtonST()
{
}

// This function is called every time the button border needs to be painted.
// This is a virtual function that can be rewritten in CButtonST-derived classes
// to produce a whole range of buttons not available by default.
//
// Parameters:
//		[IN]	pDC
//				Pointer to a CDC object that indicates the device context.
//		[IN]	pRect
//				Pointer to a CRect object that indicates the bounds of the
//				area to be painted.
//
// Return value:
//		BTNST_OK
//			Function executed successfully.
//
DWORD CWinXPButtonST::OnDrawBorder(CDC* pDC, CRect* pRect)
{
    UNREFERENCED_PARAMETER(pDC);
    UNREFERENCED_PARAMETER(pRect);

    return BTNST_OK;
} // End of OnDrawBorder

// This function is called every time the button background needs to be painted.
// If the button is in transparent mode this function will NOT be called.
// This is a virtual function that can be rewritten in CButtonST-derived classes
// to produce a whole range of buttons not available by default.
//
// Parameters:
//		[IN]	pDC
//				Pointer to a CDC object that indicates the device context.
//		[IN]	pRect
//				Pointer to a CRect object that indicates the bounds of the
//				area to be painted.
//
// Return value:
//		BTNST_OK
//			Function executed successfully.
//
DWORD CWinXPButtonST::OnDrawBackground(CDC* pDC, CRect* pRect)
{
    if (!m_bMouseOnButton && !m_bIsPressed && !m_bAlwaysBorder)
        return CButtonST::OnDrawBackground(pDC, pRect);

    // Create and select a solid brush for button background
    CBrush brushBK(
        m_crColors[(m_bAlwaysBorder && !m_bMouseOnButton && !m_bIsPressed) ? BTNST_COLOR_BK_OUT : BTNST_COLOR_BK_IN]);
    CBrush* pOldBrush = pDC->SelectObject(&brushBK);

    // Create and select a thick black pen for button border
    CPen penBorder;
    penBorder.CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
    CPen* pOldPen = pDC->SelectObject(&penBorder);

    if (m_bIsRounded)
        pDC->RoundRect(pRect, CPoint(8, 8));
    else
        pDC->Rectangle(pRect);

    // Put back the old objects
    pDC->SelectObject(pOldBrush);
    pDC->SelectObject(pOldPen);

    return BTNST_OK;
} // End of OnDrawBackground

// This function enables or disables the rounded border for the button.
//
// Parameters:
//		[IN]	bRounded
//				If TRUE the button will have a round border.
//		[IN]	bRepaint
//				If TRUE the button will be repainted.
//
// Return value:
//		BTNST_OK
//			Function executed successfully.
//
DWORD CWinXPButtonST::SetRounded(BOOL bRounded, btn_border_type type, BOOL bRepaint)
{
    m_bIsRounded = bRounded;
    m_bAlwaysBorder = (type == SHOW_BORDER_ALWAYS);
    if (bRepaint)
        Invalidate();

    return BTNST_OK;
} // End of SetRounded

#undef BASE_BUTTONST
