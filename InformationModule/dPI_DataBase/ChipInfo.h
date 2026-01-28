#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/FPI_RECT.h"

//HDR_3_________________________________ External library headers
#include <Ipvm/Base/Rect32r.h>

//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
class BaseAlignResult;

//HDR_6_________________________________ Header body
//
namespace Chip
{
class ChipInfo
{
public:
    ChipInfo() = default;

    enum eType
    {
        tyCHIP = 0,
        tyPASSIVE,
        tyARRAY,
        tyHEATSINK,
        tyBALL,
    };

    long nType{eType::tyCHIP}; // 0: Chip, 1: Cap, 2: Res, 3: Array
    float fAngle{90.f};
    CString strCompType{_T("Chip")};
    Ipvm::Rect32r frtChip{};
    std::vector<Ipvm::Rect32r> vecfrtPAD;

    // 요넘으로 수정되어야 한다.
    FPI_RECT sfrtChip{};
    std::vector<FPI_RECT> vecsfrtPAD;

    void* pDmsInfo{};

    CString strDmsName{_T("Default")};
    CString strSpecName{_T("Default")};

    // Align Spec 및 결과
    void* pSpec{};
    BaseAlignResult* pResult{};

    long nResult{}; // 검사 결과값 PASS, REJECT;
};

} // namespace Chip