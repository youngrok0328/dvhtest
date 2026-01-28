#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
#include "Enum.h"

//HDR_2_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/FPI_RECT.h"

//HDR_3_________________________________ External library headers
#include <Ipvm/Base/Rect32s.h>

//HDR_4_________________________________ Standard library headers
#include <afxstr.h>
#include <minwindef.h>
#include <tchar.h>
#include <winnt.h>
//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
namespace Package
{
//Land Mount Map Data (. csv 파일 )
class DPI_DATABASE_API Land
{
public:
    CString m_groupID{};
    CString strLandID{_T("Default")};
    float fOffsetX{};
    float fOffsetY{};
    float fWidth{};
    float fLength{};
    float fHeight{};
    long nAngle{};
    long nIndex{}; // 배열에서 몇 번째인지..
    BOOL bIgnore{}; //FALSE : 활성, TRUE : 비활성
    long nLandShapeType{eunmLandShapeType::Shape_Base_Rect}; //eunmLandShapeType 참조 //kircheis_LandShape

    Ipvm::Rect32s rtROI{}; // LayoutEditor에서의 위치
    FPI_RECT fsrtSpecROI{}; // SpecROI

    Land() = default;

    Land(LPCTSTR i_groupID, LPCTSTR i_strLandID, float i_fOffsetX, float i_fOffsetY, float i_fWidth, float i_fLength,
        float i_fHeight, long i_nAngle, long i_nIndex, BOOL i_bIgnore, long i_nLandShapeType);
};
} // namespace Package
