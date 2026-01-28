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

/*

크래시 덤프를 사용하고 싶은 EXE 내에서 아래처럼 사용할 것
 
CrashHandler::Register(false, L"D:\\Temp\\", L"MyDump");

위 구문처럼 객체를 정의하면, 크래시 발생시에 아래처럼 덤프를 남긴다.

D:\Temp\MyDump_20220309_120302.dmp

*/

class __INTEKPLUS_SHARED_CRASHHANDLER_API__ CrashHandler
{
public:
    // Windows SDK 8.1 "DbgHelp.h" Compatible
    enum class MiniDumpType
    {
        e_miniDumpNormal = 0x00000000,
        e_miniDumpWithDataSegs = 0x00000001,
        e_miniDumpWithFullMemory = 0x00000002,
        e_miniDumpWithHandleData = 0x00000004,
        e_miniDumpFilterMemory = 0x00000008,
        e_miniDumpScanMemory = 0x00000010,
        e_miniDumpWithUnloadedModules = 0x00000020,
        e_miniDumpWithIndirectlyReferencedMemory = 0x00000040,
        e_miniDumpFilterModulePaths = 0x00000080,
        e_miniDumpWithProcessThreadData = 0x00000100,
        e_miniDumpWithPrivateReadWriteMemory = 0x00000200,
        e_miniDumpWithoutOptionalData = 0x00000400,
        e_miniDumpWithFullMemoryInfo = 0x00000800,
        e_miniDumpWithThreadInfo = 0x00001000,
        e_miniDumpWithCodeSegs = 0x00002000,
        e_miniDumpWithoutAuxiliaryState = 0x00004000,
        e_miniDumpWithFullAuxiliaryState = 0x00008000,
        e_miniDumpWithPrivateWriteCopyMemory = 0x00010000,
        e_miniDumpIgnoreInaccessibleMemory = 0x00020000,
        e_miniDumpWithTokenInformation = 0x00040000,
        e_miniDumpWithModuleHeaders = 0x00080000,
        e_miniDumpFilterTriage = 0x00100000,
        e_miniDumpValidTypeFlags = 0x001fffff,
    };

public:
    // 아래 함수를 호출하면, SharedComponent.CrashHandler.dll 이 있는 디렉토리에서 SharedComponent.CrashDumper.exe 를 실행한다.
    // SharedComponent.CrashDumper.exe 를 실행하는데 실패하거나, 초기화하는데 실패하면 현재 프로세스를 강제 종료하도록 구현함.

    static void Register(const bool showMessageDialog, const wchar_t* szDumpDirectory, const wchar_t* szDumpTitle,
        const MiniDumpType miniDumpType = MiniDumpType::e_miniDumpNormal);

    using CrashHandlerPreCallback = void(__cdecl*)(void* callbackData);
    using CrashHandlerPostCallback = void(__cdecl*)(
        void* callbackData, const bool success, const wchar_t* dumpPathName, const wchar_t* dumpMessage);

    static void Register(const bool showMessageDialog, const wchar_t* szDumpDirectory, const wchar_t* szDumpTitle,
        void* callbackData, CrashHandlerPreCallback callbackBeforeDump, CrashHandlerPostCallback callbackAfterDump,
        const MiniDumpType miniDumpType = MiniDumpType::e_miniDumpNormal);
};
