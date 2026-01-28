//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "StitchResult.h"

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
StitchResult::StitchResult()
    : m_nSaveTemplateImageIndex(0)
    , m_nSaveInterpolationImageIndex(0)
{
}

StitchResult::~StitchResult()
{
}

bool StitchResult::Prepare(long stitchCount)
{
    m_additionalCriticalLog = _T("");
    m_nSaveTemplateImageIndex = 0;
    m_nSaveInterpolationImageIndex = 0;

    m_stitchRois.clear();
    m_stitchRois.resize(stitchCount);

    return true;
}
