#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
#include "../../VisionNeedLibrary/VisionCommon/VisionProcessing.h"

//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <map>
#include <vector>

//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
class MenuBar : public CButton
{
    DECLARE_DYNAMIC(MenuBar)

public:
    MenuBar();
    virtual ~MenuBar();

    void SetSelectionChangedEvent(HWND target, UINT message);
    void SetFont(CFont* font);
    void SetTitle(LPCTSTR name);
    void DeleteAllItems();
    bool IsExistItem(long id) const;
    bool AddItem(long id, LPCTSTR text, bool enabled);
    bool SetItemStateNum(long id, long stateNum);
    bool SetItemState(long id, long stateIndex, VisionProcessingState::enumState state);
    bool SetActiveItem(long id);
    long GetActiveItemID();

protected:
    HWND m_selectionChanged_target;
    UINT m_selectionChanged_message;
    struct SMenuInfo
    {
        long m_id;
        CString m_text;
        CRect m_region;
        bool m_enabled;

        std::vector<long> m_states;
    };

    CString m_title;
    std::map<long, long> m_menuIDtoIndex;
    std::vector<SMenuInfo> m_menus;
    CFont m_textFont;
    long m_activeItemID;
    long m_itemHeight;

    void DrawIntialize();

    DECLARE_MESSAGE_MAP()

public:
    afx_msg void OnPaint();
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
    afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
};
