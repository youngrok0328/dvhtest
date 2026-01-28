//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "CrashHandler.h"

//CPP_2_________________________________ This project's headers
#include "CrashInfo.h"

//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
#include <Shlwapi.h>

//CPP_5_________________________________ Standard library headers
#include <cstdio>
#include <new.h>

//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
#pragma comment(lib, "Shlwapi.lib")

class CriticalSection
{
public:
    CriticalSection()
    {
        ::InitializeCriticalSection(&m_cs);
    }

    ~CriticalSection()
    {
        ::DeleteCriticalSection(&m_cs);
    }

private:
    CRITICAL_SECTION m_cs;

    friend class SingleLock;
};

static CriticalSection g_cs;

class SingleLock
{
public:
    SingleLock(CriticalSection& cs)
        : m_cs(cs)
    {
        ::EnterCriticalSection(&m_cs.m_cs);
    }

    ~SingleLock()
    {
        ::LeaveCriticalSection(&m_cs.m_cs);
    }

private:
    CriticalSection& m_cs;
};

template<size_t size>
void GetDumperAppDirectory(wchar_t (&writerAppDirectory)[size])
{
    ::memset(writerAppDirectory, 0, sizeof(writerAppDirectory));

    ::GetModuleFileNameW(NULL, writerAppDirectory, size);

    ::PathRemoveFileSpecW(writerAppDirectory);
}

class CrashHandlerData
{
public:
    CrashHandlerData()
        : m_mappingHandle(NULL)
        , m_crashInfo(nullptr)
        , m_callbackData(nullptr)
        , m_callbackBeforeDump(nullptr)
        , m_callbackAfterDump(nullptr)
    {
        const wchar_t dumperName[] = L"SharedComponent.CrashDumper.exe\0";

        GetDumperAppDirectory(m_dumperAppDirectory);

        ::swprintf_s(m_dumperAppPathName, L"%ls\\%ls", m_dumperAppDirectory, dumperName);

        if (::PathFileExistsW(m_dumperAppPathName) == FALSE)
        {
            ::MessageBoxW(
                NULL, L"Cannot find \"SharedComponent.CrashDumper.exe\".", L"Error", MB_ICONERROR | MB_SYSTEMMODAL);
            ::TerminateProcess(::GetCurrentProcess(), STATUS_INVALID_PARAMETER);
        }

        const auto processId = ::GetProcessId(::GetCurrentProcess());

        ::swprintf_s(m_dumperCommandLine, L"%ls %d", m_dumperAppPathName, processId);

        wchar_t mappingHandleKey[512];

        ::swprintf_s(mappingHandleKey, L"%ls_%d", KEY_mappingHandle, processId);

        if (m_mappingHandle
            = ::CreateFileMappingW(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(CrashInfo), mappingHandleKey);
            m_mappingHandle)
        {
            if (m_crashInfo = ((CrashInfo*)::MapViewOfFile(
                    m_mappingHandle, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, sizeof(CrashInfo)));
                m_crashInfo)
            {
                ::memset(m_crashInfo, 0, sizeof(CrashInfo));
            }
            else
            {
                ::MessageBoxW(NULL, L"MapViewOfFile() failed!", L"Error", MB_ICONERROR | MB_SYSTEMMODAL);
                ::TerminateProcess(::GetCurrentProcess(), STATUS_INVALID_PARAMETER);
            }
        }
        else
        {
            ::MessageBoxW(NULL, L"CreateFileMapping() failed!", L"Error", MB_ICONERROR | MB_SYSTEMMODAL);
            ::TerminateProcess(::GetCurrentProcess(), STATUS_INVALID_PARAMETER);
        }
    }

    static CrashHandlerData& Get()
    {
        static CrashHandlerData crashHandlerData;

        return crashHandlerData;
    }

    ~CrashHandlerData()
    {
        if (m_crashInfo)
        {
            ::UnmapViewOfFile(m_crashInfo);
            m_crashInfo = NULL;
        }

        if (m_mappingHandle)
        {
            ::CloseHandle(m_mappingHandle);
            m_mappingHandle = NULL;
        }
    }

public:
    HANDLE m_mappingHandle;
    CrashInfo* m_crashInfo;

    wchar_t m_dumperAppDirectory[4096];
    wchar_t m_dumperAppPathName[4096];
    wchar_t m_dumperCommandLine[5120];

    void* m_callbackData;
    CrashHandler::CrashHandlerPreCallback m_callbackBeforeDump;
    CrashHandler::CrashHandlerPostCallback m_callbackAfterDump;
};

