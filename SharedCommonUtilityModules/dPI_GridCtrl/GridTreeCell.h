#pragma once

// 이현식 제작
#ifdef __DPI_GridCtrlDEF_EXPORTS__
#define dPI_GridCtrlDEF_API __declspec(dllexport)
#else
#define dPI_GridCtrlDEF_API __declspec(dllimport)
#endif


#include "GridTreeCellBase.h"

// Place these Grid Cells in a column in a grid to
//  create a tree control
class dPI_GridCtrlDEF_API CGridTreeCell : public CGridTreeCellBase
{
	friend class CGridCtrl;
    DECLARE_DYNCREATE(CGridTreeCell)

// Construction/Destruction
public:
    CGridTreeCell();
    virtual ~CGridTreeCell();


// base Attributes
public:
    virtual void SetText(LPCTSTR szText);// { m_strText = szText; }
    virtual LPCTSTR  GetText() const;//      { return (m_strText.IsEmpty())? _T("") : m_strText; }

    virtual void Reset();

    virtual void operator=(CGridCellBase& cell)
    {
        CGridTreeCellBase::operator=( cell);
    }


// editing cells
public:
    BOOL Edit(int nRow, int nCol, CRect rect, CPoint /* point */, UINT nID, UINT nChar);
    void EndEdit();
protected:
    void OnEndEdit();

protected:
    CString  m_strText;     // Cell text (or binary data if you wish...)
};
