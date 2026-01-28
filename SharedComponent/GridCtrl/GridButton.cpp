//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "GridButton.h"

//CPP_2_________________________________ This project's headers
#include "GridCell.h"
#include "GridCtrl.h"

//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
IMPLEMENT_DYNCREATE(CGridButton, CGridCell)

CGridButton::CGridButton()
    : CGridCell()
{
    m_bEnabled = TRUE;
    m_nClick = 0;
    m_nButtonType = Grid_Button_Type_Color;
    m_clrValue = RGB(128, 128, 128);
}

void CGridButton::SetButtonType(grid_button_type nBtnType)
{
    m_nButtonType = nBtnType;
}

CSize CGridButton::GetCellExtent(CDC* pDC)
{
    // Using SM_CXHSCROLL as a guide to the size of the checkbox
    int nWidth = GetSystemMetrics(SM_CXHSCROLL) + 2 * GetMargin();
    return CGridCell::GetCellExtent(pDC) + CSize(nWidth, nWidth);
}

// i/o:  i=dims of cell rect; o=dims of text rect
BOOL CGridButton::GetTextRect(LPRECT pRect)
{
    BOOL bResult = CGridCell::GetTextRect(pRect);
    if (bResult)
    {
        int nWidth = GetSystemMetrics(SM_CXHSCROLL) + 2 * GetMargin();
        pRect->left += nWidth;
        if (pRect->left > pRect->right)
            pRect->left = pRect->right;
    }
    return bResult;
}

// Override draw so that when the cell is selected, a drop arrow is shown in the RHS.
BOOL CGridButton::Draw(CDC* pDC, int nRow, int nCol, CRect rect, BOOL bEraseBkgnd /*=TRUE*/)
{
    UNREFERENCED_PARAMETER(bEraseBkgnd);
    UNREFERENCED_PARAMETER(nCol);
    UNREFERENCED_PARAMETER(nRow);

    //    BOOL bResult = CGridCell::Draw(pDC, nRow, nCol, rect,  bEraseBkgnd);
    m_Rect = rect;
    CRect BtnRect = GetButtonRect();

    pDC->SetBkMode(TRANSPARENT);
    if (m_nClick == 0)
    {
        pDC->DrawFrameControl(BtnRect, DFC_BUTTON, DFCS_BUTTONPUSH | DFCS_FLAT);
        pDC->DrawText(GetText(), GetButtonRect(0, 0), DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }
    else
    {
        pDC->DrawFrameControl(BtnRect, DFC_BUTTON, DFCS_BUTTONPUSH | DFCS_PUSHED);
        pDC->DrawText(GetText(), GetButtonRect(1, 1), DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }
    if (m_nButtonType == Grid_Button_Type_Color) // 버튼 Type이 Color를 지정하는 것이면
    {
        CBrush clrBrush(m_clrValue);
        CBrush* pOldBrush = pDC->SelectObject(&clrBrush);
        //pDC->Draw3dRect(GetColorRect(), RGB(200,200,200), RGB(128,128,128));
        pDC->Rectangle(GetColorRect());
        pDC->SelectObject(pOldBrush);
    }

    pDC->SetBkMode(OPAQUE);
    return TRUE;
}

void CGridButton::OnClickDown(CPoint PointCellRelative)
{
    if (0 < PointCellRelative.x && 0 < PointCellRelative.y && m_Rect.Width() > PointCellRelative.x
        && m_Rect.Height() > PointCellRelative.y)
    {
        m_nClick = 1;
        GetGrid()->InvalidateRect(m_Rect);
    }
}

void CGridButton::OnClickUp(CPoint PointCellRelative)
{
    if (m_nClick == 0)
        return;

    m_nClick = 0;
    // 버튼 모양 Update
    GetGrid()->InvalidateRect(m_Rect);
    if (0 < PointCellRelative.x && 0 < PointCellRelative.y && m_Rect.Width() > PointCellRelative.x
        && m_Rect.Height() > PointCellRelative.y && m_nButtonType == Grid_Button_Type_Color)
    {
        CColorDialog _Dlg;

        if (IDOK == _Dlg.DoModal())
        {
            m_clrValue = _Dlg.GetColor();
            // 색깔 Update
            GetGrid()->InvalidateRect(m_Rect);
        }
    }
    //else if(m_nButtonType == Grid_Button_Type_Normal)
    {
        CGridCtrl* pGrid = GetGrid();
        CCellID cell = pGrid->GetFocusCell();
        SendMessageToParent(cell.row, cell.col, GVN_COLUMNCLICK);
    }
}

COLORREF CGridButton::GetColorValue()
{
    return m_clrValue;
}

void CGridButton::SetColorValue(COLORREF clr)
{
    m_clrValue = clr;
}
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
// Protected implementation
//////////////////////////////////////////////////////////////////////

// Returns the dimensions and placement of the checkbox in client coords.
CRect CGridButton::GetButtonRect(int OffsetX, int OffsetY)
{
    CRect place;
    place.top = m_Rect.top + 1 + OffsetY;
    place.bottom = m_Rect.bottom - 2 + OffsetY;

    switch (m_nButtonType)
    {
        case Grid_Button_Type_Color:
            place.left = (m_Rect.left + m_Rect.right) / 2 + 1 + OffsetX;
            place.right = m_Rect.right - 1 + OffsetX;
            break;
        default:
            place.left = m_Rect.left + 1 + OffsetX;
            place.right = m_Rect.right - 1 + OffsetX;
            break;
    }

    return place;
}

CRect CGridButton::GetColorRect(int OffsetX, int OffsetY)
{
    CRect place{};
    place.top = m_Rect.top + 1 + OffsetY;
    place.bottom = m_Rect.bottom - 2 + OffsetY;

    switch (m_nButtonType)
    {
        case Grid_Button_Type_Color:
            place.left = m_Rect.left + 1 + OffsetX;
            place.right = (m_Rect.left + m_Rect.right) / 2 - 1 + OffsetX;
            break;
        default:
            // 색깔 지정이 안되는 버튼이다.
            break;
    }

    return place;
}

BOOL CGridButton::Edit(int nRow, int nCol, CRect /* rect */, CPoint /* point */, UINT /* nID */, UINT nChar)
{
    UNREFERENCED_PARAMETER(nChar);
    UNREFERENCED_PARAMETER(nRow);
    UNREFERENCED_PARAMETER(nCol);

    if (m_bEnabled)
    {
        /*		if (nChar == VK_SPACE)
		{
			CGridCtrl* pGrid = GetGrid();
			m_bChecked = !m_bChecked;
			pGrid->InvalidateRect(m_Rect);

			//pGrid->SendMessageToParent(nRow, nCol, GVN_STATECHANGED);	// This is protected member, so...
			SendMessageToParent(nRow, nCol, GVN_STATECHANGED);
		}*/
    }
    return TRUE;
}

void CGridButton::EndEdit()
{
}

BOOL CGridButton::Enable(BOOL bEnable /*=TRUE*/)
{
    BOOL bTemp = m_bEnabled;
    m_bEnabled = bEnable;
    if (!m_Rect.IsRectEmpty())
        GetGrid()->InvalidateRect(m_Rect);

    return bTemp;
}

BOOL CGridButton::GetEnabled()
{
    return m_bEnabled;
}
