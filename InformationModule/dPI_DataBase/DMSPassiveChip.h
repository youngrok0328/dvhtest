#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
#include "DMSInfo.h"

//HDR_2_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/iDataType.h"

//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
// 볼 위치는 픽셀 단위일 경우 유닛의 중앙을 원점으로 하는 왼손 좌표계를 사용한다.
// 볼 위치는 실 단위일 경우 유닛의 중앙을 원점으로 하는 오른손 좌표계를 사용한다.
namespace Chip
{
struct DPI_DATABASE_API DMSPassiveChip : public DMSInfo
{
    BOOL LinkDataBase(BOOL bSave, CiDataBase& db, long nCount);

    float fChipPAD_Gap;
    float fChipPAD_Width;
    float fChipPAD_Length;

    float fElectWidth; //PassiveElectrodeLength
    float fElectThickness;

    long nPassiveType;
    long nComponentType;

    DMSPassiveChip()
    {
        nPassiveType = enumPassiveTypeDefine::enum_PassiveType_Capacitor;

        fChipOffsetX = 0.f;
        fChipOffsetY = 0.f;
        fChipWidth = 0.45f;
        fChipLength = 0.95f;
        fChipAngle = 90.f;
        fChipThickness = 0.5f;

        fChipPAD_Gap = 0.15f;
        fChipPAD_Width = 0.25f;
        fChipPAD_Length = 0.1f;

        fElectWidth = 0.08f;
        fElectThickness = 0.5f;
    }
};
} // namespace Chip
