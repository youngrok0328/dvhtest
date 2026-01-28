#pragma once
// Author : 송준호

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
#include "GridCell.h"

//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
class __INTEKPLUS_SHARED_GRIDCTRL_API__ CGridSlideBox : public CGridCell
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

    void AddString(CString strName);
    void SetCurSel(int nCur);
    int GetCurSel();

    // Operations
    virtual CSize GetCellExtent(CDC* pDC);
    virtual void OnClickDown(CPoint PointCellRelative);
    virtual void OnClickUp(CPoint PointCellRelative);
    virtual BOOL GetTextRect(LPRECT pRect);
    virtual LPCTSTR GetText() const;

protected:
    CRect GetCheckPlacementLeft();
    CRect GetCheckPlacementRight();

    virtual BOOL Draw(CDC* pDC, int nRow, int nCol, CRect rect, BOOL bEraseBkgnd = TRUE);

protected:
    std::vector<CString> m_vecItem;
    int m_nClick;
    int m_nSelect;
    CRect m_Rect;
    BOOL m_bEnabled;
};
