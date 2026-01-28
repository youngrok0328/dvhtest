//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "SingleLock.h"

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
using namespace Ipsc;

SingleLock::SingleLock(CRITICAL_SECTION* cs)
    : m_cs(cs)
{
    ::EnterCriticalSection(m_cs);
}

SingleLock::~SingleLock()
{
    ::LeaveCriticalSection(m_cs);
}
