//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "NgrvGrabbedImageInfo.h"

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
NgrvGrabbedImageInfo::NgrvGrabbedImageInfo()
{
    Init();
}

NgrvGrabbedImageInfo::~NgrvGrabbedImageInfo()
{
    Init();
}

void NgrvGrabbedImageInfo::Init()
{
    m_nTrayID = 0;
    m_nPackageID = 0;
    m_strPackageID.Empty();
    m_nGrabPointID = 0;
    m_nSendImageSizeX = 0;
    m_nSendImageSizeY = 0;
    m_vecstrImageFileName.clear();
}

CArchive& operator<<(CArchive& ar, const NgrvGrabbedImageInfo& src)
{
    ar << src.m_nTrayID;
    ar << src.m_nPackageID;
    ar << src.m_strPackageID;
    ar << src.m_nGrabPointID;
    ar << src.m_nSendImageSizeX;
    ar << src.m_nSendImageSizeY;

    long nSize = (long)(src.m_vecstrImageFileName.size());
    ar << nSize;
    for (long i = 0; i < nSize; i++)
        ar << src.m_vecstrImageFileName[i];

    return ar;
}

CArchive& operator>>(CArchive& ar, NgrvGrabbedImageInfo& dst)
{
    dst.Init();

    ar >> dst.m_nTrayID;
    ar >> dst.m_nPackageID;
    ar >> dst.m_strPackageID;
    ar >> dst.m_nGrabPointID;
    ar >> dst.m_nSendImageSizeX;
    ar >> dst.m_nSendImageSizeY;

    long nSize = 0;
    ar >> nSize;
    dst.m_vecstrImageFileName.resize(nSize);
    for (long i = 0; i < nSize; i++)
        ar >> dst.m_vecstrImageFileName[i];

    return ar;
}
