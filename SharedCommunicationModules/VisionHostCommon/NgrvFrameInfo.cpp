//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "NgrvFrameInfo.h"

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
NgrvFrameInfo::NgrvFrameInfo()
{
    Init();
}

NgrvFrameInfo::~NgrvFrameInfo()
{
    Init();
}

void NgrvFrameInfo::Init()
{
    m_nBeginIRFrame = -1;
    m_vecEachFrameInfo.clear();
}

CArchive& operator<<(CArchive& ar, const NgrvFrameInfo& src)
{
    ar << src.m_nBeginIRFrame;

    long nSize = (long)(src.m_vecEachFrameInfo.size());
    ar << nSize;
    for (long i = 0; i < nSize; i++)
        ar << src.m_vecEachFrameInfo[i];

    return ar;
}

CArchive& operator>>(CArchive& ar, NgrvFrameInfo& dst)
{
    dst.Init();
    ar >> dst.m_nBeginIRFrame;

    long nSize = 0;
    ar >> nSize;
    dst.m_vecEachFrameInfo.resize(nSize);
    for (long i = 0; i < nSize; i++)
        ar >> dst.m_vecEachFrameInfo[i];

    return ar;
}
