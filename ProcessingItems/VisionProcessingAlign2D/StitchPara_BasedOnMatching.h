#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
#include <Ipvm/Base/Rect32r.h>
#include <Ipvm/Base/Rect32s.h>

//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
class CiDataBase;
class VisionProcessing;

//HDR_6_________________________________ Header body
//
struct MatchingROI
{
    Ipvm::Rect32r m_rtHorRef_BCU[2]; // 오른쪽에서 왼쪽으로 붙일때 사용되는 matching ROI
    Ipvm::Rect32r m_rtVerRef_BCU[2]; // 아래에서 위로 붙일때 사용되는 matching ROI

    // 저장되지 않고 계산되는 값
    Ipvm::Rect32s m_rtHorSearch[2]; // 오른쪽에서 왼쪽으로 붙일때 사용되는 search ROI
    Ipvm::Rect32s m_rtVerSearch[2]; // 아래에서 위로 붙일때 사용되는 search ROI
};

class StitchPara_BasedOnMatching
{
public:
    StitchPara_BasedOnMatching();
    ~StitchPara_BasedOnMatching();

    BOOL LinkDataBase(BOOL bSave, long version, CiDataBase& db);
    void Init();

    std::vector<MatchingROI> m_vecStitchROI;

    float m_templateSearchOffset_um;
    long m_nTemplateROICount;
};
