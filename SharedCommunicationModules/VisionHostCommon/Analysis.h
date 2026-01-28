#pragma once

//==========================================================================================
// 검사와 상관없이 Host의 Manual Inspection 에서
// 데이터 분석을 위해 수집하는 데이타이다.
// 이것을 이용하여 보다 Job 티칭을 제대로하기 위한 도와준다.
//==========================================================================================

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <memory>

//HDR_5_________________________________ Forward declarations
class ArchiveAllType;

//HDR_6_________________________________ Header body
//
class __VISION_HOST_COMMON_API__ Analysis
{
public:
    Analysis();
    Analysis(const Analysis& object);
    ~Analysis();

    void Init();
    void Serialize(ArchiveAllType& ar);
    void SetUse(bool use);
    bool isUsed() const;

    bool AccumulateZ(float z, long count);

    std::pair<float, long> GetZC(long index) const;
    long GetZCCount() const;

    Analysis& operator=(const Analysis& object);

    float m_packageOffsetX;
    float m_packageOffsetY;

private:
    bool m_use;
    long* m_histogramZ;
};
