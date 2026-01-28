#pragma once
// Author : 곽영보

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
class __INTEKPLUS_SHARED_GRIDCTRL_API__ CGridButton : public CGridCell
{
    friend class CGridCtrl;
    DECLARE_DYNCREATE(CGridButton)

public:
    enum grid_button_type
    {
        Grid_Button_Type_Normal,
        Grid_Button_Type_Color,
        Grid_Button_Type_End
    };

    CGridButton();

public:
    virtual BOOL Edit(int nRow, int nCol, CRect rect, CPoint point, UINT nID, UINT nChar);
    virtual void EndEdit();

    BOOL Enable(BOOL bEnable = TRUE);
    BOOL GetEnabled();
    void SetButtonType(grid_button_type nBtnType);
    COLORREF GetColorValue();
    void SetColorValue(COLORREF clr);

    // Operations
    virtual CSize GetCellExtent(CDC* pDC);
    virtual void OnClickDown(CPoint PointCellRelative);
    virtual void OnClickUp(CPoint PointCellRelative);
    virtual BOOL GetTextRect(LPRECT pRect);
    //	virtual LPCTSTR     GetText() const;

protected:
    CRect GetButtonRect(int OffsetX = 0, int OffsetY = 0);
    CRect GetColorRect(int OffsetX = 0, int OffsetY = 0);

    virtual BOOL Draw(CDC* pDC, int nRow, int nCol, CRect rect, BOOL bEraseBkgnd = TRUE);

protected:
    int m_nClick;
    int m_nButtonType;
    CRect m_Rect;
    BOOL m_bEnabled;
    COLORREF m_clrValue;
};