LONG __stdcall CustomExceptionHandler(_EXCEPTION_POINTERS* pExcInf)
{
    // 여러 쓰레드에서 예외가 발생하는 경우 문제가 될 수 있으므로 락을 건다.
    SingleLock lock(g_cs);

    auto& crashHandlerData = CrashHandlerData::Get();

    if (crashHandlerData.m_callbackBeforeDump)
    {
        crashHandlerData.m_callbackBeforeDump(crashHandlerData.m_callbackData);
    }

    crashHandlerData.m_crashInfo->m_targetProcessId = ::GetCurrentProcessId();
    crashHandlerData.m_crashInfo->m_targetThreadId = ::GetCurrentThreadId();
    crashHandlerData.m_crashInfo->m_exceptionPointers = pExcInf;

    STARTUPINFOW startupInfo = {0};
    startupInfo.cb = sizeof(STARTUPINFOW);

    PROCESS_INFORMATION processInformation;

    if (::CreateProcessW(crashHandlerData.m_dumperAppPathName, crashHandlerData.m_dumperCommandLine, NULL, NULL, FALSE,
            NORMAL_PRIORITY_CLASS, NULL, NULL, &startupInfo, &processInformation))
    {
        ::WaitForSingleObject(processInformation.hProcess, INFINITE);

        ::CloseHandle(processInformation.hThread);
        ::CloseHandle(processInformation.hProcess);

        if (crashHandlerData.m_callbackAfterDump)
        {
            crashHandlerData.m_callbackAfterDump(crashHandlerData.m_callbackData,
                crashHandlerData.m_crashInfo->m_success, crashHandlerData.m_crashInfo->m_dumpPathName,
                crashHandlerData.m_crashInfo->m_dumpMessage);
        }

        ::TerminateProcess(::GetCurrentProcess(), STATUS_INVALID_PARAMETER);

        return EXCEPTION_EXECUTE_HANDLER;
    }
    else
    {
        ::MessageBoxW(
            NULL, L"Cannot launch \"SharedComponent.CrashDumper.exe\".", L"Error", MB_ICONERROR | MB_SYSTEMMODAL);

        return EXCEPTION_CONTINUE_SEARCH;
    }
}

void PureCallHandler()
{
    ::RaiseException(EXCEPTION_ACCESS_VIOLATION, 0, 0, NULL);
}

void InvalidParameterHandler(
    const wchar_t* expression, const wchar_t* function, const wchar_t* file, unsigned int line, uintptr_t pReserved)
{
    UNREFERENCED_PARAMETER(expression);
    UNREFERENCED_PARAMETER(function);
    UNREFERENCED_PARAMETER(file);
    UNREFERENCED_PARAMETER(line);
    UNREFERENCED_PARAMETER(pReserved);

    ::RaiseException(EXCEPTION_ACCESS_VIOLATION, 0, 0, NULL);
}

int CustomNewHandler(size_t size)
{
    UNREFERENCED_PARAMETER(size);

    ::RaiseException(EXCEPTION_ACCESS_VIOLATION, 0, 0, NULL);

    return 0;
}

LONG __stdcall VectoredExceptionHandler(_EXCEPTION_POINTERS* pExcInf)
{
    if ((pExcInf->ExceptionRecord->ExceptionCode & 0xF0000000L) == 0xC0000000L)
    {
        return CustomExceptionHandler(pExcInf);
    }
    else
    {
        return EXCEPTION_CONTINUE_SEARCH;
    }
}

void CrashHandler::Register(const bool showMessageDialog, const wchar_t* szDumpDirectory, const wchar_t* szDumpTitle,
    const MiniDumpType miniDumpType)
{
    Register(showMessageDialog, szDumpDirectory, szDumpTitle, nullptr, nullptr, nullptr, miniDumpType);
}

void CrashHandler::Register(const bool showMessageDialog, const wchar_t* szDumpDirectory, const wchar_t* szDumpTitle,
    void* callbackData, CrashHandlerPreCallback callbackBeforeDump, CrashHandlerPostCallback callbackAfterDump,
    const MiniDumpType miniDumpType)
{
    if (::PathFileExistsW(szDumpDirectory) == FALSE)
    {
        ::MessageBoxW(NULL, L"Cannot find the crash dump directory.", L"Error", MB_ICONERROR | MB_SYSTEMMODAL);
        ::TerminateProcess(::GetCurrentProcess(), STATUS_INVALID_PARAMETER);
    }

    // 여러 쓰레드에서 예외가 발생하는 경우 문제가 될 수 있으므로 락을 건다.
    SingleLock lock(g_cs);

    auto& crashHandlerData = CrashHandlerData::Get();

    crashHandlerData.m_crashInfo->m_showMessageDialog = showMessageDialog ? TRUE : FALSE;
    ::swprintf_s(crashHandlerData.m_crashInfo->m_dumpFileDirectory, L"%ls", szDumpDirectory);
    ::swprintf_s(crashHandlerData.m_crashInfo->m_dumpFileTitle, L"%ls", szDumpTitle);
    crashHandlerData.m_crashInfo->m_miniDumpType = static_cast<int>(miniDumpType);

    crashHandlerData.m_callbackData = callbackData;
    crashHandlerData.m_callbackBeforeDump = callbackBeforeDump;
    crashHandlerData.m_callbackAfterDump = callbackAfterDump;

    // For SEH
    ::AddVectoredExceptionHandler(1, VectoredExceptionHandler);
    ::SetUnhandledExceptionFilter(CustomExceptionHandler);

    // For C-Runtime Library
    ::_set_purecall_handler(PureCallHandler);
    ::_set_invalid_parameter_handler(InvalidParameterHandler);
    ::_set_new_handler(CustomNewHandler);
}
