//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "NgrvPackageGrabInfo.h"

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
NgrvPackageGrabInfo::NgrvPackageGrabInfo()
{
    Init();
}

NgrvPackageGrabInfo::~NgrvPackageGrabInfo()
{
    Init();
}

void NgrvPackageGrabInfo::Init()
{
    m_nTrayID = -1;
    m_nPackageID = 0;
    m_strPackageID.Empty();
    m_nGrabPointNum = 0;
    m_vecGrabPointInfo.clear();
}

CArchive& operator<<(CArchive& ar, const NgrvPackageGrabInfo& src)
{
    ar << src.m_nTrayID;
    ar << src.m_nPackageID;
    ar << src.m_strPackageID;

    long nSize = (long)(src.m_vecGrabPointInfo.size());
    ar << nSize;
    for (long i = 0; i < nSize; i++)
        ar << src.m_vecGrabPointInfo[i];

    return ar;
}

CArchive& operator>>(CArchive& ar, NgrvPackageGrabInfo& dst)
{
    dst.Init();

    ar >> dst.m_nTrayID;
    ar >> dst.m_nPackageID;
    ar >> dst.m_strPackageID;

    long nSize = 0;
    ar >> nSize;
    dst.m_nGrabPointNum = nSize;
    dst.m_vecGrabPointInfo.resize(nSize);
    for (long i = 0; i < nSize; i++)
        ar >> dst.m_vecGrabPointInfo[i];

    return ar;
}
