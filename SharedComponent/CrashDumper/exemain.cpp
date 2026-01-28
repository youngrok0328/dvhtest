//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
#include "../CrashHandler/CrashInfo.h"

//CPP_4_________________________________ External library headers
#include <dbghelp.h>
#include <shellapi.h>
#include <stdio.h>
#include <tlhelp32.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
#pragma comment(lib, "dbghelp")

typedef WINBASEAPI DWORD WINAPI ThreadControlFunction(_In_ HANDLE hThread);

void ControlAllThreads(DWORD targetProcessId, ThreadControlFunction* threadControlFunction)
{
    HANDLE snapshotHandle = ::CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);

    if (snapshotHandle == INVALID_HANDLE_VALUE)
    {
        return;
    }

    THREADENTRY32 te;
    te.dwSize = sizeof(te);

    if (::Thread32First(snapshotHandle, &te))
    {
        do
        {
            if (te.dwSize >= FIELD_OFFSET(THREADENTRY32, th32OwnerProcessID) + sizeof(te.th32OwnerProcessID))
            {
                if (te.th32OwnerProcessID == targetProcessId)
                {
                    HANDLE threadHandle = ::OpenThread(THREAD_ALL_ACCESS, FALSE, te.th32ThreadID);

                    if (threadHandle != NULL)
                    {
                        threadControlFunction(threadHandle);

                        ::CloseHandle(threadHandle);
                    }
                }
            }

            te.dwSize = sizeof(te);
        }
        while (::Thread32Next(snapshotHandle, &te));
    }

    ::CloseHandle(snapshotHandle);
}

void Process(CrashInfo* crashInfo)
{
    HANDLE processHandle = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, crashInfo->m_targetProcessId);

    if (processHandle != NULL)
    {
        ControlAllThreads(crashInfo->m_targetProcessId, ::SuspendThread);

        HANDLE threadHandle = ::OpenThread(THREAD_ALL_ACCESS, FALSE, crashInfo->m_targetThreadId);

        if (threadHandle != NULL)
        {
            MINIDUMP_EXCEPTION_INFORMATION minidumpExceptionInformation = {0};
            minidumpExceptionInformation.ThreadId = crashInfo->m_targetThreadId;
            minidumpExceptionInformation.ExceptionPointers = (EXCEPTION_POINTERS*)crashInfo->m_exceptionPointers;
            minidumpExceptionInformation.ClientPointers = TRUE;

            SYSTEMTIME systemTime;
            ::GetLocalTime(&systemTime);

            ::swprintf_s(crashInfo->m_dumpPathName, L"%ls\\%ls_%04d%02d%02d_%02d%02d%02d.dmp",
                crashInfo->m_dumpFileDirectory, crashInfo->m_dumpFileTitle, systemTime.wYear, systemTime.wMonth,
                systemTime.wDay, systemTime.wHour, systemTime.wMinute, systemTime.wSecond);

            HANDLE fileHandle = ::CreateFileW(crashInfo->m_dumpPathName, GENERIC_WRITE, FILE_SHARE_WRITE, NULL,
                CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

            if (fileHandle != INVALID_HANDLE_VALUE)
            {
                if (::MiniDumpWriteDump(processHandle, crashInfo->m_targetProcessId, fileHandle,
                        MINIDUMP_TYPE(crashInfo->m_miniDumpType), &minidumpExceptionInformation, NULL, NULL))
                {
                    ::swprintf_s(crashInfo->m_dumpMessage, L"A crash dump file was saved!\r\n\r\n\"%ls\"",
                        crashInfo->m_dumpPathName);

                    crashInfo->m_success = TRUE;
                }
                else
                {
                    ::swprintf_s(crashInfo->m_dumpMessage, L"MiniDumpWriteDump() failed.");
                }

                ::CloseHandle(fileHandle);
            }
            else
            {
                ::swprintf_s(crashInfo->m_dumpMessage, L"CreateFile() failed.");
            }

            ::CloseHandle(threadHandle);
        }
        else
        {
            ::swprintf_s(crashInfo->m_dumpMessage, L"OpenThread() failed.");
        }

        ::CloseHandle(processHandle);

        ControlAllThreads(crashInfo->m_targetProcessId, ::ResumeThread);
    }
    else
    {
        ::swprintf_s(crashInfo->m_dumpMessage, L"OpenProcess() failed.");
    }
}

int APIENTRY wWinMain(
    _In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hInstance);
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(nCmdShow);

    int nArgs = 0;
    auto szArglist = ::CommandLineToArgvW(lpCmdLine, &nArgs);

    if (nArgs != 1)
    {
        ::LocalFree(szArglist);

        return 0;
    }

    const auto processId = ::_wtoi(szArglist[0]);

    wchar_t mappingHandleKey[512];

    ::swprintf_s(mappingHandleKey, L"%ls_%d", KEY_mappingHandle, processId);

    if (HANDLE mappingHandle = ::OpenFileMappingW(FILE_MAP_READ | FILE_MAP_WRITE, FALSE, mappingHandleKey))
    {
        if (CrashInfo* crashInfo
            = (CrashInfo*)::MapViewOfFile(mappingHandle, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, sizeof(CrashInfo)))
        {
            if (crashInfo->m_targetProcessId && crashInfo->m_targetThreadId && crashInfo->m_exceptionPointers)
            {
                Process(crashInfo);
            }
            else
            {
                ::swprintf_s(crashInfo->m_dumpMessage, L"Process ID, thread ID or exception pointers are invalid!");
            }

            if (crashInfo->m_showMessageDialog)
            {
                ::MessageBoxW(
                    NULL, crashInfo->m_dumpMessage, L"IntekPlus.Shared.CrashDumper.exe", MB_ICONERROR | MB_SYSTEMMODAL);
            }

            ::UnmapViewOfFile(crashInfo);
        }
        else
        {
            ::MessageBoxW(
                NULL, L"MapViewOfFile() failed!", L"IntekPlus.Shared.CrashDumper.exe", MB_ICONERROR | MB_SYSTEMMODAL);
        }

        ::CloseHandle(mappingHandle);
    }
    else
    {
        ::MessageBoxW(
            NULL, L"OpenFileMapping() failed!", L"IntekPlus.Shared.CrashDumper.exe", MB_ICONERROR | MB_SYSTEMMODAL);
    }

    ::LocalFree(szArglist);

    return 0;
}
