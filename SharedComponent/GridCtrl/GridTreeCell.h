#pragma once
// Author : 이현식

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
#include "GridTreeCellBase.h"

//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
// Place these Grid Cells in a column in a grid to
//  create a tree control
class __INTEKPLUS_SHARED_GRIDCTRL_API__ CGridTreeCell : public CGridTreeCellBase
{
    friend class CGridCtrl;
    DECLARE_DYNCREATE(CGridTreeCell)

    // Construction/Destruction

public:
    CGridTreeCell();
    virtual ~CGridTreeCell();

    // base Attributes

public:
    virtual void SetText(LPCTSTR szText)
    {
        m_strText = szText;
    }
    virtual LPCTSTR GetText() const
    {
        return (m_strText.IsEmpty()) ? CString(_T("")) : m_strText;
    }

    virtual void Reset();

    virtual void operator=(CGridCellBase& cell)
    {
        CGridTreeCellBase::operator=(cell);
    }

    // editing cells

public:
    BOOL Edit(int nRow, int nCol, CRect rect, CPoint /* point */, UINT nID, UINT nChar);
    void EndEdit();

protected:
    void OnEndEdit();

protected:
    CString m_strText; // Cell text (or binary data if you wish...)
};
