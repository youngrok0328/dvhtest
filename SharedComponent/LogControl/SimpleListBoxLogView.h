#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
#include <afxmt.h>
#include <ipvmbasedef.h>
#if defined(IPVMLIB_MAJOR_VERSION) && IPVMLIB_MAJOR_VERSION >= 9
#include <Ipvm/Base/LoggerObserver.h>
#else
#include "Base/LoggerInterface.h"
#endif

//HDR_4_________________________________ Standard library headers
#include <list>

//HDR_5_________________________________ Forward declarations
#if defined(IPVMSDK_MAJOR_VERSION) && IPVMSDK_MAJOR_VERSION < 9
namespace IntekPlus
{
namespace VisionMadang
{
}
} // namespace IntekPlus
namespace Ipvm = IntekPlus::VisionMadang;
#endif

//HDR_6_________________________________ Header body
//
#if defined(IPVMLIB_MAJOR_VERSION) && IPVMLIB_MAJOR_VERSION >= 9
class __INTEKPLUS_SHARED_LOGCONTROL_CLASS__ SimpleListBoxLogView
    : public CListBox
    , public Ipvm::LoggerObserver
#else
class __INTEKPLUS_SHARED_LOGCONTROL_CLASS__ SimpleListBoxLogView
    : public CListBox
    , public Ipvm::LoggerInterface
#endif
{
public:
    SimpleListBoxLogView() = delete;
    SimpleListBoxLogView(
        const bool useLocalTime, const bool newLogToHead, const bool autoScroll, const int32_t maxLineCount);
    virtual ~SimpleListBoxLogView();

public:
    void SetUseLocalTime(const bool useLocalTime);
    void SetNewLogToHead(const bool newLogToHead);
    void SetAutoScroll(const bool autoScroll);
    void SetMaxLineCount(const int32_t maxLineCount);

protected:
#if defined(IPVMLIB_MAJOR_VERSION) && IPVMLIB_MAJOR_VERSION >= 9
    void OnPush(const int32_t level, const SYSTEMTIME& utc, const size_t textLength, const wchar_t* text) override;
#else
    void Write(
        const int32_t level, const SYSTEMTIME& timeLocal, const SYSTEMTIME& timeSystem, const wchar_t* text) override;
#endif

    DECLARE_MESSAGE_MAP()
    afx_msg LRESULT OnUpdateLog(WPARAM wparam, LPARAM lparam);

private:
    bool m_useLocalTime;
    bool m_newLogToHead;
    bool m_autoScroll;
    long m_maxLineCount;

    CCriticalSection m_csLog;
    std::list<CString> m_pendingLog;
};
