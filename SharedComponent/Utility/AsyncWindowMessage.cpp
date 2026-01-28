//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "AsyncWindowMessage.h"

//CPP_2_________________________________ This project's headers
#include "SingleLock.h"

//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
#include <unordered_map>

//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
namespace Ipsc
{
class AsyncWindowMessageImpl
{
public:
    AsyncWindowMessageImpl()
    {
        ::InitializeCriticalSection(&m_wparamLock);
        ::InitializeCriticalSection(&m_lparamLock);
    }

    ~AsyncWindowMessageImpl()
    {
        ASSERT(m_wparamStorage.empty());
        ASSERT(m_lparamStorage.empty());

        ::DeleteCriticalSection(&m_wparamLock);
        ::DeleteCriticalSection(&m_lparamLock);
    }

    BOOL Post(const HWND hwnd, const UINT msg, std::shared_ptr<void> wparam, std::shared_ptr<void> lparam,
        const wchar_t* file, const int line)
    {
        // 데이터 추가
        const WPARAM wparamKey = reinterpret_cast<WPARAM>(wparam.get());

        if (wparamKey)
        {
            SingleLock lock(&m_wparamLock);

            m_wparamStorage[wparamKey] = {hwnd, msg, wparam, file, line};
        }

        const LPARAM lparamKey = reinterpret_cast<LPARAM>(lparam.get());

        if (lparamKey)
        {
            SingleLock lock(&m_lparamLock);

            m_lparamStorage[lparamKey] = {hwnd, msg, lparam, file, line};
        }

        return ::PostMessage(hwnd, msg, wparamKey, lparamKey);
    }

    std::shared_ptr<void> PopRawW(HWND hwnd, UINT msg, WPARAM key)
    {
        SingleLock lock(&m_wparamLock);

        auto it = m_wparamStorage.find(key);

        if (it == m_wparamStorage.end())
        {
            ASSERT(FALSE && "Key not found in AsyncWindowMessage");
            return nullptr; // 데이터가 존재하지 않으면 nullptr 반환
        }

        // 데이터가 존재하면 반환하고 삭제
        auto data = it->second;

        m_wparamStorage.erase(it);

        if (data.m_hwnd == hwnd && data.m_msg == msg)
        {
            return data.m_pointer;
        }
        else
        {
            ASSERT(FALSE && "HWND or MSG does not match in AsyncWindowMessage");
            return nullptr;
        }
    }

    std::shared_ptr<void> PopRawL(HWND hwnd, UINT msg, LPARAM key)
    {
        SingleLock lock(&m_lparamLock);

        auto it = m_lparamStorage.find(key);

        if (it == m_lparamStorage.end())
        {
            ASSERT(FALSE && "Key not found in AsyncWindowMessage");
            return nullptr; // 데이터가 존재하지 않으면 nullptr 반환
        }

        // 데이터가 존재하면 반환하고 삭제
        auto data = it->second;

        m_lparamStorage.erase(it);

        if (data.m_hwnd == hwnd && data.m_msg == msg)
        {
            return data.m_pointer;
        }
        else
        {
            ASSERT(FALSE && "HWND or MSG does not match in AsyncWindowMessage");
            return nullptr;
        }
    }

    bool IsEmpty()
    {
        return m_wparamStorage.empty() && m_lparamStorage.empty();
    }

private:
    struct ValueHolder
    {
        HWND m_hwnd;
        UINT m_msg;
        std::shared_ptr<void> m_pointer;
        CStringW m_file;
        int m_line;
    };

    std::unordered_map<WPARAM, ValueHolder> m_wparamStorage;
    std::unordered_map<LPARAM, ValueHolder> m_lparamStorage;

    CRITICAL_SECTION m_wparamLock; // 멀티스레드 환경에서 안전하게 사용하기 위한 크리티컬 섹션
    CRITICAL_SECTION m_lparamLock; // 멀티스레드 환경에서 안전하게 사용하기 위한 크리티컬 섹션
};

static AsyncWindowMessageImpl* s_asyncWindowMessageImpl{};

void InitializeAsyncWindowMessage()
{
    ASSERT(s_asyncWindowMessageImpl == nullptr);
    s_asyncWindowMessageImpl = new AsyncWindowMessageImpl;
}

void CleanupAsyncWindowMessage()
{
    ASSERT(s_asyncWindowMessageImpl);
    delete s_asyncWindowMessageImpl;
    s_asyncWindowMessageImpl = nullptr;
}

BOOL AsyncWindowMessage::Post(const HWND hwnd, const UINT msg, std::shared_ptr<void> wparam,
    std::shared_ptr<void> lparam, const wchar_t* file, const int line)
{
    ASSERT(s_asyncWindowMessageImpl);
    return s_asyncWindowMessageImpl->Post(hwnd, msg, wparam, lparam, file, line);
}

std::shared_ptr<void> AsyncWindowMessage::PopRawW(const HWND hwnd, const UINT msg, const WPARAM wparam)
{
    ASSERT(s_asyncWindowMessageImpl);
    return s_asyncWindowMessageImpl->PopRawW(hwnd, msg, wparam);
}

std::shared_ptr<void> AsyncWindowMessage::PopRawL(const HWND hwnd, const UINT msg, const LPARAM lparam)
{
    ASSERT(s_asyncWindowMessageImpl);
    return s_asyncWindowMessageImpl->PopRawL(hwnd, msg, lparam);
}

bool AsyncWindowMessage::IsEmpty()
{
    ASSERT(s_asyncWindowMessageImpl);
    return s_asyncWindowMessageImpl->IsEmpty();
}
} // namespace Ipsc
