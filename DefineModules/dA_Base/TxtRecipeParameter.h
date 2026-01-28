#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
#include <Ipvm/Base/EllipseEq32r.h>
#include <Ipvm/Base/Point32r2.h>
#include <Ipvm/Base/Point32s2.h>
#include <Ipvm/Base/Rect32r.h>
#include <Ipvm/Base/Rect32s.h>
#include <Ipvm/Base/Rectangle32r.h>

//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
class _DA_BASE_MODULE_API CTxtRecipeParameter
{
public:
    CTxtRecipeParameter();
    ~CTxtRecipeParameter();

    static CString GetTextFromParmeter(const CString strHeader, const CString strCateroty, const CString strGroup,
        const CString strParaName, const CString strParaNameAux,
        const bool bValue); //strHeader : VisionName,ModuleName//BOOL은 내부적으로 define int다. bool로 캐스팅 필수
    static CString GetTextFromParmeter(const CString strHeader, const CString strCateroty, const CString strGroup,
        const CString strParaName, const CString strParaNameAux,
        const short sValue); //strHeader : VisionName,ModuleName
    static CString GetTextFromParmeter(const CString strHeader, const CString strCateroty, const CString strGroup,
        const CString strParaName, const CString strParaNameAux, const int nValue); //strHeader : VisionName,ModuleName
    static CString GetTextFromParmeter(const CString strHeader, const CString strCateroty, const CString strGroup,
        const CString strParaName, const CString strParaNameAux, const long lValue); //strHeader : VisionName,ModuleName
    static CString GetTextFromParmeter(const CString strHeader, const CString strCateroty, const CString strGroup,
        const CString strParaName, const CString strParaNameAux,
        const float fValue); //strHeader : VisionName,ModuleName
    static CString GetTextFromParmeter(const CString strHeader, const CString strCateroty, const CString strGroup,
        const CString strParaName, const CString strParaNameAux,
        const double dValue); //strHeader : VisionName,ModuleName
    static CString GetTextFromParmeter(const CString strHeader, const CString strCateroty, const CString strGroup,
        const CString strParaName, const CString strParaNameAux,
        const CString strValue); //strHeader : VisionName,ModuleName
    static std::vector<CString> GetTextFromParmeter(const CString strHeader, const CString strCateroty,
        const CString strGroup, const CString strParaName, const Ipvm::Rect32s rtRect,
        const bool bInsertROI = FALSE); //strHeader : VisionName,ModuleName //strParaNameAux는 함수 내부에서 자동 입력
    static std::vector<CString> GetTextFromParmeter(const CString strHeader, const CString strCateroty,
        const CString strGroup, const CString strParaName, const Ipvm::Rect32r frtRect,
        const bool bInsertROI = FALSE); //strHeader : VisionName,ModuleName //strParaNameAux는 함수 내부에서 자동 입력
    static std::vector<CString> GetTextFromParmeter(const CString strHeader, const CString strCateroty,
        const CString strGroup, const CString strParaName,
        const Ipvm::Rectangle32r
            frtRect); //strHeader : VisionName,ModuleName //strParaNameAux는 함수 내부에서 자동 입력
    static std::vector<CString> GetTextFromParmeter(const CString strHeader, const CString strCateroty,
        const CString strGroup, const CString strParaName,
        const Ipvm::Point32s2 ptPoint); //strHeader : VisionName,ModuleName //strParaNameAux는 함수 내부에서 자동 입력
    static std::vector<CString> GetTextFromParmeter(const CString strHeader, const CString strCateroty,
        const CString strGroup, const CString strParaName,
        const Ipvm::Point32r2 fptPoint); //strHeader : VisionName,ModuleName //strParaNameAux는 함수 내부에서 자동 입력
    static std::vector<CString> GetTextFromParmeter(const CString strHeader, const CString strCateroty,
        const CString strGroup, const CString strParaName,
        const Ipvm::EllipseEq32r
            eqEllipse); //strHeader : VisionName,ModuleName //strParaNameAux는 함수 내부에서 자동 입력

    //이 아래에는 추후 Text를 위의 각 입력 변수로 리턴하는 함수를 만들것

    //보조 기능.
    static CString GetStringCount(long nCnt); //숫자를 입력하면 1st, 2nd, 3rd, 4th, 5th 등을 리턴
};
