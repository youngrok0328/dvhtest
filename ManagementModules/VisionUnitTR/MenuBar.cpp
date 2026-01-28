//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "MenuBar.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define MENUBAR_VSCROLL (10)

//CPP_7_________________________________ Implementation body
//
IMPLEMENT_DYNAMIC(MenuBar, CButton)

MenuBar::MenuBar()
    : m_itemHeight(25)
    , m_activeItemID(-1)
    , m_selectionChanged_target(NULL)
    , m_selectionChanged_message(0)
{
}

MenuBar::~MenuBar()
{
}

void MenuBar::SetSelectionChangedEvent(HWND target, UINT message)
{
    m_selectionChanged_target = target;
    m_selectionChanged_message = message;
}

void MenuBar::SetFont(CFont* font)
{
    LOGFONT logFont;
    font->GetLogFont(&logFont);

    m_textFont.DeleteObject();
    m_textFont.CreateFontIndirect(&logFont);
}

void MenuBar::SetTitle(LPCTSTR name)
{
    m_title = name;
    DrawIntialize();
}

void MenuBar::DeleteAllItems()
{
    if (GetSafeHwnd() == NULL)
    {
        ASSERT(!_T("Window를 생성후 사용해야 합니다."));
        return;
    }
    m_menuIDtoIndex.clear();
    m_menus.clear();

    DrawIntialize();
}

bool MenuBar::IsExistItem(long id) const
{
    return m_menuIDtoIndex.find(id) != m_menuIDtoIndex.end();
}

bool MenuBar::AddItem(long id, LPCTSTR text, bool enabled)
{
    if (GetSafeHwnd() == NULL)
    {
        ASSERT(!_T("Window를 생성후 사용해야 합니다."));
        return false;
    }

    if (m_menuIDtoIndex.find(id) != m_menuIDtoIndex.end())
    {
        ASSERT(!_T("이미 존재하는 Menu"));
        return false;
    }
    SMenuInfo info;
    info.m_id = id;
    info.m_text = text;
    info.m_enabled = enabled;

    m_menus.push_back(info);
    m_menuIDtoIndex[id] = long(m_menus.size()) - 1;

    DrawIntialize();

    return true;
}

bool MenuBar::SetItemStateNum(long id, long stateNum)
{
    if (m_menuIDtoIndex.find(id) == m_menuIDtoIndex.end())
    {
        ASSERT(!_T("그런 ID 없음"));
        return false;
    }

    long menuIndex = m_menuIDtoIndex[id];
    m_menus[menuIndex].m_states.resize(max(0, stateNum), 0);

    Invalidate(FALSE);

    return true;
}

bool MenuBar::SetItemState(long id, long stateIndex, VisionProcessingState::enumState state)
{
    if (m_menuIDtoIndex.find(id) == m_menuIDtoIndex.end())
    {
        ASSERT(!_T("그런 ID 없음"));
        return false;
    }

    long menuIndex = m_menuIDtoIndex[id];

    if (stateIndex < 0 || stateIndex >= long(m_menus[menuIndex].m_states.size()))
    {
        ASSERT(!_T("뭘 Set 하고 싶은지?"));
        return false;
    }
    m_menus[menuIndex].m_states[stateIndex] = long(state);

    Invalidate(FALSE);

    return true;
}

bool MenuBar::SetActiveItem(long id)
{
    if (m_menuIDtoIndex.find(id) == m_menuIDtoIndex.end())
    {
        // 그런 ID 없음
        return false;
    }

    m_activeItemID = id;
    Invalidate(FALSE);

    return true;
}

long MenuBar::GetActiveItemID()
{
    return m_activeItemID;
}

void MenuBar::DrawIntialize()
{
    CRect clientRegion;
    GetClientRect(clientRegion);

    long drawItemCount = long(m_menus.size());
    if (!m_title.IsEmpty())
        drawItemCount++;

    long realDrawSizeY = drawItemCount * m_itemHeight;

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

    Invalidate(FALSE);
}

BEGIN_MESSAGE_MAP(MenuBar, CButton)
ON_WM_PAINT()
ON_WM_SIZE()
ON_WM_VSCROLL()
ON_WM_MOUSEWHEEL()
ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()

// MenuBar message handlers

