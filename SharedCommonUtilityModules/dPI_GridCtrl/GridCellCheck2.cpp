#include "stdafx.h"

#include "GridCellCheck2.h"
#include "GridCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define GVN_STATECHANGED        LVN_ODSTATECHANGED	//YR - posted when combo selection is changed and check state is changed

IMPLEMENT_DYNCREATE(CGridCellCheck2, CGridCellCheck)

CGridCellCheck2::CGridCellCheck2() : CGridCellCheck()
{
	m_bEnabled = TRUE;
}

BOOL CGridCellCheck2::Edit(int nRow, int nCol, CRect /* rect */, CPoint /* point */, 
						   UINT /* nID */, UINT nChar)
{
	if (m_bEnabled)
	{
		if (nChar == VK_SPACE)
		{
			CGridCtrl* pGrid = GetGrid();
			m_bChecked = !m_bChecked;
			pGrid->InvalidateRect(m_Rect);

			//pGrid->SendMessageToParent(nRow, nCol, GVN_STATECHANGED);	// This is protected member, so...
			SendMessageToParent(nRow, nCol, GVN_STATECHANGED);
		}
	}
    return TRUE;
}

LRESULT CGridCellCheck2::SendMessageToParent(int nRow, int nCol, int /*nMessage*/)
{
    CGridCtrl* pGrid = GetGrid();
    if( pGrid)
	{
		if (!::IsWindow(pGrid->GetSafeHwnd()))
			return FALSE;

		NM_GRIDVIEW nmgv;
		nmgv.iRow         = nRow;
		nmgv.iColumn      = nCol;
		nmgv.hdr.hwndFrom = pGrid->GetSafeHwnd();
		nmgv.hdr.idFrom   = pGrid->GetDlgCtrlID();
		nmgv.hdr.code     = GVN_STATECHANGED;

		CWnd *pOwner = pGrid->GetOwner();
		if (pOwner && IsWindow(pOwner->m_hWnd))
			return pOwner->SendMessage(WM_NOTIFY, nmgv.hdr.idFrom, (LPARAM)&nmgv);
		else return 0;
	}
    else
        return 0;
}

void CGridCellCheck2::EndEdit()
{
}

BOOL CGridCellCheck2::Enable(BOOL bEnable /*=TRUE*/)
{
	BOOL bTemp = m_bEnabled;
	m_bEnabled = bEnable;
	if (!m_Rect.IsRectEmpty())
		GetGrid()->InvalidateRect(m_Rect);

	return bTemp;
}

BOOL CGridCellCheck2::GetEnabled()
{
	return m_bEnabled;
}

/*
// Returns the dimensions and placement of the checkbox in client coords.
CRect CGridCellCheck2::GetCheckPlacement()
{
	CRect place;
	int nWidth = GetSystemMetrics(SM_CXHSCROLL);

	// for centering
	int nDiff = (m_Rect.Width() - nWidth) / 2;
	place.left = m_Rect.left + nDiff;
	place.right = place.left + nWidth;

	nDiff = (m_Rect.Height() - nWidth) / 2;
	place.top = m_Rect.top + nDiff;
	place.bottom = place.top + nWidth;

	return place;
}
*/

// Override draw so that when the cell is selected, a drop arrow is shown in the RHS.
BOOL CGridCellCheck2::Draw(CDC* pDC, int nRow, int nCol, CRect rect,  BOOL bEraseBkgnd /*=TRUE*/)
{
	COLORREF saveClr = GetBackClr();
	
	if (!m_bEnabled)
		SetBackClr(::GetSysColor(COLOR_BTNFACE));
    
	BOOL bResult = CGridCell::Draw(pDC, nRow, nCol, rect, bEraseBkgnd);

	SetBackClr(saveClr);

#ifndef _WIN32_WCE
    // Store the cell's dimensions for later
    m_Rect = rect;

    CRect CheckRect = GetCheckPlacement();
    rect.left = CheckRect.right;

    // Do the draw 
	if (m_bEnabled)
		pDC->DrawFrameControl(GetCheckPlacement(), DFC_BUTTON, 
			(m_bChecked)? DFCS_BUTTONCHECK | DFCS_CHECKED : DFCS_BUTTONCHECK);
	else
		pDC->DrawFrameControl(GetCheckPlacement(), DFC_BUTTON, DFCS_INACTIVE);

#endif
    return bResult;
}

void CGridCellCheck2::OnClick(CPoint /*PointCellRelative*/)
{
	if(!m_bEditable)	return ;

	BOOL bOld = m_bChecked;

	// Respond althouch user clicks anywhere within the cell
	//CGridCellCheck::OnClick(PointCellRelative);

	// YR: for readonly check cells
	if (GetState() & GVIS_READONLY)
		return;

	m_bChecked = !m_bChecked;
	GetGrid()->InvalidateRect(m_Rect);

	if (bOld != m_bChecked)
	{
		CGridCtrl* pGrid = GetGrid();
		CCellID cell = pGrid->GetFocusCell();
		SendMessageToParent(cell.row, cell.col, GVN_STATECHANGED);
	}
}
