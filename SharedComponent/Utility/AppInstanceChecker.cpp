//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "AppInstanceChecker.h"

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
namespace Ipsc
{
AppInstanceChecker::AppInstanceChecker(LPCTSTR uniqueAppName)
    : AppInstanceChecker(uniqueAppName,
          []() -> bool
          {
              return false;
          })
{
}

AppInstanceChecker::AppInstanceChecker(LPCTSTR uniqueAppName, const std::function<bool(void)>& shouldAllowDuplicate)
{
    // Unique 이름을 사용하여 전역 뮤텍스 생성
    auto mutexHandle = ::CreateMutex(nullptr, FALSE, CString("Global\\") + uniqueAppName);

    if (mutexHandle == nullptr)
    {
        // Mutex 생성 실패. 이미 실행 중인 것으로 간주
        return;
    }

    auto lastError = ::GetLastError();

    if (lastError == ERROR_ALREADY_EXISTS || lastError == ERROR_ACCESS_DENIED)
    {
        // 중복 허용 여부 결정
        if (!shouldAllowDuplicate())
        {
            // 중복 허용하지 않음
            CloseHandle(mutexHandle);
            return;
        }
    }

    m_mutexHandle = mutexHandle;
}

AppInstanceChecker::~AppInstanceChecker()
{
    if (m_mutexHandle != nullptr)
    {
        CloseHandle(m_mutexHandle);
    }
}

bool AppInstanceChecker::IsAllowed() const
{
    return (m_mutexHandle != nullptr);
}
} // namespace Ipsc
