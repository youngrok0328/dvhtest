#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
typedef unsigned int UINT;
typedef void* HANDLE;

//HDR_6_________________________________ Header body
//
namespace Ipsc
{
// 이 함수에서 반환하는 핸들은 사용 후 반드시 CloseHandle 로 닫아주어야 한다.
__INTEKPLUS_SHARED_UTILITY_API__ [[nodiscard]] HANDLE LaunchMfcThread(
    UINT(__cdecl* proc)(void*), void* param, int priority = 0);
} // namespace Ipsc
