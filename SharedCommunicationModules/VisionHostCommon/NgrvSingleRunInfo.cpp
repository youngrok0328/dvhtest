//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "NgrvSingleRunInfo.h"

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
NgrvSingleRunInfo::NgrvSingleRunInfo()
{
    Init();
}

NgrvSingleRunInfo::~NgrvSingleRunInfo()
{
    Init();
}

void NgrvSingleRunInfo::Init()
{
    m_vecOnePointGrabInfo.clear();
}

CArchive& operator<<(CArchive& ar, const NgrvSingleRunInfo& src)
{
    long nVecSize = (long)(src.m_vecOnePointGrabInfo.size());
    ar << nVecSize;
    for (long i = 0; i < nVecSize; i++)
    {
        ar << src.m_vecOnePointGrabInfo[i];
    }

    return ar;
}

CArchive& operator>>(CArchive& ar, NgrvSingleRunInfo& dst)
{
    dst.Init();

    long nVecSize = 0;
    ar >> nVecSize;
    dst.m_vecOnePointGrabInfo.resize(nVecSize);
    for (long i = 0; i < nVecSize; i++)
    {
        ar >> dst.m_vecOnePointGrabInfo[i];
    }

    return ar;
}
