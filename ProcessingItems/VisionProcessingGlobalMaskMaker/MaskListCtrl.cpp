//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "MaskListCtrl.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
IMPLEMENT_DYNAMIC(MaskListCtrl, CButton)

MaskListCtrl::MaskListCtrl(DWORD style)
    : m_activatedItemIndex(-1)
    , m_eventStateDBClick(0, 0)
    , m_eventRClick(0, 0)
    , m_eventSelected(0, 0)
    , m_eventMoved(0, 0)
    , m_jogDrag_itemIndex(-1)
    , m_jogDrag_itemDestionationIndex(-1)
    , m_rowHeight(20)
    , m_style(style)
{
}

MaskListCtrl::~MaskListCtrl()
{
}

void MaskListCtrl::DeleteAllItems()
{
    m_items.clear();
    m_selectedItemIndexList.clear();

    DrawIntialize();
}

bool MaskListCtrl::AddItem(long key, LPCTSTR name, LPCTSTR state)
{
    for (auto& item : m_items)
    {
        if (item.m_key == key)
        {
            ASSERT(!_T("같은 키의 아이템을 다시 추가할 수는 없다"));
            return false;
        }
    }

    ItemInfo item;
    item.m_key = key;
    item.m_name = name;
    item.m_state = state;
    item.m_isValue = false;
    item.m_index = long(m_items.size());
    m_items.push_back(item);

    DrawIntialize();

    return true;
}

void MaskListCtrl::SetItemName(long key, LPCTSTR name)
{
    for (long itemIndex = 0; itemIndex < long(m_items.size()); itemIndex++)
    {
        auto& item = m_items[itemIndex];

        if (item.m_key == key)
        {
            item.m_name = name;
            break;
        }
    }

    if (GetSafeHwnd())
        Invalidate(FALSE);
}

void MaskListCtrl::SetItemState(long key, LPCTSTR state)
{
    for (long itemIndex = 0; itemIndex < long(m_items.size()); itemIndex++)
    {
        auto& item = m_items[itemIndex];

        if (item.m_key == key)
        {
            item.m_state = state;
            break;
        }
    }

    if (GetSafeHwnd())
        Invalidate(FALSE);
}

void MaskListCtrl::SetItemInvalid(long key, bool isValid)
{
    for (long itemIndex = 0; itemIndex < long(m_items.size()); itemIndex++)
    {
        auto& item = m_items[itemIndex];

        if (item.m_key == key)
        {
            item.m_isValue = isValid;
            break;
        }
    }

    if (GetSafeHwnd())
        Invalidate(FALSE);
}

void MaskListCtrl::SelectItem(long key)
{
    m_selectedItemIndexList.clear();

    for (long itemIndex = 0; itemIndex < long(m_items.size()); itemIndex++)
    {
        auto& item = m_items[itemIndex];

        if (item.m_key == key)
        {
            m_selectedItemIndexList.push_back(itemIndex);
            break;
        }
    }

    if (GetSafeHwnd())
        Invalidate(FALSE);
}

long MaskListCtrl::GetSelectedItemKey() const
{
    if (m_selectedItemIndexList.size() == 0)
        return -1;

    long selectedItemIndex = m_selectedItemIndexList[0];

    if (selectedItemIndex < 0 || selectedItemIndex >= long(m_items.size()))
    {
        return -1;
    }
    return m_items[selectedItemIndex].m_key;
}

const std::vector<long>& MaskListCtrl::GetSelectedMultiItemsKey() const
{
    return m_selectedItemIndexList;
}

void MaskListCtrl::SetEvent_StateDBClick(HWND parent, UINT message)
{
    m_eventStateDBClick = std::make_pair(parent, message);
}

void MaskListCtrl::SetEvent_ItemRClick(HWND parent, UINT message)
{
    m_eventRClick = std::make_pair(parent, message);
}

void MaskListCtrl::SetEvent_ItemSelected(HWND parent, UINT message)
{
    m_eventSelected = std::make_pair(parent, message);
}

