//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "NgrvGrabPositionInfo.h"

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
NgrvGrabPositionInfo::NgrvGrabPositionInfo()
{
    Init();
}

NgrvGrabPositionInfo::~NgrvGrabPositionInfo()
{
    Init();
}

void NgrvGrabPositionInfo::Init()
{
    m_strModuleName.Empty();
    m_bIsSaveImage = FALSE;
    m_nNormalFrameNum = 0;
    m_nIRFrameNum = 0;
    m_nAFType = NGRV_AF_MODE_EACH_POINT;
    m_nPlaneOffsetUM = 0;
}

CArchive& operator<<(CArchive& ar, const NgrvGrabPositionInfo& src)
{
    return ar << src.m_strModuleName << src.m_nNormalFrameNum << src.m_nIRFrameNum << src.m_nAFType
              << src.m_nPlaneOffsetUM << src.m_bIsSaveImage;
}

CArchive& operator>>(CArchive& ar, NgrvGrabPositionInfo& dst)
{
    dst.Init();

    return ar >> dst.m_strModuleName >> dst.m_nNormalFrameNum >> dst.m_nIRFrameNum >> dst.m_nAFType
        >> dst.m_nPlaneOffsetUM >> dst.m_bIsSaveImage;
}
