//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "NgrvGrabInfo.h"

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
NgrvGrabInfo::NgrvGrabInfo()
{
    Init();
}

NgrvGrabInfo::~NgrvGrabInfo()
{
    Init();
}

void NgrvGrabInfo::Init()
{
    m_nTrayID = 0;
    m_nPackageID = 0;
    m_strPackageID = _T("");
    m_vecNGRVSingleRunInfo.clear();
}

CArchive& operator<<(CArchive& ar, const NgrvGrabInfo& src)
{
    ar << src.m_nTrayID;
    ar << src.m_nPackageID;
    ar << src.m_strPackageID;

    long nVecSize = (long)(src.m_vecNGRVSingleRunInfo.size());
    ar << nVecSize;
    for (long i = 0; i < nVecSize; i++)
    {
        ar << src.m_vecNGRVSingleRunInfo[i];
    }

    return ar;
}

CArchive& operator>>(CArchive& ar, NgrvGrabInfo& dst)
{
    dst.Init();
    ar >> dst.m_nTrayID;
    ar >> dst.m_nPackageID;
    ar >> dst.m_strPackageID;

    long nVecSize = 0;
    ar >> nVecSize;
    dst.m_vecNGRVSingleRunInfo.resize(nVecSize);
    for (long i = 0; i < nVecSize; i++)
    {
        ar >> dst.m_vecNGRVSingleRunInfo[i];
    }

    return ar;
}
