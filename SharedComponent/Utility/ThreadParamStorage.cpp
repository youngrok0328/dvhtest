//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "ThreadParamStorage.h"

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
class ThreadParamStorageImpl
{
public:
    LPVOID Push(std::shared_ptr<void> data)
    {
        // Key 생성: data의 주소
        auto key = data.get();

        // 데이터 추가
        SingleLock lock(&m_criticalSection);

        m_map[key] = data;

        return key;
    }

    std::shared_ptr<void> PopRaw(LPVOID key)
    {
        SingleLock lock(&m_criticalSection);

        auto it = m_map.find(key);
        if (it == m_map.end())
        {
            ASSERT(FALSE && "Key not found in ThreadParamStorage");
            return nullptr; // 데이터가 존재하지 않으면 nullptr 반환
        }
        // 데이터가 존재하면 반환하고 삭제
        auto data = it->second;
        m_map.erase(it);

        return data;
    }

    static ThreadParamStorageImpl& Instance()
    {
        static ThreadParamStorageImpl instance;
        return instance;
    }

private:
    ThreadParamStorageImpl()
    {
        ::InitializeCriticalSection(&m_criticalSection);
    }

    ~ThreadParamStorageImpl()
    {
        ASSERT(m_map.empty());

        ::DeleteCriticalSection(&m_criticalSection);
    }

    std::unordered_map<LPVOID, std::shared_ptr<void>> m_map;
    CRITICAL_SECTION m_criticalSection; // 멀티스레드 환경에서 안전하게 사용하기 위한 크리티컬 섹션
};

LPVOID ThreadParamStorage::Push(std::shared_ptr<void> data)
{
    return ThreadParamStorageImpl::Instance().Push(data);
}

std::shared_ptr<void> ThreadParamStorage::PopRaw(LPVOID key)
{
    return ThreadParamStorageImpl::Instance().PopRaw(key);
}
} // namespace Ipsc
