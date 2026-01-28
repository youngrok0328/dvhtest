#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
#include "DMSInfo.h"

//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
// 볼 위치는 픽셀 단위일 경우 유닛의 중앙을 원점으로 하는 왼손 좌표계를 사용한다.
// 볼 위치는 실 단위일 경우 유닛의 중앙을 원점으로 하는 오른손 좌표계를 사용한다.
namespace Chip
{
// 일단, 편의상 걍 여기다 넣자...
struct DPI_DATABASE_API DMSBall : public DMSInfo
{
    BOOL LinkDataBase(BOOL bSave, CiDataBase& db, long nCount);

    DMSBall()
    {
        fChipWidth = 1.0f;
        fChipLength = 1.0f;
        fChipThickness = 0.f;
        fChipAngle = 0.f;
    }

    float GetBallWidth()
    {
        return fChipWidth;
    }
    float GetBallHeight()
    {
        return fChipThickness;
    }
};

} // namespace Chip