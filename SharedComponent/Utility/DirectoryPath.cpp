//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "DirectoryPath.h"

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
static CString s_rootDirectory;
static CString s_binDirectory;
static CString s_configDirectory;
static CString s_systemDirectory;

void InitializeDirectoryPaths()
{
    // 0. Bin directory
    constexpr int maxPathLength = MAX_PATH * 20;
    s_binDirectory.GetBuffer(maxPathLength);

    ::GetModuleFileName(NULL, s_binDirectory.GetBuffer(), maxPathLength);
    ::PathRemoveFileSpec(s_binDirectory.GetBuffer());
    s_binDirectory.ReleaseBuffer();

    // 1. Root directory
    s_rootDirectory = s_binDirectory;
    ::PathRemoveFileSpec(s_rootDirectory.GetBuffer());
    s_rootDirectory.ReleaseBuffer();

    // 2. others
    s_configDirectory = s_rootDirectory + _T("\\Config");
    s_systemDirectory = s_rootDirectory + _T("\\System");
}

const CString& DirectoryPath::Root()
{
    return s_rootDirectory;
}

const CString& DirectoryPath::Bin()
{
    return s_binDirectory;
}

const CString& DirectoryPath::Config()
{
    return s_configDirectory;
}

const CString& DirectoryPath::System()
{
    return s_systemDirectory;
}
} // namespace Ipsc

CString DirectoryPath::GetBinDirectory()
{
    TCHAR moduleDirectory[10 * MAX_PATH]{};

    ::GetModuleFileName(NULL, moduleDirectory, sizeof(moduleDirectory));
    ::PathRemoveFileSpec(moduleDirectory);

    return CString{moduleDirectory};
}
