//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "NgrvAfRefInfo.h"

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
NgrvAfRefInfo::NgrvAfRefInfo()
{
    Init();
}

NgrvAfRefInfo::~NgrvAfRefInfo()
{
    Init();
}

void NgrvAfRefInfo::Init()
{
    m_nVisionID = -1;
    m_bIsValidPlaneRefInfo = FALSE;
    m_vecptRefPos_UM.clear();
    m_nImageSizeX = 0;
    m_nImageSizeY = 0;
    m_vecbyImage.clear();
}

void NgrvAfRefInfo::SetImageSize(long nSizeX, long nSizeY)
{
    m_vecbyImage.clear();
    if (nSizeX <= 0 || nSizeY <= 0)
        return;

    m_nImageSizeX = nSizeX;
    m_nImageSizeY = nSizeY;

    long nSize = nSizeX * nSizeY * 3; //Color 영상이니까 3을 곱한다.
    m_vecbyImage.resize(nSize);
    BYTE* pbyImage = &m_vecbyImage[0];
    memset(pbyImage, 0, nSize * sizeof(BYTE));
}

CArchive& operator<<(CArchive& ar, const NgrvAfRefInfo& src)
{
    ar << src.m_nVisionID;
    ar << src.m_bIsValidPlaneRefInfo;

    long nVecSize = (long)(src.m_vecptRefPos_UM.size());
    ar << nVecSize;
    for (long i = 0; i < nVecSize; i++)
        ar << src.m_vecptRefPos_UM[i];

    ar << src.m_nImageSizeX;
    ar << src.m_nImageSizeY;
    long nSize = src.m_nImageSizeX * src.m_nImageSizeY * 3; //Color 영상이니까 3을 곱한다.
    if (nSize > 0)
    {
        const BYTE* pbyImage = &src.m_vecbyImage[0];
        ar.Write(pbyImage, nSize);
    }

    return ar;
}

CArchive& operator>>(CArchive& ar, NgrvAfRefInfo& dst)
{
    dst.Init();
    ar >> dst.m_nVisionID;
    ar >> dst.m_bIsValidPlaneRefInfo;

    long nVecSize = 0;
    ar >> nVecSize;
    dst.m_vecptRefPos_UM.resize(nVecSize);
    for (long i = 0; i < nVecSize; i++)
        ar >> dst.m_vecptRefPos_UM[i];

    long nSizeX = 0;
    long nSizeY = 0;
    ar >> nSizeX;
    ar >> nSizeY;
    long nSize = nSizeX * nSizeY * 3; //Color 영상이니까 3을 곱한다.

    dst.SetImageSize(nSizeX, nSizeY);
    if (nSize > 0)
    {
        BYTE* pbyImage = &dst.m_vecbyImage[0];
        ar.Read(pbyImage, nSize);
    }

    return ar;
}
