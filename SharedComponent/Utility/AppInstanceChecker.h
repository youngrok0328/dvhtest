#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <functional>

//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
namespace Ipsc
{
class __INTEKPLUS_SHARED_UTILITY_API__ AppInstanceChecker
{
public:
    AppInstanceChecker() = delete;
    AppInstanceChecker(const AppInstanceChecker&) = delete;
    AppInstanceChecker(AppInstanceChecker&&) = delete;
    explicit AppInstanceChecker(LPCTSTR uniqueAppName);
    explicit AppInstanceChecker(LPCTSTR uniqueAppName, const std::function<bool(void)>& shouldAllowDuplicate);
    ~AppInstanceChecker();

    AppInstanceChecker& operator=(const AppInstanceChecker&) = delete;
    AppInstanceChecker& operator=(AppInstanceChecker&&) = delete;

    bool IsAllowed() const;

private:
    HANDLE m_mutexHandle = nullptr;
};
} // namespace Ipsc
