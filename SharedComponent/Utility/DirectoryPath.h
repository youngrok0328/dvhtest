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
namespace Ipsc
{
struct __INTEKPLUS_SHARED_UTILITY_API__ DirectoryPath
{
    // clang-format off
    static const CString&   Root();
    static const CString&       Bin();
    static const CString&       Config();
    static const CString&       System();
    // clang-format on
};
} // namespace Ipsc

struct __INTEKPLUS_SHARED_UTILITY_API__ DirectoryPath
{
    static CString GetBinDirectory();
};
