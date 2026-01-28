//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "CallBackList_InspectionEnd.h"

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
CallBackList_InspectionEnd::CallBackList_InspectionEnd()
{
}

CallBackList_InspectionEnd::~CallBackList_InspectionEnd()
{
}

void CallBackList_InspectionEnd::Add(void* userData, VisionMainAgent::FUNC_INSPECTION_END_CALLBACK* callback)
{
    CSingleLock lock(&m_lock, TRUE);

    m_map[callback] = userData;
}

void CallBackList_InspectionEnd::Del(VisionMainAgent::FUNC_INSPECTION_END_CALLBACK* callback)
{
    CSingleLock lock(&m_lock, TRUE);

    m_map.erase(callback);
}

void CallBackList_InspectionEnd::CallForEnd(VisionUnit& visionUnit, long threadIndex)
{
    CSingleLock lock(&m_lock, TRUE);

    for (auto& itMap : m_map)
    {
        itMap.first(itMap.second, visionUnit, threadIndex);
    }
}
