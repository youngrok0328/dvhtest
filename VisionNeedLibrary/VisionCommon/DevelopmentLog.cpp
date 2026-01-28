//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "DevelopmentLog.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/DynamicSystemPath.h"

//CPP_4_________________________________ External library headers
#include <afxmt.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define MAX_LOG_QUEUE (1000)

//CPP_7_________________________________ Implementation body
//
HWND m_hwndMoniteringHwnd = NULL;
UINT m_hwndMoniteringMsg = 0;
bool g_firstLog = true;

#if IPIS500I_BUILD_NUMBER == IPIS500I_DEVELOPER_VERSION_BUILD_NUMBER
bool g_typeOption[long(DevelopmentLog::Type::End)] = {
    true,
    true,
    true,
};
bool g_makeLogFile = true;
#else
bool g_typeOption[long(DevelopmentLog::Type::End)] = {
    true,
    false,
    true,
};
bool g_makeLogFile = false;
#endif // 0

CCriticalSection g_logLock;
std::vector<std::pair<DevelopmentLog::Type, CString>> g_logList;

CString GetLogFilePath(DevelopmentLog::Type type)
{
    CString filePath = DynamicSystemPath::get(DefineFolder::Log);
    switch (type)
    {
        case DevelopmentLog::Type::Develop:
            filePath += _T("Developer_Log_Develop.txt");
            break;
        case DevelopmentLog::Type::Event:
            filePath += _T("Developer_Log_Event.txt");
            break;
        case DevelopmentLog::Type::ImageGrab:
            filePath += _T("Developer_Log_ImageGrab.txt");
            break;
        case DevelopmentLog::Type::TCP:
            filePath += _T("Developer_Log_TCP.txt");
            break;
        case DevelopmentLog::Type::Thread:
            filePath += _T("Developer_Log_Thread.txt");
            break;
        case DevelopmentLog::Type::Warning:
            filePath += _T("Developer_Log_Warning.txt");
            break;
        case DevelopmentLog::Type::Notice:
            filePath += _T("Developer_Log_Notice.txt");
            break;
    }

    return filePath;
}

bool DevelopmentLog::IsTypeEnable(Type type)
{
    return g_typeOption[long(type)];
}

void DevelopmentLog::SetTypeEnabled(Type type, bool enable)
{
    g_typeOption[long(type)] = enable;
}

void DevelopmentLog::AddLog(Type type, LPCTSTR szText, ...)
{
    if (!g_makeLogFile && !g_typeOption[long(type)])
        return;

    CString strTemp;

    va_list argList;
    va_start(argList, szText);
    strTemp.FormatV(szText, argList);
    va_end(argList);

    CTime curTime(CTime::GetCurrentTime());
    CString strEventTime;

    strEventTime.Format(
        _T("[%02d:%02d:%02d] %s\r\n"), curTime.GetHour(), curTime.GetMinute(), curTime.GetSecond(), LPCTSTR(strTemp));

    CSingleLock lock(&g_logLock, TRUE);

    if (g_makeLogFile)
    {
        if (g_firstLog)
        {
            DeleteFile(GetLogFilePath(Type::Develop));
            DeleteFile(GetLogFilePath(Type::Event));
            DeleteFile(GetLogFilePath(Type::ImageGrab));
            DeleteFile(GetLogFilePath(Type::TCP));
            DeleteFile(GetLogFilePath(Type::Thread));
            DeleteFile(GetLogFilePath(Type::Warning));
            DeleteFile(GetLogFilePath(Type::Notice));
            g_firstLog = false;
        }

        auto logFilePath = GetLogFilePath(type);
        CFile file;
        if (!file.Open(logFilePath, CFile::modeCreate | CFile::modeNoTruncate | CFile::modeWrite))
        {
            return;
        }

        CStringA asciiText(strEventTime);
        file.SeekToEnd();
        file.Write(asciiText.GetBuffer(), asciiText.GetLength());
        file.Close();
    }

    if (!g_typeOption[long(type)])
        return;

    g_logList.emplace_back(type, strEventTime);
    if (g_logList.size() > MAX_LOG_QUEUE)
        g_logList.erase(g_logList.begin());

    if (m_hwndMoniteringHwnd)
    {
        ::PostMessage(m_hwndMoniteringHwnd, m_hwndMoniteringMsg, 0, 0);
    }

    strTemp.Empty();
    strEventTime.Empty();
}

void DevelopmentLog::PopLog(std::vector<std::pair<Type, CString>>& o_log)
{
    CSingleLock lock(&g_logLock, TRUE);

    o_log = g_logList;
    g_logList.clear();
}

void DevelopmentLog::SetMonitoringHwnd(HWND hwnd, UINT message)
{
    CSingleLock lock(&g_logLock, TRUE);

    m_hwndMoniteringHwnd = hwnd;
    m_hwndMoniteringMsg = message;

    if (m_hwndMoniteringHwnd)
    {
        // 한번은 그냥 발생시켜 주자
        ::PostMessage(m_hwndMoniteringHwnd, m_hwndMoniteringMsg, 0, 0);
    }
}
