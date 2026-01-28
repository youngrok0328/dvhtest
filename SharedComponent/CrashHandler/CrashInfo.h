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
struct __INTEKPLUS_SHARED_CRASHHANDLER_API__ CrashInfo
{
    // Handler to Dumper
    BOOL m_showMessageDialog;
    wchar_t m_dumpFileDirectory[4096];
    wchar_t m_dumpFileTitle[256];
    int m_miniDumpType;

    DWORD m_targetProcessId;
    DWORD m_targetThreadId;
    void* m_exceptionPointers;

    // Dumper to Handler
    BOOL m_success;
    wchar_t m_dumpPathName[4096];
    wchar_t m_dumpMessage[4096];
};

#define KEY_mappingHandle L"{7AFF02D9-45CA-4EFA-A4EF-5D28176F1B89}"
