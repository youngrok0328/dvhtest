#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
typedef struct _RTL_CRITICAL_SECTION RTL_CRITICAL_SECTION;
typedef RTL_CRITICAL_SECTION CRITICAL_SECTION;

//HDR_6_________________________________ Header body
//
namespace Ipsc
{
class __INTEKPLUS_SHARED_UTILITY_API__ SingleLock
{
public:
    SingleLock() = delete;
    SingleLock(const SingleLock& rhs) = delete;
    explicit SingleLock(CRITICAL_SECTION* cs);
    ~SingleLock();

    SingleLock& operator=(const SingleLock& rhs) = delete;

private:
    CRITICAL_SECTION* m_cs;
};
} // namespace Ipsc
