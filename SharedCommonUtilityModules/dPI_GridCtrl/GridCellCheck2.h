#pragma once

#ifdef __DPI_GridCtrlDEF_EXPORTS__
#define dPI_GridCtrlDEF_API __declspec(dllexport)
#else
#define dPI_GridCtrlDEF_API __declspec(dllimport)
#endif

#include "GridCellCheck.h"

class dPI_GridCtrlDEF_API CGridCellCheck2 : public CGridCellCheck
{
    friend class CGridCtrl;
    DECLARE_DYNCREATE(CGridCellCheck2)

public:
    CGridCellCheck2();

public:
    virtual BOOL Edit(int nRow, int nCol, CRect rect, CPoint point, UINT nID, UINT nChar);
    virtual void EndEdit();

	BOOL Enable(BOOL bEnable = TRUE);
	BOOL GetEnabled();
    virtual void OnClick( CPoint PointCellRelative);

protected:
	//CRect GetCheckPlacement();
    virtual BOOL Draw(CDC* pDC, int nRow, int nCol, CRect rect, BOOL bEraseBkgnd = TRUE);

    LRESULT SendMessageToParent(int nRow, int nCol, int nMessage);

protected:
    BOOL  m_bEnabled;
};
