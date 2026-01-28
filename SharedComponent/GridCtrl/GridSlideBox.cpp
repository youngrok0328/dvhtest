//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "GridSlideBox.h"

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
IMPLEMENT_DYNCREATE(CGridSlideBox, CGridCell)

CGridSlideBox::CGridSlideBox()
    : CGridCell()
{
    m_bEnabled = TRUE;
    m_nSelect = -1;
    m_nClick = 0;
}

CSize CGridSlideBox::GetCellExtent(CDC* pDC)
{
    // Using SM_CXHSCROLL as a guide to the size of the checkbox
    int nWidth = GetSystemMetrics(SM_CXHSCROLL) + 2 * GetMargin();
    return CGridCell::GetCellExtent(pDC) + CSize(nWidth, nWidth);
}

// i/o:  i=dims of cell rect; o=dims of text rect
BOOL CGridSlideBox::GetTextRect(LPRECT pRect)
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
BOOL CGridSlideBox::Draw(CDC* pDC, int nRow, int nCol, CRect rect, BOOL bEraseBkgnd /*=TRUE*/)
{
    UNREFERENCED_PARAMETER(bEraseBkgnd);
    UNREFERENCED_PARAMETER(nCol);
    UNREFERENCED_PARAMETER(nRow);

    //    BOOL bResult = CGridCell::Draw(pDC, nRow, nCol, rect,  bEraseBkgnd);

#ifndef _WIN32_WCE
    // Store the cell's dimensions for later
    m_Rect = rect;

    CRect CheckRect1 = GetCheckPlacementLeft();
    CRect CheckRect2 = GetCheckPlacementRight();

    rect.left = CheckRect1.right;
    rect.right = CheckRect2.left;

    // enough room to draw?
    // if (CheckRect.Width() < rect.Width() && CheckRect.Height() < rect.Height()) {

    CPoint pt1[3], pt2[3];
    pt1[0] = CPoint(CheckRect1.left + 5, CheckRect1.top + CheckRect1.Height() / 2);
    pt1[1] = CPoint(CheckRect1.right - 5, CheckRect1.top);
    pt1[2] = CPoint(CheckRect1.right - 5, CheckRect1.bottom);

    pt2[0] = CPoint(CheckRect2.right - 5, CheckRect2.top + CheckRect1.Height() / 2);
    pt2[1] = CPoint(CheckRect2.left + 5, CheckRect2.top);
    pt2[2] = CPoint(CheckRect2.left + 5, CheckRect2.bottom);

    CBrush Brush_ButtonNormal;
    CBrush Brush_ButtonActive;
    CBrush Brush_ButtonDisable;

    CBrush* pOldBrush = pDC->GetCurrentBrush();

    Brush_ButtonNormal.CreateSolidBrush(RGB(0, 0, 255));
    Brush_ButtonActive.CreateSolidBrush(RGB(0, 255, 0));
    Brush_ButtonDisable.CreateSolidBrush(RGB(128, 0, 0));

    if (m_nClick == 1)
    {
        pDC->SelectObject(&Brush_ButtonActive);
    }
    else
    {
        if (m_nSelect == 0)
        {
            pDC->SelectObject(&Brush_ButtonDisable);
        }
        else
        {
            pDC->SelectObject(&Brush_ButtonNormal);
        }
    }

    pDC->Polygon(pt1, 3);

    if (m_nClick == 2)
    {
        pDC->SelectObject(&Brush_ButtonActive);
    }
    else
    {
        if (m_nSelect == (long)m_vecItem.size() - 1)
        {
            pDC->SelectObject(&Brush_ButtonDisable);
        }
        else
        {
            pDC->SelectObject(&Brush_ButtonNormal);
        }
    }

    pDC->Polygon(pt2, 3);

    pDC->SelectObject(pOldBrush);

    if (IsFocused())
    {
        pDC->FillSolidRect(rect, ::GetSysColor(COLOR_HIGHLIGHT));
        pDC->SetTextColor(RGB(255, 255, 255));
    }
    else
    {
        CGridDefaultCell* pDefaultCell = (CGridDefaultCell*)GetDefaultCell();
        pDC->FillSolidRect(rect, pDefaultCell->GetBackClr());
        pDC->SetTextColor(RGB(0, 0, 0));
    }
    // Do the draw
    if (m_nSelect >= 0)
    {
        pDC->SetBkMode(TRANSPARENT);
        pDC->DrawText(m_vecItem[m_nSelect], rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        pDC->SetBkMode(OPAQUE);
    }

    // }
#endif
    return TRUE;
}

void CGridSlideBox::OnClickDown(CPoint PointCellRelative)
{
    // PointCellRelative is relative to the topleft of the cell. Convert to client coords
    PointCellRelative += m_Rect.TopLeft();

    // GetCheckPlacement returns the checkbox dimensions in client coords. Only check/
    // uncheck if the user clicked in the box
    if (GetCheckPlacementLeft().PtInRect(PointCellRelative) && m_nSelect > 0)
    {
        m_nClick = 1;
        GetGrid()->InvalidateRect(m_Rect);
        return;
    }

    if (GetCheckPlacementRight().PtInRect(PointCellRelative) && m_nSelect < (long)m_vecItem.size() - 1)
    {
        m_nClick = 2;
        GetGrid()->InvalidateRect(m_Rect);
        return;
    }
}

void CGridSlideBox::OnClickUp(CPoint PointCellRelative)
{
    UNREFERENCED_PARAMETER(PointCellRelative);

    long nOldSelect = m_nSelect;

    if (m_nClick == 1)
    {
        m_nSelect--;
    }

    if (m_nClick == 2)
    {
        m_nSelect++;
    }

    m_nClick = 0;
    GetGrid()->InvalidateRect(m_Rect);

    if (nOldSelect != m_nSelect)
    {
        CGridCtrl* pGrid = GetGrid();
        CCellID cell = pGrid->GetFocusCell();
        SendMessageToParent(cell.row, cell.col, GVN_STATECHANGED);
    }
}

//////////////////////////////////////////////////////////////////////
// Operations
//////////////////////////////////////////////////////////////////////

void CGridSlideBox::SetCurSel(int nCur)
{
    m_nSelect = nCur;

    if (!m_Rect.IsRectEmpty())
        GetGrid()->InvalidateRect(m_Rect);
}

int CGridSlideBox::GetCurSel()
{
    return m_nSelect;
}

//////////////////////////////////////////////////////////////////////
// Protected implementation
//////////////////////////////////////////////////////////////////////

// Returns the dimensions and placement of the checkbox in client coords.
CRect CGridSlideBox::GetCheckPlacementLeft()
{
    CRect place;
    place.left = m_Rect.left;
    place.top = m_Rect.top;
    place.bottom = m_Rect.bottom;
    place.right = place.left + 20;

    return place;
}

CRect CGridSlideBox::GetCheckPlacementRight()
{
    CRect place;
    place.left = m_Rect.right - 20;
    place.top = m_Rect.top;
    place.bottom = m_Rect.bottom;
    place.right = m_Rect.right;

    return place;
}

BOOL CGridSlideBox::Edit(int nRow, int nCol, CRect /* rect */, CPoint /* point */, UINT /* nID */, UINT nChar)
{
    UNREFERENCED_PARAMETER(nChar);
    UNREFERENCED_PARAMETER(nCol);
    UNREFERENCED_PARAMETER(nRow);

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

void CGridSlideBox::AddString(CString strName)
{
    m_vecItem.push_back(strName);
    if (!m_Rect.IsRectEmpty())
        GetGrid()->InvalidateRect(m_Rect);
}

void CGridSlideBox::EndEdit()
{
}

BOOL CGridSlideBox::Enable(BOOL bEnable /*=TRUE*/)
{
    BOOL bTemp = m_bEnabled;
    m_bEnabled = bEnable;
    if (!m_Rect.IsRectEmpty())
        GetGrid()->InvalidateRect(m_Rect);

    return bTemp;
}

BOOL CGridSlideBox::GetEnabled()
{
    return m_bEnabled;
}

void CGridSlideBox::DeleteAllItems()
{
    m_vecItem.clear();
    m_nSelect = -1;

    if (!m_Rect.IsRectEmpty())
        GetGrid()->InvalidateRect(m_Rect);
}

LPCTSTR CGridSlideBox::GetText() const
{
    if (m_nSelect >= 0 && m_nSelect < (long)m_vecItem.size() && (long)m_vecItem.size() > 0)
        return m_vecItem[m_nSelect];
    return _T("");
}
