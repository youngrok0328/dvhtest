#pragma once

//////////////////////////////////////////////////////////////////////
//
// GridCellDateTime.h: interface for the CGridCellDateTime class.
//
// Provides the implementation for a datetime picker cell type of the
// grid control.
//
// For use with CGridCtrl v2.22+
//
//////////////////////////////////////////////////////////////////////

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
class __INTEKPLUS_SHARED_GRIDCTRL_API__ CGridCellDateTime : public CGridCell
{
    friend class CGridCtrl;
    DECLARE_DYNCREATE(CGridCellDateTime)

    CTime m_cTime;
    DWORD m_dwStyle;

public:
    CGridCellDateTime();
    CGridCellDateTime(DWORD dwStyle);
    virtual ~CGridCellDateTime();
    virtual CSize GetCellExtent(CDC* pDC);

    // editing cells

public:
    void Init(DWORD dwStyle);
    virtual BOOL Edit(int nRow, int nCol, CRect rect, CPoint point, UINT nID, UINT nChar);
    virtual CWnd* GetEditWnd() const;
    virtual void EndEdit();

    CTime* GetTime()
    {
        return &m_cTime;
    };
    void SetTime(CTime time);
};
