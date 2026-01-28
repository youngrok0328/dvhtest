#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
class ArchiveAllType;
class VisionDeviceResult;

//HDR_6_________________________________ Header body
//
class __VISION_HOST_COMMON_API__ SideVisionDeviceResult
{
public:
    SideVisionDeviceResult();
    ~SideVisionDeviceResult();

    void Init();
    void Serialize(ArchiveAllType& ar);
    BOOL
    UpdateCommonInfo(); //m_vecVisionDeviceResult에 모든 Data를 채운 후 해당 Class의 각종 ID Data와 Result Data를 업데이트하는 용도.

public:
    //이하는 원래 VisionDeviceResult에서 관리하던 ID 정보
    long m_nThreadID;
    long m_nProbeID;
    long m_nTrayID;
    long m_nInspectionRepeatIndex;
    long m_nPocketID;
    long m_nScanID; // 핸들러에서 필요해서 임시로 넣었음. 추후 변경 예정
    long m_nPane;
    long m_nTotalResult;

    std::vector<long> m_vecnSideEachModuleResult; //Front/Rear Module 각각의 Total Result
    std::vector<std::vector<long>>
        m_vec2nSideEachModuleInspectionResult; //Front/Rear 각 Module의 각각의 검사 항목의 Result를 저장
    std::vector<VisionDeviceResult> m_vecVisionDeviceResult; //Front/Rear Module 각각의 Inspection Result
};
