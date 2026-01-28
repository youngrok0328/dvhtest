//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "DebugOptionData.h"

//CPP_2_________________________________ This project's headers
#include "ArchiveAllType.h"

//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
DebugOptionData::DebugOptionData()
{
    Init();
}

DebugOptionData::~DebugOptionData()
{
}

void DebugOptionData::Init()
{
    bUse = FALSE;
    nTotalItemCheckStatus = DEBUG_NOTPASS;
    vecnItemCheckStatus.resize(0);
}

void DebugOptionData::Serialize(ArchiveAllType& ar)
{
    if (ar.IsStoring())
    {
        ar << bUse;
        ar << nTotalItemCheckStatus;
        ar << (long)(vecnItemCheckStatus.size());
        for (int n = 0; n < (long)(vecnItemCheckStatus.size()); n++)
        {
            ar << vecnItemCheckStatus[n];
        }
    }
    else
    {
        long nCount;
        ar >> bUse;
        ar >> nTotalItemCheckStatus;
        ar >> nCount;
        vecnItemCheckStatus.resize(nCount);
        for (int n = 0; n < nCount; n++)
        {
            ar >> vecnItemCheckStatus[n];
        }
    }
}
