#pragma once

// 아직 완성본이 아니므로 사용하지 마세요...
// 리소스 편집기에서 LBS_OWNERDRAWFIXED 를 설정해야 동작합니다.

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
#include <unordered_map>

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
class __INTEKPLUS_SHARED_LOGCONTROL_CLASS__ ColorListBoxLogView
    : public CListBox
    , public Ipvm::LoggerObserver
#else
class __INTEKPLUS_SHARED_LOGCONTROL_CLASS__ ColorListBoxLogView
    : public CListBox
    , public Ipvm::LoggerInterface
#endif
{
public:
    ColorListBoxLogView() = delete;
    ColorListBoxLogView(
        const bool useLocalTime, const bool newLogToHead, const bool autoScroll, const int32_t maxLineCount);
    virtual ~ColorListBoxLogView();

public:
    void SetUseLocalTime(const bool useLocalTime);
    void SetNewLogToHead(const bool newLogToHead);
    void SetAutoScroll(const bool autoScroll);
    void SetMaxLineCount(const int32_t maxLineCount);

    void SetLevelColor(const int32_t level, const COLORREF backgroundColor, const COLORREF textColor);

protected:
#if defined(IPVMLIB_MAJOR_VERSION) && IPVMLIB_MAJOR_VERSION >= 9
    void OnPush(const int32_t level, const SYSTEMTIME& utc, const size_t textLength, const wchar_t* text) override;
#else
    void Write(
        const int32_t level, const SYSTEMTIME& timeLocal, const SYSTEMTIME& timeSystem, const wchar_t* text) override;
#endif

    DECLARE_MESSAGE_MAP()
    afx_msg LRESULT OnUpdateLog(WPARAM wparam, LPARAM lparam);

    virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
    virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
    virtual void DeleteItem(LPDELETEITEMSTRUCT lpDeleteItemStruct);

private:
    bool m_useLocalTime;
    bool m_newLogToHead;
    bool m_autoScroll;
    long m_maxLineCount;

    CCriticalSection m_csLog;
    std::list<std::pair<long, CString>> m_pendingLog;

    std::unordered_map<long, std::pair<COLORREF, COLORREF>> m_levelColors;
};
