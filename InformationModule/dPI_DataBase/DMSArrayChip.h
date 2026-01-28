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
struct DPI_DATABASE_API DMSArrayChip : public DMSInfo
{
    BOOL LinkDataBase(BOOL bSave, CiDataBase& db, long nCount);

    float fElectWidth;
    float fElectLength;
    float fElectPitch;
    float fElectTip;
    float fPadSizeWidth;
    float fPadSizeLength;

    float fElectThickness;

    int nArrayType; // 0: 1Array, 1: 2Array, 2: 4Array, 3: MLCC 1Array ...
    long nComponentType;

    DMSArrayChip()
    {
        nComponentType = 0;
        nArrayType = 1;

        fChipWidth = 0.3f;
        fChipLength = 0.6f;
        fChipAngle = 90.f;
        fChipThickness = 0.3f;

        fElectWidth = 0.05f;
        fElectLength = 0.05f;
        fElectPitch = 0.03f;
        fElectTip = 0.02f;

        fPadSizeWidth = 0.02f;
        fPadSizeLength = 0.03f;

        fElectThickness = 0.3f;
    }
};

} // namespace Chip