void MaskListCtrl::SetEvent_ItemMoved(HWND parent, UINT message)
{
    m_eventMoved = std::make_pair(parent, message);
}

void MaskListCtrl::DrawIntialize()
{
    CRect clientRegion;
    GetClientRect(clientRegion);

    long drawItemCount = long(m_items.size()) + 1 /* Header */;

    long realDrawSizeY = drawItemCount * m_rowHeight;

    if (realDrawSizeY > clientRegion.Height())
    {
        // Scroll Bar를 사용한다면
        int scrollMax = realDrawSizeY - clientRegion.Height();
        int min = 0;
        int max = 0;
        GetScrollRange(SB_VERT, &min, &max);
        if (realDrawSizeY != scrollMax)
        {
            SetScrollRange(SB_VERT, 0, scrollMax);
            SetScrollPos(SB_VERT, 0);
            ShowScrollBar(SB_VERT, TRUE);
        }
    }
    else
    {
        SetScrollRange(SB_VERT, 0, 0);
        SetScrollPos(SB_VERT, 0);
        ShowScrollBar(SB_VERT, FALSE);
    }

    if (GetSafeHwnd())
        Invalidate(FALSE);
}

void MaskListCtrl::CalcItemRegion(const CRect& itemRegion, CRect& o_state, CRect& o_name, CRect& o_jog)
{
    long c_state_size_x = 30;
    long c_jog_size_x = 30;
    long c_name_size_x = itemRegion.Width() - c_state_size_x - c_jog_size_x;

    o_state = itemRegion;
    o_name = itemRegion;
    o_jog = itemRegion;

    o_state.right = c_state_size_x;
    o_name.left = o_state.right;
    o_name.right = o_name.left + c_name_size_x;
    o_jog.left = o_name.right;
    o_jog.right = o_jog.left + c_jog_size_x;
}

long MaskListCtrl::FirstItemStartY()
{
    return m_rowHeight - GetScrollPos(SB_VERT);
}

bool MaskListCtrl::IsJogEnable(ItemInfo& item)
{
    if (m_style & MaskListStyle::LastItemFixed)
    {
        if (item.m_index == long(m_items.size()) - 1)
            return false;
    }

    if (m_style & MaskListStyle::AllItemFixed)
    {
        return false;
    }

    return true;
}

long MaskListCtrl::JogRequiredDestinationIndex(long cx)
{
    // Insert 할 위치 검색 (Drag중인 아이는 없는것으로 생각하고 Insert위치를 결정한다)
    if (m_jogDrag_itemIndex >= 0)
    {
        long ltY = FirstItemStartY();
        long screenStartIndex = -1;
        for (auto& item : m_items)
        {
            if (!IsJogEnable(item))
                continue;

            CRect region(0, ltY, cx, ltY + m_rowHeight);

            if (region.top >= 0 && screenStartIndex < 0)
            {
                screenStartIndex = item.m_index;
            }
            if (region.PtInRect(CPoint(0, m_jogDrag_MousePosition.y)))
            {
                return item.m_index;
            }

            ltY += m_rowHeight;
        }

        screenStartIndex = max(screenStartIndex, 0);
        if (m_jogDrag_MousePosition.y < m_rowHeight)
        {
            // 헤더위쪽
            return screenStartIndex;
        }
        else
        {
            if (m_style & MaskListStyle::LastItemFixed)
            {
                return long(m_items.size()) - 2;
            }
            else
            {
                return long(m_items.size()) - 1;
            }
        }
    }

    return -1;
}

void MaskListCtrl::DrawJog(CDC& dc, const CRect& region)
{
    dc.FillSolidRect(region.left + 5, region.CenterPoint().y - 4, region.Width() - 10, 4, RGB(128, 128, 128));
    dc.FillSolidRect(region.left + 5, region.CenterPoint().y + 2, region.Width() - 10, 1, RGB(128, 128, 128));
}

