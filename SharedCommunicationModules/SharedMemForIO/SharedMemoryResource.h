#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
namespace Ipvm
{
struct SharedMemForIODEF_API SharedMemoryResource
{
    static void* New(const wchar_t* sharedMemoryName, const size_t bytes, const bool readOnly = false);
    static void* New(const char* sharedMemoryName, const size_t bytes, const bool readOnly = false);
    static void Delete(void* memoryAddress);
};
} // namespace Ipvm
