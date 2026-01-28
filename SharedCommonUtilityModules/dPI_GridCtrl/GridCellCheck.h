#pragma once

/////////////////////////////////////////////////////////////////////////////
// GridCellCheck.h : header file
//
// MFC Grid Control - Grid combo cell class header file
//
// Written by Chris Maunder <cmaunder@mail.com>
// Copyright (c) 1998-2002. All Rights Reserved.
//
// This code may be used in compiled form in any way you desire. This
// file may be redistributed unmodified by any means PROVIDING it is 
// not sold for profit without the authors written consent, and 
// providing that this notice and the authors name and all copyright 
// notices remains intact. 
//
// An email letting me know how you are using it would be nice as well. 
//
// This file is provided "as is" with no expressed or implied warranty.
// The author accepts no liability for any damage/loss of business that
// this product may cause.
//
// For use with CGridCtrl v2.22+
//
//////////////////////////////////////////////////////////////////////

#ifdef __DPI_GridCtrlDEF_EXPORTS__
#define dPI_GridCtrlDEF_API __declspec(dllexport)
#else
#define dPI_GridCtrlDEF_API __declspec(dllimport)
#endif

#include "GridCell.h"


class dPI_GridCtrlDEF_API CGridCellCheck : public CGridCell
{
    friend class CGridCtrl;
    DECLARE_DYNCREATE(CGridCellCheck)

public:
    CGridCellCheck();

public:
	BOOL SetCheck(BOOL bChecked = TRUE);
	BOOL GetCheck();
	void SetEditable(BOOL bEditable)	{m_bEditable = bEditable;};

// Operations
	virtual CSize GetCellExtent(CDC* pDC);
    virtual void OnClick( CPoint PointCellRelative);
    virtual BOOL GetTextRect( LPRECT pRect);

protected:
	CRect GetCheckPlacement();

    virtual BOOL Draw(CDC* pDC, int nRow, int nCol, CRect rect, BOOL bEraseBkgnd = TRUE);

protected:
    BOOL  m_bChecked;
	BOOL  m_bEditable;
    CRect m_Rect;
};
