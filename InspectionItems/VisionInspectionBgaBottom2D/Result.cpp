//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "Result.h"

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
Result::Result()
{
}

Result::~Result()
{
}

void Result::reset()
{
    m_rough_vecSweepLine.clear();
    m_rough_vecfptBallWidthEdgeTotalPoint.clear();
    m_rough_vecellipseAlignCircle.clear(); //kircheis_MED3

    m_vecSweepLine.clear();
    m_vecfptBallWidthEdgeTotalPoint.clear();
    m_vecfptBallWidthEdgeValidPoint.clear();
}
