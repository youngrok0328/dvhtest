//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "GridTreeCell.h"

//CPP_2_________________________________ This project's headers
#include "GridCtrl.h"
#include "InPlaceEdit.h"

//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
IMPLEMENT_DYNCREATE(CGridTreeCell, CGridTreeCellBase)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CGridTreeCell::CGridTreeCell()
    : CGridTreeCellBase()
{
}

CGridTreeCell::~CGridTreeCell()
{
}

void CGridTreeCell::Reset()
{
    CGridTreeCellBase::Reset();

    m_strText.Empty();
}

BOOL CGridTreeCell::Edit(int nRow, int nCol, CRect rect, CPoint /* point */, UINT nID, UINT nChar)
{
    ASSERT(m_pTreeColumn != NULL);
    DWORD dwStyle = ES_LEFT;
    if (GetFormat() & DT_RIGHT)
        dwStyle = ES_RIGHT;
    else if (GetFormat() & DT_CENTER)
        dwStyle = ES_CENTER;

    m_ucEditing = TRUE;

    // InPlaceEdit auto-deletes itself
    CGridCtrl* pGrid = GetGrid();
    m_pTreeColumn->SetEditWnd(new CInPlaceEdit(pGrid, rect, dwStyle, nID, nRow, nCol, GetText(), nChar));

    return TRUE;
}

void CGridTreeCell::EndEdit()
{
    CWnd* pEditWnd = m_pTreeColumn->GetEditWnd();
    if (pEditWnd)
        ((CInPlaceEdit*)(pEditWnd))->EndEdit();

    m_pTreeColumn->SetEditWnd(NULL);
}

void CGridTreeCell::OnEndEdit()
{
    EndEdit();
    m_ucEditing = FALSE;
}
