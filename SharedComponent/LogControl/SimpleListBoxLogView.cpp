//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "SimpleListBoxLogView.h"

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

SimpleListBoxLogView::SimpleListBoxLogView(
    const bool useLocalTime, const bool newLogToHead, const bool autoScroll, const int32_t maxLineCount)
    : m_useLocalTime(useLocalTime)
    , m_newLogToHead(newLogToHead)
    , m_autoScroll(autoScroll)
    , m_maxLineCount(maxLineCount)
{
}

SimpleListBoxLogView::~SimpleListBoxLogView()
{
}

BEGIN_MESSAGE_MAP(SimpleListBoxLogView, CListBox)
ON_MESSAGE(UM_UPDATE_LOG, &SimpleListBoxLogView::OnUpdateLog)
END_MESSAGE_MAP()

void SimpleListBoxLogView::SetUseLocalTime(const bool useLocalTime)
{
    m_useLocalTime = useLocalTime;
}

void SimpleListBoxLogView::SetNewLogToHead(const bool newLogToHead)
{
    m_newLogToHead = newLogToHead;
}

void SimpleListBoxLogView::SetAutoScroll(const bool autoScroll)
{
    m_autoScroll = autoScroll;
}

void SimpleListBoxLogView::SetMaxLineCount(const int32_t maxLineCount)
{
    m_maxLineCount = max(1, maxLineCount);
}

#if defined(IPVMLIB_MAJOR_VERSION) && IPVMLIB_MAJOR_VERSION >= 9
void SimpleListBoxLogView::OnPush(
    const int32_t level, const SYSTEMTIME& utc, const size_t textLength, const wchar_t* text)
{
    UNREFERENCED_PARAMETER(level);

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

    m_pendingLog.push_back(strWrite);

    if (GetSafeHwnd())
    {
        PostMessage(UM_UPDATE_LOG);
    }
}
#else
void SimpleListBoxLogView::Write(
    const int32_t level, const SYSTEMTIME& timeLocal, const SYSTEMTIME& timeSystem, const wchar_t* text)
{
    UNREFERENCED_PARAMETER(level);

    const auto& time = m_useLocalTime ? timeLocal : timeSystem;

    CString strWrite;
    strWrite.Format(_T("[%04d-%02d-%02d %02d:%02d:%02d.%03d] %s"), time.wYear, time.wMonth, time.wDay, time.wHour,
        time.wMinute, time.wSecond, time.wMilliseconds, text);

    CSingleLock lock(&m_csLog, TRUE);

    m_pendingLog.push_back(strWrite);

    if (GetSafeHwnd())
    {
        PostMessage(UM_UPDATE_LOG);
    }
}
#endif

LRESULT SimpleListBoxLogView::OnUpdateLog(WPARAM wparam, LPARAM lparam)
{
    UNREFERENCED_PARAMETER(wparam);
    UNREFERENCED_PARAMETER(lparam);

    std::list<CString> pendingLog;
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
        for (const auto& str : pendingLog)
        {
            InsertString(0, str);

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
        for (const auto& str : pendingLog)
        {
            InsertString(GetCount(), str);

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
