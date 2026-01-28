//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "ColorListBoxLogView.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
#if defined(IPVMLIB_MAJOR_VERSION) && IPVMLIB_MAJOR_VERSION >= 9
#include <Ipvm/Base/TimeCheck.h>
#else
#include "Gadget/TimeCheck.h"
#endif

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
enum
{
    UM_FIRST = WM_USER + 1,
    UM_UPDATE_LOG,
};

class OwnDrawItem
{
public:
    OwnDrawItem(LPCTSTR text, COLORREF background, COLORREF foreground)
        : m_text(text)
        , m_background(background)
        , m_foreground(foreground)
    {
    }

    CString m_text;
    COLORREF m_background;
    COLORREF m_foreground;
};

ColorListBoxLogView::ColorListBoxLogView(
    const bool useLocalTime, const bool newLogToHead, const bool autoScroll, const int32_t maxLineCount)
    : m_useLocalTime(useLocalTime)
    , m_newLogToHead(newLogToHead)
    , m_autoScroll(autoScroll)
    , m_maxLineCount(maxLineCount)
{
}

ColorListBoxLogView::~ColorListBoxLogView()
{
}

BEGIN_MESSAGE_MAP(ColorListBoxLogView, CListBox)
ON_MESSAGE(UM_UPDATE_LOG, &ColorListBoxLogView::OnUpdateLog)
END_MESSAGE_MAP()

void ColorListBoxLogView::SetUseLocalTime(const bool useLocalTime)
{
    m_useLocalTime = useLocalTime;
}

void ColorListBoxLogView::SetNewLogToHead(const bool newLogToHead)
{
    m_newLogToHead = newLogToHead;
}

void ColorListBoxLogView::SetAutoScroll(const bool autoScroll)
{
    m_autoScroll = autoScroll;
}

void ColorListBoxLogView::SetMaxLineCount(const int32_t maxLineCount)
{
    m_maxLineCount = max(1, maxLineCount);
}

void ColorListBoxLogView::SetLevelColor(const int32_t level, const COLORREF backgroundColor, const COLORREF textColor)
{
    m_levelColors[level] = std::make_pair(backgroundColor, textColor);
}

#if defined(IPVMLIB_MAJOR_VERSION) && IPVMLIB_MAJOR_VERSION >= 9
void ColorListBoxLogView::OnPush(
    const int32_t level, const SYSTEMTIME& utc, const size_t textLength, const wchar_t* text)
{
    SYSTEMTIME time;

    if (m_useLocalTime)
    {
        Ipvm::TimeCheck::ConvertUtcToLocalTime(utc, time);
    }
    else
    {
        time = utc;
    }

    CString strWrite;
    strWrite.Format(_T("[%04d-%02d-%02d %02d:%02d:%02d.%03d] "), time.wYear, time.wMonth, time.wDay, time.wHour,
        time.wMinute, time.wSecond, time.wMilliseconds);
    strWrite += CString(text, static_cast<int>(textLength));

    CSingleLock lock(&m_csLog, TRUE);

    m_pendingLog.emplace_back(level, strWrite);

    if (GetSafeHwnd())
    {
        PostMessage(UM_UPDATE_LOG);
    }
}
#else
void ColorListBoxLogView::Write(
    const int32_t level, const SYSTEMTIME& timeLocal, const SYSTEMTIME& timeSystem, const wchar_t* text)
{
    const auto& time = m_useLocalTime ? timeLocal : timeSystem;

    CString strWrite;
    strWrite.Format(_T("[%04d-%02d-%02d %02d:%02d:%02d.%03d] %s"), time.wYear, time.wMonth, time.wDay, time.wHour,
        time.wMinute, time.wSecond, time.wMilliseconds, text);

    CSingleLock lock(&m_csLog, TRUE);

    m_pendingLog.emplace_back(level, strWrite);

    if (GetSafeHwnd())
    {
        PostMessage(UM_UPDATE_LOG);
    }
}
#endif

void ColorListBoxLogView::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
    if (lpDrawItemStruct->itemData)
    {
        CDC dc;
        dc.Attach(lpDrawItemStruct->hDC);

        OwnDrawItem& ownDrawItem = *(OwnDrawItem*)lpDrawItemStruct->itemData;

        CBrush brush(ownDrawItem.m_background);
        dc.FillRect(&lpDrawItemStruct->rcItem, &brush);

        int oldBkMode = dc.SetBkMode(TRANSPARENT);

        CRect rect = lpDrawItemStruct->rcItem;

        UINT nFormat = DT_LEFT | DT_SINGLELINE | DT_VCENTER;

        COLORREF oldTextColor = dc.SetTextColor(ownDrawItem.m_foreground);

        dc.DrawText(ownDrawItem.m_text, -1, &rect, nFormat | DT_CALCRECT);
        dc.DrawText(ownDrawItem.m_text, -1, &rect, nFormat);

        dc.SetTextColor(oldTextColor);
        dc.SetBkMode(oldBkMode);

        dc.Detach();
    }
    else
    {
        __super::DrawItem(lpDrawItemStruct);
    }
}

void ColorListBoxLogView::MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
    __super::MeasureItem(lpMeasureItemStruct);
}

void ColorListBoxLogView::DeleteItem(LPDELETEITEMSTRUCT lpDeleteItemStruct)
{
    __super::DeleteItem(lpDeleteItemStruct);

    delete (OwnDrawItem*)(lpDeleteItemStruct->itemData);
}

LRESULT ColorListBoxLogView::OnUpdateLog(WPARAM wparam, LPARAM lparam)
{
    UNREFERENCED_PARAMETER(wparam);
    UNREFERENCED_PARAMETER(lparam);

    std::list<std::pair<long, CString>> pendingLog;
    {
        MSG msg;

        while (PeekMessage(&msg, GetSafeHwnd(), UM_UPDATE_LOG, UM_UPDATE_LOG, PM_REMOVE))
            ;

        CSingleLock lock(&m_csLog, TRUE);

        std::swap(pendingLog, m_pendingLog);
    }

    SetRedraw(FALSE);

    if (m_newLogToHead)
    {
        for (const auto& logPair : pendingLog)
        {
            OwnDrawItem* ownDrawItem = nullptr;

            auto itr = m_levelColors.find(logPair.first);

            if (itr != m_levelColors.end())
            {
                ownDrawItem = new OwnDrawItem(logPair.second, itr->second.first, itr->second.second);
            }

            SetItemData(InsertString(0, LPCTSTR(logPair.second)), DWORD_PTR(ownDrawItem));

            if (GetCount() > m_maxLineCount)
            {
                DeleteString(GetCount() - 1);
            }
        }

        if (m_autoScroll)
        {
            SetCurSel(0);
        }
    }
    else
    {
        for (const auto& logPair : pendingLog)
        {
            OwnDrawItem* ownDrawItem = nullptr;

            auto itr = m_levelColors.find(logPair.first);

            if (itr != m_levelColors.end())
            {
                ownDrawItem = new OwnDrawItem(logPair.second, itr->second.first, itr->second.second);
            }

            SetItemData(InsertString(GetCount(), LPCTSTR(logPair.second)), DWORD_PTR(ownDrawItem));

            if (GetCount() > m_maxLineCount)
            {
                DeleteString(0);
            }
        }

        if (m_autoScroll)
        {
            SetCurSel(GetCount() - 1);
        }
    }

    SetRedraw(TRUE);

    return 1L;
}
