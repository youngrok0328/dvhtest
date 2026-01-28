#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <memory>

//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
namespace Ipsc
{
/// @brief ThreadParamStorage
/// Thread에 전달할 인자를 안전하게 저장/조회하기 위한 유틸리티.
/// Push 로 등록 후, 스레드 엔트리에서 Pop<T> 으로 꺼내면 자동으로 관리된다.
struct __INTEKPLUS_SHARED_UTILITY_API__ ThreadParamStorage
{
    /// @brief 데이터를 저장하고, key(LPVOID)를 반환한다.
    static LPVOID Push(std::shared_ptr<void> data);

    /// @brief key에 해당하는 데이터를 지정한 타입으로 꺼낸다.
    /// @tparam T 구조체 타입
    template<class T>
    static std::shared_ptr<T> Pop(LPVOID key)
    {
        auto data = PopRaw(key);
        if (!data)
        {
            return nullptr;
        }

        return std::static_pointer_cast<T>(data);
    }

private:
    /// @brief key에 해당하는 데이터를 원시 shared_ptr<void>로 꺼낸다. (내부용)
    static std::shared_ptr<void> PopRaw(LPVOID key);
};
} // namespace Ipsc