void MaskListCtrl::DrawMaskItem(CDC& dc, long ltY, long cx, long cy, ItemInfo& item)
{
    COLORREF normalColor = RGB(255, 255, 196);
    COLORREF selectedColor = RGB(0, 120, 215);
    COLORREF activatedColor = RGB(200, 200, 0);

    if (!IsWindowEnabled())
    {
        normalColor = selectedColor = activatedColor = RGB(64, 64, 64);
    }

    CRect& itemRegion = item.m_region;
    CRect& stateRegion = item.m_stateRegion;
    CRect& nameRegion = item.m_nameRegion;
    CRect& jogRegion = item.m_jogRegion;

    itemRegion = CRect(0, ltY, cx, ltY + cy);

    CalcItemRegion(itemRegion, stateRegion, nameRegion, jogRegion);

    if (std::find(m_selectedItemIndexList.begin(), m_selectedItemIndexList.end(), item.m_index)
        != m_selectedItemIndexList.end())
    {
        dc.FillSolidRect(itemRegion, selectedColor);
    }
    else if (item.m_index == m_activatedItemIndex)
    {
        dc.FillSolidRect(itemRegion, activatedColor);
    }
    else
    {
        dc.FillSolidRect(itemRegion, normalColor);
    }

    dc.Draw3dRect(stateRegion, RGB(255, 255, 255), RGB(0, 0, 0));
    dc.Draw3dRect(nameRegion, RGB(255, 255, 255), RGB(0, 0, 0));
    dc.Draw3dRect(jogRegion, RGB(255, 255, 255), RGB(0, 0, 0));

    if (IsJogEnable(item))
    {
        DrawJog(dc, jogRegion);
    }

    if (item.m_isValue)
    {
        dc.SetTextColor(RGB(255, 0, 0));
    }
    else
    {
        dc.SetTextColor(RGB(0, 0, 0));
    }

    dc.DrawText(item.m_state, stateRegion, DT_VCENTER | DT_CENTER | DT_SINGLELINE);
    dc.DrawText(_T("  ") + item.m_name, nameRegion, DT_VCENTER | DT_LEFT | DT_SINGLELINE);
}

BEGIN_MESSAGE_MAP(MaskListCtrl, CButton)
ON_WM_LBUTTONDOWN()
ON_WM_LBUTTONUP()
ON_WM_MOUSEMOVE()
ON_WM_MOUSEWHEEL()
ON_WM_RBUTTONDOWN()
ON_WM_VSCROLL()
ON_WM_LBUTTONDBLCLK()
END_MESSAGE_MAP()

// MaskListCtrl message handlers

void MaskListCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
    SetCapture();

    if (point.y >= m_rowHeight)
    {
        if (!(m_style & MaskListStyle::MultiSelectItems))
        {
            m_selectedItemIndexList.clear();
        }

        // Header는 클릭할 수 없게 하자
        for (long itemIndex = 0; itemIndex < long(m_items.size()); itemIndex++)
        {
            auto& item = m_items[itemIndex];

            if (item.m_nameRegion.PtInRect(point))
            {
                auto findSelected
                    = std::find(m_selectedItemIndexList.begin(), m_selectedItemIndexList.end(), itemIndex);

                bool enable = true;
                if (findSelected != m_selectedItemIndexList.end())
                {
                    m_selectedItemIndexList.erase(findSelected);
                    enable = false;
                }
                else
                {
                    m_selectedItemIndexList.push_back(itemIndex);
                }

                m_activatedItemIndex = -1;

                Invalidate(FALSE);

                if (m_eventSelected.first != NULL)
                {
                    ::SendMessage(m_eventSelected.first, m_eventSelected.second, itemIndex, enable);
                }
                break;
            }

            if (!IsJogEnable(item))
                continue;

            if (item.m_jogRegion.PtInRect(point))
            {
                m_jogDrag_itemIndex = itemIndex;
                m_jogDrag_MousePosition = point;

                Invalidate(FALSE);
                break;
            }
        }
    }

    CButton::OnLButtonDown(nFlags, point);
}

