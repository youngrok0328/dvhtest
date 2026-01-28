//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "SurfacePrepairResult.h"

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
SurfacePrepairResult::SurfacePrepairResult(const bool detailSetupMode)
    : m_detailSetupMode(detailSetupMode)
    , m_is3D(false)
    , m_success(false)
{
}

SurfacePrepairResult::~SurfacePrepairResult()
{
}

bool SurfacePrepairResult::isDetailSetupMode() const
{
    return m_detailSetupMode;
}
