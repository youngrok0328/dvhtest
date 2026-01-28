#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
#include "GridCell.h"

//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
class __INTEKPLUS_SHARED_GRIDCTRL_API__ CGridCellImage : public CGridCell
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
