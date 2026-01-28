//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "NgrvGrabPointInfo.h"

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
NgrvGrabPointInfo::NgrvGrabPointInfo()
{
    Init();
}

NgrvGrabPointInfo::~NgrvGrabPointInfo()
{
    Init();
}

void NgrvGrabPointInfo::Init()
{
    m_strInspModuleName.Empty();
    m_strInspItemName.Empty();
    m_strReviewSide.Empty();
    m_nStitchingCountX = 1;
    m_nStitchingCountY = 1;
    m_fOverlapLengthX = 0.f;
    m_fOverlapLengthY = 0.f;
}

CArchive& operator<<(CArchive& ar, const NgrvGrabPointInfo& src)
{
    return ar << src.m_strInspModuleName << src.m_strInspItemName << src.m_strReviewSide << src.m_nStitchingCountX
              << src.m_nStitchingCountY << src.m_fOverlapLengthX << src.m_fOverlapLengthY;
}

CArchive& operator>>(CArchive& ar, NgrvGrabPointInfo& dst)
{
    dst.Init();

    return ar >> dst.m_strInspModuleName >> dst.m_strInspItemName >> dst.m_strReviewSide >> dst.m_nStitchingCountX
        >> dst.m_nStitchingCountY >> dst.m_fOverlapLengthX >> dst.m_fOverlapLengthY;
}
