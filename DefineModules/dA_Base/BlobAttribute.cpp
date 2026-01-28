//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
//CPP_2_________________________________ This project's headers
#include "BlobAttribute.h"

//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
BlobAttribute::BlobAttribute()
{
    Clear();
}

BlobAttribute::~BlobAttribute()
{
}

void BlobAttribute::Clear()
{
    m_massCenter.m_x = 0.f;
    m_massCenter.m_y = 0.f;

    // 2D
    m_localAreaPercent_All = 0.f;
    m_localAreaPercent = 0.f;
    m_keyContrast = 0.f;
    m_averageContrast = 0.f;
    m_localContrast = 0.f;
    m_auxLocalContrast = 0.f;
    m_deviation = 0.f;
    m_auxDeviation = 0.f;
    m_blobType = DARK;
    m_width = 0.f;
    m_length = 0.f;
    m_lengthX = 0.f;
    m_lengthY = 0.f;
    m_thickness = 0.f;
    m_locus = 0.f;
    m_edgeEnergy = 0.f;
    m_fMergedCount = 0.f;
    m_fBlobGV_AVR = 0.f;
    m_fBlobGV_Min = 0.f;
    m_fBlobGV_Max = 0.f;

    // 3D
    m_averageDeltaHeight = 0.f;
    m_keyDeltaHeight = 0.f;
};
