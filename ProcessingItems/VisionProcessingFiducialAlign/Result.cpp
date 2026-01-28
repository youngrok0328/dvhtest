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

void Result::Init()
{
    m_bodyCenterBefore.m_x = 0;
    m_bodyCenterBefore.m_y = 0;

    m_applyAlignTobodyCenter.m_x = 0;
    m_applyAlignTobodyCenter.m_y = 0;

    m_applyAlignTobodyRoi.fptLT.m_x = 0;
    m_applyAlignTobodyRoi.fptLT.m_y = 0;
    m_applyAlignTobodyRoi.fptLB.m_x = 0;
    m_applyAlignTobodyRoi.fptLB.m_y = 0;
    m_applyAlignTobodyRoi.fptRT.m_x = 0;
    m_applyAlignTobodyRoi.fptRT.m_y = 0;
    m_applyAlignTobodyRoi.fptRB.m_x = 0;
    m_applyAlignTobodyRoi.fptRB.m_y = 0;

    m_pads.clear();

    // Debug Result
    m_debugResult_spec_pad_rois.clear();
    m_debugResult_select_pad_rois.clear();
    m_debugResult_search_pad_rois.clear();
    m_debugResult_final_pad_rois.clear();
    //m_debugResult_edgeDetectionLines.clear();
    //m_debugResult_edgePoints.clear();
    //m_debugResult_align_Ellipses.clear();
    //m_debugResult_align_Polygons.clear();
    //m_debugResult_align_Other.clear();
    m_debugResult_align_Center.clear();
}
