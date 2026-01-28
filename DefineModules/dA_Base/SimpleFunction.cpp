//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "SimpleFunction.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

SimpleFunction::SimpleFunction()
{
}

SimpleFunction::~SimpleFunction()
{
}

CString SimpleFunction::GetStringCount(long nCnt)
{
    CString str;
    if (nCnt <= 0)
        return _T("");
    else if (nCnt == 1)
        str.Format(_T("1st"));
    else if (nCnt == 2)
        str.Format(_T("2nd"));
    else if (nCnt == 3)
        str.Format(_T("3rd"));
    else
        str.Format(_T("%dth"), nCnt);

    return str;
}

CString SimpleFunction::RemoveNonDigits(const CString& input)
{
    CString result;
    for (int i = 0; i < input.GetLength(); ++i)
    {
        if (_istdigit(input[i])) // 숫자인지 확인
        {
            result += input[i];
        }
    }
    return result;
}

CString SimpleFunction::CleanAndFormatNumber(const CString& input)
{
    // 1. 숫자만 추출
    CString digits = SimpleFunction::RemoveNonDigits(input);

    // 2. 앞자리부터 최대 3자리만 유지
    if (digits.GetLength() > 3)
        digits = digits.Left(3);

    // 3. 숫자 형식으로 변환
    int number = _ttoi(digits);

    // 4. 3자리로 포맷 (백 미만일 경우 앞에 0)
    CString formatted;
    formatted.Format(_T("%03d"), number);

    return formatted;
}

bool SimpleFunction::GetAsyncHiddenKeyDown(const bool ctrl, const bool shift)
{
    bool currentKeyDownCtrl = (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0;
    bool currentKeyDownShift = (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0;

    if (ctrl && shift)
        return currentKeyDownCtrl && currentKeyDownShift;
    else if (ctrl)
        return currentKeyDownCtrl;
    else if (shift)
        return currentKeyDownShift;

    return false;
}