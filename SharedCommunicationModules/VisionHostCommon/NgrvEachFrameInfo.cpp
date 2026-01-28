//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "NgrvEachFrameInfo.h"

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
NgrvEachFrameInfo::NgrvEachFrameInfo()
{
    Init();
}

NgrvEachFrameInfo::~NgrvEachFrameInfo()
{
    Init();
}

void NgrvEachFrameInfo::Init()
{
    m_bIsIR = FALSE;
    m_vecstrInspModuleName.clear();
}

CArchive& operator<<(CArchive& ar, const NgrvEachFrameInfo& src)
{
    ar << src.m_bIsIR;

    long nSize = (long)(src.m_vecstrInspModuleName.size());
    ar << nSize;
    for (long i = 0; i < nSize; i++)
        ar << src.m_vecstrInspModuleName[i];

    return ar;
}

CArchive& operator>>(CArchive& ar, NgrvEachFrameInfo& dst)
{
    dst.Init();

    ar >> dst.m_bIsIR;

    long nSize = 0;
    ar >> nSize;
    dst.m_vecstrInspModuleName.resize(nSize);
    for (long i = 0; i < nSize; i++)
        ar >> dst.m_vecstrInspModuleName[i];

    return ar;
}
