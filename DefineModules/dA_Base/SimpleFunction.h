#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
class _DA_BASE_MODULE_API SimpleFunction
{
public:
    SimpleFunction();
    ~SimpleFunction();

    static CString GetStringCount(long nCnt); //숫자를 입력하면 1st, 2nd, 3rd, 4th, 5th 등을 리턴
    static CString RemoveNonDigits(const CString& input); // 숫자만 남기고 나머지 제거
    static CString CleanAndFormatNumber(const CString& input); // 숫자만 남기고 나머지 제거 후, 3자리 유지
    static bool GetAsyncHiddenKeyDown(const bool ctrl, const bool shift);
    
};

template<typename T>
T ClampBetween(const T& data, const T& minVal, const T& maxVal)
{
    if (data < minVal)
        return minVal;
    if (data > maxVal)
        return maxVal;

    return data;
}