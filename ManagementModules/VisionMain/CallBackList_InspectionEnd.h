#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
#include "../../VisionNeedLibrary/VisionCommon/VisionMainAgent.h"

//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <map>

//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
class CallBackList_InspectionEnd
{
public:
    CallBackList_InspectionEnd();
    ~CallBackList_InspectionEnd();

    void Add(void* userData, VisionMainAgent::FUNC_INSPECTION_END_CALLBACK* callback);
    void Del(VisionMainAgent::FUNC_INSPECTION_END_CALLBACK* callback);

    void CallForEnd(VisionUnit& visionUnit, long threadIndex);

private:
    std::map<VisionMainAgent::FUNC_INSPECTION_END_CALLBACK*, void*> m_map;
    CCriticalSection m_lock;
};
