//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "ViewVisionBody.h"

//CPP_2_________________________________ This project's headers
#include "resource.h"

//CPP_3_________________________________ Other projects' headers
#include "../ManagementModules/VisionMainTR/VisionMainTR.h"
#include "../VisionNeedLibrary/VisionCommon/VisionBaseDef.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
IMPLEMENT_DYNAMIC(ViewVisionBody, CDialogEx)

ViewVisionBody::ViewVisionBody(VisionMainTR& visionMain, CWnd* parent)
    : CDialogEx(IDD_VIEW_VISION_BODY, parent)
    , m_visionMain(visionMain)
{
}

ViewVisionBody::~ViewVisionBody()
{
}

void ViewVisionBody::UpdateLayout()
{
    CRect rtClient;
    GetClientRect(rtClient);

    auto cx = rtClient.Width();
    auto cy = rtClient.Height();

    if (auto* primaryView = m_visionMain.GetVisionPrimaryDialog())
    {
        primaryView->MoveWindow(0, 0, cx, cy, TRUE);
    }

    // 여기서 Thread UI 를 뿌려준다...
    const CRect rtInlineArea(5, 5, cx - 5, cy - 5);

    static const long nThreadNum
        = m_visionMain.GetThreadNum(); //이하 4개의 변수는 S/W 실행 시 변경 안되는 녀석들이라  static 처리
    static const bool bIsSingleLine
        = (nThreadNum <= 3); //일단은 Apple에서 Thread 개수를 8개까지만 입력 가능하니까 2행까지 만 가능한걸로
    static const long nRowCount = bIsSingleLine ? 1 : 2; // Thread 3개까지는 1줄, 4개부터는 2줄
    static const long nColCount = bIsSingleLine
        ? nThreadNum
        : (long)((float)nThreadNum * .5f + .5f); //nTN <=3 : 1 // nTN==4 : 2 // nTN==5or6 : 3 // nTN==7or8 : 4
    long nCol = 0;
    long nRow = 0;

    for (long nThread = 0; nThread < nThreadNum; nThread++)
    {
        CWnd* pDlg = m_visionMain.GetVisionInlineDialog(nThread);

        nCol = nThread % nColCount;
        nRow = (nThread == nCol)
            ? 0
            : 1; // nThread를 nColCount로 나눈 후 나머지가 nThread와 같다는 건 몫이 0이란 이야기.. 즉, 첫 줄이다. 나눗셈 연산하기 싫어서 3항 연산처리

        if (pDlg->GetSafeHwnd())
        {
            //pDlg->MoveWindow(::GetContentSubArea(rtInlineArea, nThread, nThread + 1, nThreadNum, 0, 1, 1), TRUE);//원래 1줄 배열 Code
            pDlg->MoveWindow(
                ::GetContentSubArea(rtInlineArea, nCol, nCol + 1, nColCount, nRow, nRow + 1, nRowCount), TRUE);
        }
    }
}

void ViewVisionBody::OnOK()
{
}

void ViewVisionBody::OnCancel()
{
}

void ViewVisionBody::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(ViewVisionBody, CDialogEx)
ON_WM_SIZE()
END_MESSAGE_MAP()

// ViewVisionBody 메시지 처리기

BOOL ViewVisionBody::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    m_visionMain.InitializeVisionInlineUI(GetSafeHwnd());
    m_visionMain.ShowVisionPrimaryUI(GetSafeHwnd());

    m_initialized = TRUE;
    UpdateLayout();

    return TRUE; // return TRUE unless you set the focus to a control
    // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

void ViewVisionBody::OnSize(UINT nType, int cx, int cy)
{
    CDialogEx::OnSize(nType, cx, cy);

    if (m_initialized)
    {
        UpdateLayout();
    }
}
