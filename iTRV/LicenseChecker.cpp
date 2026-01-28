//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "LicenseChecker.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
time_t time_when_compiled()
{
    CString datestr(__DATE__);

    // 파싱을 편하게 하기 위해 공백문자를 최대 1자로 만들자
    while (datestr.Replace(_T("  "), _T(" ")))
        ;

    long find1 = datestr.Find(_T(" "));
    long find2 = datestr.Find(_T(" "), find1 + 1);

    if (find1 < 0 || find2 < 0)
    {
        exit(-1);
        return 0;
    }

    CString str_month = datestr.Left(find1);

    int month;
    if (str_month == _T("Jan"))
        month = 1;
    else if (str_month == _T("Feb"))
        month = 2;
    else if (str_month == _T("Mar"))
        month = 3;
    else if (str_month == _T("Apr"))
        month = 4;
    else if (str_month == _T("May"))
        month = 5;
    else if (str_month == _T("Jun"))
        month = 6;
    else if (str_month == _T("Jul"))
        month = 7;
    else if (str_month == _T("Aug"))
        month = 8;
    else if (str_month == _T("Sep"))
        month = 9;
    else if (str_month == _T("Oct"))
        month = 10;
    else if (str_month == _T("Nov"))
        month = 11;
    else if (str_month == _T("Dec"))
        month = 12;
    else
        exit(-1);

    CString str_day = datestr.Mid(find1 + 1, find2 - find1);
    CString str_year = datestr.Mid(find2 + 1);

    tm t = {0};
    t.tm_mon = month - 1;
    t.tm_mday = _ttoi(str_day);
    t.tm_year = _ttoi(str_year) - 1900;
    t.tm_hour = 0;
    t.tm_min = 0;
    t.tm_sec = 0;
    return mktime(&t);
}

void LicenseChecker::check()
{
    time_t build_time = time_when_compiled();
    time_t current_time;
    time(&current_time);
    long nDayDate = 24 * 60 * 60; //Hour, Minute, Second, Day

    long nExpirationDate
        = nDayDate * 180; // 배포후 최대 180일간만 사용할 수 있도록 하였다. //mc_2020.12.03 고객사 요청사항
    double diff = fabs(difftime(current_time, build_time));

    if (diff > nExpirationDate)
    {
        AfxMessageBox(_T("This version is not available. Download the new version."));
        exit(-1);
    }

    //mc_2020.12.03 TD 요청사항 만료기간이 3주 이하로 남았을 시에 Error MSG를 출력
    long nExpirationWarringDate = nDayDate * 21;
    long nExpirationWarringValue = nExpirationDate - nExpirationWarringDate;
    if (diff > nExpirationWarringValue)
    {
        long nValidDay = static_cast<long>(((nExpirationDate - diff) / nDayDate) + 1);
        CString strWarringMSG("");
        strWarringMSG.Format(
            _T("[Warring] S/W usable date will be expired in %d day(s). Please contact FSE"), nValidDay);
        AfxMessageBox(strWarringMSG);
    }
}
