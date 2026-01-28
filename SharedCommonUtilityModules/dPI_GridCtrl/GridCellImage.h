#pragma once

#ifdef __DPI_GridCtrlDEF_EXPORTS__
#define dPI_GridCtrlDEF_API __declspec(dllexport)
#else
#define dPI_GridCtrlDEF_API __declspec(dllimport)
#endif
#include "atlimage.h"
#include "GridCell.h"

// #pragma once
// #include "gridcell.h"
// 
// class  :
// 	public CGridCell
// {
// public:
// 	CGridCellImage(void);
// 	~CGridCellImage(void);
// };
class dPI_GridCtrlDEF_API CGridCellImage : public CGridCell
{
	friend class CGridCtrl;
	DECLARE_DYNCREATE(CGridCellImage)

public:
	CGridCellImage();

public:
// 	BOOL SetCheck(BOOL bChecked = TRUE);
// 	BOOL GetCheck();
// 
// 	// Operations
// 	virtual CSize GetCellExtent(CDC* pDC);
// 	virtual void OnClick( CPoint PointCellRelative);
// 	virtual BOOL GetTextRect( LPRECT pRect);
// 	CRect GetCheckPlacement();
	void SetCellImage(CString image);
protected:
	
	
	CString m_Image;
	virtual BOOL Draw(CDC* pDC, int nRow, int nCol, CRect rect, BOOL bEraseBkgnd = TRUE);
};
