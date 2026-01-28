#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <memory>

//HDR_5_________________________________ Forward declarations
typedef UINT_PTR WPARAM;
typedef LONG_PTR LPARAM;

//HDR_6_________________________________ Header body
//
namespace Ipsc
{
/// @brief AsyncWindowMessage
/// @note 특정 윈도우에 PostMessage로 메모리를 전달해야 하는 상황에서 사용한다.
struct __INTEKPLUS_SHARED_UTILITY_API__ AsyncWindowMessage
{
    /// @brief 데이터를 저장하고, key(LPVOID)를 반환한다.
    /// @param hwnd 메시지를 받을 윈도우 핸들
    /// @param msg 메시지 ID
    /// @param wparam 메시지의 WPARAM 파라미터로 전달할 데이터
    /// @param lparam 메시지의 LPARAM 파라미터로 전달할 데이터
    /// @param file 호출한 소스 파일명 (디버그용)
    /// @param line 호출한 소스 코드 라인 번호 (디버그용)
    /// @remark 메시지 핸들러에서 반드시 PopW<T>() 또는 PopL<T>()를 호출하여 데이터를 꺼내야 한다.
    static BOOL Post(const HWND hwnd, const UINT msg, std::shared_ptr<void> wparam, std::shared_ptr<void> lparam,
        const wchar_t* file = nullptr, const int line = 0);

    /// @brief wparam에 해당하는 데이터를 지정한 타입으로 꺼낸다.
    /// @tparam T 구조체 타입
    /// @param hwnd 메시지를 받은 윈도우 핸들
    /// @param msg 받은 메시지 ID
    /// @param wparam 메시지의 WPARAM 파라미터
    template<class T>
    static std::shared_ptr<T> PopW(const HWND hwnd, const UINT msg, const WPARAM wparam)
    {
        return std::static_pointer_cast<T>(PopRawW(hwnd, msg, wparam));
    }

    /// @brief lparam에 해당하는 데이터를 지정한 타입으로 꺼낸다.
    /// @tparam T 구조체 타입
    /// @param hwnd 메시지를 받은 윈도우 핸들
    /// @param msg 받은 메시지 ID
    /// @param lparam 메시지의 LPARAM 파라미터
    template<class T>
    static std::shared_ptr<T> PopL(const HWND hwnd, const UINT msg, const LPARAM lparam)
    {
        return std::static_pointer_cast<T>(PopRawL(hwnd, msg, lparam));
    }

    /// @brief 현재 메시지 파라미터 저장소가 비었는지 확인한다.
    /// @note 프로그램 종료 전에 체크해 봐서 비어있지 않으면 문제가 있는 상황이다.
    static bool IsEmpty();

private:
    /// @brief key에 해당하는 데이터를 원시 shared_ptr<void>로 꺼낸다. (내부용)
    static std::shared_ptr<void> PopRawW(const HWND hwnd, const UINT msg, const WPARAM wparam);
    static std::shared_ptr<void> PopRawL(const HWND hwnd, const UINT msg, const LPARAM lparam);
};
} // namespace Ipsc