void MaskListCtrl::OnLButtonUp(UINT nFlags, CPoint point)
{
    ReleaseCapture();
    if (m_jogDrag_itemIndex >= 0)
    {
        if (m_eventMoved.first)
        {
            ::SendMessage(
                m_eventMoved.first, m_eventMoved.second, m_jogDrag_itemIndex, m_jogDrag_itemDestionationIndex);
        }
    }
    m_jogDrag_itemIndex = -1;
    m_jogDrag_itemDestionationIndex = -1;
    Invalidate(FALSE);

    CButton::OnLButtonUp(nFlags, point);
}

void MaskListCtrl::OnMouseMove(UINT nFlags, CPoint point)
{
    if (m_jogDrag_itemIndex >= 0)
    {
        m_jogDrag_MousePosition = point;

        Invalidate(FALSE);
    }

    CButton::OnMouseMove(nFlags, point);
}

BOOL MaskListCtrl::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
    int s_min = 0, s_max = 0;
    GetScrollRange(SB_VERT, &s_min, &s_max);
    int pos = min(s_max, max(s_min, GetScrollPos(SB_VERT) - zDelta));
    SetScrollPos(SB_VERT, pos);
    Invalidate(FALSE);

    return CButton::OnMouseWheel(nFlags, zDelta, pt);
}

void MaskListCtrl::OnRButtonDown(UINT nFlags, CPoint point)
{
    m_activatedItemIndex = -1;
    if (point.y >= m_rowHeight)
    {
        // Header는 클릭할 수 없게 하자
        for (long itemIndex = 0; itemIndex < long(m_items.size()); itemIndex++)
        {
            auto& item = m_items[itemIndex];

            if (item.m_region.PtInRect(point))
            {
                m_activatedItemIndex = itemIndex;
                break;
            }
        }
    }

    Invalidate(FALSE);

    if (m_eventRClick.first != NULL)
    {
        ::SendMessage(m_eventRClick.first, m_eventRClick.second, m_activatedItemIndex, 0);
    }
    m_activatedItemIndex = -1;
    Invalidate(FALSE);

    CButton::OnRButtonDown(nFlags, point);
}

void MaskListCtrl::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
    int step = 1;
    int Mstep = 5;
    int nNext, nmin, nmax;

    GetScrollRange(SB_VERT, &nmin, &nmax);

    switch (nSBCode)
    {
        case SB_THUMBPOSITION:
        case SB_THUMBTRACK:
            nNext = nPos;
            if (nNext > nmax)
                nNext = nmax;
            else if (nNext < nmin)
                nNext = nmin;
            SetScrollPos(SB_VERT, nNext);
            break;
        case SB_LINEDOWN:
            nNext = GetScrollPos(SB_VERT);
            if (nNext + step <= nmax)
            {
                SetScrollPos(SB_VERT, nNext + step);
                nNext += step;
            }
            break;
        case SB_LINEUP:
            nNext = GetScrollPos(SB_VERT);
            if (nNext - step >= nmin)
            {
                SetScrollPos(SB_VERT, nNext - step);
                nNext -= step;
            }
            break;
        case SB_PAGEDOWN:
            nNext = GetScrollPos(SB_VERT);
            if (nNext + Mstep <= nmax)
            {
                SetScrollPos(SB_VERT, nNext + Mstep);
                nNext += Mstep;
            }
            else
            {
                nNext = nmax;
                SetScrollPos(SB_VERT, nNext);
            }
            break;
        case SB_PAGEUP:
            nNext = GetScrollPos(SB_VERT);
            if (nNext - Mstep >= nmin)
            {
                SetScrollPos(SB_VERT, nNext - Mstep);
                nNext -= Mstep;
            }
            else
            {
                nNext = nmin;
                SetScrollPos(SB_VERT, nNext);
            }
            break;
        default:
            return;
            break;
    }

    Invalidate(FALSE);

    CButton::OnVScroll(nSBCode, nPos, pScrollBar);
}

