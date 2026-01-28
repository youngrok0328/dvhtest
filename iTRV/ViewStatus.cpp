//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "ViewStatus.h"

//CPP_2_________________________________ This project's headers
#include "DialogITRV.h"
#include "FileVersion.h"
#include "resource.h"

//CPP_3_________________________________ Other projects' headers
#include "../InformationModule/dPI_SystemIni/SystemConfig.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
IMPLEMENT_DYNAMIC(ViewStatus, CDialogEx)

ViewStatus::ViewStatus(DialogITRV* parent)
    : CDialogEx(IDD_VIEW_STATUS, parent)
    , m_parent(parent)
{
}

ViewStatus::~ViewStatus()
{
}

void ViewStatus::UpdateAccessModeText()
{
    auto& system = SystemConfig::GetInstance();
    CString text;
    if (system.m_nCurrentAccessMode == _OPERATOR)
        text = L"Operator";
    else if (system.m_nCurrentAccessMode == _ENGINEER)
        text = L"Engineer";
    else if (system.m_nCurrentAccessMode == _INTEKPLUS)
        text = L"Intekplus";

    m_valueAccessMode.SetWindowText(text);
}

void ViewStatus::UpdateLayout()
{
    CRect windowRect;
    GetClientRect(&windowRect);
    auto cx = windowRect.Width();
    auto cy = windowRect.Height();

    // 앞에서부터 레이아웃 조정
    auto x = 0;
    m_titleVersion.MoveWindow(x, 0, 50, cy, TRUE);
    x += 50;
    m_valueVersion.MoveWindow(x, 0, 200, cy, TRUE);
    x += 100;

    // 뒤부터 레이아웃 조정
    m_valueAccessMode.MoveWindow(cx - 100, 0, 100, cy, TRUE);
    cx -= 100;

    m_titleAccessMode.MoveWindow(cx - 100, 0, 100, cy, TRUE);
    cx -= 100;
}

void ViewStatus::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_TITLE_VERSION, m_titleVersion);
    DDX_Control(pDX, IDC_VALUE_VERSION, m_valueVersion);
    DDX_Control(pDX, IDC_TITLE_ACCESS_MODE, m_titleAccessMode);
    DDX_Control(pDX, IDC_VALUE_ACCESS_MODE, m_valueAccessMode);
}

BEGIN_MESSAGE_MAP(ViewStatus, CDialogEx)
ON_WM_SIZE()
ON_STN_DBLCLK(IDC_VALUE_ACCESS_MODE, &ViewStatus::OnStnDblclickValueAccessMode)
END_MESSAGE_MAP()

// ViewStatus 메시지 처리기

BOOL ViewStatus::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    // 영훈 20131226 :: 파일의 버전을 받아오도록 한다.
    CString strExe = _T("iTRV.exe");

    // 영훈 20160802 : Main 화면에 표시횔 Version 부분을 세팅한다.
    FileVersion cFileVer;
    cFileVer.Open(strExe);

    auto appDetailVersion = cFileVer.GetFileVersion2();
    if (true)
    {
        auto strR = appDetailVersion.Right(1);
        long n4thHeadNum = 4;
        strR.MakeUpper();
        if (strR == _T("R") || strR == _T("D"))
            n4thHeadNum = 5;
        auto strFileVersion_Temp = appDetailVersion.Right(n4thHeadNum);
        TCHAR cVersion0 = strFileVersion_Temp.GetAt(0);

        strFileVersion_Temp = appDetailVersion;

        if (cVersion0 == _T('8') || cVersion0 == _T('9'))
        {
            long nStrCount = strFileVersion_Temp.GetLength();

            if (n4thHeadNum == 5)
                strFileVersion_Temp.Delete(nStrCount - 1, 1);

            strFileVersion_Temp += _T(" Test Version");
        }
        else
        {
            long nStrCount = strFileVersion_Temp.GetLength();

            if (n4thHeadNum == 5)
                strFileVersion_Temp.Delete(nStrCount - 1, 1);
            strFileVersion_Temp += _T(" Release Version");
        }
        m_valueVersion.SetWindowText(strFileVersion_Temp);
    }

    UpdateAccessModeText();

    return TRUE; // return TRUE unless you set the focus to a control
    // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

void ViewStatus::OnSize(UINT nType, int cx, int cy)
{
    CDialogEx::OnSize(nType, cx, cy);

    if (m_titleAccessMode.GetSafeHwnd() != nullptr)
    {
        UpdateLayout();
    }
}

void ViewStatus::OnStnDblclickValueAccessMode()
{
    m_parent->ShowAccessModeDialog();
}