void MenuBar::OnPaint()
{
    CPaintDC dc(this); // device context for painting
    CDC memDC;
    CBitmap memBitmap;

    CRect clientRegion;
    GetClientRect(clientRegion);

    long cx = clientRegion.Width();
    long cy = clientRegion.Height();
    memDC.CreateCompatibleDC(&dc);
    memBitmap.CreateCompatibleBitmap(&dc, cx, cy);

    auto* memOldBitmap = memDC.SelectObject(&memBitmap);
    auto* memOldBrush = memDC.GetCurrentBrush();
    auto* memOldFont = memDC.GetCurrentFont();

    if (m_textFont.GetSafeHandle())
    {
        memDC.SelectObject(&m_textFont);
    }

    memDC.FillSolidRect(clientRegion, RGB(160, 160, 160));

    long itemDrawY = -GetScrollPos(SB_VERT);

    if (!m_title.IsEmpty())
    {
        CRect titleRegion(0, itemDrawY, cx, itemDrawY + m_itemHeight);
        memDC.FillSolidRect(titleRegion, RGB(200, 200, 255));
        memDC.DrawText(m_title, titleRegion, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

        itemDrawY = titleRegion.bottom;
    }

    CPen linePen;
    linePen.CreatePen(PS_SOLID, 1, RGB(192, 192, 192));

    for (long itemIndex = 0; itemIndex < long(m_menus.size()); itemIndex++)
    {
        auto& item = m_menus[itemIndex];
        CRect& itemRegion = item.m_region;
        itemRegion = CRect(0, itemDrawY, cx, itemDrawY + m_itemHeight);

        COLORREF bkColor = item.m_enabled == true ? RGB(255, 255, 255) : RGB(165, 165, 165);

        if (item.m_id == m_activeItemID)
        {
            bkColor = RGB(213, 181, 158);
        }

        memDC.FillSolidRect(itemRegion, bkColor);
        memDC.SetTextColor(RGB(0, 0, 0));
        memDC.DrawText(_T(" ") + item.m_text, itemRegion, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

        memDC.SelectObject(&linePen);
        memDC.SelectStockObject(NULL_BRUSH);
        memDC.Rectangle(itemRegion);

        if (item.m_states.size())
        {
            long margin = 2;
            long boxSize = itemRegion.Height() - margin * 2;
            CRect state;
            state.left = itemRegion.right - boxSize - margin;
            state.top = itemRegion.top + margin;
            state.right = state.left + boxSize;
            state.bottom = state.top + boxSize;

            memDC.FillSolidRect(state, RGB(128, 128, 128));

            long stateSizeX = state.Width() / long(item.m_states.size());
            long stateL = state.left;

            for (long stateIndex = 0; stateIndex < long(item.m_states.size()); stateIndex++)
            {
                CRect stateOne = state;
                stateOne.left = stateL;
                if (stateIndex < long(item.m_states.size()) - 1)
                {
                    stateOne.right = stateOne.left + stateSizeX;
                }

                switch (VisionProcessingState::enumState(item.m_states[stateIndex]))
                {
                    case VisionProcessingState::enumState::Active_Invalid:
                        memDC.FillSolidRect(stateOne, RGB(255, 255, 0));
                        break;

                    case VisionProcessingState::enumState::Active_Success:
                        memDC.FillSolidRect(stateOne, RGB(67, 161, 249));
                        break;
                }

                stateL = stateOne.right;
            }
        }

        itemDrawY = itemRegion.bottom;
    }

    dc.BitBlt(0, 0, cx, cy, &memDC, 0, 0, SRCCOPY);

    memDC.SelectObject(memOldBrush);
    memDC.SelectObject(memOldFont);
    memDC.SelectObject(memOldBitmap);
}

void MenuBar::OnSize(UINT nType, int cx, int cy)
{
    CButton::OnSize(nType, cx, cy);

    DrawIntialize();
}

void MenuBar::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
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

BOOL MenuBar::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
    int s_min = 0, s_max = 0;
    GetScrollRange(SB_VERT, &s_min, &s_max);
    int pos = min(s_max, max(s_min, GetScrollPos(SB_VERT) - zDelta));
    SetScrollPos(SB_VERT, pos);
    Invalidate(FALSE);

    return CButton::OnMouseWheel(nFlags, zDelta, pt);
}

void MenuBar::OnLButtonDown(UINT nFlags, CPoint point)
{
    for (long menuIndex = 0; menuIndex < long(m_menus.size()); menuIndex++)
    {
        if (m_menus[menuIndex].m_region.PtInRect(point))
        {
            if (m_menus[menuIndex].m_enabled == false)
                continue;

            m_activeItemID = m_menus[menuIndex].m_id;

            if (m_selectionChanged_target)
            {
                ::SendMessage(m_selectionChanged_target, m_selectionChanged_message, m_activeItemID, 0);
            }

            Invalidate(FALSE);
            break;
        }
    }

    CButton::OnLButtonDown(nFlags, point);
}
