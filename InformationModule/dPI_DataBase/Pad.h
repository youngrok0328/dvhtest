#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
#include "Enum.h"

//HDR_2_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/FPI_RECT.h"

//HDR_3_________________________________ External library headers
#include <Ipvm/Base/Point32r2.h>
#include <Ipvm/Base/Rect32s.h>

//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class CiDataBase;

//HDR_6_________________________________ Header body
//
namespace Package
{
LPCTSTR DPI_DATABASE_API GetPAD_Type_Name(PAD_TYPE type);

class DPI_DATABASE_API Pad
{
public:
    CString strPadName{_T("Default")};
    float fWidth{};
    float fLength{};
    float fHeight{};
    long nAngle{};
    BOOL bMount{};
    BOOL bIgnore{}; //FALSE : 활성, TRUE : 비활성

    Ipvm::Rect32s rtROI{}; // LayoutEditor에서의 위치
    FPI_RECT fsrtSpecROI{}; // SpecROI

    Pad();
    Pad(LPCTSTR name, float i_fOffsetX, float i_fOffsetY, float i_fWidth, float i_fLength, float i_fHeight,
        long i_nAngle, PAD_TYPE i_nPadType, BOOL i_bMount, BOOL i_bIgnore);

    ~Pad();

    BOOL LinkDataBase(long version, long padIndex, BOOL bSave, CiDataBase& db);

    const Ipvm::Point32r2& GetCenter() const;
    const Ipvm::Point32r2& GetOffset() const;
    const PAD_TYPE GetType() const;

    void SetDefault();
    void SetOffsetX(float value);
    void SetOffsetY(float value);
    void SetType(PAD_TYPE type);

private:
    Ipvm::Point32r2 m_offset{};
    Ipvm::Point32r2 m_center{};
    PAD_TYPE m_padType{_typeTriangle}; //enum PAD_TYPE 참조

    void MakeCenter();
};
} // namespace Package
