//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "Ngrv2DImageInfo.h"

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
Ngrv2DImageInfo::Ngrv2DImageInfo()
{
    Init();
}

Ngrv2DImageInfo::~Ngrv2DImageInfo()
{
    Init();
}

void Ngrv2DImageInfo::Init()
{
    m_visionID = 0;
    m_imageSizeX = 0;
    m_imageSizeY = 0;
    m_totalImageSize = 0;
    m_scaleX = 0.f;
    m_scaleY = 0.f;
    m_isImageColor = FALSE;
    m_vecbyImage.clear();
}

CArchive& operator<<(CArchive& ar, const Ngrv2DImageInfo& src)
{
    ar << src.m_visionID;
    ar << src.m_imageSizeX;
    ar << src.m_imageSizeY;
    ar << src.m_totalImageSize;
    ar << src.m_scaleX;
    ar << src.m_scaleY;
    ar << src.m_isImageColor;

    long nVecSize = src.m_totalImageSize;
    ar << nVecSize;
    for (long i = 0; i < nVecSize; i++)
    {
        ar << src.m_vecbyImage[i];
    }

    return ar;
}

CArchive& operator>>(CArchive& ar, Ngrv2DImageInfo& dst)
{
    dst.Init();
    ar >> dst.m_visionID;
    ar >> dst.m_imageSizeX;
    ar >> dst.m_imageSizeY;
    ar >> dst.m_totalImageSize;
    ar >> dst.m_scaleX;
    ar >> dst.m_scaleY;
    ar >> dst.m_isImageColor;

    long nVecSize = dst.m_totalImageSize;
    ar >> nVecSize;
    dst.m_vecbyImage.resize(nVecSize);
    for (long i = 0; i < nVecSize; i++)
    {
        ar >> dst.m_vecbyImage[i];
    }

    return ar;
}