void MaskListCtrl::OnLButtonDblClk(UINT nFlags, CPoint point)
{
    for (long itemIndex = 0; itemIndex < long(m_items.size()); itemIndex++)
    {
        auto& item = m_items[itemIndex];

        if (item.m_stateRegion.PtInRect(point))
        {
            m_activatedItemIndex = itemIndex;

            Invalidate(FALSE);

            if (m_eventStateDBClick.first != NULL)
            {
                ::SendMessage(m_eventStateDBClick.first, m_eventStateDBClick.second, itemIndex, 0);
            }
            break;
        }
    }

    CButton::OnLButtonDblClk(nFlags, point);
}

void MaskListCtrl::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
    CRect region;
    GetClientRect(region);
    long cx = region.Width();
    long cy = region.Height();

    CDC* dc = CDC::FromHandle(lpDrawItemStruct->hDC);

    CDC memDC;
    CBitmap memBitmap;
    memDC.CreateCompatibleDC(dc);
    memBitmap.CreateCompatibleBitmap(dc, cx, cy);
    auto* oldMemBitmap = memDC.SelectObject(&memBitmap);

    COLORREF emptyColor = RGB(160, 160, 160);
    if (!IsWindowEnabled())
    {
        emptyColor = RGB(80, 80, 80);
    }

    memDC.FillSolidRect(0, 0, cx, cy, emptyColor);
    memDC.SetBkMode(TRANSPARENT);

    // Column 0 : State
    // Column 1 : Name
    // Column 2 : UP, DOWN Jog

    long ltY = FirstItemStartY();
    m_jogDrag_itemDestionationIndex = JogRequiredDestinationIndex(cx);

    for (long itemIndex = 0; itemIndex < long(m_items.size()); itemIndex++)
    {
        auto& item = m_items[itemIndex];

        if (m_jogDrag_itemIndex != itemIndex && m_jogDrag_itemIndex > m_jogDrag_itemDestionationIndex
            && m_jogDrag_itemDestionationIndex == itemIndex)
        {
            ltY += m_rowHeight;
        }

        if (m_jogDrag_itemIndex != itemIndex)
        {
            DrawMaskItem(memDC, ltY, cx, m_rowHeight, item);
        }

        if (m_jogDrag_itemIndex != itemIndex && m_jogDrag_itemIndex < m_jogDrag_itemDestionationIndex
            && m_jogDrag_itemDestionationIndex == itemIndex)
        {
            ltY += m_rowHeight;
        }

        ltY += m_rowHeight;
    }

    // Header
    if (true)
    {
        CRect headRegion(0, 0, cx, m_rowHeight);
        if (IsWindowEnabled())
        {
            memDC.FillSolidRect(headRegion, RGB(200, 200, 255));
        }
        else
        {
            memDC.FillSolidRect(headRegion, RGB(100, 100, 100));
        }

        CRect stateRegion;
        CRect nameRegion;
        CRect jogRegion;

        CalcItemRegion(headRegion, stateRegion, nameRegion, jogRegion);

        memDC.Draw3dRect(stateRegion, RGB(255, 255, 255), RGB(0, 0, 0));
        memDC.Draw3dRect(nameRegion, RGB(255, 255, 255), RGB(0, 0, 0));
        memDC.Draw3dRect(jogRegion, RGB(255, 255, 255), RGB(0, 0, 0));

        memDC.DrawText(_T("S"), stateRegion, DT_VCENTER | DT_CENTER | DT_SINGLELINE);
        memDC.DrawText(_T("Name"), nameRegion, DT_VCENTER | DT_CENTER | DT_SINGLELINE);
    }

    if (m_jogDrag_itemIndex >= 0)
    {
        auto& item = m_items[m_jogDrag_itemIndex];
        DrawMaskItem(memDC, m_jogDrag_MousePosition.y, cx, m_rowHeight, item);
    }

    dc->BitBlt(0, 0, cx, cy, &memDC, 0, 0, SRCCOPY);

    memDC.SelectObject(oldMemBitmap);
}
