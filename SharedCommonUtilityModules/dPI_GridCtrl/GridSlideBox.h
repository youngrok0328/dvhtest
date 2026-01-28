#pragma once

// 송준호님 제작
#ifdef __DPI_GridCtrlDEF_EXPORTS__
#define dPI_GridCtrlDEF_API __declspec(dllexport)
#else
#define dPI_GridCtrlDEF_API __declspec(dllimport)
#endif

#include "GridCell.h"
#include <VECTOR>

class dPI_GridCtrlDEF_API CGridSlideBox : public CGridCell
{
    friend class CGridCtrl;
    DECLARE_DYNCREATE(CGridSlideBox)

public:
    CGridSlideBox();

public:
	void DeleteAllItems();
    virtual BOOL Edit(int nRow, int nCol, CRect rect, CPoint point, UINT nID, UINT nChar);
	virtual void EndEdit();

	BOOL Enable(BOOL bEnable = TRUE);
	BOOL GetEnabled();

	void	AddString(CString strName);
	void	SetCurSel(int nCur);
	int		GetCurSel();

// Operations
	virtual LPCTSTR GetText() const;
	virtual CSize GetCellExtent(CDC* pDC);
    virtual void OnClickDown( CPoint PointCellRelative);
	virtual void OnClickUp( CPoint PointCellRelative);
    virtual BOOL GetTextRect( LPRECT pRect);

protected:
	CRect GetCheckPlacementLeft();
	CRect GetCheckPlacementRight();

    virtual BOOL Draw(CDC* pDC, int nRow, int nCol, CRect rect, BOOL bEraseBkgnd = TRUE);

protected:
	std::vector<CString> m_vecItem;
	int m_nClick;
	int m_nSelect;
    CRect m_Rect;
    BOOL  m_bEnabled;
};
