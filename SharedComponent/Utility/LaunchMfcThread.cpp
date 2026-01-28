//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "LaunchMfcThread.h"

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
HANDLE LaunchMfcThread(UINT(__cdecl* proc)(void*), void* param, int priority)
{
    auto winThread = ::AfxBeginThread(proc, param, priority, 0, CREATE_SUSPENDED);

    if (winThread == nullptr)
    {
        return NULL;
    }

    HANDLE handle{};

    if (::DuplicateHandle(::GetCurrentProcess(), winThread->m_hThread, ::GetCurrentProcess(), &handle, 0, FALSE,
            DUPLICATE_SAME_ACCESS)
        == FALSE)
    {
        winThread->Delete();

        return NULL;
    }

    if (winThread->ResumeThread() == 0xFFFFFFFF)
    {
        winThread->Delete();

        ::CloseHandle(handle);

        return NULL;
    }

    return handle;
}
} // namespace Ipsc
