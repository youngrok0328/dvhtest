#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
#include "GridCellCheck.h"

//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
class __INTEKPLUS_SHARED_GRIDCTRL_API__ CGridCellCheck2 : public CGridCellCheck
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
    virtual void OnClick(CPoint PointCellRelative);

protected:
    //CRect GetCheckPlacement();
    virtual BOOL Draw(CDC* pDC, int nRow, int nCol, CRect rect, BOOL bEraseBkgnd = TRUE);

    LRESULT SendMessageToParent(int nRow, int nCol, int nMessage);

protected:
    BOOL m_bEnabled;
};
