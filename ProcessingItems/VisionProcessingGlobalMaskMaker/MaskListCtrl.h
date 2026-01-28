#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <memory>
#include <vector>

//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
enum MaskListStyle
{
    None,
    LastItemFixed,
    AllItemFixed,
    MultiSelectItems,
};

class MaskListCtrl : public CButton
{
    DECLARE_DYNAMIC(MaskListCtrl)

public:
    MaskListCtrl(DWORD style);
    virtual ~MaskListCtrl();

    void DeleteAllItems();
    bool AddItem(long key, LPCTSTR name, LPCTSTR state);
    void SetItemName(long key, LPCTSTR name);
    void SetItemState(long key, LPCTSTR state);
    void SetItemInvalid(long key, bool isValid);
    void SelectItem(long key);
    long GetSelectedItemKey() const;
    const std::vector<long>& GetSelectedMultiItemsKey() const;

    void SetEvent_StateDBClick(HWND parent, UINT message);
    void SetEvent_ItemRClick(HWND parent, UINT message);
    void SetEvent_ItemSelected(HWND parent, UINT message);
    void SetEvent_ItemMoved(HWND parent, UINT message);

protected:
    struct ItemInfo
    {
        long m_key;
        long m_index;
        bool m_isValue;

        CString m_name;
        CString m_state;
        CRect m_region;
        CRect m_stateRegion;
        CRect m_nameRegion;
        CRect m_jogRegion;
    };

    std::vector<ItemInfo> m_items;
    std::vector<long> m_selectedItemIndexList;
    long m_activatedItemIndex;
    long m_rowHeight;
    DWORD m_style;

    long m_jogDrag_itemIndex;
    long m_jogDrag_itemDestionationIndex;
    CPoint m_jogDrag_MousePosition;

    std::pair<HWND, UINT> m_eventStateDBClick;
    std::pair<HWND, UINT> m_eventRClick;
    std::pair<HWND, UINT> m_eventSelected;
    std::pair<HWND, UINT> m_eventMoved;

    void DrawIntialize();
    void CalcItemRegion(const CRect& itemRegion, CRect& o_state, CRect& o_name, CRect& o_jog);
    long FirstItemStartY();
    bool IsJogEnable(ItemInfo& item);
    long JogRequiredDestinationIndex(long cx);

    void DrawJog(CDC& dc, const CRect& region);
    void DrawMaskItem(CDC& dc, long ltY, long cx, long cy, ItemInfo& item);

    DECLARE_MESSAGE_MAP()

public:
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
    afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
    afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
    virtual void DrawItem(LPDRAWITEMSTRUCT /*lpDrawItemStruct*/);
};
