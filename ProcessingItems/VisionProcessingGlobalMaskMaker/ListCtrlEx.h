#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <map>

//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
class ListCtrlEx : public CListCtrl
{
    DECLARE_DYNAMIC(ListCtrlEx)

public:
    ListCtrlEx();
    virtual ~ListCtrlEx();

    BOOL DeleteAllItems();
    void SetItemColor(int nItem, int nSubItem, COLORREF color);

protected:
    std::map<int, std::map<int, COLORREF>> m_items_color;

    DECLARE_MESSAGE_MAP()

public:
    afx_msg void OnNMCustomdraw(NMHDR* pNMHDR, LRESULT* pResult);
};
