#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/FPI_RECT.h"

//HDR_3_________________________________ External library headers
#include <Ipvm/Base/Rect32r.h>

//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
class CiDataBase;

//HDR_6_________________________________ Header body
//
// 단위: mm
namespace Chip
{
struct DPI_DATABASE_API DMSInfo
{
    CString strCompSpec;
    CString strCompType; // Component Type
    CString strCompID;
    std::vector<Ipvm::Rect32r> vecfrtPad; // 거버상의 Pad 좌표 //mc_나중에 삭제하기로 한다
    std::vector<FPI_RECT> vecsfrtPad; // 거버상의 PAD 좌표 //mc_나중에 삭제하기로 한다

    Ipvm::Rect32r frtChip; // Pad좌표 Merge된 영역 //mc_?
    FPI_RECT sfrtChip; // PAD 좌표 Merge된 영역 //mc_?

    float fChipOffsetX;
    float fChipOffsetY;
    float fChipWidth;
    float fChipLength;
    float fChipAngle;
    float fChipThickness;

    DMSInfo()
    {
        strCompSpec = _T("Default");
        strCompType = _T("Default");
        strCompID = _T("Default");
        vecfrtPad.clear();

        frtChip = Ipvm::Rect32r(0.f, 0.f, 0.f, 0.f);
        sfrtChip = FPI_RECT(
            Ipvm::Point32r2(0.f, 0.f), Ipvm::Point32r2(0.f, 0.f), Ipvm::Point32r2(0.f, 0.f), Ipvm::Point32r2(0.f, 0.f));

        fChipOffsetX = 0.f;
        fChipOffsetY = 0.f;
        fChipWidth = 0.f;
        fChipLength = 0.f;
        fChipAngle = 0.f;
        fChipThickness = 0.f;
    }
};

} // namespace Chip