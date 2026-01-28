#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/FPI_RECT.h"

//HDR_3_________________________________ External library headers
#include <Ipvm/Base/Rect32s.h>
#include <afxstr.h>
#include <minwindef.h>
#include <tchar.h>
#include <winnt.h>

//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
namespace Package
{
class DPI_DATABASE_API Component
{
public:
    CString strCompName{_T("Default")};
    float fOffsetX{};
    float fOffsetY{};
    CString strCompCategory{_T("Default")};
    CString strCompType{_T("Default")};
    float fWidth{};
    float fLength{};
    float fHeight{};
    long nAngle{};
    float fPassiveElectLenght{};
    float fPassiveElectHeight{};
    float fPassivePadGap{};
    float fPassivePadWidth{};
    float fPassivePadLenght{};
    BOOL bIgnore{}; //FALSE : 활성, TRUE : 비활성

    long nCompType{}; //enum COMPONENT_TYPE 참조
    Ipvm::Rect32s rtROI{}; // LayoutEditor에서의 실제 위치 ( Pixel 좌표계 )
    Ipvm::Rect32s rtPadROI{}; // Image 좌표계에서의 위치
    FPI_RECT fsrtROI{};
    FPI_RECT fsrtRealROI{};

    Component() = default;

    Component(LPCTSTR i_strCompName, float i_fOffsetX, float i_fOffsetY, LPCTSTR i_strCompCategory,
        LPCTSTR i_strCompType, float i_fWidth, float i_fLength, float i_fHeight, long i_nAngle,
        float i_fPassiveElectLenght, float i_fPassiveElectHeight, float i_fPassivePadGap, float i_fPassivePadWidth,
        float i_fPassivePadLenght, BOOL i_bIgnore);

    long GetCompType();
};

} // namespace Package